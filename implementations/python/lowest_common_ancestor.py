"""
Reference Implementation: Lowest Common Ancestor (LCA) via Binary Lifting
Supports O(N log N) preprocessing and O(log N) tree queries.
"""

from __future__ import annotations
import math
import unittest


class BinaryLiftingLCA:
    """Lowest Common Ancestor using Binary Lifting."""

    def __init__(self, n: int, adj: list[list[int]], root: int = 1):
        self.n = n
        self.root = root
        self.log = math.ceil(math.log2(max(n, 2))) + 1
        self.depth = [0] * (n + 1)
        self.up = [[0] * self.log for _ in range(n + 1)]
        self._build(root, adj)

    def _build(self, root: int, adj: list[list[int]]):
        stack = [(root, root, 0, False)]
        while stack:
            u, p, d, visited = stack.pop()
            if not visited:
                self.depth[u] = d
                self.up[u][0] = p
                for i in range(1, self.log):
                    self.up[u][i] = self.up[self.up[u][i - 1]][i - 1]

                stack.append((u, p, d, True))
                for v in reversed(adj[u]):
                    if v != p:
                        stack.append((v, u, d + 1, False))

    def query(self, u: int, v: int) -> int:
        """Returns the Lowest Common Ancestor of u and v in O(log N)."""
        if self.depth[u] < self.depth[v]:
            u, v = v, u

        # Step 1: Lift u to the same depth as v
        diff = self.depth[u] - self.depth[v]
        for i in range(self.log):
            if (diff >> i) & 1:
                u = self.up[u][i]

        if u == v:
            return u

        # Step 2: Jump simultaneously
        for i in range(self.log - 1, -1, -1):
            if self.up[u][i] != self.up[v][i]:
                u = self.up[u][i]
                v = self.up[v][i]

        return self.up[u][0]

    def distance(self, u: int, v: int) -> int:
        """Returns tree edge distance between u and v in O(log N)."""
        lca_node = self.query(u, v)
        return self.depth[u] + self.depth[v] - 2 * self.depth[lca_node]


class TestBinaryLiftingLCA(unittest.TestCase):
    def test_simple_tree(self):
        # Tree:
        #       1
        #      / \
        #     2   3
        #    / \   \
        #   4   5   6
        n = 6
        adj = [[] for _ in range(n + 1)]
        edges = [(1, 2), (1, 3), (2, 4), (2, 5), (3, 6)]
        for u, v in edges:
            adj[u].append(v)
            adj[v].append(u)

        lca = BinaryLiftingLCA(n, adj, root=1)
        self.assertEqual(lca.query(4, 5), 2)
        self.assertEqual(lca.query(4, 6), 1)
        self.assertEqual(lca.query(2, 4), 2)
        self.assertEqual(lca.query(1, 6), 1)
        self.assertEqual(lca.query(4, 4), 4)

        # Distance checks
        self.assertEqual(lca.distance(4, 5), 2)
        self.assertEqual(lca.distance(4, 6), 4)

    def test_line_tree(self):
        # 1 - 2 - 3 - 4 - 5
        n = 5
        adj = [[] for _ in range(n + 1)]
        for i in range(1, 5):
            adj[i].append(i + 1)
            adj[i + 1].append(i)

        lca = BinaryLiftingLCA(n, adj, root=1)
        self.assertEqual(lca.query(2, 5), 2)
        self.assertEqual(lca.query(3, 4), 3)
        self.assertEqual(lca.distance(1, 5), 4)


if __name__ == "__main__":
    unittest.main()
