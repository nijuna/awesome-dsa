/**
 * @file succinct_data_structures.cpp
 * @brief High-performance C++17 reference implementation of Succinct Bitvectors (Rank & Select).
 *
 * Implements modern succinct bitvectors following Jacobson's foundations:
 * - 64-bit packed words.
 * - 512-bit superblocks (8 words = 64 bytes, matching CPU cache line width).
 * - Only 6.25% space overhead (N + 0.0625 N bits).
 * - O(1) Access and Rank queries via hardware `__builtin_popcountll`.
 * - O(log(N / 512)) = O(log N) Select queries via superblock binary search and fast sub-word bit indexing.
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
 * @brief Succinct Bitvector with 512-bit superblocks.
 * Provides O(1) access and rank, with O(log N) select.
 */
class SuccinctBitVector {
public:
    static constexpr size_t SUPERBLOCK_BITS = 512;
    static constexpr size_t WORDS_PER_SUPERBLOCK = SUPERBLOCK_BITS / 64; // 8 words (64 bytes)

private:
    size_t n_{0};
    size_t num_words_{0};
    size_t num_superblocks_{0};
    std::vector<uint64_t> data_;
    std::vector<uint32_t> superblock_ranks_; // Cumulative 1-bits before each superblock

public:
    SuccinctBitVector() = default;

    explicit SuccinctBitVector(const std::vector<bool>& bits) : n_(bits.size()) {
        num_words_ = (n_ + 63) / 64;
        num_superblocks_ = (n_ + SUPERBLOCK_BITS - 1) / SUPERBLOCK_BITS;

        data_.assign(num_words_, 0);
        superblock_ranks_.assign(num_superblocks_ + 1, 0);

        for (size_t i = 0; i < n_; ++i) {
            if (bits[i]) {
                data_[i / 64] |= (1ULL << (i % 64));
            }
        }

        uint32_t running_rank = 0;
        for (size_t sb = 0; sb < num_superblocks_; ++sb) {
            superblock_ranks_[sb] = running_rank;
            size_t start_word = sb * WORDS_PER_SUPERBLOCK;
            size_t end_word = std::min(start_word + WORDS_PER_SUPERBLOCK, num_words_);
            for (size_t w = start_word; w < end_word; ++w) {
                running_rank += __builtin_popcountll(data_[w]);
            }
        }
        superblock_ranks_[num_superblocks_] = running_rank;
    }

    [[nodiscard]] size_t size() const noexcept { return n_; }
    [[nodiscard]] bool empty() const noexcept { return n_ == 0; }

    [[nodiscard]] uint32_t total_ones() const noexcept {
        return superblock_ranks_.empty() ? 0 : superblock_ranks_.back();
    }

    [[nodiscard]] uint32_t total_zeros() const noexcept {
        return static_cast<uint32_t>(n_) - total_ones();
    }

    [[nodiscard]] bool access(size_t i) const noexcept {
        return (data_[i / 64] >> (i % 64)) & 1ULL;
    }

    [[nodiscard]] bool operator[](size_t i) const noexcept {
        return access(i);
    }

    /**
     * @brief O(1) rank1: Count number of 1-bits in prefix [0, i].
     */
    [[nodiscard]] uint32_t rank1(int64_t i) const noexcept {
        if (i < 0) return 0;
        if (static_cast<size_t>(i) >= n_) i = n_ - 1;

        size_t idx = static_cast<size_t>(i);
        size_t sb = idx / SUPERBLOCK_BITS;
        uint32_t rank = superblock_ranks_[sb];

        size_t target_word = idx / 64;
        size_t sb_start_word = sb * WORDS_PER_SUPERBLOCK;

        for (size_t w = sb_start_word; w < target_word; ++w) {
            rank += __builtin_popcountll(data_[w]);
        }

        size_t bit_offset = idx % 64;
        uint64_t mask = (bit_offset == 63) ? ~0ULL : ((1ULL << (bit_offset + 1)) - 1ULL);
        rank += __builtin_popcountll(data_[target_word] & mask);
        return rank;
    }

    /**
     * @brief O(1) rank0: Count number of 0-bits in prefix [0, i].
     */
    [[nodiscard]] uint32_t rank0(int64_t i) const noexcept {
        if (i < 0) return 0;
        if (static_cast<size_t>(i) >= n_) i = n_ - 1;
        return static_cast<uint32_t>(i + 1) - rank1(i);
    }

    /**
     * @brief Select r-th set bit in 64-bit word (1 <= r <= popcount(w)).
     */
    static int select_in_word(uint64_t w, int r) noexcept {
        int pos = 0;
        for (int step = 32; step > 0; step /= 2) {
            uint64_t mask = (pos + step >= 64) ? ~0ULL : ((1ULL << (pos + step)) - 1ULL);
            int c = __builtin_popcountll(w & mask);
            if (c < r) {
                pos += step;
            }
        }
        return pos;
    }

