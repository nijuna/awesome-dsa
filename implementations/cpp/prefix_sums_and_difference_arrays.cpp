#include <iostream>
#include <vector>
#include <tuple>
#include <cassert>
#include <stdexcept>

namespace dsa {

/**
 * @brief Computes 1D prefix sums using the 1-based / leading-zero convention.
 *
 * pref[0] = 0
 * pref[i + 1] = pref[i] + a[i]
 *
 * Time Complexity: O(n)
 * Space Complexity: O(n)
 *
 * @param a Input vector of integers.
 * @return std::vector<long long> Vector of size n + 1 containing prefix sums.
 */
inline std::vector<long long> prefix_sums(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    std::vector<long long> pref(n + 1, 0);
    for (int i = 0; i < n; ++i) {
        pref[i + 1] = pref[i] + a[i];
    }
    return pref;
}

/**
 * @brief Queries the sum of elements in subarray a[l..r] (inclusive) in O(1) time.
 *
 * @param pref The prefix sum array produced by prefix_sums().
 * @param l Left index (0-based, 0 <= l <= r).
 * @param r Right index (0-based, r < n).
 * @return long long The subarray sum a[l] + ... + a[r].
 */
inline long long range_sum(const std::vector<long long>& pref, int l, int r) {
    if (l > r) return 0;
    return pref[r + 1] - pref[l];
}

/**
 * @brief Computes 2D prefix sums for an n x m matrix with leading-zero boundaries.
 *
 * pref[i+1][j+1] = sum of all a[r][c] for 0 <= r <= i, 0 <= c <= j.
 *
 * Time Complexity: O(n * m)
 * Space Complexity: O(n * m)
 *
 * @param a Input 2D matrix.
 * @return std::vector<std::vector<long long>> (n+1) x (m+1) table.
 */
inline std::vector<std::vector<long long>> prefix_sums_2d(
    const std::vector<std::vector<int>>& a) {

    int n = static_cast<int>(a.size());
    int m = n ? static_cast<int>(a[0].size()) : 0;
    std::vector<std::vector<long long>> pref(n + 1, std::vector<long long>(m + 1, 0));

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            pref[i + 1][j + 1] =
                pref[i][j + 1] +
                pref[i + 1][j] -
                pref[i][j] +
                a[i][j];
        }
    }
    return pref;
}

/**
 * @brief Computes the sum of elements inside subrectangle [r1..r2] x [c1..c2] in O(1).
 *
 * Uses 2D inclusion-exclusion:
 * rect_sum = pref[r2+1][c2+1] - pref[r1][c2+1] - pref[r2+1][c1] + pref[r1][c1].
 *
 * @param pref 2D prefix table from prefix_sums_2d.
 * @param r1 Top row index (0-based).
 * @param c1 Left column index (0-based).
 * @param r2 Bottom row index (0-based).
 * @param c2 Right column index (0-based).
 * @return long long Sum of entries in the subrectangle.
 */
inline long long rectangle_sum(
    const std::vector<std::vector<long long>>& pref,
    int r1, int c1, int r2, int c2) {

    if (r1 > r2 || c1 > c2) return 0;
    return pref[r2 + 1][c2 + 1]
         - pref[r1][c2 + 1]
         - pref[r2 + 1][c1]
         + pref[r1][c1];
}

/**
 * @brief Applies offline 1D range additions [l, r, x] in O(1) per update using difference array.
 *
 * @param n Size of target array (initialized to zeros).
 * @param updates Vector of (l, r, value) range additions.
 * @return std::vector<long long> Final reconstructed array of length n.
 */
inline std::vector<long long> apply_range_additions(
    int n,
    const std::vector<std::tuple<int, int, long long>>& updates) {

    if (n <= 0) return {};
    std::vector<long long> diff(n + 1, 0);

    for (const auto& [l, r, x] : updates) {
        if (l <= r && l < n) {
            diff[l] += x;
            if (r + 1 < n) {
                diff[r + 1] -= x;
            }
        }
    }

    std::vector<long long> a(n, 0);
    long long cur = 0;
    for (int i = 0; i < n; ++i) {
        cur += diff[i];
        a[i] = cur;
    }
    return a;
}

