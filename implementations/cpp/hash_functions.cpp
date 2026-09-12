/**
 * @file hash_functions.cpp
 * @brief Reference implementations for foundational non-cryptographic hash functions.
 *
 * Implements 64-bit FNV-1a, Murmur3 bit-mixer, SplitMix64, and the Carter-Wegman
 * 2-universal hash family using Mersenne prime 2^61 - 1.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <cstring>
#include <random>

namespace dsa {

/**
 * @brief 64-bit FNV-1a hash for arbitrary byte arrays.
 */
inline uint64_t fnv1a_64(const void* key, size_t len) {
    const uint8_t* data = static_cast<const uint8_t*>(key);
    uint64_t hash = 0xcbf29ce484222325ULL;
    constexpr uint64_t FNV_PRIME = 0x100000001b3ULL;
    for (size_t i = 0; i < len; ++i) {
        hash ^= data[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

/**
 * @brief MurmurHash3 32-bit integer finalizer / bit-mixer.
 */
inline uint32_t murmur3_mix32(uint32_t k) {
    k ^= k >> 16;
    k *= 0x85ebca6bU;
    k ^= k >> 13;
    k *= 0xc2b2ae35U;
    k ^= k >> 16;
    return k;
}

/**
 * @brief SplitMix64 integer mixer (widely used in Java SplittableRandom and competitive programming).
 */
inline uint64_t splitmix64(uint64_t x) {
    x += 0x9e3779b97f4a7c15ULL;
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

/**
 * @brief Carter-Wegman 2-Universal Hash Family using Mersenne prime p = 2^61 - 1.
 * Evaluates: h_{a,b}(x) = ((a * x + b) mod p) mod m
 */
class UniversalHash64 {
private:
    uint64_t a;
    uint64_t b;
    uint64_t m;
    static constexpr uint64_t MERSENNE_61 = (1ULL << 61) - 1;

public:
    UniversalHash64(uint64_t seed_a, uint64_t seed_b, uint64_t table_size)
        : a(seed_a % (MERSENNE_61 - 1) + 1), b(seed_b % MERSENNE_61), m(table_size) {
        assert(m > 0);
    }

    uint64_t hash(uint64_t x) const {
        x &= MERSENNE_61;
        __uint128_t prod = static_cast<__uint128_t>(a) * x + b;
        uint64_t mod_p = static_cast<uint64_t>(prod % MERSENNE_61);
        return mod_p % m;
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Hash Functions C++17 unit tests..." << std::endl;

    // Test 1: FNV-1a determinism and string hashing
    {
        std::string str1 = "hello world";
        std::string str2 = "hello world";
        std::string str3 = "hello worle"; // 1-character difference

        uint64_t h1 = dsa::fnv1a_64(str1.data(), str1.size());
        uint64_t h2 = dsa::fnv1a_64(str2.data(), str2.size());
        uint64_t h3 = dsa::fnv1a_64(str3.data(), str3.size());

        assert(h1 == h2);
        assert(h1 != h3);
    }

    // Test 2: Murmur3 32-bit finalizer avalanche test
    // Flipping a single bit in the input should flip on average ~16 bits (50%) in output
    {
        uint32_t total_flipped_bits = 0;
        constexpr uint32_t SAMPLES = 10'000;
        std::mt19937 rng(1337);

        for (uint32_t i = 0; i < SAMPLES; ++i) {
            uint32_t x = rng();
            uint32_t bit_to_flip = rng() % 32;
            uint32_t y = x ^ (1U << bit_to_flip);

            uint32_t hx = dsa::murmur3_mix32(x);
            uint32_t hy = dsa::murmur3_mix32(y);

            uint32_t diff = hx ^ hy;
            total_flipped_bits += __builtin_popcount(diff);
        }

        double avg_flipped = static_cast<double>(total_flipped_bits) / SAMPLES;
        // 50% of 32 bits is 16.0. Should be within [15.0, 17.0]
        assert(avg_flipped >= 15.0 && avg_flipped <= 17.0);
        std::cout << "  [Murmur3 Avalanche Verification] Avg flipped bits (target: 16.0): " << avg_flipped << std::endl;
    }

    // Test 3: SplitMix64 uniqueness across sequential integers
    {
        uint64_t prev = dsa::splitmix64(0);
        for (uint64_t i = 1; i <= 1000; ++i) {
            uint64_t cur = dsa::splitmix64(i);
            assert(cur != prev);
            prev = cur;
        }
    }

    // Test 4: Carter-Wegman Universal Hashing bucket distribution on random 64-bit keys
    {
        constexpr uint64_t BUCKETS = 100;
        dsa::UniversalHash64 uh(123456789123ULL, 987654321ULL, BUCKETS);
        std::vector<int> bucket_counts(BUCKETS, 0);

        constexpr int KEYS = 100'000;
        std::mt19937_64 rng(42);
        for (int i = 0; i < KEYS; ++i) {
            uint64_t key = rng();
            uint64_t b = uh.hash(key);
            assert(b < BUCKETS);
            bucket_counts[b]++;
        }

        // Verify all buckets have non-zero elements and stay close to expected (1000)
        for (int count : bucket_counts) {
            assert(count > 800 && count < 1200);
        }
    }

    std::cout << "[PASS] All Hash Functions C++ unit tests passed." << std::endl;
    return 0;
}
