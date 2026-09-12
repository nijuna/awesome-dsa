"""
X-Fast and Y-Fast Tries Implementation in Python 3.

Provides:
- XFastTrie: Willard's bitwise trie of height W with level hash tables,
  doubly-linked leaf list, and descendant pointers.
- YFastTrie: Space-efficient clustering using balanced micro-trees and
  an X-Fast Trie of representatives, achieving O(N) space and O(log log U) operations.
"""

import bisect
import random
import unittest
from typing import Optional, List, Dict


class XFastTrie:
    """
    X-Fast Trie over a 32-bit integer universe [0, 2^32 - 1].
    Space: O(N * W). Predecessor/Successor: O(log W).
    """
    W = 32

    class Node:
        __slots__ = ('prefix', 'depth', 'left', 'right', 'desc_min', 'desc_max', 'prev_leaf', 'next_leaf')

        def __init__(self, prefix: int, depth: int):
            self.prefix = prefix
            self.depth = depth
            self.left: Optional['XFastTrie.Node'] = None
            self.right: Optional['XFastTrie.Node'] = None
            self.prev_leaf: Optional['XFastTrie.Node'] = None
            self.next_leaf: Optional['XFastTrie.Node'] = None
            if depth == XFastTrie.W:
                self.desc_min: Optional['XFastTrie.Node'] = self
                self.desc_max: Optional['XFastTrie.Node'] = self
            else:
                self.desc_min = None
                self.desc_max = None

    def __init__(self):
        self.root = self.Node(0, 0)
        self.levels: List[Dict[int, XFastTrie.Node]] = [{} for _ in range(self.W + 1)]
        self.levels[0][0] = self.root
        self.leaf_head: Optional[XFastTrie.Node] = None
        self.leaf_tail: Optional[XFastTrie.Node] = None
        self._size = 0

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._size == 0

    def contains(self, x: int) -> bool:
        return x in self.levels[self.W]

    def insert(self, x: int) -> bool:
        if self.contains(x):
            return False

        leaf = self.Node(x, self.W)
        self.levels[self.W][x] = leaf

        # Maintain sorted linked list of leaves
        if not self.leaf_head:
            self.leaf_head = self.leaf_tail = leaf
        elif x < self.leaf_head.prefix:
            leaf.next_leaf = self.leaf_head
            self.leaf_head.prev_leaf = leaf
            self.leaf_head = leaf
        elif x > self.leaf_tail.prefix:
            leaf.prev_leaf = self.leaf_tail
            self.leaf_tail.next_leaf = leaf
            self.leaf_tail = leaf
        else:
            cur = self.leaf_head
            while cur.next_leaf and cur.next_leaf.prefix < x:
                cur = cur.next_leaf
            leaf.next_leaf = cur.next_leaf
            leaf.prev_leaf = cur
            if cur.next_leaf:
                cur.next_leaf.prev_leaf = leaf
            cur.next_leaf = leaf

        # Build path down to leaf
        path = [self.root]
        curr = self.root
        for d in range(1, self.W + 1):
            pref = x if d == self.W else (x >> (self.W - d))
            bit = (x >> (self.W - d)) & 1

            if d == self.W:
                next_node = leaf
            else:
                next_node = self.levels[d].get(pref)
                if next_node is None:
                    next_node = self.Node(pref, d)
                    self.levels[d][pref] = next_node

            if bit == 0:
                curr.left = next_node
            else:
                curr.right = next_node

            path.append(next_node)
            curr = next_node

        # Bottom-up update of desc_min and desc_max
        for i in range(self.W - 1, -1, -1):
            u = path[i]
            u.desc_min = u.left.desc_min if u.left else (u.right.desc_min if u.right else None)
            u.desc_max = u.right.desc_max if u.right else (u.left.desc_max if u.left else None)

        self._size += 1
        return True

    def erase(self, x: int) -> bool:
        if not self.contains(x):
            return False
        leaf = self.levels[self.W][x]

        if leaf.prev_leaf:
            leaf.prev_leaf.next_leaf = leaf.next_leaf
        else:
            self.leaf_head = leaf.next_leaf

        if leaf.next_leaf:
            leaf.next_leaf.prev_leaf = leaf.prev_leaf
        else:
            self.leaf_tail = leaf.prev_leaf

        del self.levels[self.W][x]

        path = [self.root]
        curr = self.root
        for d in range(1, self.W + 1):
            bit = (x >> (self.W - d)) & 1
            curr = curr.left if bit == 0 else curr.right
            path.append(curr)

        child_deleted = True
        for d in range(self.W - 1, -1, -1):
            u = path[d]
            bit = (x >> (self.W - 1 - d)) & 1

            if child_deleted:
                if bit == 0:
                    u.left = None
                else:
                    u.right = None

            if d > 0 and u.left is None and u.right is None:
                del self.levels[d][u.prefix]
                child_deleted = True
            else:
                u.desc_min = u.left.desc_min if u.left else (u.right.desc_min if u.right else None)
                u.desc_max = u.right.desc_max if u.right else (u.left.desc_max if u.left else None)
                child_deleted = False

        self._size -= 1
        return True

    def predecessor(self, x: int) -> Optional[int]:
        if self._size == 0:
            return None
        if x in self.levels[self.W]:
            return x

        low, high = 0, self.W
        lcp_depth = 0
        lcp_node = self.root

        while low <= high:
            mid = (low + high) // 2
            pref = 0 if mid == 0 else (x >> (self.W - mid))
            node = self.levels[mid].get(pref)
            if node is not None:
                lcp_depth = mid
                lcp_node = node
                low = mid + 1
            else:
                high = mid - 1

        next_bit = (x >> (self.W - 1 - lcp_depth)) & 1
        cand = lcp_node.desc_min if next_bit == 0 else lcp_node.desc_max
        if cand and cand.prefix > x:
            cand = cand.prev_leaf

        if cand and cand.prefix <= x:
            return cand.prefix
        return None

    def successor(self, x: int) -> Optional[int]:
        if self._size == 0:
            return None
        if x in self.levels[self.W]:
            return x

        low, high = 0, self.W
        lcp_depth = 0
        lcp_node = self.root

        while low <= high:
            mid = (low + high) // 2
            pref = 0 if mid == 0 else (x >> (self.W - mid))
            node = self.levels[mid].get(pref)
            if node is not None:
                lcp_depth = mid
                lcp_node = node
                low = mid + 1
            else:
                high = mid - 1

        next_bit = (x >> (self.W - 1 - lcp_depth)) & 1
        cand = lcp_node.desc_min if next_bit == 0 else lcp_node.desc_max
        if cand and cand.prefix < x:
            cand = cand.next_leaf

        if cand and cand.prefix >= x:
            return cand.prefix
        return None


