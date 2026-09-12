#include <cassert>
#include <cstdint>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

namespace dsa {

/**
 * @brief Utilities for single 64-bit machine words.
 */
class WordBits {
public:
    static constexpr size_t BITS_PER_WORD = 64;

    static inline bool test(uint64_t word, size_t bit) {
        return (word & (1ULL << bit)) != 0;
    }

    static inline uint64_t set(uint64_t word, size_t bit) {
        return word | (1ULL << bit);
    }

    static inline uint64_t clear(uint64_t word, size_t bit) {
        return word & ~(1ULL << bit);
    }

    static inline uint64_t flip(uint64_t word, size_t bit) {
        return word ^ (1ULL << bit);
    }

    static inline int popcount(uint64_t word) {
        return __builtin_popcountll(word);
    }

    static inline int ctz(uint64_t word) {
        if (word == 0) return 64;
        return __builtin_ctzll(word);
    }

    static inline uint64_t lowest_set_bit(uint64_t word) {
        return word & (~word + 1ULL);
    }

    template <typename Callback>
    static void iterate_set_bits(uint64_t word, Callback&& cb) {
        while (word != 0) {
            int bit = __builtin_ctzll(word);
            cb(static_cast<size_t>(bit));
            word &= (word - 1ULL);
        }
    }
};

/**
 * @brief Dynamically sized contiguous bitvector backed by 64-bit integer words.
 *
 * Provides O(1) single-bit access and O(N / 64) whole-vector bitwise operations,
 * delivering roughly 64x throughput improvements over boolean arrays.
 */
class BitVector {
private:
    size_t num_bits_;
    std::vector<uint64_t> words_;

    static constexpr size_t BITS_PER_WORD = 64;

    static inline size_t word_index(size_t bit) {
        return bit / BITS_PER_WORD;
    }

    static inline size_t bit_offset(size_t bit) {
        return bit % BITS_PER_WORD;
    }

    static inline uint64_t bit_mask(size_t bit) {
        return 1ULL << bit_offset(bit);
    }

    void mask_trailing_bits() {
        size_t rem = num_bits_ % BITS_PER_WORD;
        if (rem != 0 && !words_.empty()) {
            uint64_t mask = (1ULL << rem) - 1ULL;
            words_.back() &= mask;
        }
    }

public:
    explicit BitVector(size_t num_bits = 0, bool init_val = false)
        : num_bits_(num_bits),
          words_((num_bits + BITS_PER_WORD - 1) / BITS_PER_WORD, init_val ? ~0ULL : 0ULL) {
        mask_trailing_bits();
    }

    size_t size() const noexcept {
        return num_bits_;
    }

    size_t num_words() const noexcept {
        return words_.size();
    }

    bool empty() const noexcept {
        return num_bits_ == 0;
    }

    bool test(size_t i) const {
        if (i >= num_bits_) {
            throw std::out_of_range("BitVector::test index out of range");
        }
        return (words_[word_index(i)] & bit_mask(i)) != 0;
    }

    bool operator[](size_t i) const {
        return test(i);
    }

    void set(size_t i, bool val = true) {
        if (i >= num_bits_) {
            throw std::out_of_range("BitVector::set index out of range");
        }
        if (val) {
            words_[word_index(i)] |= bit_mask(i);
        } else {
            words_[word_index(i)] &= ~bit_mask(i);
        }
    }

    void reset(size_t i) {
        set(i, false);
    }

    void flip(size_t i) {
        if (i >= num_bits_) {
            throw std::out_of_range("BitVector::flip index out of range");
        }
        words_[word_index(i)] ^= bit_mask(i);
    }

    void clear() {
        std::fill(words_.begin(), words_.end(), 0ULL);
    }

    void set_all() {
        std::fill(words_.begin(), words_.end(), ~0ULL);
        mask_trailing_bits();
    }

    size_t count() const {
        size_t total = 0;
        for (uint64_t w : words_) {
            total += static_cast<size_t>(__builtin_popcountll(w));
        }
        return total;
    }

    bool any() const {
        for (uint64_t w : words_) {
            if (w != 0) return true;
        }
        return false;
    }

    bool none() const {
        return !any();
    }

    bool all() const {
        if (num_bits_ == 0) return true;
        return count() == num_bits_;
    }

