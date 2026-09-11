"""
Reference Implementation: Disjoint Set Union (Union-Find)
Demonstrates:
1. Standard DSU with path compression and union by size (near-constant amortized O(alpha(n))).
2. Rollback DSU with an undo stack for offline dynamic connectivity (O(log n) find, O(1) rollback).
3. Parity / Potential DSU for bipartite graph consistency and 2-coloring constraints.

Language: Python 3
"""

from typing import List, Tuple, Optional
import unittest


class DisjointSetUnion:
    """Standard DSU with recursive path compression and union by size."""

    def __init__(self, n: int):
        self.parent: List[int] = list(range(n))
        self.size: List[int] = [1] * n
        self.num_components: int = n

    def find(self, x: int) -> int:
        if self.parent[x] != x:
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]

    def unite(self, a: int, b: int) -> bool:
        root_a = self.find(a)
        root_b = self.find(b)

        if root_a == root_b:
            return False

        if self.size[root_a] < self.size[root_b]:
            root_a, root_b = root_b, root_a

        self.parent[root_b] = root_a
        self.size[root_a] += self.size[root_b]
        self.num_components -= 1
        return True

    def same(self, a: int, b: int) -> bool:
        return self.find(a) == self.find(b)

    def size_of(self, x: int) -> int:
        return self.size[self.find(x)]


class RollbackDSU:
    """Rollback DSU with union by size and an undo stack, without path compression."""

    def __init__(self, n: int):
        self.parent: List[int] = list(range(n))
        self.size: List[int] = [1] * n
        self.history: List[Tuple[int, int, int]] = []  # (child, parent, old_parent_size)
        self.num_components: int = n

    def find(self, x: int) -> int:
        while self.parent[x] != x:
            x = self.parent[x]
        return x

    def unite(self, a: int, b: int) -> bool:
        root_a = self.find(a)
        root_b = self.find(b)

        if root_a == root_b:
            return False

        if self.size[root_a] < self.size[root_b]:
            root_a, root_b = root_b, root_a

        self.history.append((root_b, root_a, self.size[root_a]))
        self.parent[root_b] = root_a
        self.size[root_a] += self.size[root_b]
        self.num_components -= 1
        return True

    def same(self, a: int, b: int) -> bool:
        return self.find(a) == self.find(b)

    def checkpoint(self) -> int:
        return len(self.history)

    def rollback_to(self, cp: int) -> None:
        while len(self.history) > cp:
            child, parent, old_size = self.history.pop()
            self.parent[child] = child
            self.size[parent] = old_size
            self.num_components += 1


class ParityDSU:
    """Parity DSU tracking 2-coloring relations for bipartite constraint verification."""

    def __init__(self, n: int):
        self.parent: List[int] = list(range(n))
        self.parity: List[int] = [0] * n  # 0: same color as parent, 1: opposite
        self.size: List[int] = [1] * n
        self.is_bipartite: bool = True

    def find(self, x: int) -> int:
        if self.parent[x] != x:
            orig = self.parent[x]
            self.parent[x] = self.find(self.parent[x])
            self.parity[x] ^= self.parity[orig]
        return self.parent[x]

    def add_relation(self, a: int, b: int, rel: int) -> bool:
        root_a = self.find(a)
        root_b = self.find(b)

        if root_a == root_b:
            if (self.parity[a] ^ self.parity[b]) != rel:
                self.is_bipartite = False
                return False
            return True

        if self.size[root_a] < self.size[root_b]:
            root_a, root_b = root_b, root_a
            a, b = b, a

        self.parent[root_b] = root_a
        self.parity[root_b] = self.parity[a] ^ self.parity[b] ^ rel
        self.size[root_a] += self.size[root_b]
        return True


class TestDisjointSetUnion(unittest.TestCase):
    def test_standard_dsu(self):
        dsu = DisjointSetUnion(10)
        self.assertEqual(dsu.num_components, 10)
        for i in range(10):
            self.assertEqual(dsu.size_of(i), 1)
            self.assertEqual(dsu.find(i), i)

        self.assertTrue(dsu.unite(1, 2))
        self.assertTrue(dsu.unite(2, 3))
        self.assertTrue(dsu.same(1, 3))
        self.assertFalse(dsu.same(1, 4))
        self.assertEqual(dsu.size_of(1), 3)
        self.assertEqual(dsu.num_components, 8)

        self.assertFalse(dsu.unite(1, 3))  # Cycle
        self.assertEqual(dsu.num_components, 8)

        self.assertTrue(dsu.unite(4, 5))
        self.assertTrue(dsu.unite(3, 4))
        self.assertTrue(dsu.same(1, 5))
        self.assertEqual(dsu.size_of(5), 5)
        self.assertEqual(dsu.num_components, 6)

    def test_rollback_dsu(self):
        rdsu = RollbackDSU(6)
        self.assertEqual(rdsu.num_components, 6)

        rdsu.unite(0, 1)
        rdsu.unite(1, 2)
        self.assertTrue(rdsu.same(0, 2))
        self.assertEqual(rdsu.num_components, 4)

        cp = rdsu.checkpoint()

        rdsu.unite(3, 4)
        rdsu.unite(4, 5)
        rdsu.unite(2, 5)
        self.assertTrue(rdsu.same(0, 5))
        self.assertEqual(rdsu.num_components, 1)

        rdsu.rollback_to(cp)
        self.assertTrue(rdsu.same(0, 2))
        self.assertFalse(rdsu.same(0, 5))
        self.assertFalse(rdsu.same(3, 4))
        self.assertEqual(rdsu.num_components, 4)

        rdsu.rollback_to(0)
        self.assertFalse(rdsu.same(0, 1))
        self.assertEqual(rdsu.num_components, 6)

    def test_parity_dsu(self):
        pdsu = ParityDSU(5)
        self.assertTrue(pdsu.add_relation(0, 1, 1))
        self.assertTrue(pdsu.add_relation(1, 2, 1))
        self.assertTrue(pdsu.add_relation(2, 3, 1))
        self.assertTrue(pdsu.add_relation(3, 0, 1))  # Even cycle
        self.assertTrue(pdsu.is_bipartite)

        # Contradiction: odd cycle
        self.assertFalse(pdsu.add_relation(0, 2, 1))
        self.assertFalse(pdsu.is_bipartite)


if __name__ == "__main__":
    unittest.main()
