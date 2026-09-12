"""
Order-Statistic Tree (Augmented Balanced Binary Search Tree).

Implements find_by_order(k) (select) and order_of_key(x) (rank) in O(log n) time
using an augmented Treap data structure.
"""

import random
import unittest
from typing import Optional


class Node:
    """Treap node augmented with subtree size."""

    def __init__(self, key: int, priority: int):
        self.key = key
        self.priority = priority
        self.size = 1
        self.left: Optional["Node"] = None
        self.right: Optional["Node"] = None


class OrderStatisticTree:
    """Augmented Treap-based Order-Statistic Tree."""

    def __init__(self, seed: int = 42):
        self.root: Optional[Node] = None
        self.rng = random.Random(seed)

    @staticmethod
    def _get_size(node: Optional[Node]) -> int:
        return node.size if node else 0

    @classmethod
    def _update_size(cls, node: Optional[Node]) -> None:
        if node:
            node.size = 1 + cls._get_size(node.left) + cls._get_size(node.right)

    def _rotate_right(self, y: Node) -> Node:
        x = y.left
        assert x is not None
        y.left = x.right
        x.right = y
        self._update_size(y)
        self._update_size(x)
        return x

    def _rotate_left(self, x: Node) -> Node:
        y = x.right
        assert y is not None
        x.right = y.left
        y.left = x
        self._update_size(x)
        self._update_size(y)
        return y

    def _insert(self, node: Optional[Node], key: int) -> Node:
        if not node:
            return Node(key, self.rng.randint(1, 10**9))
        if key < node.key:
            node.left = self._insert(node.left, key)
            if node.left.priority > node.priority:
                node = self._rotate_right(node)
        elif key > node.key:
            node.right = self._insert(node.right, key)
            if node.right.priority > node.priority:
                node = self._rotate_left(node)
        self._update_size(node)
        return node

    def _erase(self, node: Optional[Node], key: int) -> Optional[Node]:
        if not node:
            return None
        if key < node.key:
            node.left = self._erase(node.left, key)
        elif key > node.key:
            node.right = self._erase(node.right, key)
        else:
            if not node.left and not node.right:
                return None
            elif not node.left:
                return node.right
            elif not node.right:
                return node.left
            else:
                if node.left.priority > node.right.priority:
                    node = self._rotate_right(node)
                    node.right = self._erase(node.right, key)
                else:
                    node = self._rotate_left(node)
                    node.left = self._erase(node.left, key)
        self._update_size(node)
        return node

    def insert(self, key: int) -> None:
        self.root = self._insert(self.root, key)

    def erase(self, key: int) -> None:
        self.root = self._erase(self.root, key)

    def size(self) -> int:
        return self._get_size(self.root)

    def find_by_order(self, k: int) -> int:
        """Returns the k-th smallest element (0-indexed) in O(log n)."""
        if k < 0 or k >= self.size():
            raise IndexError("Index k out of range")
        curr = self.root
        while curr:
            left_size = self._get_size(curr.left)
            if k == left_size:
                return curr.key
            elif k < left_size:
                curr = curr.left
            else:
                k -= left_size + 1
                curr = curr.right
        raise RuntimeError("Unreachable")

    def order_of_key(self, key: int) -> int:
        """Returns number of elements strictly smaller than key in O(log n)."""
        rank = 0
        curr = self.root
        while curr:
            if key <= curr.key:
                curr = curr.left
            else:
                rank += self._get_size(curr.left) + 1
                curr = curr.right
        return rank


class TestOrderStatisticTree(unittest.TestCase):
    def test_basic_operations(self):
        ost = OrderStatisticTree()
        for x in [10, 20, 5, 15, 30]:
            ost.insert(x)

        self.assertEqual(ost.size(), 5)
        # Sorted: 5, 10, 15, 20, 30
        self.assertEqual(ost.find_by_order(0), 5)
        self.assertEqual(ost.find_by_order(1), 10)
        self.assertEqual(ost.find_by_order(2), 15)
        self.assertEqual(ost.find_by_order(3), 20)
        self.assertEqual(ost.find_by_order(4), 30)

        self.assertEqual(ost.order_of_key(5), 0)
        self.assertEqual(ost.order_of_key(10), 1)
        self.assertEqual(ost.order_of_key(12), 2)
        self.assertEqual(ost.order_of_key(35), 5)

        ost.erase(15)
        self.assertEqual(ost.size(), 4)
        self.assertEqual(ost.find_by_order(2), 20)
        self.assertEqual(ost.order_of_key(20), 2)

    def test_stress_against_sorted_oracle(self):
        ost = OrderStatisticTree(seed=999)
        oracle = []
        rng = random.Random(123)

        for _ in range(1000):
            val = rng.randint(1, 5000)
            if val not in oracle:
                ost.insert(val)
                oracle.append(val)
                oracle.sort()

        self.assertEqual(ost.size(), len(oracle))
        for idx in range(0, len(oracle), 25):
            self.assertEqual(ost.find_by_order(idx), oracle[idx])
            self.assertEqual(ost.order_of_key(oracle[idx]), idx)


if __name__ == "__main__":
    unittest.main()
