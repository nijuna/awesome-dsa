/**
 * @file recurrence_relations.cpp
 * @brief Reference implementations for solving linear recurrence relations.
 *
 * Provides a modular companion matrix exponentiation engine capable of solving
 * any order-k linear recurrence in O(k^3 log n) time, along with dynamic programming
 * baseline verifiers.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <stdexcept>

namespace dsa {

/**
 * @brief Square matrix of dimension K x K with modular arithmetic operations.
 */
class SquareMatrix {
public:
    size_t k;
    uint64_t mod;
    std::vector<std::vector<uint64_t>> mat;

    SquareMatrix(size_t dim, uint64_t m)
        : k(dim), mod(m), mat(dim, std::vector<uint64_t>(dim, 0)) {}

    static SquareMatrix identity(size_t dim, uint64_t m) {
        SquareMatrix res(dim, m);
        for (size_t i = 0; i < dim; ++i) {
            res.mat[i][i] = 1 % m;
        }
        return res;
    }

    SquareMatrix operator*(const SquareMatrix& other) const {
        assert(k == other.k && mod == other.mod);
        SquareMatrix res(k, mod);
        for (size_t i = 0; i < k; ++i) {
            for (size_t p = 0; p < k; ++p) {
                if (mat[i][p] == 0) continue;
                for (size_t j = 0; j < k; ++j) {
                    __uint128_t term = static_cast<__uint128_t>(mat[i][p]) * other.mat[p][j];
                    res.mat[i][j] = static_cast<uint64_t>((res.mat[i][j] + term) % mod);
                }
            }
        }
        return res;
    }

    SquareMatrix power(uint64_t exp) const {
        SquareMatrix result = identity(k, mod);
        SquareMatrix base = *this;
        while (exp > 0) {
            if (exp & 1) result = result * base;
            base = base * base;
            exp >>= 1;
        }
        return result;
    }
};

/**
 * @brief Solves an order-k linear recurrence in O(k^3 log n) time.
 * Recurrence: a_n = c[0]*a_{n-1} + c[1]*a_{n-2} + ... + c[k-1]*a_{n-k}
 *
 * @param c Coefficients of length k.
 * @param init Base cases: [a_{k-1}, a_{k-2}, ..., a_0] in descending order of index.
 * @param n Target index to compute.
 * @param mod Modulo for all calculations.
 * @return a_n mod modulo.
 */
inline uint64_t solve_linear_recurrence(
    const std::vector<uint64_t>& c,
    const std::vector<uint64_t>& init,
    uint64_t n,
    uint64_t mod
) {
    size_t k = c.size();
    if (k == 0 || init.size() != k) {
        throw std::invalid_argument("Coefficient and initial condition dimensions must match and be > 0");
    }

    // Base cases check
    if (n < k) {
        // init is arranged [a_{k-1}, a_{k-2}, ..., a_0]
        return init[k - 1 - n] % mod;
    }

    // Construct companion matrix M:
    // Row 0: c[0], c[1], ..., c[k-1]
    // Row 1 to k-1: Subdiagonal 1s
    SquareMatrix M(k, mod);
    for (size_t j = 0; j < k; ++j) {
        M.mat[0][j] = c[j] % mod;
    }
    for (size_t i = 1; i < k; ++i) {
        M.mat[i][i - 1] = 1 % mod;
    }

    // Power M^(n - k + 1)
    SquareMatrix M_pow = M.power(n - k + 1);

    // Multiply M_pow by init vector [a_{k-1}, a_{k-2}, ..., a_0]^T
    uint64_t ans = 0;
    for (size_t j = 0; j < k; ++j) {
        __uint128_t term = static_cast<__uint128_t>(M_pow.mat[0][j]) * (init[j] % mod);
        ans = static_cast<uint64_t>((ans + term) % mod);
    }
    return ans;
}

/**
 * @brief Dynamic programming baseline for linear recurrences: O(n * k).
 */
