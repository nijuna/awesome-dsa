"""
Reference implementations of Scapegoat Trees and AA Trees.

Implements:
1. ScapegoatTree with alpha=2/3 weight balance, logarithmic height bound, and local rebuilding.
2. AATree with Andersson's skew and split operations eliminating Red-Black complexity.
3. Differential testing against Python dict.
"""

import math
import random
import unittest
from typing import Optional, Any, List


class ScapegoatNode:
    __slots__ = ('key', 'val', 'left', 'right')

    def __init__(self, key: Any, val: Any):
        self.key = key
        self.val = val
        self.left: Optional['ScapegoatNode'] = None
        self.right: Optional['ScapegoatNode'] = None


class ScapegoatTree:
    """Scapegoat Tree with alpha=2/3 weight balance and O(log N) amortized operations."""

    def __init__(self):
        self.root: Optional[ScapegoatNode] = None
        self.n = 0
        self.q = 0

    def __len__(self) -> int:
        return self.n

    def is_empty(self) -> bool:
        return self.n == 0

    @staticmethod
    def _size(u: Optional[ScapegoatNode]) -> int:
        if not u:
            return 0
        return 1 + ScapegoatTree._size(u.left) + ScapegoatTree._size(u.right)

    @staticmethod
    def _max_height(n: int) -> int:
        if n == 0:
            return 0
        return int(math.floor(math.log(n) / math.log(1.5)))

    def _flatten(self, u: Optional[ScapegoatNode], nodes: List[ScapegoatNode]) -> None:
        if not u:
            return
        self._flatten(u.left, nodes)
        nodes.append(u)
        self._flatten(u.right, nodes)

    def _build_balanced(self, nodes: List[ScapegoatNode], l: int, r: int) -> Optional[ScapegoatNode]:
        if l > r:
            return None
        mid = (l + r) // 2
        mid_node = nodes[mid]
        mid_node.left = self._build_balanced(nodes, l, mid - 1)
        mid_node.right = self._build_balanced(nodes, mid + 1, r)
        return mid_node

    def _rebuild(self, u: Optional[ScapegoatNode]) -> Optional[ScapegoatNode]:
        if not u:
            return None
        nodes: List[ScapegoatNode] = []
        self._flatten(u, nodes)
        return self._build_balanced(nodes, 0, len(nodes) - 1)

    def find(self, key: Any) -> Optional[Any]:
        curr = self.root
        while curr:
            if key < curr.key:
                curr = curr.left
            elif curr.key < key:
                curr = curr.right
            else:
                return curr.val
        return None

    def contains(self, key: Any) -> bool:
        return self.find(key) is not None

    def insert(self, key: Any, val: Any) -> None:
        if not self.root:
            self.root = ScapegoatNode(key, val)
            self.n = 1
            self.q = 1
            return

        path: List[ScapegoatNode] = []
        curr = self.root
        depth = 0

        while curr:
            path.append(curr)
            depth += 1
            if key < curr.key:
                if not curr.left:
                    curr.left = ScapegoatNode(key, val)
                    break
                curr = curr.left
            elif curr.key < key:
                if not curr.right:
                    curr.right = ScapegoatNode(key, val)
                    break
                curr = curr.right
            else:
                curr.val = val
                return

        self.n += 1
        self.q = max(self.q, self.n)

        if depth > self._max_height(self.n):
            # Find scapegoat on path
            sg_idx = -1
            for i in range(len(path) - 1, -1, -1):
                p = path[i]
                p_sz = self._size(p)
                left_sz = self._size(p.left)
                right_sz = self._size(p.right)
                if 3 * left_sz > 2 * p_sz or 3 * right_sz > 2 * p_sz:
                    sg_idx = i
                    break

            if sg_idx != -1:
                sg_node = path[sg_idx]
                rebuilt = self._rebuild(sg_node)
                if sg_idx == 0:
                    self.root = rebuilt
                else:
                    parent = path[sg_idx - 1]
                    if parent.left is sg_node:
                        parent.left = rebuilt
                    else:
                        parent.right = rebuilt
            else:
                self.root = self._rebuild(self.root)

    def erase(self, key: Any) -> bool:
        parent = None
        curr = self.root
        is_left = False

        while curr:
            if key < curr.key:
                parent = curr
                curr = curr.left
                is_left = True
            elif curr.key < key:
                parent = curr
                curr = curr.right
                is_left = False
            else:
                break

        if not curr:
            return False

        # Node found: delete
        if not curr.left:
            replacement = curr.right
        elif not curr.right:
            replacement = curr.left
        else:
            succ_parent = curr
            succ = curr.right
            while succ.left:
                succ_parent = succ
                succ = succ.left

            if succ_parent is not curr:
                succ_parent.left = succ.right
                succ.right = curr.right
            succ.left = curr.left
            replacement = succ

        if not parent:
            self.root = replacement
        elif is_left:
            parent.left = replacement
        else:
            parent.right = replacement

        self.n -= 1
        if 3 * self.n < 2 * self.q:
            self.root = self._rebuild(self.root)
            self.q = self.n
        return True

    def min(self) -> Any:
        assert self.root is not None, "Tree is empty"
        curr = self.root
        while curr.left:
            curr = curr.left
        return curr.key

    def max(self) -> Any:
        assert self.root is not None, "Tree is empty"
        curr = self.root
        while curr.right:
            curr = curr.right
        return curr.key

    def verify_invariants(self) -> bool:
        if not self.root:
            return self.n == 0

        def check(u: Optional[ScapegoatNode], min_k: Any, max_k: Any) -> bool:
            if not u:
                return True
            if min_k is not None and not (min_k < u.key):
                return False
            if max_k is not None and not (u.key < max_k):
                return False
            return check(u.left, min_k, u.key) and check(u.right, u.key, max_k)

        return check(self.root, None, None)


