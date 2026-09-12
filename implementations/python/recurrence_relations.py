"""
Linear Recurrence Relations and Matrix Exponentiation Engine.

Provides an exact companion matrix solver for order-k linear recurrences
operating in O(k^3 log n) time, verified against dynamic programming baselines.
"""

import unittest
from typing import List


def matrix_mult(
    A: List[List[int]], B: List[List[int]], mod: int
) -> List[List[int]]:
    """Multiplies two k x k matrices under modulo arithmetic."""
    k = len(A)
    C = [[0] * k for _ in range(k)]
    for i in range(k):
        for p in range(k):
            if A[i][p] == 0:
                continue
            for j in range(k):
                C[i][j] = (C[i][j] + A[i][p] * B[p][j]) % mod
    return C


def matrix_pow(A: List[List[int]], exp: int, mod: int) -> List[List[int]]:
    """Computes A^exp under modulo arithmetic via binary exponentiation."""
    k = len(A)
    result = [[1 if i == j else 0 for j in range(k)] for i in range(k)]
    base = [row[:] for row in A]
    while exp > 0:
        if exp & 1:
            result = matrix_mult(result, base, mod)
        base = matrix_mult(base, base, mod)
        exp >>= 1
    return result


def solve_linear_recurrence(
    coefficients: List[int], initial_terms: List[int], n: int, mod: int
) -> int:
    """
    Solves order-k recurrence: a_n = sum_{j=0}^{k-1} c[j] * a_{n - 1 - j}
    in O(k^3 log n) time.

    initial_terms are given in descending index order: [a_{k-1}, a_{k-2}, ..., a_0].
    """
    k = len(coefficients)
    if k == 0 or len(initial_terms) != k:
        raise ValueError("Coefficients and initial terms must have equal length > 0")

    if n < k:
        return initial_terms[k - 1 - n] % mod

    # Construct companion matrix M
    M = [[0] * k for _ in range(k)]
    for j in range(k):
        M[0][j] = coefficients[j] % mod
    for i in range(1, k):
        M[i][i - 1] = 1 % mod

    # M^(n - k + 1)
    M_pow = matrix_pow(M, n - k + 1, mod)

    ans = 0
    for j in range(k):
        ans = (ans + M_pow[0][j] * (initial_terms[j] % mod)) % mod
    return ans


def solve_linear_recurrence_dp(
    coefficients: List[int], initial_terms: List[int], n: int, mod: int
) -> int:
    """Dynamic programming verification baseline in O(n * k) time."""
    k = len(coefficients)
    if n < k:
        return initial_terms[k - 1 - n] % mod

    dp = [0] * (n + 1)
    for i in range(k):
        dp[i] = initial_terms[k - 1 - i] % mod

    for i in range(k, n + 1):
        total = 0
        for j in range(k):
            total = (total + coefficients[j] * dp[i - 1 - j]) % mod
        dp[i] = total
    return dp[n]


def fibonacci(n: int, mod: int = 1_000_000_007) -> int:
    """Computes n-th Fibonacci number in O(log n) time."""
    if n == 0:
        return 0
    if n == 1:
        return 1 % mod
    return solve_linear_recurrence([1, 1], [1, 0], n, mod)


def tribonacci(n: int, mod: int = 1_000_000_007) -> int:
    """Computes n-th Tribonacci number in O(log n) time."""
    if n == 0:
        return 0
    if n in (1, 2):
        return 1 % mod
    return solve_linear_recurrence([1, 1, 1], [1, 1, 0], n, mod)


class TestRecurrenceRelations(unittest.TestCase):
    def test_fibonacci_base_cases(self):
        self.assertEqual(fibonacci(0), 0)
        self.assertEqual(fibonacci(1), 1)
        self.assertEqual(fibonacci(2), 1)
        self.assertEqual(fibonacci(3), 2)
        self.assertEqual(fibonacci(10), 55)

    def test_tribonacci_base_cases(self):
        self.assertEqual(tribonacci(0), 0)
        self.assertEqual(tribonacci(1), 1)
        self.assertEqual(tribonacci(2), 1)
        self.assertEqual(tribonacci(3), 2)
        self.assertEqual(tribonacci(4), 4)
        self.assertEqual(tribonacci(5), 7)

    def test_cross_validation_dp(self):
        mod = 1_000_000_007
        c = [2, 3, 1]
        init = [4, 1, 1]
        for n in range(50):
            mat_res = solve_linear_recurrence(c, init, n, mod)
            dp_res = solve_linear_recurrence_dp(c, init, n, mod)
            self.assertEqual(mat_res, dp_res)

    def test_large_n(self):
        mod = 1_000_000_007
        # F(10^18) should compute in a fraction of a second
        res = fibonacci(10**18, mod)
        self.assertEqual(res, 209783453)


if __name__ == "__main__":
    unittest.main()
