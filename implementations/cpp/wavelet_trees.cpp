/**
 * @file wavelet_trees.cpp
 * @brief High-performance C++17 implementation of Wavelet Trees.
 *
 * Implements Grossi, Gupta, and Vitter's succinct sequence decomposition (SODA 2003):
 * - BitVector with packed 64-bit words and prefix popcount directory for O(1) rank queries.
 * - Wavelet Tree over arbitrary integer alphabet Sigma:
 *   1. Access(i): Retrieve A[i] in O(log |Sigma|).
 *   2. Rank(i, c): Count occurrences of c in A[0 ... i] in O(log |Sigma|).
 *   3. RangeCount(L, R, c): Count occurrences of c in A[L ... R] in O(log |Sigma|).
 *   4. Quantile(L, R, k): Find k-th smallest element in A[L ... R] in O(log |Sigma|).
 *   5. RangeFrequency(L, R, low, high): Count elements in A[L ... R] with values in [low, high] in O(log |Sigma|).
 * - Total Space: O(N log |Sigma|) bits = O(N) words.
 *
 * Fully compliant with -std=c++17 -O3 -Wall -Wextra -Werror.
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

namespace dsa {

/**
 * @brief BitVector supporting O(1) rank queries using 64-bit blocks and prefix directory.
 */
class BitVector {
private:
    std::vector<uint64_t> words_;
    std::vector<uint32_t> prefix_popcounts_;
    size_t n_{0};

public:
    BitVector() = default;

    explicit BitVector(const std::vector<bool>& bits) : n_(bits.size()) {
        size_t num_words = (n_ + 63) / 64;
        words_.assign(num_words, 0);
        prefix_popcounts_.assign(num_words + 1, 0);

        for (size_t i = 0; i < n_; ++i) {
            if (bits[i]) {
                words_[i / 64] |= (1ULL << (i % 64));
            }
        }

        uint32_t running = 0;
        for (size_t i = 0; i < num_words; ++i) {
            prefix_popcounts_[i] = running;
            running += __builtin_popcountll(words_[i]);
        }
        prefix_popcounts_[num_words] = running;
    }

    [[nodiscard]] size_t size() const noexcept { return n_; }

    [[nodiscard]] bool get(size_t i) const noexcept {
        return (words_[i / 64] >> (i % 64)) & 1ULL;
    }

    /**
     * @brief Number of 1-bits in prefix [0, i].
     */
    [[nodiscard]] uint32_t rank1(int64_t i) const noexcept {
        if (i < 0) return 0;
        if (static_cast<size_t>(i) >= n_) i = n_ - 1;

        size_t word_idx = static_cast<size_t>(i) / 64;
        size_t bit_idx = static_cast<size_t>(i) % 64;

        uint32_t count = prefix_popcounts_[word_idx];
        uint64_t mask = (bit_idx == 63) ? ~0ULL : ((1ULL << (bit_idx + 1)) - 1ULL);
        count += __builtin_popcountll(words_[word_idx] & mask);
        return count;
    }

    /**
     * @brief Number of 0-bits in prefix [0, i].
     */
    [[nodiscard]] uint32_t rank0(int64_t i) const noexcept {
        if (i < 0) return 0;
        if (static_cast<size_t>(i) >= n_) i = n_ - 1;
        return static_cast<uint32_t>(i + 1) - rank1(i);
    }
};

/**
 * @brief Wavelet Tree over an arbitrary sequence of integers in [min_val, max_val].
 */
class WaveletTree {
private:
    int64_t low_{0};
    int64_t high_{0};
    BitVector bv_;
    WaveletTree* left_{nullptr};
    WaveletTree* right_{nullptr};

public:
    WaveletTree() = default;

