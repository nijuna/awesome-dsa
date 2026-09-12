"""
Reference Implementation: Interval and Matrix Dynamic Programming
Demonstrates:
1. Matrix Chain Multiplication (MCM): Cost computation O(n^3) and parenthesization reconstruction.
2. Optimal Binary Search Tree (OBST): Classical O(n^3) and Knuth-optimized O(n^2) search cost.
3. Burst Balloons: Inverted interval DP ("choose last balloon to burst") O(n^3).
4. Cutting Sticks: Segment-based interval DP O(c^3).
5. Longest Palindromic Subsequence (LPS): Boundary-shrinking O(n^2) DP with witness reconstruction.

Language: Python 3
"""

import unittest
from typing import List, Tuple


# ============================================================================
# 1. Matrix Chain Multiplication (MCM)
# ============================================================================

def matrix_chain_cost(dims: List[int]) -> int:
    """
    Computes the minimum scalar multiplication cost for a chain of matrices.
    dims has size n + 1, where matrix A_i has dimensions dims[i] x dims[i+1].
    Time: O(n^3), Space: O(n^2).
    """
    n = len(dims) - 1
    if n <= 1:
        return 0

    inf = float('inf')
    dp = [[0] * n for _ in range(n)]

    for length in range(2, n + 1):
        for i in range(n - length + 1):
            j = i + length - 1
            dp[i][j] = inf

            for k in range(i, j):
                cost = dp[i][k] + dp[k + 1][j] + dims[i] * dims[k + 1] * dims[j + 1]
                if cost < dp[i][j]:
                    dp[i][j] = cost

    return dp[0][n - 1]


def matrix_chain_parenthesization(dims: List[int]) -> Tuple[int, str]:
    """
    Computes minimum cost and reconstructs the optimal parenthesization string.
    Returns (min_cost, parenthesized_expression).
    """
    n = len(dims) - 1
    if n <= 0:
        return 0, ""
    if n == 1:
        return 0, "A0"

    inf = float('inf')
    dp = [[0] * n for _ in range(n)]
    split = [[-1] * n for _ in range(n)]

    for length in range(2, n + 1):
        for i in range(n - length + 1):
            j = i + length - 1
            dp[i][j] = inf

            for k in range(i, j):
                cost = dp[i][k] + dp[k + 1][j] + dims[i] * dims[k + 1] * dims[j + 1]
                if cost < dp[i][j]:
                    dp[i][j] = cost
                    split[i][j] = k

    def build(i: int, j: int) -> str:
        if i == j:
            return f"A{i}"
        k = split[i][j]
        return f"({build(i, k)} x {build(k + 1, j)})"

    return dp[0][n - 1], build(0, n - 1)


# ============================================================================
# 2. Optimal Binary Search Tree (OBST)
# ============================================================================

def optimal_bst_cost(freq: List[int]) -> int:
    """
    Classical O(n^3) OBST search cost computation with prefix sum optimization.
    freq[i] is the search frequency of key i.
    """
    n = len(freq)
    if n == 0:
        return 0

    prefix = [0] * (n + 1)
    for i in range(n):
        prefix[i + 1] = prefix[i] + freq[i]

    def range_sum(l: int, r: int) -> int:
        return prefix[r + 1] - prefix[l]

    dp = [[0] * n for _ in range(n)]

    for i in range(n):
        dp[i][i] = freq[i]

    for length in range(2, n + 1):
        for i in range(n - length + 1):
            j = i + length - 1
            dp[i][j] = float('inf')
            total = range_sum(i, j)

            for r in range(i, j + 1):
                left = dp[i][r - 1] if r > i else 0
                right = dp[r + 1][j] if r < j else 0
                cost = left + right + total
                if cost < dp[i][j]:
                    dp[i][j] = cost

    return dp[0][n - 1]


def optimal_bst_cost_knuth(freq: List[int]) -> int:
    """
    Knuth-Yao Optimized OBST: O(n^2) Time.
    Uses monotonicity of optimal root positions: opt[i][j-1] <= opt[i][j] <= opt[i+1][j].
    """
    n = len(freq)
    if n == 0:
        return 0

    prefix = [0] * (n + 1)
    for i in range(n):
        prefix[i + 1] = prefix[i] + freq[i]

    def range_sum(l: int, r: int) -> int:
        return prefix[r + 1] - prefix[l]

    dp = [[0] * n for _ in range(n)]
    opt = [[0] * n for _ in range(n)]

    for i in range(n):
        dp[i][i] = freq[i]
        opt[i][i] = i

    for length in range(2, n + 1):
        for i in range(n - length + 1):
            j = i + length - 1
            dp[i][j] = float('inf')
            total = range_sum(i, j)

            start_r = opt[i][j - 1]
            end_r = opt[i + 1][j] if (i + 1 <= j) else j

            for r in range(start_r, end_r + 1):
                left = dp[i][r - 1] if r > i else 0
                right = dp[r + 1][j] if r < j else 0
                cost = left + right + total
                if cost < dp[i][j]:
                    dp[i][j] = cost
                    opt[i][j] = r

    return dp[0][n - 1]


# ============================================================================
# 3. Burst Balloons ("Choose the Last Action")
# ============================================================================

def burst_balloons(nums: List[int]) -> int:
    """
    Burst Balloons: Maximizes coins by choosing the LAST balloon to burst in interval (i, j).
    Time: O(n^3), Space: O(n^2).
    """
    if not nums:
        return 0

    a = [1] + list(nums) + [1]
    n = len(a)
    dp = [[0] * n for _ in range(n)]

    for length in range(2, n):
        for i in range(n - length):
            j = i + length
            for k in range(i + 1, j):
                gain = dp[i][k] + dp[k][j] + a[i] * a[k] * a[j]
                if gain > dp[i][j]:
                    dp[i][j] = gain

    return dp[0][n - 1]