    /**
     * @brief select1(k): 0-indexed position of the k-th 1-bit (1 <= k <= total_ones).
     */
    [[nodiscard]] int64_t select1(uint32_t k) const noexcept {
        if (k == 0 || k > total_ones()) return -1;

        // Binary search over superblocks
        size_t low = 0, high = num_superblocks_ - 1, sb = 0;
        while (low <= high) {
            size_t mid = low + (high - low) / 2;
            if (superblock_ranks_[mid + 1] >= k) {
                sb = mid;
                if (mid == 0) break;
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        }

        uint32_t rank_needed = k - superblock_ranks_[sb];
        size_t start_word = sb * WORDS_PER_SUPERBLOCK;
        size_t end_word = std::min(start_word + WORDS_PER_SUPERBLOCK, num_words_);

        for (size_t w = start_word; w < end_word; ++w) {
            int c = __builtin_popcountll(data_[w]);
            if (static_cast<uint32_t>(c) >= rank_needed) {
                int bit_idx = select_in_word(data_[w], rank_needed);
                return static_cast<int64_t>(w * 64 + bit_idx);
            }
            rank_needed -= c;
        }

        return -1;
    }

    /**
     * @brief select0(k): 0-indexed position of the k-th 0-bit (1 <= k <= total_zeros).
     */
    [[nodiscard]] int64_t select0(uint32_t k) const noexcept {
        if (k == 0 || k > total_zeros()) return -1;

        int64_t low = 0, high = static_cast<int64_t>(n_) - 1;
        int64_t ans = -1;

        while (low <= high) {
            int64_t mid = low + (high - low) / 2;
            if (rank0(mid) >= k) {
                ans = mid;
                high = mid - 1;
            } else {
                low = mid + 1;
            }
        }
        return ans;
    }
};

} // namespace dsa

int main() {
    using namespace dsa;
    std::cout << "Testing SuccinctBitVector with 512-bit superblocks...\n";

    // 1. Basic Unit Tests
    {
        std::vector<bool> bits = {1, 0, 1, 1, 0, 0, 1, 0, 1, 1};
        SuccinctBitVector sbv(bits);

        assert(sbv.size() == 10);
        assert(sbv.total_ones() == 6);
        assert(sbv.total_zeros() == 4);

        assert(sbv.rank1(0) == 1);
        assert(sbv.rank1(1) == 1);
        assert(sbv.rank1(2) == 2);
        assert(sbv.rank1(3) == 3);
        assert(sbv.rank1(4) == 3);
        assert(sbv.rank1(9) == 6);

        assert(sbv.rank0(0) == 0);
        assert(sbv.rank0(1) == 1);
        assert(sbv.rank0(4) == 2);
        assert(sbv.rank0(9) == 4);

        assert(sbv.select1(1) == 0);
        assert(sbv.select1(2) == 2);
        assert(sbv.select1(3) == 3);
        assert(sbv.select1(4) == 6);
        assert(sbv.select1(5) == 8);
        assert(sbv.select1(6) == 9);

        assert(sbv.select0(1) == 1);
        assert(sbv.select0(2) == 4);
        assert(sbv.select0(3) == 5);
        assert(sbv.select0(4) == 7);
    }

    // 2. Large Scale Verification across multiple 512-bit superblocks
    {
        size_t N = 10000;
        std::vector<bool> bits(N);
        std::mt19937 rng(42);
        for (size_t i = 0; i < N; ++i) {
            bits[i] = (rng() % 3 == 0); // ~33% ones
        }

        SuccinctBitVector sbv(bits);

        std::vector<uint32_t> oracle_rank1(N, 0);
        std::vector<uint32_t> oracle_rank0(N, 0);
        std::vector<int64_t> ones_pos;
        std::vector<int64_t> zeros_pos;

        uint32_t r1 = 0, r0 = 0;
        for (size_t i = 0; i < N; ++i) {
            if (bits[i]) {
                r1++;
                ones_pos.push_back(i);
            } else {
                r0++;
                zeros_pos.push_back(i);
            }
            oracle_rank1[i] = r1;
            oracle_rank0[i] = r0;
        }

        assert(sbv.total_ones() == r1);
        assert(sbv.total_zeros() == r0);

        for (size_t i = 0; i < N; ++i) {
            assert(sbv.access(i) == bits[i]);
            assert(sbv.rank1(i) == oracle_rank1[i]);
            assert(sbv.rank0(i) == oracle_rank0[i]);
        }

        for (size_t k = 1; k <= ones_pos.size(); ++k) {
            assert(sbv.select1(k) == ones_pos[k - 1]);
        }

        for (size_t k = 1; k <= zeros_pos.size(); ++k) {
            assert(sbv.select0(k) == zeros_pos[k - 1]);
        }
    }

    std::cout << "SuccinctBitVector all tests passed with 100% precision!\n";
    return 0;
}
