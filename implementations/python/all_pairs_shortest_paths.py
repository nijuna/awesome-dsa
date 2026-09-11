"""
Reference Implementation: All-Pairs Shortest Paths (APSP)
Demonstrates:
1. Floyd-Warshall Algorithm (O(V^3) Dense Dynamic Programming with In-Place Storage).
2. Next-Hop Predecessor Matrix & Path Reconstruction.
3. Negative Cycle Detection via Diagonal Invariant (dist[i][i] < 0).
4. Johnson's Algorithm for Sparse Graphs with Potential Reweighting (O(V * E + V^2 log V)).
5. Warshall's Transitive Closure (O(V^3)).

Language: Python 3
"""

import heapq
from typing import List, Optional, Tuple
import unittest

INF = 10**18


def floyd_warshall(
    n: int,
    adj_matrix: List[List[int]],
) -> Tuple[List[List[int]], List[List[int]], bool]:
    """Computes APSP using the Floyd-Warshall dynamic programming algorithm.

    Returns: (distance_matrix, next_hop_matrix, has_negative_cycle)
    Time Complexity: O(V^3)
    Space Complexity: O(V^2)
    """
    dist = [row[:] for row in adj_matrix]
    nxt = [[-1] * n for _ in range(n)]

    for i in range(n):
        dist[i][i] = min(dist[i][i], 0)
        nxt[i][i] = i
        for j in range(n):
            if i != j and adj_matrix[i][j] != INF:
                nxt[i][j] = j

    for k in range(n):
        for i in range(n):
            if dist[i][k] == INF:
                continue
            for j in range(n):
                if dist[k][j] == INF:
                    continue
                nd = dist[i][k] + dist[k][j]
                if nd < dist[i][j]:
                    dist[i][j] = nd
                    nxt[i][j] = nxt[i][k]

    has_negative_cycle = any(dist[i][i] < 0 for i in range(n))
    return dist, nxt, has_negative_cycle


def reconstruct_path(u: int, v: int, nxt: List[List[int]]) -> List[int]:
    """Reconstructs the directed shortest path from u to v."""
    if nxt[u][v] == -1:
        return []
    path = [u]
    curr = u
    n = len(nxt)
    while curr != v:
        curr = nxt[curr][v]
        if curr == -1 or len(path) > n:
            return []  # Unreachable or trapped in negative cycle
        path.append(curr)
    return path


def _bellman_ford_potential(
    n: int,
    edges: List[Tuple[int, int, int]],
) -> Optional[List[int]]:
    super_source = n
    h = [INF] * (n + 1)
    h[super_source] = 0

    ext_edges = edges[:] + [(super_source, v, 0) for v in range(n)]

    for _ in range(n):
        changed = False
        for u, v, w in ext_edges:
            if h[u] == INF:
                continue
            nd = h[u] + w
            if nd < h[v]:
                h[v] = nd
                changed = True
        if not changed:
            break

    for u, v, w in ext_edges:
        if h[u] == INF:
            continue
        if h[u] + w < h[v]:
            return None  # Negative cycle detected

    return h[:-1]


def _dijkstra(
    n: int,
    graph: List[List[Tuple[int, int]]],
    source: int,
) -> List[int]:
    dist = [INF] * n
    dist[source] = 0
    pq = [(0, source)]

    while pq:
        d, u = heapq.heappop(pq)
        if d > dist[u]:
            continue
        for v, w in graph[u]:
            nd = d + w
            if nd < dist[v]:
                dist[v] = nd
                heapq.heappush(pq, (nd, v))
    return dist


