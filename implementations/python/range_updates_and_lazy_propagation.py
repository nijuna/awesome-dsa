"""
Segment Tree with Lazy Propagation implementation in Python.

Supports:
1. Range Add update: add X to all elements in [l, r] in O(log n).
2. Range Set update: set all elements in [l, r] to X in O(log n).
3. Range Sum Query in O(log n).
4. Range Min Query in O(log n).
5. Comprehensive unit tests.
"""

import unittest
from typing import List, Optional


class LazySegmentTree:
    """Segment Tree with lazy propagation supporting range add, range set, range sum, and range min."""

    def __init__(self, a: List[int]):
        self.n = len(a)
        size = 4 * max(1, self.n)
        self.tree_sum = [0] * size
        self.tree_min = [float('inf')] * size
        self.lazy_add = [0] * size
        self.lazy_set = [0] * size
        self.has_set = [False] * size

        if self.n > 0:
            self._build(1, 0, self.n - 1, a)

    def _apply_set(self, node: int, l: int, r: int, val: int) -> None:
        self.has_set[node] = True
        self.lazy_set[node] = val
        self.lazy_add[node] = 0
        self.tree_sum[node] = val * (r - l + 1)
        self.tree_min[node] = val

    def _apply_add(self, node: int, l: int, r: int, val: int) -> None:
        if self.has_set[node]:
            self.lazy_set[node] += val
        else:
            self.lazy_add[node] += val
        self.tree_sum[node] += val * (r - l + 1)
        self.tree_min[node] += val

    def _push_down(self, node: int, l: int, r: int) -> None:
        if l == r:
            return
        mid = (l + r) // 2
        lc, rc = 2 * node, 2 * node + 1

        if self.has_set[node]:
            self._apply_set(lc, l, mid, self.lazy_set[node])
            self._apply_set(rc, mid + 1, r, self.lazy_set[node])
            self.has_set[node] = False
            self.lazy_set[node] = 0

        if self.lazy_add[node] != 0:
            self._apply_add(lc, l, mid, self.lazy_add[node])
            self._apply_add(rc, mid + 1, r, self.lazy_add[node])
            self.lazy_add[node] = 0

    def _push_up(self, node: int) -> None:
        self.tree_sum[node] = self.tree_sum[2 * node] + self.tree_sum[2 * node + 1]
        self.tree_min[node] = min(self.tree_min[2 * node], self.tree_min[2 * node + 1])

    def _build(self, node: int, l: int, r: int, a: List[int]) -> None:
        if l == r:
            self.tree_sum[node] = a[l]
            self.tree_min[node] = a[l]
            return
        mid = (l + r) // 2
        self._build(2 * node, l, mid, a)
        self._build(2 * node + 1, mid + 1, r, a)
        self._push_up(node)

    def range_add(self, ql: int, qr: int, val: int) -> None:
        self._range_add(1, 0, self.n - 1, ql, qr, val)

    def _range_add(self, node: int, l: int, r: int, ql: int, qr: int, val: int) -> None:
        if ql <= l and r <= qr:
            self._apply_add(node, l, r, val)
            return
        self._push_down(node, l, r)
        mid = (l + r) // 2
        if ql <= mid:
            self._range_add(2 * node, l, mid, ql, qr, val)
        if qr > mid:
            self._range_add(2 * node + 1, mid + 1, r, ql, qr, val)
        self._push_up(node)

    def range_set(self, ql: int, qr: int, val: int) -> None:
        self._range_set(1, 0, self.n - 1, ql, qr, val)

    def _range_set(self, node: int, l: int, r: int, ql: int, qr: int, val: int) -> None:
        if ql <= l and r <= qr:
            self._apply_set(node, l, r, val)
            return
        self._push_down(node, l, r)
        mid = (l + r) // 2
        if ql <= mid:
            self._range_set(2 * node, l, mid, ql, qr, val)
        if qr > mid:
            self._range_set(2 * node + 1, mid + 1, r, ql, qr, val)
        self._push_up(node)

    def query_sum(self, ql: int, qr: int) -> int:
        return self._query_sum(1, 0, self.n - 1, ql, qr)

    def _query_sum(self, node: int, l: int, r: int, ql: int, qr: int) -> int:
        if ql <= l and r <= qr:
            return self.tree_sum[node]
        self._push_down(node, l, r)
        mid = (l + r) // 2
        res = 0
        if ql <= mid:
            res += self._query_sum(2 * node, l, mid, ql, qr)
        if qr > mid:
            res += self._query_sum(2 * node + 1, mid + 1, r, ql, qr)
        return res

    def query_min(self, ql: int, qr: int) -> int:
        return self._query_min(1, 0, self.n - 1, ql, qr)

    def _query_min(self, node: int, l: int, r: int, ql: int, qr: int) -> int:
        if ql <= l and r <= qr:
            return self.tree_min[node]
        self._push_down(node, l, r)
        mid = (l + r) // 2
        res = float('inf')
        if ql <= mid:
            res = min(res, self._query_min(2 * node, l, mid, ql, qr))
        if qr > mid:
            res = min(res, self._query_min(2 * node + 1, mid + 1, r, ql, qr))
        return res


class TestLazySegmentTree(unittest.TestCase):
    def test_lazy_segment_tree(self):
        a = [1, 2, 3, 4, 5, 6, 7, 8]
        tree = LazySegmentTree(a)

        self.assertEqual(tree.query_sum(0, 7), 36)
        self.assertEqual(tree.query_min(0, 7), 1)
        self.assertEqual(tree.query_sum(2, 4), 12)

        # Range add +10 to [1, 3] -> [1, 12, 13, 14, 5, 6, 7, 8]
        tree.range_add(1, 3, 10)
        self.assertEqual(tree.query_sum(1, 3), 39)
        self.assertEqual(tree.query_min(1, 3), 12)
        self.assertEqual(tree.query_sum(0, 7), 66)

        # Range set [2, 5] to 0 -> [1, 12, 0, 0, 0, 0, 7, 8]
        tree.range_set(2, 5, 0)
        self.assertEqual(tree.query_sum(2, 5), 0)
        self.assertEqual(tree.query_min(2, 5), 0)
        self.assertEqual(tree.query_sum(0, 7), 28)

        # Range add +5 to [3, 6] -> [1, 12, 0, 5, 5, 5, 12, 8]
        tree.range_add(3, 6, 5)
        self.assertEqual(tree.query_sum(3, 6), 27)
        self.assertEqual(tree.query_min(3, 6), 5)
        self.assertEqual(tree.query_sum(0, 7), 48)


if __name__ == '__main__':
    unittest.main()
