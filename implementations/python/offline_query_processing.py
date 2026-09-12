"""Offline Query Processing Paradigms: CDQ Divide-and-Conquer for 3D Partial Orders
and Offline Sweep-Line with Fenwick Tree for Range Queries.

Implements Arthur's Two-Layer API:
  - Layer 1: Rollback Fenwick Tree (Binary Indexed Tree), in-place dimension sorting,
             multi-dimensional partial order dominance logic.
  - Layer 2: Safe OfflineQueryEngine supporting 3D dominance count O(N log^2 N),
             distinct elements offline sweep O((N + Q) log N), and differential verification.
"""

from __future__ import annotations
import bisect
import random
import unittest
from typing import List, Tuple, Sequence, Dict


class RollbackFenwickTree:
    def __init__(self, size: int):
        self.size = size
        self.tree = [0] * (size + 1)

    def add(self, idx: int, delta: int) -> None:
        while idx <= self.size:
            self.tree[idx] += delta
            idx += idx & -idx

    def query(self, idx: int) -> int:
        total = 0
        while idx > 0:
            total += self.tree[idx]
            idx -= idx & -idx
        return total

    def query_range(self, l: int, r: int) -> int:
        if l > r:
            return 0
        return self.query(r) - self.query(l - 1)


class Element3D:
    def __init__(self, item_id: int, a: int, b: int, c: int, count: int = 1):
        self.id = item_id
        self.a = a
        self.b = b
        self.c = c
        self.count = count
        self.ans = 0

    def key(self) -> Tuple[int, int, int]:
        return (self.a, self.b, self.c)


class OfflineQueryEngine:
    @staticmethod
    def solve_3d_partial_order(points: Sequence[Tuple[int, int, int]]) -> List[int]:
        """Calculates for each point i, count of j (j != i) such that a_j <= a_i, b_j <= b_i, c_j <= c_i.

        Complexity: O(N log^2 N) via CDQ Divide-and-Conquer.
        """
        n = len(points)
        if n == 0:
            return []

        elems = [Element3D(i, p[0], p[1], p[2]) for i, p in enumerate(points)]
        elems.sort(key=lambda x: (x.a, x.b, x.c))

        # Deduplicate identical points
        unique_elems: List[Element3D] = []
        for e in elems:
            if unique_elems and e.key() == unique_elems[-1].key():
                unique_elems[-1].count += 1
            else:
                unique_elems.append(Element3D(e.id, e.a, e.b, e.c, e.count))

        # Coordinate compress c
        all_c = sorted(list(set(e.c for e in unique_elems)))
        c_map = {val: i + 1 for i, val in enumerate(all_c)}
        for e in unique_elems:
            e.c = c_map[e.c]

        bit = RollbackFenwickTree(len(all_c) + 2)

        def _cdq(left: int, right: int) -> None:
            if left >= right:
                return
            mid = (left + right) // 2
            _cdq(left, mid)
            _cdq(mid + 1, right)

            # Two-pointer merge on dimension b
            temp: List[Element3D] = []
            i = left
            j = mid + 1

            while i <= mid and j <= right:
                if unique_elems[i].b <= unique_elems[j].b:
                    bit.add(unique_elems[i].c, unique_elems[i].count)
                    temp.append(unique_elems[i])
                    i += 1
                else:
                    unique_elems[j].ans += bit.query(unique_elems[j].c)
                    temp.append(unique_elems[j])
                    j += 1

            while j <= right:
                unique_elems[j].ans += bit.query(unique_elems[j].c)
                temp.append(unique_elems[j])
                j += 1

            # Roll back BIT modifications from left half
            for p in range(left, i):
                bit.add(unique_elems[p].c, -unique_elems[p].count)

            while i <= mid:
                temp.append(unique_elems[i])
                i += 1

            unique_elems[left:right + 1] = temp

        _cdq(0, len(unique_elems) - 1)

        results = [0] * n
        for e in unique_elems:
            dominance = e.ans + (e.count - 1)
            # Assign to original points sharing same coordinates
            for i, p in enumerate(points):
                if (p[0], p[1], c_map[p[2]]) == (e.a, e.b, e.c):
                    results[i] = dominance

        return results

    @staticmethod
    def solve_distinct_range_queries(arr: Sequence[int],
                                      queries: Sequence[Tuple[int, int]]) -> List[int]:
        """Solves distinct elements queries offline in O((N + Q) log N) via sweep-line and BIT."""
        n = len(arr)
        q_count = len(queries)
        if q_count == 0:
            return []

        queries_by_r: List[List[Tuple[int, int]]] = [[] for _ in range(n)]
        for q_id, (l, r) in enumerate(queries):
            queries_by_r[r].append((q_id, l))

        bit = RollbackFenwickTree(n + 1)
        last_pos: Dict[int, int] = {}
        answers = [0] * q_count

        for r in range(n):
            val = arr[r]
            if val in last_pos:
                bit.add(last_pos[val] + 1, -1)
            last_pos[val] = r
            bit.add(r + 1, 1)

            for q_id, l in queries_by_r[r]:
                answers[q_id] = bit.query_range(l + 1, r + 1)

        return answers


# ============================================================================
# Differential Oracles
# ============================================================================

def naive_3d_partial_order(points: Sequence[Tuple[int, int, int]]) -> List[int]:
    n = len(points)
    results = [0] * n
    for i in range(n):
        cnt = 0
        for j in range(n):
            if i == j:
                continue
            if (points[j][0] <= points[i][0] and
                points[j][1] <= points[i][1] and
                points[j][2] <= points[i][2]):
                cnt += 1
        results[i] = cnt
    return results


def naive_distinct_queries(arr: Sequence[int], queries: Sequence[Tuple[int, int]]) -> List[int]:
    return [len(set(arr[l:r + 1])) for l, r in queries]


# ============================================================================
# Unit & Differential Tests
# ============================================================================

class TestOfflineQueryProcessing(unittest.TestCase):
    def test_basic_queries(self):
        # 3D Partial order
        pts = [(1, 1, 1), (2, 2, 2), (3, 3, 3), (1, 2, 3)]
        res_cdq = OfflineQueryEngine.solve_3d_partial_order(pts)
        self.assertEqual(res_cdq, [0, 1, 3, 1])

        # Distinct range queries
        arr = [1, 1, 2, 1, 3]
        queries = [(0, 4), (1, 3), (2, 4), (0, 1)]
        res_distinct = OfflineQueryEngine.solve_distinct_range_queries(arr, queries)
        self.assertEqual(res_distinct, [3, 2, 3, 1])

    def test_differential_cdq(self):
        rng = random.Random(42)
        for _ in range(25):
            n = 35
            pts = [(rng.randint(1, 25), rng.randint(1, 25), rng.randint(1, 25)) for _ in range(n)]
            cdq = OfflineQueryEngine.solve_3d_partial_order(pts)
            naive = naive_3d_partial_order(pts)
            self.assertEqual(cdq, naive)

    def test_differential_sweep_line(self):
        rng = random.Random(1337)
        for _ in range(25):
            n = 50
            q = 30
            arr = [rng.randint(1, 20) for _ in range(n)]
            queries = []
            for _ in range(q):
                l = rng.randint(0, n - 1)
                r = rng.randint(0, n - 1)
                if l > r:
                    l, r = r, l
                queries.append((l, r))

            sweep_res = OfflineQueryEngine.solve_distinct_range_queries(arr, queries)
            naive_res = naive_distinct_queries(arr, queries)
            self.assertEqual(sweep_res, naive_res)


if __name__ == "__main__":
    unittest.main()
