"""
Reference Implementation: Topological Sorting & DAG Dynamic Programming
Demonstrates:
1. DFS-based topological sort with 3-color directed cycle detection.
2. Kahn's algorithm (in-degree peeling with FIFO queue).
3. Lexicographical topological sort using a min-priority queue.
4. Longest path / critical path computation in a DAG via topological DP.

Language: Python 3
"""

from collections import deque
import heapq
from typing import List, Tuple
import unittest


def topological_sort_dfs(graph: List[List[int]]) -> List[int]:
    n = len(graph)
    color = [0] * n  # 0 = White, 1 = Gray, 2 = Black
    order = []

    def dfs(u: int):
        color[u] = 1
        for v in graph[u]:
            if color[v] == 1:
                raise ValueError("Cycle detected: topological order does not exist")
            if color[v] == 0:
                dfs(v)
        color[u] = 2
        order.append(u)

    for u in range(n):
        if color[u] == 0:
            dfs(u)

    order.reverse()
    return order


def topological_sort_kahn(graph: List[List[int]]) -> List[int]:
    n = len(graph)
    indegree = [0] * n

    for u in range(n):
        for v in graph[u]:
            indegree[v] += 1

    q = deque([u for u in range(n) if indegree[u] == 0])
    order = []

    while q:
        u = q.popleft()
        order.append(u)

        for v in graph[u]:
            indegree[v] -= 1
            if indegree[v] == 0:
                q.append(v)

    if len(order) != n:
        raise ValueError("Cycle detected: topological order does not exist")

    return order


def lexicographically_smallest_topological_sort(graph: List[List[int]]) -> List[int]:
    n = len(graph)
    indegree = [0] * n

    for u in range(n):
        for v in graph[u]:
            indegree[v] += 1

    heap = [u for u in range(n) if indegree[u] == 0]
    heapq.heapify(heap)

    order = []

    while heap:
        u = heapq.heappop(heap)
        order.append(u)

        for v in graph[u]:
            indegree[v] -= 1
            if indegree[v] == 0:
                heapq.heappush(heap, v)

    if len(order) != n:
        raise ValueError("Cycle detected: topological order does not exist")

    return order


def longest_path_dag(n: int, graph: List[List[Tuple[int, int]]]) -> int:
    unweighted = [[] for _ in range(n)]
    for u in range(n):
        for v, _w in graph[u]:
            unweighted[u].append(v)

    order = topological_sort_kahn(unweighted)
    dist = [0] * n

    for u in order:
        for v, w in graph[u]:
            if dist[u] + w > dist[v]:
                dist[v] = dist[u] + w

    return max(dist) if dist else 0


class TestTopologicalSort(unittest.TestCase):
    def setUp(self):
        # 5 -> 2, 5 -> 0, 4 -> 0, 4 -> 1, 2 -> 3, 3 -> 1
        self.dag = [
            [],        # 0
            [],        # 1
            [3],       # 2
            [1],       # 3
            [0, 1],    # 4
            [2, 0],    # 5
        ]

    def _verify_order(self, order: List[int]):
        self.assertEqual(len(order), 6)
        pos = {node: i for i, node in enumerate(order)}
        for u in range(6):
            for v in self.dag[u]:
                self.assertLess(pos[u], pos[v])

    def test_dfs_order(self):
        order = topological_sort_dfs(self.dag)
        self._verify_order(order)

    def test_kahn_order(self):
        order = topological_sort_kahn(self.dag)
        self._verify_order(order)

    def test_lexicographical_order(self):
        order = lexicographically_smallest_topological_sort(self.dag)
        self._verify_order(order)
        self.assertEqual(order, [4, 5, 0, 2, 3, 1])

    def test_cycle_detection(self):
        cyclic = [row[:] for row in self.dag]
        cyclic[1].append(5)  # Creates cycle: 5 -> 2 -> 3 -> 1 -> 5
        with self.assertRaises(ValueError):
            topological_sort_dfs(cyclic)
        with self.assertRaises(ValueError):
            topological_sort_kahn(cyclic)

    def test_longest_path(self):
        # 0 --(3)--> 1 --(4)--> 3
        # 0 --(2)--> 2 --(6)--> 3
        # Longest: 0 -> 2 -> 3 = 8
        weighted_dag = [
            [(1, 3), (2, 2)],
            [(3, 4)],
            [(3, 6)],
            [],
        ]
        self.assertEqual(longest_path_dag(4, weighted_dag), 8)


if __name__ == "__main__":
    unittest.main()