# ============================================================================
# 4. Cutting Sticks
# ============================================================================

def cutting_sticks(stick_len: int, cuts: List[int]) -> int:
    """
    Cutting Sticks: Computes minimum cost to perform cuts on a stick of given length.
    Each cut costs the current length of the segment being cut.
    Time: O(c^3) where c is the number of cut positions.
    """
    if not cuts or stick_len <= 0:
        return 0

    pos = [0] + sorted(cuts) + [stick_len]
    m = len(pos)
    dp = [[0] * m for _ in range(m)]

    for length in range(2, m):
        for i in range(m - length):
            j = i + length
            dp[i][j] = float('inf')
            segment_cost = pos[j] - pos[i]
            for k in range(i + 1, j):
                cost = dp[i][k] + dp[k][j] + segment_cost
                if cost < dp[i][j]:
                    dp[i][j] = cost

    return dp[0][m - 1]


# ============================================================================
# 5. Longest Palindromic Subsequence (LPS)
# ============================================================================

def longest_palindromic_subsequence(s: str) -> int:
    """
    Computes length of Longest Palindromic Subsequence using boundary shrinking DP.
    Time: O(n^2), Space: O(n^2).
    """
    n = len(s)
    if n <= 1:
        return n

    dp = [[0] * n for _ in range(n)]

    for i in range(n):
        dp[i][i] = 1

    for length in range(2, n + 1):
        for i in range(n - length + 1):
            j = i + length - 1
            if s[i] == s[j]:
                dp[i][j] = 2 if length == 2 else dp[i + 1][j - 1] + 2
            else:
                dp[i][j] = max(dp[i + 1][j], dp[i][j - 1])

    return dp[0][n - 1]


def longest_palindromic_subsequence_reconstruct(s: str) -> str:
    """
    Reconstructs one optimal Longest Palindromic Subsequence witness string.
    Time: O(n^2), Space: O(n^2).
    """
    n = len(s)
    if n == 0:
        return ""
    if n == 1:
        return s

    dp = [[0] * n for _ in range(n)]

    for i in range(n):
        dp[i][i] = 1

    for length in range(2, n + 1):
        for i in range(n - length + 1):
            j = i + length - 1
            if s[i] == s[j]:
                dp[i][j] = 2 if length == 2 else dp[i + 1][j - 1] + 2
            else:
                dp[i][j] = max(dp[i + 1][j], dp[i][j - 1])

    i, j = 0, n - 1
    left_half = []
    right_half = []

    while i <= j:
        if i == j:
            left_half.append(s[i])
            break
        if s[i] == s[j]:
            left_half.append(s[i])
            right_half.append(s[j])
            i += 1
            j -= 1
        elif dp[i + 1][j] >= dp[i][j - 1]:
            i += 1
        else:
            j -= 1

    right_half.reverse()
    return "".join(left_half + right_half)


# ============================================================================
# Unit Tests
# ============================================================================

class TestIntervalAndMatrixDP(unittest.TestCase):
    def test_matrix_chain_multiplication(self):
        dims = [10, 100, 5, 50]
        cost = matrix_chain_cost(dims)
        self.assertEqual(cost, 7500)

        rec_cost, expr = matrix_chain_parenthesization(dims)
        self.assertEqual(rec_cost, 7500)
        self.assertEqual(expr, "((A0 x A1) x A2)")

        clrs_dims = [30, 35, 15, 5, 10, 20, 25]
        self.assertEqual(matrix_chain_cost(clrs_dims), 15125)

    def test_optimal_bst(self):
        freq = [34, 8, 50]
        cost_cubic = optimal_bst_cost(freq)
        cost_knuth = optimal_bst_cost_knuth(freq)
        self.assertEqual(cost_cubic, cost_knuth)
        self.assertEqual(cost_cubic, 142)

        freq2 = [4, 2, 6, 3]
        self.assertEqual(optimal_bst_cost(freq2), optimal_bst_cost_knuth(freq2))

    def test_burst_balloons(self):
        self.assertEqual(burst_balloons([3, 1, 5, 8]), 167)
        self.assertEqual(burst_balloons([5]), 5)
        self.assertEqual(burst_balloons([]), 0)

    def test_cutting_sticks(self):
        self.assertEqual(cutting_sticks(10, [2, 4, 7]), 20)
        self.assertEqual(cutting_sticks(5, []), 0)
        self.assertEqual(cutting_sticks(10, [5]), 10)

    def test_longest_palindromic_subsequence(self):
        s1 = "bbbab"
        self.assertEqual(longest_palindromic_subsequence(s1), 4)
        rec1 = longest_palindromic_subsequence_reconstruct(s1)
        self.assertEqual(len(rec1), 4)
        self.assertEqual(rec1, "bbbb")

        s2 = "cbbd"
        self.assertEqual(longest_palindromic_subsequence(s2), 2)
        rec2 = longest_palindromic_subsequence_reconstruct(s2)
        self.assertEqual(len(rec2), 2)
        self.assertEqual(rec2, "bb")

        self.assertEqual(longest_palindromic_subsequence(""), 0)
        self.assertEqual(longest_palindromic_subsequence_reconstruct(""), "")
        self.assertEqual(longest_palindromic_subsequence("a"), 1)
        self.assertEqual(longest_palindromic_subsequence_reconstruct("a"), "a")


if __name__ == '__main__':
    unittest.main()
