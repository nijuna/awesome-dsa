"""
Reference Implementation: The Knapsack Problem Family
Demonstrates:
1. 0-1 Knapsack: Classical 2D DP (O(nW) Time, O(nW) Space) & 1D Backward Rolling Array (O(W) Space).
2. 0-1 Knapsack Item Reconstruction via Backtracking.
3. Unbounded Knapsack: 1D Forward Rolling Array (O(nW) Time, O(W) Space) & Choice Tracking Reconstruction.
4. Bounded Knapsack: Binary Power Splitting (O(W sum log m) Time) & Full Multiplicity Reconstruction.
5. Variations: Exact-Fill 0-1 Knapsack (Sentinel Initialization) & Subset Sum Feasibility.

Language: Python 3
"""

import unittest
from typing import List, Tuple


# ============================================================================
# 1. 0-1 Knapsack (Each Item at Most Once)
# ============================================================================

def knapsack_01_2d(weight: List[int], value: List[int], W: int) -> int:
    """
    2D Classical Formulation: O(nW) Time, O(nW) Space.
    dp[i][c] = max value considering first i items with capacity c.
    """
    n = len(weight)
    dp = [[0] * (W + 1) for _ in range(n + 1)]

    for i in range(1, n + 1):
        w = weight[i - 1]
        v = value[i - 1]
        for c in range(W + 1):
            dp[i][c] = dp[i - 1][c]
            if w <= c:
                dp[i][c] = max(dp[i][c], dp[i - 1][c - w] + v)

    return dp[n][W]


def knapsack_01_1d(weight: List[int], value: List[int], W: int) -> int:
    """
    1D Space-Optimized Rolling Array: O(nW) Time, O(W) Space.
    Backward capacity loop (W down to w) prevents item reuse in the same round.
    """
    dp = [0] * (W + 1)

    for w, v in zip(weight, value):
        for c in range(W, w - 1, -1):
            dp[c] = max(dp[c], dp[c - w] + v)

    return dp[W]


def knapsack_01_reconstruct(weight: List[int], value: List[int], W: int) -> Tuple[int, List[int]]:
    """
    0-1 Knapsack Item Reconstruction using full 2D table.
    Returns (max_value, chosen_indices).
    """
    n = len(weight)
    dp = [[0] * (W + 1) for _ in range(n + 1)]

    for i in range(1, n + 1):
        w = weight[i - 1]
        v = value[i - 1]
        for c in range(W + 1):
            dp[i][c] = dp[i - 1][c]
            if w <= c:
                dp[i][c] = max(dp[i][c], dp[i - 1][c - w] + v)

    chosen = []
    c = W
    for i in range(n, 0, -1):
        if dp[i][c] != dp[i - 1][c]:
            chosen.append(i - 1)
            c -= weight[i - 1]

    chosen.reverse()
    return dp[n][W], chosen


# ============================================================================
# 2. Unbounded Knapsack (Unlimited Copies of Each Item)
# ============================================================================

def knapsack_unbounded(weight: List[int], value: List[int], W: int) -> int:
    """
    1D Forward Rolling Array: O(nW) Time, O(W) Space.
    Forward capacity loop (w up to W) allows unrestricted reuse in the same round.
    """
    dp = [0] * (W + 1)

    for w, v in zip(weight, value):
        for c in range(w, W + 1):
            dp[c] = max(dp[c], dp[c - w] + v)

    return dp[W]


def knapsack_unbounded_reconstruct(weight: List[int], value: List[int], W: int) -> Tuple[int, List[int]]:
    """
    Unbounded Knapsack Reconstruction using a 1D choice-tracking array.
    Returns (max_value, chosen_indices).
    """
    dp = [0] * (W + 1)
    choice = [-1] * (W + 1)

    for i, (w, v) in enumerate(zip(weight, value)):
        for c in range(w, W + 1):
            candidate = dp[c - w] + v
            if candidate > dp[c]:
                dp[c] = candidate
                choice[c] = i

    chosen = []
    c = W
    while c > 0 and choice[c] != -1:
        i = choice[c]
        chosen.append(i)
        c -= weight[i]

    chosen.reverse()
    return dp[W], chosen


# ============================================================================
# 3. Bounded Knapsack (Multiplicity m_i via Binary Power Splitting)
# ============================================================================

def knapsack_bounded_binary_split(weight: List[int], value: List[int], count: List[int], W: int) -> int:
    """
    Bounded Knapsack with Binary Power Splitting: O(W * sum(log count_i)) Time.
    Decomposes count_i into powers {1, 2, 4, ..., remainder}.
    """
    expanded = []

    for w, v, k in zip(weight, value, count):
        power = 1
        while power <= k:
            expanded.append((w * power, v * power))
            k -= power
            power <<= 1
        if k > 0:
            expanded.append((w * k, v * k))

    dp = [0] * (W + 1)
    for ew, ev in expanded:
        for c in range(W, ew - 1, -1):
            dp[c] = max(dp[c], dp[c - ew] + ev)

    return dp[W]


