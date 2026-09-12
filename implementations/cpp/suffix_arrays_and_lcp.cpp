/**
 * Reference Implementation: Suffix Arrays & LCP Array
 * Demonstrates:
 * 1. O(n log n) Suffix Array construction via Prefix Doubling with Counting Sort
 * 2. O(n log^2 n) Suffix Array construction (clean baseline for comparison)
 * 3. Kasai's algorithm for linear-time LCP Array construction - O(n)
 * 4. Binary Search Substring Search & Range Query - O(m log n)
 * 5. Distinct Substrings Counting - O(n) after sa and lcp
 * 6. Longest Repeated Substring extraction - O(n) after sa and lcp
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <random>

namespace suffix_structures {

// ============================================================================
// 1. Suffix Array Construction (Prefix Doubling)
// ============================================================================

/**
 * Builds the Suffix Array using the Prefix Doubling technique with counting sort.
 * sa[r] is the starting index of the suffix having sorted rank r.
 * Time Complexity: O(n log n). Auxiliary Space: O(n).
 */
std::vector<int> build_suffix_array(const std::string& s) {
    int n = static_cast<int>(s.size());
    if (n == 0) return {};

    std::vector<int> sa(n), rank(n), tmp_sa(n), tmp_rank(n);

    // Initial rank based on ASCII value of the 1st character
    for (int i = 0; i < n; ++i) {
        sa[i] = i;
        rank[i] = static_cast<unsigned char>(s[i]);
    }

    // Sort initial 1-character prefixes
    std::sort(sa.begin(), sa.end(), [&](int a, int b) {
        return rank[a] < rank[b];
    });

    tmp_rank[sa[0]] = 0;
    for (int i = 1; i < n; ++i) {
        tmp_rank[sa[i]] = tmp_rank[sa[i - 1]] + (rank[sa[i]] != rank[sa[i - 1]] ? 1 : 0);
    }
    rank = tmp_rank;

    // Doubling phases: len = 1, 2, 4, 8, ...
    for (int len = 1; len < n && rank[sa[n - 1]] < n - 1; len <<= 1) {
        // Step 1: Sort by second component (rank[i + len])
        int p = 0;
        for (int i = n - len; i < n; ++i) {
            tmp_sa[p++] = i;
        }
        for (int i = 0; i < n; ++i) {
            if (sa[i] >= len) {
                tmp_sa[p++] = sa[i] - len;
            }
        }

        // Step 2: Counting sort by first component (rank[tmp_sa[i]])
        int max_rank = rank[sa[n - 1]] + 1;
        std::vector<int> count(max_rank, 0);
        for (int i = 0; i < n; ++i) {
            count[rank[tmp_sa[i]]]++;
        }
        for (int i = 1; i < max_rank; ++i) {
            count[i] += count[i - 1];
        }
        for (int i = n - 1; i >= 0; --i) {
            sa[--count[rank[tmp_sa[i]]]] = tmp_sa[i];
        }

        // Step 3: Compute new compressed ranks
        tmp_rank[sa[0]] = 0;
        for (int i = 1; i < n; ++i) {
            int prev = sa[i - 1];
            int curr = sa[i];
            bool differ = (rank[prev] != rank[curr]) ||
                          ((prev + len < n ? rank[prev + len] : -1) !=
                           (curr + len < n ? rank[curr + len] : -1));
            tmp_rank[curr] = tmp_rank[prev] + (differ ? 1 : 0);
        }
        rank = tmp_rank;
    }

    return sa;
}

/**
 * Clean baseline prefix-doubling suffix array using std::sort.
 * Time Complexity: O(n log^2 n). Auxiliary Space: O(n).
 */
