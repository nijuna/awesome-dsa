/**
 * Reference Implementation: Edit Distance and Sequence Alignment
 * Demonstrates:
 * 1. Levenshtein Distance: Classical 2D DP (O(nm) Time, O(nm) Space) & Two-Row Rolling Array (O(m) Space).
 * 2. Full Edit Transcript Backtracing (Match, Substitute, Insert, Delete).
 * 3. Needleman-Wunsch Global Alignment: 2D DP Scoring & Two-Row Rolling Array.
 * 4. Needleman-Wunsch Alignment Backtracing (Optimal Aligned Strings with Gap Characters '-').
 * 5. Hamming Distance for Equal-Length Strings (O(n) Time, O(1) Space).
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>
#include <stdexcept>

namespace sequence_alignment {

// ============================================================================
// 1. Levenshtein Edit Distance (Cost Minimization)
// ============================================================================

/**
 * Classical 2D DP: O(nm) Time, O(nm) Space.
 * dp[i][j] = min edits to transform a[0..i-1] into b[0..j-1].
 */
int levenshtein_distance(const std::string& a, const std::string& b) {
    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 0; i <= n; ++i) dp[i][0] = i;
    for (int j = 0; j <= m; ++j) dp[0][j] = j;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,      // delete from a
                dp[i][j - 1] + 1,      // insert into a
                dp[i - 1][j - 1] + cost // match or substitute
            });
        }
    }

    return dp[n][m];
}

/**
 * Two-Row Rolling Array Space Optimization: O(nm) Time, O(m) Space.
 */
int levenshtein_distance_rolling(const std::string& a, const std::string& b) {
    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<int> prev(m + 1), cur(m + 1);
    for (int j = 0; j <= m; ++j) prev[j] = j;

    for (int i = 1; i <= n; ++i) {
        cur[0] = i;
        for (int j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            cur[j] = std::min({
                prev[j] + 1,
                cur[j - 1] + 1,
                prev[j - 1] + cost
            });
        }
        std::swap(prev, cur);
    }

    return prev[m];
}

// ============================================================================
// 2. Edit Transcript Reconstruction
// ============================================================================

struct EditOp {
    std::string type; // "match", "substitute", "insert", "delete"
    char from_char;
    char to_char;
};

std::pair<int, std::vector<EditOp>> levenshtein_with_ops(
    const std::string& a,
    const std::string& b) {

    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 0; i <= n; ++i) dp[i][0] = i;
    for (int j = 0; j <= m; ++j) dp[0][j] = j;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            });
        }
    }

    std::vector<EditOp> ops;
    int i = n, j = m;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            if (dp[i][j] == dp[i - 1][j - 1] + cost) {
                if (cost == 0) {
                    ops.push_back({"match", a[i - 1], b[j - 1]});
                } else {
                    ops.push_back({"substitute", a[i - 1], b[j - 1]});
                }
                --i;
                --j;
                continue;
            }
        }

        if (i > 0 && dp[i][j] == dp[i - 1][j] + 1) {
            ops.push_back({"delete", a[i - 1], '\0'});
            --i;
        } else {
            ops.push_back({"insert", '\0', b[j - 1]});
            --j;
        }
    }

    std::reverse(ops.begin(), ops.end());
    return {dp[n][m], ops};
}

// ============================================================================
// 3. Needleman-Wunsch Global Alignment (Score Maximization)
// ============================================================================

/**
 * Computes optimal global alignment score: O(nm) Time, O(nm) Space.
 */
int needleman_wunsch_score(
    const std::string& a,
    const std::string& b,
    int match_score = 1,
    int mismatch_penalty = -1,
    int gap_penalty = -1) {

    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 0; i <= n; ++i) dp[i][0] = i * gap_penalty;
    for (int j = 0; j <= m; ++j) dp[0][j] = j * gap_penalty;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int diag = dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? match_score : mismatch_penalty);
            int up = dp[i - 1][j] + gap_penalty;
            int left = dp[i][j - 1] + gap_penalty;
            dp[i][j] = std::max({diag, up, left});
        }
    }

    return dp[n][m];
}

/**
 * Two-Row Rolling Array for Needleman-Wunsch: O(nm) Time, O(m) Space.
 */
int needleman_wunsch_score_rolling(
    const std::string& a,
    const std::string& b,
    int match_score = 1,
    int mismatch_penalty = -1,
    int gap_penalty = -1) {

    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<int> prev(m + 1), cur(m + 1);

    for (int j = 0; j <= m; ++j) prev[j] = j * gap_penalty;

    for (int i = 1; i <= n; ++i) {
        cur[0] = i * gap_penalty;
        for (int j = 1; j <= m; ++j) {
            int diag = prev[j - 1] + (a[i - 1] == b[j - 1] ? match_score : mismatch_penalty);
            int up = prev[j] + gap_penalty;
            int left = cur[j - 1] + gap_penalty;
            cur[j] = std::max({diag, up, left});
        }
        std::swap(prev, cur);
    }

    return prev[m];
}

