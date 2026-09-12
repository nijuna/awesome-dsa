/**
 * Reference Implementation: Quickselect and Median of Medians
 * Demonstrates:
 * 1. Lomuto Partition Scheme - O(n)
 * 2. Randomized Quickselect (Iterative Lomuto) - Expected O(n), Worst O(n^2)
 * 3. Randomized Quickselect with 3-Way Partitioning (Dutch National Flag) - Expected O(n)
 * 4. Deterministic Median of Medians Selection (BFPRT) - Worst-case O(n)
 * 5. Top-K Smallest Elements Selection - Expected O(n)
 * 6. Max-Heap Selection Comparison - O(n log k)
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <random>
#include <stdexcept>
#include <cassert>
#include <queue>

namespace selection {

// ============================================================================
// 1. Lomuto Partition
// ============================================================================

/**
 * Standard Lomuto partition scheme on subarray a[left..right].
 * Uses a[right] as pivot. Places pivot at its final sorted rank index.
 * Invariant: a[left..i-1] < pivot, a[i..j-1] >= pivot.
 * Time Complexity: O(n). Auxiliary Space: O(1).
 */
int lomuto_partition(std::vector<int>& a, int left, int right) {
    int pivot = a[right];
    int i = left;

    for (int j = left; j < right; ++j) {
        if (a[j] < pivot) {
            std::swap(a[i], a[j]);
            ++i;
        }
    }

    std::swap(a[i], a[right]);
    return i;
}

// ============================================================================
// 2. Randomized Quickselect (Iterative Lomuto)
// ============================================================================

/**
 * Selects the k-th smallest element (0-based) using randomized Quickselect.
 * Time Complexity: Expected O(n), Worst-case O(n^2).
 * Space Complexity: O(1) auxiliary (iterative, in-place on copy).
 */
int quickselect_lomuto(std::vector<int> a, int k) {
    if (k < 0 || k >= static_cast<int>(a.size())) {
        throw std::out_of_range("k out of range");
    }

    std::mt19937 rng(1337); // deterministic seed for reproducibility
    int left = 0;
    int right = static_cast<int>(a.size()) - 1;

    while (left <= right) {
        std::uniform_int_distribution<int> dist(left, right);
        int pivot_index = dist(rng);
        std::swap(a[pivot_index], a[right]);

        int p = lomuto_partition(a, left, right);

        if (p == k) {
            return a[p];
        } else if (k < p) {
            right = p - 1;
        } else {
            left = p + 1;
        }
    }

    throw std::logic_error("unreachable");
}

// ============================================================================
// 3. Randomized Quickselect with 3-Way Partition (Dutch National Flag)
// ============================================================================

/**
 * In-place 3-way Quickselect helper on vector reference.
 */
int quickselect_three_way_inplace(std::vector<int>& a, int k) {
    if (k < 0 || k >= static_cast<int>(a.size())) {
        throw std::out_of_range("k out of range");
    }

    std::mt19937 rng(42);
    int left = 0;
    int right = static_cast<int>(a.size()) - 1;

    while (left <= right) {
        std::uniform_int_distribution<int> dist(left, right);
        int pivot_idx = dist(rng);
        int pivot = a[pivot_idx];

        int lt = left;
        int i = left;
        int gt = right;

        while (i <= gt) {
            if (a[i] < pivot) {
                std::swap(a[lt], a[i]);
                ++lt;
                ++i;
            } else if (a[i] > pivot) {
                std::swap(a[i], a[gt]);
                --gt;
            } else {
                ++i;
            }
        }

        if (k < lt) {
            right = lt - 1;
        } else if (k > gt) {
            left = gt + 1;
        } else {
            return a[k]; // k falls inside the equal block
        }
    }

    throw std::logic_error("unreachable");
}

/**
 * Selects the k-th smallest element (0-based) handling duplicate values efficiently.
 * Splits range into three regions: (< pivot), (== pivot), (> pivot).
 * Time Complexity: Expected O(n). Space Complexity: O(1) auxiliary.
 */
int quickselect_three_way(std::vector<int> a, int k) {
    return quickselect_three_way_inplace(a, k);
}

// ============================================================================
// 4. Deterministic Median of Medians Selection (BFPRT)
// ============================================================================

/**
 * Selects the k-th smallest element (0-based) in guaranteed worst-case O(n) time.
 * Groups elements into blocks of 5, finds their medians, and recursively
 * selects the median of medians as pivot.
 * Time Complexity: Worst-case O(n).
 * Auxiliary Space: O(n) for recursive partition buffers.
 */
