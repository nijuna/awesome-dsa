/**
 * Reference Implementation: Interval and Matrix Dynamic Programming
 * Demonstrates:
 * 1. Matrix Chain Multiplication (MCM): Cost computation O(n^3) and parenthesization reconstruction.
 * 2. Optimal Binary Search Tree (OBST): Classical O(n^3) and Knuth-optimized O(n^2) search cost.
 * 3. Burst Balloons: Inverted interval DP ("choose last balloon to burst") O(n^3).
 * 4. Cutting Sticks: Segment-based interval DP O(c^3).
 * 5. Longest Palindromic Subsequence (LPS): Boundary-shrinking O(n^2) DP with witness reconstruction.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>
#include <cassert>
#include <memory>

namespace interval_dp {

// ============================================================================
// 1. Matrix Chain Multiplication (MCM)
// ============================================================================

/**
 * Computes the minimum scalar multiplication cost for a chain of matrices.
 * dims has size n + 1, where matrix A_i has dimensions dims[i] x dims[i+1].
 * Time: O(n^3), Space: O(n^2)
 */
long long matrix_chain_cost(const std::vector<int>& dims) {
    int n = static_cast<int>(dims.size()) - 1;
    if (n <= 1) return 0;

    const long long INF = std::numeric_limits<long long>::max() / 4;
    std::vector<std::vector<long long>> dp(n, std::vector<long long>(n, 0));

    // Fill table by increasing interval length
    for (int len = 2; len <= n; ++len) {
        for (int i = 0; i + len - 1 < n; ++i) {
            int j = i + len - 1;
            dp[i][j] = INF;

            for (int k = i; k < j; ++k) {
                long long cost = dp[i][k] + dp[k + 1][j]
                    + 1LL * dims[i] * dims[k + 1] * dims[j + 1];
                dp[i][j] = std::min(dp[i][j], cost);
            }
        }
    }

    return dp[0][n - 1];
}

/**
 * Computes minimum cost and reconstructs the optimal parenthesization string.
 * Returns {min_cost, parenthesized_expression}.
 */
std::pair<long long, std::string> matrix_chain_parenthesization(const std::vector<int>& dims) {
    int n = static_cast<int>(dims.size()) - 1;
    if (n <= 0) return {0, ""};
    if (n == 1) return {0, "A0"};

    const long long INF = std::numeric_limits<long long>::max() / 4;
    std::vector<std::vector<long long>> dp(n, std::vector<long long>(n, 0));
    std::vector<std::vector<int>> split(n, std::vector<int>(n, -1));

    for (int len = 2; len <= n; ++len) {
        for (int i = 0; i + len - 1 < n; ++i) {
            int j = i + len - 1;
            dp[i][j] = INF;

            for (int k = i; k < j; ++k) {
                long long cost = dp[i][k] + dp[k + 1][j]
                    + 1LL * dims[i] * dims[k + 1] * dims[j + 1];
                if (cost < dp[i][j]) {
                    dp[i][j] = cost;
                    split[i][j] = k;
                }
            }
        }
    }

    auto build = [&](auto&& self, int i, int j) -> std::string {
        if (i == j) return "A" + std::to_string(i);
        int k = split[i][j];
        return "(" + self(self, i, k) + " x " + self(self, k + 1, j) + ")";
    };

    return {dp[0][n - 1], build(build, 0, n - 1)};
}

// ============================================================================
// 2. Optimal Binary Search Tree (OBST)
// ============================================================================

/**
 * Classical O(n^3) OBST search cost computation with prefix sum optimization.
 * freq[i] is the search frequency of key i.
 */
long long optimal_bst_cost(const std::vector<int>& freq) {
    int n = static_cast<int>(freq.size());
    if (n == 0) return 0;

    const long long INF = std::numeric_limits<long long>::max() / 4;
    std::vector<long long> prefix(n + 1, 0);
    for (int i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + freq[i];
    }

    auto range_sum = [&](int l, int r) -> long long {
        return prefix[r + 1] - prefix[l];
    };

    std::vector<std::vector<long long>> dp(n, std::vector<long long>(n, 0));

    for (int i = 0; i < n; ++i) {
        dp[i][i] = freq[i];
    }

    for (int len = 2; len <= n; ++len) {
        for (int i = 0; i + len - 1 < n; ++i) {
            int j = i + len - 1;
            dp[i][j] = INF;
            long long total = range_sum(i, j);

            for (int r = i; r <= j; ++r) {
                long long left = (r > i ? dp[i][r - 1] : 0);
                long long right = (r < j ? dp[r + 1][j] : 0);
                dp[i][j] = std::min(dp[i][j], left + right + total);
            }
        }
    }

    return dp[0][n - 1];
}

/**
 * Knuth-Yao Optimized OBST: O(n^2) Time.
 * Uses monotonicity of optimal root positions: opt[i][j-1] <= opt[i][j] <= opt[i+1][j].
 */
