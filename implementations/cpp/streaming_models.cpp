/**
 * @file streaming_models.cpp
 * @brief High-performance C++17 reference implementation of Streaming Models and Algorithms.
 *
 * Implements three core streaming paradigms:
 * 1. Misra-Gries Algorithm (1982): Deterministic epsilon-heavy hitters in the Cash Register model.
 *    - Guarantees that any item occurring > N / k times is retained.
 *    - Uses O(k) words of space.
 * 2. Reservoir Sampling (Algorithm R, Vitter 1985): Single-pass uniform sampling.
 *    - Uniformly selects k samples from a stream of unknown length N.
 *    - Every item has exact probability k / N of inclusion.
 * 3. DGIM Algorithm (Datar-Gionis-Indyk-Motwani 2002): Sliding Window model.
 *    - Counts 1-bits in a sliding window of width W with (1 +/- eps) relative error.
 *    - Uses O((1 / eps) * log^2 W) bits of space.
 *
 * Fully compliant with -std=c++17 -O3 -Wall -Wextra -Werror.
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <deque>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <random>

namespace dsa {

/**
 * @brief Misra-Gries algorithm for deterministic epsilon-heavy hitters.
 * Memory: O(k) words. Finds all elements with frequency > N / k.
 */
template <typename T>
class MisraGries {
private:
    size_t k_{0};
    std::unordered_map<T, int64_t> counters_;
    int64_t total_elements_{0};

public:
    explicit MisraGries(size_t k) : k_(k) {
        assert(k > 1);
    }

    void process(const T& item) {
        total_elements_++;
        auto it = counters_.find(item);
        if (it != counters_.end()) {
            it->second++;
        } else if (counters_.size() < k_ - 1) {
            counters_[item] = 1;
        } else {
            std::vector<T> to_remove;
            for (auto& kv : counters_) {
                kv.second--;
                if (kv.second == 0) {
                    to_remove.push_back(kv.first);
                }
            }
            for (const auto& rem : to_remove) {
                counters_.erase(rem);
            }
        }
    }

    [[nodiscard]] int64_t estimate(const T& item) const {
        auto it = counters_.find(item);
        if (it != counters_.end()) return it->second;
        return 0;
    }

    [[nodiscard]] const std::unordered_map<T, int64_t>& get_candidates() const noexcept {
        return counters_;
    }

    [[nodiscard]] int64_t total_count() const noexcept {
        return total_elements_;
    }

    [[nodiscard]] size_t capacity() const noexcept {
        return k_;
    }
};

/**
 * @brief Reservoir Sampling (Algorithm R).
 * Uniformly samples k items from an arbitrarily long stream in O(k) space.
 */
template <typename T>
class ReservoirSampler {
private:
    size_t k_{0};
    size_t count_{0};
    std::vector<T> reservoir_;
    mutable std::mt19937 rng_;

public:
    explicit ReservoirSampler(size_t k, uint32_t seed = 42) : k_(k), rng_(seed) {
        assert(k > 0);
        reservoir_.reserve(k);
    }

    void process(const T& item) {
        count_++;
        if (reservoir_.size() < k_) {
            reservoir_.push_back(item);
        } else {
            std::uniform_int_distribution<size_t> dist(0, count_ - 1);
            size_t j = dist(rng_);
            if (j < k_) {
                reservoir_[j] = item;
            }
        }
    }

    [[nodiscard]] const std::vector<T>& sample() const noexcept {
        return reservoir_;
    }

    [[nodiscard]] size_t total_seen() const noexcept {
        return count_;
    }
};

/**
 * @brief DGIM Algorithm for counting 1-bits in a sliding window with (1 +/- eps) relative error.
 */
