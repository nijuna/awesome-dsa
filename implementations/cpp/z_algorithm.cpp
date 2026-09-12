#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace dsa {

/**
 * @brief Computes the Z-function for a given string.
 *
 * For a string s of length n, z[i] is the length of the longest substring
 * starting at index i that matches a prefix of s.
 * By standard convention, z[0] = 0.
 *
 * Time Complexity: O(n) amortized time, where n = s.size().
 * Space Complexity: O(n) to store the result vector.
 *
 * @param s Input string.
 * @return std::vector<int> Array of Z-values of length n.
 */
inline std::vector<int> z_function(const std::string& s) {
    int n = static_cast<int>(s.size());
    std::vector<int> z(n, 0);
    if (n == 0) return z;

    int l = 0;
    int r = 0;
    for (int i = 1; i < n; ++i) {
        if (i <= r) {
            z[i] = std::min(r - i + 1, z[i - l]);
        }

        while (i + z[i] < n && s[z[i]] == s[i + z[i]]) {
            ++z[i];
        }

        if (i + z[i] - 1 > r) {
            l = i;
            r = i + z[i] - 1;
        }
    }

    return z;
}

/**
 * @brief Searches for all occurrences of pattern inside text using the Z-algorithm.
 *
 * Constructs the combined string: pattern + separator + text.
 * Any position i in the text portion where z[i] >= pattern.size() indicates
 * an exact match starting at text index (i - pattern.size() - 1).
 *
 * Time Complexity: O(n + m) where n = text.size(), m = pattern.size().
 * Space Complexity: O(n + m) for combined string and Z-array.
 *
 * @param text The target string to search in.
 * @param pattern The pattern string to locate.
 * @param separator A delimiter not present in either text or pattern (default '#').
 * @return std::vector<int> 0-based starting indices of pattern occurrences in text.
 */
inline std::vector<int> z_search(const std::string& text,
                                 const std::string& pattern,
                                 char separator = '#') {
    std::vector<int> matches;
    if (pattern.empty()) return matches;
    int m = static_cast<int>(pattern.size());
    int n = static_cast<int>(text.size());
    if (m > n) return matches;

    std::string combined;
    combined.reserve(pattern.size() + 1 + text.size());
    combined.append(pattern);
    combined.push_back(separator);
    combined.append(text);

    std::vector<int> z = z_function(combined);
    int total_len = static_cast<int>(combined.size());

    for (int i = m + 1; i < total_len; ++i) {
        if (z[i] >= m) {
            matches.push_back(i - m - 1);
        }
    }

    return matches;
}

/**
 * @brief Finds all proper border lengths of a string s.
 *
 * A border of a string is a substring that is both a proper prefix and proper suffix.
 * Using the Z-function, a suffix starting at position i is a border if i + z[i] == n.
 *
 * Time Complexity: O(n).
 * Space Complexity: O(n).
 *
 * @param s Input string.
 * @return std::vector<int> Sorted list of valid border lengths.
 */
inline std::vector<int> border_lengths(const std::string& s) {
    int n = static_cast<int>(s.size());
    if (n <= 1) return {};

    std::vector<int> z = z_function(s);
    std::vector<int> borders;

    for (int i = 1; i < n; ++i) {
        if (i + z[i] == n) {
            borders.push_back(z[i]);
        }
    }

    std::sort(borders.begin(), borders.end());
    return borders;
}

/**
 * @brief Finds the smallest exact repetition period of string s.
 *
 * A string has period p if characters repeat every p positions and n % p == 0.
 * In terms of Z-values, candidate p is a period iff p + z[p] == n.
 *
 * Time Complexity: O(n).
 * Space Complexity: O(n).
 *
 * @param s Input string.
 * @return int Length of smallest period (returns n if no smaller period exists, 0 if empty).
 */
inline int smallest_period(const std::string& s) {
    int n = static_cast<int>(s.size());
    if (n == 0) return 0;

    std::vector<int> z = z_function(s);
    for (int p = 1; p < n; ++p) {
        if (p + z[p] == n && n % p == 0) {
            return p;
        }
    }
    return n;
}

/**
 * @brief Counts the total occurrences of each prefix of length 1..n inside s.
 *
 * If z[i] = L, then all prefixes of lengths 1..L appear starting at index i.
 * Prefix of length n trivially appears once (at index 0).
 *
 * Time Complexity: O(n).
 * Space Complexity: O(n).
 *
 * @param s Input string.
 * @return std::vector<int> Vector of size n + 1 where ans[len] is the count of prefix s[0..len-1].
 */
