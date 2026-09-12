"""
Differential and Invariant Testing Framework for Data Structures.

Implements a self-verifying AVL tree candidate and tests it differentially
against Python's built-in set oracle across thousands of randomized mutations,
validating internal balance and binary search invariants at each step.
"""

import random
import unittest
from typing import List, Optional


class AVLNode:
    def __init__(self, key: int):
        self.key = key
        self.height = 1
        self.left: Optional[AVLNode] = None
        self.right: Optional[AVLNode] = None


class AVLTree:
    """Self-verifying AVL tree implementing set semantics."""

    def __init__(self):
        self.root: Optional[AVLNode] = None
        self._size = 0

    def _height(self, node: Optional[AVLNode]) -> int:
        return node.height if node else 0

    def _balance_factor(self, node: Optional[AVLNode]) -> int:
        return self._height(node.left) - self._height(node.right) if node else 0

    def _update_height(self, node: AVLNode) -> None:
        node.height = 1 + max(self._height(node.left), self._height(node.right))

    def _rotate_right(self, y: AVLNode) -> AVLNode:
        x = y.left
        assert x is not None
        t2 = x.right

        x.right = y
        y.left = t2

        self._update_height(y)
        self._update_height(x)
        return x

    def _rotate_left(self, x: AVLNode) -> AVLNode:
        y = x.right
        assert y is not None
        t2 = y.left

        y.left = x
        x.right = t2

        self._update_height(x)
        self._update_height(y)
        return y

    def _balance(self, node: AVLNode) -> AVLNode:
        self._update_height(node)
        bf = self._balance_factor(node)

        # Left heavy
        if bf > 1:
            if self._balance_factor(node.left) < 0:
                assert node.left is not None
                node.left = self._rotate_left(node.left)
            return self._rotate_right(node)

        # Right heavy
        if bf < -1:
            if self._balance_factor(node.right) > 0:
                assert node.right is not None
                node.right = self._rotate_right(node.right)
            return self._rotate_left(node)

        return node

    def insert(self, key: int) -> bool:
        """Inserts a key into the AVL tree. Returns True if inserted, False if duplicate."""
        inserted = False

        def _insert(node: Optional[AVLNode], val: int) -> AVLNode:
            nonlocal inserted
            if not node:
                inserted = True
                return AVLNode(val)

            if val < node.key:
                node.left = _insert(node.left, val)
            elif val > node.key:
                node.right = _insert(node.right, val)
            else:
                inserted = False
                return node

            return self._balance(node)

        self.root = _insert(self.root, key)
        if inserted:
            self._size += 1
        return inserted

    def _min_node(self, node: AVLNode) -> AVLNode:
        curr = node
        while curr.left:
            curr = curr.left
        return curr

    def erase(self, key: int) -> bool:
        """Erases a key from the AVL tree. Returns True if erased, False if not found."""
        erased = False

        def _erase(node: Optional[AVLNode], val: int) -> Optional[AVLNode]:
            nonlocal erased
            if not node:
                erased = False
                return None

            if val < node.key:
                node.left = _erase(node.left, val)
            elif val > node.key:
                node.right = _erase(node.right, val)
            else:
                erased = True
                if not node.left or not node.right:
                    return node.left if node.left else node.right
                else:
                    succ = self._min_node(node.right)
                    node.key = succ.key
                    node.right = _erase(node.right, succ.key)

            return self._balance(node)

        self.root = _erase(self.root, key)
        if erased:
            self._size -= 1
        return erased

    def contains(self, key: int) -> bool:
        curr = self.root
        while curr:
            if key < curr.key:
                curr = curr.left
            elif key > curr.key:
                curr = curr.right
            else:
                return True
        return False

    def size(self) -> int:
        return self._size

    def empty(self) -> bool:
        return self._size == 0

    def to_list(self) -> List[int]:
        out: List[int] = []

        def _in_order(node: Optional[AVLNode]):
            if not node:
                return
            _in_order(node.left)
            out.append(node.key)
            _in_order(node.right)

        _in_order(self.root)
        return out

    def verify_invariants(self) -> bool:
        """Validates strict BST ordering, height metadata, and AVL balance condition."""

        def _verify(node: Optional[AVLNode], low: float, high: float) -> bool:
            if not node:
                return True
            if not (low < node.key < high):
                return False

            lh = self._height(node.left)
            rh = self._height(node.right)

            if node.height != 1 + max(lh, rh):
                return False
            if abs(lh - rh) > 1:
                return False

            return _verify(node.left, low, node.key) and _verify(node.right, node.key, high)

        return _verify(self.root, float("-inf"), float("inf"))


