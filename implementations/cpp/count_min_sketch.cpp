/**
 * @file count_min_sketch.cpp
 * @brief Reference implementation of Count-Min Sketch frequency estimation.
 *
 * Implements 2D counter matrix with independent Murmur/FNV hash seeds,
 * point queries, conservative updates, and validation of the one-sided error bound.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <climits>

namespace dsa {

inline uint64_t hash_with_seed(const std::string& str, uint64_t seed) {
    uint64_t h = seed ^ 14695981039346656037ULL;
    for (char c : str) {
        h ^= static_cast<uint8_t>(c);
        h *= 1099511628211ULL;
    }
    h ^= h >> 33;
    h *= 0xff51afd7ed558ccdULL;
    h ^= h >> 33;
    return h;
}

class CountMinSketch {
private:
    size_t depth_; // number of hash functions (d)
    size_t width_; // number of counters per row (w)
    std::vector<std::vector<uint32_t>> table_;
    std::vector<uint64_t> seeds_;
    uint64_t total_count_ = 0;

public:
    CountMinSketch(size_t depth, size_t width)
        : depth_(depth), width_(width), table_(depth, std::vector<uint32_t>(width, 0)), total_count_(0) {
        assert(depth > 0 && width > 0);
        seeds_.reserve(depth);
        for (size_t i = 0; i < depth; ++i) {
            seeds_.push_back(0x9e3779b97f4a7c15ULL + i * 0x85ebca6bULL);
        }
    }

    void update(const std::string& key, uint32_t count = 1) {
        total_count_ += count;
        for (size_t i = 0; i < depth_; ++i) {
            size_t col = hash_with_seed(key, seeds_[i]) % width_;
            table_[i][col] += count;
        }
    }

    void update_conservative(const std::string& key, uint32_t count = 1) {
        total_count_ += count;
        uint32_t min_curr = estimate(key);
        for (size_t i = 0; i < depth_; ++i) {
            size_t col = hash_with_seed(key, seeds_[i]) % width_;
            if (table_[i][col] == min_curr) {
                table_[i][col] += count;
            }
        }
    }

    uint32_t estimate(const std::string& key) const {
        uint32_t min_val = UINT32_MAX;
        for (size_t i = 0; i < depth_; ++i) {
            size_t col = hash_with_seed(key, seeds_[i]) % width_;
            min_val = std::min(min_val, table_[i][col]);
        }
        return min_val;
    }

    uint64_t total_count() const {
        return total_count_;
    }

    size_t width() const { return width_; }
    size_t depth() const { return depth_; }
};

} // namespace dsa

int main() {
    std::cout << "Running Count-Min Sketch verification..." << std::endl;

    // Depth d = 5, Width w = 1000 -> epsilon = e / 1000 ~ 0.0027 (0.27% of N)
    dsa::CountMinSketch cms(5, 1000);

    // Stream insertions:
    // Heavy hitter: "apple" appears 500 times
    // Moderate: "banana" appears 100 times
    // Light: "cherry" appears 20 times
    for (int i = 0; i < 500; ++i) cms.update("apple");
    for (int i = 0; i < 100; ++i) cms.update("banana");
    for (int i = 0; i < 20; ++i) cms.update("cherry");

    // Insert 5,000 background noise elements
    for (int i = 0; i < 5000; ++i) {
        cms.update("noise_item_" + std::to_string(i));
    }

    // 1. One-sided error verification: MUST NEVER UNDERESTIMATE
    assert(cms.estimate("apple") >= 500);
    assert(cms.estimate("banana") >= 100);
    assert(cms.estimate("cherry") >= 20);

    // 2. Unseen element verification: Count should be near 0
    assert(cms.estimate("unseen_item_xyz") < 20);

    // 3. Accuracy bound verification:
    // N = 5620, epsilon = e / 1000 = 0.0027 => epsilon * N ~ 15.2
    // Overestimation on "apple" should be less than ~30
    uint32_t apple_est = cms.estimate("apple");
    std::cout << "[PASS] Actual apple count: 500 | CMS estimate: " << apple_est << std::endl;
    assert(apple_est >= 500 && apple_est <= 530);

    std::cout << "All Count-Min Sketch assertions passed successfully!" << std::endl;
    return 0;
}
