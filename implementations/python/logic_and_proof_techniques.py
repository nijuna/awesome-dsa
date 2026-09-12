"""
Formal Logic, Mathematical Induction, and Loop Invariant Verification.

Provides truth-table verification of logical equivalences (De Morgan, contraposition),
inductive proof checkers, and loop-invariant-instrumented algorithmic procedures.
"""

import unittest
from typing import Callable, List


def implies(p: bool, q: bool) -> bool:
    """Logical implication: P => Q."""
    return (not p) or q


def is_tautology_2vars(expr: Callable[[bool, bool], bool]) -> bool:
    """Checks whether boolean expression f(P, Q) is true for all 4 truth assignments."""
    for p in (False, True):
        for q in (False, True):
            if not expr(p, q):
                return False
    return True


def verify_induction(base: int, max_n: int, prop: Callable[[int], bool]) -> bool:
    """
    Empirically verifies weak induction for predicate prop(n) on [base, max_n].
    Checks base case and transmission implication prop(k) => prop(k+1).
    """
    if not prop(base):
        return False
    for k in range(base, max_n):
        if not implies(prop(k), prop(k + 1)):
            return False
    return True


def binary_search_with_invariant(arr: List[int], target: int) -> int:
    """
    Binary search formally instrumented with loop invariant assertions.
    Invariant: If target is in arr, it must be within arr[low .. high].
    """
    low = 0
    high = len(arr) - 1

    def assert_invariant(l: int, h: int):
        for idx, val in enumerate(arr):
            if val == target:
                assert l <= idx <= h, f"Invariant violated: idx={idx} not in [{l}, {h}]"

    assert_invariant(low, high)

    while low <= high:
        mid = (low + high) // 2
        if arr[mid] == target:
            return mid
        elif arr[mid] < target:
            low = mid + 1
        else:
            high = mid - 1
        assert_invariant(low, high)

    assert target not in arr
    return -1


def dutch_national_flag_with_invariant(nums: List[int]) -> None:
    """
    Sorts an array of 0s, 1s, and 2s in-place with invariant checking.
    Invariant:
      nums[0 .. low-1] == 0
      nums[low .. mid-1] == 1
      nums[mid .. high] == unclassified
      nums[high+1 .. end] == 2
    """
    low = 0
    mid = 0
    high = len(nums) - 1

    def assert_invariant(l: int, m: int, h: int):
        for i in range(l):
            assert nums[i] == 0
        for i in range(l, m):
            assert nums[i] == 1
        for i in range(h + 1, len(nums)):
            assert nums[i] == 2

    assert_invariant(low, mid, high)

    while mid <= high:
        if nums[mid] == 0:
            nums[low], nums[mid] = nums[mid], nums[low]
            low += 1
            mid += 1
        elif nums[mid] == 1:
            mid += 1
        else:
            nums[mid], nums[high] = nums[high], nums[mid]
            high -= 1
        assert_invariant(low, mid, high)

    assert_invariant(low, mid, high)


class TestLogicAndProofTechniques(unittest.TestCase):
    def test_de_morgan_laws(self):
        # !(P && Q) <=> (!P || !Q)
        self.assertTrue(is_tautology_2vars(lambda p, q: (not (p and q)) == ((not p) or (not q))))
        # !(P || Q) <=> (!P && !Q)
        self.assertTrue(is_tautology_2vars(lambda p, q: (not (p or q)) == ((not p) and (not q))))

    def test_contrapositive_equivalence(self):
        # (P => Q) <=> (!Q => !P)
        self.assertTrue(is_tautology_2vars(lambda p, q: implies(p, q) == implies(not q, not p)))

    def test_implication_identity(self):
        # (P => Q) <=> (!P || Q)
        self.assertTrue(is_tautology_2vars(lambda p, q: implies(p, q) == ((not p) or q)))

    def test_induction_sum(self):
        # sum_{i=1}^n i == n(n+1)/2
        self.assertTrue(
            verify_induction(
                1, 500, lambda n: sum(range(1, n + 1)) == (n * (n + 1)) // 2
            )
        )

    def test_binary_search_invariant(self):
        arr = [1, 3, 5, 7, 9, 11, 13, 15]
        self.assertEqual(binary_search_with_invariant(arr, 7), 3)
        self.assertEqual(binary_search_with_invariant(arr, 1), 0)
        self.assertEqual(binary_search_with_invariant(arr, 15), 7)
        self.assertEqual(binary_search_with_invariant(arr, 8), -1)

    def test_dutch_national_flag_invariant(self):
        nums = [2, 0, 2, 1, 1, 0, 2, 1, 0]
        dutch_national_flag_with_invariant(nums)
        self.assertEqual(nums, sorted([2, 0, 2, 1, 1, 0, 2, 1, 0]))


if __name__ == "__main__":
    unittest.main()
