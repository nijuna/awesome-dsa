"""
Reference Implementation: Divide and Conquer Paradigm Mechanics
Demonstrates:
1. Merge Sort (Recursive Half-Splitting & Stable Buffer Merging, O(N log N)).
2. Inversion Counting via Augmented Merge Sort (O(N log N)).
3. Binary Search (Degenerate Divide-and-Conquer, O(log N)).
4. Maximum Subarray via Divide-and-Conquer (O(N log N)) vs. Kadane's Algorithm (O(N)).
5. QuickSelect (Single-Branch Selection for k-th Smallest Element, O(N) Average).
6. Karatsuba Fast Integer Multiplication (Recurrence T(N) = 3 T(N/2) + O(N) => O(N^1.585)).

Language: Python 3
"""

from typing import List
import unittest


def merge_sort(a: List[int]) -> List[int]:
    """Sorts an array using Divide-and-Conquer Merge Sort."""
    arr = a[:]
    temp = [0] * len(arr)

    def solve(left: int, right: int):
        if right - left <= 1:
            return

        mid = left + (right - left) // 2
        solve(left, mid)
        solve(mid, right)

        i, j, k = left, mid, left
        while i < mid and j < right:
            if arr[i] <= arr[j]:
                temp[k] = arr[i]
                i += 1
            else:
                temp[k] = arr[j]
                j += 1
            k += 1

        while i < mid:
            temp[k] = arr[i]
            i += 1
            k += 1

        while j < right:
            temp[k] = arr[j]
            j += 1
            k += 1

        for p in range(left, right):
            arr[p] = temp[p]

    solve(0, len(arr))
    return arr


def count_inversions(a: List[int]) -> int:
    """Counts the number of inverted pairs (i < j and a[i] > a[j]) in O(N log N)."""
    arr = a[:]
    temp = [0] * len(arr)

    def solve(left: int, right: int) -> int:
        if right - left <= 1:
            return 0

        mid = left + (right - left) // 2
        inv = solve(left, mid) + solve(mid, right)

        i, j, k = left, mid, left
        while i < mid and j < right:
            if arr[i] <= arr[j]:
                temp[k] = arr[i]
                i += 1
            else:
                temp[k] = arr[j]
                j += 1
                inv += mid - i  # Elements remaining in left half are strictly greater
            k += 1

        while i < mid:
            temp[k] = arr[i]
            i += 1
            k += 1

        while j < right:
            temp[k] = arr[j]
            j += 1
            k += 1

        for p in range(left, right):
            arr[p] = temp[p]

        return inv

    return solve(0, len(arr))


def binary_search_index(a: List[int], target: int) -> int:
    """Finds index of target in sorted array a, or returns -1."""
    low, high = 0, len(a) - 1

    while low <= high:
        mid = low + (high - low) // 2
        if a[mid] == target:
            return mid
        elif a[mid] < target:
            low = mid + 1
        else:
            high = mid - 1

    return -1


def maximum_subarray_dc(a: List[int]) -> int:
    """Finds the maximum contiguous subarray sum in O(N log N)."""
    if not a:
        return 0

    def solve(left: int, right: int) -> int:
        if right - left == 1:
            return a[left]

        mid = left + (right - left) // 2
        left_best = solve(left, mid)
        right_best = solve(mid, right)

        best_left_suffix = -(10**18)
        s = 0
        for i in range(mid - 1, left - 1, -1):
            s += a[i]
            best_left_suffix = max(best_left_suffix, s)

        best_right_prefix = -(10**18)
        s = 0
        for i in range(mid, right):
            s += a[i]
            best_right_prefix = max(best_right_prefix, s)

        cross = best_left_suffix + best_right_prefix
        return max(left_best, right_best, cross)

    return solve(0, len(a))


def maximum_subarray_kadane(a: List[int]) -> int:
    """Computes maximum contiguous subarray sum in O(N) time."""
    if not a:
        return 0
    max_so_far = a[0]
    curr_max = a[0]
    for x in a[1:]:
        curr_max = max(x, curr_max + x)
        max_so_far = max(max_so_far, curr_max)
    return max_so_far


def quickselect(a: List[int], k: int) -> int:
    """Finds the k-th smallest element (0-indexed) in O(N) average time."""
    if k < 0 or k >= len(a):
        raise IndexError("k out of range")

    arr = a[:]
    left, right = 0, len(arr) - 1

    while True:
        pivot = arr[right]
        i = left

        for j in range(left, right):
            if arr[j] <= pivot:
                arr[i], arr[j] = arr[j], arr[i]
                i += 1

        arr[i], arr[right] = arr[right], arr[i]

        if i == k:
            return arr[i]
        elif k < i:
            right = i - 1
        else:
            left = i + 1


def karatsuba(x: int, y: int) -> int:
    """Multiplies two non-negative integers using Karatsuba's algorithm."""
    if x < 10 or y < 10:
        return x * y

    n = max(len(str(x)), len(str(y)))
    m = n // 2

    # Split: x = a * 10^m + b, y = c * 10^m + d
    power = 10**m
    a = x // power
    b = x % power
    c = y // power
    d = y % power

    z2 = karatsuba(a, c)
    z0 = karatsuba(b, d)
    z1 = karatsuba(a + b, c + d) - z2 - z0

    return z2 * (10 ** (2 * m)) + z1 * power + z0


class TestDivideAndConquer(unittest.TestCase):
    def test_merge_sort(self):
        a = [38, 27, 43, 3, 9, 82, 10]
        self.assertEqual(merge_sort(a), [3, 9, 10, 27, 38, 43, 82])

    def test_inversion_counting(self):
        self.assertEqual(count_inversions([2, 4, 1, 3, 5]), 3)
        self.assertEqual(count_inversions([5, 4, 3, 2, 1]), 10)
        self.assertEqual(count_inversions([1, 2, 3, 4, 5]), 0)

    def test_binary_search(self):
        a = [-10, -3, 0, 5, 9, 12, 18, 42]
        self.assertEqual(binary_search_index(a, 9), 4)
        self.assertEqual(binary_search_index(a, -10), 0)
        self.assertEqual(binary_search_index(a, 42), 7)
        self.assertEqual(binary_search_index(a, 99), -1)

    def test_maximum_subarray(self):
        a = [-2, 1, -3, 4, -1, 2, 1, -5, 4]
        self.assertEqual(maximum_subarray_dc(a), 6)
        self.assertEqual(maximum_subarray_dc(a), maximum_subarray_kadane(a))

        neg = [-5, -2, -8, -1, -4]
        self.assertEqual(maximum_subarray_dc(neg), -1)
        self.assertEqual(maximum_subarray_dc(neg), maximum_subarray_kadane(neg))

    def test_quickselect(self):
        a = [7, 10, 4, 3, 20, 15]
        self.assertEqual(quickselect(a, 0), 3)
        self.assertEqual(quickselect(a, 2), 7)
        self.assertEqual(quickselect(a, 5), 20)

    def test_karatsuba(self):
        x = 123456789
        y = 987654321
        self.assertEqual(karatsuba(x, y), x * y)


if __name__ == "__main__":
    unittest.main()
