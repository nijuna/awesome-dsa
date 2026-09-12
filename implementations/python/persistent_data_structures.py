"""
Persistent Segment Tree via Path Copying in Python.

Implements immutable versions, point updates with O(log N) path copying,
range sum queries, and differential verification against a versioned list oracle.
"""

import random
import unittest
from typing import List, Optional, Tuple


class Node:
    __slots__ = ("left", "right", "sum")

    def __init__(self, left: int = 0, right: int = 0, total_sum: int = 0):
        self.left = left
        self.right = right
        self.sum = total_sum


class PersistentSegmentTree:
    """Persistent Segment Tree supporting historical point updates and range queries."""

    def __init__(self, initial_array: List[int]):
        self.n = len(initial_array)
        self.pool: List[Node] = [Node()]  # Index 0 is null sentinel
        self.roots: List[int] = []

        if self.n > 0:
            root = self._build(initial_array, 0, self.n - 1)
            self.roots.append(root)  # Version 0

    def _allocate_node(self) -> int:
        self.pool.append(Node())
        return len(self.pool) - 1

    def _build(self, arr: List[int], l: int, r: int) -> int:
        idx = self._allocate_node()
        if l == r:
            self.pool[idx].sum = arr[l]
            return idx

        mid = (l + r) // 2
        left_child = self._build(arr, l, mid)
        right_child = self._build(arr, mid + 1, r)
        self.pool[idx].left = left_child
        self.pool[idx].right = right_child
        self.pool[idx].sum = self.pool[left_child].sum + self.pool[right_child].sum
        return idx

    def _update_recursive(self, prev_node: int, l: int, r: int, target_idx: int, new_val: int) -> int:
        curr_node = self._allocate_node()
        # Path copy: clone previous node
        self.pool[curr_node].left = self.pool[prev_node].left
        self.pool[curr_node].right = self.pool[prev_node].right

        if l == r:
            self.pool[curr_node].sum = new_val
            return curr_node

        mid = (l + r) // 2
        if target_idx <= mid:
            self.pool[curr_node].left = self._update_recursive(
                self.pool[prev_node].left, l, mid, target_idx, new_val
            )
        else:
            self.pool[curr_node].right = self._update_recursive(
                self.pool[prev_node].right, mid + 1, r, target_idx, new_val
            )

        self.pool[curr_node].sum = (
            self.pool[self.pool[curr_node].left].sum + self.pool[self.pool[curr_node].right].sum
        )
        return curr_node

    def update(self, base_version: int, target_idx: int, new_val: int) -> int:
        """Creates a new version by updating target_idx in base_version. Returns new version index."""
        assert 0 <= base_version < len(self.roots)
        assert 0 <= target_idx < self.n
        new_root = self._update_recursive(self.roots[base_version], 0, self.n - 1, target_idx, new_val)
        self.roots.append(new_root)
        return len(self.roots) - 1

    def _query_recursive(self, node: int, l: int, r: int, ql: int, qr: int) -> int:
        if node == 0 or ql > r or qr < l:
            return 0
        if ql <= l and r <= qr:
            return self.pool[node].sum

        mid = (l + r) // 2
        return self._query_recursive(self.pool[node].left, l, mid, ql, qr) + self._query_recursive(
            self.pool[node].right, mid + 1, r, ql, qr
        )

    def query(self, version: int, ql: int, qr: int) -> int:
        """Queries range sum [ql, qr] in a specific historical version."""
        assert 0 <= version < len(self.roots)
        assert 0 <= ql <= qr < self.n
        return self._query_recursive(self.roots[version], 0, self.n - 1, ql, qr)

    def try_query(self, version: int, ql: int, qr: int) -> Tuple[bool, Optional[int]]:
        """Safe query adapter returning (success, result)."""
        if version < 0 or version >= len(self.roots) or ql < 0 or qr >= self.n or ql > qr:
            return False, None
        return True, self._query_recursive(self.roots[version], 0, self.n - 1, ql, qr)

    @property
    def version_count(self) -> int:
        return len(self.roots)


class TestPersistentSegmentTree(unittest.TestCase):
    def test_versioning_and_immutability(self):
        initial = [1, 2, 3, 4, 5, 6, 7, 8]
        pst = PersistentSegmentTree(initial)

        self.assertEqual(pst.version_count, 1)
        self.assertEqual(pst.query(0, 0, 7), 36)
        self.assertEqual(pst.query(0, 2, 4), 12)

        # Version 1: update index 2 (3 -> 10)
        v1 = pst.update(0, 2, 10)
        self.assertEqual(v1, 1)

        # Version 2: update index 7 (8 -> 0) on Version 1
        v2 = pst.update(v1, 7, 0)
        self.assertEqual(v2, 2)

        # Version 3: branch off Version 0, update index 0 (1 -> 100)
        v3 = pst.update(0, 0, 100)
        self.assertEqual(v3, 3)

        # Invariant: Version 0 remains 100% immutable
        self.assertEqual(pst.query(0, 0, 7), 36)
        self.assertEqual(pst.query(0, 2, 2), 3)

        # Check other versions
        self.assertEqual(pst.query(v1, 0, 7), 43)
        self.assertEqual(pst.query(v2, 0, 7), 35)
        self.assertEqual(pst.query(v3, 0, 7), 135)

        # Safe adapter test
        ok, val = pst.try_query(v1, 2, 4)
        self.assertTrue(ok)
        self.assertEqual(val, 19)

        bad_ok, _ = pst.try_query(999, 0, 1)
        self.assertFalse(bad_ok)

    def test_differential_oracle_stress(self):
        rng = random.Random(42)
        initial = [rng.randint(-50, 50) for _ in range(16)]
        pst = PersistentSegmentTree(initial)
        oracle = [list(initial)]

        for _ in range(200):
            base_v = rng.randint(0, len(oracle) - 1)
            target_idx = rng.randint(0, len(initial) - 1)
            new_val = rng.randint(-100, 100)

            new_v = pst.update(base_v, target_idx, new_val)

            new_oracle_vec = list(oracle[base_v])
            new_oracle_vec[target_idx] = new_val
            oracle.append(new_oracle_vec)
            self.assertEqual(new_v, len(oracle) - 1)

            # Sample query checks
            for _ in range(2):
                query_v = rng.randint(0, len(oracle) - 1)
                ql = rng.randint(0, len(initial) - 1)
                qr = rng.randint(0, len(initial) - 1)
                if ql > qr:
                    ql, qr = qr, ql

                pst_sum = pst.query(query_v, ql, qr)
                oracle_sum = sum(oracle[query_v][ql : qr + 1])
                self.assertEqual(pst_sum, oracle_sum)


if __name__ == "__main__":
    unittest.main()
