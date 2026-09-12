"""
Meet-in-the-Middle algorithmic paradigm implementation in Python.

Provides:
1. Exact Subset Sum via Meet-in-the-Middle and binary search.
2. Knapsack capacity variant (maximum subset sum <= W).
3. 4-Sum problem reducing O(n^4) search to O(n^2).
4. Bidirectional Breadth-First Search on unweighted graphs.
5. Comprehensive unit tests.
"""

import bisect
from collections import deque, Counter
from typing import List, Dict
import unittest


def generate_subset_sums(arr: List[int]) -> List[int]:
    """Generate all 2^len(arr) subset sums of arr."""
    sums = [0]
    for x in arr:
        sums.extend([s + x for s in sums])
    return sums


def subset_sum_exact(nums: List[int], target: int) -> bool:
    """
    Check if any subset of nums sums exactly to target using Meet-in-the-Middle.
    Splits nums into two halves of size n/2, reducing 2^n to O(2^(n/2) log(2^(n/2))).
    """
    n = len(nums)
    if n == 0:
        return target == 0

    mid = n // 2
    left = nums[:mid]
    right = nums[mid:]

    left_sums = generate_subset_sums(left)
    right_sums = sorted(generate_subset_sums(right))

    for s_left in left_sums:
        needed = target - s_left
        idx = bisect.bisect_left(right_sums, needed)
        if idx < len(right_sums) and right_sums[idx] == needed:
            return True
    return False


def max_subset_sum_le(nums: List[int], W: int) -> int:
    """
    Find maximum subset sum <= W using Meet-in-the-Middle.
    """
    n = len(nums)
    if n == 0:
        return 0

    mid = n // 2
    left = nums[:mid]
    right = nums[mid:]

    left_sums = generate_subset_sums(left)
    right_sums = sorted(generate_subset_sums(right))

    best = 0
    for s_left in left_sums:
        if s_left > W:
            continue
        rem = W - s_left
        idx = bisect.bisect_right(right_sums, rem)
        if idx > 0:
            total = s_left + right_sums[idx - 1]
            if total > best:
                best = total
    return best


def four_sum_count(A: List[int], B: List[int], C: List[int], D: List[int], target: int = 0) -> int:
    """
    Count tuples (i, j, k, l) such that A[i] + B[j] + C[k] + D[l] == target.
    Reduces O(n^4) brute force to O(n^2) hash map lookup.
    """
    ab_sums: Dict[int, int] = Counter(a + b for a in A for b in B)
    return sum(ab_sums.get(target - (c + d), 0) for c in C for d in D)


def bidirectional_bfs(n: int, adj: List[List[int]], start: int, target: int) -> int:
    """
    Shortest path distance between start and target in an unweighted graph using Bidirectional BFS.
    Returns -1 if target is unreachable.
    """
    if start == target:
        return 0

    dist_start = {start: 0}
    dist_target = {target: 0}
    q_start = deque([start])
    q_target = deque([target])

    while q_start and q_target:
        # Expand the smaller queue to balance tree expansion
        if len(q_start) <= len(q_target):
            curr = q_start.popleft()
            for neighbor in adj[curr]:
                if neighbor in dist_target:
                    return dist_start[curr] + 1 + dist_target[neighbor]
                if neighbor not in dist_start:
                    dist_start[neighbor] = dist_start[curr] + 1
                    q_start.append(neighbor)
        else:
            curr = q_target.popleft()
            for neighbor in adj[curr]:
                if neighbor in dist_start:
                    return dist_target[curr] + 1 + dist_start[neighbor]
                if neighbor not in dist_target:
                    dist_target[neighbor] = dist_target[curr] + 1
                    q_target.append(neighbor)

    return -1


class TestMeetInTheMiddle(unittest.TestCase):
    def test_subset_sum_exact(self):
        nums = [3, 34, 4, 12, 5, 2]
        self.assertTrue(subset_sum_exact(nums, 9))
        self.assertTrue(subset_sum_exact(nums, 14))
        self.assertFalse(subset_sum_exact(nums, 30))
        self.assertTrue(subset_sum_exact(nums, 0))

    def test_max_subset_sum_le(self):
        nums = [10, 20, 30, 40, 50]
        self.assertEqual(max_subset_sum_le(nums, 65), 60)
        self.assertEqual(max_subset_sum_le(nums, 100), 100)
        self.assertEqual(max_subset_sum_le(nums, 5), 0)

    def test_four_sum_count(self):
        A = [1, 2]
        B = [-2, -1]
        C = [-1, 2]
        D = [0, 2]
        self.assertEqual(four_sum_count(A, B, C, D, 0), 2)

    def test_bidirectional_bfs(self):
        # Line graph 0 - 1 - 2 - 3 - 4
        adj = [
            [1],
            [0, 2],
            [1, 3],
            [2, 4],
            [3]
        ]
        self.assertEqual(bidirectional_bfs(5, adj, 0, 4), 4)
        self.assertEqual(bidirectional_bfs(5, adj, 1, 3), 2)
        self.assertEqual(bidirectional_bfs(5, adj, 2, 2), 0)

        # Disconnected
        disc_adj = [[1], [0], [3], [2]]
        self.assertEqual(bidirectional_bfs(4, disc_adj, 0, 3), -1)


if __name__ == '__main__':
    unittest.main()
