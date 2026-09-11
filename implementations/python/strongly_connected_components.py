"""
Reference Implementation: Strongly Connected Components (SCC) & Condensation Graphs
Demonstrates:
1. Kosaraju's 2-Pass DFS Algorithm (Finishing Order + Transpose Graph).
2. Tarjan's 1-Pass DFS Algorithm (Discovery Times + Low-Link Values + Active Stack).
3. Condensation DAG Construction (Deduplicated Component Meta-Graph).
4. 2-Satisfiability (2-SAT) Solver via Implication Graph and SCC Decomposition.

Language: Python 3
"""

from typing import List, Optional
import unittest


def strongly_connected_components_kosaraju(graph: List[List[int]]) -> List[List[int]]:
    """Computes SCCs using Kosaraju's two-pass DFS algorithm.

    Time Complexity: O(V + E)
    Space Complexity: O(V + E)
    """
    n = len(graph)
    visited = [False] * n
    order = []

    def dfs1(u: int):
        visited[u] = True
        for v in graph[u]:
            if not visited[v]:
                dfs1(v)
        order.append(u)

    for u in range(n):
        if not visited[u]:
            dfs1(u)

    transpose = [[] for _ in range(n)]
    for u in range(n):
        for v in graph[u]:
            transpose[v].append(u)

    visited = [False] * n
    order.reverse()
    components = []

    def dfs2(u: int, comp: List[int]):
        visited[u] = True
        comp.append(u)
        for v in transpose[u]:
            if not visited[v]:
                dfs2(v, comp)

    for u in order:
        if not visited[u]:
            comp = []
            dfs2(u, comp)
            components.append(comp)

    return components


def strongly_connected_components_tarjan(graph: List[List[int]]) -> List[List[int]]:
    """Computes SCCs using Tarjan's single-pass DFS algorithm with discovery times and low-link values.

    Time Complexity: O(V + E)
    Space Complexity: O(V)
    """
    n = len(graph)
    disc = [-1] * n
    low = [-1] * n
    on_stack = [False] * n
    stack = []
    components = []
    timer = 0

    def dfs(u: int):
        nonlocal timer
        disc[u] = low[u] = timer
        timer += 1
        stack.append(u)
        on_stack[u] = True

        for v in graph[u]:
            if disc[v] == -1:
                # Tree edge
                dfs(v)
                low[u] = min(low[u], low[v])
            elif on_stack[v]:
                # Back edge to a node currently in the active DFS recursion stack
                low[u] = min(low[u], disc[v])

        # u is the root of an SCC
        if low[u] == disc[u]:
            comp = []
            while True:
                v = stack.pop()
                on_stack[v] = False
                comp.append(v)
                if v == u:
                    break
            components.append(comp)

    for u in range(n):
        if disc[u] == -1:
            dfs(u)

    return components


def make_component_id(n: int, components: List[List[int]]) -> List[int]:
    """Maps each vertex to its zero-indexed component identifier."""
    component_id = [-1] * n
    for cid, comp in enumerate(components):
        for u in comp:
            component_id[u] = cid
    return component_id


def build_condensation_dag(
    graph: List[List[int]],
    component_id: List[int],
    component_count: int,
) -> List[List[int]]:
    """Builds the condensation DAG of components with deduplicated cross-edges."""
    dag_set = [set() for _ in range(component_count)]

    for u in range(len(graph)):
        for v in graph[u]:
            cu = component_id[u]
            cv = component_id[v]
            if cu != cv:
                dag_set[cu].add(cv)

    return [sorted(neighbors) for neighbors in dag_set]


class TwoSatSolver:
    """Solves 2-Satisfiability problems via implication graphs and SCC decomposition."""

    def __init__(self, num_vars: int):
        self.num_vars = num_vars
        self.adj = [[] for _ in range(2 * num_vars)]

    def add_clause(self, u: int, val_u: bool, v: int, val_v: bool):
        """Adds clause: (u == val_u) OR (v == val_v)."""
        lit_u = 2 * u + (0 if val_u else 1)
        lit_v = 2 * v + (0 if val_v else 1)
        neg_u = lit_u ^ 1
        neg_v = lit_v ^ 1
        # ~lit_u => lit_v, ~lit_v => lit_u
        self.adj[neg_u].append(lit_v)
        self.adj[neg_v].append(lit_u)

    def solve(self) -> Optional[List[bool]]:
        """Finds a satisfying truth assignment if one exists, or returns None if unsatisfiable."""
        sccs = strongly_connected_components_tarjan(self.adj)
        comp_id = make_component_id(2 * self.num_vars, sccs)

        for i in range(self.num_vars):
            if comp_id[2 * i] == comp_id[2 * i + 1]:
                return None  # Contradiction: x and ~x belong to the same SCC

        # Tarjan produces components in reverse topological order.
        # comp_id[2*i] < comp_id[2*i+1] means literal x is finished earlier in reverse topological order
        # (topologically later / reachable from ~x), so assign True.
        assignment = [comp_id[2 * i] < comp_id[2 * i + 1] for i in range(self.num_vars)]
        return assignment


