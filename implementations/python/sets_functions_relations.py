"""
Sets, Binary Relations, and Function Property Verification.

Provides finite set operations, formal property checkers for binary relations
(reflexivity, symmetry, antisymmetry, transitivity), Warshall's algorithm,
and function classification (injective, surjective, bijective).
"""

import unittest
from typing import Dict, List, Set, Tuple


class BinaryRelation:
    """Represents a binary relation on a finite domain {0, 1, ..., n-1}."""

    def __init__(self, n: int):
        self.n = n
        self.pairs: Set[Tuple[int, int]] = set()

    def add_pair(self, u: int, v: int) -> None:
        self.pairs.add((u, v))

    def has_pair(self, u: int, v: int) -> bool:
        return (u, v) in self.pairs

    def is_reflexive(self) -> bool:
        return all((i, i) in self.pairs for i in range(self.n))

    def is_symmetric(self) -> bool:
        return all((v, u) in self.pairs for (u, v) in self.pairs)

    def is_antisymmetric(self) -> bool:
        return all(u == v or (v, u) not in self.pairs for (u, v) in self.pairs)

    def is_transitive(self) -> bool:
        for u, v in self.pairs:
            for w in range(self.n):
                if (v, w) in self.pairs and (u, w) not in self.pairs:
                    return False
        return True

    def is_equivalence_relation(self) -> bool:
        return self.is_reflexive() and self.is_symmetric() and self.is_transitive()

    def is_partial_order(self) -> bool:
        return self.is_reflexive() and self.is_antisymmetric() and self.is_transitive()

    def transitive_closure(self) -> "BinaryRelation":
        """Computes transitive closure using Warshall's algorithm."""
        reach = [[False] * self.n for _ in range(self.n)]
        for u, v in self.pairs:
            reach[u][v] = True

        for k in range(self.n):
            for i in range(self.n):
                if reach[i][k]:
                    for j in range(self.n):
                        if reach[k][j]:
                            reach[i][j] = True

        closure = BinaryRelation(self.n)
        for i in range(self.n):
            for j in range(self.n):
                if reach[i][j]:
                    closure.add_pair(i, j)
        return closure


def is_injective(mapping: List[int], codomain_size: int) -> bool:
    """Checks whether mapping f is one-to-one."""
    return len(set(mapping)) == len(mapping)


def is_surjective(mapping: List[int], codomain_size: int) -> bool:
    """Checks whether mapping f is onto."""
    return len(set(mapping)) == codomain_size


def is_bijective(mapping: List[int], codomain_size: int) -> bool:
    """Checks whether mapping f is a bijection."""
    return is_injective(mapping, codomain_size) and is_surjective(mapping, codomain_size)


class TestSetsFunctionsRelations(unittest.TestCase):
    def test_equivalence_relation(self):
        # Modulo 2 congruence on {0, 1, 2, 3}
        rel = BinaryRelation(4)
        for i in range(4):
            for j in range(4):
                if (i % 2) == (j % 2):
                    rel.add_pair(i, j)

        self.assertTrue(rel.is_reflexive())
        self.assertTrue(rel.is_symmetric())
        self.assertTrue(rel.is_transitive())
        self.assertTrue(rel.is_equivalence_relation())
        self.assertFalse(rel.is_antisymmetric())

    def test_partial_order(self):
        # <= on {0, 1, 2}
        rel = BinaryRelation(3)
        for i in range(3):
            for j in range(i, 3):
                rel.add_pair(i, j)

        self.assertTrue(rel.is_reflexive())
        self.assertTrue(rel.is_antisymmetric())
        self.assertTrue(rel.is_transitive())
        self.assertTrue(rel.is_partial_order())
        self.assertFalse(rel.is_symmetric())

    def test_transitive_closure(self):
        rel = BinaryRelation(4)
        rel.add_pair(0, 1)
        rel.add_pair(1, 2)
        rel.add_pair(2, 3)

        self.assertFalse(rel.is_transitive())
        closure = rel.transitive_closure()
        self.assertTrue(closure.is_transitive())
        self.assertTrue(closure.has_pair(0, 3))
        self.assertFalse(closure.has_pair(3, 0))

    def test_function_properties(self):
        # f: {0, 1} -> {0, 1, 2} as f = [0, 1]
        self.assertTrue(is_injective([0, 1], 3))
        self.assertFalse(is_surjective([0, 1], 3))

        # Bijection on 3 elements
        self.assertTrue(is_bijective([2, 0, 1], 3))


if __name__ == "__main__":
    unittest.main()
