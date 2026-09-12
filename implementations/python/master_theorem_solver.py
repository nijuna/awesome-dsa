"""
Master Theorem and Akra-Bazzi recurrence solver in Python.

Provides:
1. MasterTheoremSolver for T(n) = a T(n/b) + Theta(n^d log^k n).
2. Akra-Bazzi numerical solver for uneven splits.
3. Unit tests.
"""

import math
import unittest
from typing import List, Tuple, Dict, Any


class MasterTheoremSolver:
    @staticmethod
    def solve(a: float, b: float, d: float, k: int = 0) -> Dict[str, Any]:
        if a < 1.0 or b <= 1.0:
            raise ValueError("Master theorem requires a >= 1 and b > 1")

        log_b_a = math.log(a) / math.log(b)
        EPS = 1e-9

        if log_b_a > d + EPS:
            case = 1
            theta = f"Theta(n^{log_b_a:.3f})"
        elif abs(log_b_a - d) <= EPS:
            case = 2
            if k + 1 == 1:
                theta = f"Theta(n^{d:.0f} * log n)" if d == int(d) else f"Theta(n^{d:.3f} * log n)"
            elif k + 1 > 1:
                theta = f"Theta(n^{d:.0f} * log^{k+1} n)"
            else:
                theta = f"Theta(n^{d:.0f})"
        else:
            case = 3
            if k == 1:
                theta = f"Theta(n^{d:.0f} * log n)"
            elif k > 1:
                theta = f"Theta(n^{d:.0f} * log^{k} n)"
            else:
                theta = f"Theta(n^{d:.0f})"

        return {
            "case": case,
            "log_b_a": log_b_a,
            "theta": theta
        }

    @staticmethod
    def solve_akra_bazzi_p(terms: List[Tuple[float, float]]) -> float:
        """Solves sum(a_i * (b_i)^p) = 1 for uneven divide and conquer."""
        low, high = -10.0, 10.0
        for _ in range(100):
            mid = (low + high) / 2.0
            val = sum(a * (b ** mid) for a, b in terms)
            if val > 1.0:
                low = mid
            else:
                high = mid
        return (low + high) / 2.0


class TestMasterTheoremSolver(unittest.TestCase):
    def test_merge_sort(self):
        res = MasterTheoremSolver.solve(2, 2, 1, 0)
        self.assertEqual(res["case"], 2)
        self.assertAlmostEqual(res["log_b_a"], 1.0)

    def test_binary_search(self):
        res = MasterTheoremSolver.solve(1, 2, 0, 0)
        self.assertEqual(res["case"], 2)
        self.assertAlmostEqual(res["log_b_a"], 0.0)

    def test_strassen(self):
        res = MasterTheoremSolver.solve(7, 2, 2, 0)
        self.assertEqual(res["case"], 1)
        self.assertAlmostEqual(res["log_b_a"], math.log2(7), places=3)

    def test_akra_bazzi(self):
        # T(n) = T(n/3) + T(2n/3) + O(n)
        terms = [(1.0, 1.0 / 3.0), (1.0, 2.0 / 3.0)]
        p = MasterTheoremSolver.solve_akra_bazzi_p(terms)
        self.assertAlmostEqual(p, 1.0, places=4)


if __name__ == '__main__':
    unittest.main()
