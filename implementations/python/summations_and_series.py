"""
Foundational Algorithmic Summations and Series.

Implements closed-form evaluators, Euler-Maclaurin asymptotic bounds,
and Kahan compensated summation with an extensive test suite.
"""

import math
import unittest
from typing import List, Tuple


def sum_integers(n: int) -> int:
    """Computes sum_{i=1}^n i = n(n+1)/2 safely."""
    if n < 0:
        raise ValueError("n must be non-negative")
    return (n * (n + 1)) // 2


def sum_squares(n: int) -> int:
    """Computes sum_{i=1}^n i^2 = n(n+1)(2n+1)/6."""
    if n < 0:
        raise ValueError("n must be non-negative")
    return (n * (n + 1) * (2 * n + 1)) // 6


def sum_cubes(n: int) -> int:
    """Computes sum_{i=1}^n i^3 = (n(n+1)/2)^2."""
    s = sum_integers(n)
    return s * s


def sum_geometric(a: float, r: float, n: int) -> float:
    """Computes finite geometric sum sum_{i=0}^{n-1} a * r^i."""
    if n < 0:
        raise ValueError("n must be non-negative")
    if n == 0:
        return 0.0
    if r == 1.0:
        return a * n
    return a * (1.0 - r**n) / (1.0 - r)


def sum_geometric_infinite(a: float, r: float) -> float:
    """Computes infinite geometric sum a / (1 - r) for |r| < 1."""
    if abs(r) >= 1.0:
        raise ValueError("Infinite geometric series converges only for |r| < 1.0")
    return a / (1.0 - r)


def sum_arithmetico_geometric_2(k: int) -> int:
    """Computes sum_{i=1}^k i * 2^i = (k-1) * 2^(k+1) + 2."""
    if k < 0:
        raise ValueError("k must be non-negative")
    if k == 0:
        return 0
    return (k - 1) * (1 << (k + 1)) + 2


def harmonic_number_exact(n: int) -> float:
    """Computes H_n = sum_{i=1}^n 1/i directly."""
    if n <= 0:
        return 0.0
    return sum(1.0 / i for i in range(1, n + 1))


def harmonic_number_euler_maclaurin(n: int) -> float:
    """
    Computes high-precision asymptotic approximation of H_n:
    H_n ~ ln(n) + gamma + 1/(2n) - 1/(12n^2) + 1/(120n^4)
    """
    if n <= 0:
        return 0.0
    if n < 10:
        return harmonic_number_exact(n)
    EULER_MASCHERONI = 0.5772156649015328606065
    return (
        math.log(n)
        + EULER_MASCHERONI
        + (1.0 / (2.0 * n))
        - (1.0 / (12.0 * n**2))
        + (1.0 / (120.0 * n**4))
    )


def kahan_sum(values: List[float]) -> float:
    """Kahan compensated summation algorithm for numerical stability."""
    total = 0.0
    c = 0.0
    for x in values:
        y = x - c
        t = total + y
        c = (t - total) - y
        total = t
    return total


class TestSummationsAndSeries(unittest.TestCase):
    def test_arithmetic_sum(self):
        for n in range(1, 201):
            expected = sum(range(1, n + 1))
            self.assertEqual(sum_integers(n), expected)

    def test_sum_of_squares(self):
        for n in range(1, 101):
            expected = sum(i**2 for i in range(1, n + 1))
            self.assertEqual(sum_squares(n), expected)

    def test_sum_of_cubes(self):
        for n in range(1, 101):
            expected = sum(i**3 for i in range(1, n + 1))
            self.assertEqual(sum_cubes(n), expected)

    def test_geometric_finite(self):
        a, r, n = 3.0, 0.5, 10
        expected = sum(a * (r**i) for i in range(n))
        self.assertAlmostEqual(sum_geometric(a, r, n), expected, places=9)

    def test_geometric_infinite(self):
        a, r = 5.0, 0.5
        expected = 10.0  # 5 / (1 - 0.5) = 10
        self.assertAlmostEqual(sum_geometric_infinite(a, r), expected, places=9)
        with self.assertRaises(ValueError):
            sum_geometric_infinite(1.0, 1.2)

    def test_arithmetico_geometric(self):
        for k in range(1, 25):
            expected = sum(i * (1 << i) for i in range(1, k + 1))
            self.assertEqual(sum_arithmetico_geometric_2(k), expected)

    def test_harmonic_approximation(self):
        n = 50_000
        exact = harmonic_number_exact(n)
        approx = harmonic_number_euler_maclaurin(n)
        self.assertAlmostEqual(exact, approx, places=8)

    def test_kahan_sum_precision(self):
        # 1.0 + 10^6 * 1e-8 = 1.01
        vals = [1.0] + [1e-8] * 1_000_000
        result = kahan_sum(vals)
        self.assertAlmostEqual(result, 1.01, places=10)


if __name__ == "__main__":
    unittest.main()
