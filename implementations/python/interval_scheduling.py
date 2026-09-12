"""
Reference Implementation: Interval Scheduling & Partitioning Patterns
Demonstrates:
1. Unweighted Interval Scheduling (Earliest Finish Time Greedy) - O(n log n)
2. Schedule Reconstruction for Unweighted Scheduling - O(n log n)
3. Meeting Rooms II / Interval Partitioning (Min-Heap) - O(n log n)
4. Meeting Rooms II / Interval Partitioning (Sweep-Line Event Delta) - O(n log n)
5. Weighted Interval Scheduling (DP + Binary Search) - O(n log n)
6. Optimal Subset Reconstruction for Weighted Scheduling - O(n log n)
7. Merge Overlapping Intervals - O(n log n)
8. Insert Interval into Sorted Disjoint Intervals - O(n)

Language: Python 3
"""

import heapq
from bisect import bisect_right
from typing import List, Tuple
import unittest


# ============================================================================
# 1. Unweighted Interval Scheduling (Max Disjoint Intervals)
# ============================================================================

def max_non_overlapping_intervals(intervals: List[Tuple[int, int]]) -> int:
    """
    Computes the maximum number of mutually non-overlapping intervals.
    Greedy criterion: Earliest Finish Time.
    Convention: half-open [s, e), so interval b can start when a finishes (b.start >= a.end).
    Time Complexity: O(n log n). Space Complexity: O(1) auxiliary (or O(n) for sort).
    """
    if not intervals:
        return 0

    sorted_intervals = sorted(intervals, key=lambda x: (x[1], x[0]))
    count = 0
    last_end = -float("inf")

    for s, e in sorted_intervals:
        if s >= last_end:
            count += 1
            last_end = e

    return count


def reconstruct_non_overlapping_schedule(
    intervals: List[Tuple[int, int]]
) -> List[Tuple[int, int]]:
    """
    Reconstructs the actual optimal subset of mutually non-overlapping intervals.
    Time Complexity: O(n log n). Space Complexity: O(n).
    """
    if not intervals:
        return []

    sorted_intervals = sorted(intervals, key=lambda x: (x[1], x[0]))
    chosen: List[Tuple[int, int]] = []
    last_end = -float("inf")

    for s, e in sorted_intervals:
        if s >= last_end:
            chosen.append((s, e))
            last_end = e

    return chosen


# ============================================================================
# 2. Meeting Rooms II / Interval Partitioning
# ============================================================================

def min_meeting_rooms_heap(intervals: List[Tuple[int, int]]) -> int:
    """
    Calculates the minimum number of resources (meeting rooms) required.
    Approach A: Min-Heap of room end-times.
    Sorts by start time. Rooms freeing on or before current start time are reused.
    Time Complexity: O(n log n). Space Complexity: O(n).
    """
    if not intervals:
        return 0

    sorted_intervals = sorted(intervals, key=lambda x: (x[0], x[1]))
    pq: List[int] = []
    peak_rooms = 0

    for s, e in sorted_intervals:
        while pq and pq[0] <= s:
            heapq.heappop(pq)
        heapq.heappush(pq, e)
        peak_rooms = max(peak_rooms, len(pq))

    return peak_rooms


def min_meeting_rooms_sweepline(intervals: List[Tuple[int, int]]) -> int:
    """
    Calculates the minimum number of meeting rooms using a Sweep-Line algorithm.
    Approach B: Event delta tracking (+1 at start, -1 at end).
    Tie-breaking: In half-open [s, e), an end at time t frees a room before a new
    meeting at time t begins. Thus -1 is sorted before +1 at the same coordinate.
    Time Complexity: O(n log n). Space Complexity: O(n).
    """
    if not intervals:
        return 0

    events: List[Tuple[int, int]] = []
    for s, e in intervals:
        events.append((s, +1))
        events.append((e, -1))

    # Sort primarily by timestamp ascending.
    # Secondarily by delta ascending: -1 comes before +1 on collision.
    events.sort(key=lambda x: (x[0], x[1]))

    active = 0
    peak = 0
    for _time, delta in events:
        active += delta
        peak = max(peak, active)

    return peak


# ============================================================================
# 3. Weighted Interval Scheduling (DP + Binary Search)
# ============================================================================

