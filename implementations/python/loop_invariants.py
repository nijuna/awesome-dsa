"""
Formal Loop Invariants and Program Verification Engine.

Implements Insertion Sort, Binary Search, and Two Pointers Container With Most Water
instrumented with runtime invariant checking at initialization, maintenance, and termination.
"""

import unittest
from typing import List


def insertion_sort_with_invariant(arr: List[int]) -> None:
    """Insertion sort instrumented with prefix-sorted invariant check."""

    def is_prefix_sorted(length: int) -> bool:
        return all(arr[k - 1] <= arr[k] for k in range(1, length))

    if len(arr) <= 1:
        return

    # Initialization
    assert is_prefix_sorted(1)

    for i in range(1, len(arr)):
        key = arr[i]
        j = i - 1
        while j >= 0 and arr[j] > key:
            arr[j + 1] = arr[j]
            j -= 1
        arr[j + 1] = key

        # Maintenance
        assert is_prefix_sorted(i + 1)

    # Termination
    assert is_prefix_sorted(len(arr))


def binary_search_with_variant(arr: List[int], target: int) -> int:
    """Binary search with invariant and decreasing loop variant."""
    low = 0
    high = len(arr) - 1

    def check_invariant(l: int, h: int):
        for idx, val in enumerate(arr):
            if val == target:
                assert l <= idx <= h

    check_invariant(low, high)
    prev_variant = high - low + 1

    while low <= high:
        curr_variant = high - low + 1
        assert curr_variant <= prev_variant
        prev_variant = curr_variant

        mid = (low + high) // 2
        if arr[mid] == target:
            return mid
        elif arr[mid] < target:
            low = mid + 1
        else:
            high = mid - 1
        check_invariant(low, high)

    assert target not in arr
    return -1


def max_area_with_invariant(heights: List[int]) -> int:
    """Container With Most Water with two-pointer elimination invariant."""
    if len(heights) < 2:
        return 0

    l = 0
    r = len(heights) - 1
    max_water = 0

    while l < r:
        width = r - l
        h = min(heights[l], heights[r])
        max_water = max(max_water, width * h)

        if heights[l] < heights[r]:
            for k in range(l + 1, r):
                test_area = (k - l) * min(heights[l], heights[k])
                assert test_area <= max_water
            l += 1
        else:
            for k in range(l, r):
                test_area = (r - k) * min(heights[k], heights[r])
                assert test_area <= max_water
            r -= 1

    return max_water


class TestLoopInvariants(unittest.TestCase):
    def test_insertion_sort(self):
        arr = [5, 2, 9, 1, 5, 6]
        insertion_sort_with_invariant(arr)
        self.assertEqual(arr, sorted([5, 2, 9, 1, 5, 6]))

    def test_binary_search(self):
        arr = [10, 20, 30, 40, 50, 60, 70]
        self.assertEqual(binary_search_with_variant(arr, 40), 3)
        self.assertEqual(binary_search_with_variant(arr, 10), 0)
        self.assertEqual(binary_search_with_variant(arr, 70), 6)
        self.assertEqual(binary_search_with_variant(arr, 35), -1)

    def test_container_with_most_water(self):
        heights = [1, 8, 6, 2, 5, 4, 8, 3, 7]
        self.assertEqual(max_area_with_invariant(heights), 49)


if __name__ == "__main__":
    unittest.main()