std::vector<int> build_suffix_array_simple(const std::string& s) {
    int n = static_cast<int>(s.size());
    if (n == 0) return {};

    std::vector<int> sa(n), rank(n), tmp(n);
    for (int i = 0; i < n; ++i) {
        sa[i] = i;
        rank[i] = static_cast<unsigned char>(s[i]);
    }

    for (int len = 1; len < n; len <<= 1) {
        auto cmp = [&](int a, int b) {
            if (rank[a] != rank[b]) return rank[a] < rank[b];
            int ra = (a + len < n ? rank[a + len] : -1);
            int rb = (b + len < n ? rank[b + len] : -1);
            return ra < rb;
        };

        std::sort(sa.begin(), sa.end(), cmp);

        tmp[sa[0]] = 0;
        for (int i = 1; i < n; ++i) {
            tmp[sa[i]] = tmp[sa[i - 1]] + (cmp(sa[i - 1], sa[i]) ? 1 : 0);
        }

        rank = tmp;
        if (rank[sa[n - 1]] == n - 1) break;
    }

    return sa;
}

// ============================================================================
// 2. Kasai's LCP Array Construction
// ============================================================================

/**
 * Computes the Longest Common Prefix (LCP) array using Kasai's algorithm.
 * lcp[r] = length of the longest common prefix of suffixes sa[r] and sa[r - 1].
 * Convention: lcp[0] = 0.
 * Time Complexity: O(n). Auxiliary Space: O(n) for the inverse rank array.
 */
std::vector<int> build_lcp_array(const std::string& s, const std::vector<int>& sa) {
    int n = static_cast<int>(s.size());
    if (n == 0) return {};

    std::vector<int> rank(n), lcp(n, 0);
    for (int i = 0; i < n; ++i) {
        rank[sa[i]] = i;
    }

    int h = 0; // carried-over LCP length
    for (int i = 0; i < n; ++i) {
        int r = rank[i];
        if (r == 0) continue;

        int j = sa[r - 1];
        while (i + h < n && j + h < n && s[i + h] == s[j + h]) {
            ++h;
        }

        lcp[r] = h;
        if (h > 0) {
            --h; // invariant: next suffix loses at most 1 character of prefix match
        }
    }

    return lcp;
}

// ============================================================================
// 3. Substring Search & Range Query via Binary Search
// ============================================================================

/**
 * Checks whether pattern occurs as a substring in s via binary search on sa.
 * Time Complexity: O(m log n) where m = |pattern|. Space: O(1).
 */
