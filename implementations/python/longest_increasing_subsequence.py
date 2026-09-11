"""
Reference Implementation: Longest Increasing Subsequence (LIS)
Demonstrates:
1. Classical Quadratic DP: O(N^2) Time, O(N) Space.
2. Quadratic Parent-Pointer Reconstruction: O(N^2) Time.
3. Patience Sorting / Binary Search (Tails Array): O(N log N) Time, O(N) Space.
4. Optimized Subsequence Reconstruction via Tail Indices & Predecessors: O(N log N) Time.
5. Variant: Longest Non-Decreasing Subsequence via Bisect Right: O(N log N) Time.

Language: Python 3
"""

import unittest
from bisect import bisect_left, bisect_right
from typing import List


# ============================================================================
# 1. Classical DP Formulation: O(N^2) Time, O(N) Space
# ============================================================================

def lis_length_n2(a: List[int]) -> int:
    """
    Computes LIS length in O(N^2) time using dynamic programming:
    dp[i] = length of LIS ending strictly at index i.
    """
    n = len(a)
    if n == 0:
        return 0

    dp = [1] * n
    best = 1

    for i in range(n):
        for j in range(i):
            if a[j] < a[i]:
                dp[i] = max(dp[i], dp[j] + 1)
        best = max(best, dp[i])

    return best


def lis_sequence_n2(a: List[int]) -> List[int]:
    """
    Reconstructs an actual LIS sequence in O(N^2) time using parent links.
    """
    n = len(a)
    if n == 0:
        return []

    dp = [1] * n
    parent = [-1] * n
    best_len = 1
    best_end = 0

    for i in range(n):
        for j in range(i):
            if a[j] < a[i] and dp[j] + 1 > dp[i]:
                dp[i] = dp[j] + 1
                parent[i] = j

        if dp[i] > best_len:
            best_len = dp[i]
            best_end = i

    seq = []
    cur = best_end
    while cur != -1:
        seq.append(a[cur])
        cur = parent[cur]

    seq.reverse()
    return seq


# ============================================================================
# 2. Patience Sorting / Binary Search Formulation: O(N log N) Time
# ============================================================================

def lis_length_nlogn(a: List[int]) -> int:
    """
    Computes LIS length in O(N log N) time using the tails array.
    tails[k] stores the minimum tail element among all valid increasing
    subsequences of length (k + 1) discovered so far.
    """
    tails = []

    for x in a:
        pos = bisect_left(tails, x)
        if pos == len(tails):
            tails.append(x)
        else:
            tails[pos] = x

    return len(tails)


def lis_sequence_nlogn(a: List[int]) -> List[int]:
    """
    Reconstructs an actual LIS sequence in O(N log N) time.
    Maintains:
    - tail_values: smallest tail value for each length
    - tail_indices: array index in `a` of that tail representative
    - parent: tracks predecessor for each index at the moment it joins a prefix
    """
    n = len(a)
    if n == 0:
        return []

    tail_values = []
    tail_indices = []
    parent = [-1] * n

    for i, x in enumerate(a):
        pos = bisect_left(tail_values, x)

        if pos == len(tail_values):
            tail_values.append(x)
            tail_indices.append(i)
        else:
            tail_values[pos] = x
            tail_indices[pos] = i

        if pos > 0:
            parent[i] = tail_indices[pos - 1]

    seq = []
    cur = tail_indices[-1]
    while cur != -1:
        seq.append(a[cur])
        cur = parent[cur]

    seq.reverse()
    return seq


# ============================================================================
# 3. Variant: Longest Non-Decreasing Subsequence (O(N log N))
# ============================================================================

def longest_non_decreasing_length(a: List[int]) -> int:
    """
    Computes Longest Non-Decreasing Subsequence length (allowing equal elements)
    by using bisect_right instead of bisect_left.
    """
    tails = []

    for x in a:
        pos = bisect_right(tails, x)
        if pos == len(tails):
            tails.append(x)
        else:
            tails[pos] = x

    return len(tails)


# ============================================================================
# Unit Tests
# ============================================================================

class TestLongestIncreasingSubsequence(unittest.TestCase):
    def test_empty(self):
        self.assertEqual(lis_length_n2([]), 0)
        self.assertEqual(lis_length_nlogn([]), 0)
        self.assertEqual(lis_sequence_n2([]), [])
        self.assertEqual(lis_sequence_nlogn([]), [])
        self.assertEqual(longest_non_decreasing_length([]), 0)

    def test_single_element(self):
        self.assertEqual(lis_length_n2([42]), 1)
        self.assertEqual(lis_length_nlogn([42]), 1)
        self.assertEqual(lis_sequence_n2([42]), [42])
        self.assertEqual(lis_sequence_nlogn([42]), [42])
        self.assertEqual(longest_non_decreasing_length([42]), 1)

    def test_canonical_example(self):
        seq = [10, 9, 2, 5, 3, 7, 101, 18]
        self.assertEqual(lis_length_n2(seq), 4)
        self.assertEqual(lis_length_nlogn(seq), 4)

        rec_n2 = lis_sequence_n2(seq)
        rec_nlogn = lis_sequence_nlogn(seq)

        self.assertEqual(len(rec_n2), 4)
        self.assertEqual(len(rec_nlogn), 4)

        # Ensure strictly increasing
        self.assertTrue(all(rec_n2[i] < rec_n2[i + 1] for i in range(len(rec_n2) - 1)))
        self.assertTrue(all(rec_nlogn[i] < rec_nlogn[i + 1] for i in range(len(rec_nlogn) - 1)))

    def test_second_example(self):
        seq = [3, 1, 5, 2, 6, 4, 9]
        self.assertEqual(lis_length_n2(seq), 4)
        self.assertEqual(lis_length_nlogn(seq), 4)

        rec = lis_sequence_nlogn(seq)
        self.assertEqual(len(rec), 4)
        self.assertTrue(all(rec[i] < rec[i + 1] for i in range(len(rec) - 1)))

    def test_strictly_decreasing(self):
        seq = [5, 4, 3, 2, 1]
        self.assertEqual(lis_length_n2(seq), 1)
        self.assertEqual(lis_length_nlogn(seq), 1)
        self.assertEqual(len(lis_sequence_n2(seq)), 1)
        self.assertEqual(len(lis_sequence_nlogn(seq)), 1)
        self.assertEqual(longest_non_decreasing_length(seq), 1)

    def test_strictly_increasing(self):
        seq = [1, 2, 3, 4, 5]
        self.assertEqual(lis_length_n2(seq), 5)
        self.assertEqual(lis_length_nlogn(seq), 5)
        self.assertEqual(lis_sequence_n2(seq), seq)
        self.assertEqual(lis_sequence_nlogn(seq), seq)
        self.assertEqual(longest_non_decreasing_length(seq), 5)

    def test_duplicates(self):
        seq = [2, 2, 2, 2]
        self.assertEqual(lis_length_n2(seq), 1)
        self.assertEqual(lis_length_nlogn(seq), 1)
        self.assertEqual(longest_non_decreasing_length(seq), 4)

        mixed = [1, 3, 2, 2, 5]
        self.assertEqual(lis_length_n2(mixed), 3)
        self.assertEqual(lis_length_nlogn(mixed), 3)
        self.assertEqual(longest_non_decreasing_length(mixed), 4)


if __name__ == '__main__':
    unittest.main()
