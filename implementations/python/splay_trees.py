"""
Reference implementation of Splay Trees with Sleator-Tarjan Splay Rotations.

Implements self-adjusting binary search trees with zig, zig-zig, and zig-zag splaying,
BST invariant verification, and differential testing against Python dict.
"""

import random
import unittest
from typing import Optional, Any


class SplayNode:
    __slots__ = ('key', 'val', 'left', 'right', 'parent')

    def __init__(self, key: Any, val: Any, parent: Optional['SplayNode'] = None):
        self.key = key
        self.val = val
        self.left: Optional['SplayNode'] = None
        self.right: Optional['SplayNode'] = None
        self.parent: Optional['SplayNode'] = parent


class SplayTree:
    """Splay Tree: Self-adjusting binary search tree with O(log N) amortized operations."""

    def __init__(self):
        self.root: Optional[SplayNode] = None
        self._size = 0

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self.root is None

    def _rotate(self, x: SplayNode) -> None:
        p = x.parent
        assert p is not None
        g = p.parent
        is_right = (p.right is x)

        if is_right:
            p.right = x.left
            if x.left:
                x.left.parent = p
            x.left = p
        else:
            p.left = x.right
            if x.right:
                x.right.parent = p
            x.right = p

        p.parent = x
        x.parent = g

        if g:
            if g.left is p:
                g.left = x
            else:
                g.right = x
        else:
            self.root = x

    def _splay(self, x: Optional[SplayNode]) -> None:
        if not x:
            return
        while x.parent:
            p = x.parent
            g = p.parent
            if not g:
                # Zig step
                self._rotate(x)
            elif (g.left is p) == (p.left is x):
                # Zig-Zig step: Rotate parent first, then node x
                self._rotate(p)
                self._rotate(x)
            else:
                # Zig-Zag step: Rotate node x twice
                self._rotate(x)
                self._rotate(x)
        self.root = x

    def _find_node(self, key: Any) -> Optional[SplayNode]:
        if not self.root:
            return None
        curr = self.root
        last = curr
        while curr:
            last = curr
            if key < curr.key:
                curr = curr.left
            elif curr.key < key:
                curr = curr.right
            else:
                self._splay(curr)
                return curr
        # Splay last visited node on miss to preserve amortized bound
        self._splay(last)
        return None

    def find(self, key: Any) -> Optional[Any]:
        """Find value for key, splaying accessed or last node to root."""
        node = self._find_node(key)
        return node.val if node else None

    def contains(self, key: Any) -> bool:
        return self._find_node(key) is not None

    def insert(self, key: Any, val: Any) -> None:
        """Insert or update key-val pair."""
        if not self.root:
            self.root = SplayNode(key, val)
            self._size = 1
            return

        found = self._find_node(key)
        if found:
            found.val = val
            return

        # root is now the node closest to key
        new_node = SplayNode(key, val)
        if key < self.root.key:
            new_node.left = self.root.left
            new_node.right = self.root
            if self.root.left:
                self.root.left.parent = new_node
            self.root.left = None
        else:
            new_node.right = self.root.right
            new_node.left = self.root
            if self.root.right:
                self.root.right.parent = new_node
            self.root.right = None

        self.root.parent = new_node
        self.root = new_node
        self._size += 1

    def erase(self, key: Any) -> bool:
        """Delete key from splay tree. Returns True if key was deleted, False if not found."""
        found = self._find_node(key)
        if not found:
            return False

        left_sub = self.root.left
        right_sub = self.root.right
        self._size -= 1

        if not left_sub:
            self.root = right_sub
            if self.root:
                self.root.parent = None
        else:
            left_sub.parent = None
            max_left = left_sub
            while max_left.right:
                max_left = max_left.right
            self.root = left_sub
            self._splay(max_left)
            self.root.right = right_sub
            if right_sub:
                right_sub.parent = self.root
        return True

    def min(self) -> Any:
        assert self.root is not None, "Tree is empty"
        curr = self.root
        while curr.left:
            curr = curr.left
        self._splay(curr)
        return self.root.key

    def max(self) -> Any:
        assert self.root is not None, "Tree is empty"
        curr = self.root
        while curr.right:
            curr = curr.right
        self._splay(curr)
        return self.root.key

    def verify_invariants(self) -> bool:
        if not self.root:
            return self._size == 0
        if self.root.parent is not None:
            return False

        def check(n: Optional[SplayNode], min_k: Any, max_k: Any) -> bool:
            if not n:
                return True
            if min_k is not None and not (min_k < n.key):
                return False
            if max_k is not None and not (n.key < max_k):
                return False
            if n.left:
                if n.left.parent is not n or not check(n.left, min_k, n.key):
                    return False
            if n.right:
                if n.right.parent is not n or not check(n.right, n.key, max_k):
                    return False
            return True

        return check(self.root, None, None)


class TestSplayTree(unittest.TestCase):
    def test_basic_operations(self):
        splay = SplayTree()
        splay.insert(50, 500)
        splay.insert(20, 200)
        splay.insert(70, 700)
        splay.insert(10, 100)
        splay.insert(30, 300)

        self.assertEqual(len(splay), 5)
        self.assertTrue(splay.verify_invariants())
        self.assertEqual(splay.min(), 10)
        self.assertEqual(splay.max(), 70)

        self.assertEqual(splay.find(30), 300)
        self.assertEqual(splay.root.key, 30)  # Splayed to root
        self.assertTrue(splay.verify_invariants())

        self.assertTrue(splay.erase(20))
        self.assertFalse(splay.contains(20))
        self.assertEqual(len(splay), 4)
        self.assertTrue(splay.verify_invariants())

    def test_differential_fuzzing(self):
        splay = SplayTree()
        oracle = {}

        rng = random.Random(1337)
        for _ in range(1000):
            op = rng.randint(0, 3)
            key = rng.randint(0, 200)

            if op == 0:
                val = key * 10
                splay.insert(key, val)
                oracle[key] = val
            elif op == 1:
                r1 = splay.erase(key)
                r2 = key in oracle
                if r2:
                    del oracle[key]
                self.assertEqual(r1, r2)
            elif op == 2:
                v1 = splay.find(key)
                v2 = oracle.get(key)
                self.assertEqual(v1, v2)
            else:
                if oracle:
                    self.assertEqual(splay.min(), min(oracle))
                    self.assertEqual(splay.max(), max(oracle))
                else:
                    self.assertTrue(splay.is_empty())

            self.assertEqual(len(splay), len(oracle))
            self.assertTrue(splay.verify_invariants())


if __name__ == '__main__':
    unittest.main()
