"""
Reference Implementation: Graph Representations & Memory Layouts
Demonstrates:
1. Adjacency Matrix (O(1) Edge Lookup, O(V^2) Storage).
2. Adjacency List (List-of-Lists, O(V + E) Sparse Storage).
3. Edge List (Flat List of Edge Tuples, O(E) Storage).
4. Compressed Sparse Row (CSR / Forward Star, Contiguous O(V + E)).
5. Implicit Grid Graph (Zero-Allocation Neighborhood Generator).
6. Inter-Representation Conversion Pipelines & Traversal Parity Verification.

Language: Python 3
"""

from collections import deque
from typing import Generator, List, Optional, Tuple
import unittest

NO_EDGE = 10**18


class AdjacencyMatrix:
    def __init__(self, vertices: int):
        self.n = vertices
        self.mat = [[NO_EDGE] * vertices for _ in range(vertices)]
        for i in range(vertices):
            self.mat[i][i] = 0

    def add_edge(self, u: int, v: int, weight: int = 1, directed: bool = True):
        self.mat[u][v] = weight
        if not directed:
            self.mat[v][u] = weight

    def has_edge(self, u: int, v: int) -> bool:
        return u != v and self.mat[u][v] != NO_EDGE

    def get_neighbors(self, u: int) -> List[int]:
        return [v for v in range(self.n) if u != v and self.mat[u][v] != NO_EDGE]


class AdjacencyList:
    def __init__(self, vertices: int):
        self.n = vertices
        self.adj = [[] for _ in range(vertices)]

    def add_edge(self, u: int, v: int, weight: int = 1, directed: bool = True):
        self.adj[u].append((v, weight))
        if not directed:
            self.adj[v].append((u, weight))

    def has_edge(self, u: int, v: int) -> bool:
        return any(neighbor == v for neighbor, _ in self.adj[u])

    def get_neighbors(self, u: int) -> List[Tuple[int, int]]:
        return self.adj[u]


class EdgeList:
    def __init__(self, vertices: int):
        self.n = vertices
        self.edges = []

    def add_edge(self, u: int, v: int, weight: int = 1):
        self.edges.append((u, v, weight))


class CSRGraph:
    def __init__(self, n: int, offsets: List[int], to: List[int], weights: List[int]):
        self.n = n
        self.offsets = offsets
        self.to = to
        self.weights = weights

    @classmethod
    def from_edges(
        cls,
        vertices: int,
        edges: List[Tuple[int, int, int]],
        directed: bool = True,
    ) -> "CSRGraph":
        all_edges = []
        for u, v, w in edges:
            all_edges.append((u, v, w))
            if not directed:
                all_edges.append((v, u, w))

        offsets = [0] * (vertices + 1)
        for u, _, _ in all_edges:
            offsets[u + 1] += 1

        for i in range(1, vertices + 1):
            offsets[i] += offsets[i - 1]

        total_edges = len(all_edges)
        to = [0] * total_edges
        weights = [0] * total_edges
        cur_head = offsets[:]

        for u, v, w in all_edges:
            pos = cur_head[u]
            to[pos] = v
            weights[pos] = w
            cur_head[u] += 1

        return cls(vertices, offsets, to, weights)

    def degree(self, u: int) -> int:
        return self.offsets[u + 1] - self.offsets[u]

    def get_neighbors(self, u: int) -> List[int]:
        return self.to[self.offsets[u] : self.offsets[u + 1]]


class ImplicitGrid:
    def __init__(self, rows: int, cols: int):
        self.rows = rows
        self.cols = cols

    def get_neighbors4(self, r: int, c: int) -> Generator[Tuple[int, int], None, None]:
        for dr, dc in [(-1, 0), (1, 0), (0, -1), (0, 1)]:
            nr, nc = r + dr, c + dc
            if 0 <= nr < self.rows and 0 <= nc < self.cols:
                yield nr, nc

    def get_neighbors8(self, r: int, c: int) -> Generator[Tuple[int, int], None, None]:
        for dr in [-1, 0, 1]:
            for dc in [-1, 0, 1]:
                if dr == 0 and dc == 0:
                    continue
                nr, nc = r + dr, c + dc
                if 0 <= nr < self.rows and 0 <= nc < self.cols:
                    yield nr, nc


def edge_list_to_adj_list(el: EdgeList, directed: bool = True) -> AdjacencyList:
    al = AdjacencyList(el.n)
    for u, v, w in el.edges:
        al.add_edge(u, v, w, directed)
    return al