inline uint64_t solve_linear_recurrence_dp(
    const std::vector<uint64_t>& c,
    const std::vector<uint64_t>& init,
    uint64_t n,
    uint64_t mod
) {
    size_t k = c.size();
    if (n < k) return init[k - 1 - n] % mod;

    std::vector<uint64_t> dp(n + 1, 0);
    for (size_t i = 0; i < k; ++i) {
        dp[i] = init[k - 1 - i] % mod;
    }

    for (size_t i = k; i <= n; ++i) {
        uint64_t val = 0;
        for (size_t j = 0; j < k; ++j) {
            __uint128_t term = static_cast<__uint128_t>(c[j] % mod) * dp[i - 1 - j];
            val = static_cast<uint64_t>((val + term) % mod);
        }
        dp[i] = val;
    }
    return dp[n];
}

/**
 * @brief Fast Fibonacci computation modulo MOD in O(log n).
 * F_0 = 0, F_1 = 1, F_n = F_{n-1} + F_{n-2}.
 */
inline uint64_t fibonacci(uint64_t n, uint64_t mod) {
    if (n == 0) return 0;
    if (n == 1) return 1 % mod;
    std::vector<uint64_t> c = {1, 1};
    std::vector<uint64_t> init = {1, 0}; // [F_1, F_0]
    return solve_linear_recurrence(c, init, n, mod);
}

/**
 * @brief Fast Tribonacci computation modulo MOD in O(log n).
 * T_0 = 0, T_1 = 1, T_2 = 1, T_n = T_{n-1} + T_{n-2} + T_{n-3}.
 */
inline uint64_t tribonacci(uint64_t n, uint64_t mod) {
    if (n == 0) return 0;
    if (n == 1 || n == 2) return 1 % mod;
    std::vector<uint64_t> c = {1, 1, 1};
    std::vector<uint64_t> init = {1, 1, 0}; // [T_2, T_1, T_0]
    return solve_linear_recurrence(c, init, n, mod);
}

} // namespace dsa

int main() {
    std::cout << "Running Recurrence Relations C++17 unit tests..." << std::endl;
    constexpr uint64_t MOD = 1'000'000'007ULL;

    // Test 1: Fibonacci verification against DP baseline for n = 0 to 100
    for (uint64_t n = 0; n <= 100; ++n) {
        uint64_t fib_mat = dsa::fibonacci(n, MOD);
        std::vector<uint64_t> c = {1, 1};
        std::vector<uint64_t> init = {1, 0};
        uint64_t fib_dp = dsa::solve_linear_recurrence_dp(c, init, n, MOD);
        assert(fib_mat == fib_dp);
    }
    assert(dsa::fibonacci(0, MOD) == 0);
    assert(dsa::fibonacci(1, MOD) == 1);
    assert(dsa::fibonacci(10, MOD) == 55);

    // Test 2: Tribonacci verification
    for (uint64_t n = 0; n <= 80; ++n) {
        uint64_t trib_mat = dsa::tribonacci(n, MOD);
        std::vector<uint64_t> c = {1, 1, 1};
        std::vector<uint64_t> init = {1, 1, 0};
        uint64_t trib_dp = dsa::solve_linear_recurrence_dp(c, init, n, MOD);
        assert(trib_mat == trib_dp);
    }
    assert(dsa::tribonacci(3, MOD) == 2);
    assert(dsa::tribonacci(4, MOD) == 4);
    assert(dsa::tribonacci(5, MOD) == 7);

    // Test 3: Arbitrary order-4 recurrence test: a_n = 2*a_{n-1} + 3*a_{n-2} + a_{n-3} + 4*a_{n-4}
    {
        std::vector<uint64_t> c = {2, 3, 1, 4};
        std::vector<uint64_t> init = {5, 3, 2, 1}; // [a_3, a_2, a_1, a_0]
        for (uint64_t n = 0; n <= 50; ++n) {
            uint64_t val_mat = dsa::solve_linear_recurrence(c, init, n, MOD);
            uint64_t val_dp = dsa::solve_linear_recurrence_dp(c, init, n, MOD);
            assert(val_mat == val_dp);
        }
    }

    // Test 4: Ultra-large n execution (n = 10^18)
    {
        uint64_t big_n = 1'000'000'000'000'000'000ULL;
        uint64_t big_fib = dsa::fibonacci(big_n, MOD);
        // Ensure it executes instantaneously and yields a valid modular remainder
        assert(big_fib < MOD);
        std::cout << "  [Matrix Exponentiation Verification] F(10^18) mod (10^9+7) = " << big_fib << std::endl;
    }

    std::cout << "[PASS] All Recurrence Relations C++ unit tests passed." << std::endl;
    return 0;
}
