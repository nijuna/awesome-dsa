"""
Reference implementation of van Emde Boas Trees and Cache-Oblivious Search Tree Layout.

Implements:
1. van Emde Boas Tree with O(log log U) integer operations (insert, delete, successor, predecessor).
2. Cache-Oblivious Complete Binary Search Tree using the van Emde Boas recursive layout.
3. Differential testing against sorted list / bisect oracle.
"""

import bisect
import random
import unittest
from typing import Optional, List


class VanEmdeBoasTree:
    """van Emde Boas Tree for integer universe U = 2^k.
    
    Achieves O(log log U) time per operation.
    """

    def __init__(self, bits: int):
        assert 1 <= bits <= 32
        self.bits = bits
        self.upper_bits = (bits + 1) // 2
        self.lower_bits = bits // 2
        self.lower_mask = (1 << self.lower_bits) - 1

        self.min_val: Optional[int] = None
        self.max_val: Optional[int] = None

        if bits > 1:
            self.summary: Optional[VanEmdeBoasTree] = VanEmdeBoasTree(self.upper_bits)
            num_clusters = 1 << self.upper_bits
            self.clusters: List[Optional[VanEmdeBoasTree]] = [
                VanEmdeBoasTree(self.lower_bits) for _ in range(num_clusters)
            ]
        else:
            self.summary = None
            self.clusters = []

    def is_empty(self) -> bool:
        return self.min_val is None

    def _high(self, x: int) -> int:
        return x >> self.lower_bits

    def _low(self, x: int) -> int:
        return x & self.lower_mask

    def _index(self, c: int, i: int) -> int:
        return (c << self.lower_bits) | i

    def min(self) -> Optional[int]:
        return self.min_val

    def max(self) -> Optional[int]:
        return self.max_val

    def contains(self, x: int) -> bool:
        if self.is_empty() or x < self.min_val or x > self.max_val:
            return False
        if x == self.min_val or x == self.max_val:
            return True
        if self.bits == 1:
            return False
        return self.clusters[self._high(x)].contains(self._low(x))

    def insert(self, x: int) -> None:
        if self.is_empty():
            self.min_val = self.max_val = x
            return

        if x == self.min_val or x == self.max_val:
            return

        if x < self.min_val:
            x, self.min_val = self.min_val, x

        if self.bits > 1:
            c = self._high(x)
            i = self._low(x)
            if self.clusters[c].is_empty():
                self.summary.insert(c)
                self.clusters[c].insert(i)
            else:
                self.clusters[c].insert(i)

        if x > self.max_val:
            self.max_val = x

    def successor(self, x: int) -> Optional[int]:
        if self.is_empty():
            return None
        if self.bits == 1:
            if x == 0 and self.max_val == 1:
                return 1
            return None

        if x < self.min_val:
            return self.min_val

        c = self._high(x)
        i = self._low(x)
        max_in_c = self.clusters[c].max()

        if max_in_c is not None and i < max_in_c:
            offset = self.clusters[c].successor(i)
            if offset is not None:
                return self._index(c, offset)

        succ_c = self.summary.successor(c)
        if succ_c is None:
            return None

        offset = self.clusters[succ_c].min()
        return self._index(succ_c, offset)

    def predecessor(self, x: int) -> Optional[int]:
        if self.is_empty():
            return None
        if self.bits == 1:
            if x == 1 and self.min_val == 0:
                return 0
            return None

        if x > self.max_val:
            return self.max_val

        c = self._high(x)
        i = self._low(x)
        min_in_c = self.clusters[c].min()

        if min_in_c is not None and i > min_in_c:
            offset = self.clusters[c].predecessor(i)
            if offset is not None:
                return self._index(c, offset)

        pred_c = self.summary.predecessor(c)
        if pred_c is not None:
            offset = self.clusters[pred_c].max()
            return self._index(pred_c, offset)

        if x > self.min_val:
            return self.min_val

        return None

    def erase(self, x: int) -> None:
        if self.is_empty() or x < self.min_val or x > self.max_val:
            return

        if self.min_val == self.max_val:
            if x == self.min_val:
                self.min_val = self.max_val = None
            return

        if self.bits == 1:
            if x == 0:
                self.min_val = 1
            else:
                self.min_val = 0
            self.max_val = self.min_val
            return

        if x == self.min_val:
            first_c = self.summary.min()
            x = self._index(first_c, self.clusters[first_c].min())
            self.min_val = x

        c = self._high(x)
        i = self._low(x)
        self.clusters[c].erase(i)

        if self.clusters[c].is_empty():
            self.summary.erase(c)
            if x == self.max_val:
                if self.summary.is_empty():
                    self.max_val = self.min_val
                else:
                    last_c = self.summary.max()
                    self.max_val = self._index(last_c, self.clusters[last_c].max())
        elif x == self.max_val:
            self.max_val = self._index(c, self.clusters[c].max())