class AANode:
    __slots__ = ('key', 'val', 'level', 'left', 'right')

    def __init__(self, key: Any, val: Any, level: int = 1):
        self.key = key
        self.val = val
        self.level = level
        self.left: Optional['AANode'] = None
        self.right: Optional['AANode'] = None


class AATree:
    """AA Tree (Andersson 1993): Balanced BST using skew and split primitives."""

    def __init__(self):
        self.root: Optional[AANode] = None
        self._size = 0

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self.root is None

    @staticmethod
    def _skew(t: Optional[AANode]) -> Optional[AANode]:
        if not t or not t.left:
            return t
        if t.left.level == t.level:
            l = t.left
            t.left = l.right
            l.right = t
            return l
        return t

    @staticmethod
    def _split(t: Optional[AANode]) -> Optional[AANode]:
        if not t or not t.right or not t.right.right:
            return t
        if t.level == t.right.right.level:
            r = t.right
            t.right = r.left
            r.left = t
            r.level += 1
            return r
        return t

    def find(self, key: Any) -> Optional[Any]:
        curr = self.root
        while curr:
            if key < curr.key:
                curr = curr.left
            elif curr.key < key:
                curr = curr.right
            else:
                return curr.val
        return None

    def contains(self, key: Any) -> bool:
        return self.find(key) is not None

    def insert(self, key: Any, val: Any) -> None:
        inserted = False

        def _insert(t: Optional[AANode]) -> AANode:
            nonlocal inserted
            if not t:
                inserted = True
                return AANode(key, val, 1)
            if key < t.key:
                t.left = _insert(t.left)
            elif curr_key := t.key:
                if curr_key < key:
                    t.right = _insert(t.right)
                else:
                    t.val = val
                    return t
            t = self._skew(t)
            t = self._split(t)
            return t

        self.root = _insert(self.root)
        if inserted:
            self._size += 1

    def erase(self, key: Any) -> bool:
        erased = False

        def _erase(t: Optional[AANode], target_k: Any) -> Optional[AANode]:
            nonlocal erased
            if not t:
                return None
            if target_k < t.key:
                t.left = _erase(t.left, target_k)
            elif t.key < target_k:
                t.right = _erase(t.right, target_k)
            else:
                erased = True
                if not t.left and not t.right:
                    return None
                elif not t.left:
                    succ = t.right
                    while succ.left:
                        succ = succ.left
                    t.key, t.val = succ.key, succ.val
                    t.right = _erase(t.right, succ.key)
                else:
                    pred = t.left
                    while pred.right:
                        pred = pred.right
                    t.key, t.val = pred.key, pred.val
                    t.left = _erase(t.left, pred.key)

            def lvl(n: Optional[AANode]) -> int:
                return n.level if n else 0

            should_be = min(lvl(t.left), lvl(t.right)) + 1
            if should_be < t.level:
                t.level = should_be
                if t.right and t.right.level > should_be:
                    t.right.level = should_be

            t = self._skew(t)
            if t.right:
                t.right = self._skew(t.right)
                if t.right.right:
                    t.right.right = self._skew(t.right.right)
            t = self._split(t)
            if t.right:
                t.right = self._split(t.right)
            return t

        self.root = _erase(self.root, key)
        if erased:
            self._size -= 1
        return erased

    def min(self) -> Any:
        assert self.root is not None, "Tree is empty"
        curr = self.root
        while curr.left:
            curr = curr.left
        return curr.key

    def max(self) -> Any:
        assert self.root is not None, "Tree is empty"
        curr = self.root
        while curr.right:
            curr = curr.right
        return curr.key

    def verify_invariants(self) -> bool:
        if not self.root:
            return self._size == 0

        def check(t: Optional[AANode], min_k: Any, max_k: Any) -> bool:
            if not t:
                return True
            if min_k is not None and not (min_k < t.key):
                return False
            if max_k is not None and not (t.key < max_k):
                return False
            if not t.left and not t.right and t.level != 1:
                return False
            if t.left and t.left.level != t.level - 1:
                return False
            if t.right and not (t.right.level == t.level or t.right.level == t.level - 1):
                return False
            if t.right and t.right.right and t.right.right.level >= t.level:
                return False
            return check(t.left, min_k, t.key) and check(t.right, t.key, max_k)

        return check(self.root, None, None)


