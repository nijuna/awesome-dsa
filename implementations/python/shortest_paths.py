"""
Reference Implementation: Single-Source Shortest Paths (SSSP)
Demonstrates:
1. Dijkstra's Algorithm (Min-Priority Queue with Stale Pair Pruning).
2. DAG Shortest & Longest Paths via Topological Relaxation (O(V + E)).
3. Bellman-Ford Algorithm with Early-Stopping & Negative Cycle Detection (O(V * E)).
4. Shortest Path Faster Algorithm (SPFA) with Queue-Based Relaxation.
5. Path Reconstruction via Parent Predecessor Pointers.

Language: Python 3
"""

from collections import deque
import heapq
from typing import List, Optional, Tuple
import unittest

INF = 10**18
NEG_INF = -(10**18)


def dijkstra(
    graph: List[List[Tuple[int, int]]],
    source: int,
) -> Tuple[List[int], List[int]]:
    """Computes shortest paths from source using Dijkstra's algorithm.

    Requires all edge weights to be non-negative.
    Time Complexity: O((V + E) log V)
    Space Complexity: O(V)
    """
    n = len(graph)
    dist = [INF] * n
    parent = [-1] * n
    dist[source] = 0

    pq = [(0, source)]

    while pq:
        d, u = heapq.heappop(pq)

        # Prune stale priority queue entries
        if d > dist[u]:
            continue

        for v, w in graph[u]:
            nd = d + w
            if nd < dist[v]:
                dist[v] = nd
                parent[v] = u
                heapq.heappush(pq, (nd, v))

    return dist, parent


def reconstruct_path(source: int, target: int, parent: List[int]) -> List[int]:
    """Reconstructs the directed shortest path from source to target."""
    if source == target:
        return [source]
    if parent[target] == -1:
        return []  # Target is unreachable

    path = []
    v = target
    while v != -1:
        path.append(v)
        v = parent[v]
    path.reverse()
    if not path or path[0] != source:
        return []
    return path


def dag_shortest_paths(
    graph: List[List[Tuple[int, int]]],
    topo_order: List[int],
    source: int,
) -> Tuple[List[int], List[int]]:
    """Computes shortest paths on a DAG using topological relaxation.

    Supports negative edge weights because no cycles can exist.
    Time Complexity: O(V + E)
    Space Complexity: O(V)
    """
    n = len(graph)
    dist = [INF] * n
    parent = [-1] * n
    dist[source] = 0

    for u in topo_order:
        if dist[u] == INF:
            continue
        for v, w in graph[u]:
            nd = dist[u] + w
            if nd < dist[v]:
                dist[v] = nd
                parent[v] = u

    return dist, parent


def dag_longest_paths(
    graph: List[List[Tuple[int, int]]],
    topo_order: List[int],
    source: int,
) -> Tuple[List[int], List[int]]:
    """Computes longest paths (critical path) on a DAG using topological relaxation.

    Time Complexity: O(V + E)
    Space Complexity: O(V)
    """
    n = len(graph)
    dist = [NEG_INF] * n
    parent = [-1] * n
    dist[source] = 0

    for u in topo_order:
        if dist[u] == NEG_INF:
            continue
        for v, w in graph[u]:
            nd = dist[u] + w
            if nd > dist[v]:
                dist[v] = nd
                parent[v] = u

    return dist, parent


def bellman_ford(
    n: int,
    edges: List[Tuple[int, int, int]],
    source: int,
) -> Tuple[Optional[List[int]], Optional[List[int]], bool]:
    """Computes SSSP with arbitrary edge weights using Bellman-Ford.

    Returns (dist, parent, True) if no reachable negative cycle exists,
    or (None, None, False) if a negative cycle is reachable from source.
    Time Complexity: O(V * E)
    Space Complexity: O(V)
    """
    dist = [INF] * n
    parent = [-1] * n
    dist[source] = 0

    for _ in range(n - 1):
        any_update = False
        for u, v, w in edges:
            if dist[u] == INF:
                continue
            nd = dist[u] + w
            if nd < dist[v]:
                dist[v] = nd
                parent[v] = u
                any_update = True
        if not any_update:
            break

    # Check for negative cycles reachable from source
    for u, v, w in edges:
        if dist[u] == INF:
            continue
        if dist[u] + w < dist[v]:
            return None, None, False

    return dist, parent, True