/**
 * @brief Constructs a difference array from an existing arbitrary array.
 *
 * diff[0] = a[0], diff[i] = a[i] - a[i-1].
 *
 * Time Complexity: O(n)
 *
 * @param a Initial vector.
 * @return std::vector<long long> Difference array.
 */
inline std::vector<long long> build_difference_array(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    std::vector<long long> diff(n, 0);
    if (n == 0) return diff;

    diff[0] = a[0];
    for (int i = 1; i < n; ++i) {
        diff[i] = a[i] - a[i - 1];
    }
    return diff;
}

/**
 * @brief Reconstructs the original array from its difference array via prefix summation.
 *
 * Time Complexity: O(n)
 *
 * @param diff Difference array.
 * @return std::vector<long long> Reconstructed array.
 */
inline std::vector<long long> reconstruct_from_difference(
    const std::vector<long long>& diff) {

    int n = static_cast<int>(diff.size());
    std::vector<long long> a(n, 0);
    if (n == 0) return a;

    a[0] = diff[0];
    for (int i = 1; i < n; ++i) {
        a[i] = a[i - 1] + diff[i];
    }
    return a;
}

/**
 * @brief Applies offline 2D rectangle additions [r1, c1, r2, c2, x] in O(1) per update.
 *
 * Corner marks:
 * diff[r1][c1] += x
 * diff[r1][c2+1] -= x
 * diff[r2+1][c1] -= x
 * diff[r2+1][c2+1] += x
 *
 * Reconstructed via 2D prefix accumulation:
 * a[i][j] = diff[i][j] + a[i-1][j] + a[i][j-1] - a[i-1][j-1].
 *
 * Time Complexity: O(updates.size() + n * m)
 *
 * @param n Rows.
 * @param m Columns.
 * @param updates List of (r1, c1, r2, c2, x).
 * @return std::vector<std::vector<long long>> Reconstructed n x m matrix.
 */
inline std::vector<std::vector<long long>> apply_rectangle_additions(
    int n, int m,
    const std::vector<std::tuple<int, int, int, int, long long>>& updates) {

    if (n <= 0 || m <= 0) return {};
    std::vector<std::vector<long long>> diff(n + 1, std::vector<long long>(m + 1, 0));

    for (const auto& [r1, c1, r2, c2, x] : updates) {
        if (r1 <= r2 && c1 <= c2 && r1 < n && c1 < m) {
            diff[r1][c1] += x;
            if (c2 + 1 < m) diff[r1][c2 + 1] -= x;
            if (r2 + 1 < n) diff[r2 + 1][c1] -= x;
            if (r2 + 1 < n && c2 + 1 < m) diff[r2 + 1][c2 + 1] += x;
        }
    }

    std::vector<std::vector<long long>> a(n, std::vector<long long>(m, 0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            long long up = (i > 0 ? a[i - 1][j] : 0);
            long long left = (j > 0 ? a[i][j - 1] : 0);
            long long diag = (i > 0 && j > 0 ? a[i - 1][j - 1] : 0);
            a[i][j] = diff[i][j] + up + left - diag;
        }
    }
    return a;
}

} // namespace dsa

// ============================================================================
// Unit Tests
// ============================================================================

void test_1d_prefix_sums() {
    std::vector<int> a = {2, 4, 1, 7, 3, 6};
    auto pref = dsa::prefix_sums(a);
    assert(pref.size() == 7);
    assert(dsa::range_sum(pref, 0, 5) == 23);
    assert(dsa::range_sum(pref, 1, 4) == 15); // 4 + 1 + 7 + 3
    assert(dsa::range_sum(pref, 2, 2) == 1);
    assert(dsa::range_sum(pref, 3, 2) == 0); // Invalid range

    // Negative numbers
    std::vector<int> neg = {3, -2, 5, -1};
    auto pref_neg = dsa::prefix_sums(neg);
    assert(dsa::range_sum(pref_neg, 0, 3) == 5);
    assert(dsa::range_sum(pref_neg, 1, 2) == 3); // -2 + 5

    // Empty array
    std::vector<int> empty;
    auto pref_empty = dsa::prefix_sums(empty);
    assert(pref_empty.size() == 1);
    assert(pref_empty[0] == 0);
}