long long optimal_bst_cost_knuth(const std::vector<int>& freq) {
    int n = static_cast<int>(freq.size());
    if (n == 0) return 0;

    const long long INF = std::numeric_limits<long long>::max() / 4;
    std::vector<long long> prefix(n + 1, 0);
    for (int i = 0; i < n; ++i) {
        prefix[i + 1] = prefix[i] + freq[i];
    }

    auto range_sum = [&](int l, int r) -> long long {
        return prefix[r + 1] - prefix[l];
    };

    std::vector<std::vector<long long>> dp(n, std::vector<long long>(n, 0));
    std::vector<std::vector<int>> opt(n, std::vector<int>(n, 0));

    for (int i = 0; i < n; ++i) {
        dp[i][i] = freq[i];
        opt[i][i] = i;
    }

    for (int len = 2; len <= n; ++len) {
        for (int i = 0; i + len - 1 < n; ++i) {
            int j = i + len - 1;
            dp[i][j] = INF;
            long long total = range_sum(i, j);

            int start_r = opt[i][j - 1];
            int end_r = (i + 1 <= j ? opt[i + 1][j] : j);

            for (int r = start_r; r <= end_r; ++r) {
                long long left = (r > i ? dp[i][r - 1] : 0);
                long long right = (r < j ? dp[r + 1][j] : 0);
                long long cost = left + right + total;
                if (cost < dp[i][j]) {
                    dp[i][j] = cost;
                    opt[i][j] = r;
                }
            }
        }
    }

    return dp[0][n - 1];
}

// ============================================================================
// 3. Burst Balloons ("Choose the Last Action")
// ============================================================================

/**
 * Burst Balloons: Maximizes coins by choosing the LAST balloon to burst in interval (i, j).
 * Time: O(n^3), Space: O(n^2).
 */
int burst_balloons(const std::vector<int>& nums) {
    int orig_n = static_cast<int>(nums.size());
    if (orig_n == 0) return 0;

    // Pad with boundary 1s on left and right
    std::vector<int> a;
    a.reserve(orig_n + 2);
    a.push_back(1);
    for (int x : nums) a.push_back(x);
    a.push_back(1);

    int n = static_cast<int>(a.size());
    std::vector<std::vector<int>> dp(n, std::vector<int>(n, 0));

    // len represents open interval size between i and j (distance = j - i)
    for (int len = 2; len < n; ++len) {
        for (int i = 0; i + len < n; ++i) {
            int j = i + len;
            // Try all possible last balloons k burst strictly between i and j
            for (int k = i + 1; k < j; ++k) {
                int gain = dp[i][k] + dp[k][j] + a[i] * a[k] * a[j];
                dp[i][j] = std::max(dp[i][j], gain);
            }
        }
    }

    return dp[0][n - 1];
}

// ============================================================================
// 4. Cutting Sticks
// ============================================================================

/**
 * Cutting Sticks: Computes minimum cost to perform cuts on a stick of given length.
 * Each cut costs the current length of the segment being cut.
 * Time: O(c^3) where c is the number of cut positions.
 */
int cutting_sticks(int stick_len, std::vector<int> cuts) {
    if (cuts.empty() || stick_len <= 0) return 0;

    std::sort(cuts.begin(), cuts.end());
    std::vector<int> pos;
    pos.reserve(cuts.size() + 2);
    pos.push_back(0);
    for (int c : cuts) pos.push_back(c);
    pos.push_back(stick_len);

    int m = static_cast<int>(pos.size());
    const int INF = std::numeric_limits<int>::max() / 4;
    std::vector<std::vector<int>> dp(m, std::vector<int>(m, 0));

    for (int len = 2; len < m; ++len) {
        for (int i = 0; i + len < m; ++i) {
            int j = i + len;
            if (len == 1) {
                dp[i][j] = 0;
                continue;
            }
            dp[i][j] = INF;
            int segment_cost = pos[j] - pos[i];
            for (int k = i + 1; k < j; ++k) {
                dp[i][j] = std::min(dp[i][j], dp[i][k] + dp[k][j] + segment_cost);
            }
        }
    }

    return dp[0][m - 1];
}

// ============================================================================
// 5. Longest Palindromic Subsequence (LPS)
// ============================================================================

/**
 * Computes length of Longest Palindromic Subsequence using boundary shrinking DP.
 * Time: O(n^2), Space: O(n^2).
 */
int longest_palindromic_subsequence(const std::string& s) {
    int n = static_cast<int>(s.size());
    if (n <= 1) return n;

    std::vector<std::vector<int>> dp(n, std::vector<int>(n, 0));

    for (int i = 0; i < n; ++i) {
        dp[i][i] = 1;
    }

    for (int len = 2; len <= n; ++len) {
        for (int i = 0; i + len - 1 < n; ++i) {
            int j = i + len - 1;
            if (s[i] == s[j]) {
                dp[i][j] = (len == 2 ? 2 : dp[i + 1][j - 1] + 2);
            } else {
                dp[i][j] = std::max(dp[i + 1][j], dp[i][j - 1]);
            }
        }
    }

    return dp[0][n - 1];
}

/**
 * Reconstructs one optimal Longest Palindromic Subsequence witness string.
 * Time: O(n^2), Space: O(n^2).
 */
