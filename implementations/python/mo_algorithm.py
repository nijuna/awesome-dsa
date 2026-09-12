"""Mo's Algorithm for Offline Range Queries with Block Decomposition,
Zig-Zag Alternation, and Hilbert Curve Ordering.

Implements Arthur's Two-Layer API:
  - Layer 1: Query sorting predicates (block decomposition, snake/zig-zag order,
             Hilbert space-filling curve 1D projection), active window expansion/contraction.
  - Layer 2: High-level MoEngine supporting distinct elements queries O((N + Q) * sqrt(N)),
             powerful array frequency queries, and differential verification against naive oracles.
"""

from __future__ import annotations
import math
import random
import unittest
from collections import defaultdict
from typing import List, Tuple, Sequence


def hilbert_order(x: int, y: int, p: int = 21, rotate: int = 0) -> int:
    """Projects 2D coordinate (x, y) onto a 1D Hilbert space-filling curve."""
    if p == 0:
        return 0
    hpow = 1 << (p - 1)
    seg = (0 if y < hpow else 3) if x < hpow else (1 if y < hpow else 2)
    seg = (seg + rotate) & 3
    rotate_delta = (3, 0, 0, 1)
    nx = x & (x ^ hpow)
    ny = y & (y ^ hpow)
    nrot = (rotate + rotate_delta[seg]) & 3
    sub_square_size = 1 << (2 * p - 2)
    ans = seg * sub_square_size
    add = hilbert_order(nx, ny, p - 1, nrot)
    ans += add if (seg == 1 or seg == 2) else (sub_square_size - add - 1)
    return ans


class MoQuery:
    def __init__(self, q_id: int, left: int, right: int, block_size: int, use_hilbert: bool = True):
        self.id = q_id
        self.l = left
        self.r = right
        self.block = left // block_size
        self.hilbert_ord = hilbert_order(left, right) if use_hilbert else 0


class MoEngine:
    @staticmethod
    def solve_distinct_elements(arr: Sequence[int],
                                raw_queries: Sequence[Tuple[int, int]],
                                use_hilbert: bool = True) -> List[int]:
        """Counts distinct elements in range [L, R] for each query in O((N + Q) * sqrt(N))."""
        n = len(arr)
        q_count = len(raw_queries)
        if q_count == 0:
            return []

        block_size = max(1, int(n / math.sqrt(q_count)))
        queries = [MoQuery(i, l, r, block_size, use_hilbert) for i, (l, r) in enumerate(raw_queries)]

        if use_hilbert:
            queries.sort(key=lambda q: q.hilbert_ord)
        else:
            queries.sort(key=lambda q: (q.block, q.r if (q.block % 2 == 1) else -q.r))

        freq = defaultdict(int)
        current_distinct = 0
        answers = [0] * q_count

        def add(idx: int) -> None:
            nonlocal current_distinct
            val = arr[idx]
            if freq[val] == 0:
                current_distinct += 1
            freq[val] += 1

        def remove(idx: int) -> None:
            nonlocal current_distinct
            val = arr[idx]
            freq[val] -= 1
            if freq[val] == 0:
                current_distinct -= 1

        cur_l = 0
        cur_r = -1

        for q in queries:
            while cur_l > q.l:
                cur_l -= 1
                add(cur_l)
            while cur_r < q.r:
                cur_r += 1
                add(cur_r)
            while cur_l < q.l:
                remove(cur_l)
                cur_l += 1
            while cur_r > q.r:
                remove(cur_r)
                cur_r -= 1
            answers[q.id] = current_distinct

        return answers

    @staticmethod
    def solve_powerful_array(arr: Sequence[int],
                             raw_queries: Sequence[Tuple[int, int]]) -> List[int]:
        """Calculates sum(freq[x]^2 * x) for each query in O((N + Q) * sqrt(N))."""
        n = len(arr)
        q_count = len(raw_queries)
        if q_count == 0:
            return []

        block_size = max(1, int(n / math.sqrt(q_count)))
        queries = [MoQuery(i, l, r, block_size, True) for i, (l, r) in enumerate(raw_queries)]
        queries.sort(key=lambda q: q.hilbert_ord)

        freq = defaultdict(int)
        current_sum = 0
        answers = [0] * q_count

        def add(idx: int) -> None:
            nonlocal current_sum
            val = arr[idx]
            f = freq[val]
            current_sum += (2 * f + 1) * val
            freq[val] = f + 1

        def remove(idx: int) -> None:
            nonlocal current_sum
            val = arr[idx]
            f = freq[val]
            current_sum -= (2 * f - 1) * val
            freq[val] = f - 1

        cur_l = 0
        cur_r = -1

        for q in queries:
            while cur_l > q.l:
                cur_l -= 1
                add(cur_l)
            while cur_r < q.r:
                cur_r += 1
                add(cur_r)
            while cur_l < q.l:
                remove(cur_l)
                cur_l += 1
            while cur_r > q.r:
                remove(cur_r)
                cur_r -= 1
            answers[q.id] = current_sum

        return answers


# ============================================================================
# Differential Oracles
# ============================================================================

def naive_distinct_queries(arr: Sequence[int], queries: Sequence[Tuple[int, int]]) -> List[int]:
    return [len(set(arr[l:r + 1])) for l, r in queries]


def naive_powerful_queries(arr: Sequence[int], queries: Sequence[Tuple[int, int]]) -> List[int]:
    res = []
    for l, r in queries:
        counts = defaultdict(int)
        for x in arr[l:r + 1]:
            counts[x] += 1
        res.append(sum(f * f * x for x, f in counts.items()))
    return res


# ============================================================================
# Unit & Differential Tests
# ============================================================================

class TestMoAlgorithm(unittest.TestCase):
    def test_basic_queries(self):
        arr = [1, 1, 2, 1, 3]
        queries = [(0, 4), (1, 3), (2, 4), (0, 1)]

        # Distinct
        res_distinct = MoEngine.solve_distinct_elements(arr, queries)
        self.assertEqual(res_distinct, [3, 2, 3, 1])

        # Powerful array
        res_power = MoEngine.solve_powerful_array(arr, queries)
        self.assertEqual(res_power, [14, 6, 6, 4])

    def test_differential_random(self):
        rng = random.Random(1337)
        for _ in range(25):
            n = 60
            q = 40
            arr = [rng.randint(1, 20) for _ in range(n)]
            queries = []
            for _ in range(q):
                l = rng.randint(0, n - 1)
                r = rng.randint(0, n - 1)
                if l > r:
                    l, r = r, l
                queries.append((l, r))

            # Block zig-zag vs Hilbert vs Naive
            res_block = MoEngine.solve_distinct_elements(arr, queries, use_hilbert=False)
            res_hilbert = MoEngine.solve_distinct_elements(arr, queries, use_hilbert=True)
            naive_distinct = naive_distinct_queries(arr, queries)
            self.assertEqual(res_block, naive_distinct)
            self.assertEqual(res_hilbert, naive_distinct)

            # Powerful array
            res_power = MoEngine.solve_powerful_array(arr, queries)
            naive_power = naive_powerful_queries(arr, queries)
            self.assertEqual(res_power, naive_power)


if __name__ == "__main__":
    unittest.main()
