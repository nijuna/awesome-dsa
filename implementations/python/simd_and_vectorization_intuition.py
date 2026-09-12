"""
SIMD and Vectorization Intuition Simulation and Testing in Python.

Implements software simulation of SIMD vector registers (4-lane),
vectorized arithmetic, horizontal reduction, scalar tail loops, and unit tests.
"""

import unittest
from typing import List


class Simd4f:
    """Simulates a 4-lane 32-bit floating point SIMD vector register."""

    def __init__(self, lanes: List[float] = None):
        self.lanes = list(lanes) if lanes else [0.0, 0.0, 0.0, 0.0]

    @classmethod
    def load(cls, data: List[float], offset: int) -> "Simd4f":
        return cls(data[offset : offset + 4])

    def fma(self, a: "Simd4f", b: "Simd4f") -> None:
        """Fused multiply-accumulate: self += a * b"""
        for i in range(4):
            self.lanes[i] += a.lanes[i] * b.lanes[i]

    def horizontal_sum(self) -> float:
        return sum(self.lanes)


def dot_product_scalar(a: List[float], b: List[float]) -> float:
    return sum(x * y for x, y in zip(a, b))


def dot_product_vectorized(a: List[float], b: List[float]) -> float:
    n = len(a)
    acc = Simd4f()
    vector_limit = n - (n % 4)

    # 1. Main Vector Loop
    for i in range(0, vector_limit, 4):
        va = Simd4f.load(a, i)
        vb = Simd4f.load(b, i)
        acc.fma(va, vb)

    # 2. Horizontal Reduction
    total = acc.horizontal_sum()

    # 3. Scalar Cleanup Tail
    for i in range(vector_limit, n):
        total += a[i] * b[i]

    return total


class TestSimdAndVectorization(unittest.TestCase):
    def test_vectorized_dot_product_equivalence(self):
        sizes = [0, 1, 2, 3, 4, 7, 8, 15, 16, 99, 250]
        for n in sizes:
            a = [float(i + 1) * 0.5 for i in range(n)]
            b = [float(i % 5 + 1) * 0.25 for i in range(n)]

            scalar_res = dot_product_scalar(a, b)
            vec_res = dot_product_vectorized(a, b)
            self.assertAlmostEqual(scalar_res, vec_res, places=4, msg=f"Failed at size {n}")

    def test_simd_fma_and_reduction(self):
        v1 = Simd4f([1.0, 2.0, 3.0, 4.0])
        v2 = Simd4f([2.0, 2.0, 2.0, 2.0])
        acc = Simd4f([0.0, 0.0, 0.0, 0.0])

        acc.fma(v1, v2)
        # acc should be [2, 4, 6, 8]
        self.assertEqual(acc.lanes, [2.0, 4.0, 6.0, 8.0])
        self.assertEqual(acc.horizontal_sum(), 20.0)


if __name__ == "__main__":
    unittest.main()
