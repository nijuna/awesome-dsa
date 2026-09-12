"""
Treap (Randomized Cartesian Tree)

This module provides reference implementations of:
- Treap: Combining BST on keys with Heap on random priorities via split and merge
"""

from typing import List, Optional, Tuple
import random
import unittest


class Treap:
    class _Node:
        def __init__(self, key: int, priority: int):
            self.key = key
            self.priority = priority
            self.size = 1
            self.left: Optional['Treap._Node'] = None
            self.right: Optional['Treap._Node'] = None

    def __init__(self, seed: int = 1337):
        self._root: Optional[Treap._Node] = None
        self._rng = random.Random(seed)

    def empty(self) -> bool:
        return self._root is None

    def size(self) -> int:
        return self._get_size(self._root)

    def _get_size(self, node: Optional['Treap._Node']) -> int:
        return node.size if node else 0

    def _update_size(self, node: Optional['Treap._Node']) -> None:
        if node:
            node.size = 1 + self._get_size(node.left) + self._get_size(node.right)

    def _split(
        self, node: Optional['Treap._Node'], key: int
    ) -> Tuple[Optional['Treap._Node'], Optional['Treap._Node']]:
        """Splits tree rooted at node into left (keys <= key) and right (keys > key)."""
        if not node:
            return None, None

        if node.key <= key:
            node.right, right = self._split(node.right, key)
            self._update_size(node)
            return node, right
        else:
            left, node.left = self._split(node.left, key)
            self._update_size(node)
            return left, node

    def _merge(
        self, left: Optional['Treap._Node'], right: Optional['Treap._Node']
    ) -> Optional['Treap._Node']:
        """Merges two treaps where all keys in left < all keys in right."""
        if not left:
            return right
        if not right:
            return left

        if left.priority >= right.priority:
            left.right = self._merge(left.right, right)
            self._update_size(left)
            return left
        else:
            right.left = self._merge(left, right.left)
            self._update_size(right)
            return right

    def insert(self, key: int) -> bool:
        """Inserts a key. Returns False if duplicate."""
        if self.contains(key):
            return False

        p = self._rng.randint(0, 1 << 30)
        new_node = self._Node(key, p)

        left, right = self._split(self._root, key)
        self._root = self._merge(self._merge(left, new_node), right)
        return True

    def erase(self, key: int) -> bool:
        """Removes a key. Returns True if removed, False if not found."""
        if not self.contains(key):
            return False

        left, right = self._split(self._root, key - 1)
        mid, right = self._split(right, key)

        self._root = self._merge(left, right)
        return True

    def contains(self, key: int) -> bool:
        """Searches for key following BST property."""
        cur = self._root
        while cur:
            if cur.key == key:
                return True
            cur = cur.left if key < cur.key else cur.right
        return False

    def kth_element(self, k: int) -> Optional[int]:
        """Returns k-th smallest element (0-indexed)."""
        if k < 0 or k >= self.size():
            return None
        cur = self._root
        while cur:
            left_sz = self._get_size(cur.left)
            if k == left_sz:
                return cur.key
            elif k < left_sz:
                cur = cur.left
            else:
                k -= left_sz + 1
                cur = cur.right
        return None

    def inorder(self) -> List[int]:
        """Returns keys in sorted order."""
        res: List[int] = []

        def _inorder(node: Optional['Treap._Node']):
            if not node:
                return
            _inorder(node.left)
            res.append(node.key)
            _inorder(node.right)

        _inorder(self._root)
        return res

    def verify_invariants(self) -> bool:
        def _verify(node, min_k, max_k):
            if not node:
                return True
            if min_k is not None and node.key <= min_k:
                return False
            if max_k is not None and node.key >= max_k:
                return False
            if node.left and node.left.priority > node.priority:
                return False
            if node.right and node.right.priority > node.priority:
                return False
            if node.size != 1 + self._get_size(node.left) + self._get_size(node.right):
                return False
            return _verify(node.left, min_k, node.key) and _verify(node.right, node.key, max_k)

        return _verify(self._root, None, None)


# ============================================================================
# Unit Tests
# ============================================================================

class TestTreap(unittest.TestCase):
    def test_treap_basic(self):
        tr = Treap(42)
        self.assertTrue(tr.empty())
        keys = [50, 30, 70, 20, 40, 60, 80]
        for k in keys:
            self.assertTrue(tr.insert(k))

        self.assertFalse(tr.insert(50))
        self.assertEqual(tr.size(), 7)
        self.assertTrue(tr.verify_invariants())

        for k in keys:
            self.assertTrue(tr.contains(k))
        self.assertFalse(tr.contains(999))

        self.assertEqual(tr.inorder(), [20, 30, 40, 50, 60, 70, 80])
        self.assertEqual(tr.kth_element(0), 20)
        self.assertEqual(tr.kth_element(3), 50)
        self.assertEqual(tr.kth_element(6), 80)
        self.assertIsNone(tr.kth_element(7))

    def test_treap_erase(self):
        tr = Treap(42)
        for k in [50, 30, 70, 20, 40, 60, 80]:
            tr.insert(k)

        self.assertTrue(tr.erase(20))
        self.assertFalse(tr.contains(20))
        self.assertTrue(tr.verify_invariants())

        self.assertTrue(tr.erase(50))
        self.assertFalse(tr.contains(50))
        self.assertTrue(tr.verify_invariants())

        self.assertEqual(tr.inorder(), [30, 40, 60, 70, 80])
        self.assertFalse(tr.erase(999))


if __name__ == "__main__":
    unittest.main()
