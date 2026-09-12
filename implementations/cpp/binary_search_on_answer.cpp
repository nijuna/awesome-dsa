/**
 * Reference Implementation: Binary Search on Answer
 * Demonstrates:
 * 1. Generic First True and Last True integer search templates.
 * 2. Continuous real-valued binary search with fixed-iteration convergence.
 * 3. Capacity Allocation: Split Array Largest Sum / Ship Packages in D Days.
 * 4. Rate & Speed: Koko Eating Bananas with ceiling arithmetic.
 * 5. Maximize Minimum Distance: Aggressive Cows / Stalls placement.
 * 6. Value-Space Rank Search: K-th smallest element in a sorted matrix.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <numeric>
#include <algorithm>
#include <cassert>
#include <cmath>

namespace binary_search_patterns {

// ============================================================================
// 1. Generic Binary Search Templates
// ============================================================================

/**
 * Searches for the first x in [low, high] where feasible(x) == true.
 * Assumes monotonic pattern: F F F ... T T T
 * Invariant: answer always lies in [low, high].
 */
template <typename Predicate>
long long first_true(long long low, long long high, Predicate feasible) {
    while (low < high) {
        long long mid = low + (high - low) / 2; // downward biased
        if (feasible(mid)) {
            high = mid; // answer could be mid or to the left
        } else {
            low = mid + 1; // mid is infeasible
        }
    }
    return low;
}

/**
 * Searches for the last x in [low, high] where feasible(x) == true.
 * Assumes monotonic pattern: T T T ... F F F
 * Invariant: answer always lies in [low, high].
 */
template <typename Predicate>
long long last_true(long long low, long long high, Predicate feasible) {
    while (low < high) {
        long long mid = low + (high - low + 1) / 2; // upward biased to prevent infinite loops
        if (feasible(mid)) {
            low = mid; // answer could be mid or to the right
        } else {
            high = mid - 1; // mid is infeasible
        }
    }
    return low;
}

/**
 * Continuous real-valued binary search over [low, high] using fixed iterations.
 * Guarantees precision to (high - low) / 2^iterations.
 */
template <typename Predicate>
double binary_search_real(double low, double high, Predicate feasible, int iterations = 80) {
    for (int i = 0; i < iterations; ++i) {
        double mid = (low + high) / 2.0;
        if (feasible(mid)) {
            high = mid;
        } else {
            low = mid;
        }
    }
    return high;
}

// ============================================================================
// 2. Capacity Allocation: Split Array Largest Sum
// ============================================================================

/**
 * Split Array Largest Sum:
 * Minimizes the largest subarray sum when splitting into at most k contiguous parts.
 * Time: O(n * log(sum - max)), Space: O(1).
 */
long long split_array_largest_sum(const std::vector<int>& a, int k) {
    if (a.empty()) return 0;

    auto feasible = [&](long long cap) -> bool {
        int parts = 1;
        long long cur = 0;
        for (int x : a) {
            if (cur + x <= cap) {
                cur += x;
            } else {
                ++parts;
                cur = x;
            }
        }
        return parts <= k;
    };

    long long low = *std::max_element(a.begin(), a.end());
    long long high = std::accumulate(a.begin(), a.end(), 0LL);

    return first_true(low, high, feasible);
}

// ============================================================================
// 3. Rate & Speed: Koko Eating Bananas
// ============================================================================

/**
 * Koko Eating Bananas:
 * Finds minimum integer eating speed k to consume all piles within h hours.
 * Uses ceiling division: ceil(p / k) = (p + k - 1) / k.
 * Time: O(n * log(max(piles))), Space: O(1).
 */
int min_eating_speed(const std::vector<int>& piles, int h) {
    if (piles.empty()) return 0;

    auto feasible = [&](long long k) -> bool {
        if (k <= 0) return false;
        long long hours = 0;
        for (int p : piles) {
            hours += (p + k - 1) / k;
        }
        return hours <= h;
    };

    long long low = 1;
    long long high = *std::max_element(piles.begin(), piles.end());

    return static_cast<int>(first_true(low, high, feasible));
}

// ============================================================================
// 4. Maximize Minimum Distance: Aggressive Cows
// ============================================================================

/**
 * Aggressive Cows:
 * Places k cows in sorted stall positions maximizing the minimum distance between any two cows.
 * Time: O(n log n + n * log(range)), Space: O(1).
 */
int aggressive_cows(std::vector<int> pos, int k) {
    if (pos.size() < static_cast<size_t>(k)) return 0;
    std::sort(pos.begin(), pos.end());

    auto feasible = [&](long long dist) -> bool {
        int used = 1;
        int last = pos[0];
        for (size_t i = 1; i < pos.size(); ++i) {
            if (pos[i] - last >= dist) {
                ++used;
                last = pos[i];
            }
        }
        return used >= k;
    };

    long long low = 0;
    long long high = pos.back() - pos.front();

    return static_cast<int>(last_true(low, high, feasible));
}