std::string longest_palindromic_subsequence_reconstruct(const std::string& s) {
    int n = static_cast<int>(s.size());
    if (n == 0) return "";
    if (n == 1) return s;

    std::vector<std::vector<int>> dp(n, std::vector<int>(n, 0));

    for (int i = 0; i < n; ++i) {
        dp[i][i] = 1;
    }

    for (int len = 2; len <= n; ++len) {
        for (int i = 0; i + len - 1 < n; ++i) {
            int j = i + len - 1;
            if (s[i] == s[j]) {
                dp[i][j] = (len == 2 ? 2 : dp[i + 1][j - 1] + 2);
            } else {
                dp[i][j] = std::max(dp[i + 1][j], dp[i][j - 1]);
            }
        }
    }

    // Backtrack to extract palindrome
    int i = 0;
    int j = n - 1;
    std::string left_half;
    std::string right_half;

    while (i <= j) {
        if (i == j) {
            left_half.push_back(s[i]);
            break;
        }
        if (s[i] == s[j]) {
            left_half.push_back(s[i]);
            right_half.push_back(s[j]);
            ++i;
            --j;
        } else if (dp[i + 1][j] >= dp[i][j - 1]) {
            ++i;
        } else {
            --j;
        }
    }

    std::reverse(right_half.begin(), right_half.end());
    return left_half + right_half;
}

} // namespace interval_dp

// ============================================================================
// Unit Tests
// ============================================================================

int main() {
    using namespace interval_dp;

    // Test 1: Matrix Chain Multiplication - Arthur's Worked Example
    // A0: 10x100, A1: 100x5, A2: 5x50 -> dims = {10, 100, 5, 50}
    // (A0 x A1) x A2 costs 7500
    // A0 x (A1 x A2) costs 75000
    {
        std::vector<int> dims = {10, 100, 5, 50};
        long long cost = matrix_chain_cost(dims);
        assert(cost == 7500);

        auto [rec_cost, expr] = matrix_chain_parenthesization(dims);
        assert(rec_cost == 7500);
        assert(expr == "((A0 x A1) x A2)");

        // 4 matrices: CLRS standard example
        // A0: 30x35, A1: 35x15, A2: 15x5, A3: 5x10, A4: 10x20, A5: 20x25
        std::vector<int> clrs_dims = {30, 35, 15, 5, 10, 20, 25};
        assert(matrix_chain_cost(clrs_dims) == 15125);
    }

    // Test 2: Optimal BST - Classical O(n^3) and Knuth O(n^2)
    {
        std::vector<int> freq = {34, 8, 50};
        long long cost_cubic = optimal_bst_cost(freq);
        long long cost_knuth = optimal_bst_cost_knuth(freq);
        assert(cost_cubic == cost_knuth);
        assert(cost_cubic == 142); // root=2(50), left=0(34) with right=1(8)

        std::vector<int> freq2 = {4, 2, 6, 3};
        assert(optimal_bst_cost(freq2) == optimal_bst_cost_knuth(freq2));
    }

    // Test 3: Burst Balloons
    // nums = [3, 1, 5, 8] -> ans = 167
    {
        std::vector<int> nums = {3, 1, 5, 8};
        assert(burst_balloons(nums) == 167);

        std::vector<int> nums_single = {5};
        assert(burst_balloons(nums_single) == 5);

        assert(burst_balloons({}) == 0);
    }

    // Test 4: Cutting Sticks
    // Stick length 10, cuts at [2, 4, 7]
    {
        int length = 10;
        std::vector<int> cuts = {2, 4, 7};
        int cost = cutting_sticks(length, cuts);
        assert(cost == 20); // (cut at 4 -> cost 10; cut left at 2 -> cost 4; cut right at 7 -> cost 6 => 20)

        assert(cutting_sticks(5, {}) == 0);
        assert(cutting_sticks(10, {5}) == 10);
    }

    // Test 5: Longest Palindromic Subsequence (LPS)
    // s = "bbbab" -> "bbbb" (len 4)
    // s = "cbbd" -> "bb" (len 2)
    {
        std::string s1 = "bbbab";
        assert(longest_palindromic_subsequence(s1) == 4);
        std::string rec1 = longest_palindromic_subsequence_reconstruct(s1);
        assert(rec1.size() == 4);
        assert(rec1 == "bbbb");

        std::string s2 = "cbbd";
        assert(longest_palindromic_subsequence(s2) == 2);
        std::string rec2 = longest_palindromic_subsequence_reconstruct(s2);
        assert(rec2.size() == 2);
        assert(rec2 == "bb");

        // Single and empty string
        assert(longest_palindromic_subsequence("") == 0);
        assert(longest_palindromic_subsequence_reconstruct("") == "");
        assert(longest_palindromic_subsequence("a") == 1);
        assert(longest_palindromic_subsequence_reconstruct("a") == "a");
    }

    std::cout << "[PASS] All Interval and Matrix DP C++ unit tests passed." << std::endl;
    return 0;
}
