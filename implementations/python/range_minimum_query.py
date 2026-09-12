"""
Range Minimum Query (RMQ) implementations in Python.

Provides:
1. StaticRMQ via Sparse Table (O(1) query).
2. DynamicRMQ via Segment Tree (O(log n) query and point update).
3. build_cartesian_tree via monotonic stack in O(n).
4. Comprehensive unit tests.
"""

import math
import unittest
from typing import List, Optional


class StaticRMQ:
    """Static RMQ via Sparse Table storing indices of minimums with O(1) query."""

    def __init__(self, a: List[int]):
        self.a = list(a)
        n = len(self.a)
        self.log2 = [0] * (n + 1)
        for i in range(2, n + 1):
            self.log2[i] = self.log2[i // 2] + 1

        K = self.log2[n] + 1 if n > 0 else 0
        self.st = [[0] * n for _ in range(K)]

        if n > 0:
            for i in range(n):
                self.st[0][i] = i

            k = 1
            while (1 << k) <= n:
                length = 1 << k
                half = length >> 1
                for i in range(n - length + 1):
                    left_idx = self.st[k - 1][i]
                    right_idx = self.st[k - 1][i + half]
                    self.st[k][i] = left_idx if self.a[left_idx] <= self.a[right_idx] else right_idx
                k += 1

    def query_index(self, l: int, r: int) -> int:
        length = r - l + 1
        k = self.log2[length]
        left_idx = self.st[k][l]
        right_idx = self.st[k][r - (1 << k) + 1]
        return left_idx if self.a[left_idx] <= self.a[right_idx] else right_idx

    def query_value(self, l: int, r: int) -> int:
        return self.a[self.query_index(l, r)]


class DynamicRMQ:
    """Dynamic RMQ via Segment Tree supporting point updates."""

    def __init__(self, a: List[int]):
        self.n = len(a)
        self.tree = [float('inf')] * (4 * max(1, self.n))
        if self.n > 0:
            self._build(1, 0, self.n - 1, a)

    def _build(self, node: int, left: int, right: int, a: List[int]) -> None:
        if left == right:
            self.tree[node] = a[left]
            return
        mid = (left + right) // 2
        self._build(2 * node, left, mid, a)
        self._build(2 * node + 1, mid + 1, right, a)
        self.tree[node] = min(self.tree[2 * node], self.tree[2 * node + 1])

    def query(self, l: int, r: int) -> int:
        return self._query(1, 0, self.n - 1, l, r)

    def _query(self, node: int, left: int, right: int, ql: int, qr: int) -> int:
        if qr < left or right < ql:
            return float('inf')
        if ql <= left and right <= qr:
            return self.tree[node]
        mid = (left + right) // 2
        return min(self._query(2 * node, left, mid, ql, qr),
                   self._query(2 * node + 1, mid + 1, right, ql, qr))

    def update(self, idx: int, val: int) -> None:
        self._update(1, 0, self.n - 1, idx, val)

    def _update(self, node: int, left: int, right: int, idx: int, val: int) -> None:
        if left == right:
            self.tree[node] = val
            return
        mid = (left + right) // 2
        if idx <= mid:
            self._update(2 * node, left, mid, idx, val)
        else:
            self._update(2 * node + 1, mid + 1, right, idx, val)
        self.tree[node] = min(self.tree[2 * node], self.tree[2 * node + 1])


class CartesianNode:
    def __init__(self, val: int, index: int):
        self.val = val
        self.index = index
        self.left: Optional[int] = None
        self.right: Optional[int] = None
        self.parent: Optional[int] = None


def build_cartesian_tree(a: List[int]) -> Tuple[int, List[CartesianNode]]:
    """Constructs min-Cartesian tree in linear O(n) time using monotonic stack."""
    n = len(a)
    nodes = [CartesianNode(a[i], i) for i in range(n)]
    stack: List[int] = []

    for i in range(n):
        last_popped = None
        while stack and nodes[stack[-1]].val > a[i]:
            last_popped = stack.pop()
        if last_popped is not None:
            nodes[i].left = last_popped
            nodes[last_popped].parent = i
        if stack:
            nodes[stack[-1]].right = i
            nodes[i].parent = stack[-1]
        stack.append(i)

    root = -1
    for i in range(n):
        if nodes[i].parent is None:
            root = i
            break
    return root, nodes


class TestRangeMinimumQuery(unittest.TestCase):
    def test_static_rmq(self):
        a = [7, 2, 3, 0, 5, 10, 3, 12, 18]
        rmq = StaticRMQ(a)
        self.assertEqual(rmq.query_value(0, 4), 0)
        self.assertEqual(rmq.query_index(0, 4), 3)
        self.assertEqual(rmq.query_value(4, 7), 3)
        self.assertEqual(rmq.query_index(4, 7), 6)
        self.assertEqual(rmq.query_value(1, 2), 2)
        self.assertEqual(rmq.query_index(1, 2), 1)

    def test_dynamic_rmq(self):
        a = [5, 8, 6, 3, 2, 7]
        seg = DynamicRMQ(a)
        self.assertEqual(seg.query(0, 5), 2)
        self.assertEqual(seg.query(0, 2), 5)
        seg.update(4, 9)
        self.assertEqual(seg.query(0, 5), 3)
        seg.update(1, -1)
        self.assertEqual(seg.query(0, 2), -1)
        self.assertEqual(seg.query(0, 5), -1)

    def test_cartesian_tree(self):
        a = [9, 3, 7, 1, 8, 12, 10, 20, 15, 18, 5]
        root, nodes = build_cartesian_tree(a)
        self.assertEqual(root, 3)
        self.assertEqual(nodes[root].val, 1)


if __name__ == '__main__':
    unittest.main()
