"""
Mathematical Induction Algorithmic Realizations in Python.

Implements constructive L-Tromino tiling on 2^k deficient boards
and structural induction tree verification with unit tests.
"""

import unittest
from typing import List, Optional, Tuple


class TrominoBoard:
    """Tromino tiling solver utilizing divide-and-conquer inductive decomposition."""

    def __init__(self, k: int, hole_r: int, hole_c: int):
        self.n = 1 << k
        self.grid = [[0] * self.n for _ in range(self.n)]
        self.grid[hole_r][hole_c] = -1
        self.tile_id = 1
        self._tile_recursive(0, 0, self.n, hole_r, hole_c)

    def _tile_recursive(self, top: int, left: int, size: int, hole_r: int, hole_c: int) -> None:
        if size == 1:
            return

        current_tile = self.tile_id
        self.tile_id += 1

        half = size // 2
        mid_r = top + half
        mid_c = left + half

        hole_top_left = hole_r < mid_r and hole_c < mid_c
        hole_top_right = hole_r < mid_r and hole_c >= mid_c
        hole_bottom_left = hole_r >= mid_r and hole_c < mid_c
        hole_bottom_right = hole_r >= mid_r and hole_c >= mid_c

        # Place central L-tromino covering 3 intact quadrants
        if not hole_top_left:
            self.grid[mid_r - 1][mid_c - 1] = current_tile
        if not hole_top_right:
            self.grid[mid_r - 1][mid_c] = current_tile
        if not hole_bottom_left:
            self.grid[mid_r][mid_c - 1] = current_tile
        if not hole_bottom_right:
            self.grid[mid_r][mid_c] = current_tile

        # Recurse on all 4 quadrants
        self._tile_recursive(
            top, left, half,
            hole_r if hole_top_left else mid_r - 1,
            hole_c if hole_top_left else mid_c - 1
        )
        self._tile_recursive(
            top, mid_c, half,
            hole_r if hole_top_right else mid_r - 1,
            hole_c if hole_top_right else mid_c
        )
        self._tile_recursive(
            mid_r, left, half,
            hole_r if hole_bottom_left else mid_r,
            hole_c if hole_bottom_left else mid_c - 1
        )
        self._tile_recursive(
            mid_r, mid_c, half,
            hole_r if hole_bottom_right else mid_r,
            hole_c if hole_bottom_right else mid_c
        )

    def is_valid_tiling(self, hole_r: int, hole_c: int) -> bool:
        if self.grid[hole_r][hole_c] != -1:
            return False

        counts = {}
        for r in range(self.n):
            for c in range(self.n):
                if r == hole_r and c == hole_c:
                    continue
                tid = self.grid[r][c]
                if tid <= 0 or tid >= self.tile_id:
                    return False
                counts[tid] = counts.get(tid, 0) + 1

        return all(cnt == 3 for cnt in counts.values())


class TreeNode:
    def __init__(self, val: int, left: Optional["TreeNode"] = None, right: Optional["TreeNode"] = None):
        self.val = val
        self.left = left
        self.right = right


def count_leaves_and_internals(root: Optional[TreeNode]) -> Tuple[int, int]:
    """Returns (leaves_count, internal_nodes_count)."""
    if not root:
        return 0, 0
    if not root.left and not root.right:
        return 1, 0  # 1 leaf, 0 internal
    left_leaves, left_internals = count_leaves_and_internals(root.left)
    right_leaves, right_internals = count_leaves_and_internals(root.right)
    return (left_leaves + right_leaves, 1 + left_internals + right_internals)


class TestInduction(unittest.TestCase):
    def test_tromino_tiling(self):
        for k in [1, 2, 3]:
            dim = 1 << k
            for hr, hc in [(0, 0), (dim - 1, dim - 1), (dim // 2, dim // 2)]:
                board = TrominoBoard(k, hr, hc)
                self.assertTrue(board.is_valid_tiling(hr, hc))

    def test_structural_induction_tree(self):
        # Full binary tree:
        #        (1)
        #       /   \
        #     (2)   (3)
        #    /   \
        #  (4)   (5)
        root = TreeNode(1, TreeNode(2, TreeNode(4), TreeNode(5)), TreeNode(3))
        leaves, internals = count_leaves_and_internals(root)
        self.assertEqual(internals, 2)
        self.assertEqual(leaves, 3)
        self.assertEqual(leaves, internals + 1)


if __name__ == "__main__":
    unittest.main()