class DGIM {
public:
    struct Bucket {
        uint64_t timestamp;
        uint64_t size; // Power of 2: 1, 2, 4, 8, ...
    };

private:
    uint64_t window_size_{0};
    size_t k_{0}; // max buckets of each size: ceil(1 / eps)
    uint64_t current_time_{0};
    std::deque<Bucket> buckets_;

public:
    DGIM(uint64_t window_size, double eps = 0.5) : window_size_(window_size) {
        assert(window_size > 0);
        assert(eps > 0.0 && eps <= 1.0);
        k_ = std::max(size_t(2), static_cast<size_t>(std::ceil(1.0 / eps)));
    }

    void process(bool bit) {
        current_time_++;

        // Drop buckets outside sliding window
        while (!buckets_.empty() && buckets_.back().timestamp + window_size_ <= current_time_) {
            buckets_.pop_back();
        }

        if (!bit) return;

        // Add new bucket of size 1 at front (newest timestamp)
        buckets_.push_front({current_time_, 1});

        // Merge check from size 1 upwards
        uint64_t cur_size = 1;
        while (true) {
            std::vector<size_t> matching;
            for (size_t i = 0; i < buckets_.size(); ++i) {
                if (buckets_[i].size == cur_size) {
                    matching.push_back(i);
                }
            }

            if (matching.size() > k_) {
                // Merge the two oldest of this size (last two indices in matching)
                size_t idx_newer = matching[matching.size() - 2];
                size_t idx_older = matching[matching.size() - 1];

                buckets_[idx_newer].size *= 2;
                buckets_.erase(buckets_.begin() + idx_older);
                cur_size *= 2;
            } else {
                break;
            }
        }
    }

    [[nodiscard]] uint64_t count_ones() const {
        if (buckets_.empty()) return 0;

        uint64_t total = 0;
        uint64_t last_size = 0;

        for (const auto& b : buckets_) {
            total += b.size;
            last_size = b.size;
        }

        return total - (last_size / 2);
    }

    [[nodiscard]] size_t num_buckets() const noexcept {
        return buckets_.size();
    }
};

} // namespace dsa

int main() {
    using namespace dsa;
    std::cout << "Testing Streaming Models and Algorithms...\n";

    // 1. Verify Misra-Gries Heavy Hitters
    {
        MisraGries<int> mg(4); // k = 4, threshold = N / 4 = 25%
        for (int i = 0; i < 400; ++i) mg.process(99);
        for (int i = 0; i < 300; ++i) mg.process(42);
        for (int i = 0; i < 300; ++i) mg.process(1000 + i);

        auto candidates = mg.get_candidates();
        assert(candidates.count(99) > 0);
        assert(candidates.count(42) > 0);
        assert(mg.estimate(99) >= 400 - 1000 / 4);
        assert(mg.estimate(42) >= 300 - 1000 / 4);
        assert(mg.total_count() == 1000);
    }

    // 2. Verify Reservoir Sampling
    {
        ReservoirSampler<int> sampler(10, 1337);
        for (int i = 0; i < 1000; ++i) {
            sampler.process(i);
        }
        assert(sampler.sample().size() == 10);
        assert(sampler.total_seen() == 1000);
    }

    // 3. Verify DGIM Sliding Window
    {
        size_t W = 100;
        DGIM dgim(W, 0.5);
        std::deque<bool> actual_window;

        std::mt19937 rng(42);
        for (int step = 0; step < 5000; ++step) {
            bool b = (rng() % 3 == 0); // 1 with ~33% prob
            dgim.process(b);
            actual_window.push_back(b);
            if (actual_window.size() > W) {
                actual_window.pop_front();
            }

            if (step >= static_cast<int>(W)) {
                uint64_t true_count = 0;
                for (bool val : actual_window) {
                    if (val) true_count++;
                }
                uint64_t est_count = dgim.count_ones();
                if (true_count > 0) {
                    double rel_error = std::abs(static_cast<double>(est_count) - static_cast<double>(true_count)) / true_count;
                    assert(rel_error <= 0.5001);
                }
            }
        }
    }

    std::cout << "All streaming models and algorithms passed differential verification!\n";
    return 0;
}
