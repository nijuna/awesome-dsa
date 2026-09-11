"""
Reference Implementation: 1D and 2D Dynamic Programming Foundations
Demonstrates:
1. 1D State Transitions: Fibonacci (Memoization, Tabulation, O(1) Space Optimization).
2. 1D Combinatorial Counting: Climbing Stairs (Prefix Aggregation).
3. 2D Grid Path Counting: Overlapping Subproblems in Two Dimensions.
4. 2D Grid Cost Optimization: Minimum Path Sum on Weighted Lattices.
5. Optimal Solution Reconstruction: Backtracking via Parent Pointers.

Language: Python 3
"""

import unittest
from typing import List, Tuple


# ============================================================================
# 1. 1D DP: Fibonacci Sequences
# ============================================================================

def fibonacci_memo(n: int) -> int:
    """
    Top-down memoization: O(N) Time, O(N) Auxiliary Space & Stack Depth.
    """
    if n < 0:
        raise ValueError("Fibonacci index must be non-negative")
    if n <= 1:
        return n

    memo = [-1] * (n + 1)
    memo[0], memo[1] = 0, 1

    def solve(x: int) -> int:
        if memo[x] != -1:
            return memo[x]
        memo[x] = solve(x - 1) + solve(x - 2)
        return memo[x]

    return solve(n)


def fibonacci_tab(n: int) -> int:
    """
    Bottom-up tabulation: O(N) Time, O(N) Table Space.
    """
    if n < 0:
        raise ValueError("Fibonacci index must be non-negative")
    if n <= 1:
        return n

    dp = [0] * (n + 1)
    dp[1] = 1

    for i in range(2, n + 1):
        dp[i] = dp[i - 1] + dp[i - 2]

    return dp[n]


def fibonacci_optimized(n: int) -> int:
    """
    Space-optimized tabulation: O(N) Time, O(1) Space.
    """
    if n < 0:
        raise ValueError("Fibonacci index must be non-negative")
    if n <= 1:
        return n

    prev2, prev1 = 0, 1
    for _ in range(2, n + 1):
        prev2, prev1 = prev1, prev1 + prev2
    return prev1


# ============================================================================
# 2. 1D Combinatorial Counting: Climbing Stairs
# ============================================================================

def climbing_stairs(n: int) -> int:
    """
    Computes ways to reach step n taking 1 or 2 steps: O(N) Time, O(1) Space.
    """
    if n < 0:
        raise ValueError("Step count must be non-negative")
    if n <= 1:
        return 1

    prev2, prev1 = 1, 1
    for _ in range(2, n + 1):
        prev2, prev1 = prev1, prev1 + prev2
    return prev1


# ============================================================================
# 3. 2D Grid Path Counting
# ============================================================================

def count_grid_paths(rows: int, cols: int) -> int:
    """
    Computes unique paths from (0,0) to (rows-1, cols-1) moving only Right or Down.
    Time: O(rows * cols), Space: O(rows * cols).
    """
    if rows <= 0 or cols <= 0:
        return 0

    dp = [[0] * cols for _ in range(rows)]
    dp[0][0] = 1

    for i in range(rows):
        for j in range(cols):
            if i > 0:
                dp[i][j] += dp[i - 1][j]
            if j > 0:
                dp[i][j] += dp[i][j - 1]

    return dp[rows - 1][cols - 1]


def count_grid_paths_optimized(rows: int, cols: int) -> int:
    """
    Space-optimized 2D path counting using a single-row buffer: O(cols) Space.
    """
    if rows <= 0 or cols <= 0:
        return 0

    dp = [1] * cols

    for _ in range(1, rows):
        for j in range(1, cols):
            dp[j] += dp[j - 1]

    return dp[cols - 1]


# ============================================================================
# 4. 2D Grid Optimization: Minimum Path Sum
# ============================================================================

def min_path_sum(cost: List[List[int]]) -> int:
    """
    Computes minimum cost from (0,0) to (rows-1, cols-1) moving Right or Down.
    Time: O(rows * cols), Space: O(rows * cols).
    """
    if not cost or not cost[0]:
        return 0

    rows, cols = len(cost), len(cost[0])
    inf = float('inf')
    dp = [[inf] * cols for _ in range(rows)]
    dp[0][0] = cost[0][0]

    for i in range(rows):
        for j in range(cols):
            if i == 0 and j == 0:
                continue

            best = inf
            if i > 0:
                best = min(best, dp[i - 1][j])
            if j > 0:
                best = min(best, dp[i][j - 1])

            dp[i][j] = best + cost[i][j]

    return int(dp[rows - 1][cols - 1])