def weighted_interval_scheduling(intervals: List[Tuple[int, int, int]]) -> int:
    """
    Computes the maximum total weight achievable by a set of mutually disjoint intervals.
    Each interval is (start, end, weight).
    Uses DP with binary search for the latest compatible predecessor p(i).
    dp[i] = max(dp[i - 1], weight[i] + dp[p(i)])
    Time Complexity: O(n log n). Space Complexity: O(n).
    """
    if not intervals:
        return 0

    # Sort primarily by finish time
    sorted_intervals = sorted(intervals, key=lambda x: (x[1], x[0]))
    n = len(sorted_intervals)

    ends = [0]
    starts = [0]
    weights = [0]

    for s, e, w in sorted_intervals:
        starts.append(s)
        ends.append(e)
        weights.append(w)

    # p[i] = largest index j < i with ends[j] <= starts[i]
    p = [0] * (n + 1)
    for i in range(1, n + 1):
        p[i] = bisect_right(ends, starts[i], 0, i) - 1

    dp = [0] * (n + 1)
    for i in range(1, n + 1):
        dp[i] = max(dp[i - 1], weights[i] + dp[p[i]])

    return dp[n]


def weighted_interval_scheduling_reconstruct(
    intervals: List[Tuple[int, int, int]]
) -> Tuple[int, List[Tuple[int, int, int]]]:
    """
    Computes the maximum total weight and reconstructs the optimal subset of intervals.
    Time Complexity: O(n log n). Space Complexity: O(n).
    """
    if not intervals:
        return 0, []

    sorted_intervals = sorted(intervals, key=lambda x: (x[1], x[0]))
    n = len(sorted_intervals)

    ends = [0]
    starts = [0]
    weights = [0]
    items: List[Tuple[int, int, int]] = [(-1, -1, 0)]

    for s, e, w in sorted_intervals:
        starts.append(s)
        ends.append(e)
        weights.append(w)
        items.append((s, e, w))

    p = [0] * (n + 1)
    for i in range(1, n + 1):
        p[i] = bisect_right(ends, starts[i], 0, i) - 1

    dp = [0] * (n + 1)
    for i in range(1, n + 1):
        dp[i] = max(dp[i - 1], weights[i] + dp[p[i]])

    chosen: List[Tuple[int, int, int]] = []
    curr = n
    while curr > 0:
        if dp[curr] == dp[curr - 1]:
            curr -= 1
        else:
            chosen.append(items[curr])
            curr = p[curr]

    chosen.reverse()
    return dp[n], chosen


# ============================================================================
# 4. Interval Merging & Insertion
# ============================================================================

def merge_intervals(intervals: List[Tuple[int, int]]) -> List[Tuple[int, int]]:
    """
    Merges all overlapping intervals into contiguous disjoint intervals.
    Time Complexity: O(n log n). Space Complexity: O(n).
    """
    if not intervals:
        return []

    sorted_intervals = sorted(intervals, key=lambda x: (x[0], x[1]))
    merged: List[Tuple[int, int]] = [sorted_intervals[0]]

    for s, e in sorted_intervals[1:]:
        last_s, last_e = merged[-1]
        if s <= last_e:
            merged[-1] = (last_s, max(last_e, e))
        else:
            merged.append((s, e))

    return merged


def insert_interval(
    intervals: List[Tuple[int, int]], new_interval: Tuple[int, int]
) -> List[Tuple[int, int]]:
    """
    Inserts a new interval into an already sorted list of non-overlapping intervals
    and merges overlapping intervals in a single linear scan.
    Three phases:
      1. Intervals ending strictly before new_interval starts.
      2. Intervals overlapping with new_interval.
      3. Intervals starting strictly after new_interval ends.
    Time Complexity: O(n). Space Complexity: O(n).
    """
    result: List[Tuple[int, int]] = []
    i = 0
    n = len(intervals)
    new_s, new_e = new_interval

    # Phase 1: completely before
    while i < n and intervals[i][1] < new_s:
        result.append(intervals[i])
        i += 1

    # Phase 2: overlapping
    while i < n and intervals[i][0] <= new_e:
        new_s = min(new_s, intervals[i][0])
        new_e = max(new_e, intervals[i][1])
        i += 1
    result.append((new_s, new_e))

    # Phase 3: completely after
    while i < n:
        result.append(intervals[i])
        i += 1

    return result


# ============================================================================
# Unit Tests
# ============================================================================