class TestStronglyConnectedComponents(unittest.TestCase):
    def _assert_same_partition(self, n: int, scc_a: List[List[int]], scc_b: List[List[int]]):
        id_a = make_component_id(n, scc_a)
        id_b = make_component_id(n, scc_b)
        for u in range(n):
            for v in range(n):
                self.assertEqual(
                    id_a[u] == id_a[v],
                    id_b[u] == id_b[v],
                    f"Partition mismatch between nodes {u} and {v}",
                )

    def test_two_scc_graph(self):
        # 0 -> 1 -> 2 -> 0, 2 -> 3, 3 -> 4 -> 5 -> 3
        g = [
            [1],
            [2],
            [0, 3],
            [4],
            [5],
            [3],
        ]
        scc_k = strongly_connected_components_kosaraju(g)
        scc_t = strongly_connected_components_tarjan(g)

        self.assertEqual(len(scc_k), 2)
        self.assertEqual(len(scc_t), 2)
        self._assert_same_partition(6, scc_k, scc_t)

        comp_id = make_component_id(6, scc_t)
        self.assertEqual(comp_id[0], comp_id[1])
        self.assertEqual(comp_id[1], comp_id[2])
        self.assertEqual(comp_id[3], comp_id[4])
        self.assertEqual(comp_id[4], comp_id[5])
        self.assertNotEqual(comp_id[0], comp_id[3])

        dag = build_condensation_dag(g, comp_id, 2)
        self.assertEqual(len(dag), 2)
        c_source = comp_id[0]
        c_sink = comp_id[3]
        self.assertEqual(dag[c_source], [c_sink])
        self.assertEqual(dag[c_sink], [])

    def test_linear_dag(self):
        # 0 -> 1 -> 2 -> 3
        g = [[1], [2], [3], []]
        scc_k = strongly_connected_components_kosaraju(g)
        scc_t = strongly_connected_components_tarjan(g)

        self.assertEqual(len(scc_k), 4)
        self.assertEqual(len(scc_t), 4)
        self._assert_same_partition(4, scc_k, scc_t)

    def test_single_cycle(self):
        # 0 -> 1 -> 2 -> 3 -> 4 -> 0
        g = [[1], [2], [3], [4], [0]]
        scc_k = strongly_connected_components_kosaraju(g)
        scc_t = strongly_connected_components_tarjan(g)

        self.assertEqual(len(scc_k), 1)
        self.assertEqual(len(scc_t), 1)
        self.assertEqual(len(scc_k[0]), 5)
        self.assertEqual(len(scc_t[0]), 5)
        self._assert_same_partition(5, scc_k, scc_t)

    def test_disconnected_graph(self):
        # 0 -> 1 -> 0, 2 (isolated), 3 -> 4 -> 5 -> 3
        g = [[1], [0], [], [4], [5], [3]]
        scc_k = strongly_connected_components_kosaraju(g)
        scc_t = strongly_connected_components_tarjan(g)

        self.assertEqual(len(scc_k), 3)
        self.assertEqual(len(scc_t), 3)
        self._assert_same_partition(6, scc_k, scc_t)

    def test_twosat_satisfiable(self):
        solver = TwoSatSolver(2)
        clauses = [
            (0, True, 1, True),
            (0, False, 1, True),
            (0, True, 1, False),
        ]
        for u, vu, v, vv in clauses:
            solver.add_clause(u, vu, v, vv)

        ans = solver.solve()
        self.assertIsNotNone(ans)
        self.assertEqual(len(ans), 2)
        for u, vu, v, vv in clauses:
            self.assertTrue((ans[u] == vu) or (ans[v] == vv))

    def test_twosat_unsatisfiable_single_var(self):
        solver = TwoSatSolver(1)
        solver.add_clause(0, True, 0, True)
        solver.add_clause(0, False, 0, False)
        ans = solver.solve()
        self.assertIsNone(ans)

    def test_twosat_xor_xnor_unsat(self):
        solver = TwoSatSolver(2)
        solver.add_clause(0, True, 1, True)
        solver.add_clause(0, False, 1, False)
        solver.add_clause(0, False, 1, True)
        solver.add_clause(0, True, 1, False)
        ans = solver.solve()
        self.assertIsNone(ans)

    def test_twosat_complex_sat(self):
        solver = TwoSatSolver(3)
        clauses = [
            (0, True, 1, True),
            (1, False, 2, True),
            (2, False, 0, False),
            (0, True, 2, False),
        ]
        for u, vu, v, vv in clauses:
            solver.add_clause(u, vu, v, vv)

        ans = solver.solve()
        self.assertIsNotNone(ans)
        for u, vu, v, vv in clauses:
            self.assertTrue((ans[u] == vu) or (ans[v] == vv))


if __name__ == "__main__":
    unittest.main()