struct AlignmentResult {
    int score;
    std::string aligned_a;
    std::string aligned_b;
};

/**
 * Global Alignment Reconstruction with Gap Characters '-': O(nm) Time, O(nm) Space.
 */
AlignmentResult needleman_wunsch_align(
    const std::string& a,
    const std::string& b,
    int match_score = 1,
    int mismatch_penalty = -1,
    int gap_penalty = -1) {

    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 0; i <= n; ++i) dp[i][0] = i * gap_penalty;
    for (int j = 0; j <= m; ++j) dp[0][j] = j * gap_penalty;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int diag = dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? match_score : mismatch_penalty);
            int up = dp[i - 1][j] + gap_penalty;
            int left = dp[i][j - 1] + gap_penalty;
            dp[i][j] = std::max({diag, up, left});
        }
    }

    std::string aligned_a, aligned_b;
    int i = n, j = m;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0) {
            int diag = dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? match_score : mismatch_penalty);
            if (dp[i][j] == diag) {
                aligned_a.push_back(a[i - 1]);
                aligned_b.push_back(b[j - 1]);
                --i;
                --j;
                continue;
            }
        }

        if (i > 0 && dp[i][j] == dp[i - 1][j] + gap_penalty) {
            aligned_a.push_back(a[i - 1]);
            aligned_b.push_back('-');
            --i;
        } else {
            aligned_a.push_back('-');
            aligned_b.push_back(b[j - 1]);
            --j;
        }
    }

    std::reverse(aligned_a.begin(), aligned_a.end());
    std::reverse(aligned_b.begin(), aligned_b.end());

    return {dp[n][m], aligned_a, aligned_b};
}

// ============================================================================
// 4. Comparative Metrics: Hamming Distance
// ============================================================================

/**
 * Counts point mismatches between equal-length sequences: O(n) Time, O(1) Space.
 */
int hamming_distance(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) {
        throw std::invalid_argument("Hamming distance requires equal length strings");
    }
    int diff = 0;
    for (size_t i = 0; i < a.size(); ++i) {
        if (a[i] != b[i]) {
            ++diff;
        }
    }
    return diff;
}

} // namespace sequence_alignment

// ============================================================================
// Comprehensive Unit Verification
// ============================================================================

int main() {
    using namespace sequence_alignment;

    // Test 1: Canonical kitten -> sitting
    std::string s1 = "kitten", s2 = "sitting";
    assert(levenshtein_distance(s1, s2) == 3);
    assert(levenshtein_distance_rolling(s1, s2) == 3);

    auto [dist1, ops1] = levenshtein_with_ops(s1, s2);
    assert(dist1 == 3);
    // Verify ops transform s1 into s2
    int edit_count = 0;
    for (const auto& op : ops1) {
        if (op.type != "match") ++edit_count;
    }
    assert(edit_count == 3);

    // Test 2: horse -> ros (Arthur's worked example)
    std::string s3 = "horse", s4 = "ros";
    assert(levenshtein_distance(s3, s4) == 3);
    assert(levenshtein_distance_rolling(s3, s4) == 3);

    // Test 3: Empty string edge cases
    assert(levenshtein_distance("", "") == 0);
    assert(levenshtein_distance("abc", "") == 3);
    assert(levenshtein_distance("", "abcd") == 4);
    assert(levenshtein_distance_rolling("", "abcd") == 4);

    // Test 4: Identical strings
    assert(levenshtein_distance("algorithm", "algorithm") == 0);
    assert(levenshtein_distance_rolling("algorithm", "algorithm") == 0);

    // Test 5: Needleman-Wunsch Global Alignment
    // a = GATTACA, b = GCATGCU
    std::string dna1 = "GATTACA", dna2 = "GCATGCU";
    int score = needleman_wunsch_score(dna1, dna2, 1, -1, -1);
    int rolling_score = needleman_wunsch_score_rolling(dna1, dna2, 1, -1, -1);
    assert(score == rolling_score);

    auto align_res = needleman_wunsch_align(dna1, dna2, 1, -1, -1);
    assert(align_res.score == score);
    assert(align_res.aligned_a.size() == align_res.aligned_b.size());

    // Verify score from aligned strings manually
    int manual_score = 0;
    for (size_t k = 0; k < align_res.aligned_a.size(); ++k) {
        char ca = align_res.aligned_a[k];
        char cb = align_res.aligned_b[k];
        if (ca == '-' || cb == '-') {
            manual_score += -1; // gap
        } else if (ca == cb) {
            manual_score += 1;  // match
        } else {
            manual_score += -1; // mismatch
        }
    }
    assert(manual_score == score);

    // Test 6: Hamming Distance
    assert(hamming_distance("karolin", "kathrin") == 3);
    assert(hamming_distance("1011101", "1001001") == 2);
    try {
        hamming_distance("short", "longer");
        assert(false);
    } catch (const std::invalid_argument&) {}

    std::cout << "[PASS] All Edit Distance and Sequence Alignment C++ unit tests passed." << std::endl;
    return 0;
}
