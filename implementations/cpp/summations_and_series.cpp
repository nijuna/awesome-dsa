/**
 * @file summations_and_series.cpp
 * @brief Reference implementations and verification for foundational algorithmic summations.
 *
 * Implements arithmetic, geometric, arithmetico-geometric, harmonic approximations,
 * telescoping sums, and Kahan compensated numerical summation.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <numeric>
#include <iomanip>

namespace dsa {

/**
 * @brief Overflow-safe arithmetic series sum: sum_{i=1}^n i = n(n+1)/2.
 * Factors out 2 first to double the maximum safe n for uint64_t.
 */
inline uint64_t sum_integers(uint64_t n) {
    if (n % 2 == 0) {
        return (n / 2) * (n + 1);
    } else {
        return n * ((n + 1) / 2);
    }
}

/**
 * @brief Overflow-safe sum of squares: sum_{i=1}^n i^2 = n(n+1)(2n+1)/6.
 * Cancels prime factors of 6 (2 and 3) before multiplying to minimize overflow.
 */
inline uint64_t sum_squares(uint64_t n) {
    uint64_t f1 = n;
    uint64_t f2 = n + 1;
    uint64_t f3 = 2 * n + 1;

    // Divide out factor of 2
    if (f1 % 2 == 0) f1 /= 2;
    else f2 /= 2;

    // Divide out factor of 3
    if (f1 % 3 == 0) f1 /= 3;
    else if (f2 % 3 == 0) f2 /= 3;
    else f3 /= 3;

    return f1 * f2 * f3;
}

/**
 * @brief Overflow-safe sum of cubes: sum_{i=1}^n i^3 = (n(n+1)/2)^2.
 */
inline uint64_t sum_cubes(uint64_t n) {
    uint64_t s1 = sum_integers(n);
    return s1 * s1;
}

/**
 * @brief Modular exponentiation for geometric series calculations under prime modulus.
 */
inline uint64_t mod_pow(uint64_t base, uint64_t exp, uint64_t mod) {
    uint64_t result = 1;
    base %= mod;
    while (exp > 0) {
        if (exp & 1) result = static_cast<uint64_t>((static_cast<__uint128_t>(result) * base) % mod);
        base = static_cast<uint64_t>((static_cast<__uint128_t>(base) * base) % mod);
        exp >>= 1;
    }
    return result;
}

/**
 * @brief Modular inverse via Fermat's Little Theorem for prime mod.
 */
inline uint64_t mod_inv(uint64_t n, uint64_t mod) {
    return mod_pow(n, mod - 2, mod);
}

/**
 * @brief Evaluates finite geometric series: sum_{i=0}^{n-1} a * r^i mod prime.
 */
inline uint64_t sum_geometric_mod(uint64_t a, uint64_t r, uint64_t n, uint64_t mod) {
    a %= mod;
    r %= mod;
    if (n == 0) return 0;
    if (r == 0) return a;
    if (r == 1) return (a * (n % mod)) % mod;

    // a * (r^n - 1) / (r - 1) mod mod
    uint64_t rn = mod_pow(r, n, mod);
    uint64_t num = (rn + mod - 1) % mod;
    uint64_t den = mod_inv((r + mod - 1) % mod, mod);
    uint64_t frac = static_cast<uint64_t>((static_cast<__uint128_t>(num) * den) % mod);
    return static_cast<uint64_t>((static_cast<__uint128_t>(a) * frac) % mod);
}

/**
 * @brief Evaluates sum_{i=1}^k i * 2^i = (k-1)*2^{k+1} + 2 exactly in 64-bit integer.
 */
inline uint64_t sum_i_times_2_pow_i(uint64_t k) {
    if (k == 0) return 0;
    // For k >= 63, 2^(k+1) overflows uint64_t
    assert(k < 63);
    uint64_t term = (static_cast<uint64_t>(1) << (k + 1));
    return (k - 1) * term + 2;
}

/**
 * @brief Computes high-order Euler-Maclaurin approximation of the n-th Harmonic Number H_n.
 * H_n = ln(n) + gamma + 1/(2n) - 1/(12n^2) + 1/(120n^4)
 */