def johnson_apsp(
    n: int,
    edges: List[Tuple[int, int, int]],
) -> Tuple[Optional[List[List[int]]], bool]:
    """Computes APSP on sparse graphs using Johnson's algorithm.

    Time Complexity: O(V * E + V^2 log V)
    Space Complexity: O(V^2)
    """
    h = _bellman_ford_potential(n, edges)
    if h is None:
        return None, True  # Negative cycle detected

    reweighted_graph = [[] for _ in range(n)]
    for u, v, w in edges:
        rw = w + h[u] - h[v]
        reweighted_graph[u].append((v, rw))

    all_dist = [[INF] * n for _ in range(n)]
    for s in range(n):
        d = _dijkstra(n, reweighted_graph, s)
        for v in range(n):
            if d[v] != INF:
                all_dist[s][v] = d[v] - h[s] + h[v]

    return all_dist, False


def warshall_transitive_closure(adj: List[List[bool]]) -> List[List[bool]]:
    """Computes the reachability matrix using Warshall's algorithm."""
    n = len(adj)
    reach = [row[:] for row in adj]

    for i in range(n):
        reach[i][i] = True

    for k in range(n):
        for i in range(n):
            if not reach[i][k]:
                continue
            for j in range(n):
                reach[i][j] = reach[i][j] or reach[k][j]

    return reach


class TestAllPairsShortestPaths(unittest.TestCase):
    def test_arthur_example(self):
        n = 4
        adj = [[INF] * n for _ in range(n)]
        adj[0][1] = 3
        adj[0][2] = 8
        adj[1][2] = 2
        adj[2][3] = 1
        adj[1][3] = 7

        edges = [
            (0, 1, 3),
            (0, 2, 8),
            (1, 2, 2),
            (2, 3, 1),
            (1, 3, 7),
        ]

        dist_fw, nxt, neg_fw = floyd_warshall(n, adj)
        dist_jn, neg_jn = johnson_apsp(n, edges)

        self.assertFalse(neg_fw)
        self.assertFalse(neg_jn)
        self.assertEqual(dist_fw[0][0], 0)
        self.assertEqual(dist_fw[0][1], 3)
        self.assertEqual(dist_fw[0][2], 5)
        self.assertEqual(dist_fw[0][3], 6)
        self.assertEqual(dist_fw, dist_jn)

        path_0_3 = reconstruct_path(0, 3, nxt)
        self.assertEqual(path_0_3, [0, 1, 2, 3])
        path_3_0 = reconstruct_path(3, 0, nxt)
        self.assertEqual(path_3_0, [])

    def test_negative_edges_no_cycle(self):
        n = 4
        adj = [[INF] * n for _ in range(n)]
        adj[0][1] = 1
        adj[1][2] = -2
        adj[2][3] = 3
        adj[0][3] = 10

        edges = [(0, 1, 1), (1, 2, -2), (2, 3, 3), (0, 3, 10)]

        dist_fw, _, neg_fw = floyd_warshall(n, adj)
        dist_jn, neg_jn = johnson_apsp(n, edges)

        self.assertFalse(neg_fw)
        self.assertFalse(neg_jn)
        self.assertEqual(dist_fw[0][3], 2)
        self.assertEqual(dist_fw, dist_jn)

    def test_negative_cycle_detection(self):
        n = 4
        adj = [[INF] * n for _ in range(n)]
        adj[0][1] = 1
        adj[1][2] = -2
        adj[2][3] = -1
        adj[3][1] = -2

        edges = [(0, 1, 1), (1, 2, -2), (2, 3, -1), (3, 1, -2)]

        _, _, neg_fw = floyd_warshall(n, adj)
        _, neg_jn = johnson_apsp(n, edges)

        self.assertTrue(neg_fw)
        self.assertTrue(neg_jn)

    def test_transitive_closure(self):
        n = 5
        adj = [[False] * n for _ in range(n)]
        adj[0][1] = True
        adj[1][2] = True
        adj[2][3] = True
        adj[4][0] = True

        reach = warshall_transitive_closure(adj)
        self.assertTrue(reach[4][3])
        self.assertFalse(reach[3][0])
        self.assertTrue(reach[2][2])


if __name__ == "__main__":
    unittest.main()
