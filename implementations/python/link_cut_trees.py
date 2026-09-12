"""
Reference implementation of Link-Cut Trees with Path XOR Aggregates.

Implements Sleator & Tarjan's Link-Cut Tree using preferred path decomposition
with auxiliary splay trees, lazy path reversal, dynamic link/cut, and differential
verification against an independent tree oracle.
"""

import random
import unittest


class LctNode:
    __slots__ = ('ch', 'p', 'val', 'agg', 'rev')

    def __init__(self):
        self.ch = [0, 0]
        self.p = 0
        self.val = 0
        self.agg = 0
        self.rev = False


class LinkCutTree:
    """Link-Cut Tree representing dynamic forests of rooted trees.
    
    Supports link, cut, make_root, and path queries in amortized O(log N) time.
    """

    def __init__(self, n: int):
        self.n = n
        self.tree = [LctNode() for _ in range(n + 1)]

    def _is_root(self, x: int) -> bool:
        p = self.tree[x].p
        return self.tree[p].ch[0] != x and self.tree[p].ch[1] != x

    def _push_up(self, x: int) -> None:
        if x == 0:
            return
        left_agg = self.tree[self.tree[x].ch[0]].agg
        right_agg = self.tree[self.tree[x].ch[1]].agg
        self.tree[x].agg = left_agg ^ self.tree[x].val ^ right_agg

    def _push_down(self, x: int) -> None:
        if x == 0 or not self.tree[x].rev:
            return
        self.tree[x].ch[0], self.tree[x].ch[1] = self.tree[x].ch[1], self.tree[x].ch[0]
        c0, c1 = self.tree[x].ch[0], self.tree[x].ch[1]
        if c0 != 0:
            self.tree[c0].rev ^= True
        if c1 != 0:
            self.tree[c1].rev ^= True
        self.tree[x].rev = False

    def _rotate(self, x: int) -> None:
        y = self.tree[x].p
        z = self.tree[y].p
        k = 1 if self.tree[y].ch[1] == x else 0

        if not self._is_root(y):
            self.tree[z].ch[1 if self.tree[z].ch[1] == y else 0] = x
        self.tree[x].p = z

        self.tree[y].ch[k] = self.tree[x].ch[k ^ 1]
        if self.tree[x].ch[k ^ 1] != 0:
            self.tree[self.tree[x].ch[k ^ 1]].p = y

        self.tree[x].ch[k ^ 1] = y
        self.tree[y].p = x

        self._push_up(y)
        self._push_up(x)

    def _splay(self, x: int) -> None:
        stk = []
        curr = x
        stk.append(curr)
        while not self._is_root(curr):
            curr = self.tree[curr].p
            stk.append(curr)

        while stk:
            self._push_down(stk.pop())

        while not self._is_root(x):
            y = self.tree[x].p
            z = self.tree[y].p
            if not self._is_root(y):
                if (self.tree[y].ch[1] == x) ^ (self.tree[z].ch[1] == y):
                    self._rotate(x)
                else:
                    self._rotate(y)
            self._rotate(x)

    def set_val(self, x: int, val: int) -> None:
        assert 1 <= x <= self.n
        self._splay(x)
        self.tree[x].val = val
        self._push_up(x)

    def get_val(self, x: int) -> int:
        assert 1 <= x <= self.n
        self._splay(x)
        return self.tree[x].val

    def access(self, x: int) -> None:
        """Expose the path from represented root to node x into a single preferred path."""
        assert 1 <= x <= self.n
        t = 0
        while x != 0:
            self._splay(x)
            self.tree[x].ch[1] = t
            self._push_up(x)
            t = x
            x = self.tree[x].p

    def make_root(self, x: int) -> None:
        """Make node x the root of its represented tree."""
        assert 1 <= x <= self.n
        self.access(x)
        self._splay(x)
        self.tree[x].rev ^= True

    def find_root(self, x: int) -> int:
        """Find the root of the represented tree containing node x."""
        assert 1 <= x <= self.n
        self.access(x)
        self._splay(x)
        while self.tree[x].ch[0] != 0:
            self._push_down(x)
            x = self.tree[x].ch[0]
        self._splay(x)
        return x

    # --- Layer A: Fast Core API (Preconditioned) ---

    def link(self, x: int, y: int) -> None:
        """Connect tree of x to node y. Assumes x and y are in different trees."""
        assert 1 <= x <= self.n and 1 <= y <= self.n
        self.make_root(x)
        assert self.find_root(y) != x, "Cannot link nodes in the same tree (creates cycle)"
        self.tree[x].p = y

    def cut(self, x: int, y: int) -> None:
        """Sever the edge between adjacent nodes x and y."""
        assert 1 <= x <= self.n and 1 <= y <= self.n
        self.make_root(x)
        self.access(y)
        self._splay(y)
        assert self.tree[y].ch[0] == x and self.tree[x].ch[1] == 0, "Edge does not exist"
        self.tree[y].ch[0] = 0
        self.tree[x].p = 0
        self._push_up(y)

    def query_path(self, x: int, y: int) -> int:
        """Compute aggregate (XOR sum) along path between x and y."""
        assert 1 <= x <= self.n and 1 <= y <= self.n
        self.make_root(x)
        self.access(y)
        self._splay(y)
        return self.tree[y].agg

    # --- Layer B: Safe Adapter API ---

    def is_connected(self, x: int, y: int) -> bool:
        """Check if x and y belong to the same represented tree."""
        assert 1 <= x <= self.n and 1 <= y <= self.n
        if x == y:
            return True
        self.make_root(x)
        return self.find_root(y) == x

    def try_link(self, x: int, y: int) -> bool:
        """Safe link: returns True if link succeeded, False if already connected."""
        assert 1 <= x <= self.n and 1 <= y <= self.n
        self.make_root(x)
        if self.find_root(y) == x:
            return False
        self.tree[x].p = y
        return True

    def try_cut(self, x: int, y: int) -> bool:
        """Safe cut: returns True if cut succeeded, False if edge does not exist."""
        assert 1 <= x <= self.n and 1 <= y <= self.n
        self.make_root(x)
        if self.find_root(y) != x:
            return False
        self.access(y)
        self._splay(y)
        if self.tree[y].ch[0] != x or self.tree[x].ch[1] != 0:
            return False
        self.tree[y].ch[0] = 0
        self.tree[x].p = 0
        self._push_up(y)
        return True


