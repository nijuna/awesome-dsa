"""
Branch Prediction and CPU Pipelines Primitives and Hardware Simulation.

Implements branchless arithmetic alternatives (select, min, max, clamp, lower_bound)
and software models of branch prediction hardware (2-bit saturating counter / bimodal predictor).
"""

import unittest
from typing import List


def conditional_sum_branchy(data: List[int], threshold: int) -> int:
    """Conditional accumulation with an explicit conditional branch."""
    total = 0
    for x in data:
        if x >= threshold:
            total += x
    return total


def conditional_sum_branchless(data: List[int], threshold: int) -> int:
    """Conditional accumulation without branches using boolean arithmetic."""
    total = 0
    for x in data:
        total += x * int(x >= threshold)
    return total


def select_branchless(cond: bool, a: int, b: int) -> int:
    """Branchless select: returns a if cond else b using bitwise masking."""
    mask = -int(cond)  # True -> -1 (all 1s in binary), False -> 0
    return (a & mask) | (b & ~mask)


def min_branchless(a: int, b: int) -> int:
    """Branchless minimum for 64-bit integers."""
    diff = a - b
    mask = diff >> 63  # -1 if a < b, 0 if a >= b
    return b + (diff & mask)


def max_branchless(a: int, b: int) -> int:
    """Branchless maximum for 64-bit integers."""
    diff = a - b
    mask = diff >> 63  # -1 if a < b, 0 if a >= b
    return a - (diff & mask)


def clamp_branchless(x: int, low: int, high: int) -> int:
    """Branchless clamp of x between [low, high]."""
    return max_branchless(low, min_branchless(x, high))


def branchless_lower_bound(arr: List[int], target: int) -> int:
    """
    Branchless binary search (lower bound index).
    Advances index via conditional select arithmetic instead of unpredictable branch jumps.
    """
    n = len(arr)
    if n == 0:
        return 0

    base = 0
    while n > 1:
        half = n // 2
        # Advance base conditionally without if/else branch
        advance = half if arr[base + half] < target else 0
        base += advance
        n -= half

    return base + 1 if arr[base] < target else base


class BimodalPredictor:
    """
    Simulates a 2-bit saturating counter branch predictor.

    States:
        0: Strongly Not Taken (SNT)
        1: Weakly Not Taken (WNT)
        2: Weakly Taken (WT)
        3: Strongly Taken (ST)
    """

    def __init__(self, size: int = 1024):
        self.size = size
        self.table = [2] * size  # Initialize all entries to Weakly Taken (2)
        self.mask = size - 1

    def _index(self, pc: int) -> int:
        return pc & self.mask

    def predict(self, pc: int) -> bool:
        """Predicts branch outcome: True for Taken, False for Not Taken."""
        return self.table[self._index(pc)] >= 2

    def update(self, pc: int, actual_taken: bool) -> None:
        """Updates the 2-bit counter state based on actual branch outcome."""
        idx = self._index(pc)
        state = self.table[idx]
        if actual_taken:
            if state < 3:
                self.table[idx] = state + 1
        else:
            if state > 0:
                self.table[idx] = state - 1


class TestBranchPredictionAndPipelines(unittest.TestCase):
    def test_select_branchless(self):
        cases = [
            (True, 42, 10, 42),
            (False, 42, 10, 10),
            (True, -5, -20, -5),
            (False, -5, -20, -20),
            (True, 0, 100, 0),
            (False, 0, 100, 100),
        ]
        for cond, a, b, expected in cases:
            self.assertEqual(select_branchless(cond, a, b), expected)

    def test_min_max_clamp_branchless(self):
        vals = [-100, -50, -1, 0, 1, 25, 100]
        for a in vals:
            for b in vals:
                self.assertEqual(min_branchless(a, b), min(a, b))
                self.assertEqual(max_branchless(a, b), max(a, b))

        for x in range(-50, 150):
            expected_clamp = min(max(x, 0), 100)
            self.assertEqual(clamp_branchless(x, 0, 100), expected_clamp)

    def test_conditional_sum_equivalence(self):
        data = [12, 250, 89, 130, 200, 4, 15, 180, 210, 64]
        threshold = 128
        self.assertEqual(conditional_sum_branchy(data, threshold), conditional_sum_branchless(data, threshold))

    def test_branchless_lower_bound(self):
        import bisect

        arr = [2, 5, 8, 12, 16, 23, 38, 56, 72, 91]
        for target in range(0, 105):
            expected = bisect.bisect_left(arr, target)
            actual = branchless_lower_bound(arr, target)
            self.assertEqual(actual, expected, f"Failed on target {target}")

        # Edge cases
        self.assertEqual(branchless_lower_bound([], 10), 0)
        self.assertEqual(branchless_lower_bound([10], 5), 0)
        self.assertEqual(branchless_lower_bound([10], 10), 0)
        self.assertEqual(branchless_lower_bound([10], 15), 1)

    def test_bimodal_predictor(self):
        predictor = BimodalPredictor(size=64)
        pc = 0x1004

        # Initial prediction is True (Weakly Taken = 2)
        self.assertTrue(predictor.predict(pc))

        # Train with taken branches
        for _ in range(10):
            predictor.update(pc, True)
        self.assertTrue(predictor.predict(pc))

        # Single anomaly: Not Taken
        predictor.update(pc, False)
        # Should still predict Taken due to 2-bit hysteresis (transition ST -> WT)
        self.assertTrue(predictor.predict(pc))

        # Second Not Taken
        predictor.update(pc, False)
        # Now transitions WT -> WNT, so prediction becomes False
        self.assertFalse(predictor.predict(pc))


if __name__ == "__main__":
    unittest.main()
