"""
Reference Implementation: Quickselect and Median of Medians
Demonstrates:
1. Lomuto Partition Scheme - O(n)
2. Randomized Quickselect (Iterative Lomuto) - Expected O(n), Worst O(n^2)
3. Randomized Quickselect with 3-Way Partitioning (Dutch National Flag) - Expected O(n)
4. Deterministic Median of Medians Selection (BFPRT) - Worst-case O(n)
5. Top-K Smallest Elements Selection - Expected O(n)
6. Max-Heap Selection Comparison - O(n log k)

Language: Python 3
"""

import random
import heapq
from typing import List
import unittest


# ============================================================================
# 1. Lomuto Partition
# ============================================================================

def lomuto_partition(a: List[int], left: int, right: int) -> int:
    """
    Standard Lomuto partition on subarray a[left..right].
    Uses a[right] as the pivot element.
    Returns the final position of the pivot.
    """
    pivot = a[right]
    i = left

    for j in range(left, right):
        if a[j] < pivot:
            a[i], a[j] = a[j], a[i]
            i += 1

    a[i], a[right] = a[right], a[i]
    return i


# ============================================================================
# 2. Randomized Quickselect (Iterative Lomuto)
# ============================================================================

def quickselect_lomuto(a: List[int], k: int) -> int:
    """
    Selects the k-th smallest element (0-based) using randomized Quickselect with Lomuto partitioning.
    Time Complexity: Expected O(n), Worst-case O(n^2).
    Space Complexity: O(1) auxiliary (operates on a shallow copy).
    """
    if k < 0 or k >= len(a):
        raise IndexError("k out of range")

    arr = a[:]
    left, right = 0, len(arr) - 1

    while left <= right:
        pivot_index = random.randint(left, right)
        arr[pivot_index], arr[right] = arr[right], arr[pivot_index]

        p = lomuto_partition(arr, left, right)

        if p == k:
            return arr[p]
        elif k < p:
            right = p - 1
        else:
            left = p + 1

    raise RuntimeError("unreachable")


# ============================================================================
# 3. Randomized Quickselect with 3-Way Partition (Dutch National Flag)
# ============================================================================

def _quickselect_three_way_inplace(a: List[int], k: int) -> int:
    """In-place 3-way partition helper."""
    if k < 0 or k >= len(a):
        raise IndexError("k out of range")

    left, right = 0, len(a) - 1

    while left <= right:
        pivot = a[random.randint(left, right)]

        lt = left
        i = left
        gt = right

        while i <= gt:
            if a[i] < pivot:
                a[lt], a[i] = a[i], a[lt]
                lt += 1
                i += 1
            elif a[i] > pivot:
                a[i], a[gt] = a[gt], a[i]
                gt -= 1
            else:
                i += 1

        if k < lt:
            right = lt - 1
        elif k > gt:
            left = gt + 1
        else:
            return a[k]

    raise RuntimeError("unreachable")


def quickselect_three_way(a: List[int], k: int) -> int:
    """
    Selects the k-th smallest element (0-based) using 3-way partitioning.
    Robust against arrays with heavy duplicates.
    Time Complexity: Expected O(n). Space Complexity: O(1) auxiliary.
    """
    return _quickselect_three_way_inplace(a[:], k)


# ============================================================================
# 4. Deterministic Median of Medians Selection (BFPRT)
# ============================================================================