class NaiveTreeOracle:
    """Naive Tree Oracle for differential verification."""

    def __init__(self, n: int):
        self.n = n
        self.val = [0] * (n + 1)
        self.adj = [[] for _ in range(n + 1)]

    def set_val(self, x: int, v: int) -> None:
        self.val[x] = v

    def _dfs_path(self, u: int, target: int, parent: int, path: list) -> bool:
        path.append(u)
        if u == target:
            return True
        for v in self.adj[u]:
            if v != parent:
                if self._dfs_path(v, target, u, path):
                    return True
        path.pop()
        return False

    def is_connected(self, u: int, v: int) -> bool:
        path = []
        return self._dfs_path(u, v, 0, path)

    def link(self, u: int, v: int) -> bool:
        if self.is_connected(u, v):
            return False
        self.adj[u].append(v)
        self.adj[v].append(u)
        return True

    def cut(self, u: int, v: int) -> bool:
        if v not in self.adj[u] or u not in self.adj[v]:
            return False
        self.adj[u].remove(v)
        self.adj[v].remove(u)
        return True

    def query_path(self, u: int, v: int) -> int:
        path = []
        found = self._dfs_path(u, v, 0, path)
        assert found
        res = 0
        for node in path:
            res ^= self.val[node]
        return res


class TestLinkCutTree(unittest.TestCase):
    def test_basic_link_cut_and_query(self):
        n = 10
        lct = LinkCutTree(n)
        oracle = NaiveTreeOracle(n)

        for i in range(1, n + 1):
            val = i * 23 + 7
            lct.set_val(i, val)
            oracle.set_val(i, val)

        self.assertTrue(lct.try_link(1, 2))
        self.assertTrue(oracle.link(1, 2))

        self.assertTrue(lct.try_link(2, 3))
        self.assertTrue(oracle.link(2, 3))

        self.assertTrue(lct.try_link(3, 4))
        self.assertTrue(oracle.link(3, 4))

        # Cycle rejection
        self.assertFalse(lct.try_link(1, 4))
        self.assertFalse(oracle.link(1, 4))

        # Connectivity
        self.assertTrue(lct.is_connected(1, 4))
        self.assertTrue(oracle.is_connected(1, 4))
        self.assertFalse(lct.is_connected(1, 5))
        self.assertFalse(oracle.is_connected(1, 5))

        # Path XOR query
        self.assertEqual(lct.query_path(1, 4), oracle.query_path(1, 4))

        # Dynamic cut
        self.assertTrue(lct.try_cut(2, 3))
        self.assertTrue(oracle.cut(2, 3))
        self.assertFalse(lct.is_connected(1, 4))
        self.assertFalse(oracle.is_connected(1, 4))
        self.assertTrue(lct.is_connected(1, 2))
        self.assertTrue(oracle.is_connected(1, 2))
        self.assertTrue(lct.is_connected(3, 4))
        self.assertTrue(oracle.is_connected(3, 4))

    def test_randomized_differential(self):
        rng = random.Random(42)
        n = 15
        lct = LinkCutTree(n)
        oracle = NaiveTreeOracle(n)

        for i in range(1, n + 1):
            val = rng.randint(0, 1000)
            lct.set_val(i, val)
            oracle.set_val(i, val)

        for _ in range(300):
            op = rng.randint(0, 3)
            u = rng.randint(1, n)
            v = rng.randint(1, n)

            if op == 0:
                # Link
                r1 = lct.try_link(u, v)
                r2 = oracle.link(u, v)
                self.assertEqual(r1, r2)
            elif op == 1:
                # Cut
                r1 = lct.try_cut(u, v)
                r2 = oracle.cut(u, v)
                self.assertEqual(r1, r2)
            elif op == 2:
                # Query path
                conn1 = lct.is_connected(u, v)
                conn2 = oracle.is_connected(u, v)
                self.assertEqual(conn1, conn2)
                if conn1:
                    q1 = lct.query_path(u, v)
                    q2 = oracle.query_path(u, v)
                    self.assertEqual(q1, q2)
            else:
                # Update
                new_val = rng.randint(0, 10000)
                lct.set_val(u, new_val)
                oracle.set_val(u, new_val)


if __name__ == '__main__':
    unittest.main()
