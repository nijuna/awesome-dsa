/**
 * Reference Implementation: Longest Increasing Subsequence (LIS)
 * Demonstrates:
 * 1. Classical Quadratic DP: O(N^2) Time, O(N) Space.
 * 2. Quadratic Parent-Pointer Reconstruction: O(N^2) Time.
 * 3. Patience Sorting / Binary Search (Tails Array): O(N log N) Time, O(N) Space.
 * 4. Optimized Subsequence Reconstruction via Tail Indices & Predecessors: O(N log N) Time.
 * 5. Variant: Longest Non-Decreasing Subsequence via Upper Bound: O(N log N) Time.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

namespace lis {

// ============================================================================
// 1. Classical DP Formulation: O(N^2) Time, O(N) Space
// ============================================================================

/**
 * Computes LIS length in O(N^2) time by defining:
 * dp[i] = length of LIS ending strictly at index i.
 */
int lis_length_n2(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    if (n == 0) return 0;

    std::vector<int> dp(n, 1);
    int best = 1;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            if (a[j] < a[i]) {
                dp[i] = std::max(dp[i], dp[j] + 1);
            }
        }
        best = std::max(best, dp[i]);
    }

    return best;
}

/**
 * Reconstructs the actual LIS sequence in O(N^2) time using predecessor links.
 */
std::vector<int> lis_sequence_n2(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    if (n == 0) return {};

    std::vector<int> dp(n, 1);
    std::vector<int> parent(n, -1);
    int best_len = 1;
    int best_end = 0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            if (a[j] < a[i] && dp[j] + 1 > dp[i]) {
                dp[i] = dp[j] + 1;
                parent[i] = j;
            }
        }
        if (dp[i] > best_len) {
            best_len = dp[i];
            best_end = i;
        }
    }

    std::vector<int> seq;
    for (int cur = best_end; cur != -1; cur = parent[cur]) {
        seq.push_back(a[cur]);
    }
    std::reverse(seq.begin(), seq.end());
    return seq;
}

// ============================================================================
// 2. Patience Sorting / Binary Search Formulation: O(N log N) Time
// ============================================================================

/**
 * Computes LIS length in O(N log N) time using the tails array.
 * tails[k] stores the minimum tail element among all valid increasing
 * subsequences of length (k + 1) discovered so far.
 */
int lis_length_nlogn(const std::vector<int>& a) {
    std::vector<int> tails;

    for (int x : a) {
        auto it = std::lower_bound(tails.begin(), tails.end(), x);
        if (it == tails.end()) {
            tails.push_back(x);
        } else {
            *it = x;
        }
    }

    return static_cast<int>(tails.size());
}

/**
 * Reconstructs an actual LIS sequence in O(N log N) time.
 * Maintains:
 * - tail_values: smallest tail value for each length
 * - tail_indices: array index in `a` of that tail representative
 * - parent: tracks predecessor for each index at the moment it joins a prefix
 */
std::vector<int> lis_sequence_nlogn(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    if (n == 0) return {};

    std::vector<int> tail_values;
    std::vector<int> tail_indices;
    std::vector<int> parent(n, -1);

    for (int i = 0; i < n; ++i) {
        int x = a[i];
        int pos = static_cast<int>(
            std::lower_bound(tail_values.begin(), tail_values.end(), x) - tail_values.begin()
        );

        if (pos == static_cast<int>(tail_values.size())) {
            tail_values.push_back(x);
            tail_indices.push_back(i);
        } else {
            tail_values[pos] = x;
            tail_indices[pos] = i;
        }

        if (pos > 0) {
            parent[i] = tail_indices[pos - 1];
        }
    }

    std::vector<int> seq;
    int cur = tail_indices.back();
    while (cur != -1) {
        seq.push_back(a[cur]);
        cur = parent[cur];
    }
    std::reverse(seq.begin(), seq.end());
    return seq;
}

// ============================================================================
// 3. Variant: Longest Non-Decreasing Subsequence (O(N log N))
// ============================================================================

/**
 * Computes Longest Non-Decreasing Subsequence length (allowing equal elements)
 * by using std::upper_bound instead of std::lower_bound.
 */