def median_of_medians_select(a: List[int], k: int) -> int:
    """
    Selects the k-th smallest element (0-based) with guaranteed worst-case O(n) time.
    Uses groups of 5 and recursive median-of-medians pivot selection.
    Time Complexity: Worst-case O(n).
    Space Complexity: O(n) auxiliary.
    """
    if k < 0 or k >= len(a):
        raise IndexError("k out of range")

    def select(arr: List[int], rank: int) -> int:
        n = len(arr)
        if n <= 5:
            return sorted(arr)[rank]

        # Step 1: Divide into groups of 5 and take their medians
        groups = [arr[i:i+5] for i in range(0, n, 5)]
        medians = [sorted(group)[len(group) // 2] for group in groups]

        # Step 2: Recursively find median of medians
        pivot = select(medians, len(medians) // 2)

        # Step 3: Three-way partition around pivot value
        lows = [x for x in arr if x < pivot]
        highs = [x for x in arr if x > pivot]
        pivots = [x for x in arr if x == pivot]

        # Step 4: Recurse into the relevant subset
        if rank < len(lows):
            return select(lows, rank)
        elif rank < len(lows) + len(pivots):
            return pivot
        else:
            return select(highs, rank - len(lows) - len(pivots))

    return select(a[:], k)


# ============================================================================
# 5. Top-K Smallest Elements Selection
# ============================================================================

def top_k_smallest(a: List[int], k: int) -> List[int]:
    """
    Returns the k smallest elements from array a (in arbitrary order).
    Time Complexity: Expected O(n). Auxiliary Space: O(n).
    """
    if k <= 0:
        return []
    if k >= len(a):
        return a[:]

    arr = a[:]
    _quickselect_three_way_inplace(arr, k - 1)
    return arr[:k]


# ============================================================================
# 6. Max-Heap Selection Comparison (for small k or streaming)
# ============================================================================

def heap_select_kth_smallest(a: List[int], k: int) -> int:
    """
    Selects the k-th smallest element (0-based) using a max-heap of size k + 1.
    Time Complexity: O(n log k). Space Complexity: O(k).
    """
    if k < 0 or k >= len(a):
        raise IndexError("k out of range")

    # In Python, heapq is a min-heap; invert values to simulate max-heap
    max_heap: List[int] = []

    for x in a:
        heapq.heappush(max_heap, -x)
        if len(max_heap) > k + 1:
            heapq.heappop(max_heap)

    return -max_heap[0]


# ============================================================================
# Unit Tests
# ============================================================================

class TestSelection(unittest.TestCase):

    def test_out_of_bounds_and_single_element(self):
        with self.assertRaises(IndexError):
            quickselect_lomuto([], 0)
        with self.assertRaises(IndexError):
            quickselect_three_way([], 0)
        with self.assertRaises(IndexError):
            median_of_medians_select([], 0)
        with self.assertRaises(IndexError):
            heap_select_kth_smallest([], 0)

        single = [42]
        self.assertEqual(quickselect_lomuto(single, 0), 42)
        self.assertEqual(quickselect_three_way(single, 0), 42)
        self.assertEqual(median_of_medians_select(single, 0), 42)
        self.assertEqual(heap_select_kth_smallest(single, 0), 42)

    def test_arthur_worked_example(self):
        # [9, 1, 8, 2, 7, 3, 6], sorted: [1, 2, 3, 6, 7, 8, 9]
        a = [9, 1, 8, 2, 7, 3, 6]
        self.assertEqual(quickselect_lomuto(a, 0), 1)
        self.assertEqual(quickselect_lomuto(a, 3), 6)
        self.assertEqual(quickselect_lomuto(a, 4), 7)
        self.assertEqual(quickselect_lomuto(a, 6), 9)

        self.assertEqual(quickselect_three_way(a, 0), 1)
        self.assertEqual(quickselect_three_way(a, 3), 6)
        self.assertEqual(quickselect_three_way(a, 4), 7)
        self.assertEqual(quickselect_three_way(a, 6), 9)

        self.assertEqual(median_of_medians_select(a, 0), 1)
        self.assertEqual(median_of_medians_select(a, 3), 6)
        self.assertEqual(median_of_medians_select(a, 4), 7)
        self.assertEqual(median_of_medians_select(a, 6), 9)

        self.assertEqual(heap_select_kth_smallest(a, 4), 7)

    def test_massive_duplicates(self):
        all_same = [7] * 100
        for k in range(100):
            self.assertEqual(quickselect_three_way(all_same, k), 7)
            self.assertEqual(median_of_medians_select(all_same, k), 7)
            self.assertEqual(heap_select_kth_smallest(all_same, k), 7)

        repeated = [1] * 10 + [2] * 20 + [3] * 15
        self.assertEqual(quickselect_three_way(repeated, 5), 1)
        self.assertEqual(quickselect_three_way(repeated, 10), 2)
        self.assertEqual(quickselect_three_way(repeated, 25), 2)
        self.assertEqual(quickselect_three_way(repeated, 30), 3)
        self.assertEqual(quickselect_three_way(repeated, 44), 3)

        self.assertEqual(median_of_medians_select(repeated, 5), 1)
        self.assertEqual(median_of_medians_select(repeated, 10), 2)
        self.assertEqual(median_of_medians_select(repeated, 25), 2)
        self.assertEqual(median_of_medians_select(repeated, 30), 3)
        self.assertEqual(median_of_medians_select(repeated, 44), 3)

    def test_sorted_and_reversed(self):
        asc = list(range(1, 11))
        desc = list(range(10, 0, -1))

        for k in range(10):
            self.assertEqual(quickselect_lomuto(asc, k), k + 1)
            self.assertEqual(quickselect_lomuto(desc, k), k + 1)
            self.assertEqual(quickselect_three_way(asc, k), k + 1)
            self.assertEqual(quickselect_three_way(desc, k), k + 1)
            self.assertEqual(median_of_medians_select(asc, k), k + 1)
            self.assertEqual(median_of_medians_select(desc, k), k + 1)

    def test_cross_validation_random(self):
        sizes = [2, 3, 4, 5, 6, 7, 11, 15, 23, 50, 105]
        for n in sizes:
            arr = [random.randint(-500, 500) for _ in range(n)]
            sorted_arr = sorted(arr)
            test_ranks = [0, n // 4, n // 2, 3 * n // 4, n - 1]
            for k in test_ranks:
                expected = sorted_arr[k]
                self.assertEqual(quickselect_lomuto(arr, k), expected)
                self.assertEqual(quickselect_three_way(arr, k), expected)
                self.assertEqual(median_of_medians_select(arr, k), expected)
                self.assertEqual(heap_select_kth_smallest(arr, k), expected)

    def test_top_k_smallest(self):
        a = [14, 2, 8, 1, 9, 3, 7, 5]
        top4 = top_k_smallest(a, 4)
        self.assertEqual(sorted(top4), [1, 2, 3, 5])
        self.assertEqual(top_k_smallest(a, 0), [])
        self.assertEqual(len(top_k_smallest(a, 10)), 8)


if __name__ == "__main__":
    unittest.main()