int median_of_medians_select(std::vector<int> a, int k) {
    if (k < 0 || k >= static_cast<int>(a.size())) {
        throw std::out_of_range("k out of range");
    }

    auto select = [&](auto&& self, std::vector<int> arr, int rank) -> int {
        int n = static_cast<int>(arr.size());
        if (n <= 5) {
            std::sort(arr.begin(), arr.end());
            return arr[rank];
        }

        // Step 1: Divide into groups of 5 and collect medians
        std::vector<int> medians;
        medians.reserve((n + 4) / 5);
        for (int i = 0; i < n; i += 5) {
            int r = std::min(i + 5, n);
            std::vector<int> group(arr.begin() + i, arr.begin() + r);
            std::sort(group.begin(), group.end());
            medians.push_back(group[group.size() / 2]);
        }

        // Step 2: Recursively find the median of medians
        int pivot = self(self, medians, static_cast<int>(medians.size()) / 2);

        // Step 3: Three-way partition around the pivot value
        std::vector<int> lows, highs, pivots;
        lows.reserve(n);
        highs.reserve(n);
        pivots.reserve(n);

        for (int x : arr) {
            if (x < pivot) lows.push_back(x);
            else if (x > pivot) highs.push_back(x);
            else pivots.push_back(x);
        }

        // Step 4: Recurse into the relevant subset
        int n_lows = static_cast<int>(lows.size());
        int n_pivots = static_cast<int>(pivots.size());

        if (rank < n_lows) {
            return self(self, lows, rank);
        }
        if (rank < n_lows + n_pivots) {
            return pivot;
        }
        return self(self, highs, rank - n_lows - n_pivots);
    };

    return select(select, a, k);
}

// ============================================================================
// 5. Top-K Smallest Elements Selection
// ============================================================================

/**
 * Returns the k smallest elements (unsorted or partially sorted).
 * Time Complexity: Expected O(n). Auxiliary Space: O(n) for result.
 */
std::vector<int> top_k_smallest(std::vector<int> a, int k) {
    if (k <= 0) return {};
    if (k >= static_cast<int>(a.size())) return a;

    // Quickselect puts the k-th smallest element at index k - 1,
    // ensuring all elements in a[0..k-1] are <= a[k-1].
    quickselect_three_way_inplace(a, k - 1);
    return std::vector<int>(a.begin(), a.begin() + k);
}

// ============================================================================
// 6. Max-Heap Selection Comparison (for small k or streaming)
// ============================================================================

/**
 * Selects the k-th smallest element (0-based) using a max-heap of size k + 1.
 * Time Complexity: O(n log k). Space Complexity: O(k).
 */
int heap_select_kth_smallest(const std::vector<int>& a, int k) {
    if (k < 0 || k >= static_cast<int>(a.size())) {
        throw std::out_of_range("k out of range");
    }

    std::priority_queue<int> max_heap;

    for (int x : a) {
        max_heap.push(x);
        if (static_cast<int>(max_heap.size()) > k + 1) {
            max_heap.pop();
        }
    }

    return max_heap.top();
}

} // namespace selection

// ============================================================================
// Unit Tests & Edge Case Verification
// ============================================================================

