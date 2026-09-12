/**
 * Reference Implementation: Prefix Function and Knuth-Morris-Pratt (KMP)
 * Demonstrates:
 * 1. Linear-time prefix function (pi table / border array) computation - O(m)
 * 2. KMP exact string matching with non-backtracking text scan - O(n + m)
 * 3. Overlapping match reporting
 * 4. Smallest period detection (string compression factorization)
 * 5. All border lengths extraction via failure-link chain
 * 6. Prefix occurrence frequency counting across the entire string
 * 7. KMP Deterministic Finite Automaton (DFA) state-transition construction
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <random>

namespace kmp {

// ============================================================================
// 1. Prefix Function (Pi Table)
// ============================================================================

/**
 * Computes the prefix function (border array) for a string s.
 * pi[i] is the length of the longest proper prefix of s[0..i] that is also a suffix.
 * Time Complexity: O(m) amortized. Auxiliary Space: O(m).
 */
std::vector<int> compute_prefix_function(const std::string& s) {
    int n = static_cast<int>(s.size());
    std::vector<int> pi(n, 0);

    for (int i = 1; i < n; ++i) {
        int j = pi[i - 1];

        // Fallback chain through borders of borders
        while (j > 0 && s[i] != s[j]) {
            j = pi[j - 1];
        }

        if (s[i] == s[j]) {
            ++j;
        }

        pi[i] = j;
    }

    return pi;
}

// ============================================================================
// 2. KMP Exact String Matching
// ============================================================================

/**
 * Finds all 0-based starting indices where pattern occurs in text.
 * Convention: if pattern is empty, returns an empty vector.
 * Time Complexity: O(n + m). Auxiliary Space: O(m) for the pi array.
 */
std::vector<int> kmp_search(const std::string& text, const std::string& pattern) {
    std::vector<int> matches;
    if (pattern.empty() || text.empty() || text.size() < pattern.size()) {
        return matches;
    }

    std::vector<int> pi = compute_prefix_function(pattern);
    int n = static_cast<int>(text.size());
    int m = static_cast<int>(pattern.size());
    int j = 0;

    for (int i = 0; i < n; ++i) {
        while (j > 0 && text[i] != pattern[j]) {
            j = pi[j - 1];
        }

        if (text[i] == pattern[j]) {
            ++j;
        }

        if (j == m) {
            matches.push_back(i - m + 1);
            // Fallback to allow overlapping matches
            j = pi[j - 1];
        }
    }

    return matches;
}

// ============================================================================
// 3. String Periodicity & Borders
// ============================================================================

/**
 * Computes the length of the smallest period of string s.
 * If s can be represented as repeated copies of a prefix of length p, returns p.
 * Otherwise, returns s.size().
 * Time Complexity: O(m). Auxiliary Space: O(m).
 */
int smallest_period(const std::string& s) {
    if (s.empty()) return 0;
    std::vector<int> pi = compute_prefix_function(s);
    int n = static_cast<int>(s.size());
    int p = n - pi[n - 1];
    return (n % p == 0 ? p : n);
}

/**
 * Extracts all proper border lengths of string s in strictly decreasing order.
 * Follows the chain: pi[n-1], pi[pi[n-1]-1], ... down to > 0.
 * Time Complexity: O(m). Auxiliary Space: O(m).
 */
std::vector<int> all_borders(const std::string& s) {
    if (s.empty()) return {};
    std::vector<int> pi = compute_prefix_function(s);
    int n = static_cast<int>(s.size());
    std::vector<int> borders;

    int j = pi[n - 1];
    while (j > 0) {
        borders.push_back(j);
        j = pi[j - 1];
    }

    return borders;
}

// ============================================================================
// 4. Counting Occurrences of Each Prefix
// ============================================================================

/**
 * Counts how many times each prefix s[0..i] appears as a substring in s.
 * Returns vector ans of size s.size(), where ans[i] is the count for prefix s[0..i].
 * Time Complexity: O(m). Auxiliary Space: O(m).
 */
