/**
 * Reference Implementation: Standard Bloom Filter with Kirsch-Mitzenmacher Double Hashing
 * Demonstrates optimal bit-array sizing, optimal hash count calculation,
 * zero false negatives guarantee, and empirical false positive measurement.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <string>

class BloomFilter {
private:
    std::vector<uint64_t> bits_;
    std::size_t num_bits_;
    std::size_t num_hashes_;
    std::size_t items_added_;

    static uint64_t splitmix64(uint64_t x) noexcept {
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebULL;
        x ^= x >> 31;
        return x;
    }

    // Hash function yielding two 32-bit hashes for Kirsch-Mitzenmacher optimization
    static std::pair<uint32_t, uint32_t> hash_pair(const std::string& key) noexcept {
        uint64_t h = 0x84222325cbf849c3ULL; // FNV-1a 64-bit basis
        for (char c : key) {
            h ^= static_cast<uint8_t>(c);
            h *= 0x100000001b3ULL;
        }
        uint64_t mixed = splitmix64(h);
        return {static_cast<uint32_t>(mixed >> 32), static_cast<uint32_t>(mixed & 0xFFFFFFFF)};
    }

public:
    /**
     * Constructs optimal Bloom Filter based on expected item count and target false positive probability.
     * Formula: m = - (n * ln(p)) / (ln(2)^2)
     *          k = (m / n) * ln(2)
     */
    BloomFilter(std::size_t expected_items, double target_fp_prob)
        : items_added_(0) {
        if (expected_items == 0) expected_items = 1;
        if (target_fp_prob <= 0.0 || target_fp_prob >= 1.0) target_fp_prob = 0.01;

        double ln2 = std::log(2.0);
        double m = - (static_cast<double>(expected_items) * std::log(target_fp_prob)) / (ln2 * ln2);
        num_bits_ = static_cast<std::size_t>(std::ceil(m));
        if (num_bits_ < 64) num_bits_ = 64;

        double k = (static_cast<double>(num_bits_) / expected_items) * ln2;
        num_hashes_ = static_cast<std::size_t>(std::round(k));
        if (num_hashes_ < 1) num_hashes_ = 1;

        bits_.resize((num_bits_ + 63) / 64, 0);
    }

    void add(const std::string& key) {
        auto [h1, h2] = hash_pair(key);
        for (std::size_t i = 0; i < num_hashes_; ++i) {
            // Kirsch-Mitzenmacher technique: g_i(x) = h1(x) + i * h2(x) mod m
            std::size_t bit_idx = (static_cast<uint64_t>(h1) + i * static_cast<uint64_t>(h2)) % num_bits_;
            bits_[bit_idx / 64] |= (1ULL << (bit_idx % 64));
        }
        ++items_added_;
    }

    [[nodiscard]] bool contains(const std::string& key) const {
        auto [h1, h2] = hash_pair(key);
        for (std::size_t i = 0; i < num_hashes_; ++i) {
            std::size_t bit_idx = (static_cast<uint64_t>(h1) + i * static_cast<uint64_t>(h2)) % num_bits_;
            if (!(bits_[bit_idx / 64] & (1ULL << (bit_idx % 64)))) {
                return false; // Definitely not present (Zero False Negatives)
            }
        }
        return true; // Probably present (Subject to False Positive rate)
    }

    [[nodiscard]] std::size_t bit_count() const noexcept { return num_bits_; }
    [[nodiscard]] std::size_t hash_count() const noexcept { return num_hashes_; }
    [[nodiscard]] std::size_t items_added() const noexcept { return items_added_; }

    [[nodiscard]] double expected_false_positive_rate() const noexcept {
        // p = (1 - e^(-k * n / m))^k
        double exponent = -static_cast<double>(num_hashes_ * items_added_) / num_bits_;
        return std::pow(1.0 - std::exp(exponent), static_cast<double>(num_hashes_));
    }
};

void run_tests() {
    const std::size_t N = 5000;
    const double target_p = 0.01; // 1% false positive target
    BloomFilter filter(N, target_p);

    assert(filter.bit_count() >= N * 7); // ~9.6 bits per item for 1%
    assert(filter.hash_count() >= 5);

    // 1. Insert N items
    for (std::size_t i = 0; i < N; ++i) {
        filter.add("member_key_" + std::to_string(i));
    }

    // 2. Verify Zero False Negatives invariant
    for (std::size_t i = 0; i < N; ++i) {
        bool found = filter.contains("member_key_" + std::to_string(i));
        assert(found); // Must ALWAYS be true!
    }

    // 3. Measure Empirical False Positive Rate on disjoint keys
    const std::size_t test_disjoint = 10000;
    std::size_t false_positives = 0;
    for (std::size_t i = 0; i < test_disjoint; ++i) {
        if (filter.contains("disjoint_query_" + std::to_string(i))) {
            ++false_positives;
        }
    }

    double empirical_fp = static_cast<double>(false_positives) / test_disjoint;
    std::cout << "[INFO] Empirical False Positive Rate: " << (empirical_fp * 100.0) 
              << "% (Target: " << (target_p * 100.0) << "%)\n";

    // Empirical FP rate must be comfortably within reasonable variance of 1%
    assert(empirical_fp < 0.03); // Upper confidence bound

    std::cout << "[PASS] All BloomFilter C++ unit tests passed.\n";
}

int main() {
    run_tests();
    return 0;
}