class TestScapegoatAndAATrees(unittest.TestCase):
    def test_basic_operations(self):
        sg = ScapegoatTree()
        aa = AATree()

        for k in [50, 20, 70, 10, 30]:
            sg.insert(k, k * 10)
            aa.insert(k, k * 10)

        self.assertEqual(len(sg), 5)
        self.assertEqual(len(aa), 5)
        self.assertTrue(sg.verify_invariants())
        self.assertTrue(aa.verify_invariants())

        self.assertEqual(sg.min(), 10)
        self.assertEqual(aa.min(), 10)
        self.assertEqual(sg.max(), 70)
        self.assertEqual(aa.max(), 70)

        self.assertTrue(sg.erase(20))
        self.assertTrue(aa.erase(20))
        self.assertFalse(sg.contains(20))
        self.assertFalse(aa.contains(20))
        self.assertEqual(len(sg), 4)
        self.assertEqual(len(aa), 4)

    def test_differential_fuzzing(self):
        sg = ScapegoatTree()
        aa = AATree()
        oracle = {}

        rng = random.Random(42)
        for _ in range(1000):
            op = rng.randint(0, 3)
            key = rng.randint(0, 250)

            if op == 0:
                val = key * 7
                sg.insert(key, val)
                aa.insert(key, val)
                oracle[key] = val
            elif op == 1:
                r1 = sg.erase(key)
                r2 = aa.erase(key)
                r3 = key in oracle
                if r3:
                    del oracle[key]
                self.assertEqual(r1, r3)
                self.assertEqual(r2, r3)
            elif op == 2:
                v1 = sg.find(key)
                v2 = aa.find(key)
                v3 = oracle.get(key)
                self.assertEqual(v1, v3)
                self.assertEqual(v2, v3)
            else:
                if oracle:
                    self.assertEqual(sg.min(), min(oracle))
                    self.assertEqual(aa.min(), min(oracle))
                    self.assertEqual(sg.max(), max(oracle))
                    self.assertEqual(aa.max(), max(oracle))
                else:
                    self.assertTrue(sg.is_empty())
                    self.assertTrue(aa.is_empty())

            self.assertEqual(len(sg), len(oracle))
            self.assertEqual(len(aa), len(oracle))
            self.assertTrue(sg.verify_invariants())
            self.assertTrue(aa.verify_invariants())


if __name__ == '__main__':
    unittest.main()