class YFastTrie:
    """
    Y-Fast Trie over a 32-bit integer universe [0, 2^32 - 1].
    Space: O(N). Predecessor/Successor/Insert/Erase: O(log W) = O(log log U) amortized.
    """
    W = 32
    MAX_LEAVES = 2 * W  # 64
    MIN_LEAVES = W // 2  # 16

    class Cluster:
        __slots__ = ('items', 'rep', 'prev', 'next')

        def __init__(self):
            self.items: List[int] = []
            self.rep: int = 0
            self.prev: Optional['YFastTrie.Cluster'] = None
            self.next: Optional['YFastTrie.Cluster'] = None

    def __init__(self):
        self.rep_trie = XFastTrie()
        self.rep_to_cluster: Dict[int, YFastTrie.Cluster] = {}
        self.cluster_head: Optional[YFastTrie.Cluster] = None
        self.cluster_tail: Optional[YFastTrie.Cluster] = None
        self._size = 0

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._size == 0

    def _find_cluster(self, x: int) -> Optional[Cluster]:
        if self._size == 0:
            return None
        succ_rep = self.rep_trie.successor(x)
        if succ_rep is not None:
            return self.rep_to_cluster[succ_rep]
        return self.cluster_tail

    def contains(self, x: int) -> bool:
        c = self._find_cluster(x)
        if not c:
            return False
        idx = bisect.bisect_left(c.items, x)
        return idx < len(c.items) and c.items[idx] == x

    def insert(self, x: int) -> bool:
        if self._size == 0:
            c = self.Cluster()
            c.items.append(x)
            c.rep = x
            self.rep_trie.insert(x)
            self.rep_to_cluster[x] = c
            self.cluster_head = self.cluster_tail = c
            self._size += 1
            return True

        c = self._find_cluster(x)
        assert c is not None
        idx = bisect.bisect_left(c.items, x)
        if idx < len(c.items) and c.items[idx] == x:
            return False

        c.items.insert(idx, x)
        self._size += 1

        new_max = c.items[-1]
        if new_max != c.rep:
            del self.rep_to_cluster[c.rep]
            self.rep_trie.erase(c.rep)
            c.rep = new_max
            self.rep_trie.insert(new_max)
            self.rep_to_cluster[new_max] = c

        if len(c.items) > self.MAX_LEAVES:
            self._split_cluster(c)
        return True

    def _split_cluster(self, c: Cluster):
        n = self.Cluster()
        n.next = c.next
        n.prev = c
        if c.next:
            c.next.prev = n
        else:
            self.cluster_tail = n
        c.next = n

        half = len(c.items) // 2
        n.items = c.items[half:]
        c.items = c.items[:half]

        del self.rep_to_cluster[c.rep]
        self.rep_trie.erase(c.rep)
        c.rep = c.items[-1]
        self.rep_trie.insert(c.rep)
        self.rep_to_cluster[c.rep] = c

        n.rep = n.items[-1]
        self.rep_trie.insert(n.rep)
        self.rep_to_cluster[n.rep] = n

    def erase(self, x: int) -> bool:
        if self._size == 0:
            return False
        c = self._find_cluster(x)
        if not c:
            return False
        idx = bisect.bisect_left(c.items, x)
        if idx >= len(c.items) or c.items[idx] != x:
            return False

        c.items.pop(idx)
        self._size -= 1

        if not c.items:
            del self.rep_to_cluster[c.rep]
            self.rep_trie.erase(c.rep)
            if c.prev:
                c.prev.next = c.next
            else:
                self.cluster_head = c.next
            if c.next:
                c.next.prev = c.prev
            else:
                self.cluster_tail = c.prev
            return True

        new_max = c.items[-1]
        if new_max != c.rep:
            del self.rep_to_cluster[c.rep]
            self.rep_trie.erase(c.rep)
            c.rep = new_max
            self.rep_trie.insert(new_max)
            self.rep_to_cluster[new_max] = c

        if len(c.items) < self.MIN_LEAVES and c.next:
            n = c.next
            if len(c.items) + len(n.items) <= self.MAX_LEAVES:
                c.items.extend(n.items)
                del self.rep_to_cluster[n.rep]
                self.rep_trie.erase(n.rep)
                c.next = n.next
                if n.next:
                    n.next.prev = c
                else:
                    self.cluster_tail = c

                merged_max = c.items[-1]
                if merged_max != c.rep:
                    del self.rep_to_cluster[c.rep]
                    self.rep_trie.erase(c.rep)
                    c.rep = merged_max
                    self.rep_trie.insert(merged_max)
                    self.rep_to_cluster[merged_max] = c

        return True

    def predecessor(self, x: int) -> Optional[int]:
        if self._size == 0:
            return None
        c = self._find_cluster(x)
        if c:
            idx = bisect.bisect_right(c.items, x)
            if idx > 0:
                return c.items[idx - 1]
            if c.prev and c.prev.items:
                return c.prev.items[-1]
        return None

    def successor(self, x: int) -> Optional[int]:
        if self._size == 0:
            return None
        c = self._find_cluster(x)
        if c:
            idx = bisect.bisect_left(c.items, x)
            if idx < len(c.items):
                return c.items[idx]
            if c.next and c.next.items:
                return c.next.items[0]
        return None


