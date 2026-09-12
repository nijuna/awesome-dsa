"""
Reference implementation of Highest-Label Push-Relabel Maximum Flow with Gap Heuristic.

Implements preflow discharge, height labels, highest-label bucket selection,
gap relabeling, current-edge pointers, and differential verification against an
independent Dinic oracle.
"""

from collections import deque
import unittest


class FlowEdge:
    __slots__ = ('to', 'rev', 'cap', 'flow')

    def __init__(self, to: int, rev: int, cap: int):
        self.to = to
        self.rev = rev
        self.cap = cap
        self.flow = 0


class PushRelabel:
    """Highest-Label Push-Relabel Maximum Flow Algorithm with Gap Heuristic.
    
    Achieves O(V^2 sqrt(E)) time complexity.
    """

    def __init__(self, n: int):
        self.n = n
        self.g = [[] for _ in range(n)]
        self.excess = [0] * n
        self.height = [0] * n
        self.current_edge = [0] * n
        self.count = [0] * (2 * n + 1)
        self.buckets = [[] for _ in range(2 * n + 1)]
        self.highest_active = 0
        self.s = -1
        self.t = -1

    def add_edge(self, u: int, v: int, cap: int) -> None:
        """Add a directed edge from u to v with capacity cap."""
        assert 0 <= u < self.n and 0 <= v < self.n
        forward = FlowEdge(v, len(self.g[v]), cap)
        backward = FlowEdge(u, len(self.g[u]), 0)
        self.g[u].append(forward)
        self.g[v].append(backward)

    def _enqueue(self, u: int) -> None:
        if u != self.s and u != self.t and self.excess[u] > 0 and self.height[u] < 2 * self.n:
            self.buckets[self.height[u]].append(u)
            self.highest_active = max(self.highest_active, self.height[u])

    def _push(self, u: int, edge: FlowEdge) -> None:
        delta = min(self.excess[u], edge.cap - edge.flow)
        if delta > 0 and self.height[u] == self.height[edge.to] + 1:
            edge.flow += delta
            self.g[edge.to][edge.rev].flow -= delta
            self.excess[u] -= delta
            self.excess[edge.to] += delta
            self._enqueue(edge.to)

    def _gap(self, h: int) -> None:
        for v in range(self.n):
            if v != self.s and v != self.t and self.height[v] > h and self.height[v] < self.n:
                self.count[self.height[v]] -= 1
                self.height[v] = max(self.height[v], self.n + 1)
                self.count[self.height[v]] += 1
                self._enqueue(v)

    def _relabel(self, u: int) -> None:
        self.count[self.height[u]] -= 1
        min_h = 2 * self.n
        for edge in self.g[u]:
            if edge.cap - edge.flow > 0:
                min_h = min(min_h, self.height[edge.to])

        if self.count[self.height[u]] == 0 and self.height[u] < self.n:
            self._gap(self.height[u])

        self.height[u] = min_h + 1
        if self.height[u] < len(self.count):
            self.count[self.height[u]] += 1
        self._enqueue(u)
        self.current_edge[u] = 0

    def _discharge(self, u: int) -> None:
        while self.excess[u] > 0:
            if self.current_edge[u] < len(self.g[u]):
                edge = self.g[u][self.current_edge[u]]
                if edge.cap - edge.flow > 0 and self.height[u] == self.height[edge.to] + 1:
                    self._push(u, edge)
                else:
                    self.current_edge[u] += 1
            else:
                self._relabel(u)
                if self.height[u] >= 2 * self.n:
                    break

    def compute_max_flow(self, s: int, t: int) -> int:
        """Compute the maximum s-t flow value."""
        self.s = s
        self.t = t
        self.height[s] = self.n
        self.count[self.n] = 1
        self.count[0] = self.n - 1

        for edge in self.g[s]:
            if edge.cap > 0:
                flow = edge.cap
                edge.flow += flow
                self.g[edge.to][edge.rev].flow -= flow
                self.excess[s] -= flow
                self.excess[edge.to] += flow
                self._enqueue(edge.to)

        while self.highest_active >= 0:
            if not self.buckets[self.highest_active]:
                self.highest_active -= 1
            else:
                u = self.buckets[self.highest_active].pop()
                self._discharge(u)

        max_flow = sum(self.g[edge.to][edge.rev].flow for edge in self.g[t])
        return max_flow

    def verify_flow_invariants(self, s: int, t: int) -> bool:
        """Verify capacity constraints and flow conservation at all intermediate vertices."""
        # 1. Capacity constraints: 0 <= f(e) <= c(e)
        for u in range(self.n):
            for edge in self.g[u]:
                if edge.cap > 0 and (edge.flow < 0 or edge.flow > edge.cap):
                    return False

        # 2. Flow conservation for intermediate vertices
        for u in range(self.n):
            if u == s or u == t:
                continue
            in_flow = 0
            out_flow = 0
            for edge in self.g[u]:
                if edge.cap > 0:
                    out_flow += edge.flow
                rev = self.g[edge.to][edge.rev]
                if rev.cap > 0:
                    in_flow += rev.flow
            if in_flow != out_flow:
                return False

        return True