std::vector<int> count_prefix_occurrences(const std::string& s) {
    int n = static_cast<int>(s.size());
    if (n == 0) return {};

    std::vector<int> pi = compute_prefix_function(s);
    std::vector<int> count(n + 1, 0);

    // Count how often each border length appears
    for (int i = 0; i < n; ++i) {
        count[pi[i]]++;
    }

    // Propagate counts downward through the failure link tree
    for (int i = n - 1; i > 0; --i) {
        count[pi[i - 1]] += count[i];
    }

    // Each prefix s[0..i] of length i+1 appears at least once as itself
    for (int i = 1; i <= n; ++i) {
        count[i]++;
    }

    std::vector<int> ans(n, 0);
    for (int i = 0; i < n; ++i) {
        ans[i] = count[i + 1];
    }

    return ans;
}

// ============================================================================
// 5. KMP Finite Automaton
// ============================================================================

/**
 * Builds the KMP Deterministic Finite Automaton (DFA).
 * Transitions: aut[state][char - base_char] -> next_state.
 * Time Complexity: O(m * alphabet_size). Space: O(m * alphabet_size).
 */
std::vector<std::vector<int>> build_kmp_automaton(
    const std::string& pattern, int alphabet_size = 26, char base_char = 'a') {

    int m = static_cast<int>(pattern.size());
    std::vector<int> pi = compute_prefix_function(pattern);
    std::vector<std::vector<int>> aut(m + 1, std::vector<int>(alphabet_size, 0));

    for (int i = 0; i <= m; ++i) {
        for (int c = 0; c < alphabet_size; ++c) {
            char ch = static_cast<char>(base_char + c);
            if (i > 0 && (i == m || ch != pattern[i])) {
                aut[i][c] = aut[pi[i - 1]][c];
            } else {
                aut[i][c] = i + ((i < m && ch == pattern[i]) ? 1 : 0);
            }
        }
    }

    return aut;
}

} // namespace kmp

// ============================================================================
// Unit Tests & Edge Case Verification
// ============================================================================

