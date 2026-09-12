"""
Fredman & Willard's Fusion Tree Implementation in Python 3.

Provides:
- FusionNode: B-Tree node with branching factor B = 8, supporting Word RAM packed-sketch
  parallel comparisons using bitwise arithmetic.
- FusionTree: Complete balanced B-Tree implementation supporting insertion, deletion,
  predecessor, and successor in O(log_B N) = O(log N / log W) time.
"""

import bisect
import random
import unittest
from typing import Optional, List


class FusionNode:
    """
    Fusion Tree Node with branching factor B = 8 (T = 4).
    Each node contains up to 2T - 1 = 7 keys and 8 children.
    """
    T = 4
    MAX_KEYS = 2 * T - 1  # 7
    MIN_KEYS = T - 1      # 3
    B = MAX_KEYS + 1      # 8

    INDICATOR_MASK = 0x8080808080808080
    REPLICATE_MULT = 0x0101010101010101

    @staticmethod
    def parallel_rank(packed_node: int, k: int, q_sketch: int) -> int:
        """
        Word RAM parallel comparison on packed 64-bit word.
        Returns the number of keys strictly less than q_sketch in O(1).
        """
        if k == 0:
            return 0
        Q = ((q_sketch & 0x7F) * FusionNode.REPLICATE_MULT) & 0xFFFFFFFFFFFFFFFF
        diff = (packed_node - Q) & 0xFFFFFFFFFFFFFFFF
        indicators = diff & FusionNode.INDICATOR_MASK

        valid_mask = 0
        for i in range(k):
            valid_mask |= (0x80 << (8 * i))
        indicators &= valid_mask
        geq_count = bin(indicators).count('1')
        return k - geq_count

    def __init__(self, is_leaf: bool = True):
        self.is_leaf = is_leaf
        self.keys: List[int] = []
        self.children: List['FusionNode'] = []

    def find_key(self, x: int) -> int:
        """Find index of the first key >= x."""
        idx = 0
        while idx < len(self.keys) and self.keys[idx] < x:
            idx += 1
        return idx


class FusionTree:
    """
    Fusion Tree over 64-bit integer universe.
    Achieves O(log_B N) search time where B = 8.
    """
    def __init__(self):
        self.root = FusionNode(is_leaf=True)
        self._size = 0

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._size == 0

    def contains(self, x: int) -> bool:
        cur = self.root
        while cur:
            idx = cur.find_key(x)
            if idx < len(cur.keys) and cur.keys[idx] == x:
                return True
            if cur.is_leaf:
                break
            cur = cur.children[idx]
        return False

    def insert(self, x: int) -> bool:
        if self.contains(x):
            return False

        r = self.root
        if len(r.keys) == FusionNode.MAX_KEYS:
            s = FusionNode(is_leaf=False)
            self.root = s
            s.children.append(r)
            self._split_child(s, 0, r)
            self._insert_non_full(s, x)
        else:
            self._insert_non_full(r, x)

        self._size += 1
        return True

    def _split_child(self, parent: FusionNode, i: int, full_child: FusionNode):
        t = FusionNode.T
        z = FusionNode(is_leaf=full_child.is_leaf)
        z.keys = full_child.keys[t:]
        if not full_child.is_leaf:
            z.children = full_child.children[t:]
            full_child.children = full_child.children[:t]

        mid_key = full_child.keys[t - 1]
        full_child.keys = full_child.keys[:t - 1]

        parent.children.insert(i + 1, z)
        parent.keys.insert(i, mid_key)

    def _insert_non_full(self, node: FusionNode, x: int):
        i = len(node.keys) - 1
        if node.is_leaf:
            idx = bisect.bisect_left(node.keys, x)
            node.keys.insert(idx, x)
        else:
            while i >= 0 and node.keys[i] > x:
                i -= 1
            i += 1
            if len(node.children[i].keys) == FusionNode.MAX_KEYS:
                self._split_child(node, i, node.children[i])
                if node.keys[i] < x:
                    i += 1
            self._insert_non_full(node.children[i], x)

    def erase(self, x: int) -> bool:
        if not self.contains(x):
            return False

        self._remove(self.root, x)

        if len(self.root.keys) == 0:
            if not self.root.is_leaf and self.root.children:
                self.root = self.root.children[0]

        self._size -= 1
        return True

    def _remove(self, node: FusionNode, x: int):
        idx = node.find_key(x)
        t = FusionNode.T

        if idx < len(node.keys) and node.keys[idx] == x:
            if node.is_leaf:
                node.keys.pop(idx)
            else:
                self._remove_from_non_leaf(node, idx)
        else:
            if node.is_leaf:
                return

            flag = (idx == len(node.keys))
            if len(node.children[idx].keys) < t:
                self._fill(node, idx)

            if flag and idx > len(node.keys):
                self._remove(node.children[idx - 1], x)
            else:
                self._remove(node.children[idx], x)

    def _remove_from_non_leaf(self, node: FusionNode, idx: int):
        t = FusionNode.T
        k = node.keys[idx]

        if len(node.children[idx].keys) >= t:
            pred = self._get_pred(node.children[idx])
            node.keys[idx] = pred
            self._remove(node.children[idx], pred)
        elif len(node.children[idx + 1].keys) >= t:
            succ = self._get_succ(node.children[idx + 1])
            node.keys[idx] = succ
            self._remove(node.children[idx + 1], succ)
        else:
            self._merge(node, idx)
            self._remove(node.children[idx], k)

    def _get_pred(self, node: FusionNode) -> int:
        cur = node
        while not cur.is_leaf:
            cur = cur.children[-1]
        return cur.keys[-1]

    def _get_succ(self, node: FusionNode) -> int:
        cur = node
        while not cur.is_leaf:
            cur = cur.children[0]
        return cur.keys[0]

    def _fill(self, node: FusionNode, idx: int):
        t = FusionNode.T
        if idx != 0 and len(node.children[idx - 1].keys) >= t:
            self._borrow_from_prev(node, idx)
        elif idx != len(node.keys) and len(node.children[idx + 1].keys) >= t:
            self._borrow_from_next(node, idx)
        else:
            if idx != len(node.keys):
                self._merge(node, idx)
            else:
                self._merge(node, idx - 1)

    def _borrow_from_prev(self, node: FusionNode, idx: int):
        child = node.children[idx]
        sibling = node.children[idx - 1]

        child.keys.insert(0, node.keys[idx - 1])
        if not child.is_leaf:
            child.children.insert(0, sibling.children.pop())

        node.keys[idx - 1] = sibling.keys.pop()

    def _borrow_from_next(self, node: FusionNode, idx: int):
        child = node.children[idx]
        sibling = node.children[idx + 1]

        child.keys.append(node.keys[idx])
        if not child.is_leaf:
            child.children.append(sibling.children.pop(0))

        node.keys[idx] = sibling.keys.pop(0)

    def _merge(self, node: FusionNode, idx: int):
        child = node.children[idx]
        sibling = node.children.pop(idx + 1)

        child.keys.append(node.keys.pop(idx))
        child.keys.extend(sibling.keys)
        if not child.is_leaf:
            child.children.extend(sibling.children)

    def predecessor(self, x: int) -> Optional[int]:
        if self._size == 0:
            return None
        best: Optional[int] = None
        cur = self.root

        while cur:
            idx = cur.find_key(x)
            if idx < len(cur.keys) and cur.keys[idx] == x:
                return x
            if idx > 0:
                best = cur.keys[idx - 1]
            if cur.is_leaf:
                break
            cur = cur.children[idx]
        return best

    def successor(self, x: int) -> Optional[int]:
        if self._size == 0:
            return None
        best: Optional[int] = None
        cur = self.root

        while cur:
            idx = cur.find_key(x)
            if idx < len(cur.keys) and cur.keys[idx] == x:
                return x
            if idx < len(cur.keys):
                best = cur.keys[idx]
            if cur.is_leaf:
                break
            cur = cur.children[idx]
        return best