    std::vector<size_t> get_set_bits() const {
        std::vector<size_t> indices;
        indices.reserve(count());
        for (size_t w_idx = 0; w_idx < words_.size(); ++w_idx) {
            uint64_t word = words_[w_idx];
            size_t base = w_idx * BITS_PER_WORD;
            while (word != 0) {
                int bit = __builtin_ctzll(word);
                indices.push_back(base + static_cast<size_t>(bit));
                word &= (word - 1ULL);
            }
        }
        return indices;
    }

    BitVector operator~() const {
        BitVector result = *this;
        for (size_t i = 0; i < result.words_.size(); ++i) {
            result.words_[i] = ~result.words_[i];
        }
        result.mask_trailing_bits();
        return result;
    }

    BitVector& operator&=(const BitVector& other) {
        if (num_bits_ != other.num_bits_) {
            throw std::invalid_argument("BitVector sizes must match for operator&=");
        }
        for (size_t i = 0; i < words_.size(); ++i) {
            words_[i] &= other.words_[i];
        }
        return *this;
    }

    BitVector& operator|=(const BitVector& other) {
        if (num_bits_ != other.num_bits_) {
            throw std::invalid_argument("BitVector sizes must match for operator|=");
        }
        for (size_t i = 0; i < words_.size(); ++i) {
            words_[i] |= other.words_[i];
        }
        return *this;
    }

    BitVector& operator^=(const BitVector& other) {
        if (num_bits_ != other.num_bits_) {
            throw std::invalid_argument("BitVector sizes must match for operator^=");
        }
        for (size_t i = 0; i < words_.size(); ++i) {
            words_[i] ^= other.words_[i];
        }
        return *this;
    }

    friend BitVector operator&(BitVector lhs, const BitVector& rhs) {
        lhs &= rhs;
        return lhs;
    }

    friend BitVector operator|(BitVector lhs, const BitVector& rhs) {
        lhs |= rhs;
        return lhs;
    }

    friend BitVector operator^(BitVector lhs, const BitVector& rhs) {
        lhs ^= rhs;
        return lhs;
    }

    bool operator==(const BitVector& other) const {
        if (num_bits_ != other.num_bits_) return false;
        return words_ == other.words_;
    }

    bool operator!=(const BitVector& other) const {
        return !(*this == other);
    }

    std::string to_string() const {
        std::string s(num_bits_, '0');
        for (size_t i = 0; i < num_bits_; ++i) {
            if (test(i)) {
                s[i] = '1';
            }
        }
        return s;
    }
};

/**
 * @brief Sieve of Eratosthenes using BitVector visited table.
 * Demonstrates high density and cache efficiency.
 */
inline std::vector<size_t> sieve_primes(size_t limit) {
    if (limit < 2) return {};
    BitVector is_prime(limit + 1, true);
    is_prime.reset(0);
    is_prime.reset(1);

    for (size_t p = 2; p * p <= limit; ++p) {
        if (is_prime.test(p)) {
            for (size_t mult = p * p; mult <= limit; mult += p) {
                is_prime.reset(mult);
            }
        }
    }
    return is_prime.get_set_bits();
}

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Bitsets and Bitvectors C++17 Verification..." << std::endl;

    // 1. Test WordBits primitives
    {
        uint64_t w = 0;
        w = WordBits::set(w, 3);
        w = WordBits::set(w, 7);
        w = WordBits::set(w, 63);
        assert(WordBits::test(w, 3));
        assert(WordBits::test(w, 7));
        assert(WordBits::test(w, 63));
        assert(!WordBits::test(w, 0));
        assert(!WordBits::test(w, 62));

        assert(WordBits::popcount(w) == 3);
        assert(WordBits::ctz(w) == 3);
        assert(WordBits::lowest_set_bit(w) == (1ULL << 3));

        std::vector<size_t> collected;
        WordBits::iterate_set_bits(w, [&](size_t bit) { collected.push_back(bit); });
        assert(collected.size() == 3);
        assert(collected[0] == 3 && collected[1] == 7 && collected[2] == 63);

        w = WordBits::clear(w, 7);
        assert(!WordBits::test(w, 7));
        assert(WordBits::popcount(w) == 2);

        w = WordBits::flip(w, 0);
        assert(WordBits::test(w, 0));
        w = WordBits::flip(w, 0);
        assert(!WordBits::test(w, 0));
    }

