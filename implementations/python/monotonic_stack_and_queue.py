"""
Reference Implementation: Monotonic Stack and Queue
Demonstrates:
1. Next Greater Element (NGE) and Daily Temperatures (decreasing stack of indices).
2. Stock Span Problem (previous greater element barrier).
3. Largest Rectangle in Histogram (previous and next smaller element boundaries).
4. Trapping Rain Water (horizontal basin accumulation via monotonic stack).
5. Next Greater Element II on Circular Arrays (2n virtual scan).
6. Sliding Window Maximum and Minimum (monotonic deque with expiration).

Language: Python 3
"""

import unittest
from collections import deque
from typing import List


# ============================================================================
# 1. Next Greater Element and Daily Temperatures
# ============================================================================

def next_greater_indices(a: List[int]) -> List[int]:
    """
    Computes the index of the next strictly greater element for each position.
    Returns -1 if no greater element exists to the right.
    Time: O(n) amortized, Space: O(n).
    """
    n = len(a)
    ans = [-1] * n
    st = []  # stores indices with decreasing values

    for i in range(n):
        while st and a[i] > a[st[-1]]:
            ans[st.pop()] = i
        st.append(i)

    return ans


def daily_temperatures(temp: List[int]) -> List[int]:
    """
    Daily Temperatures: Number of days to wait until a warmer temperature.
    Returns 0 if no warmer day appears in the future.
    Time: O(n) amortized, Space: O(n).
    """
    n = len(temp)
    ans = [0] * n
    st = []

    for i in range(n):
        while st and temp[i] > temp[st[-1]]:
            j = st.pop()
            ans[j] = i - j
        st.append(i)

    return ans


# ============================================================================
# 2. Stock Span Problem
# ============================================================================

def stock_span(price: List[int]) -> List[int]:
    """
    Stock Span: Consecutive days ending at day i with price <= price[i].
    Time: O(n) amortized, Space: O(n).
    """
    n = len(price)
    span = [0] * n
    st = []  # indices of strictly greater price barriers

    for i in range(n):
        while st and price[st[-1]] <= price[i]:
            st.pop()

        prev_greater = st[-1] if st else -1
        span[i] = i - prev_greater
        st.append(i)

    return span


# ============================================================================
# 3. Largest Rectangle in Histogram
# ============================================================================

def largest_rectangle_histogram(h: List[int]) -> int:
    """
    Largest Rectangle in Histogram:
    Finds maximum rectangular area using previous and next smaller boundaries.
    Width = right[i] - left[i] - 1.
    Time: O(n) amortized, Space: O(n).
    """
    n = len(h)
    if n == 0:
        return 0

    left = [-1] * n
    right = [n] * n
    st = []

    for i in range(n):
        while st and h[st[-1]] >= h[i]:
            st.pop()
        left[i] = st[-1] if st else -1
        st.append(i)

    st.clear()

    for i in range(n - 1, -1, -1):
        while st and h[st[-1]] >= h[i]:
            st.pop()
        right[i] = st[-1] if st else n
        st.append(i)

    max_area = 0
    for i in range(n):
        width = right[i] - left[i] - 1
        area = h[i] * width
        if area > max_area:
            max_area = area

    return max_area


# ============================================================================
# 4. Trapping Rain Water (Monotonic Stack Method)
# ============================================================================

def trap_rain_water_stack(h: List[int]) -> int:
    """
    Trapping Rain Water:
    Accumulates water volume by discovering horizontal basins bounded by taller walls.
    Time: O(n) amortized, Space: O(n).
    """
    n = len(h)
    water = 0
    st = []

    for i in range(n):
        while st and h[i] > h[st[-1]]:
            mid = st.pop()
            if not st:
                break

            left = st[-1]
            width = i - left - 1
            bounded_height = min(h[left], h[i]) - h[mid]
            water += width * bounded_height

        st.append(i)

    return water


# ============================================================================
# 5. Circular Array Next Greater Element (NGE II)
# ============================================================================

