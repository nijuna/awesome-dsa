"""
Binary Search Trees (BSTs)

This module provides reference implementations of:
- BinarySearchTree: Search, insert, 3-case deletion, predecessor, and successor
"""

from typing import List, Optional
import unittest


class BinarySearchTree:
    class _Node:
        def __init__(self, key: int):
            self.key = key
            self.left: Optional['BinarySearchTree._Node'] = None
            self.right: Optional['BinarySearchTree._Node'] = None

    def __init__(self):
        self._root: Optional[BinarySearchTree._Node] = None
        self._size = 0

    def empty(self) -> bool:
        return self._size == 0

    def size(self) -> int:
        return self._size

    def insert(self, key: int) -> bool:
        """Inserts a key. Returns False if duplicate."""
        if not self._root:
            self._root = self._Node(key)
            self._size += 1
            return True

        cur = self._root
        while True:
            if key == cur.key:
                return False
            elif key < cur.key:
                if not cur.left:
                    cur.left = self._Node(key)
                    self._size += 1
                    return True
                cur = cur.left
            else:
                if not cur.right:
                    cur.right = self._Node(key)
                    self._size += 1
                    return True
                cur = cur.right

    def contains(self, key: int) -> bool:
        """Searches for a key iteratively."""
        cur = self._root
        while cur:
            if key == cur.key:
                return True
            cur = cur.left if key < cur.key else cur.right
        return False

    def remove(self, key: int) -> bool:
        """Removes a key handling leaf, 1-child, and 2-child cases."""
        removed = [False]

        def _remove(node: Optional[BinarySearchTree._Node], k: int) -> Optional[BinarySearchTree._Node]:
            if not node:
                return None
            if k < node.key:
                node.left = _remove(node.left, k)
            elif k > node.key:
                node.right = _remove(node.right, k)
            else:
                removed[0] = True
                if not node.left:
                    return node.right
                elif not node.right:
                    return node.left
                # 2 children: replace with in-order successor
                succ = node.right
                while succ.left:
                    succ = succ.left
                node.key = succ.key
                node.right = _remove(node.right, succ.key)
            return node

        self._root = _remove(self._root, key)
        if removed[0]:
            self._size -= 1
        return removed[0]

    def inorder(self) -> List[int]:
        """Returns sorted in-order traversal of keys."""
        res: List[int] = []

        def _inorder(node: Optional[BinarySearchTree._Node]):
            if not node:
                return
            _inorder(node.left)
            res.append(node.key)
            _inorder(node.right)

        _inorder(self._root)
        return res

    def min_value(self) -> Optional[int]:
        if not self._root:
            return None
        cur = self._root
        while cur.left:
            cur = cur.left
        return cur.key

    def max_value(self) -> Optional[int]:
        if not self._root:
            return None
        cur = self._root
        while cur.right:
            cur = cur.right
        return cur.key

    def successor(self, key: int) -> Optional[int]:
        cur = self._root
        succ = None
        while cur:
            if key < cur.key:
                succ = cur
                cur = cur.left
            else:
                cur = cur.right
        return succ.key if succ else None

    def predecessor(self, key: int) -> Optional[int]:
        cur = self._root
        pred = None
        while cur:
            if key > cur.key:
                pred = cur
                cur = cur.right
            else:
                cur = cur.left
        return pred.key if pred else None


# ============================================================================
# Unit Tests
# ============================================================================

class TestBinarySearchTree(unittest.TestCase):
    def test_bst_basic(self):
        bst = BinarySearchTree()
        self.assertTrue(bst.empty())
        for x in [50, 30, 70, 20, 40, 60, 80]:
            self.assertTrue(bst.insert(x))

        self.assertFalse(bst.insert(50))
        self.assertEqual(bst.size(), 7)

        self.assertTrue(bst.contains(50))
        self.assertTrue(bst.contains(20))
        self.assertFalse(bst.contains(99))

        self.assertEqual(bst.inorder(), [20, 30, 40, 50, 60, 70, 80])
        self.assertEqual(bst.min_value(), 20)
        self.assertEqual(bst.max_value(), 80)

    def test_predecessor_successor(self):
        bst = BinarySearchTree()
        for x in [50, 30, 70, 20, 40, 60, 80]:
            bst.insert(x)

        self.assertEqual(bst.successor(40), 50)
        self.assertEqual(bst.successor(50), 60)
        self.assertIsNone(bst.successor(80))

        self.assertEqual(bst.predecessor(50), 40)
        self.assertEqual(bst.predecessor(80), 70)
        self.assertIsNone(bst.predecessor(20))

    def test_deletions(self):
        bst = BinarySearchTree()
        for x in [50, 30, 70, 20, 40, 60, 80]:
            bst.insert(x)

        # Leaf (20)
        self.assertTrue(bst.remove(20))
        self.assertEqual(bst.inorder(), [30, 40, 50, 60, 70, 80])

        # 1 child (30 has 40)
        self.assertTrue(bst.remove(30))
        self.assertEqual(bst.inorder(), [40, 50, 60, 70, 80])

        # 2 children (root 50)
        self.assertTrue(bst.remove(50))
        self.assertEqual(bst.inorder(), [40, 60, 70, 80])

        self.assertFalse(bst.remove(999))


if __name__ == "__main__":
    unittest.main()
