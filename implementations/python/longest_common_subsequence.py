"""
Reference Implementation: Longest Common Subsequence (LCS)
Demonstrates:
1. Classical 2D Dynamic Programming for LCS Length: O(nm) Time, O(nm) Space.
2. Backtracking Reconstruction of an Optimal Subsequence: O(nm) Time.
3. Space-Optimized Two-Row Rolling Array for LCS Length: O(nm) Time, O(min(n, m)) Space.
4. Shortest Common Supersequence (SCS) Length and String Reconstruction: O(nm) Time.
5. Minimum Insertion-Deletion Distance (Unit-Cost Edit Distance without Substitutions): O(nm) Time.

Language: Python 3
"""

import unittest
from typing import List


# ============================================================================
# 1. Classical 2D Dynamic Programming (Length)
# ============================================================================

def lcs_table(x: str, y: str) -> List[List[int]]:
    """
    Classical 2D DP table computation: O(nm) Time, O(nm) Space.
    dp[i][j] = length of LCS between prefix x[0..i-1] and prefix y[0..j-1].
    """
    n, m = len(x), len(y)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            if x[i - 1] == y[j - 1]:
                dp[i][j] = dp[i - 1][j - 1] + 1
            else:
                dp[i][j] = max(dp[i - 1][j], dp[i][j - 1])

    return dp


def lcs_length(x: str, y: str) -> int:
    """
    Returns the length of the Longest Common Subsequence: O(nm) Time, O(nm) Space.
    """
    n, m = len(x), len(y)
    dp = lcs_table(x, y)
    return dp[n][m]


# ============================================================================
# 2. Subsequence Reconstruction
# ============================================================================

def lcs_sequence(x: str, y: str) -> str:
    """
    Reconstructs one optimal Longest Common Subsequence via backtracking.
    Time: O(nm) to build table + O(n + m) to backtrack.
    Space: O(nm) auxiliary space.
    """
    n, m = len(x), len(y)
    dp = lcs_table(x, y)

    result = []
    i, j = n, m

    while i > 0 and j > 0:
        if x[i - 1] == y[j - 1]:
            result.append(x[i - 1])
            i -= 1
            j -= 1
        elif dp[i - 1][j] >= dp[i][j - 1]:
            # Deterministic tie-breaking: prefer moving up
            i -= 1
        else:
            j -= 1

    result.reverse()
    return "".join(result)


# ============================================================================
# 3. Space-Optimized Rolling Array (Length Only)
# ============================================================================

def lcs_length_rolling(x: str, y: str) -> int:
    """
    Two-Row Rolling Array: O(nm) Time, O(min(n, m)) Space.
    Computes LCS length using only two rows of memory.
    """
    if len(x) < len(y):
        x, y = y, x

    n, m = len(x), len(y)
    prev = [0] * (m + 1)
    cur = [0] * (m + 1)

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            if x[i - 1] == y[j - 1]:
                cur[j] = prev[j - 1] + 1
            else:
                cur[j] = max(prev[j], cur[j - 1])
        prev, cur = cur, prev
        for j in range(m + 1):
            cur[j] = 0

    return prev[m]


# ============================================================================
# 4. Shortest Common Supersequence (SCS)
# ============================================================================

def shortest_common_supersequence_length(x: str, y: str) -> int:
    """
    Shortest Common Supersequence length: n + m - LCS(x, y).
    """
    return len(x) + len(y) - lcs_length(x, y)


def shortest_common_supersequence(x: str, y: str) -> str:
    """
    Reconstructs an optimal Shortest Common Supersequence using the LCS table.
    Time: O(nm) Time, O(nm) Space.
    """
    n, m = len(x), len(y)
    dp = lcs_table(x, y)

    result = []
    i, j = n, m

    while i > 0 and j > 0:
        if x[i - 1] == y[j - 1]:
            result.append(x[i - 1])
            i -= 1
            j -= 1
        elif dp[i - 1][j] >= dp[i][j - 1]:
            result.append(x[i - 1])
            i -= 1
        else:
            result.append(y[j - 1])
            j -= 1

    while i > 0:
        result.append(x[i - 1])
        i -= 1
    while j > 0:
        result.append(y[j - 1])
        j -= 1

    result.reverse()
    return "".join(result)


# ============================================================================
# 5. Minimum Insertion-Deletion Distance
# ============================================================================

def min_insert_delete_distance(x: str, y: str) -> int:
    """
    Minimum number of character insertions and deletions to transform x into y:
    Distance = (n - LCS) + (m - LCS) = n + m - 2 * LCS.
    """
    return len(x) + len(y) - 2 * lcs_length(x, y)