    // 2. Test BitVector initialization and boundary sizes
    {
        BitVector b0(0);
        assert(b0.size() == 0);
        assert(b0.num_words() == 0);
        assert(b0.empty());
        assert(b0.count() == 0);
        assert(b0.all());

        BitVector b1(1, true);
        assert(b1.size() == 1);
        assert(b1.num_words() == 1);
        assert(b1.test(0));
        assert(b1.count() == 1);
        assert(b1.all());

        BitVector b64(64, true);
        assert(b64.size() == 64);
        assert(b64.num_words() == 1);
        assert(b64.count() == 64);
        assert(b64.all());

        BitVector b65(65, false);
        assert(b65.size() == 65);
        assert(b65.num_words() == 2);
        assert(b65.count() == 0);
        assert(b65.none());
    }

    // 3. Test BitVector single-bit modifications across multiple words
    {
        BitVector bv(200, false);
        assert(bv.size() == 200);
        assert(bv.num_words() == 4);

        bv.set(0);
        bv.set(63);
        bv.set(64);
        bv.set(127);
        bv.set(128);
        bv.set(199);

        assert(bv.test(0));
        assert(bv.test(63));
        assert(bv.test(64));
        assert(bv.test(127));
        assert(bv.test(128));
        assert(bv.test(199));
        assert(!bv.test(1));
        assert(!bv.test(65));
        assert(!bv.test(198));

        assert(bv.count() == 6);
        assert(bv.any());
        assert(!bv.none());
        assert(!bv.all());

        auto set_bits = bv.get_set_bits();
        assert(set_bits.size() == 6);
        assert((set_bits == std::vector<size_t>{0, 63, 64, 127, 128, 199}));

        bv.flip(64);
        assert(!bv.test(64));
        assert(bv.count() == 5);

        bv.flip(64);
        assert(bv.test(64));
        assert(bv.count() == 6);
    }

    // 4. Test Whole-Vector Bitwise Operations and Trailing Bit Masking
    {
        BitVector a(70, false);
        BitVector b(70, false);

        a.set(10);
        a.set(65);
        a.set(69);

        b.set(10);
        b.set(20);
        b.set(65);

        BitVector and_res = a & b;
        assert(and_res.count() == 2);
        assert(and_res.test(10) && and_res.test(65));
        assert(!and_res.test(20) && !and_res.test(69));

        BitVector or_res = a | b;
        assert(or_res.count() == 4);
        assert(or_res.test(10) && or_res.test(20) && or_res.test(65) && or_res.test(69));

        BitVector xor_res = a ^ b;
        assert(xor_res.count() == 2);
        assert(xor_res.test(20) && xor_res.test(69));
        assert(!xor_res.test(10) && !xor_res.test(65));

        BitVector not_a = ~a;
        assert(not_a.size() == 70);
        assert(not_a.count() == 70 - 3);
        assert(!not_a.test(10) && !not_a.test(65) && !not_a.test(69));
        assert(not_a.test(0) && not_a.test(68));

        // Test set_all and clear
        a.set_all();
        assert(a.count() == 70);
        assert(a.all());
        a.clear();
        assert(a.count() == 0);
        assert(a.none());
    }

    // 5. Test Exceptions on Out of Range & Mismatched Sizes
    {
        BitVector bv(10);
        bool caught = false;
        try {
            bv.test(10);
        } catch (const std::out_of_range&) {
            caught = true;
        }
        assert(caught);

        caught = false;
        try {
            bv.set(10);
        } catch (const std::out_of_range&) {
            caught = true;
        }
        assert(caught);

        BitVector other(11);
        caught = false;
        try {
            bv &= other;
        } catch (const std::invalid_argument&) {
            caught = true;
        }
        assert(caught);
    }

    // 6. Test Prime Sieve with BitVector
    {
        auto primes = sieve_primes(30);
        std::vector<size_t> expected = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29};
        assert(primes == expected);

        auto primes100 = sieve_primes(100);
        assert(primes100.size() == 25);
    }

    std::cout << "[PASSED] Bitsets and Bitvectors C++17 All Tests Passed!" << std::endl;
    return 0;
}