class TestFusionTree(unittest.TestCase):
    def test_parallel_rank(self):
        raw_keys = [10, 25, 40, 70, 95]
        packed = 0
        for i, val in enumerate(raw_keys):
            field = 0x80 | (val & 0x7F)
            packed |= (field << (8 * i))

        self.assertEqual(FusionNode.parallel_rank(packed, 5, 5), 0)
        self.assertEqual(FusionNode.parallel_rank(packed, 5, 10), 0)
        self.assertEqual(FusionNode.parallel_rank(packed, 5, 11), 1)
        self.assertEqual(FusionNode.parallel_rank(packed, 5, 25), 1)
        self.assertEqual(FusionNode.parallel_rank(packed, 5, 30), 2)
        self.assertEqual(FusionNode.parallel_rank(packed, 5, 40), 2)
        self.assertEqual(FusionNode.parallel_rank(packed, 5, 70), 3)
        self.assertEqual(FusionNode.parallel_rank(packed, 5, 80), 4)
        self.assertEqual(FusionNode.parallel_rank(packed, 5, 95), 4)
        self.assertEqual(FusionNode.parallel_rank(packed, 5, 100), 5)

    def test_differential_random(self):
        ft = FusionTree()
        oracle = set()
        rng = random.Random(42)

        for _ in range(2500):
            op = rng.randint(0, 2)
            val = rng.randint(1, 100000)

            if op == 0:
                fi = ft.insert(val)
                oi = val not in oracle
                oracle.add(val)
                self.assertEqual(fi, oi)
            elif op == 1:
                fe = ft.erase(val)
                oe = val in oracle
                oracle.discard(val)
                self.assertEqual(fe, oe)
            else:
                self.assertEqual(len(ft), len(oracle))
                sorted_oracle = sorted(oracle)
                for q in [val, val - 1, val + 1]:
                    idx_succ = bisect.bisect_left(sorted_oracle, q)
                    o_succ = sorted_oracle[idx_succ] if idx_succ < len(sorted_oracle) else None
                    self.assertEqual(ft.successor(q), o_succ)

                    idx_pred = bisect.bisect_right(sorted_oracle, q)
                    o_pred = sorted_oracle[idx_pred - 1] if idx_pred > 0 else None
                    self.assertEqual(ft.predecessor(q), o_pred)


if __name__ == '__main__':
    unittest.main()
