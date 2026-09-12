"""
Cut and Cycle Properties in Graphs and Spanning Trees in Python.

Implements Kruskal's, Prim's, and Reverse-Delete algorithms and asserts
that all algorithms arrive at the exact same optimal spanning tree weight.
"""

import heapq
import unittest
from typing import List, NamedTuple, Tuple


class Edge(NamedTuple):
    u: int
    v: int
    weight: int


class DSU:
    def __init__(self, n: int):
        self.parent = list(range(n))
        self.rank = [0] * n

    def find(self, i: int) -> int:
        if self.parent[i] == i:
            return i
        self.parent[i] = self.find(self.parent[i])
        return self.parent[i]

    def unite(self, i: int, j: int) -> bool:
        root_i = self.find(i)
        root_j = self.find(j)
        if root_i == root_j:
            return False
        if self.rank[root_i] < self.rank[root_j]:
            self.parent[root_i] = root_j
        elif self.rank[root_i] > self.rank[root_j]:
            self.parent[root_j] = root_i
        else:
            self.parent[root_j] = root_i
            self.rank[root_i] += 1
        return True


def kruskal_mst(num_vertices: int, edges: List[Edge]) -> int:
    sorted_edges = sorted(edges, key=lambda e: e.weight)
    dsu = DSU(num_vertices)
    total = 0
    count = 0
    for e in sorted_edges:
        if dsu.unite(e.u, e.v):
            total += e.weight
            count += 1
            if count == num_vertices - 1:
                break
    return total


def prim_mst(num_vertices: int, edges: List[Edge]) -> int:
    adj = [[] for _ in range(num_vertices)]
    for e in edges:
        adj[e.u].append((e.v, e.weight))
        adj[e.v].append((e.u, e.weight))

    visited = [False] * num_vertices
    pq: List[Tuple[int, int]] = [(0, 0)]  # (weight, u)
    total = 0
    visited_count = 0

    while pq and visited_count < num_vertices:
        w, u = heapq.heappop(pq)
        if visited[u]:
            continue
        visited[u] = True
        total += w
        visited_count += 1

        for v, weight in adj[u]:
            if not visited[v]:
                heapq.heappush(pq, (weight, v))

    return total


def reverse_delete_mst(num_vertices: int, edges: List[Edge]) -> int:
    sorted_edges = sorted(edges, key=lambda e: e.weight, reverse=True)
    active = [True] * len(sorted_edges)

    def is_connected(active_mask: List[bool]) -> bool:
        dsu = DSU(num_vertices)
        comps = num_vertices
        for idx, act in enumerate(active_mask):
            if act:
                if dsu.unite(sorted_edges[idx].u, sorted_edges[idx].v):
                    comps -= 1
        return comps == 1

    for i in range(len(sorted_edges)):
        active[i] = False
        if not is_connected(active):
            active[i] = True  # Restore bridge

    return sum(sorted_edges[i].weight for i in range(len(sorted_edges)) if active[i])


class TestCutAndCycleProperties(unittest.TestCase):
    def test_mst_equivalence(self):
        v = 6
        edges = [
            Edge(0, 1, 4),
            Edge(0, 2, 2),
            Edge(1, 2, 1),
            Edge(1, 3, 5),
            Edge(2, 3, 8),
            Edge(2, 4, 10),
            Edge(3, 4, 2),
            Edge(3, 5, 6),
            Edge(4, 5, 3),
        ]
        k_w = kruskal_mst(v, edges)
        p_w = prim_mst(v, edges)
        r_w = reverse_delete_mst(v, edges)

        self.assertEqual(k_w, 13)
        self.assertEqual(p_w, 13)
        self.assertEqual(r_w, 13)


if __name__ == "__main__":
    unittest.main()