class TestDataStructureVerification(unittest.TestCase):
    def test_boundary_conditions(self):
        tree = AVLTree()
        self.assertTrue(tree.empty())
        self.assertEqual(tree.size(), 0)
        self.assertFalse(tree.contains(10))
        self.assertFalse(tree.erase(10))
        self.assertTrue(tree.verify_invariants())

        # Single element
        self.assertTrue(tree.insert(42))
        self.assertFalse(tree.empty())
        self.assertEqual(tree.size(), 1)
        self.assertTrue(tree.contains(42))
        self.assertTrue(tree.verify_invariants())

        # Duplicate
        self.assertFalse(tree.insert(42))
        self.assertEqual(tree.size(), 1)

        # Removal to empty
        self.assertTrue(tree.erase(42))
        self.assertTrue(tree.empty())
        self.assertEqual(tree.size(), 0)
        self.assertTrue(tree.verify_invariants())

    def test_differential_oracle_stress(self):
        """Differential execution test: candidate AVL vs Python built-in set oracle."""
        rng = random.Random(987654321)
        candidate = AVLTree()
        oracle = set()

        # Execute 5,000 randomized operations
        for step in range(5000):
            op = rng.randint(0, 99)
            key = rng.randint(-300, 300)

            if op < 40:
                # 40% Insert
                cand_res = candidate.insert(key)
                orac_res = key not in oracle
                if orac_res:
                    oracle.add(key)
                self.assertEqual(cand_res, orac_res, f"Insert mismatch at step {step} for key {key}")
            elif op < 70:
                # 30% Erase
                cand_res = candidate.erase(key)
                orac_res = key in oracle
                if orac_res:
                    oracle.remove(key)
                self.assertEqual(cand_res, orac_res, f"Erase mismatch at step {step} for key {key}")
            elif op < 90:
                # 20% Contains
                cand_res = candidate.contains(key)
                orac_res = key in oracle
                self.assertEqual(cand_res, orac_res, f"Contains mismatch at step {step} for key {key}")
            else:
                # 10% Full State / Order comparison
                self.assertEqual(candidate.size(), len(oracle))
                self.assertEqual(candidate.empty(), len(oracle) == 0)
                cand_items = candidate.to_list()
                orac_items = sorted(list(oracle))
                self.assertEqual(cand_items, orac_items)

            # Invariant check
            self.assertTrue(candidate.verify_invariants(), f"Invariant broken at step {step}")

    def test_metamorphic_properties(self):
        """Metamorphic testing: idempotence and round-trip consistency."""
        tree = AVLTree()
        keys = [15, 6, 18, 3, 7, 17, 20, 2, 4, 13, 9]
        for k in keys:
            tree.insert(k)

        orig_list = tree.to_list()
        # Idempotence: re-inserting existing keys produces no state change
        for k in keys:
            self.assertFalse(tree.insert(k))
        self.assertEqual(tree.to_list(), orig_list)

        # Round-trip reversibility: inserting and erasing new key restores state
        tree.insert(999)
        tree.erase(999)
        self.assertEqual(tree.to_list(), orig_list)


if __name__ == "__main__":
    unittest.main()