class DinicOracle:
    """Independent implementation of Dinic's Algorithm used as test oracle."""

    def __init__(self, n: int):
        self.n = n
        self.g = [[] for _ in range(n)]
        self.level = [-1] * n
        self.ptr = [0] * n

    def add_edge(self, u: int, v: int, cap: int) -> None:
        forward = FlowEdge(v, len(self.g[v]), cap)
        backward = FlowEdge(u, len(self.g[u]), 0)
        self.g[u].append(forward)
        self.g[v].append(backward)

    def _bfs(self, s: int, t: int) -> bool:
        self.level = [-1] * self.n
        self.level[s] = 0
        q = deque([s])
        while q:
            v = q.popleft()
            for edge in self.g[v]:
                if edge.cap - edge.flow > 0 and self.level[edge.to] == -1:
                    self.level[edge.to] = self.level[v] + 1
                    q.append(edge.to)
        return self.level[t] != -1

    def _dfs(self, v: int, t: int, pushed: int) -> int:
        if pushed == 0 or v == t:
            return pushed
        for cid in range(self.ptr[v], len(self.g[v])):
            self.ptr[v] = cid
            edge = self.g[v][cid]
            tr = edge.to
            if self.level[v] + 1 != self.level[tr] or edge.cap - edge.flow == 0:
                continue
            tr_push = self._dfs(tr, t, min(pushed, edge.cap - edge.flow))
            if tr_push == 0:
                continue
            edge.flow += tr_push
            self.g[tr][edge.rev].flow -= tr_push
            return tr_push
        return 0

    def compute_max_flow(self, s: int, t: int) -> int:
        flow = 0
        while self._bfs(s, t):
            self.ptr = [0] * self.n
            while True:
                pushed = self._dfs(s, t, float('inf'))
                if pushed == 0:
                    break
                flow += pushed
        return flow


class TestPushRelabel(unittest.TestCase):
    def test_textbook_network(self):
        pr = PushRelabel(6)
        dinic = DinicOracle(6)

        edges = [
            (0, 1, 16), (0, 2, 13), (1, 2, 10), (1, 3, 12),
            (2, 1, 4), (2, 4, 14), (3, 2, 9), (3, 5, 20),
            (4, 3, 7), (4, 5, 4)
        ]
        for u, v, c in edges:
            pr.add_edge(u, v, c)
            dinic.add_edge(u, v, c)

        pr_flow = pr.compute_max_flow(0, 5)
        dinic_flow = dinic.compute_max_flow(0, 5)

        self.assertEqual(pr_flow, 23)
        self.assertEqual(pr_flow, dinic_flow)
        self.assertTrue(pr.verify_flow_invariants(0, 5))

    def test_disconnected_sink(self):
        pr = PushRelabel(4)
        dinic = DinicOracle(4)

        # 0 -> 1 -> 2, 3 is disconnected
        pr.add_edge(0, 1, 10)
        pr.add_edge(1, 2, 10)
        dinic.add_edge(0, 1, 10)
        dinic.add_edge(1, 2, 10)

        pr_flow = pr.compute_max_flow(0, 3)
        dinic_flow = dinic.compute_max_flow(0, 3)

        self.assertEqual(pr_flow, 0)
        self.assertEqual(pr_flow, dinic_flow)
        self.assertTrue(pr.verify_flow_invariants(0, 3))

    def test_simple_path(self):
        pr = PushRelabel(3)
        dinic = DinicOracle(3)

        pr.add_edge(0, 1, 10)
        pr.add_edge(1, 2, 7)
        dinic.add_edge(0, 1, 10)
        dinic.add_edge(1, 2, 7)

        pr_flow = pr.compute_max_flow(0, 2)
        dinic_flow = dinic.compute_max_flow(0, 2)

        self.assertEqual(pr_flow, 7)
        self.assertEqual(pr_flow, dinic_flow)
        self.assertTrue(pr.verify_flow_invariants(0, 2))

    def test_randomized_differential(self):
        for trial in range(10):
            n = 12
            pr = PushRelabel(n)
            dinic = DinicOracle(n)

            for u in range(n):
                for v in range(u + 1, n):
                    if (u + v + trial) % 3 == 0:
                        cap = (u * 7 + v * 13 + trial * 5) % 40 + 1
                        pr.add_edge(u, v, cap)
                        dinic.add_edge(u, v, cap)

            pr_flow = pr.compute_max_flow(0, n - 1)
            dinic_flow = dinic.compute_max_flow(0, n - 1)

            self.assertEqual(pr_flow, dinic_flow)
            self.assertTrue(pr.verify_flow_invariants(0, n - 1))


if __name__ == '__main__':
    unittest.main()
