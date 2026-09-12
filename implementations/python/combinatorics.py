"""
Combinatorics and Discrete Enumeration Engine.

Provides modular factorials, combinations, permutations, Catalan numbers,
Lucas' theorem for large n, and derangements with full unit test coverage.
"""

import math
import unittest
from typing import List


class ModularCombinatorics:
    """Precomputes factorials and inverse factorials for O(1) query answering."""

    def __init__(self, max_n: int, mod: int = 1_000_000_007):
        self.max_n = max_n
        self.mod = mod
        self.fact = [1] * (max_n + 1)
        self.inv_fact = [1] * (max_n + 1)

        for i in range(1, max_n + 1):
            self.fact[i] = (self.fact[i - 1] * i) % mod

        self.inv_fact[max_n] = pow(self.fact[max_n], mod - 2, mod)

        for i in range(max_n, 0, -1):
            self.inv_fact[i - 1] = (self.inv_fact[i] * i) % mod

    def nCr(self, n: int, r: int) -> int:
        """Computes nCr modulo mod in O(1) time."""
        if r < 0 or r > n:
            return 0
        if n > self.max_n:
            raise ValueError(f"n={n} exceeds precomputed max_n={self.max_n}")
        num = self.fact[n]
        den = (self.inv_fact[r] * self.inv_fact[n - r]) % self.mod
        return (num * den) % self.mod

    def nPr(self, n: int, r: int) -> int:
        """Computes nPr modulo mod in O(1) time."""
        if r < 0 or r > n:
            return 0
        if n > self.max_n:
            raise ValueError(f"n={n} exceeds precomputed max_n={self.max_n}")
        return (self.fact[n] * self.inv_fact[n - r]) % self.mod

    def catalan(self, n: int) -> int:
        """Computes n-th Catalan number C_n modulo mod in O(1) time."""
        if 2 * n > self.max_n:
            raise ValueError(f"2n={2*n} exceeds precomputed max_n={self.max_n}")
        c2n_n = self.nCr(2 * n, n)
        inv_n1 = pow(n + 1, self.mod - 2, self.mod)
        return (c2n_n * inv_n1) % self.mod

    def stars_and_bars(self, n: int, k: int) -> int:
        """Non-negative integer solutions to x_1 + ... + x_k = n: C(n + k - 1, k - 1)."""
        if k == 0:
            return 1 if n == 0 else 0
        return self.nCr(n + k - 1, k - 1)


def nCr_lucas(n: int, r: int, prime_mod: int) -> int:
    """Computes C(n, r) modulo prime_mod for large n, r via Lucas' Theorem."""
    if r < 0 or r > n:
        return 0
    small_comb = ModularCombinatorics(prime_mod - 1, prime_mod)
    ans = 1
    while n > 0 or r > 0:
        ni = n % prime_mod
        ri = r % prime_mod
        if ri > ni:
            return 0
        ans = (ans * small_comb.nCr(ni, ri)) % prime_mod
        n //= prime_mod
        r //= prime_mod
    return ans


def derangements(n: int, mod: int = 1_000_000_007) -> int:
    """Computes number of derangements D_n modulo mod in O(n) time."""
    if n == 0:
        return 1 % mod
    if n == 1:
        return 0
    prev2, prev1 = 1 % mod, 0
    cur = 0
    for i in range(2, n + 1):
        cur = ((i - 1) * (prev1 + prev2)) % mod
        prev2 = prev1
        prev1 = cur
    return cur


class TestCombinatorics(unittest.TestCase):
    def setUp(self):
        self.comb = ModularCombinatorics(50_000)

    def test_nCr_small_values(self):
        for n in range(15):
            for r in range(n + 1):
                expected = math.comb(n, r) % 1_000_000_007
                self.assertEqual(self.comb.nCr(n, r), expected)

    def test_pascal_identity(self):
        for n in range(2, 50):
            for r in range(1, n):
                lhs = self.comb.nCr(n, r)
                rhs = (self.comb.nCr(n - 1, r - 1) + self.comb.nCr(n - 1, r)) % 1_000_000_007
                self.assertEqual(lhs, rhs)

    def test_catalan_numbers(self):
        expected = [1, 1, 2, 5, 14, 42, 132, 429, 1430, 4862]
        for i, val in enumerate(expected):
            self.assertEqual(self.comb.catalan(i), val)

    def test_stars_and_bars(self):
        # 4 balls into 3 bins: C(4 + 3 - 1, 3 - 1) = C(6, 2) = 15
        self.assertEqual(self.comb.stars_and_bars(4, 3), 15)

    def test_lucas_theorem(self):
        p = 11
        # C(15, 6) mod 11
        # 15 = 1*11 + 4, 6 = 0*11 + 6 -> C(1, 0) * C(4, 6) = 1 * 0 = 0
        self.assertEqual(nCr_lucas(15, 6, p), 0)
        # C(25, 12) = 5,200,300 -> 5200300 % 11 = 9
        # 25 = 2*11 + 3, 12 = 1*11 + 1 -> C(2, 1) * C(3, 1) = 2 * 3 = 6 wait
        # 5200300 = 11 * 472754 + 6 -> 6!
        self.assertEqual(nCr_lucas(25, 12, p), 6)

    def test_derangements(self):
        expected = [1, 0, 1, 2, 9, 44, 265, 1854]
        for i, val in enumerate(expected):
            self.assertEqual(derangements(i), val)


if __name__ == "__main__":
    unittest.main()