int main() {
    using namespace selection;

    std::cout << "Running Quickselect and Median of Medians C++17 unit tests...\n";

    // ------------------------------------------------------------------------
    // Test 1: Out of range and single element
    // ------------------------------------------------------------------------
    {
        bool caught = false;
        try {
            quickselect_lomuto({}, 0);
        } catch (const std::out_of_range&) {
            caught = true;
        }
        assert(caught);

        std::vector<int> single = {42};
        assert(quickselect_lomuto(single, 0) == 42);
        assert(quickselect_three_way(single, 0) == 42);
        assert(median_of_medians_select(single, 0) == 42);
        assert(heap_select_kth_smallest(single, 0) == 42);
    }

    // ------------------------------------------------------------------------
    // Test 2: Arthur's Worked Intuition Example
    // Input: [9, 1, 8, 2, 7, 3, 6], sorted: [1, 2, 3, 6, 7, 8, 9]
    // Index 0: 1, Index 3: 6, Index 4: 7, Index 6: 9
    // ------------------------------------------------------------------------
    {
        std::vector<int> a = {9, 1, 8, 2, 7, 3, 6};
        assert(quickselect_lomuto(a, 0) == 1);
        assert(quickselect_lomuto(a, 3) == 6);
        assert(quickselect_lomuto(a, 4) == 7);
        assert(quickselect_lomuto(a, 6) == 9);

        assert(quickselect_three_way(a, 0) == 1);
        assert(quickselect_three_way(a, 3) == 6);
        assert(quickselect_three_way(a, 4) == 7);
        assert(quickselect_three_way(a, 6) == 9);

        assert(median_of_medians_select(a, 0) == 1);
        assert(median_of_medians_select(a, 3) == 6);
        assert(median_of_medians_select(a, 4) == 7);
        assert(median_of_medians_select(a, 6) == 9);

        assert(heap_select_kth_smallest(a, 4) == 7);
    }

    // ------------------------------------------------------------------------
    // Test 3: Arrays with Massive Duplicates
    // ------------------------------------------------------------------------
    {
        std::vector<int> all_same(100, 7);
        for (int k = 0; k < 100; ++k) {
            assert(quickselect_three_way(all_same, k) == 7);
            assert(median_of_medians_select(all_same, k) == 7);
            assert(heap_select_kth_smallest(all_same, k) == 7);
        }

        // Mixture of repeated values: 10 ones, 20 twos, 15 threes
        std::vector<int> repeated;
        for (int i = 0; i < 10; ++i) repeated.push_back(1);
        for (int i = 0; i < 20; ++i) repeated.push_back(2);
        for (int i = 0; i < 15; ++i) repeated.push_back(3);

        // Ranks 0..9 should be 1
        // Ranks 10..29 should be 2
        // Ranks 30..44 should be 3
        assert(quickselect_three_way(repeated, 5) == 1);
        assert(quickselect_three_way(repeated, 10) == 2);
        assert(quickselect_three_way(repeated, 25) == 2);
        assert(quickselect_three_way(repeated, 30) == 3);
        assert(quickselect_three_way(repeated, 44) == 3);

        assert(median_of_medians_select(repeated, 5) == 1);
        assert(median_of_medians_select(repeated, 10) == 2);
        assert(median_of_medians_select(repeated, 25) == 2);
        assert(median_of_medians_select(repeated, 30) == 3);
        assert(median_of_medians_select(repeated, 44) == 3);
    }

    // ------------------------------------------------------------------------
    // Test 4: Already Sorted and Reverse Sorted Arrays (Worst-Case Checks)
    // ------------------------------------------------------------------------
    {
        std::vector<int> asc = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        std::vector<int> desc = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};

        for (int k = 0; k < 10; ++k) {
            assert(quickselect_lomuto(asc, k) == k + 1);
            assert(quickselect_lomuto(desc, k) == k + 1);

            assert(quickselect_three_way(asc, k) == k + 1);
            assert(quickselect_three_way(desc, k) == k + 1);

            assert(median_of_medians_select(asc, k) == k + 1);
            assert(median_of_medians_select(desc, k) == k + 1);
        }
    }

    // ------------------------------------------------------------------------
    // Test 5: Comprehensive Cross-Validation on Diverse Sizes & Seeds
    // ------------------------------------------------------------------------
    {
        std::mt19937 rng(999);
        std::vector<int> sizes = {2, 3, 4, 5, 6, 7, 11, 15, 23, 50, 105};

        for (int n : sizes) {
            std::vector<int> arr(n);
            std::uniform_int_distribution<int> val_dist(-500, 500);
            for (int i = 0; i < n; ++i) {
                arr[i] = val_dist(rng);
            }

            std::vector<int> sorted_arr = arr;
            std::sort(sorted_arr.begin(), sorted_arr.end());

            // Check random ranks including 0, n-1, and median
            std::vector<int> test_ranks = {0, n / 4, n / 2, 3 * n / 4, n - 1};
            for (int k : test_ranks) {
                int expected = sorted_arr[k];
                assert(quickselect_lomuto(arr, k) == expected);
                assert(quickselect_three_way(arr, k) == expected);
                assert(median_of_medians_select(arr, k) == expected);
                assert(heap_select_kth_smallest(arr, k) == expected);
            }
        }
    }

    // ------------------------------------------------------------------------
    // Test 6: Top-K Smallest Elements
    // ------------------------------------------------------------------------
    {
        std::vector<int> a = {14, 2, 8, 1, 9, 3, 7, 5};
        int k = 4;
        auto top4 = top_k_smallest(a, k);
        assert(top4.size() == 4);

        std::sort(top4.begin(), top4.end());
        assert(top4 == (std::vector<int>{1, 2, 3, 5}));

        assert(top_k_smallest(a, 0).empty());
        assert(top_k_smallest(a, 10).size() == 8);
    }

    std::cout << "All Quickselect and Median of Medians C++17 unit tests passed successfully!\n";
    return 0;
}