# ============================================================================
# Unit Tests
# ============================================================================

def is_subsequence(sub: str, full: str) -> bool:
    i = 0
    for char in full:
        if i < len(sub) and sub[i] == char:
            i += 1
    return i == len(sub)


class TestLongestCommonSubsequence(unittest.TestCase):
    def test_clrs_example(self):
        x = "ABCBDAB"
        y = "BDCABA"

        length = lcs_length(x, y)
        self.assertEqual(length, 4)
        self.assertEqual(lcs_length_rolling(x, y), 4)

        seq = lcs_sequence(x, y)
        self.assertEqual(len(seq), 4)
        self.assertTrue(is_subsequence(seq, x))
        self.assertTrue(is_subsequence(seq, y))

        scs_len = shortest_common_supersequence_length(x, y)
        self.assertEqual(scs_len, 9)
        scs_str = shortest_common_supersequence(x, y)
        self.assertEqual(len(scs_str), 9)
        self.assertTrue(is_subsequence(x, scs_str))
        self.assertTrue(is_subsequence(y, scs_str))

        id_dist = min_insert_delete_distance(x, y)
        self.assertEqual(id_dist, 5)

    def test_tie_breaking(self):
        x = "ABCD"
        y = "ACBD"

        self.assertEqual(lcs_length(x, y), 3)
        self.assertEqual(lcs_length_rolling(x, y), 3)

        seq = lcs_sequence(x, y)
        self.assertEqual(len(seq), 3)
        self.assertTrue(is_subsequence(seq, x))
        self.assertTrue(is_subsequence(seq, y))
        self.assertIn(seq, ["ABD", "ACD"])

        scs_str = shortest_common_supersequence(x, y)
        self.assertEqual(len(scs_str), 5)
        self.assertTrue(is_subsequence(x, scs_str))
        self.assertTrue(is_subsequence(y, scs_str))

    def test_empty_strings(self):
        self.assertEqual(lcs_length("", ""), 0)
        self.assertEqual(lcs_length_rolling("", ""), 0)
        self.assertEqual(lcs_sequence("", ""), "")
        self.assertEqual(shortest_common_supersequence_length("", ""), 0)
        self.assertEqual(shortest_common_supersequence("", ""), "")
        self.assertEqual(min_insert_delete_distance("", ""), 0)

        self.assertEqual(lcs_length("HELLO", ""), 0)
        self.assertEqual(lcs_length_rolling("HELLO", ""), 0)
        self.assertEqual(lcs_sequence("HELLO", ""), "")
        self.assertEqual(shortest_common_supersequence_length("HELLO", ""), 5)
        self.assertEqual(shortest_common_supersequence("HELLO", ""), "HELLO")
        self.assertEqual(min_insert_delete_distance("HELLO", ""), 5)

        self.assertEqual(lcs_length("", "WORLD"), 0)
        self.assertEqual(lcs_length_rolling("", "WORLD"), 0)
        self.assertEqual(lcs_sequence("", "WORLD"), "")
        self.assertEqual(shortest_common_supersequence_length("", "WORLD"), 5)
        self.assertEqual(shortest_common_supersequence("", "WORLD"), "WORLD")
        self.assertEqual(min_insert_delete_distance("", "WORLD"), 5)

    def test_identical_strings(self):
        s = "ALGORITHM"
        self.assertEqual(lcs_length(s, s), 9)
        self.assertEqual(lcs_length_rolling(s, s), 9)
        self.assertEqual(lcs_sequence(s, s), "ALGORITHM")
        self.assertEqual(shortest_common_supersequence_length(s, s), 9)
        self.assertEqual(shortest_common_supersequence(s, s), "ALGORITHM")
        self.assertEqual(min_insert_delete_distance(s, s), 0)

    def test_disjoint_strings(self):
        x = "ABC"
        y = "DEF"
        self.assertEqual(lcs_length(x, y), 0)
        self.assertEqual(lcs_length_rolling(x, y), 0)
        self.assertEqual(lcs_sequence(x, y), "")
        self.assertEqual(shortest_common_supersequence_length(x, y), 6)
        self.assertEqual(len(shortest_common_supersequence(x, y)), 6)
        self.assertEqual(min_insert_delete_distance(x, y), 6)

    def test_single_characters(self):
        self.assertEqual(lcs_length("A", "A"), 1)
        self.assertEqual(lcs_sequence("A", "A"), "A")
        self.assertEqual(lcs_length("A", "B"), 0)
        self.assertEqual(lcs_sequence("A", "B"), "")


if __name__ == "__main__":
    unittest.main()
