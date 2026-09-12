/**
 * @file hyperloglog.cpp
 * @brief Reference implementation of the HyperLogLog cardinality estimation algorithm.
 *
 * Implements 64-bit hashing, leading-zero bit counting, harmonic mean estimation,
 * LinearCounting small-range correction, and MapReduce-style sketch merging.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <algorithm>

namespace dsa {

inline uint64_t hash64(const std::string& str) {
    uint64_t h = 14695981039346656037ULL;
    for (char c : str) {
        h ^= static_cast<uint8_t>(c);
        h *= 1099511628211ULL;
    }
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    return h;
}

class HyperLogLog {
private:
    uint32_t p_; // precision bits
    uint32_t m_; // number of registers = 2^p
    std::vector<uint8_t> registers_;

    double get_alpha() const {
        if (m_ == 16) return 0.673;
        if (m_ == 32) return 0.697;
        if (m_ == 64) return 0.709;
        return 0.7213 / (1.0 + 1.079 / m_);
    }

    static int count_leading_zeros(uint64_t val, int max_bits) {
        if (val == 0) return max_bits;
        int lz = __builtin_clzll(val);
        // __builtin_clzll operates on 64-bit integers; adjust if max_bits < 64
        int offset = 64 - max_bits;
        return std::min(lz - offset + 1, max_bits);
    }

public:
    explicit HyperLogLog(uint32_t precision = 10)
        : p_(precision), m_(1U << precision), registers_(1U << precision, 0) {
        assert(precision >= 4 && precision <= 16);
    }

    void add(const std::string& item) {
        uint64_t x = hash64(item);
        // First p bits determine register index
        uint32_t idx = static_cast<uint32_t>(x >> (64 - p_));
        // Remaining 64 - p bits
        uint64_t w = x & ((1ULL << (64 - p_)) - 1);

        int lz = count_leading_zeros(w, 64 - p_);
        if (lz > registers_[idx]) {
            registers_[idx] = static_cast<uint8_t>(lz);
        }
    }

    double estimate() const {
        double sum = 0.0;
        uint32_t empty_registers = 0;

        for (uint8_t r : registers_) {
            sum += std::ldexp(1.0, -static_cast<int>(r)); // 2^(-r)
            if (r == 0) empty_registers++;
        }

        double alpha = get_alpha();
        double raw_estimate = alpha * static_cast<double>(m_) * static_cast<double>(m_) / sum;

        // Small range correction (Linear Counting)
        if (raw_estimate <= 2.5 * m_ && empty_registers > 0) {
            return static_cast<double>(m_) * std::log(static_cast<double>(m_) / empty_registers);
        }

        return raw_estimate;
    }

    void merge(const HyperLogLog& other) {
        assert(p_ == other.p_ && m_ == other.m_);
        for (size_t i = 0; i < m_; ++i) {
            registers_[i] = std::max(registers_[i], other.registers_[i]);
        }
    }

    size_t memory_bytes() const {
        return registers_.size();
    }
};

} // namespace dsa

int main() {
    std::cout << "Running HyperLogLog verification..." << std::endl;

    // Precision p = 10 -> m = 1024 registers -> Standard Error ~ 1.04 / sqrt(1024) ~ 3.25%
    dsa::HyperLogLog hll1(10);
    assert(hll1.memory_bytes() == 1024); // Exactly 1 KB of RAM!

    // 1. Accuracy test with 20,000 distinct items
    const int DISTINCT_COUNT = 20000;
    for (int i = 0; i < DISTINCT_COUNT; ++i) {
        hll1.add("item_stream_A_" + std::to_string(i));
    }

    double est1 = hll1.estimate();
    double error_rate1 = std::abs(est1 - DISTINCT_COUNT) / DISTINCT_COUNT;
    std::cout << "[PASS] 20,000 distinct items estimated as: " << est1
              << " (Relative error: " << error_rate1 * 100.0 << "%)" << std::endl;
    // Error must be well within 3 standard deviations (< 10%)
    assert(error_rate1 < 0.10);

    // 2. MapReduce Merge Verification
    dsa::HyperLogLog hll2(10);
    // Add 10,000 completely new distinct items
    for (int i = 20000; i < 30000; ++i) {
        hll2.add("item_stream_B_" + std::to_string(i));
    }

    // Merge hll2 into hll1 (Total distinct items = 30,000)
    hll1.merge(hll2);
    double merged_est = hll1.estimate();
    double error_rate_merged = std::abs(merged_est - 30000.0) / 30000.0;

    std::cout << "[PASS] Merged sketch estimated cardinality: " << merged_est
              << " (Expected: 30000, Error: " << error_rate_merged * 100.0 << "%)" << std::endl;
    assert(error_rate_merged < 0.10);

    std::cout << "All HyperLogLog assertions passed successfully!" << std::endl;
    return 0;
}