# ============================================================================
# 5. Solution Path Reconstruction (Backtracking via Parent Links)
# ============================================================================

def reconstruct_min_path(cost: List[List[int]]) -> List[Tuple[int, int]]:
    """
    Reconstructs the exact sequence of grid coordinates forming the minimum path.
    Returns list of (row, col) pairs from (0,0) to (rows-1, cols-1).
    """
    if not cost or not cost[0]:
        return []

    rows, cols = len(cost), len(cost[0])
    inf = float('inf')
    dp = [[inf] * cols for _ in range(rows)]
    parent = [[(-1, -1)] * cols for _ in range(rows)]

    dp[0][0] = cost[0][0]

    for i in range(rows):
        for j in range(cols):
            if i == 0 and j == 0:
                continue

            from_up = dp[i - 1][j] if i > 0 else inf
            from_left = dp[i][j - 1] if j > 0 else inf

            if from_up <= from_left:
                dp[i][j] = from_up + cost[i][j]
                parent[i][j] = (i - 1, j)
            else:
                dp[i][j] = from_left + cost[i][j]
                parent[i][j] = (i, j - 1)

    path = []
    i, j = rows - 1, cols - 1
    while i != -1 and j != -1:
        path.append((i, j))
        i, j = parent[i][j]

    path.reverse()
    return path


# ============================================================================
# Unit Tests
# ============================================================================

class TestDynamicProgrammingFoundations(unittest.TestCase):
    def test_fibonacci(self):
        cases = [
            (0, 0), (1, 1), (2, 1), (3, 2), (4, 3), (5, 5),
            (10, 55), (20, 6765), (30, 832040), (45, 1134903170)
        ]
        for n, expected in cases:
            self.assertEqual(fibonacci_memo(n), expected)
            self.assertEqual(fibonacci_tab(n), expected)
            self.assertEqual(fibonacci_optimized(n), expected)

        with self.assertRaises(ValueError):
            fibonacci_memo(-1)

    def test_climbing_stairs(self):
        self.assertEqual(climbing_stairs(0), 1)
        self.assertEqual(climbing_stairs(1), 1)
        self.assertEqual(climbing_stairs(2), 2)
        self.assertEqual(climbing_stairs(3), 3)
        self.assertEqual(climbing_stairs(4), 5)
        self.assertEqual(climbing_stairs(5), 8)

    def test_grid_paths(self):
        self.assertEqual(count_grid_paths(1, 1), 1)
        self.assertEqual(count_grid_paths(1, 5), 1)
        self.assertEqual(count_grid_paths(5, 1), 1)
        self.assertEqual(count_grid_paths(3, 3), 6)
        self.assertEqual(count_grid_paths(3, 7), 28)
        self.assertEqual(count_grid_paths(5, 5), 70)

        self.assertEqual(count_grid_paths_optimized(3, 3), 6)
        self.assertEqual(count_grid_paths_optimized(3, 7), 28)
        self.assertEqual(count_grid_paths_optimized(5, 5), 70)

        self.assertEqual(count_grid_paths(0, 5), 0)
        self.assertEqual(count_grid_paths_optimized(5, 0), 0)

    def test_min_path_sum_and_reconstruction(self):
        grid = [
            [1, 3, 1],
            [1, 5, 1],
            [4, 2, 1]
        ]
        min_cost = min_path_sum(grid)
        self.assertEqual(min_cost, 7)

        path = reconstruct_min_path(grid)
        expected_path = [(0, 0), (0, 1), (0, 2), (1, 2), (2, 2)]
        self.assertEqual(path, expected_path)

        path_sum = sum(grid[r][c] for r, c in path)
        self.assertEqual(path_sum, min_cost)

    def test_edge_grids(self):
        single = [[42]]
        self.assertEqual(min_path_sum(single), 42)
        self.assertEqual(reconstruct_min_path(single), [(0, 0)])

        row_grid = [[2, 4, 1, 3]]
        self.assertEqual(min_path_sum(row_grid), 10)
        self.assertEqual(len(reconstruct_min_path(row_grid)), 4)

        col_grid = [[2], [4], [1], [3]]
        self.assertEqual(min_path_sum(col_grid), 10)
        self.assertEqual(len(reconstruct_min_path(col_grid)), 4)


if __name__ == '__main__':
    unittest.main()