// ============================================================================
// 5. Value-Space Rank Search: K-th Smallest in Sorted Matrix
// ============================================================================

/**
 * K-th Smallest Element in a Matrix where each row and column is sorted ascending.
 * Binary searches over the value range [matrix[0][0], matrix[n-1][n-1]].
 * Counts elements <= x in O(n) using staircase walk.
 * Time: O(n * log(max_val - min_val)), Space: O(1).
 */
int kth_smallest_sorted_matrix(const std::vector<std::vector<int>>& matrix, int k) {
    int n = static_cast<int>(matrix.size());
    if (n == 0) return 0;

    // Counts matrix elements <= target
    auto count_less_or_equal = [&](long long target) -> int {
        int count = 0;
        int row = n - 1;
        int col = 0;

        while (row >= 0 && col < n) {
            if (matrix[row][col] <= target) {
                count += (row + 1); // all elements in column col above row are <= target
                ++col;
            } else {
                --row;
            }
        }
        return count;
    };

    auto feasible = [&](long long val) -> bool {
        return count_less_or_equal(val) >= k;
    };

    long long low = matrix[0][0];
    long long high = matrix[n - 1][n - 1];

    return static_cast<int>(first_true(low, high, feasible));
}

} // namespace binary_search_patterns

// ============================================================================
// Unit Tests
// ============================================================================

int main() {
    using namespace binary_search_patterns;

    // Test 1: Generic first_true and last_true
    {
        // Predicate: x >= 7 over [0, 20] -> first true is 7
        auto p1 = [](long long x) { return x >= 7; };
        assert(first_true(0, 20, p1) == 7);

        // Predicate: x <= 14 over [0, 20] -> last true is 14
        auto p2 = [](long long x) { return x <= 14; };
        assert(last_true(0, 20, p2) == 14);

        // Boundary checks
        assert(first_true(5, 5, [](long long) { return true; }) == 5);
        assert(last_true(5, 5, [](long long) { return true; }) == 5);
    }

    // Test 2: Split Array Largest Sum
    // nums = [7, 2, 5, 10, 8], k = 2 -> split [7, 2, 5] and [10, 8] -> max sum = 18
    {
        std::vector<int> nums = {7, 2, 5, 10, 8};
        assert(split_array_largest_sum(nums, 2) == 18);

        // k = 1 -> sum is total = 32
        assert(split_array_largest_sum(nums, 1) == 32);

        // k = 5 -> max element is 10
        assert(split_array_largest_sum(nums, 5) == 10);
    }

    // Test 3: Koko Eating Bananas
    // piles = [3, 6, 7, 11], h = 8 -> speed = 4
    // (3/4=1, 6/4=2, 7/4=2, 11/4=3 -> 1+2+2+3 = 8 hours)
    {
        std::vector<int> piles = {3, 6, 7, 11};
        assert(min_eating_speed(piles, 8) == 4);

        // piles = [30, 11, 23, 4, 20], h = 5 -> speed = 30
        std::vector<int> piles2 = {30, 11, 23, 4, 20};
        assert(min_eating_speed(piles2, 5) == 30);

        // h = 6 -> speed = 23
        assert(min_eating_speed(piles2, 6) == 23);
    }

    // Test 4: Aggressive Cows
    // pos = [1, 2, 8, 4, 9], k = 3 -> sorted [1, 2, 4, 8, 9]
    // Place at 1, 4, 8 -> min dist = 3. Place at 1, 4, 9 -> min dist = 3.
    // Can we do 4? 1, 8 (2 cows) -> no. So max min dist = 3.
    {
        std::vector<int> pos = {1, 2, 8, 4, 9};
        assert(aggressive_cows(pos, 3) == 3);

        // 2 cows in [1, 100] -> distance = 99
        assert(aggressive_cows({1, 100}, 2) == 99);
    }

    // Test 5: K-th Smallest in Sorted Matrix
    // matrix = [
    //   [ 1,  5,  9],
    //   [10, 11, 13],
    //   [12, 13, 15]
    // ], k = 8 -> 13
    {
        std::vector<std::vector<int>> mat = {
            { 1,  5,  9},
            {10, 11, 13},
            {12, 13, 15}
        };
        assert(kth_smallest_sorted_matrix(mat, 8) == 13);
        assert(kth_smallest_sorted_matrix(mat, 1) == 1);
        assert(kth_smallest_sorted_matrix(mat, 9) == 15);
    }

    // Test 6: Continuous Binary Search
    // Find sqrt(2): x^2 >= 2
    {
        auto feasible = [](double x) { return x * x >= 2.0; };
        double root2 = binary_search_real(1.0, 2.0, feasible, 60);
        assert(std::abs(root2 - std::sqrt(2.0)) < 1e-9);
    }

    std::cout << "[PASS] All Binary Search on Answer C++ unit tests passed." << std::endl;
    return 0;
}