bool contains_substring(const std::string& s, const std::vector<int>& sa, const std::string& pattern) {
    int n = static_cast<int>(s.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0) return true;
    if (n == 0 || n < m) return false;

    int low = 0, high = n - 1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int start = sa[mid];

        // Compare up to m characters without heap allocations
        int cmp = s.compare(start, std::min(m, n - start), pattern, 0, m);

        if (cmp == 0 && n - start >= m) {
            return true;
        } else if (cmp < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return false;
}

/**
 * Finds the range [L, R] of indices in the suffix array whose suffixes start with pattern.
 * If pattern does not appear, returns {-1, -1}.
 * Time Complexity: O(m log n). Space: O(1).
 */
std::pair<int, int> find_substring_range(
    const std::string& s, const std::vector<int>& sa, const std::string& pattern) {

    int n = static_cast<int>(s.size());
    int m = static_cast<int>(pattern.size());
    if (m == 0) return {0, n - 1};
    if (n == 0 || n < m) return {-1, -1};

    // Find lower bound in sa
    int low = 0, high = n - 1, first_pos = -1;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int start = sa[mid];
        int cmp = s.compare(start, std::min(m, n - start), pattern, 0, m);

        if (cmp >= 0) {
            if (cmp == 0 && n - start >= m) first_pos = mid;
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    if (first_pos == -1) return {-1, -1};

    // Find upper bound in sa
    low = first_pos;
    high = n - 1;
    int last_pos = first_pos;
    while (low <= high) {
        int mid = low + (high - low) / 2;
        int start = sa[mid];
        int cmp = s.compare(start, std::min(m, n - start), pattern, 0, m);

        if (cmp <= 0) {
            if (cmp == 0 && n - start >= m) last_pos = mid;
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return {first_pos, last_pos};
}

/**
 * Counts the number of times pattern appears in string s.
 * Time Complexity: O(m log n). Space: O(1).
 */
int count_occurrences(const std::string& s, const std::vector<int>& sa, const std::string& pattern) {
    auto [l, r] = find_substring_range(s, sa, pattern);
    if (l == -1) return 0;
    return r - l + 1;
}

// ============================================================================
// 4. Distinct Substrings & Longest Repeated Substring
// ============================================================================

/**
 * Counts total distinct substrings in string s.
 * Formula: sum_{i=0}^{n-1} (n - sa[i]) - sum_{i=0}^{n-1} lcp[i].
 * Time Complexity: O(n) after sa and lcp are built.
 */
long long count_distinct_substrings(const std::string& s, const std::vector<int>& sa, const std::vector<int>& lcp) {
    int n = static_cast<int>(s.size());
    if (n == 0) return 0;

    long long total = 0;
    for (int i = 0; i < n; ++i) {
        total += (n - sa[i]) - lcp[i];
    }

    return total;
}

/**
 * Extracts the longest repeated substring in s.
 * Discovers the maximum value in the LCP array.
 * Time Complexity: O(n) after sa and lcp are built.
 */
std::string longest_repeated_substring(const std::string& s, const std::vector<int>& sa, const std::vector<int>& lcp) {
    int n = static_cast<int>(s.size());
    if (n == 0) return "";

    int best_len = 0;
    int best_pos = 0;

    for (int i = 1; i < n; ++i) {
        if (lcp[i] > best_len) {
            best_len = lcp[i];
            best_pos = sa[i];
        }
    }

    return s.substr(best_pos, best_len);
}

} // namespace suffix_structures

// ============================================================================
// Unit Tests & Edge Case Verification
// ============================================================================

int main() {
    using namespace suffix_structures;

    std::cout << "Running Suffix Arrays and LCP C++17 unit tests...\n";

    // ------------------------------------------------------------------------
    // Test 1: Arthur's Running Example: "banana"
    // Suffixes:
    // 0: banana
    // 1: anana
    // 2: nana
    // 3: ana
    // 4: na
    // 5: a
    // Sorted:
    // 0: a (5)
    // 1: ana (3)
    // 2: anana (1)
    // 3: banana (0)
    // 4: na (4)
    // 5: nana (2)
    // Expected SA:  [5, 3, 1, 0, 4, 2]
    // Expected LCP: [0, 1, 3, 0, 0, 2]
    // Distinct substrings: (1-0) + (3-1) + (5-3) + (6-0) + (2-0) + (4-2) = 1+2+2+6+2+2 = 15
    // Longest repeated substring: "ana" (len 3)
    // ------------------------------------------------------------------------
    {
        std::string s = "banana";
        auto sa = build_suffix_array(s);
        auto sa_simple = build_suffix_array_simple(s);
        assert(sa == (std::vector<int>{5, 3, 1, 0, 4, 2}));
        assert(sa_simple == sa);

        auto lcp = build_lcp_array(s, sa);
        assert(lcp == (std::vector<int>{0, 1, 3, 0, 0, 2}));

        long long distinct = count_distinct_substrings(s, sa, lcp);
        assert(distinct == 15);

        std::string lrs = longest_repeated_substring(s, sa, lcp);
        assert(lrs == "ana");

        // Substring searches
        assert(contains_substring(s, sa, "ana") == true);
        assert(contains_substring(s, sa, "banana") == true);
        assert(contains_substring(s, sa, "a") == true);
        assert(contains_substring(s, sa, "nan") == true);
        assert(contains_substring(s, sa, "xyz") == false);
        assert(contains_substring(s, sa, "bananass") == false);

        assert(count_occurrences(s, sa, "a") == 3);
        assert(count_occurrences(s, sa, "ana") == 2);
        assert(count_occurrences(s, sa, "banana") == 1);
        assert(count_occurrences(s, sa, "nan") == 1);
        assert(count_occurrences(s, sa, "apple") == 0);
    }

    // ------------------------------------------------------------------------
    // Test 2: Identical Characters: "aaaa"
    // Sorted: "a" (3), "aa" (2), "aaa" (1), "aaaa" (0)
    // SA:  [3, 2, 1, 0]
    // LCP: [0, 1, 2, 3]
    // Distinct substrings: 4 ("a", "aa", "aaa", "aaaa")
    // Longest repeated: "aaa"
    // ------------------------------------------------------------------------
    {
        std::string s = "aaaa";
        auto sa = build_suffix_array(s);
        assert(sa == (std::vector<int>{3, 2, 1, 0}));

        auto lcp = build_lcp_array(s, sa);
        assert(lcp == (std::vector<int>{0, 1, 2, 3}));

        assert(count_distinct_substrings(s, sa, lcp) == 4);
        assert(longest_repeated_substring(s, sa, lcp) == "aaa");
        assert(count_occurrences(s, sa, "aa") == 3);
    }

    // ------------------------------------------------------------------------
    // Test 3: Distinct Characters: "abcdef"
    // SA:  [0, 1, 2, 3, 4, 5]
    // LCP: [0, 0, 0, 0, 0, 0]
    // Distinct substrings: 6 * 7 / 2 = 21
    // Longest repeated: ""
    // ------------------------------------------------------------------------
    {
        std::string s = "abcdef";
        auto sa = build_suffix_array(s);
        assert(sa == (std::vector<int>{0, 1, 2, 3, 4, 5}));

        auto lcp = build_lcp_array(s, sa);
        assert(lcp == (std::vector<int>{0, 0, 0, 0, 0, 0}));

        assert(count_distinct_substrings(s, sa, lcp) == 21);
        assert(longest_repeated_substring(s, sa, lcp) == "");
    }

    // ------------------------------------------------------------------------
    // Test 4: Single Character and Empty String
    // ------------------------------------------------------------------------
    {
        std::string s = "z";
        auto sa = build_suffix_array(s);
        assert(sa == (std::vector<int>{0}));
        auto lcp = build_lcp_array(s, sa);
        assert(lcp == (std::vector<int>{0}));
        assert(count_distinct_substrings(s, sa, lcp) == 1);
        assert(longest_repeated_substring(s, sa, lcp) == "");

        assert(build_suffix_array("").empty());
        assert(build_lcp_array("", {}).empty());
        assert(count_distinct_substrings("", {}, {}) == 0);
        assert(longest_repeated_substring("", {}, {}) == "");
    }

    // ------------------------------------------------------------------------
    // Test 5: Random String Cross-Validation Against Naive Suffix Sort
    // ------------------------------------------------------------------------
    {
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> char_dist(0, 3); // 'a' - 'd'

        for (int iter = 0; iter < 50; ++iter) {
            int n = 20 + (iter % 30);
            std::string s;
            for (int i = 0; i < n; ++i) s.push_back(static_cast<char>('a' + char_dist(rng)));

            auto sa = build_suffix_array(s);
            auto sa_simple = build_suffix_array_simple(s);
            assert(sa == sa_simple);

            // Naive reference
            std::vector<int> naive_sa(n);
            for (int i = 0; i < n; ++i) naive_sa[i] = i;
            std::sort(naive_sa.begin(), naive_sa.end(), [&](int a, int b) {
                return s.substr(a) < s.substr(b);
            });
            assert(sa == naive_sa);

            auto lcp = build_lcp_array(s, sa);

            // Verify LCP values against brute force
            for (int i = 1; i < n; ++i) {
                int h = 0;
                int p1 = sa[i - 1], p2 = sa[i];
                while (p1 + h < n && p2 + h < n && s[p1 + h] == s[p2 + h]) {
                    ++h;
                }
                assert(lcp[i] == h);
            }
        }
    }

    std::cout << "All Suffix Arrays and LCP C++17 unit tests passed successfully!\n";
    return 0;
}