def next_greater_circular(a: List[int]) -> List[int]:
    """
    Next Greater Element on a Circular Array:
    Scans 2n virtual positions; only pushes unresolved indices from the first pass.
    Returns values of next greater elements (-1 if none exists).
    Time: O(n) amortized, Space: O(n).
    """
    n = len(a)
    if n == 0:
        return []

    ans = [-1] * n
    st = []

    for i in range(2 * n):
        idx = i % n

        while st and a[idx] > a[st[-1]]:
            ans[st.pop()] = a[idx]

        if i < n:
            st.append(idx)

    return ans


# ============================================================================
# 6. Sliding Window Maximum and Minimum (Monotonic Deque)
# ============================================================================

def sliding_window_maximum(a: List[int], k: int) -> List[int]:
    """
    Sliding Window Maximum:
    Uses a monotonic deque maintaining indices of decreasing values.
    Front is always the maximum of the current window of size k.
    Time: O(n) amortized, Space: O(k).
    """
    n = len(a)
    if n == 0 or k <= 0:
        return []
    k = min(k, n)

    dq = deque()
    ans = []

    for i in range(n):
        while dq and dq[0] <= i - k:
            dq.popleft()

        while dq and a[dq[-1]] <= a[i]:
            dq.pop()

        dq.append(i)

        if i >= k - 1:
            ans.append(a[dq[0]])

    return ans


def sliding_window_minimum(a: List[int], k: int) -> List[int]:
    """
    Sliding Window Minimum:
    Uses a monotonic deque maintaining indices of increasing values.
    Front is always the minimum of the current window of size k.
    Time: O(n) amortized, Space: O(k).
    """
    n = len(a)
    if n == 0 or k <= 0:
        return []
    k = min(k, n)

    dq = deque()
    ans = []

    for i in range(n):
        while dq and dq[0] <= i - k:
            dq.popleft()

        while dq and a[dq[-1]] >= a[i]:
            dq.pop()

        dq.append(i)

        if i >= k - 1:
            ans.append(a[dq[0]])

    return ans


# ============================================================================
# Unit Tests
# ============================================================================

class TestMonotonicStackAndQueue(unittest.TestCase):
    def test_next_greater_and_daily_temp(self):
        a = [2, 1, 2, 4, 3]
        self.assertEqual(next_greater_indices(a), [3, 2, 3, -1, -1])

        temp = [73, 74, 75, 71, 69, 72, 76, 73]
        self.assertEqual(daily_temperatures(temp), [1, 1, 4, 2, 1, 1, 0, 0])

    def test_stock_span(self):
        prices = [100, 80, 60, 70, 60, 75, 85]
        self.assertEqual(stock_span(prices), [1, 1, 1, 2, 1, 4, 6])

    def test_largest_rectangle_histogram(self):
        self.assertEqual(largest_rectangle_histogram([2, 1, 5, 6, 2, 3]), 10)
        self.assertEqual(largest_rectangle_histogram([2, 2, 2, 2]), 8)
        self.assertEqual(largest_rectangle_histogram([1, 2, 3, 4]), 6)
        self.assertEqual(largest_rectangle_histogram([]), 0)

    def test_trap_rain_water_stack(self):
        h = [0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1]
        self.assertEqual(trap_rain_water_stack(h), 6)
        self.assertEqual(trap_rain_water_stack([3, 2, 1]), 0)
        self.assertEqual(trap_rain_water_stack([]), 0)

    def test_circular_next_greater(self):
        self.assertEqual(next_greater_circular([1, 2, 1]), [2, -1, 2])
        self.assertEqual(next_greater_circular([1, 2, 3, 4, 3]), [2, 3, 4, -1, 4])
        self.assertEqual(next_greater_circular([]), [])

    def test_sliding_window_extrema(self):
        a = [1, 3, -1, -3, 5, 3, 6, 7]
        k = 3
        self.assertEqual(sliding_window_maximum(a, k), [3, 3, 5, 5, 6, 7])
        self.assertEqual(sliding_window_minimum(a, k), [-1, -3, -3, -3, 3, 3])
        self.assertEqual(sliding_window_maximum(a, 1), a)
        self.assertEqual(sliding_window_minimum(a, 1), a)
        self.assertEqual(sliding_window_maximum([], 3), [])


if __name__ == '__main__':
    unittest.main()
