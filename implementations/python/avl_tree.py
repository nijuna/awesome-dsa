"""
Reference Implementation: Self-Balancing AVL Tree
Demonstrates strict balance factor maintenance (BF in {-1, 0, 1}),
LL/RR/LR/RL rotations, deterministic O(log n) search/insert/delete,
and height bound guarantees.

Language: Python 3
"""

from typing import Any, Optional, List, Tuple
import unittest


class AVLNode:
    def __init__(self, key: Any, val: Any):
        self.key = key
        self.val = val
        self.height: int = 1
        self.left: Optional["AVLNode"] = None
        self.right: Optional["AVLNode"] = None


class AVLTree:
    def __init__(self):
        self._root: Optional[AVLNode] = None
        self._size: int = 0

    @staticmethod
    def _get_height(node: Optional[AVLNode]) -> int:
        return node.height if node else 0

    @staticmethod
    def _get_balance_factor(node: Optional[AVLNode]) -> int:
        return AVLTree._get_height(node.left) - AVLTree._get_height(node.right) if node else 0

    @staticmethod
    def _update_height(node: AVLNode) -> None:
        node.height = 1 + max(AVLTree._get_height(node.left), AVLTree._get_height(node.right))

    @staticmethod
    def _rotate_right(y: AVLNode) -> AVLNode:
        x = y.left
        assert x is not None
        b = x.right

        x.right = y
        y.left = b

        AVLTree._update_height(y)
        AVLTree._update_height(x)

        return x

    @staticmethod
    def _rotate_left(x: AVLNode) -> AVLNode:
        y = x.right
        assert y is not None
        b = y.left

        y.left = x
        x.right = b

        AVLTree._update_height(x)
        AVLTree._update_height(y)

        return y

    @classmethod
    def _rebalance(cls, node: AVLNode) -> AVLNode:
        cls._update_height(node)
        bf = cls._get_balance_factor(node)

        # Left heavy
        if bf > 1:
            if cls._get_balance_factor(node.left) < 0:
                node.left = cls._rotate_left(node.left)
            return cls._rotate_right(node)

        # Right heavy
        if bf < -1:
            if cls._get_balance_factor(node.right) > 0:
                node.right = cls._rotate_right(node.right)
            return cls._rotate_left(node)

        return node

    def _insert(self, node: Optional[AVLNode], key: Any, val: Any) -> Tuple[AVLNode, bool]:
        if node is None:
            return AVLNode(key, val), True

        if key < node.key:
            node.left, inserted = self._insert(node.left, key, val)
        elif key > node.key:
            node.right, inserted = self._insert(node.right, key, val)
        else:
            node.val = val
            return node, False

        return self._rebalance(node), inserted

    def insert(self, key: Any, val: Any) -> None:
        self._root, inserted = self._insert(self._root, key, val)
        if inserted:
            self._size += 1

    @staticmethod
    def _find_min(node: AVLNode) -> AVLNode:
        curr = node
        while curr.left:
            curr = curr.left
        return curr

    def _erase(self, node: Optional[AVLNode], key: Any) -> Tuple[Optional[AVLNode], bool]:
        if node is None:
            return None, False

        if key < node.key:
            node.left, erased = self._erase(node.left, key)
        elif key > node.key:
            node.right, erased = self._erase(node.right, key)
        else:
            erased = True
            if node.left is None:
                return node.right, True
            if node.right is None:
                return node.left, True

            successor = self._find_min(node.right)
            node.key = successor.key
            node.val = successor.val
            node.right, _ = self._erase(node.right, successor.key)

        return self._rebalance(node), erased

    def erase(self, key: Any) -> bool:
        self._root, erased = self._erase(self._root, key)
        if erased:
            self._size -= 1
        return erased

    def find(self, key: Any) -> Optional[Any]:
        curr = self._root
        while curr:
            if key < curr.key:
                curr = curr.left
            elif key > curr.key:
                curr = curr.right
            else:
                return curr.val
        return None

    def __contains__(self, key: Any) -> bool:
        return self.find(key) is not None

    def __len__(self) -> int:
        return self._size

    @property
    def height(self) -> int:
        return self._get_height(self._root)

    def inorder_keys(self) -> List[Any]:
        keys = []
        def _inorder(n: Optional[AVLNode]):
            if not n:
                return
            _inorder(n.left)
            keys.append(n.key)
            _inorder(n.right)
        _inorder(self._root)
        return keys

    def is_valid_avl(self) -> bool:
        def _verify(n: Optional[AVLNode]) -> bool:
            if not n:
                return True
            bf = self._get_balance_factor(n)
            if bf < -1 or bf > 1:
                return False
            expected_h = 1 + max(self._get_height(n.left), self._get_height(n.right))
            if n.height != expected_h:
                return False
            return _verify(n.left) and _verify(n.right)
        return _verify(self._root)


class TestAVLTree(unittest.TestCase):
    def test_basic_operations(self):
        tree = AVLTree()
        self.assertEqual(len(tree), 0)
        self.assertEqual(tree.height, 0)
        self.assertTrue(tree.is_valid_avl())

        # Sequential insert (RR rotation testing)
        for i in range(1, 101):
            tree.insert(i, f"val_{i}")
            self.assertTrue(tree.is_valid_avl())

        self.assertEqual(len(tree), 100)
        self.assertLessEqual(tree.height, 9)
        self.assertEqual(tree.inorder_keys(), list(range(1, 101)))

        # Lookups
        for i in range(1, 101):
            self.assertEqual(tree.find(i), f"val_{i}")
            self.assertTrue(i in tree)
        self.assertIsNone(tree.find(0))
        self.assertIsNone(tree.find(101))

        # Update existing
        tree.insert(50, "updated_50")
        self.assertEqual(len(tree), 100)
        self.assertEqual(tree.find(50), "updated_50")

        # Deletions
        self.assertTrue(tree.erase(1))  # leaf
        self.assertTrue(tree.is_valid_avl())
        self.assertEqual(len(tree), 99)
        self.assertFalse(1 in tree)

        self.assertTrue(tree.erase(50))  # two children
        self.assertTrue(tree.is_valid_avl())
        self.assertEqual(len(tree), 98)

        self.assertFalse(tree.erase(999))

        # Erase all
        for i in range(2, 101):
            if i != 50:
                self.assertTrue(tree.erase(i))
                self.assertTrue(tree.is_valid_avl())

        self.assertEqual(len(tree), 0)
        self.assertEqual(tree.height, 0)

    def test_rotations(self):
        # LR rotation
        lr_tree = AVLTree()
        lr_tree.insert(30, 30)
        lr_tree.insert(10, 10)
        lr_tree.insert(20, 20)
        self.assertTrue(lr_tree.is_valid_avl())
        self.assertEqual(lr_tree.height, 2)
        self.assertEqual(lr_tree.inorder_keys(), [10, 20, 30])

        # RL rotation
        rl_tree = AVLTree()
        rl_tree.insert(10, 10)
        rl_tree.insert(30, 30)
        rl_tree.insert(20, 20)
        self.assertTrue(rl_tree.is_valid_avl())
        self.assertEqual(rl_tree.height, 2)
        self.assertEqual(rl_tree.inorder_keys(), [10, 20, 30])


if __name__ == "__main__":
    unittest.main()