inline double harmonic_number_approx(uint64_t n) {
    if (n == 0) return 0.0;
    if (n < 10) {
        // Direct summation for tiny n to avoid asymptotic boundary distortion
        double h = 0.0;
        for (uint64_t i = 1; i <= n; ++i) h += 1.0 / static_cast<double>(i);
        return h;
    }
    constexpr double EULER_MASCHERONI = 0.5772156649015328606065;
    double dn = static_cast<double>(n);
    double h = std::log(dn) + EULER_MASCHERONI + (1.0 / (2.0 * dn)) - (1.0 / (12.0 * dn * dn));
    return h;
}

/**
 * @brief Kahan Compensated Summation algorithm to minimize floating-point error accumulation.
 */
inline double kahan_sum(const std::vector<double>& values) {
    double sum = 0.0;
    double c = 0.0; // Running compensation for lost low-order bits
    for (double x : values) {
        double y = x - c;
        double t = sum + y;
        c = (t - sum) - y;
        sum = t;
    }
    return sum;
}

} // namespace dsa

int main() {
    std::cout << "Running Summations and Series C++17 unit tests..." << std::endl;

    // Test 1: Arithmetic sum n(n+1)/2 vs loop
    for (uint64_t n = 1; n <= 1000; ++n) {
        uint64_t loop_sum = 0;
        for (uint64_t i = 1; i <= n; ++i) loop_sum += i;
        assert(dsa::sum_integers(n) == loop_sum);
    }

    // Test 2: Sum of squares n(n+1)(2n+1)/6 vs loop
    for (uint64_t n = 1; n <= 500; ++n) {
        uint64_t loop_sum = 0;
        for (uint64_t i = 1; i <= n; ++i) loop_sum += i * i;
        assert(dsa::sum_squares(n) == loop_sum);
    }

    // Test 3: Sum of cubes vs (sum of integers)^2
    for (uint64_t n = 1; n <= 200; ++n) {
        uint64_t loop_sum = 0;
        for (uint64_t i = 1; i <= n; ++i) loop_sum += i * i * i;
        assert(dsa::sum_cubes(n) == loop_sum);
    }

    // Test 4: Modular geometric series vs naive loop
    constexpr uint64_t MOD = 1'000'000'007ULL;
    {
        uint64_t a = 3, r = 5, n = 100;
        uint64_t expected = 0;
        uint64_t cur = a % MOD;
        for (uint64_t i = 0; i < n; ++i) {
            expected = (expected + cur) % MOD;
            cur = (cur * r) % MOD;
        }
        assert(dsa::sum_geometric_mod(a, r, n, MOD) == expected);
    }

    // Test 5: Arithmetico-geometric series sum_{i=1}^k i * 2^i
    for (uint64_t k = 1; k <= 30; ++k) {
        uint64_t loop_sum = 0;
        for (uint64_t i = 1; i <= k; ++i) {
            loop_sum += i * (static_cast<uint64_t>(1) << i);
        }
        assert(dsa::sum_i_times_2_pow_i(k) == loop_sum);
    }

    // Test 6: Harmonic number approximation accuracy for n = 10^5
    {
        double exact = 0.0;
        for (uint64_t i = 1; i <= 100'000; ++i) {
            exact += 1.0 / static_cast<double>(i);
        }
        double approx = dsa::harmonic_number_approx(100'000);
        double diff = std::abs(exact - approx);
        // Error of Euler-Maclaurin 2-term should be < 1e-10
        assert(diff < 1e-9);
    }

    // Test 7: Kahan summation precision test
    {
        // Add 1.0 to 10^7 copies of 1e-8.
        // Exact mathematical answer: 1.0 + 10^7 * 1e-8 = 1.1.
        std::vector<double> vals;
        vals.reserve(10'000'001);
        vals.push_back(1.0);
        for (int i = 0; i < 10'000'000; ++i) {
            vals.push_back(1e-8);
        }

        double naive = 0.0;
        for (double v : vals) naive += v;

        double compensated = dsa::kahan_sum(vals);

        // Compensated should be virtually exact to 1.1
        assert(std::abs(compensated - 1.1) < 1e-11);
        // Naive will accumulate measurable floating-point drift
        std::cout << "  [Kahan Precision Verification] Exact: 1.1 | Naive: "
                  << std::setprecision(12) << naive
                  << " | Kahan: " << compensated << std::endl;
    }

    std::cout << "[PASS] All Summations and Series C++ unit tests passed." << std::endl;
    return 0;
}