int longest_non_decreasing_length(const std::vector<int>& a) {
    std::vector<int> tails;

    for (int x : a) {
        auto it = std::upper_bound(tails.begin(), tails.end(), x);
        if (it == tails.end()) {
            tails.push_back(x);
        } else {
            *it = x;
        }
    }

    return static_cast<int>(tails.size());
}

} // namespace lis

// ============================================================================
// Comprehensive Unit Verification
// ============================================================================

int main() {
    using namespace lis;

    // Test 1: Empty Array
    std::vector<int> empty;
    assert(lis_length_n2(empty) == 0);
    assert(lis_length_nlogn(empty) == 0);
    assert(lis_sequence_n2(empty).empty());
    assert(lis_sequence_nlogn(empty).empty());
    assert(longest_non_decreasing_length(empty) == 0);

    // Test 2: Single Element
    std::vector<int> single = {42};
    assert(lis_length_n2(single) == 1);
    assert(lis_length_nlogn(single) == 1);
    assert(lis_sequence_n2(single) == single);
    assert(lis_sequence_nlogn(single) == single);
    assert(longest_non_decreasing_length(single) == 1);

    // Test 3: Canonical Worked Example [10, 9, 2, 5, 3, 7, 101, 18]
    std::vector<int> seq1 = {10, 9, 2, 5, 3, 7, 101, 18};
    assert(lis_length_n2(seq1) == 4);
    assert(lis_length_nlogn(seq1) == 4);

    auto rec_n2 = lis_sequence_n2(seq1);
    auto rec_nlogn = lis_sequence_nlogn(seq1);
    assert(rec_n2.size() == 4);
    assert(rec_nlogn.size() == 4);

    // Validate that reconstructed sequences are strictly increasing
    auto is_strictly_increasing = [](const std::vector<int>& v) -> bool {
        for (size_t i = 1; i < v.size(); ++i) {
            if (v[i] <= v[i - 1]) return false;
        }
        return true;
    };
    assert(is_strictly_increasing(rec_n2));
    assert(is_strictly_increasing(rec_nlogn));

    // Test 4: Second Example [3, 1, 5, 2, 6, 4, 9]
    std::vector<int> seq2 = {3, 1, 5, 2, 6, 4, 9};
    assert(lis_length_n2(seq2) == 4);
    assert(lis_length_nlogn(seq2) == 4);
    assert(is_strictly_increasing(lis_sequence_n2(seq2)));
    assert(is_strictly_increasing(lis_sequence_nlogn(seq2)));

    // Test 5: Strictly Decreasing Array
    std::vector<int> dec = {5, 4, 3, 2, 1};
    assert(lis_length_n2(dec) == 1);
    assert(lis_length_nlogn(dec) == 1);
    assert(lis_sequence_n2(dec).size() == 1);
    assert(lis_sequence_nlogn(dec).size() == 1);
    assert(longest_non_decreasing_length(dec) == 1);

    // Test 6: Strictly Increasing Array
    std::vector<int> inc = {1, 2, 3, 4, 5};
    assert(lis_length_n2(inc) == 5);
    assert(lis_length_nlogn(inc) == 5);
    assert(lis_sequence_n2(inc) == inc);
    assert(lis_sequence_nlogn(inc) == inc);
    assert(longest_non_decreasing_length(inc) == 5);

    // Test 7: Array with Duplicates
    // For [2, 2, 2, 2]: strictly increasing LIS length is 1; non-decreasing length is 4.
    std::vector<int> dups = {2, 2, 2, 2};
    assert(lis_length_n2(dups) == 1);
    assert(lis_length_nlogn(dups) == 1);
    assert(longest_non_decreasing_length(dups) == 4);

    // Non-decreasing with mixed elements: [1, 3, 2, 2, 5]
    std::vector<int> mixed = {1, 3, 2, 2, 5};
    // strictly increasing: [1, 2, 5] or [1, 3, 5] -> len 3
    assert(lis_length_n2(mixed) == 3);
    assert(lis_length_nlogn(mixed) == 3);
    // non-decreasing: [1, 2, 2, 5] -> len 4
    assert(longest_non_decreasing_length(mixed) == 4);

    std::cout << "[PASS] All Longest Increasing Subsequence C++ unit tests passed." << std::endl;
    return 0;
}