class TestIntervalScheduling(unittest.TestCase):

    def test_unweighted_greedy_and_counterexample(self):
        # Arthur's counterexample: [1, 10), [2, 3), [4, 5), [6, 7), [8, 9)
        # Greedy by start time picks [1, 10) (count 1).
        # Greedy by finish time picks [2, 3), [4, 5), [6, 7), [8, 9) (count 4).
        intervals = [(1, 10), (2, 3), (4, 5), (6, 7), (8, 9)]
        self.assertEqual(max_non_overlapping_intervals(intervals), 4)

        chosen = reconstruct_non_overlapping_schedule(intervals)
        self.assertEqual(chosen, [(2, 3), (4, 5), (6, 7), (8, 9)])

    def test_unweighted_empty_and_single(self):
        self.assertEqual(max_non_overlapping_intervals([]), 0)
        self.assertEqual(reconstruct_non_overlapping_schedule([]), [])

        single = [(5, 10)]
        self.assertEqual(max_non_overlapping_intervals(single), 1)
        self.assertEqual(reconstruct_non_overlapping_schedule(single), [(5, 10)])

    def test_unweighted_touching(self):
        touching = [(1, 3), (3, 5), (5, 7)]
        self.assertEqual(max_non_overlapping_intervals(touching), 3)
        self.assertEqual(reconstruct_non_overlapping_schedule(touching), [(1, 3), (3, 5), (5, 7)])

    def test_meeting_rooms_heap_and_sweepline_equivalence(self):
        intervals = [(0, 30), (5, 10), (15, 20)]
        self.assertEqual(min_meeting_rooms_heap(intervals), 2)
        self.assertEqual(min_meeting_rooms_sweepline(intervals), 2)

        # Touching intervals in [s, e): room frees at 5 before next starts at 5 -> 1 room
        touching = [(1, 5), (5, 10), (10, 15)]
        self.assertEqual(min_meeting_rooms_heap(touching), 1)
        self.assertEqual(min_meeting_rooms_sweepline(touching), 1)

        # All overlapping
        all_overlap = [(1, 10), (2, 9), (3, 8), (4, 7)]
        self.assertEqual(min_meeting_rooms_heap(all_overlap), 4)
        self.assertEqual(min_meeting_rooms_sweepline(all_overlap), 4)

        # Empty
        self.assertEqual(min_meeting_rooms_heap([]), 0)
        self.assertEqual(min_meeting_rooms_sweepline([]), 0)

    def test_weighted_counterexample_and_reconstruction(self):
        # Arthur's counterexample: [1, 2, w=2], [2, 3, w=2], [1, 3, w=10]
        # Greedy earliest finish picks [1, 2] and [2, 3] (total weight = 4).
        # Optimal DP picks [1, 3] (total weight = 10).
        intervals = [(1, 2, 2), (2, 3, 2), (1, 3, 10)]
        self.assertEqual(weighted_interval_scheduling(intervals), 10)

        max_wt, chosen = weighted_interval_scheduling_reconstruct(intervals)
        self.assertEqual(max_wt, 10)
        self.assertEqual(chosen, [(1, 3, 10)])

    def test_weighted_complex_large_values(self):
        intervals = [
            (1, 4, 1_000_000_000),
            (3, 5, 2_000_000_000),
            (0, 6, 2_500_000_000),
            (4, 7, 2_000_000_000),
            (3, 8, 1_000_000_000),
            (5, 9, 3_000_000_000),
            (6, 10, 2_000_000_000),
            (8, 11, 4_000_000_000),
        ]
        ans = weighted_interval_scheduling(intervals)
        self.assertEqual(ans, 7_000_000_000)

        max_wt, chosen = weighted_interval_scheduling_reconstruct(intervals)
        self.assertEqual(max_wt, 7_000_000_000)
        self.assertEqual(
            chosen,
            [(1, 4, 1_000_000_000), (4, 7, 2_000_000_000), (8, 11, 4_000_000_000)]
        )

    def test_merge_intervals(self):
        intervals = [(1, 3), (2, 6), (8, 10), (15, 18)]
        self.assertEqual(merge_intervals(intervals), [(1, 6), (8, 10), (15, 18)])

        touching = [(1, 4), (4, 5)]
        self.assertEqual(merge_intervals(touching), [(1, 5)])

        contained = [(1, 10), (2, 3), (4, 8)]
        self.assertEqual(merge_intervals(contained), [(1, 10)])

        self.assertEqual(merge_intervals([]), [])

    def test_insert_interval(self):
        intervals1 = [(1, 3), (6, 9)]
        self.assertEqual(insert_interval(intervals1, (2, 5)), [(1, 5), (6, 9)])

        intervals2 = [(1, 2), (3, 5), (6, 7), (8, 10), (12, 16)]
        self.assertEqual(
            insert_interval(intervals2, (4, 8)),
            [(1, 2), (3, 10), (12, 16)]
        )

        # Before all
        self.assertEqual(insert_interval(intervals1, (0, 0)), [(0, 0), (1, 3), (6, 9)])

        # After all
        self.assertEqual(insert_interval(intervals1, (11, 12)), [(1, 3), (6, 9), (11, 12)])

        # Empty list
        self.assertEqual(insert_interval([], (5, 7)), [(5, 7)])


if __name__ == "__main__":
    unittest.main()
