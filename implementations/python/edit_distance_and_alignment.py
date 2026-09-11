"""
Reference Implementation: Edit Distance and Sequence Alignment
Demonstrates:
1. Levenshtein Distance: Classical 2D DP (O(nm) Time, O(nm) Space) & Two-Row Rolling Array (O(m) Space).
2. Full Edit Transcript Backtracing (Match, Substitute, Insert, Delete).
3. Needleman-Wunsch Global Alignment: 2D DP Scoring & Two-Row Rolling Array.
4. Needleman-Wunsch Alignment Backtracing (Optimal Aligned Strings with Gap Characters '-').
5. Hamming Distance for Equal-Length Strings (O(n) Time, O(1) Space).

Language: Python 3
"""

import unittest
from typing import List, Tuple, Optional


# ============================================================================
# 1. Levenshtein Edit Distance (Cost Minimization)
# ============================================================================

def levenshtein_distance(a: str, b: str) -> int:
    """
    Classical 2D DP: O(nm) Time, O(nm) Space.
    dp[i][j] = min edits to transform a[0..i-1] into b[0..j-1].
    """
    n, m = len(a), len(b)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(n + 1):
        dp[i][0] = i
    for j in range(m + 1):
        dp[0][j] = j

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            cost = 0 if a[i - 1] == b[j - 1] else 1
            dp[i][j] = min(
                dp[i - 1][j] + 1,      # delete from a
                dp[i][j - 1] + 1,      # insert into a
                dp[i - 1][j - 1] + cost # match or substitute
            )

    return dp[n][m]


def levenshtein_distance_rolling(a: str, b: str) -> int:
    """
    Two-Row Rolling Array Space Optimization: O(nm) Time, O(m) Space.
    """
    n, m = len(a), len(b)
    prev = list(range(m + 1))
    cur = [0] * (m + 1)

    for i in range(1, n + 1):
        cur[0] = i
        for j in range(1, m + 1):
            cost = 0 if a[i - 1] == b[j - 1] else 1
            cur[j] = min(
                prev[j] + 1,
                cur[j - 1] + 1,
                prev[j - 1] + cost
            )
        prev, cur = cur, prev

    return prev[m]


# ============================================================================
# 2. Edit Transcript Reconstruction
# ============================================================================

def levenshtein_with_ops(a: str, b: str) -> Tuple[int, List[Tuple[str, Optional[str], Optional[str]]]]:
    """
    Reconstructs the full sequence of edit operations from a to b.
    Returns (distance, ops) where each op is (type, from_char, to_char).
    """
    n, m = len(a), len(b)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(n + 1):
        dp[i][0] = i
    for j in range(m + 1):
        dp[0][j] = j

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            cost = 0 if a[i - 1] == b[j - 1] else 1
            dp[i][j] = min(
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            )

    ops = []
    i, j = n, m

    while i > 0 or j > 0:
        if i > 0 and j > 0:
            cost = 0 if a[i - 1] == b[j - 1] else 1
            if dp[i][j] == dp[i - 1][j - 1] + cost:
                if cost == 0:
                    ops.append(("match", a[i - 1], b[j - 1]))
                else:
                    ops.append(("substitute", a[i - 1], b[j - 1]))
                i -= 1
                j -= 1
                continue

        if i > 0 and dp[i][j] == dp[i - 1][j] + 1:
            ops.append(("delete", a[i - 1], None))
            i -= 1
        else:
            ops.append(("insert", None, b[j - 1]))
            j -= 1

    ops.reverse()
    return dp[n][m], ops


# ============================================================================
# 3. Needleman-Wunsch Global Alignment (Score Maximization)
# ============================================================================

def needleman_wunsch_score(
    a: str,
    b: str,
    match_score: int = 1,
    mismatch_penalty: int = -1,
    gap_penalty: int = -1
) -> int:
    """
    Computes optimal global alignment score: O(nm) Time, O(nm) Space.
    """
    n, m = len(a), len(b)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(n + 1):
        dp[i][0] = i * gap_penalty
    for j in range(m + 1):
        dp[0][j] = j * gap_penalty

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            diag = dp[i - 1][j - 1] + (match_score if a[i - 1] == b[j - 1] else mismatch_penalty)
            up = dp[i - 1][j] + gap_penalty
            left = dp[i][j - 1] + gap_penalty
            dp[i][j] = max(diag, up, left)

    return dp[n][m]


def needleman_wunsch_score_rolling(
    a: str,
    b: str,
    match_score: int = 1,
    mismatch_penalty: int = -1,
    gap_penalty: int = -1
) -> int:
    """
    Two-Row Rolling Array for Needleman-Wunsch: O(nm) Time, O(m) Space.
    """
    n, m = len(a), len(b)
    prev = [j * gap_penalty for j in range(m + 1)]
    cur = [0] * (m + 1)

    for i in range(1, n + 1):
        cur[0] = i * gap_penalty
        for j in range(1, m + 1):
            diag = prev[j - 1] + (match_score if a[i - 1] == b[j - 1] else mismatch_penalty)
            up = prev[j] + gap_penalty
            left = cur[j - 1] + gap_penalty
            cur[j] = max(diag, up, left)
        prev, cur = cur, prev

    return prev[m]


