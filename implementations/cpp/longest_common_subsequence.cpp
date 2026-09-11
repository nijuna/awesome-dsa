/**
 * Reference Implementation: Longest Common Subsequence (LCS)
 * Demonstrates:
 * 1. Classical 2D Dynamic Programming for LCS Length: O(nm) Time, O(nm) Space.
 * 2. Backtracking Reconstruction of an Optimal Subsequence: O(nm) Time.
 * 3. Space-Optimized Two-Row Rolling Array for LCS Length: O(nm) Time, O(min(n, m)) Space.
 * 4. Shortest Common Supersequence (SCS) Length and String Reconstruction: O(nm) Time.
 * 5. Minimum Insertion-Deletion Distance (Unit-Cost Edit Distance without Substitutions): O(nm) Time.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>

namespace lcs {

// ============================================================================
// 1. Classical 2D Dynamic Programming (Length)
// ============================================================================

/**
 * Classical 2D DP table computation: O(nm) Time, O(nm) Space.
 * dp[i][j] = length of LCS between prefix x[0..i-1] and prefix y[0..j-1].
 */
std::vector<std::vector<int>> lcs_table(const std::string& x, const std::string& y) {
    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (x[i - 1] == y[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    return dp;
}

/**
 * Returns the length of the Longest Common Subsequence: O(nm) Time, O(nm) Space.
 */
int lcs_length(const std::string& x, const std::string& y) {
    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());
    auto dp = lcs_table(x, y);
    return dp[n][m];
}

// ============================================================================
// 2. Subsequence Reconstruction
// ============================================================================

/**
 * Reconstructs one optimal Longest Common Subsequence via backtracking.
 * Time: O(nm) to build table + O(n + m) to backtrack.
 * Space: O(nm) auxiliary space.
 */
std::string lcs_sequence(const std::string& x, const std::string& y) {
    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());
    auto dp = lcs_table(x, y);

    std::string result;
    int i = n;
    int j = m;

    while (i > 0 && j > 0) {
        if (x[i - 1] == y[j - 1]) {
            result.push_back(x[i - 1]);
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            // Deterministic tie-breaking: prefer moving up
            --i;
        } else {
            --j;
        }
    }

    std::reverse(result.begin(), result.end());
    return result;
}

// ============================================================================
// 3. Space-Optimized Rolling Array (Length Only)
// ============================================================================

/**
 * Two-Row Rolling Array: O(nm) Time, O(min(n, m)) Space.
 * Computes LCS length using only two rows of memory.
 */
int lcs_length_rolling(const std::string& x, const std::string& y) {
    // Ensure y is the shorter string to minimize auxiliary vector memory
    if (x.size() < y.size()) {
        return lcs_length_rolling(y, x);
    }

    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());

    std::vector<int> prev(m + 1, 0);
    std::vector<int> cur(m + 1, 0);

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (x[i - 1] == y[j - 1]) {
                cur[j] = prev[j - 1] + 1;
            } else {
                cur[j] = std::max(prev[j], cur[j - 1]);
            }
        }
        std::swap(prev, cur);
        std::fill(cur.begin(), cur.end(), 0);
    }

    return prev[m];
}

// ============================================================================
// 4. Shortest Common Supersequence (SCS)
// ============================================================================

/**
 * Shortest Common Supersequence length: n + m - LCS(x, y).
 */
int shortest_common_supersequence_length(const std::string& x, const std::string& y) {
    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());
    return n + m - lcs_length(x, y);
}

/**
 * Reconstructs an optimal Shortest Common Supersequence using the LCS table.
 * Time: O(nm) Time, O(nm) Space.
 */
std::string shortest_common_supersequence(const std::string& x, const std::string& y) {
    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());
    auto dp = lcs_table(x, y);

    std::string result;
    int i = n;
    int j = m;

    while (i > 0 && j > 0) {
        if (x[i - 1] == y[j - 1]) {
            result.push_back(x[i - 1]);
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            result.push_back(x[i - 1]);
            --i;
        } else {
            result.push_back(y[j - 1]);
            --j;
        }
    }

    // Append remaining characters from x or y
    while (i > 0) {
        result.push_back(x[i - 1]);
        --i;
    }
    while (j > 0) {
        result.push_back(y[j - 1]);
        --j;
    }

    std::reverse(result.begin(), result.end());
    return result;
}

// ============================================================================
// 5. Minimum Insertion-Deletion Distance
// ============================================================================

/**
 * Minimum number of character insertions and deletions to transform x into y:
 * Distance = (n - LCS) + (m - LCS) = n + m - 2 * LCS.
 */
int min_insert_delete_distance(const std::string& x, const std::string& y) {
    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());
    return n + m - 2 * lcs_length(x, y);
}

} // namespace lcs

// ============================================================================
// Helper validation routines & Unit Tests
// ============================================================================