void test_2d_prefix_sums() {
    std::vector<std::vector<int>> grid = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    auto pref2d = dsa::prefix_sums_2d(grid);
    assert(pref2d.size() == 4 && pref2d[0].size() == 4);

    // Full grid sum: 1..9 = 45
    assert(dsa::rectangle_sum(pref2d, 0, 0, 2, 2) == 45);

    // Single cell: (1, 1) = 5
    assert(dsa::rectangle_sum(pref2d, 1, 1, 1, 1) == 5);

    // Subgrid: rows 1..2, cols 0..1 -> 4 + 5 + 7 + 8 = 24
    assert(dsa::rectangle_sum(pref2d, 1, 0, 2, 1) == 24);

    // Top-left 2x2: 1 + 2 + 4 + 5 = 12
    assert(dsa::rectangle_sum(pref2d, 0, 0, 1, 1) == 12);
}

void test_1d_difference_array() {
    int n = 6;
    std::vector<std::tuple<int, int, long long>> updates = {
        {1, 3, 5},
        {2, 5, 2},
        {0, 2, 1}
    };
    // Expected:
    // idx 0: +1 = 1
    // idx 1: +5 +1 = 6
    // idx 2: +5 +2 +1 = 8
    // idx 3: +5 +2 = 7
    // idx 4: +2 = 2
    // idx 5: +2 = 2
    auto res = dsa::apply_range_additions(n, updates);
    std::vector<long long> expected = {1, 6, 8, 7, 2, 2};
    assert(res == expected);

    // Build and reconstruct roundtrip
    std::vector<int> a = {3, 1, 4, 1, 5, 9};
    auto diff = dsa::build_difference_array(a);
    auto reconstructed = dsa::reconstruct_from_difference(diff);
    for (size_t i = 0; i < a.size(); ++i) {
        assert(reconstructed[i] == a[i]);
    }
}

void test_2d_difference_array() {
    int n = 3, m = 3;
    std::vector<std::tuple<int, int, int, int, long long>> updates = {
        {0, 0, 1, 1, 3},  // Add 3 to top-left 2x2
        {1, 1, 2, 2, 2}   // Add 2 to bottom-right 2x2
    };
    auto res = dsa::apply_rectangle_additions(n, m, updates);
    std::vector<std::vector<long long>> expected = {
        {3, 3, 0},
        {3, 5, 2},
        {0, 2, 2}
    };
    assert(res == expected);
}

void test_combined_workflow() {
    // 1. Start with 0-array of size 5
    // 2. Apply range additions
    int n = 5;
    std::vector<std::tuple<int, int, long long>> updates = {
        {0, 2, 10},
        {2, 4, 5}
    };
    auto final_array = dsa::apply_range_additions(n, updates);
    // final_array: [10, 10, 15, 5, 5]

    // 3. Build prefix sums on final array
    std::vector<int> final_ints;
    for (auto v : final_array) final_ints.push_back(static_cast<int>(v));
    auto pref = dsa::prefix_sums(final_ints);

    // 4. Query range sums:
    // sum(1..3) = 10 + 15 + 5 = 30
    assert(dsa::range_sum(pref, 1, 3) == 30);
    // sum(0..4) = 45
    assert(dsa::range_sum(pref, 0, 4) == 45);
}

int main() {
    std::cout << "Running Prefix Sums and Difference Arrays C++17 unit tests...\n";
    test_1d_prefix_sums();
    test_2d_prefix_sums();
    test_1d_difference_array();
    test_2d_difference_array();
    test_combined_workflow();
    std::cout << "All Prefix Sums and Difference Arrays C++17 unit tests passed successfully!\n";
    return 0;
}