class CacheObliviousSearchTree:
    """Cache-Oblivious Search Tree using van Emde Boas recursive layout."""

    def __init__(self, height: int, sorted_keys: List[int]):
        self.height = height
        self.n = (1 << height) - 1
        assert len(sorted_keys) == self.n

        self.layout = [0] * self.n
        self.bst_to_veb = [0] * (1 << height)
        self.veb_to_bst = [0] * self.n

        self._current_pos = 0
        self._build_layout(1, height)
        assert self._current_pos == self.n

        # In-order mapping of complete BST
        bst_nodes = [0] * (1 << height)
        key_idx = 0

        def inorder(u: int):
            nonlocal key_idx
            if u > self.n:
                return
            inorder(2 * u)
            bst_nodes[u] = sorted_keys[key_idx]
            key_idx += 1
            inorder(2 * u + 1)

        inorder(1)
        assert key_idx == self.n

        for i in range(self.n):
            bst_idx = self.veb_to_bst[i]
            self.layout[i] = bst_nodes[bst_idx]

    def _build_layout(self, bst_idx: int, h: int) -> None:
        if h == 1:
            self.bst_to_veb[bst_idx] = self._current_pos
            self.veb_to_bst[self._current_pos] = bst_idx
            self._current_pos += 1
            return

        h_top = h // 2
        h_bot = h - h_top

        self._build_layout(bst_idx, h_top)

        num_bottom = 1 << h_top
        for i in range(num_bottom):
            bot_root = (bst_idx << h_top) + i
            self._build_layout(bot_root, h_bot)

    def search(self, key: int) -> bool:
        u = 1
        while u <= self.n:
            veb_pos = self.bst_to_veb[u]
            val = self.layout[veb_pos]
            if key == val:
                return True
            elif key < val:
                u = 2 * u
            else:
                u = 2 * u + 1
        return False


class TestVanEmdeBoasAndLayout(unittest.TestCase):
    def test_veb_differential(self):
        bits = 8
        veb = VanEmdeBoasTree(bits)
        oracle = []

        rng = random.Random(42)
        for _ in range(1000):
            op = rng.randint(0, 3)
            x = rng.randint(0, (1 << bits) - 1)

            if op == 0:
                # Insert
                veb.insert(x)
                if x not in oracle:
                    bisect.insort(oracle, x)
            elif op == 1:
                # Erase
                veb.erase(x)
                if x in oracle:
                    oracle.remove(x)
            elif op == 2:
                # Successor
                res = veb.successor(x)
                idx = bisect.bisect_right(oracle, x)
                expected = oracle[idx] if idx < len(oracle) else None
                self.assertEqual(res, expected)
            else:
                # Predecessor
                res = veb.predecessor(x)
                idx = bisect.bisect_left(oracle, x)
                expected = oracle[idx - 1] if idx > 0 else None
                self.assertEqual(res, expected)

            if not oracle:
                self.assertTrue(veb.is_empty())
            else:
                self.assertEqual(veb.min(), oracle[0])
                self.assertEqual(veb.max(), oracle[-1])

    def test_cache_oblivious_layout(self):
        height = 4
        n_keys = (1 << height) - 1
        keys = [(i + 1) * 10 for i in range(n_keys)]
        tree = CacheObliviousSearchTree(height, keys)

        for k in keys:
            self.assertTrue(tree.search(k))

        for absent in [0, 5, 15, 25, 95, 155, 200]:
            self.assertFalse(tree.search(absent))


if __name__ == '__main__':
    unittest.main()