def knapsack_bounded_reconstruct(weight: List[int], value: List[int], count: List[int], W: int) -> Tuple[int, List[int]]:
    """
    Bounded Knapsack Multiplicity Reconstruction.
    Returns (max_value, counts) where counts[i] is the number of copies chosen of item i.
    """
    expanded = []
    n = len(weight)

    for i, (w, v, k) in enumerate(zip(weight, value, count)):
        power = 1
        while power <= k:
            expanded.append((w * power, v * power, i, power))
            k -= power
            power <<= 1
        if k > 0:
            expanded.append((w * k, v * k, i, k))

    m = len(expanded)
    dp = [[0] * (W + 1) for _ in range(m + 1)]

    for i in range(1, m + 1):
        ew, ev, _, _ = expanded[i - 1]
        for c in range(W + 1):
            dp[i][c] = dp[i - 1][c]
            if ew <= c:
                dp[i][c] = max(dp[i][c], dp[i - 1][c - ew] + ev)

    used = [0] * n
    c = W
    for i in range(m, 0, -1):
        if dp[i][c] != dp[i - 1][c]:
            ew, _, idx, mult = expanded[i - 1]
            used[idx] += mult
            c -= ew

    return dp[m][W], used


# ============================================================================
# 4. Knapsack Variations: Exact-Fill & Subset Sum Feasibility
# ============================================================================

def knapsack_01_exact_fill(weight: List[int], value: List[int], W: int) -> int:
    """
    Exact-Fill 0-1 Knapsack: Returns max value achieving exactly weight W,
    or -1 if impossible. Uses -inf sentinels.
    """
    neg_inf = float('-inf')
    dp = [neg_inf] * (W + 1)
    dp[0] = 0

    for w, v in zip(weight, value):
        for c in range(W, w - 1, -1):
            if dp[c - w] != neg_inf:
                dp[c] = max(dp[c], dp[c - w] + v)

    return int(dp[W]) if dp[W] != neg_inf else -1


def subset_sum_feasible(nums: List[int], target: int) -> bool:
    """
    Subset Sum Feasibility: Returns True if any subset sums exactly to target S.
    """
    if target < 0:
        return False
    dp = [False] * (target + 1)
    dp[0] = True

    for x in nums:
        for s in range(target, x - 1, -1):
            if dp[s - x]:
                dp[s] = True

    return dp[target]


# ============================================================================
# Unit Tests
# ============================================================================

class TestKnapsackFamily(unittest.TestCase):
    def test_01_knapsack(self):
        w = [2, 3, 4, 5]
        v = [3, 4, 5, 6]
        W = 5

        self.assertEqual(knapsack_01_2d(w, v, W), 7)
        self.assertEqual(knapsack_01_1d(w, v, W), 7)

        val, chosen = knapsack_01_reconstruct(w, v, W)
        self.assertEqual(val, 7)
        self.assertEqual(chosen, [0, 1])

    def test_greedy_failure(self):
        w = [6, 5, 5]
        v = [10, 8, 8]
        # Greedy takes item 0 (v=10), remaining cap 4, total 10.
        # Optimal takes items 1 & 2 (v=16).
        self.assertEqual(knapsack_01_1d(w, v, 10), 16)

    def test_unbounded_knapsack(self):
        w = [3]
        v = [5]
        self.assertEqual(knapsack_01_1d(w, v, 9), 5)
        self.assertEqual(knapsack_unbounded(w, v, 9), 15)

        val, chosen = knapsack_unbounded_reconstruct(w, v, 9)
        self.assertEqual(val, 15)
        self.assertEqual(chosen, [0, 0, 0])

    def test_bounded_knapsack(self):
        w = [3]
        v = [5]
        count = [2]
        self.assertEqual(knapsack_bounded_binary_split(w, v, count, 9), 10)

        val, used = knapsack_bounded_reconstruct(w, v, count, 9)
        self.assertEqual(val, 10)
        self.assertEqual(used, [2])

    def test_complex_bounded(self):
        w = [2, 3, 4]
        v = [3, 4, 6]
        count = [3, 2, 1]
        W = 8

        bnd_res = knapsack_bounded_binary_split(w, v, count, W)
        val, used = knapsack_bounded_reconstruct(w, v, count, W)
        self.assertEqual(bnd_res, val)

        total_w = sum(u * item_w for u, item_w in zip(used, w))
        total_v = sum(u * item_v for u, item_v in zip(used, v))
        self.assertLessEqual(total_w, W)
        self.assertEqual(total_v, bnd_res)

    def test_exact_fill(self):
        w = [3, 4, 7]
        v = [10, 15, 30]
        self.assertEqual(knapsack_01_exact_fill(w, v, 7), 30)
        self.assertEqual(knapsack_01_exact_fill(w, v, 8), -1)

    def test_subset_sum(self):
        nums = [3, 34, 4, 12, 5, 2]
        self.assertTrue(subset_sum_feasible(nums, 9))
        self.assertFalse(subset_sum_feasible(nums, 30))
        self.assertTrue(subset_sum_feasible(nums, 0))

    def test_edge_cases(self):
        self.assertEqual(knapsack_01_1d([], [], 10), 0)
        self.assertEqual(knapsack_01_1d([1, 2], [10, 20], 0), 0)


if __name__ == '__main__':
    unittest.main()
