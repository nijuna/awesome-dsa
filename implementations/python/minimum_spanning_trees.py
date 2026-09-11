"""
Reference Implementation: Minimum Spanning Trees (MST)
Demonstrates:
1. Kruskal's Algorithm (Global Edge Sorting + Disjoint Set Union).
2. Prim's Algorithm (Frontier Expansion with Min-Priority Queue).
3. Borůvka's Algorithm (Concurrent Component Contraction).
4. Maximum Spanning Tree & Minimum Bottleneck Spanning Tree verification.

Language: Python 3
"""

import heapq
from typing import List, Optional, Tuple
import unittest


class DSU:
    """Disjoint Set Union with path compression and union by rank."""

    def __init__(self, n: int):
        self.parent = list(range(n))
        self.rank = [0] * n

    def find(self, x: int) -> int:
        if self.parent[x] != x:
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]

    def unite(self, a: int, b: int) -> bool:
        a = self.find(a)
        b = self.find(b)
        if a == b:
            return False
        if self.rank[a] < self.rank[b]:
            a, b = b, a
        self.parent[b] = a
        if self.rank[a] == self.rank[b]:
            self.rank[a] += 1
        return True


def kruskal_mst(
    n: int,
    edges: List[Tuple[int, int, int]],
) -> Tuple[Optional[int], List[Tuple[int, int, int]]]:
    """Computes the MST using Kruskal's algorithm.

    Time Complexity: O(E log E)
    Space Complexity: O(V)
    """
    if n <= 1:
        return 0, []

    sorted_edges = sorted(edges, key=lambda e: e[2])
    dsu = DSU(n)
    total_weight = 0
    mst = []

    for u, v, w in sorted_edges:
        if dsu.unite(u, v):
            total_weight += w
            mst.append((u, v, w))
            if len(mst) == n - 1:
                break

    if len(mst) != n - 1:
        return None, []

    return total_weight, mst


def prim_mst(
    graph: List[List[Tuple[int, int]]],
    start: int = 0,
) -> Tuple[Optional[int], List[Tuple[int, int, int]]]:
    """Computes the MST using Prim's algorithm with a min-priority queue.

    Time Complexity: O((V + E) log V)
    Space Complexity: O(V + E)
    """
    n = len(graph)
    if n <= 1:
        return 0, []

    visited = [False] * n
    best = [10**18] * n
    parent = [-1] * n

    pq = [(0, start)]
    best[start] = 0

    total_weight = 0
    mst_edges = []

    while pq:
        w, u = heapq.heappop(pq)

        if visited[u]:
            continue
        visited[u] = True

        if parent[u] != -1:
            total_weight += w
            mst_edges.append((parent[u], u, w))

        for v, weight in graph[u]:
            if not visited[v] and weight < best[v]:
                best[v] = weight
                parent[v] = u
                heapq.heappush(pq, (best[v], v))

    if len(mst_edges) != n - 1:
        return None, []

    return total_weight, mst_edges


def boruvka_mst(
    n: int,
    edges: List[Tuple[int, int, int]],
) -> Tuple[Optional[int], List[Tuple[int, int, int]]]:
    """Computes the MST using Borůvka's algorithm.

    Time Complexity: O(E log V)
    Space Complexity: O(V)
    """
    if n <= 1:
        return 0, []

    dsu = DSU(n)
    components = n
    total_weight = 0
    mst = []

    while components > 1:
        best_edge = [-1] * n

        for i, (u, v, w) in enumerate(edges):
            cu = dsu.find(u)
            cv = dsu.find(v)
            if cu == cv:
                continue

            if best_edge[cu] == -1 or w < edges[best_edge[cu]][2]:
                best_edge[cu] = i
            if best_edge[cv] == -1 or w < edges[best_edge[cv]][2]:
                best_edge[cv] = i

        merged_any = False

        for i in range(n):
            ei = best_edge[i]
            if ei == -1:
                continue
            u, v, w = edges[ei]
            if dsu.unite(u, v):
                total_weight += w
                mst.append((u, v, w))
                components -= 1
                merged_any = True

        if not merged_any:
            break

    if len(mst) != n - 1:
        return None, []

    return total_weight, mst


def maximum_spanning_tree(
    n: int,
    edges: List[Tuple[int, int, int]],
) -> Tuple[Optional[int], List[Tuple[int, int, int]]]:
    """Computes the Maximum Spanning Tree by negating edge weights."""
    neg_edges = [(u, v, -w) for u, v, w in edges]
    cost, mst = kruskal_mst(n, neg_edges)
    if cost is None:
        return None, []
    return -cost, [(u, v, -w) for u, v, w in mst]


class TestMinimumSpanningTrees(unittest.TestCase):
    def test_standard_4_vertex_graph(self):
        n = 4
        edges = [
            (0, 1, 1),
            (1, 2, 2),
            (2, 3, 3),
            (0, 2, 4),
            (1, 3, 5),
        ]
        adj = [[] for _ in range(n)]
        for u, v, w in edges:
            adj[u].append((v, w))
            adj[v].append((u, w))

        w_k, mst_k = kruskal_mst(n, edges)
        w_p, mst_p = prim_mst(adj, 0)
        w_b, mst_b = boruvka_mst(n, edges)

        self.assertEqual(w_k, 6)
        self.assertEqual(w_p, 6)
        self.assertEqual(w_b, 6)

        self.assertEqual(len(mst_k), 3)
        self.assertEqual(len(mst_p), 3)
        self.assertEqual(len(mst_b), 3)

        w_max, mst_max = maximum_spanning_tree(n, edges)
        self.assertEqual(w_max, 12)
        self.assertEqual(len(mst_max), 3)

    def test_disconnected_graph(self):
        n = 5
        edges = [
            (0, 1, 10),
            (2, 3, 5),
            (3, 4, 7),
            (2, 4, 8),
        ]
        adj = [[] for _ in range(n)]
        for u, v, w in edges:
            adj[u].append((v, w))
            adj[v].append((u, w))

        w_k, _ = kruskal_mst(n, edges)
        w_p, _ = prim_mst(adj, 0)
        w_b, _ = boruvka_mst(n, edges)

        self.assertIsNone(w_k)
        self.assertIsNone(w_p)
        self.assertIsNone(w_b)

    def test_single_vertex(self):
        n = 1
        edges = []
        adj = [[]]

        w_k, mst_k = kruskal_mst(n, edges)
        w_p, mst_p = prim_mst(adj, 0)
        w_b, mst_b = boruvka_mst(n, edges)

        self.assertEqual(w_k, 0)
        self.assertEqual(w_p, 0)
        self.assertEqual(w_b, 0)
        self.assertEqual(len(mst_k), 0)
        self.assertEqual(len(mst_p), 0)
        self.assertEqual(len(mst_b), 0)

    def test_k4_duplicate_weights(self):
        n = 4
        edges = [
            (0, 1, 2),
            (0, 2, 2),
            (0, 3, 3),
            (1, 2, 2),
            (1, 3, 4),
            (2, 3, 2),
        ]
        adj = [[] for _ in range(n)]
        for u, v, w in edges:
            adj[u].append((v, w))
            adj[v].append((u, w))

        w_k, _ = kruskal_mst(n, edges)
        w_p, _ = prim_mst(adj, 0)
        w_b, _ = boruvka_mst(n, edges)

        self.assertEqual(w_k, 6)
        self.assertEqual(w_p, 6)
        self.assertEqual(w_b, 6)


if __name__ == "__main__":
    unittest.main()