    template <typename It>
    WaveletTree(It from, It to, int64_t low, int64_t high) : low_(low), high_(high) {
        if (from >= to || low_ >= high_) return;

        int64_t mid = low_ + (high_ - low_) / 2;
        std::vector<bool> bits;
        bits.reserve(to - from);

        for (auto it = from; it != to; ++it) {
            bits.push_back(*it > mid);
        }
        bv_ = BitVector(bits);

        // Partition elements stably into left (<= mid) and right (> mid)
        auto pivot = std::stable_partition(from, to, [mid](int64_t x) {
            return x <= mid;
        });

        left_ = new WaveletTree(from, pivot, low_, mid);
        right_ = new WaveletTree(pivot, to, mid + 1, high_);
    }

    ~WaveletTree() {
        delete left_;
        delete right_;
    }

    WaveletTree(const WaveletTree&) = delete;
    WaveletTree& operator=(const WaveletTree&) = delete;

    WaveletTree(WaveletTree&& other) noexcept
        : low_(other.low_), high_(other.high_), bv_(std::move(other.bv_)),
          left_(other.left_), right_(other.right_) {
        other.left_ = other.right_ = nullptr;
    }

    WaveletTree& operator=(WaveletTree&& other) noexcept {
        if (this != &other) {
            delete left_;
            delete right_;
            low_ = other.low_;
            high_ = other.high_;
            bv_ = std::move(other.bv_);
            left_ = other.left_;
            right_ = other.right_;
            other.left_ = other.right_ = nullptr;
        }
        return *this;
    }

    /**
     * @brief Access element at index i (0-indexed).
     */
    [[nodiscard]] int64_t access(size_t i) const {
        if (low_ == high_) return low_;
        bool bit = bv_.get(i);
        if (!bit) {
            return left_->access(bv_.rank0(static_cast<int64_t>(i)) - 1);
        } else {
            return right_->access(bv_.rank1(static_cast<int64_t>(i)) - 1);
        }
    }

    /**
     * @brief Count occurrences of symbol c in prefix [0, i].
     */
    [[nodiscard]] uint32_t rank(int64_t i, int64_t c) const {
        if (i < 0 || c < low_ || c > high_) return 0;
        if (low_ == high_) return static_cast<uint32_t>(i + 1);

        int64_t mid = low_ + (high_ - low_) / 2;
        if (c <= mid) {
            return left_->rank(static_cast<int64_t>(bv_.rank0(i)) - 1, c);
        } else {
            return right_->rank(static_cast<int64_t>(bv_.rank1(i)) - 1, c);
        }
    }

    /**
     * @brief Count occurrences of symbol c in range [L, R].
     */
    [[nodiscard]] uint32_t range_count(size_t L, size_t R, int64_t c) const {
        if (L > R) return 0;
        return rank(static_cast<int64_t>(R), c) - rank(static_cast<int64_t>(L) - 1, c);
    }

    /**
     * @brief Find k-th smallest element (1-indexed) in range [L, R].
     */
    [[nodiscard]] int64_t quantile(size_t L, size_t R, uint32_t k) const {
        if (low_ == high_) return low_;

        int64_t left_L = static_cast<int64_t>(bv_.rank0(static_cast<int64_t>(L) - 1));
        int64_t left_R = static_cast<int64_t>(bv_.rank0(static_cast<int64_t>(R))) - 1;
        uint32_t zeros_in_range = (left_R >= left_L) ? static_cast<uint32_t>(left_R - left_L + 1) : 0;

        if (k <= zeros_in_range) {
            return left_->quantile(static_cast<size_t>(left_L), static_cast<size_t>(left_R), k);
        } else {
            int64_t right_L = static_cast<int64_t>(bv_.rank1(static_cast<int64_t>(L) - 1));
            int64_t right_R = static_cast<int64_t>(bv_.rank1(static_cast<int64_t>(R))) - 1;
            return right_->quantile(static_cast<size_t>(right_L), static_cast<size_t>(right_R), k - zeros_in_range);
        }
    }

