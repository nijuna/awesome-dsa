"""
Square Root (Sqrt) Decomposition with Lazy Range Updates.

Implements block decomposition for point updates, range sum queries,
and lazy range additions in O(sqrt(n)) time and O(n) space.
"""

import math
import random
import unittest
from typing import List


class SqrtDecomposition:
    """Sqrt decomposition supporting point updates, range sum, and range addition."""

    def __init__(self, data: List[int]):
        self.n = len(data)
        self.arr = list(data)
        if self.n == 0:
            self.block_size = 1
            self.num_blocks = 0
            self.block_sum: List[int] = []
            self.lazy: List[int] = []
            return

        self.block_size = max(1, int(math.isqrt(self.n)))
        self.num_blocks = (self.n + self.block_size - 1) // self.block_size

        self.block_sum = [0] * self.num_blocks
        self.lazy = [0] * self.num_blocks

        for i in range(self.n):
            self.block_sum[i // self.block_size] += self.arr[i]

    def update_point(self, idx: int, new_val: int) -> None:
        """Sets arr[idx] = new_val in O(1) time."""
        if not (0 <= idx < self.n):
            raise IndexError("Index out of range")
        b = idx // self.block_size
        effective = self.arr[idx] + self.lazy[b]
        diff = new_val - effective
        self.arr[idx] = new_val - self.lazy[b]
        self.block_sum[b] += diff

    def range_add(self, l: int, r: int, delta: int) -> None:
        """Adds delta to all elements in arr[l .. r] in O(sqrt(n)) time."""
        if not (0 <= l <= r < self.n):
            raise IndexError("Invalid range indices")

        b_l = l // self.block_size
        b_r = r // self.block_size

        if b_l == b_r:
            for i in range(l, r + 1):
                self.arr[i] += delta
            self.block_sum[b_l] += delta * (r - l + 1)
            return

        # Left partial block
        end_l = (b_l + 1) * self.block_size - 1
        for i in range(l, end_l + 1):
            self.arr[i] += delta
        self.block_sum[b_l] += delta * (end_l - l + 1)

        # Full intermediate blocks
        for b in range(b_l + 1, b_r):
            self.lazy[b] += delta
            self.block_sum[b] += delta * self.block_size

        # Right partial block
        start_r = b_r * self.block_size
        for i in range(start_r, r + 1):
            self.arr[i] += delta
        self.block_sum[b_r] += delta * (r - start_r + 1)

    def query_sum(self, l: int, r: int) -> int:
        """Computes sum of arr[l .. r] in O(sqrt(n)) time."""
        if not (0 <= l <= r < self.n):
            raise IndexError("Invalid range indices")

        b_l = l // self.block_size
        b_r = r // self.block_size
        total = 0

        if b_l == b_r:
            for i in range(l, r + 1):
                total += self.arr[i] + self.lazy[b_l]
            return total

        # Left partial block
        end_l = (b_l + 1) * self.block_size - 1
        for i in range(l, end_l + 1):
            total += self.arr[i] + self.lazy[b_l]

        # Full intermediate blocks
        for b in range(b_l + 1, b_r):
            total += self.block_sum[b]

        # Right partial block
        start_r = b_r * self.block_size
        for i in range(start_r, r + 1):
            total += self.arr[i] + self.lazy[b_r]

        return total


class TestSqrtDecomposition(unittest.TestCase):
    def test_basic_sum_and_point_update(self):
        data = [1, 3, 5, 7, 9, 11, 13, 15, 17]
        sd = SqrtDecomposition(data)

        self.assertEqual(sd.query_sum(1, 5), 35)
        sd.update_point(3, 10)  # 7 -> 10
        self.assertEqual(sd.query_sum(1, 5), 38)

    def test_range_add_lazy(self):
        data = list(range(1, 11))  # 1 to 10
        sd = SqrtDecomposition(data)

        sd.range_add(2, 7, 5)
        self.assertEqual(sd.query_sum(3, 3), 9)  # 4 + 5 = 9
        self.assertEqual(sd.query_sum(0, 9), 55 + 6 * 5)  # 85

    def test_stress_against_oracle(self):
        rng = random.Random(42)
        N = 500
        initial = [rng.randint(-100, 100) for _ in range(N)]
        oracle = list(initial)
        sd = SqrtDecomposition(initial)

        for _ in range(2000):
            op = rng.randint(0, 2)
            idx1 = rng.randint(0, N - 1)
            idx2 = rng.randint(0, N - 1)
            l = min(idx1, idx2)
            r = max(idx1, idx2)

            if op == 0:
                expected = sum(oracle[l : r + 1])
                self.assertEqual(sd.query_sum(l, r), expected)
            elif op == 1:
                delta = rng.randint(-50, 50)
                for i in range(l, r + 1):
                    oracle[i] += delta
                sd.range_add(l, r, delta)
            else:
                new_v = rng.randint(-100, 100)
                oracle[l] = new_v
                sd.update_point(l, new_v)


if __name__ == "__main__":
    unittest.main()
