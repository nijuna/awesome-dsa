/**
 * @file combinatorics.cpp
 * @brief High-performance modular combinatorics engine.
 *
 * Implements O(N) precomputed modular factorials, O(1) combinations and permutations,
 * Catalan numbers, Lucas' theorem for large n, and derangements.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <stdexcept>

namespace dsa {

/**
 * @brief Binary exponentiation under modulo: a^b mod m.
 */
inline uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t res = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) res = static_cast<uint64_t>((static_cast<__uint128_t>(res) * base) % mod);
        base = static_cast<uint64_t>((static_cast<__uint128_t>(base) * base) % mod);
        exp >>= 1;
    }
    return res;
}

/**
 * @brief Modular multiplicative inverse via Fermat's Little Theorem (requires prime mod).
 */
inline uint64_t mod_inverse(uint64_t a, uint64_t prime_mod) {
    return mod_pow(a, prime_mod - 2, prime_mod);
}

/**
 * @brief Precomputed modular combinatorics table for O(1) query execution.
 */
class ModularCombinatorics {
private:
    uint64_t max_n;
    uint64_t mod;
    std::vector<uint64_t> fact;
    std::vector<uint64_t> inv_fact;

public:
    ModularCombinatorics(uint64_t n, uint64_t m) : max_n(n), mod(m), fact(n + 1), inv_fact(n + 1) {
        fact[0] = 1;
        for (uint64_t i = 1; i <= max_n; ++i) {
            fact[i] = static_cast<uint64_t>((static_cast<__uint128_t>(fact[i - 1]) * i) % mod);
        }

        // Compute inverse of fact[max_n] using Fermat's Little Theorem
        inv_fact[max_n] = mod_inverse(fact[max_n], mod);

        // Linear backward sweep: inv_fact[i - 1] = inv_fact[i] * i mod mod
        for (uint64_t i = max_n; i >= 1; --i) {
            inv_fact[i - 1] = static_cast<uint64_t>((static_cast<__uint128_t>(inv_fact[i]) * i) % mod);
        }
    }

    /**
     * @brief Computes nCr (combinations) modulo mod in O(1) time.
     */
    uint64_t nCr(uint64_t n, uint64_t r) const {
        if (r > n) return 0;
        if (n > max_n) throw std::out_of_range("n exceeds precomputed maximum size");
        __uint128_t num = fact[n];
        __uint128_t den = (static_cast<__uint128_t>(inv_fact[r]) * inv_fact[n - r]) % mod;
        return static_cast<uint64_t>((num * den) % mod);
    }

    /**
     * @brief Computes nPr (permutations) modulo mod in O(1) time.
     */
    uint64_t nPr(uint64_t n, uint64_t r) const {
        if (r > n) return 0;
        if (n > max_n) throw std::out_of_range("n exceeds precomputed maximum size");
        return static_cast<uint64_t>((static_cast<__uint128_t>(fact[n]) * inv_fact[n - r]) % mod);
    }

    /**
     * @brief Computes n-th Catalan number C_n = 1/(n+1) * C(2n, n) mod mod in O(1) time.
     */
    uint64_t catalan(uint64_t n) const {
        if (2 * n > max_n) throw std::out_of_range("2n exceeds precomputed maximum size");
        uint64_t c2n_n = nCr(2 * n, n);
        uint64_t inv_n1 = mod_inverse(n + 1, mod);
        return static_cast<uint64_t>((static_cast<__uint128_t>(c2n_n) * inv_n1) % mod);
    }

    /**
     * @brief Stars and Bars: Number of non-negative integer solutions to x_1 + ... + x_k = n.
     * Formula: C(n + k - 1, k - 1).
     */
    uint64_t stars_and_bars(uint64_t n, uint64_t k) const {
        if (k == 0) return (n == 0 ? 1 : 0);
        return nCr(n + k - 1, k - 1);
    }
};

/**
 * @brief Lucas' Theorem: Computes C(n, r) modulo prime p for arbitrarily large n, r.
 * Complexity: O(p + log_p n).
 */