static bool is_subsequence(const std::string& sub, const std::string& full) {
    size_t i = 0;
    for (char c : full) {
        if (i < sub.size() && sub[i] == c) {
            ++i;
        }
    }
    return i == sub.size();
}

int main() {
    using namespace lcs;

    // Test 1: Classical CLRS Example
    // X = "ABCBDAB", Y = "BDCABA"
    {
        std::string x = "ABCBDAB";
        std::string y = "BDCABA";

        int len = lcs_length(x, y);
        assert(len == 4);

        int rolling_len = lcs_length_rolling(x, y);
        assert(rolling_len == 4);

        std::string seq = lcs_sequence(x, y);
        assert(static_cast<int>(seq.size()) == 4);
        assert(is_subsequence(seq, x));
        assert(is_subsequence(seq, y));

        // Verify SCS
        int scs_len = shortest_common_supersequence_length(x, y);
        assert(scs_len == 7 + 6 - 4); // 9
        std::string scs_str = shortest_common_supersequence(x, y);
        assert(static_cast<int>(scs_str.size()) == 9);
        assert(is_subsequence(x, scs_str));
        assert(is_subsequence(y, scs_str));

        // Verify insertion-deletion distance
        int id_dist = min_insert_delete_distance(x, y);
        assert(id_dist == (7 - 4) + (6 - 4)); // 5
    }

    // Test 2: Tie-Breaking Example from Arthur
    // X = "ABCD", Y = "ACBD"
    // Optimal LCS can be "ABD" or "ACD" (length 3).
    {
        std::string x = "ABCD";
        std::string y = "ACBD";

        int len = lcs_length(x, y);
        assert(len == 3);
        assert(lcs_length_rolling(x, y) == 3);

        std::string seq = lcs_sequence(x, y);
        assert(seq.size() == 3);
        assert(is_subsequence(seq, x));
        assert(is_subsequence(seq, y));
        // With deterministic tie-breaking (dp[i-1][j] >= dp[i][j-1]), it selects "ABD"
        assert(seq == "ABD" || seq == "ACD");

        std::string scs_str = shortest_common_supersequence(x, y);
        assert(scs_str.size() == 5);
        assert(is_subsequence(x, scs_str));
        assert(is_subsequence(y, scs_str));
    }

    // Test 3: Empty string edge cases
    {
        assert(lcs_length("", "") == 0);
        assert(lcs_length_rolling("", "") == 0);
        assert(lcs_sequence("", "") == "");
        assert(shortest_common_supersequence_length("", "") == 0);
        assert(shortest_common_supersequence("", "") == "");
        assert(min_insert_delete_distance("", "") == 0);

        assert(lcs_length("HELLO", "") == 0);
        assert(lcs_length_rolling("HELLO", "") == 0);
        assert(lcs_sequence("HELLO", "") == "");
        assert(shortest_common_supersequence_length("HELLO", "") == 5);
        assert(shortest_common_supersequence("HELLO", "") == "HELLO");
        assert(min_insert_delete_distance("HELLO", "") == 5);

        assert(lcs_length("", "WORLD") == 0);
        assert(lcs_length_rolling("", "WORLD") == 0);
        assert(lcs_sequence("", "WORLD") == "");
        assert(shortest_common_supersequence_length("", "WORLD") == 5);
        assert(shortest_common_supersequence("", "WORLD") == "WORLD");
        assert(min_insert_delete_distance("", "WORLD") == 5);
    }

    // Test 4: Completely identical strings
    {
        std::string s = "ALGORITHM";
        assert(lcs_length(s, s) == 9);
        assert(lcs_length_rolling(s, s) == 9);
        assert(lcs_sequence(s, s) == "ALGORITHM");
        assert(shortest_common_supersequence_length(s, s) == 9);
        assert(shortest_common_supersequence(s, s) == "ALGORITHM");
        assert(min_insert_delete_distance(s, s) == 0);
    }

    // Test 5: Completely disjoint strings
    {
        std::string x = "ABC";
        std::string y = "DEF";
        assert(lcs_length(x, y) == 0);
        assert(lcs_length_rolling(x, y) == 0);
        assert(lcs_sequence(x, y) == "");
        assert(shortest_common_supersequence_length(x, y) == 6);
        assert(shortest_common_supersequence(x, y).size() == 6);
        assert(min_insert_delete_distance(x, y) == 6);
    }

    // Test 6: Single-element matching and mismatching
    {
        assert(lcs_length("A", "A") == 1);
        assert(lcs_sequence("A", "A") == "A");
        assert(lcs_length("A", "B") == 0);
        assert(lcs_sequence("A", "B") == "");
    }

    std::cout << "[PASS] All Longest Common Subsequence C++ unit tests passed." << std::endl;
    return 0;
}
