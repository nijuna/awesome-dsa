"""
Reference Implementation: Binary Search on Answer
Demonstrates:
1. Generic First True and Last True integer search templates.
2. Continuous real-valued binary search with fixed-iteration convergence.
3. Capacity Allocation: Split Array Largest Sum / Ship Packages in D Days.
4. Rate & Speed: Koko Eating Bananas with ceiling arithmetic.
5. Maximize Minimum Distance: Aggressive Cows / Stalls placement.
6. Value-Space Rank Search: K-th smallest element in a sorted matrix.

Language: Python 3
"""

import unittest
import math
from typing import List, Callable


# ============================================================================
# 1. Generic Binary Search Templates
# ============================================================================

def first_true(low: int, high: int, feasible: Callable[[int], bool]) -> int:
    """
    Searches for the first x in [low, high] where feasible(x) is True.
    Assumes monotonic pattern: F F F ... T T T.
    Invariant: answer always lies in [low, high].
    """
    while low < high:
        mid = low + (high - low) // 2
        if feasible(mid):
            high = mid
        else:
            low = mid + 1
    return low


def last_true(low: int, high: int, feasible: Callable[[int], bool]) -> int:
    """
    Searches for the last x in [low, high] where feasible(x) is True.
    Assumes monotonic pattern: T T T ... F F F.
    Invariant: answer always lies in [low, high].
    """
    while low < high:
        mid = low + (high - low + 1) // 2
        if feasible(mid):
            low = mid
        else:
            high = mid - 1
    return low


def binary_search_real(
    low: float, high: float, feasible: Callable[[float], bool], iterations: int = 80
) -> float:
    """
    Continuous real-valued binary search over [low, high] using fixed iterations.
    Guarantees precision to (high - low) / 2^iterations.
    """
    for _ in range(iterations):
        mid = (low + high) / 2.0
        if feasible(mid):
            high = mid
        else:
            low = mid
    return high


# ============================================================================
# 2. Capacity Allocation: Split Array Largest Sum
# ============================================================================

def split_array_largest_sum(a: List[int], k: int) -> int:
    """
    Split Array Largest Sum:
    Minimizes the largest subarray sum when splitting into at most k contiguous parts.
    Time: O(n * log(sum - max)), Space: O(1).
    """
    if not a:
        return 0

    def feasible(cap: int) -> bool:
        parts = 1
        cur = 0
        for x in a:
            if cur + x <= cap:
                cur += x
            else:
                parts += 1
                cur = x
        return parts <= k

    low = max(a)
    high = sum(a)

    return first_true(low, high, feasible)


# ============================================================================
# 3. Rate & Speed: Koko Eating Bananas
# ============================================================================

def min_eating_speed(piles: List[int], h: int) -> int:
    """
    Koko Eating Bananas:
    Finds minimum integer eating speed k to consume all piles within h hours.
    Uses ceiling division: ceil(p / k) = (p + k - 1) // k.
    Time: O(n * log(max(piles))), Space: O(1).
    """
    if not piles:
        return 0

    def feasible(k: int) -> bool:
        if k <= 0:
            return False
        hours = 0
        for p in piles:
            hours += (p + k - 1) // k
        return hours <= h

    low = 1
    high = max(piles)

    return first_true(low, high, feasible)


# ============================================================================
# 4. Maximize Minimum Distance: Aggressive Cows
# ============================================================================

def aggressive_cows(pos: List[int], k: int) -> int:
    """
    Aggressive Cows:
    Places k cows in sorted stall positions maximizing the minimum distance between any two cows.
    Time: O(n log n + n * log(range)), Space: O(1).
    """
    if len(pos) < k:
        return 0
    pos = sorted(pos)

    def feasible(dist: int) -> bool:
        used = 1
        last = pos[0]
        for x in pos[1:]:
            if x - last >= dist:
                used += 1
                last = x
        return used >= k

    low = 0
    high = pos[-1] - pos[0]

    return last_true(low, high, feasible)


# ============================================================================
# 5. Value-Space Rank Search: K-th Smallest in Sorted Matrix
# ============================================================================

def kth_smallest_sorted_matrix(matrix: List[List[int]], k: int) -> int:
    """
    K-th Smallest Element in a Matrix where each row and column is sorted ascending.
    Binary searches over the value range [matrix[0][0], matrix[n-1][n-1]].
    Counts elements <= x in O(n) using staircase walk.
    Time: O(n * log(max_val - min_val)), Space: O(1).
    """
    n = len(matrix)
    if n == 0:
        return 0

    def count_less_or_equal(target: int) -> int:
        count = 0
        row = n - 1
        col = 0
        while row >= 0 and col < n:
            if matrix[row][col] <= target:
                count += (row + 1)
                col += 1
            else:
                row -= 1
        return count

    def feasible(val: int) -> bool:
        return count_less_or_equal(val) >= k

    low = matrix[0][0]
    high = matrix[n - 1][n - 1]

    return first_true(low, high, feasible)


# ============================================================================
# Unit Tests
# ============================================================================

class TestBinarySearchOnAnswer(unittest.TestCase):
    def test_templates(self):
        self.assertEqual(first_true(0, 20, lambda x: x >= 7), 7)
        self.assertEqual(last_true(0, 20, lambda x: x <= 14), 14)
        self.assertEqual(first_true(5, 5, lambda x: True), 5)
        self.assertEqual(last_true(5, 5, lambda x: True), 5)

    def test_split_array(self):
        nums = [7, 2, 5, 10, 8]
        self.assertEqual(split_array_largest_sum(nums, 2), 18)
        self.assertEqual(split_array_largest_sum(nums, 1), 32)
        self.assertEqual(split_array_largest_sum(nums, 5), 10)
        self.assertEqual(split_array_largest_sum([], 1), 0)

    def test_koko_eating_bananas(self):
        self.assertEqual(min_eating_speed([3, 6, 7, 11], 8), 4)
        self.assertEqual(min_eating_speed([30, 11, 23, 4, 20], 5), 30)
        self.assertEqual(min_eating_speed([30, 11, 23, 4, 20], 6), 23)
        self.assertEqual(min_eating_speed([], 5), 0)

    def test_aggressive_cows(self):
        pos = [1, 2, 8, 4, 9]
        self.assertEqual(aggressive_cows(pos, 3), 3)
        self.assertEqual(aggressive_cows([1, 100], 2), 99)
        self.assertEqual(aggressive_cows([1], 2), 0)

    def test_kth_smallest_matrix(self):
        mat = [
            [1, 5, 9],
            [10, 11, 13],
            [12, 13, 15]
        ]
        self.assertEqual(kth_smallest_sorted_matrix(mat, 8), 13)
        self.assertEqual(kth_smallest_sorted_matrix(mat, 1), 1)
        self.assertEqual(kth_smallest_sorted_matrix(mat, 9), 15)

    def test_continuous_binary_search(self):
        root2 = binary_search_real(1.0, 2.0, lambda x: x * x >= 2.0, 60)
        self.assertAlmostEqual(root2, math.sqrt(2.0), places=7)


if __name__ == '__main__':
    unittest.main()
