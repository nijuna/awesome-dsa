"""
Recursion and Backtracking implementation in Python.

Provides:
1. N-Queens Solver with diagonal conflict tracking.
2. Power set / Subset generation (with and without duplicate handling).
3. Permutation generation.
4. Combination Sum with pruning.
5. Comprehensive unit tests.
"""

import unittest
from typing import List


class NQueens:
    """N-Queens solver using backtracking with conflict tracking."""

    def __init__(self, n: int):
        self.n = n

    def solve(self) -> List[List[str]]:
        if self.n <= 0:
            return []

        solutions: List[List[str]] = []
        col_placement = [-1] * self.n
        used_cols = set()
        used_diag1 = set()  # r + c
        used_diag2 = set()  # r - c

        def backtrack(row: int) -> None:
            if row == self.n:
                board = []
                for r in range(self.n):
                    c = col_placement[r]
                    row_str = '.' * c + 'Q' + '.' * (self.n - c - 1)
                    board.append(row_str)
                solutions.append(board)
                return

            for c in range(self.n):
                d1 = row + c
                d2 = row - c
                if c in used_cols or d1 in used_diag1 or d2 in used_diag2:
                    continue  # Pruned

                # Choose
                col_placement[row] = c
                used_cols.add(c)
                used_diag1.add(d1)
                used_diag2.add(d2)

                # Explore
                backtrack(row + 1)

                # Unchoose
                used_cols.remove(c)
                used_diag1.remove(d1)
                used_diag2.remove(d2)

        backtrack(0)
        return solutions

    def count_solutions(self) -> int:
        return len(self.solve())


def generate_subsets(nums: List[int]) -> List[List[int]]:
    """Generate power set using choose-explore-unchoose backtracking."""
    result: List[List[int]] = []
    curr: List[int] = []

    def backtrack(idx: int) -> None:
        if idx == len(nums):
            result.append(list(curr))
            return
        # Branch 1: exclude
        backtrack(idx + 1)
        # Branch 2: include
        curr.append(nums[idx])
        backtrack(idx + 1)
        curr.pop()

    backtrack(0)
    return result


def generate_subsets_unique(nums: List[int]) -> List[List[int]]:
    """Generate unique subsets when input contains duplicates (Subsets II)."""
    sorted_nums = sorted(nums)
    result: List[List[int]] = []
    curr: List[int] = []

    def backtrack(start: int) -> None:
        result.append(list(curr))
        for i in range(start, len(sorted_nums)):
            if i > start and sorted_nums[i] == sorted_nums[i - 1]:
                continue
            curr.append(sorted_nums[i])
            backtrack(i + 1)
            curr.pop()

    backtrack(0)
    return result


def generate_permutations(nums: List[int]) -> List[List[int]]:
    """Generate all permutations using backtracking and swapping."""
    result: List[List[int]] = []
    arr = list(nums)

    def backtrack(start: int) -> None:
        if start == len(arr):
            result.append(list(arr))
            return
        for i in range(start, len(arr)):
            arr[start], arr[i] = arr[i], arr[start]
            backtrack(start + 1)
            arr[start], arr[i] = arr[i], arr[start]

    backtrack(0)
    return result


def combination_sum(candidates: List[int], target: int) -> List[List[int]]:
    """Find all unique combinations summing to target with candidate reuse."""
    candidates = sorted(candidates)
    result: List[List[int]] = []
    curr: List[int] = []

    def backtrack(start: int, remain: int) -> None:
        if remain == 0:
            result.append(list(curr))
            return
        for i in range(start, len(candidates)):
            if candidates[i] > remain:
                break  # Prune
            curr.append(candidates[i])
            backtrack(i, remain - candidates[i])
            curr.pop()

    backtrack(0, target)
    return result


class TestRecursionAndBacktracking(unittest.TestCase):
    def test_n_queens(self):
        self.assertEqual(NQueens(1).count_solutions(), 1)
        self.assertEqual(NQueens(2).count_solutions(), 0)
        self.assertEqual(NQueens(3).count_solutions(), 0)
        self.assertEqual(NQueens(4).count_solutions(), 2)
        sols4 = NQueens(4).solve()
        self.assertEqual(len(sols4), 2)
        self.assertEqual(len(sols4[0]), 4)
        self.assertEqual(NQueens(8).count_solutions(), 92)

    def test_subsets(self):
        subs = generate_subsets([1, 2, 3])
        self.assertEqual(len(subs), 8)

        uniq_subs = generate_subsets_unique([1, 2, 2])
        self.assertEqual(len(uniq_subs), 6)

    def test_permutations(self):
        perms = generate_permutations([1, 2, 3])
        self.assertEqual(len(perms), 6)
        self.assertIn([1, 2, 3], perms)
        self.assertIn([3, 2, 1], perms)

        single = generate_permutations([9])
        self.assertEqual(single, [[9]])

    def test_combination_sum(self):
        res = combination_sum([2, 3, 6, 7], 7)
        self.assertEqual(res, [[2, 2, 3], [7]])
        self.assertEqual(combination_sum([2], 1), [])


if __name__ == '__main__':
    unittest.main()
