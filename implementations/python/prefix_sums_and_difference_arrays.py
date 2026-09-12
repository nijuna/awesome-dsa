"""
Prefix Sums and Difference Arrays

This module provides production-grade reference implementations of:
- 1D prefix sums and range sum queries
- 2D prefix sums and rectangle sum queries
- 1D difference arrays for offline range additions
- 2D difference arrays for offline rectangle additions
- Build and reconstruction routines
"""

from typing import List, Tuple
import unittest


def prefix_sums(a: List[int]) -> List[int]:
    """
    Computes 1D prefix sums using the 1-based / leading-zero convention.
    pref[0] = 0, pref[i+1] = pref[i] + a[i].

    Time Complexity: O(n)
    Space Complexity: O(n)
    """
    pref = [0] * (len(a) + 1)
    for i, x in enumerate(a):
        pref[i + 1] = pref[i] + x
    return pref


def range_sum(pref: List[int], l: int, r: int) -> int:
    """
    Queries the sum of elements in subarray a[l..r] (inclusive) in O(1) time.
    """
    if l > r:
        return 0
    return pref[r + 1] - pref[l]


def prefix_sums_2d(a: List[List[int]]) -> List[List[int]]:
    """
    Computes 2D prefix sums for an n x m matrix with leading-zero boundaries.
    pref[i+1][j+1] = sum of all a[r][c] for 0 <= r <= i, 0 <= c <= j.

    Time Complexity: O(n * m)
    Space Complexity: O(n * m)
    """
    n = len(a)
    m = len(a[0]) if n else 0
    pref = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(n):
        for j in range(m):
            pref[i + 1][j + 1] = (
                pref[i][j + 1]
                + pref[i + 1][j]
                - pref[i][j]
                + a[i][j]
            )
    return pref


def rectangle_sum(pref: List[List[int]], r1: int, c1: int, r2: int, c2: int) -> int:
    """
    Computes the sum of elements inside subrectangle [r1..r2] x [c1..c2] in O(1) time.
    """
    if r1 > r2 or c1 > c2:
        return 0
    return (
        pref[r2 + 1][c2 + 1]
        - pref[r1][c2 + 1]
        - pref[r2 + 1][c1]
        + pref[r1][c1]
    )


def apply_range_additions(n: int, updates: List[Tuple[int, int, int]]) -> List[int]:
    """
    Applies offline 1D range additions [l, r, x] in O(1) per update using a difference array.

    Time Complexity: O(len(updates) + n)
    Space Complexity: O(n)
    """
    if n <= 0:
        return []
    diff = [0] * (n + 1)

    for l, r, x in updates:
        if l <= r and l < n:
            diff[l] += x
            if r + 1 < n:
                diff[r + 1] -= x

    a = [0] * n
    cur = 0
    for i in range(n):
        cur += diff[i]
        a[i] = cur

    return a


def build_difference_array(a: List[int]) -> List[int]:
    """
    Constructs a difference array from an existing arbitrary array.
    diff[0] = a[0], diff[i] = a[i] - a[i-1].
    """
    n = len(a)
    if n == 0:
        return []

    diff = [0] * n
    diff[0] = a[0]
    for i in range(1, n):
        diff[i] = a[i] - a[i - 1]
    return diff


def reconstruct_from_difference(diff: List[int]) -> List[int]:
    """
    Reconstructs the original array from its difference array via prefix summation.
    """
    n = len(diff)
    if n == 0:
        return []

    a = [0] * n
    a[0] = diff[0]
    for i in range(1, n):
        a[i] = a[i - 1] + diff[i]
    return a