def needleman_wunsch_align(
    a: str,
    b: str,
    match_score: int = 1,
    mismatch_penalty: int = -1,
    gap_penalty: int = -1
) -> Tuple[int, str, str]:
    """
    Global Alignment Reconstruction with Gap Characters '-': O(nm) Time, O(nm) Space.
    Returns (score, aligned_a, aligned_b).
    """
    n, m = len(a), len(b)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(n + 1):
        dp[i][0] = i * gap_penalty
    for j in range(m + 1):
        dp[0][j] = j * gap_penalty

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            diag = dp[i - 1][j - 1] + (match_score if a[i - 1] == b[j - 1] else mismatch_penalty)
            up = dp[i - 1][j] + gap_penalty
            left = dp[i][j - 1] + gap_penalty
            dp[i][j] = max(diag, up, left)

    aligned_a = []
    aligned_b = []
    i, j = n, m

    while i > 0 or j > 0:
        if i > 0 and j > 0:
            diag = dp[i - 1][j - 1] + (match_score if a[i - 1] == b[j - 1] else mismatch_penalty)
            if dp[i][j] == diag:
                aligned_a.append(a[i - 1])
                aligned_b.append(b[j - 1])
                i -= 1
                j -= 1
                continue

        if i > 0 and dp[i][j] == dp[i - 1][j] + gap_penalty:
            aligned_a.append(a[i - 1])
            aligned_b.append('-')
            i -= 1
        else:
            aligned_a.append('-')
            aligned_b.append(b[j - 1])
            j -= 1

    aligned_a.reverse()
    aligned_b.reverse()
    return dp[n][m], ''.join(aligned_a), ''.join(aligned_b)


# ============================================================================
# 4. Comparative Metrics: Hamming Distance
# ============================================================================

def hamming_distance(a: str, b: str) -> int:
    """
    Counts point mismatches between equal-length sequences: O(n) Time, O(1) Space.
    """
    if len(a) != len(b):
        raise ValueError("Hamming distance requires equal length strings")
    return sum(ca != cb for ca, cb in zip(a, b))


# ============================================================================
# Unit Tests
# ============================================================================

class TestEditDistanceAndAlignment(unittest.TestCase):
    def test_levenshtein_kitten_sitting(self):
        s1, s2 = "kitten", "sitting"
        self.assertEqual(levenshtein_distance(s1, s2), 3)
        self.assertEqual(levenshtein_distance_rolling(s1, s2), 3)

        dist, ops = levenshtein_with_ops(s1, s2)
        self.assertEqual(dist, 3)
        non_matches = [op for op in ops if op[0] != "match"]
        self.assertEqual(len(non_matches), 3)

    def test_levenshtein_horse_ros(self):
        s1, s2 = "horse", "ros"
        self.assertEqual(levenshtein_distance(s1, s2), 3)
        self.assertEqual(levenshtein_distance_rolling(s1, s2), 3)

    def test_empty_and_identical(self):
        self.assertEqual(levenshtein_distance("", ""), 0)
        self.assertEqual(levenshtein_distance("abc", ""), 3)
        self.assertEqual(levenshtein_distance("", "abcd"), 4)
        self.assertEqual(levenshtein_distance("algorithm", "algorithm"), 0)
        self.assertEqual(levenshtein_distance_rolling("algorithm", "algorithm"), 0)

    def test_needleman_wunsch(self):
        dna1, dna2 = "GATTACA", "GCATGCU"
        score = needleman_wunsch_score(dna1, dna2, 1, -1, -1)
        rolling = needleman_wunsch_score_rolling(dna1, dna2, 1, -1, -1)
        self.assertEqual(score, rolling)

        align_score, a_str, b_str = needleman_wunsch_align(dna1, dna2, 1, -1, -1)
        self.assertEqual(align_score, score)
        self.assertEqual(len(a_str), len(b_str))

        # Manual validation
        calc = 0
        for ca, cb in zip(a_str, b_str):
            if ca == '-' or cb == '-':
                calc += -1
            elif ca == cb:
                calc += 1
            else:
                calc += -1
        self.assertEqual(calc, score)

    def test_hamming_distance(self):
        self.assertEqual(hamming_distance("karolin", "kathrin"), 3)
        self.assertEqual(hamming_distance("1011101", "1001001"), 2)
        with self.assertRaises(ValueError):
            hamming_distance("abc", "abcd")


if __name__ == '__main__':
    unittest.main()