inline std::vector<int> count_prefix_occurrences(const std::string& s) {
    int n = static_cast<int>(s.size());
    if (n == 0) return {0};

    std::vector<int> z = z_function(s);
    std::vector<int> count(n + 1, 0);

    for (int i = 1; i < n; ++i) {
        if (z[i] > 0) {
            count[z[i]]++;
        }
    }

    // Accumulate suffix sums: a match of length L contributes to all prefix lengths <= L
    for (int len = n - 1; len >= 1; --len) {
        count[len] += count[len + 1];
    }

    // Every prefix of length 1..n appears at least once at index 0
    for (int len = 1; len <= n; ++len) {
        count[len] += 1;
    }

    return count;
}

} // namespace dsa

// ============================================================================
// Unit Tests
// ============================================================================

void test_empty_and_single() {
    auto z_empty = dsa::z_function("");
    assert(z_empty.empty());
    assert(dsa::smallest_period("") == 0);

    auto z_single = dsa::z_function("a");
    assert(z_single.size() == 1);
    assert(z_single[0] == 0);
    assert(dsa::border_lengths("a").empty());
    assert(dsa::smallest_period("a") == 1);
}

void test_canonical_ababa() {
    std::string s = "ababa";
    auto z = dsa::z_function(s);
    std::vector<int> expected = {0, 0, 3, 0, 1};
    assert(z == expected);

    auto borders = dsa::border_lengths(s);
    std::vector<int> expected_borders = {1, 3};
    assert(borders == expected_borders);

    assert(dsa::smallest_period(s) == 5);

    auto prefix_counts = dsa::count_prefix_occurrences(s);
    assert(prefix_counts[1] == 3); // "a" appears at 0, 2, 4
    assert(prefix_counts[2] == 2); // "ab" appears at 0, 2
    assert(prefix_counts[3] == 2); // "aba" appears at 0, 2
    assert(prefix_counts[4] == 1); // "abab" appears at 0
    assert(prefix_counts[5] == 1); // "ababa" appears at 0
}

void test_all_identical_chars() {
    std::string s = "aaaaa";
    auto z = dsa::z_function(s);
    std::vector<int> expected = {0, 4, 3, 2, 1};
    assert(z == expected);

    auto borders = dsa::border_lengths(s);
    std::vector<int> expected_borders = {1, 2, 3, 4};
    assert(borders == expected_borders);

    assert(dsa::smallest_period(s) == 1);

    auto prefix_counts = dsa::count_prefix_occurrences(s);
    assert(prefix_counts[1] == 5);
    assert(prefix_counts[2] == 4);
    assert(prefix_counts[3] == 3);
    assert(prefix_counts[4] == 2);
    assert(prefix_counts[5] == 1);
}

void test_pattern_matching() {
    std::string text = "abacaba";
    std::string pattern = "aba";
    auto matches = dsa::z_search(text, pattern);
    std::vector<int> expected = {0, 4};
    assert(matches == expected);

    // Overlapping occurrences
    text = "aaaaa";
    pattern = "aa";
    matches = dsa::z_search(text, pattern);
    std::vector<int> expected_overlap = {0, 1, 2, 3};
    assert(matches == expected_overlap);

    // Pattern not found
    matches = dsa::z_search("abcdef", "xyz");
    assert(matches.empty());

    // Pattern longer than text
    matches = dsa::z_search("abc", "abcdef");
    assert(matches.empty());

    // Empty pattern
    matches = dsa::z_search("abc", "");
    assert(matches.empty());
}

void test_periods() {
    assert(dsa::smallest_period("abcabcabc") == 3);
    assert(dsa::smallest_period("abababab") == 2);
    assert(dsa::smallest_period("abcdef") == 6);
    assert(dsa::smallest_period("abcab") == 5);
}

void test_z_invariants() {
    std::string s = "aabcaabxaabcaab";
    auto z = dsa::z_function(s);
    int n = static_cast<int>(s.size());

    // Check invariant definition for every position i
    for (int i = 1; i < n; ++i) {
        int len = z[i];
        if (len > 0) {
            assert(s.substr(0, len) == s.substr(i, len));
        }
        if (i + len < n) {
            assert(s[len] != s[i + len]);
        }
    }
}

int main() {
    std::cout << "Running Z-Algorithm C++17 unit tests...\n";
    test_empty_and_single();
    test_canonical_ababa();
    test_all_identical_chars();
    test_pattern_matching();
    test_periods();
    test_z_invariants();
    std::cout << "All Z-Algorithm C++17 unit tests passed successfully!\n";
    return 0;
}