inline uint64_t nCr_lucas(uint64_t n, uint64_t r, uint64_t prime_mod) {
    if (r > n) return 0;
    ModularCombinatorics small_comb(prime_mod - 1, prime_mod);

    uint64_t ans = 1;
    while (n > 0 || r > 0) {
        uint64_t ni = n % prime_mod;
        uint64_t ri = r % prime_mod;
        if (ri > ni) return 0;
        ans = static_cast<uint64_t>((static_cast<__uint128_t>(ans) * small_comb.nCr(ni, ri)) % prime_mod);
        n /= prime_mod;
        r /= prime_mod;
    }
    return ans;
}

/**
 * @brief Computes number of derangements D_n modulo mod in O(n) time.
 * Recurrence: D_n = (n - 1) * (D_{n-1} + D_{n-2}) mod mod.
 */
inline uint64_t derangements(uint64_t n, uint64_t mod) {
    if (n == 0) return 1 % mod;
    if (n == 1) return 0;
    uint64_t prev2 = 1 % mod; // D_0
    uint64_t prev1 = 0;       // D_1
    uint64_t cur = 0;
    for (uint64_t i = 2; i <= n; ++i) {
        __uint128_t sum = (prev1 + prev2) % mod;
        cur = static_cast<uint64_t>((static_cast<__uint128_t>(i - 1) * sum) % mod);
        prev2 = prev1;
        prev1 = cur;
    }
    return cur;
}

} // namespace dsa

int main() {
    std::cout << "Running Combinatorics C++17 unit tests..." << std::endl;
    constexpr uint64_t MOD = 1'000'000'007ULL;

    dsa::ModularCombinatorics comb(100'000, MOD);

    // Test 1: Pascal's Identity verification: C(n, k) = C(n-1, k-1) + C(n-1, k)
    for (uint64_t n = 2; n <= 100; ++n) {
        for (uint64_t k = 1; k < n; ++k) {
            uint64_t lhs = comb.nCr(n, k);
            uint64_t rhs = (comb.nCr(n - 1, k - 1) + comb.nCr(n - 1, k)) % MOD;
            assert(lhs == rhs);
        }
    }

    // Test 2: Standard binomial values
    assert(comb.nCr(5, 0) == 1);
    assert(comb.nCr(5, 1) == 5);
    assert(comb.nCr(5, 2) == 10);
    assert(comb.nCr(5, 3) == 10);
    assert(comb.nCr(5, 4) == 5);
    assert(comb.nCr(5, 5) == 1);
    assert(comb.nCr(5, 6) == 0); // Out of bounds r > n

    // Test 3: Permutations
    assert(comb.nPr(5, 2) == 20);
    assert(comb.nPr(5, 5) == 120);
    assert(comb.nPr(5, 0) == 1);

    // Test 4: Catalan Numbers: C_0=1, C_1=1, C_2=2, C_3=5, C_4=14, C_5=42, C_6=132
    std::vector<uint64_t> expected_catalan = {1, 1, 2, 5, 14, 42, 132, 429, 1430, 4862};
    for (size_t i = 0; i < expected_catalan.size(); ++i) {
        assert(comb.catalan(i) == expected_catalan[i]);
    }

    // Test 5: Stars and Bars (3 items into 2 bins: (3,0), (2,1), (1,2), (0,3) -> 4 ways)
    assert(comb.stars_and_bars(3, 2) == 4);

    // Test 6: Lucas' Theorem with large n
    {
        uint64_t small_prime = 13;
        // Verify nCr_lucas matches small_comb when n < prime
        dsa::ModularCombinatorics small_c(small_prime - 1, small_prime);
        assert(dsa::nCr_lucas(10, 3, small_prime) == small_c.nCr(10, 3));

        // Test with n >= prime: C(27, 10) mod 13
        // 27 in base 13 is 2*13 + 1 -> (2, 1)
        // 10 in base 13 is 0*13 + 10 -> (0, 10)
        // C(2, 0) * C(1, 10) = 1 * 0 = 0 mod 13
        assert(dsa::nCr_lucas(27, 10, small_prime) == 0);
    }

    // Test 7: Derangements: D_0=1, D_1=0, D_2=1, D_3=2, D_4=9, D_5=44, D_6=265
    std::vector<uint64_t> expected_derange = {1, 0, 1, 2, 9, 44, 265};
    for (size_t i = 0; i < expected_derange.size(); ++i) {
        assert(dsa::derangements(i, MOD) == expected_derange[i]);
    }

    std::cout << "[PASS] All Combinatorics C++ unit tests passed." << std::endl;
    return 0;
}
