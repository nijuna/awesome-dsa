"""
Reference Implementation: Graph Traversal Patterns
Demonstrates:
1. Standard BFS (layer-by-layer exploration, unweighted shortest path, parent reconstruction).
2. Multi-source BFS (simultaneous wavefront expansion).
3. 0-1 BFS with collections.deque (O(V + E) shortest paths on 0/1 weighted graphs).
4. DFS (recursive and iterative explicit stack).
5. Cycle detection for undirected and directed graphs (3-color state tracking).
6. Connected components enumeration.

Language: Python 3
"""

from collections import deque
from typing import List, Tuple, Optional
import unittest


class BFSResult:
    def __init__(self, dist: List[int], parent: List[int]):
        self.dist = dist
        self.parent = parent

    def reconstruct_path(self, target: int) -> List[int]:
        if self.dist[target] == -1:
            return []
        path = []
        curr = target
        while curr != -1:
            path.append(curr)
            curr = self.parent[curr]
        path.reverse()
        return path


def bfs(adj: List[List[int]], start: int) -> BFSResult:
    n = len(adj)
    dist = [-1] * n
    parent = [-1] * n
    q = deque()

    dist[start] = 0
    q.append(start)

    while q:
        u = q.popleft()
        for v in adj[u]:
            if dist[v] == -1:
                dist[v] = dist[u] + 1
                parent[v] = u
                q.append(v)

    return BFSResult(dist, parent)


def multi_source_bfs(adj: List[List[int]], sources: List[int]) -> List[int]:
    n = len(adj)
    dist = [-1] * n
    q = deque()

    for s in sources:
        dist[s] = 0
        q.append(s)

    while q:
        u = q.popleft()
        for v in adj[u]:
            if dist[v] == -1:
                dist[v] = dist[u] + 1
                q.append(v)

    return dist


def zero_one_bfs(adj: List[List[Tuple[int, int]]], start: int) -> List[int]:
    n = len(adj)
    dist = [float("inf")] * n
    dq = deque()

    dist[start] = 0
    dq.append(start)

    while dq:
        u = dq.popleft()
        for v, w in adj[u]:
            if dist[u] + w < dist[v]:
                dist[v] = dist[u] + w
                if w == 0:
                    dq.appendleft(v)
                else:
                    dq.append(v)

    return dist


def dfs_recursive(adj: List[List[int]], start: int) -> List[int]:
    n = len(adj)
    visited = [False] * n
    order = []

    def visit(u: int):
        visited[u] = True
        order.append(u)
        for v in adj[u]:
            if not visited[v]:
                visit(v)

    visit(start)
    return order


def dfs_iterative(adj: List[List[int]], start: int) -> List[int]:
    n = len(adj)
    visited = [False] * n
    order = []
    stack = [start]

    while stack:
        u = stack.pop()
        if visited[u]:
            continue
        visited[u] = True
        order.append(u)

        for v in reversed(adj[u]):
            if not visited[v]:
                stack.append(v)

    return order


def has_cycle_undirected(adj: List[List[int]]) -> bool:
    n = len(adj)
    visited = [False] * n

    def visit(u: int, parent: int) -> bool:
        visited[u] = True
        for v in adj[u]:
            if not visited[v]:
                if visit(v, u):
                    return True
            elif v != parent:
                return True
        return False

    for i in range(n):
        if not visited[i]:
            if visit(i, -1):
                return True
    return False


def has_cycle_directed(adj: List[List[int]]) -> bool:
    n = len(adj)
    color = [0] * n  # 0=White, 1=Gray, 2=Black

    def visit(u: int) -> bool:
        color[u] = 1
        for v in adj[u]:
            if color[v] == 1:
                return True
            if color[v] == 0:
                if visit(v):
                    return True
        color[u] = 2
        return False

    for i in range(n):
        if color[i] == 0:
            if visit(i):
                return True
    return False


def get_connected_components(adj: List[List[int]]) -> List[List[int]]:
    n = len(adj)
    visited = [False] * n
    components = []

    for i in range(n):
        if not visited[i]:
            comp = []
            q = deque([i])
            visited[i] = True

            while q:
                u = q.popleft()
                comp.append(u)
                for v in adj[u]:
                    if not visited[v]:
                        visited[v] = True
                        q.append(v)
            components.append(comp)

    return components


class TestGraphTraversals(unittest.TestCase):
    def setUp(self):
        # 0 - 1 - 3 - 5
        # |   |
        # 2 - 4
        self.adj1 = [
            [1, 2],
            [0, 3, 4],
            [0, 4],
            [1, 5],
            [1, 2],
            [3],
        ]

    def test_bfs_and_path(self):
        res = bfs(self.adj1, 0)
        self.assertEqual(res.dist, [0, 1, 1, 2, 2, 3])
        self.assertEqual(res.reconstruct_path(5), [0, 1, 3, 5])

    def test_multi_source_bfs(self):
        dist = multi_source_bfs(self.adj1, [2, 5])
        self.assertEqual(dist[2], 0)
        self.assertEqual(dist[5], 0)
        self.assertEqual(dist[0], 1)
        self.assertEqual(dist[3], 1)

    def test_zero_one_bfs(self):
        adj01 = [
            [(1, 1), (2, 0)],
            [(3, 1)],
            [(3, 0)],
            [],
        ]
        dist = zero_one_bfs(adj01, 0)
        self.assertEqual(dist, [0, 1, 0, 0])

    def test_dfs_orders(self):
        rec = dfs_recursive(self.adj1, 0)
        it = dfs_iterative(self.adj1, 0)
        self.assertEqual(len(rec), 6)
        self.assertEqual(len(it), 6)
        self.assertEqual(rec, it)

    def test_cycle_detection(self):
        cycle_undirected = [[1, 2], [0, 2], [0, 1]]
        tree_undirected = [[1], [0, 2], [1]]
        self.assertTrue(has_cycle_undirected(cycle_undirected))
        self.assertFalse(has_cycle_undirected(tree_undirected))

        cycle_directed = [[1], [2], [0]]
        dag_directed = [[1], [2], []]
        self.assertTrue(has_cycle_directed(cycle_directed))
        self.assertFalse(has_cycle_directed(dag_directed))

    def test_connected_components(self):
        disconnected = [[1], [0], [], [4], [3]]
        comps = get_connected_components(disconnected)
        self.assertEqual(len(comps), 3)


if __name__ == "__main__":
    unittest.main()
