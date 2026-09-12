"""
Elementary Number Theory and Modular Arithmetic Engine.

Implements Euclidean GCD, Extended Euclidean Algorithm, modular inverse,
linear sieve with smallest prime factors (SPF), Euler's totient, and CRT.
"""

import math
import unittest
from typing import List, Tuple


def gcd(a: int, b: int) -> int:
    """Euclidean algorithm for greatest common divisor: O(log min(a, b))."""
    while b != 0:
        a, b = b, a % b
    return abs(a)


def ext_gcd(a: int, b: int) -> Tuple[int, int, int]:
    """
    Extended Euclidean Algorithm.
    Returns (g, x, y) such that a*x + b*y = g = gcd(a, b).
    """
    if b == 0:
        return a, 1, 0
    g, x1, y1 = ext_gcd(b, a % b)
    x = y1
    y = x1 - (a // b) * y1
    return g, x, y


def mod_inverse(a: int, m: int) -> int:
    """Computes modular multiplicative inverse of a modulo m using Extended GCD."""
    g, x, _ = ext_gcd(a, m)
    if g != 1:
        raise ValueError(f"Modular inverse does not exist: gcd({a}, {m}) = {g} != 1")
    return (x % m + m) % m


def linear_sieve(n: int) -> Tuple[List[int], List[int]]:
    """
    Euler's Linear Sieve: computes primes and SPF up to n in strict O(n) time.
    Returns (primes, spf).
    """
    spf = [0] * (n + 1)
    primes = []
    for i in range(2, n + 1):
        if spf[i] == 0:
            spf[i] = i
            primes.append(i)
        for p in primes:
            if p > spf[i] or i * p > n:
                break
            spf[i * p] = p
    return primes, spf


def euler_totient(n: int) -> int:
    """Computes Euler's Totient Function phi(n) in O(sqrt(n)) time."""
    if n <= 0:
        return 0
    res = n
    p = 2
    temp = n
    while p * p <= temp:
        if temp % p == 0:
            while temp % p == 0:
                temp //= p
            res -= res // p
        p += 1
    if temp > 1:
        res -= res // temp
    return res


def chinese_remainder_theorem(remainders: List[int], moduli: List[int]) -> int:
    """
    Solves system x = remainders[i] mod moduli[i] for pairwise coprime moduli
    using the Chinese Remainder Theorem.
    """
    if len(remainders) != len(moduli) or not remainders:
        raise ValueError("Remainders and moduli must be non-empty and of equal length")

    M = 1
    for m in moduli:
        M *= m

    x = 0
    for r, m in zip(remainders, moduli):
        Mi = M // m
        yi = mod_inverse(Mi % m, m)
        x = (x + r * Mi * yi) % M
    return (x % M + M) % M


class TestNumberTheoryBasics(unittest.TestCase):
    def test_gcd(self):
        self.assertEqual(gcd(48, 18), 6)
        self.assertEqual(gcd(101, 10), 1)
        self.assertEqual(gcd(0, 25), 25)

    def test_extended_gcd(self):
        a, b = 35, 15
        g, x, y = ext_gcd(a, b)
        self.assertEqual(g, 5)
        self.assertEqual(a * x + b * y, g)

    def test_modular_inverse(self):
        self.assertEqual(mod_inverse(3, 11), 4)
        self.assertEqual(mod_inverse(7, 26), 15)
        with self.assertRaises(ValueError):
            mod_inverse(6, 9)

    def test_linear_sieve(self):
        primes, spf = linear_sieve(100)
        self.assertEqual(len(primes), 25)
        self.assertEqual(primes[0], 2)
        self.assertEqual(primes[-1], 97)
        self.assertEqual(spf[77], 7)
        self.assertEqual(spf[91], 7)
        self.assertEqual(spf[97], 97)

    def test_euler_totient(self):
        self.assertEqual(euler_totient(1), 1)
        self.assertEqual(euler_totient(2), 1)
        self.assertEqual(euler_totient(6), 2)
        self.assertEqual(euler_totient(9), 6)
        self.assertEqual(euler_totient(12), 4)
        self.assertEqual(euler_totient(13), 12)

    def test_chinese_remainder_theorem(self):
        rems = [2, 3, 2]
        mods = [3, 5, 7]
        sol = chinese_remainder_theorem(rems, mods)
        self.assertEqual(sol, 23)
        for r, m in zip(rems, mods):
            self.assertEqual(sol % m, r)


if __name__ == "__main__":
    unittest.main()
