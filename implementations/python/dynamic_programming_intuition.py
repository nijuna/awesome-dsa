"""
Dynamic Programming Intuition implementations in Python.

Demonstrates:
1. Fibonacci: Naive Recursion vs Top-Down Memoization vs Bottom-Up Tabulation vs Space Optimization.
2. 2D Grid Unique Paths: Full 2D Table vs 1D Rolling Array.
3. 0-1 Knapsack: Optimal Substructure & 1D reverse-iteration table.
4. Comprehensive unit tests.
"""

import unittest
from typing import List, Dict


def fib_naive(n: int) -> int:
    """Naive recursion: O(2^n) time, O(n) stack space."""
    if n <= 1:
        return n
    return fib_naive(n - 1) + fib_naive(n - 2)


def fib_memo(n: int, memo: Dict[int, int] = None) -> int:
    """Top-down memoization: O(n) time, O(n) space."""
    if memo is None:
        memo = {}
    if n <= 1:
        return n
    if n not in memo:
        memo[n] = fib_memo(n - 1, memo) + fib_memo(n - 2, memo)
    return memo[n]


def fib_tab(n: int) -> int:
    """Bottom-up tabulation: O(n) time, O(n) table space."""
    if n <= 1:
        return n
    dp = [0] * (n + 1)
    dp[1] = 1
    for i in range(2, n + 1):
        dp[i] = dp[i - 1] + dp[i - 2]
    return dp[n]


def fib_opt(n: int) -> int:
    """Space-optimized tabulation: O(n) time, O(1) space."""
    if n <= 1:
        return n
    prev2, prev1 = 0, 1
    for _ in range(2, n + 1):
        curr = prev1 + prev2
        prev2 = prev1
        prev1 = curr
    return prev1


def grid_paths_2d(m: int, n: int) -> int:
    """Unique paths in m x n grid using full 2D table: O(m * n) time and space."""
    dp = [[1] * n for _ in range(m)]
    for i in range(1, m):
        for j in range(1, n):
            dp[i][j] = dp[i - 1][j] + dp[i][j - 1]
    return dp[m - 1][n - 1]


def grid_paths_1d(m: int, n: int) -> int:
    """Unique paths in m x n grid using 1D rolling array: O(m * n) time, O(n) space."""
    dp = [1] * n
    for _ in range(1, m):
        for j in range(1, n):
            dp[j] += dp[j - 1]
    return dp[n - 1]


def knapsack_1d(weights: List[int], values: List[int], capacity: int) -> int:
    """0-1 Knapsack using 1D reverse-iteration table: O(n * W) time, O(W) space."""
    dp = [0] * (capacity + 1)
    for w, v in zip(weights, values):
        for cap in range(capacity, w - 1, -1):
            dp[cap] = max(dp[cap], dp[cap - w] + v)
    return dp[capacity]


class TestDynamicProgrammingIntuition(unittest.TestCase):
    def test_fibonacci(self):
        n = 15
        val_naive = fib_naive(n)
        val_memo = fib_memo(n)
        val_tab = fib_tab(n)
        val_opt = fib_opt(n)

        self.assertEqual(val_naive, 610)
        self.assertEqual(val_memo, 610)
        self.assertEqual(val_tab, 610)
        self.assertEqual(val_opt, 610)
        self.assertEqual(fib_opt(50), 12586269025)

    def test_grid_paths(self):
        self.assertEqual(grid_paths_2d(3, 7), 28)
        self.assertEqual(grid_paths_1d(3, 7), 28)
        self.assertEqual(grid_paths_1d(1, 1), 1)
        self.assertEqual(grid_paths_1d(3, 3), 6)

    def test_knapsack(self):
        weights = [1, 3, 4, 5]
        values = [1, 4, 5, 7]
        capacity = 7
        self.assertEqual(knapsack_1d(weights, values, capacity), 9)


if __name__ == '__main__':
    unittest.main()
