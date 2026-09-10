"""
Reference Implementation: Fenwick Tree (Binary Indexed Tree / BIT)
Supports O(N) linear construction, O(log N) point update, and O(log N) prefix/range sum queries.
"""

from __future__ import annotations
import unittest


class FenwickTree:
    """1-based Fenwick Tree for dynamic prefix sum queries and point updates."""

    def __init__(self, n: int):
        self.n = n
        self.tree = [0] * (n + 1)

    @classmethod
    def from_array(cls, arr: list[int]) -> FenwickTree:
        """Linear time O(N) constructor from an initial array of values."""
        n = len(arr)
        bit = cls(n)
        for i in range(1, n + 1):
            bit.tree[i] += arr[i - 1]
            parent = i + (i & -i)
            if parent <= n:
                bit.tree[parent] += bit.tree[i]
        return bit

    def add(self, i: int, delta: int):
        """Adds delta to 1-based index i in O(log N)."""
        if i <= 0 or i > self.n:
            raise IndexError(f"Index {i} out of bounds for Fenwick Tree of size {self.n}")
        while i <= self.n:
            self.tree[i] += delta
            i += i & -i

    def query(self, i: int) -> int:
        """Returns prefix sum A[1..i] in O(log N)."""
        if i > self.n:
            i = self.n
        total = 0
        while i > 0:
            total += self.tree[i]
            i -= i & -i
        return total

    def range_query(self, l: int, r: int) -> int:
        """Returns range sum A[l..r] in O(log N)."""
        if l > r:
            return 0
        return self.query(r) - self.query(l - 1)


class TestFenwickTree(unittest.TestCase):
    def test_basic_operations(self):
        arr = [1, 3, 5, 7, 9, 11]
        bit = FenwickTree.from_array(arr)

        self.assertEqual(bit.query(1), 1)
        self.assertEqual(bit.query(3), 9)  # 1 + 3 + 5
        self.assertEqual(bit.range_query(2, 4), 15)  # 3 + 5 + 7
        self.assertEqual(bit.range_query(1, 6), 36)

        # Update element 3 (value 5 -> 5 + 6 = 11)
        bit.add(3, 6)
        self.assertEqual(bit.query(3), 15)
        self.assertEqual(bit.range_query(2, 4), 21)
        self.assertEqual(bit.range_query(1, 6), 42)

    def test_edge_cases(self):
        bit = FenwickTree(1)
        bit.add(1, 42)
        self.assertEqual(bit.query(1), 42)
        self.assertEqual(bit.range_query(1, 1), 42)
        self.assertEqual(bit.range_query(2, 1), 0)


if __name__ == "__main__":
    unittest.main()