def spfa(
    graph: List[List[Tuple[int, int]]],
    source: int,
) -> Tuple[Optional[List[int]], Optional[List[int]], bool]:
    """Computes SSSP using the queue-based Shortest Path Faster Algorithm (SPFA).

    Returns (dist, parent, True) if no reachable negative cycle exists,
    or (None, None, False) if a negative cycle is reachable from source.
    Average Time Complexity: O(k * E) where k << V on sparse graphs.
    Worst-case Time Complexity: O(V * E).
    """
    n = len(graph)
    dist = [INF] * n
    parent = [-1] * n
    in_queue = [False] * n
    relax_count = [0] * n

    q = deque([source])
    dist[source] = 0
    in_queue[source] = True

    while q:
        u = q.popleft()
        in_queue[u] = False

        for v, w in graph[u]:
            if dist[u] == INF:
                continue
            nd = dist[u] + w
            if nd < dist[v]:
                dist[v] = nd
                parent[v] = u

                if not in_queue[v]:
                    q.append(v)
                    in_queue[v] = True
                    relax_count[v] += 1
                    if relax_count[v] >= n:
                        return None, None, False

    return dist, parent, True


class TestShortestPaths(unittest.TestCase):
    def test_dijkstra_basic(self):
        # 0 --(4)--> 1, 0 --(1)--> 2, 2 --(2)--> 1, 1 --(1)--> 3, 2 --(5)--> 3
        g = [
            [(1, 4), (2, 1)],
            [(3, 1)],
            [(1, 2), (3, 5)],
            [],
            [],  # Node 4 unreachable
        ]
        dist, parent = dijkstra(g, 0)
        self.assertEqual(dist[0], 0)
        self.assertEqual(dist[1], 3)
        self.assertEqual(dist[2], 1)
        self.assertEqual(dist[3], 4)
        self.assertEqual(dist[4], INF)

        path_to_3 = reconstruct_path(0, 3, parent)
        self.assertEqual(path_to_3, [0, 2, 1, 3])
        path_to_4 = reconstruct_path(0, 4, parent)
        self.assertEqual(path_to_4, [])

    def test_dag_shortest_and_longest_paths(self):
        # 0 --(3)--> 1, 0 --(2)--> 2, 1 --(-4)--> 2, 2 --(5)--> 3
        g = [
            [(1, 3), (2, 2)],
            [(2, -4)],
            [(3, 5)],
            [],
        ]
        topo = [0, 1, 2, 3]

        sp, parent_sp = dag_shortest_paths(g, topo, 0)
        self.assertEqual(sp, [0, 3, -1, 4])
        self.assertEqual(reconstruct_path(0, 3, parent_sp), [0, 1, 2, 3])

        lp, parent_lp = dag_longest_paths(g, topo, 0)
        self.assertEqual(lp, [0, 3, 2, 7])
        self.assertEqual(reconstruct_path(0, 3, parent_lp), [0, 2, 3])

    def test_bellman_ford_and_spfa_negative_edges(self):
        edges = [
            (0, 1, -1),
            (0, 2, 4),
            (1, 2, 3),
            (1, 3, 2),
            (1, 4, 2),
            (3, 2, 5),
            (3, 1, 1),
            (4, 3, -3),
        ]
        n = 5
        dist_bf, parent_bf, ok_bf = bellman_ford(n, edges, 0)
        self.assertTrue(ok_bf)
        self.assertEqual(dist_bf, [0, -1, 2, -2, 1])
        self.assertEqual(reconstruct_path(0, 3, parent_bf), [0, 1, 4, 3])

        # Adjacency list for SPFA
        adj = [[] for _ in range(n)]
        for u, v, w in edges:
            adj[u].append((v, w))

        dist_spfa, parent_spfa, ok_spfa = spfa(adj, 0)
        self.assertTrue(ok_spfa)
        self.assertEqual(dist_bf, dist_spfa)

    def test_negative_cycle_detection(self):
        edges = [
            (0, 1, 1),
            (1, 2, -2),
            (2, 3, -1),
            (3, 1, -2),  # Negative cycle: 1 -> 2 -> 3 -> 1 (weight = -5)
        ]
        n = 4
        _, _, ok_bf = bellman_ford(n, edges, 0)
        self.assertFalse(ok_bf)

        adj = [[] for _ in range(n)]
        for u, v, w in edges:
            adj[u].append((v, w))

        _, _, ok_spfa = spfa(adj, 0)
        self.assertFalse(ok_spfa)


if __name__ == "__main__":
    unittest.main()
