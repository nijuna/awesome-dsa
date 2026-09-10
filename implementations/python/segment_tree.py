"""
Reference Implementation: Segment Tree with Lazy Propagation
Supports O(N) build, O(log N) range updates, and O(log N) range sum queries.
"""

from __future__ import annotations
import unittest


class LazySegmentTree:
    """Segment Tree supporting range addition updates and range sum queries with Lazy Propagation."""

    def __init__(self, arr: list[int]):
        self.n = len(arr)
        self.tree = [0] * (4 * self.n)
        self.lazy = [0] * (4 * self.n)
        if self.n > 0:
            self._build(arr, 1, 0, self.n - 1)

    def _build(self, arr: list[int], node: int, start: int, end: int):
        if start == end:
            self.tree[node] = arr[start]
            return
        mid = (start + end) // 2
        left = 2 * node
        right = 2 * node + 1
        self._build(arr, left, start, mid)
        self._build(arr, right, mid + 1, end)
        self.tree[node] = self.tree[left] + self.tree[right]

    def _push_down(self, node: int, start: int, end: int):
        """Propagates pending lazy tag to child nodes."""
        if self.lazy[node] != 0:
            val = self.lazy[node]
            mid = (start + end) // 2
            left = 2 * node
            right = 2 * node + 1

            # Update children tree values
            self.tree[left] += val * (mid - start + 1)
            self.tree[right] += val * (end - mid)

            # Accumulate lazy tags on children
            self.lazy[left] += val
            self.lazy[right] += val

            # Clear current node lazy tag
            self.lazy[node] = 0

    def range_update(self, l: int, r: int, val: int):
        """Adds val to all elements in range [l, r] (0-indexed) in O(log N)."""
        if self.n == 0 or l > r or r < 0 or l >= self.n:
            return
        l = max(l, 0)
        r = min(r, self.n - 1)
        self._update(1, 0, self.n - 1, l, r, val)

    def _update(self, node: int, start: int, end: int, l: int, r: int, val: int):
        if l <= start and end <= r:
            self.tree[node] += val * (end - start + 1)
            self.lazy[node] += val
            return

        self._push_down(node, start, end)
        mid = (start + end) // 2
        left = 2 * node
        right = 2 * node + 1

        if l <= mid:
            self._update(left, start, mid, l, r, val)
        if r > mid:
            self._update(right, mid + 1, end, l, r, val)

        self.tree[node] = self.tree[left] + self.tree[right]

    def range_query(self, l: int, r: int) -> int:
        """Returns sum of elements in range [l, r] (0-indexed) in O(log N)."""
        if self.n == 0 or l > r or r < 0 or l >= self.n:
            return 0
        l = max(l, 0)
        r = min(r, self.n - 1)
        return self._query(1, 0, self.n - 1, l, r)

    def _query(self, node: int, start: int, end: int, l: int, r: int) -> int:
        if l <= start and end <= r:
            return self.tree[node]

        self._push_down(node, start, end)
        mid = (start + end) // 2
        left = 2 * node
        right = 2 * node + 1
        total = 0

        if l <= mid:
            total += self._query(left, start, mid, l, r)
        if r > mid:
            total += self._query(right, mid + 1, end, l, r)

        return total


class TestLazySegmentTree(unittest.TestCase):
    def test_basic_queries_and_updates(self):
        arr = [1, 2, 3, 4, 5]
        st = LazySegmentTree(arr)

        self.assertEqual(st.range_query(0, 4), 15)
        self.assertEqual(st.range_query(1, 3), 9)  # 2 + 3 + 4

        # Add 10 to range [1, 3] -> arr becomes [1, 12, 13, 14, 5]
        st.range_update(1, 3, 10)
        self.assertEqual(st.range_query(1, 3), 39)
        self.assertEqual(st.range_query(0, 4), 45)
        self.assertEqual(st.range_query(0, 0), 1)
        self.assertEqual(st.range_query(4, 4), 5)

    def test_edge_cases(self):
        st = LazySegmentTree([100])
        self.assertEqual(st.range_query(0, 0), 100)
        st.range_update(0, 0, 50)
        self.assertEqual(st.range_query(0, 0), 150)
        self.assertEqual(st.range_query(1, 2), 0)


if __name__ == "__main__":
    unittest.main()