def adj_list_to_edge_list(al: AdjacencyList) -> EdgeList:
    el = EdgeList(al.n)
    for u in range(al.n):
        for v, w in al.adj[u]:
            el.add_edge(u, v, w)
    return el


def adj_list_to_adj_matrix(al: AdjacencyList) -> AdjacencyMatrix:
    mat = AdjacencyMatrix(al.n)
    for u in range(al.n):
        for v, w in al.adj[u]:
            mat.add_edge(u, v, w, directed=True)
    return mat


def adj_matrix_to_adj_list(mat: AdjacencyMatrix) -> AdjacencyList:
    al = AdjacencyList(mat.n)
    for u in range(mat.n):
        for v in range(mat.n):
            if u != v and mat.mat[u][v] != NO_EDGE:
                al.add_edge(u, v, mat.mat[u][v], directed=True)
    return al


class TestGraphRepresentations(unittest.TestCase):
    def test_core_representations_and_bfs(self):
        n = 4
        el = EdgeList(n)
        el.add_edge(0, 1, 10)
        el.add_edge(0, 3, 40)
        el.add_edge(1, 2, 20)
        el.add_edge(2, 3, 30)

        al = edge_list_to_adj_list(el, directed=True)
        mat = adj_list_to_adj_matrix(al)
        csr = CSRGraph.from_edges(n, el.edges, directed=True)

        # Verify edge lookups
        self.assertTrue(mat.has_edge(0, 1) and mat.has_edge(0, 3))
        self.assertTrue(mat.has_edge(1, 2) and mat.has_edge(2, 3))
        self.assertFalse(mat.has_edge(3, 0) or mat.has_edge(0, 2))

        self.assertTrue(al.has_edge(0, 1) and al.has_edge(0, 3))
        self.assertFalse(al.has_edge(3, 0) or al.has_edge(0, 2))

        # Verify CSR degrees
        self.assertEqual(csr.degree(0), 2)
        self.assertEqual(csr.degree(1), 1)
        self.assertEqual(csr.degree(2), 1)
        self.assertEqual(csr.degree(3), 0)
        self.assertEqual(csr.offsets, [0, 2, 3, 4, 4])

        # BFS traversals
        def bfs_mat(src):
            order = []
            vis = [False] * n
            q = deque([src])
            vis[src] = True
            while q:
                u = q.popleft()
                order.append(u)
                for v in mat.get_neighbors(u):
                    if not vis[v]:
                        vis[v] = True
                        q.append(v)
            return order

        def bfs_al(src):
            order = []
            vis = [False] * n
            q = deque([src])
            vis[src] = True
            while q:
                u = q.popleft()
                order.append(u)
                for v, _ in al.get_neighbors(u):
                    if not vis[v]:
                        vis[v] = True
                        q.append(v)
            return order

        def bfs_csr(src):
            order = []
            vis = [False] * n
            q = deque([src])
            vis[src] = True
            while q:
                u = q.popleft()
                order.append(u)
                for v in csr.get_neighbors(u):
                    if not vis[v]:
                        vis[v] = True
                        q.append(v)
            return order

        order_mat = bfs_mat(0)
        order_al = bfs_al(0)
        order_csr = bfs_csr(0)

        self.assertEqual(order_mat, [0, 1, 3, 2])
        self.assertEqual(order_mat, order_al)
        self.assertEqual(order_mat, order_csr)

    def test_roundtrip_conversion(self):
        n = 3
        al = AdjacencyList(n)
        al.add_edge(0, 1, 5, directed=True)
        al.add_edge(1, 2, 7, directed=True)

        mat = adj_list_to_adj_matrix(al)
        al2 = adj_matrix_to_adj_list(mat)
        mat2 = adj_list_to_adj_matrix(al2)

        self.assertEqual(mat.mat, mat2.mat)

    def test_implicit_grid(self):
        grid = ImplicitGrid(3, 4)
        neighbors4_corner = list(grid.get_neighbors4(0, 0))
        self.assertEqual(len(neighbors4_corner), 2)

        neighbors4_interior = list(grid.get_neighbors4(1, 1))
        self.assertEqual(len(neighbors4_interior), 4)

        neighbors8_interior = list(grid.get_neighbors8(1, 1))
        self.assertEqual(len(neighbors8_interior), 8)


if __name__ == "__main__":
    unittest.main()
