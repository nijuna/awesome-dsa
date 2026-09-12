"""
Sparse Tables for Static Range Queries

This module provides production-grade reference implementations of:
- SparseTableMin: Range Minimum Queries in O(1) time
- SparseTableMax: Range Maximum Queries in O(1) time
- SparseTableGCD: Range Greatest Common Divisor in O(1) time
- SparseTableArgMin: Index of Range Minimum in O(1) time
"""

from typing import List
import math
import unittest


def build_log_table(n: int) -> List[int]:
    """Precomputes floor(log2(i)) for i = 1..n in O(n) time."""
    if n <= 0:
        return []
    log_table = [0] * (n + 1)
    for i in range(2, n + 1):
        log_table[i] = log_table[i // 2] + 1
    return log_table


class SparseTableMin:
    """
    Sparse Table for Range Minimum Query (RMQ) on static arrays.
    Precomputation: O(n log n)
    Query: O(1)
    """
    def __init__(self, a: List[int]):
        self.n = len(a)
        if self.n == 0:
            self.log = []
            self.st = []
            return

        self.log = build_log_table(self.n)
        k_max = self.log[self.n] + 1
        self.st = [[0] * self.n for _ in range(k_max)]
        self.st[0] = a[:]

        k = 1
        while (1 << k) <= self.n:
            length = 1 << k
            half = length >> 1
            for i in range(self.n - length + 1):
                self.st[k][i] = min(self.st[k - 1][i], self.st[k - 1][i + half])
            k += 1

    def query(self, l: int, r: int) -> int:
        if self.n == 0 or l < 0 or r >= self.n or l > r:
            raise IndexError("Invalid query range")
        length = r - l + 1
        k = self.log[length]
        return min(self.st[k][l], self.st[k][r - (1 << k) + 1])


class SparseTableMax:
    """Range Maximum Queries in O(1) time."""
    def __init__(self, a: List[int]):
        self.n = len(a)
        if self.n == 0:
            self.log = []
            self.st = []
            return

        self.log = build_log_table(self.n)
        k_max = self.log[self.n] + 1
        self.st = [[0] * self.n for _ in range(k_max)]
        self.st[0] = a[:]

        k = 1
        while (1 << k) <= self.n:
            length = 1 << k
            half = length >> 1
            for i in range(self.n - length + 1):
                self.st[k][i] = max(self.st[k - 1][i], self.st[k - 1][i + half])
            k += 1

    def query(self, l: int, r: int) -> int:
        if self.n == 0 or l < 0 or r >= self.n or l > r:
            raise IndexError("Invalid query range")
        length = r - l + 1
        k = self.log[length]
        return max(self.st[k][l], self.st[k][r - (1 << k) + 1])


class SparseTableGCD:
    """Range Greatest Common Divisor queries in O(1) gcd evaluations."""
    def __init__(self, a: List[int]):
        self.n = len(a)
        if self.n == 0:
            self.log = []
            self.st = []
            return

        self.log = build_log_table(self.n)
        k_max = self.log[self.n] + 1
        self.st = [[0] * self.n for _ in range(k_max)]
        self.st[0] = a[:]

        k = 1
        while (1 << k) <= self.n:
            length = 1 << k
            half = length >> 1
            for i in range(self.n - length + 1):
                self.st[k][i] = math.gcd(self.st[k - 1][i], self.st[k - 1][i + half])
            k += 1

    def query(self, l: int, r: int) -> int:
        if self.n == 0 or l < 0 or r >= self.n or l > r:
            raise IndexError("Invalid query range")
        length = r - l + 1
        k = self.log[length]
        return math.gcd(self.st[k][l], self.st[k][r - (1 << k) + 1])


class SparseTableArgMin:
    """Range Minimum Query returning the index of the minimum element."""
    def __init__(self, a: List[int]):
        self.a = a[:]
        self.n = len(a)
        if self.n == 0:
            self.log = []
            self.st = []
            return

        self.log = build_log_table(self.n)
        k_max = self.log[self.n] + 1
        self.st = [[0] * self.n for _ in range(k_max)]
        self.st[0] = list(range(self.n))

        k = 1
        while (1 << k) <= self.n:
            length = 1 << k
            half = length >> 1
            for i in range(self.n - length + 1):
                left_idx = self.st[k - 1][i]
                right_idx = self.st[k - 1][i + half]
                self.st[k][i] = left_idx if self.a[left_idx] <= self.a[right_idx] else right_idx
            k += 1

    def query_index(self, l: int, r: int) -> int:
        if self.n == 0 or l < 0 or r >= self.n or l > r:
            raise IndexError("Invalid query range")
        length = r - l + 1
        k = self.log[length]
        left_idx = self.st[k][l]
        right_idx = self.st[k][r - (1 << k) + 1]
        return left_idx if self.a[left_idx] <= self.a[right_idx] else right_idx

    def query_value(self, l: int, r: int) -> int:
        return self.a[self.query_index(l, r)]


# ============================================================================
# Unit Tests
# ============================================================================

class TestSparseTable(unittest.TestCase):
    def test_rmq_min(self):
        a = [5, 2, 4, 7, 1, 3, 6]
        st = SparseTableMin(a)
        self.assertEqual(st.query(0, 6), 1)
        self.assertEqual(st.query(0, 1), 2)
        self.assertEqual(st.query(1, 3), 2)
        self.assertEqual(st.query(1, 5), 1)
        self.assertEqual(st.query(4, 4), 1)
        self.assertEqual(st.query(5, 6), 3)

        # Negative values
        st_neg = SparseTableMin([10, -5, 3, -8, 20])
        self.assertEqual(st_neg.query(0, 4), -8)
        self.assertEqual(st_neg.query(0, 2), -5)
        self.assertEqual(st_neg.query(4, 4), 20)

    def test_rmq_max(self):
        a = [3, 9, 2, 8, 1, 7]
        st = SparseTableMax(a)
        self.assertEqual(st.query(0, 5), 9)
        self.assertEqual(st.query(2, 4), 8)
        self.assertEqual(st.query(0, 0), 3)

    def test_gcd(self):
        a = [24, 18, 42, 60, 100]
        st = SparseTableGCD(a)
        self.assertEqual(st.query(0, 1), 6)
        self.assertEqual(st.query(0, 2), 6)
        self.assertEqual(st.query(3, 4), 20)
        self.assertEqual(st.query(0, 4), 2)

    def test_argmin(self):
        a = [5, 2, 4, 2, 1, 3, 1]
        st = SparseTableArgMin(a)
        self.assertEqual(st.query_index(0, 3), 1)
        self.assertEqual(st.query_index(4, 6), 4)
        self.assertEqual(st.query_value(0, 6), 1)

    def test_single_element(self):
        st = SparseTableMin([42])
        self.assertEqual(st.query(0, 0), 42)


if __name__ == "__main__":
    unittest.main()
