/**
 * @file number_theory_basics.cpp
 * @brief Reference implementations for elementary number theory algorithms.
 *
 * Implements Euclidean GCD, Extended Euclidean Algorithm, modular inverse,
 * linear sieve (Euler's sieve) with SPF, Euler's totient function, and Chinese Remainder Theorem.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <numeric>
#include <stdexcept>

namespace dsa {

/**
 * @brief Standard Euclidean algorithm for greatest common divisor: O(log min(a, b)).
 */
inline uint64_t gcd(uint64_t a, uint64_t b) {
    while (b != 0) {
        uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}

/**
 * @brief Extended Euclidean Algorithm.
 * Finds integers x, y such that a*x + b*y = gcd(a, b).
 * @return gcd(a, b).
 */
inline int64_t ext_gcd(int64_t a, int64_t b, int64_t& x, int64_t& y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    int64_t x1 = 0, y1 = 0;
    int64_t d = ext_gcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return d;
}

/**
 * @brief Computes modular multiplicative inverse of a modulo m using Extended GCD.
 * Valid for composite or prime m, provided gcd(a, m) == 1.
 */
inline int64_t mod_inverse(int64_t a, int64_t m) {
    int64_t x = 0, y = 0;
    int64_t g = ext_gcd(a, m, x, y);
    if (g != 1) {
        throw std::invalid_argument("Modular inverse does not exist: gcd(a, m) != 1");
    }
    return (x % m + m) % m;
}

/**
 * @brief Modular exponentiation (base^exp mod m).
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
 * @brief Result structure for Euler's Linear Sieve.
 */
struct SieveResult {
    std::vector<uint32_t> primes;
    std::vector<uint32_t> spf; // Smallest Prime Factor
};

/**
 * @brief Euler's Linear Sieve: computes primes and SPF up to n in strict O(n) time.
 */
inline SieveResult linear_sieve(uint32_t n) {
    SieveResult res;
    res.spf.assign(n + 1, 0);
    for (uint32_t i = 2; i <= n; ++i) {
        if (res.spf[i] == 0) {
            res.spf[i] = i;
            res.primes.push_back(i);
        }
        for (uint32_t p : res.primes) {
            if (p > res.spf[i] || static_cast<uint64_t>(i) * p > n) {
                break;
            }
            res.spf[i * p] = p;
        }
    }
    return res;
}

/**
 * @brief Computes Euler's Totient Function phi(n) in O(sqrt(n)) time.
 * Returns count of positive integers <= n coprime to n.
 */
inline uint64_t euler_totient(uint64_t n) {
    uint64_t result = n;
    for (uint64_t p = 2; p * p <= n; ++p) {
        if (n % p == 0) {
            while (n % p == 0) {
                n /= p;
            }
            result -= result / p;
        }
    }
    if (n > 1) {
        result -= result / n;
    }
    return result;
}

/**
 * @brief Chinese Remainder Theorem solver.
 * Solves system: x = remainders[i] mod moduli[i] for pairwise coprime moduli.
 *
 * @return Unique solution x mod (product of moduli).
 */
inline int64_t chinese_remainder_theorem(
    const std::vector<int64_t>& remainders,
    const std::vector<int64_t>& moduli
) {
    size_t k = remainders.size();
    if (k == 0 || k != moduli.size()) {
        throw std::invalid_argument("Dimension mismatch in CRT inputs");
    }

    int64_t M = 1;
    for (int64_t m : moduli) {
        M *= m;
    }

    int64_t x = 0;
    for (size_t i = 0; i < k; ++i) {
        int64_t Mi = M / moduli[i];
        int64_t yi = mod_inverse(Mi % moduli[i], moduli[i]);
        int64_t term = static_cast<int64_t>((static_cast<__int128_t>(remainders[i]) * Mi % M * yi) % M);
        x = (x + term) % M;
    }
    return (x % M + M) % M;
}

} // namespace dsa

int main() {
    std::cout << "Running Number Theory Basics C++17 unit tests..." << std::endl;

    // Test 1: Euclidean GCD
    assert(dsa::gcd(48, 18) == 6);
    assert(dsa::gcd(101, 10) == 1);
    assert(dsa::gcd(0, 25) == 25);
    assert(dsa::gcd(25, 0) == 25);

    // Test 2: Extended Euclidean Algorithm (Bézout's identity)
    {
        int64_t a = 35, b = 15;
        int64_t x = 0, y = 0;
        int64_t g = dsa::ext_gcd(a, b, x, y);
        assert(g == 5);
        assert(a * x + b * y == g);
    }

    // Test 3: Modular inverse with composite and prime moduli
    {
        // Inverse of 3 mod 11 -> 3 * 4 = 12 = 1 mod 11
        assert(dsa::mod_inverse(3, 11) == 4);
        // Inverse of 7 mod 26 (composite) -> 7 * 15 = 105 = 4*26 + 1 -> 15
        assert(dsa::mod_inverse(7, 26) == 15);
        // Non-coprime should throw
        bool threw = false;
        try {
            dsa::mod_inverse(6, 9);
        } catch (const std::invalid_argument&) {
            threw = true;
        }
        assert(threw);
    }

    // Test 4: Modular exponentiation
    assert(dsa::mod_pow(2, 10, 1000) == 24);
    assert(dsa::mod_pow(3, 5, 13) == (243 % 13));

    // Test 5: Linear Sieve verification against known primes
    {
        uint32_t limit = 100;
        dsa::SieveResult res = dsa::linear_sieve(limit);
        // There are 25 primes <= 100
        assert(res.primes.size() == 25);
        assert(res.primes[0] == 2);
        assert(res.primes[1] == 3);
        assert(res.primes[24] == 97);
        // Verify smallest prime factor
        assert(res.spf[77] == 7);
        assert(res.spf[91] == 7);
        assert(res.spf[97] == 97);
    }

    // Test 6: Euler's Totient function
    // phi(1)=1, phi(2)=1, phi(3)=2, phi(4)=2, phi(5)=4, phi(6)=2, phi(9)=6, phi(10)=4, phi(12)=4
    assert(dsa::euler_totient(1) == 1);
    assert(dsa::euler_totient(2) == 1);
    assert(dsa::euler_totient(6) == 2);
    assert(dsa::euler_totient(9) == 6);
    assert(dsa::euler_totient(12) == 4);
    assert(dsa::euler_totient(13) == 12); // prime p -> p - 1

    // Test 7: Chinese Remainder Theorem
    {
        // x = 2 mod 3
        // x = 3 mod 5
        // x = 2 mod 7
        // Solution: x = 23 (23%3=2, 23%5=3, 23%7=2)
        std::vector<int64_t> rems = {2, 3, 2};
        std::vector<int64_t> mods = {3, 5, 7};
        int64_t sol = dsa::chinese_remainder_theorem(rems, mods);
        assert(sol == 23);
    }

    std::cout << "[PASS] All Number Theory Basics C++ unit tests passed." << std::endl;
    return 0;
}
