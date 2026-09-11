"""
Reference Implementation: Median Maintenance & Sliding Window Median
Demonstrates:
1. Dual-heap running median over an unbounded stream (O(log n) insert, O(1) query).
2. Sliding-window median with hash-map lazy deletion (O(log k) amortized per step).

Language: Python 3
"""

import heapq
from collections import defaultdict
from typing import Any, List, Optional
import unittest


class MedianMaintenance:
    """Tracks the running median of an unbounded stream using dual heaps."""

    def __init__(self):
        self.low: List[float] = []   # Max-heap (simulated by storing negated values)
        self.high: List[float] = []  # Min-heap

    def __len__(self) -> int:
        return len(self.low) + len(self.high)

    def is_empty(self) -> bool:
        return len(self) == 0

    def add(self, x: float) -> None:
        if not self.low or x <= -self.low[0]:
            heapq.heappush(self.low, -x)
        else:
            heapq.heappush(self.high, x)

        # Balance invariant: len(low) == len(high) or len(low) == len(high) + 1
        if len(self.low) > len(self.high) + 1:
            heapq.heappush(self.high, -heapq.heappop(self.low))
        elif len(self.high) > len(self.low):
            heapq.heappush(self.low, -heapq.heappop(self.high))

    def lower_median(self) -> float:
        if self.is_empty():
            raise IndexError("MedianMaintenance.lower_median(): stream is empty")
        return -self.low[0]

    def upper_median(self) -> float:
        if self.is_empty():
            raise IndexError("MedianMaintenance.upper_median(): stream is empty")
        if len(self.low) > len(self.high):
            return -self.low[0]
        return self.high[0]

    def find_median(self) -> float:
        if self.is_empty():
            raise IndexError("MedianMaintenance.find_median(): stream is empty")
        if len(self.low) > len(self.high):
            return float(-self.low[0])
        return (-self.low[0] + self.high[0]) / 2.0


class SlidingWindowMedian:
    """Tracks the median of a sliding window of size k using dual heaps and lazy deletion."""

    def __init__(self):
        self.low: List[float] = []   # Max-heap (negated)
        self.high: List[float] = []  # Min-heap
        self.delayed = defaultdict(int)
        self.low_valid = 0
        self.high_valid = 0

    def _prune(self, heap: List[float], is_max: bool) -> None:
        while heap:
            val = -heap[0] if is_max else heap[0]
            if self.delayed[val] > 0:
                self.delayed[val] -= 1
                heapq.heappop(heap)
            else:
                break

    def _balance(self) -> None:
        if self.low_valid > self.high_valid + 1:
            val = -heapq.heappop(self.low)
            heapq.heappush(self.high, val)
            self.low_valid -= 1
            self.high_valid += 1
            self._prune(self.low, True)
        elif self.high_valid > self.low_valid:
            val = heapq.heappop(self.high)
            heapq.heappush(self.low, -val)
            self.high_valid -= 1
            self.low_valid += 1
            self._prune(self.high, False)

    def add(self, x: float) -> None:
        if not self.low or x <= -self.low[0]:
            heapq.heappush(self.low, -x)
            self.low_valid += 1
        else:
            heapq.heappush(self.high, x)
            self.high_valid += 1
        self._balance()

    def remove(self, x: float) -> None:
        self.delayed[x] += 1
        if self.low and x <= -self.low[0]:
            self.low_valid -= 1
            if x == -self.low[0]:
                self._prune(self.low, True)
        else:
            self.high_valid -= 1
            if self.high and x == self.high[0]:
                self._prune(self.high, False)
        self._balance()

    def find_median(self) -> float:
        self._prune(self.low, True)
        self._prune(self.high, False)
        if self.low_valid > self.high_valid:
            return float(-self.low[0])
        return (-self.low[0] + self.high[0]) / 2.0


class TestMedianMaintenance(unittest.TestCase):
    def test_running_median(self):
        mm = MedianMaintenance()
        self.assertTrue(mm.is_empty())
        self.assertEqual(len(mm), 0)

        # 5, 2, 10, 4, 8
        mm.add(5)
        self.assertEqual(mm.find_median(), 5.0)
        self.assertEqual(mm.lower_median(), 5.0)

        mm.add(2)
        self.assertEqual(mm.find_median(), 3.5)
        self.assertEqual(mm.lower_median(), 2.0)
        self.assertEqual(mm.upper_median(), 5.0)

        mm.add(10)
        self.assertEqual(mm.find_median(), 5.0)

        mm.add(4)
        self.assertEqual(mm.find_median(), 4.5)
        self.assertEqual(mm.lower_median(), 4.0)

        mm.add(8)
        self.assertEqual(mm.find_median(), 5.0)

    def test_stress_against_sort(self):
        stream = [15, -3, 42, 7, 0, -100, 88, 23, 14, 5, 9, 11, 2]
        mm = MedianMaintenance()
        buffer = []

        for val in stream:
            mm.add(val)
            buffer.append(val)
            s = sorted(buffer)
            n = len(s)
            if n % 2 == 1:
                expected = float(s[n // 2])
            else:
                expected = (s[n // 2 - 1] + s[n // 2]) / 2.0
            self.assertAlmostEqual(mm.find_median(), expected)

    def test_sliding_window_median(self):
        nums = [1, 3, -1, -3, 5, 3, 6, 7]
        k = 3
        swm = SlidingWindowMedian()
        results = []

        for i, val in enumerate(nums):
            swm.add(val)
            if i >= k:
                swm.remove(nums[i - k])
            if i >= k - 1:
                results.append(swm.find_median())

        expected = [1.0, -1.0, -1.0, 3.0, 5.0, 6.0]
        self.assertEqual(results, expected)

    def test_empty_exceptions(self):
        mm = MedianMaintenance()
        with self.assertRaises(IndexError):
            mm.find_median()
        with self.assertRaises(IndexError):
            mm.lower_median()


if __name__ == "__main__":
    unittest.main()