    /**
     * @brief Count number of elements in range [L, R] whose values lie in [val_low, val_high].
     */
    [[nodiscard]] uint32_t range_frequency(size_t L, size_t R, int64_t val_low, int64_t val_high) const {
        if (L > R || val_low > high_ || val_high < low_) return 0;
        if (val_low <= low_ && high_ <= val_high) {
            return static_cast<uint32_t>(R - L + 1);
        }

        int64_t left_L = static_cast<int64_t>(bv_.rank0(static_cast<int64_t>(L) - 1));
        int64_t left_R = static_cast<int64_t>(bv_.rank0(static_cast<int64_t>(R))) - 1;

        int64_t right_L = static_cast<int64_t>(bv_.rank1(static_cast<int64_t>(L) - 1));
        int64_t right_R = static_cast<int64_t>(bv_.rank1(static_cast<int64_t>(R))) - 1;

        uint32_t res = 0;
        if (left_R >= left_L && left_) {
            res += left_->range_frequency(static_cast<size_t>(left_L), static_cast<size_t>(left_R), val_low, val_high);
        }
        if (right_R >= right_L && right_) {
            res += right_->range_frequency(static_cast<size_t>(right_L), static_cast<size_t>(right_R), val_low, val_high);
        }
        return res;
    }
};

} // namespace dsa

int main() {
    using namespace dsa;
    std::cout << "Testing Wavelet Tree against brute-force oracles...\n";

    // 1. Basic operations verification
    {
        std::vector<int64_t> arr = {5, 2, 8, 3, 9, 2, 7, 1, 6, 4, 3, 8};
        std::vector<int64_t> copy_arr = arr;
        WaveletTree wt(copy_arr.begin(), copy_arr.end(), 1, 9);

        for (size_t i = 0; i < arr.size(); ++i) {
            assert(wt.access(i) == arr[i]);
        }

        assert(wt.range_count(0, 11, 2) == 2);
        assert(wt.range_count(0, 11, 8) == 2);
        assert(wt.range_count(0, 11, 10) == 0);

        assert(wt.quantile(0, 4, 1) == 2); // {5, 2, 8, 3, 9} -> sorted: {2, 3, 5, 8, 9}
        assert(wt.quantile(0, 4, 2) == 3);
        assert(wt.quantile(0, 4, 3) == 5);
        assert(wt.quantile(0, 4, 5) == 9);

        assert(wt.range_frequency(0, 4, 2, 5) == 3); // 2, 3, 5
    }

    // 2. Large Scale Randomized Differential Verification
    {
        std::mt19937 rng(1337);
        size_t N = 1000;
        std::vector<int64_t> big_arr(N);
        std::uniform_int_distribution<int64_t> val_dist(0, 200);
        for (size_t i = 0; i < N; ++i) {
            big_arr[i] = val_dist(rng);
        }
        std::vector<int64_t> big_copy = big_arr;
        WaveletTree big_wt(big_copy.begin(), big_copy.end(), 0, 200);

        for (int it = 0; it < 5000; ++it) {
            size_t l = rng() % N;
            size_t r = rng() % N;
            if (l > r) std::swap(l, r);

            // Quantile check
            uint32_t k = (rng() % (r - l + 1)) + 1;
            std::vector<int64_t> sub(big_arr.begin() + l, big_arr.begin() + r + 1);
            std::sort(sub.begin(), sub.end());
            int64_t expected_kth = sub[k - 1];
            int64_t actual_kth = big_wt.quantile(l, r, k);
            assert(actual_kth == expected_kth);

            // Range frequency check
            int64_t v1 = val_dist(rng);
            int64_t v2 = val_dist(rng);
            if (v1 > v2) std::swap(v1, v2);

            uint32_t expected_freq = 0;
            for (size_t i = l; i <= r; ++i) {
                if (big_arr[i] >= v1 && big_arr[i] <= v2) expected_freq++;
            }
            uint32_t actual_freq = big_wt.range_frequency(l, r, v1, v2);
            assert(actual_freq == expected_freq);

            // Range count check
            int64_t c = val_dist(rng);
            uint32_t expected_c = 0;
            for (size_t i = l; i <= r; ++i) {
                if (big_arr[i] == c) expected_c++;
            }
            assert(big_wt.range_count(l, r, c) == expected_c);
        }
    }

    std::cout << "Wavelet Tree all tests passed with 100% precision!\n";
    return 0;
}
