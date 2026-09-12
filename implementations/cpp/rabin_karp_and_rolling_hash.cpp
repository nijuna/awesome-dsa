#include <cassert>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

namespace dsa {

/**
 * @brief Double hash value pair.
 */
struct HashVal {
    uint64_t h1;
    uint64_t h2;

    bool operator==(const HashVal& o) const noexcept {
        return h1 == o.h1 && h2 == o.h2;
    }
    bool operator!=(const HashVal& o) const noexcept {
        return !(*this == o);
    }
};

/**
 * @brief Prefix rolling hash supporting O(1) substring hash queries via double hashing.
 */
class PrefixRollingHash {
private:
    static constexpr uint64_t B1 = 911ULL;
    static constexpr uint64_t M1 = 1000000007ULL;
    static constexpr uint64_t B2 = 997ULL;
    static constexpr uint64_t M2 = 1000000009ULL;

    std::string s_;
    std::vector<uint64_t> pref1_, pref2_;
    std::vector<uint64_t> pow1_, pow2_;

    static inline uint64_t char_val(char c) {
        return static_cast<uint64_t>(static_cast<unsigned char>(c)) + 1ULL;
    }

public:
    explicit PrefixRollingHash(const std::string& s)
        : s_(s), pref1_(s.size() + 1, 0), pref2_(s.size() + 1, 0),
          pow1_(s.size() + 1, 1), pow2_(s.size() + 1, 1) {
        for (size_t i = 0; i < s.size(); ++i) {
            uint64_t v = char_val(s[i]);
            pref1_[i + 1] = (pref1_[i] * B1 + v) % M1;
            pref2_[i + 1] = (pref2_[i] * B2 + v) % M2;
            pow1_[i + 1] = (pow1_[i] * B1) % M1;
            pow2_[i + 1] = (pow2_[i] * B2) % M2;
        }
    }

    size_t size() const noexcept { return s_.size(); }

    /**
     * @brief Returns the double hash of substring s[l..r] (inclusive).
     */
    HashVal query(size_t l, size_t r) const {
        if (l > r || r >= s_.size()) {
            throw std::out_of_range("PrefixRollingHash::query index out of range");
        }
        size_t len = r - l + 1;

        int64_t val1 = static_cast<int64_t>(pref1_[r + 1]) -
                       static_cast<int64_t>((pref1_[l] * pow1_[len]) % M1);
        if (val1 < 0) val1 += M1;

        int64_t val2 = static_cast<int64_t>(pref2_[r + 1]) -
                       static_cast<int64_t>((pref2_[l] * pow2_[len]) % M2);
        if (val2 < 0) val2 += M2;

        return {static_cast<uint64_t>(val1), static_cast<uint64_t>(val2)};
    }
};

/**
 * @brief Rabin-Karp exact string matching algorithm.
 * Returns all starting indices where pattern occurs in text.
 */
inline std::vector<int> rabin_karp_search(const std::string& text, const std::string& pattern) {
    std::vector<int> matches;
    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0 || m > n) return matches;

    const int64_t B = 911382323LL;
    const int64_t M = 972663749LL;

    auto val = [](char c) -> int64_t {
        return static_cast<int64_t>(static_cast<unsigned char>(c)) + 1LL;
    };

    int64_t pattern_hash = 0;
    int64_t window_hash = 0;
    int64_t power = 1;

    for (int i = 0; i < m - 1; ++i) {
        power = (power * B) % M;
    }

    for (int i = 0; i < m; ++i) {
        pattern_hash = (pattern_hash * B + val(pattern[i])) % M;
        window_hash = (window_hash * B + val(text[i])) % M;
    }

    for (int i = 0; i + m <= n; ++i) {
        if (window_hash == pattern_hash) {
            // Confirm with direct character comparison to eliminate collisions
            if (text.compare(i, m, pattern) == 0) {
                matches.push_back(i);
            }
        }

        if (i + m < n) {
            window_hash = (window_hash - val(text[i]) * power) % M;
            if (window_hash < 0) window_hash += M;
            window_hash = (window_hash * B + val(text[i + m])) % M;
        }
    }

    return matches;
}

/**
 * @brief Compute Longest Common Prefix (LCP) of two substrings using binary search and prefix hashing.
 * Time complexity: O(log(min(len1, len2))).
 */
inline size_t compute_lcp(const PrefixRollingHash& h1, size_t idx1,
                          const PrefixRollingHash& h2, size_t idx2,
                          size_t max_len) {
    size_t low = 0, high = max_len;
    size_t best = 0;

    while (low <= high) {
        size_t mid = low + (high - low) / 2;
        if (mid == 0) {
            low = 1;
            continue;
        }
        if (h1.query(idx1, idx1 + mid - 1) == h2.query(idx2, idx2 + mid - 1)) {
            best = mid;
            low = mid + 1; // Try longer prefix
        } else {
            high = mid - 1;
        }
    }
    return best;
}

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Rabin-Karp and Rolling Hash C++17 Verification..." << std::endl;

    // 1. Rabin-Karp Search
    {
        std::string text = "AABAACAADAABAABA";
        std::string pat = "AABA";
        auto matches = rabin_karp_search(text, pat);
        std::vector<int> expected = {0, 9, 12};
        assert(matches == expected);

        // Pattern not present
        assert(rabin_karp_search("ABCDEF", "XYZ").empty());

        // Pattern equals text
        assert(rabin_karp_search("ABC", "ABC") == std::vector<int>{0});

        // Pattern longer than text
        assert(rabin_karp_search("AB", "ABC").empty());

        // Overlapping matches
        std::string text2 = "AAAA";
        std::string pat2 = "AA";
        assert((rabin_karp_search(text2, pat2) == std::vector<int>{0, 1, 2}));
    }

    // 2. Prefix Rolling Hash Substring Queries
    {
        std::string s = "abacaba";
        PrefixRollingHash prh(s);

        // Identical substrings "aba" at indices [0..2] and [4..6]
        HashVal h1 = prh.query(0, 2);
        HashVal h2 = prh.query(4, 6);
        assert(h1 == h2);

        // Distinct substrings "aba" vs "bac"
        HashVal h3 = prh.query(1, 3);
        assert(h1 != h3);
    }

    // 3. Binary Search LCP with Prefix Rolling Hash
    {
        std::string s = "banana";
        PrefixRollingHash prh(s);

        // Suffix "anana" (index 1) vs "ana" (index 3)
        // LCP should be "ana" (length 3)
        size_t lcp = compute_lcp(prh, 1, prh, 3, 3);
        assert(lcp == 3);

        // Suffix "banana" (index 0) vs "anana" (index 1) -> LCP = 0
        size_t lcp2 = compute_lcp(prh, 0, prh, 1, 5);
        assert(lcp2 == 0);
    }

    std::cout << "[PASSED] Rabin-Karp and Rolling Hash C++17 All Tests Passed!" << std::endl;
    return 0;
}