class TestFastTries(unittest.TestCase):
    def test_basic_x_fast_trie(self):
        xft = XFastTrie()
        self.assertTrue(xft.is_empty())
        self.assertEqual(len(xft), 0)
        self.assertIsNone(xft.predecessor(10))
        self.assertIsNone(xft.successor(10))

        self.assertTrue(xft.insert(100))
        self.assertTrue(xft.insert(200))
        self.assertTrue(xft.insert(300))
        self.assertFalse(xft.insert(100))

        self.assertEqual(len(xft), 3)
        self.assertTrue(xft.contains(100))
        self.assertTrue(xft.contains(200))
        self.assertTrue(xft.contains(300))
        self.assertFalse(xft.contains(150))

        self.assertIsNone(xft.predecessor(50))
        self.assertEqual(xft.predecessor(100), 100)
        self.assertEqual(xft.predecessor(150), 100)
        self.assertEqual(xft.predecessor(200), 200)
        self.assertEqual(xft.predecessor(350), 300)

        self.assertEqual(xft.successor(50), 100)
        self.assertEqual(xft.successor(100), 100)
        self.assertEqual(xft.successor(150), 200)
        self.assertEqual(xft.successor(300), 300)
        self.assertIsNone(xft.successor(350))

        self.assertTrue(xft.erase(200))
        self.assertFalse(xft.contains(200))
        self.assertEqual(len(xft), 2)
        self.assertEqual(xft.successor(150), 300)
        self.assertEqual(xft.predecessor(250), 100)

    def test_basic_y_fast_trie(self):
        yft = YFastTrie()
        self.assertTrue(yft.is_empty())
        self.assertEqual(len(yft), 0)

        self.assertTrue(yft.insert(50))
        self.assertTrue(yft.insert(150))
        self.assertTrue(yft.insert(250))
        self.assertFalse(yft.insert(50))

        self.assertEqual(len(yft), 3)
        self.assertTrue(yft.contains(50))
        self.assertTrue(yft.contains(150))
        self.assertTrue(yft.contains(250))
        self.assertFalse(yft.contains(100))

        self.assertIsNone(yft.predecessor(40))
        self.assertEqual(yft.predecessor(50), 50)
        self.assertEqual(yft.predecessor(100), 50)
        self.assertEqual(yft.predecessor(150), 150)
        self.assertEqual(yft.predecessor(300), 250)

        self.assertEqual(yft.successor(10), 50)
        self.assertEqual(yft.successor(50), 50)
        self.assertEqual(yft.successor(100), 150)
        self.assertEqual(yft.successor(250), 250)
        self.assertIsNone(yft.successor(300))

        self.assertTrue(yft.erase(150))
        self.assertFalse(yft.contains(150))
        self.assertEqual(len(yft), 2)
        self.assertEqual(yft.successor(100), 250)

    def test_random_differential_verification(self):
        xft = XFastTrie()
        yft = YFastTrie()
        oracle = set()
        rng = random.Random(42)

        for _ in range(2500):
            op = rng.randint(0, 2)
            val = rng.randint(1, 50000)

            if op == 0:
                xi = xft.insert(val)
                yi = yft.insert(val)
                oi = val not in oracle
                oracle.add(val)
                self.assertEqual(xi, oi)
                self.assertEqual(yi, oi)
            elif op == 1:
                xe = xft.erase(val)
                ye = yft.erase(val)
                oe = val in oracle
                oracle.discard(val)
                self.assertEqual(xe, oe)
                self.assertEqual(ye, oe)
            else:
                self.assertEqual(len(xft), len(oracle))
                self.assertEqual(len(yft), len(oracle))

                sorted_oracle = sorted(oracle)
                for q in [val, val - 1, val + 1]:
                    idx_succ = bisect.bisect_left(sorted_oracle, q)
                    o_succ = sorted_oracle[idx_succ] if idx_succ < len(sorted_oracle) else None
                    self.assertEqual(xft.successor(q), o_succ)
                    self.assertEqual(yft.successor(q), o_succ)

                    idx_pred = bisect.bisect_right(sorted_oracle, q)
                    o_pred = sorted_oracle[idx_pred - 1] if idx_pred > 0 else None
                    self.assertEqual(xft.predecessor(q), o_pred)
                    self.assertEqual(yft.predecessor(q), o_pred)


if __name__ == '__main__':
    unittest.main()