int main() {
    using namespace kmp;

    std::cout << "Running Prefix Function and KMP C++17 unit tests...\n";

    // ------------------------------------------------------------------------
    // Test 1: Arthur's Worked Example 1: "ababac"
    // Expected pi: [0, 0, 1, 2, 3, 0]
    // ------------------------------------------------------------------------
    {
        std::string s = "ababac";
        std::vector<int> expected = {0, 0, 1, 2, 3, 0};
        assert(compute_prefix_function(s) == expected);
    }

    // ------------------------------------------------------------------------
    // Test 2: Arthur's Worked Example 2: "aabaaab"
    // Positions:
    // a -> 0
    // aa -> 1
    // aab -> 0
    // aaba -> 1
    // aabaa -> 2
    // aabaaa -> 2
    // aabaaab -> 3 ("aab")
    // Expected pi: [0, 1, 0, 1, 2, 2, 3]
    // ------------------------------------------------------------------------
    {
        std::string s = "aabaaab";
        std::vector<int> expected = {0, 1, 0, 1, 2, 2, 3};
        assert(compute_prefix_function(s) == expected);
    }

    // ------------------------------------------------------------------------
    // Test 3: KMP Search - Standard & Overlapping Matches
    // text: "ababcababa", pattern: "ababa" -> matches at index 5
    // text: "aaaaa", pattern: "aaa" -> matches at indices 0, 1, 2
    // ------------------------------------------------------------------------
    {
        std::string text1 = "ababcababa";
        std::string pat1 = "ababa";
        auto matches1 = kmp_search(text1, pat1);
        assert(matches1 == (std::vector<int>{5}));

        std::string text2 = "aaaaa";
        std::string pat2 = "aaa";
        auto matches2 = kmp_search(text2, pat2);
        assert(matches2 == (std::vector<int>{0, 1, 2}));
    }

    // ------------------------------------------------------------------------
    // Test 4: Boundary Cases (Empty strings, no match, pattern longer than text)
    // ------------------------------------------------------------------------
    {
        assert(compute_prefix_function("").empty());
        assert(kmp_search("", "abc").empty());
        assert(kmp_search("abc", "").empty());
        assert(kmp_search("abc", "abcdef").empty());
        assert(kmp_search("abcdef", "xyz").empty());

        // Single character
        assert(compute_prefix_function("a") == (std::vector<int>{0}));
        assert(kmp_search("banana", "a") == (std::vector<int>{1, 3, 5}));
    }

    // ------------------------------------------------------------------------
    // Test 5: Periodicity & Borders
    // "abababab" -> period 2 ("ab")
    // "abcabcabc" -> period 3 ("abc")
    // "abcdef" -> period 6
    // "ababa" borders: len 3 ("aba"), len 1 ("a")
    // ------------------------------------------------------------------------
    {
        assert(smallest_period("abababab") == 2);
        assert(smallest_period("abcabcabc") == 3);
        assert(smallest_period("abcdef") == 6);
        assert(smallest_period("") == 0);

        auto borders = all_borders("ababa");
        assert(borders == (std::vector<int>{3, 1}));
        assert(all_borders("abcdef").empty());
    }

    // ------------------------------------------------------------------------
    // Test 6: Counting Prefix Occurrences
    // In "ababa":
    // prefix len 1 ("a"): occurs at indices 0, 2, 4 -> count = 3
    // prefix len 2 ("ab"): occurs at indices 0, 2 -> count = 2
    // prefix len 3 ("aba"): occurs at indices 0, 2 -> count = 2
    // prefix len 4 ("abab"): occurs at index 0 -> count = 1
    // prefix len 5 ("ababa"): occurs at index 0 -> count = 1
    // ------------------------------------------------------------------------
    {
        std::string s = "ababa";
        auto occ = count_prefix_occurrences(s);
        assert(occ[0] == 3); // "a"
        assert(occ[1] == 2); // "ab"
        assert(occ[2] == 2); // "aba"
        assert(occ[3] == 1); // "abab"
        assert(occ[4] == 1); // "ababa"
    }

    // ------------------------------------------------------------------------
    // Test 7: Automaton Search Equivalence
    // ------------------------------------------------------------------------
    {
        std::string pattern = "ababa";
        std::string text = "ababcabababacababa";
        auto aut = build_kmp_automaton(pattern);

        std::vector<int> aut_matches;
        int state = 0;
        int m = static_cast<int>(pattern.size());

        for (int i = 0; i < static_cast<int>(text.size()); ++i) {
            state = aut[state][text[i] - 'a'];
            if (state == m) {
                aut_matches.push_back(i - m + 1);
            }
        }

        auto direct_matches = kmp_search(text, pattern);
        assert(aut_matches == direct_matches);
    }

    // ------------------------------------------------------------------------
    // Test 8: Random Cross-Validation Against std::string::find
    // ------------------------------------------------------------------------
    {
        std::mt19937 rng(42);
        std::uniform_int_distribution<int> char_dist(0, 3); // small alphabet 'a'-'d' for frequent matches

        for (int iter = 0; iter < 100; ++iter) {
            int n = 50 + (iter % 30);
            int m = 3 + (iter % 6);

            std::string text, pattern;
            for (int i = 0; i < n; ++i) text.push_back(static_cast<char>('a' + char_dist(rng)));
            for (int i = 0; i < m; ++i) pattern.push_back(static_cast<char>('a' + char_dist(rng)));

            auto kmp_res = kmp_search(text, pattern);

            std::vector<int> brute_res;
            size_t pos = text.find(pattern, 0);
            while (pos != std::string::npos) {
                brute_res.push_back(static_cast<int>(pos));
                pos = text.find(pattern, pos + 1); // allow overlapping
            }

            assert(kmp_res == brute_res);
        }
    }

    std::cout << "All Prefix Function and KMP C++17 unit tests passed successfully!\n";
    return 0;
}