def apply_rectangle_additions(
    n: int, m: int, updates: List[Tuple[int, int, int, int, int]]
) -> List[List[int]]:
    """
    Applies offline 2D rectangle additions [r1, c1, r2, c2, x] in O(1) per update.

    Corner updates:
    diff[r1][c1] += x
    diff[r1][c2+1] -= x
    diff[r2+1][c1] -= x
    diff[r2+1][c2+1] += x

    Reconstructed via 2D prefix summation in O(n * m).
    """
    if n <= 0 or m <= 0:
        return []

    diff = [[0] * (m + 1) for _ in range(n + 1)]

    for r1, c1, r2, c2, x in updates:
        if r1 <= r2 and c1 <= c2 and r1 < n and c1 < m:
            diff[r1][c1] += x
            if c2 + 1 < m:
                diff[r1][c2 + 1] -= x
            if r2 + 1 < n:
                diff[r2 + 1][c1] -= x
            if r2 + 1 < n and c2 + 1 < m:
                diff[r2 + 1][c2 + 1] += x

    a = [[0] * m for _ in range(n)]
    for i in range(n):
        for j in range(m):
            up = a[i - 1][j] if i > 0 else 0
            left = a[i][j - 1] if j > 0 else 0
            diag = a[i - 1][j - 1] if i > 0 and j > 0 else 0
            a[i][j] = diff[i][j] + up + left - diag

    return a


# ============================================================================
# Unit Tests
# ============================================================================

class TestPrefixSumsAndDifferenceArrays(unittest.TestCase):
    def test_1d_prefix_sums(self):
        a = [2, 4, 1, 7, 3, 6]
        pref = prefix_sums(a)
        self.assertEqual(len(pref), 7)
        self.assertEqual(range_sum(pref, 0, 5), 23)
        self.assertEqual(range_sum(pref, 1, 4), 15)
        self.assertEqual(range_sum(pref, 2, 2), 1)
        self.assertEqual(range_sum(pref, 3, 2), 0)

        # Negative numbers
        neg = [3, -2, 5, -1]
        pref_neg = prefix_sums(neg)
        self.assertEqual(range_sum(pref_neg, 0, 3), 5)
        self.assertEqual(range_sum(pref_neg, 1, 2), 3)

        # Empty
        self.assertEqual(prefix_sums([]), [0])

    def test_2d_prefix_sums(self):
        grid = [
            [1, 2, 3],
            [4, 5, 6],
            [7, 8, 9]
        ]
        pref2d = prefix_sums_2d(grid)
        self.assertEqual(rectangle_sum(pref2d, 0, 0, 2, 2), 45)
        self.assertEqual(rectangle_sum(pref2d, 1, 1, 1, 1), 5)
        self.assertEqual(rectangle_sum(pref2d, 1, 0, 2, 1), 24)
        self.assertEqual(rectangle_sum(pref2d, 0, 0, 1, 1), 12)

    def test_1d_difference_array(self):
        n = 6
        updates = [
            (1, 3, 5),
            (2, 5, 2),
            (0, 2, 1)
        ]
        res = apply_range_additions(n, updates)
        self.assertEqual(res, [1, 6, 8, 7, 2, 2])

        # Roundtrip
        a = [3, 1, 4, 1, 5, 9]
        diff = build_difference_array(a)
        reconstructed = reconstruct_from_difference(diff)
        self.assertEqual(reconstructed, a)

    def test_2d_difference_array(self):
        n, m = 3, 3
        updates = [
            (0, 0, 1, 1, 3),
            (1, 1, 2, 2, 2)
        ]
        res = apply_rectangle_additions(n, m, updates)
        expected = [
            [3, 3, 0],
            [3, 5, 2],
            [0, 2, 2]
        ]
        self.assertEqual(res, expected)

    def test_combined_workflow(self):
        n = 5
        updates = [
            (0, 2, 10),
            (2, 4, 5)
        ]
        final_array = apply_range_additions(n, updates)
        self.assertEqual(final_array, [10, 10, 15, 5, 5])

        pref = prefix_sums(final_array)
        self.assertEqual(range_sum(pref, 1, 3), 30)
        self.assertEqual(range_sum(pref, 0, 4), 45)


if __name__ == "__main__":
    unittest.main()
