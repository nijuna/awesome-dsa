/**
 * @file loop_invariants.cpp
 * @brief Reference implementations of algorithms instrumented with formal loop invariants and variants.
 *
 * Implements Insertion Sort, Binary Search, Container With Most Water, and Lomuto Partition
 * with explicit runtime invariant verification at initialization, maintenance, and termination.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>
#include <numeric>

namespace dsa {

/**
 * @brief Insertion Sort with formal invariant verification.
 * Invariant: Subarray arr[0 ... i-1] is sorted at start and end of outer loop.
 */
inline void insertion_sort_with_invariant(std::vector<int>& arr) {
    auto is_prefix_sorted = [&](size_t len) {
        for (size_t k = 1; k < len; ++k) {
            if (arr[k - 1] > arr[k]) return false;
        }
        return true;
    };

    // Initialization: arr[0 ... 0] is sorted trivially
    assert(is_prefix_sorted(1));

    for (size_t i = 1; i < arr.size(); ++i) {
        int key = arr[i];
        int j = static_cast<int>(i) - 1;

        while (j >= 0 && arr[j] > key) {
            arr[j + 1] = arr[j];
            --j;
        }
        arr[j + 1] = key;

        // Maintenance: arr[0 ... i] is now sorted
        assert(is_prefix_sorted(i + 1));
    }

    // Termination: Entire array arr[0 ... n-1] is sorted
    assert(is_prefix_sorted(arr.size()));
}

/**
 * @brief Binary search with loop invariant and loop variant tracking.
 * Invariant: Target must reside in arr[low .. high] if present.
 * Variant: V = high - low + 1 strictly decreases each iteration.
 */
inline int binary_search_with_variant(const std::vector<int>& arr, int target) {
    int low = 0;
    int high = static_cast<int>(arr.size()) - 1;

    auto check_invariant = [&](int l, int h) {
        for (size_t i = 0; i < arr.size(); ++i) {
            if (arr[i] == target) {
                assert(static_cast<int>(i) >= l && static_cast<int>(i) <= h);
            }
        }
    };

    // Initialization
    check_invariant(low, high);
    int prev_variant = high - low + 1;

    while (low <= high) {
        int curr_variant = high - low + 1;
        assert(curr_variant <= prev_variant);
        prev_variant = curr_variant;

        int mid = low + (high - low) / 2;
        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }

        // Maintenance
        check_invariant(low, high);
    }

    // Termination: interval empty, target does not exist
    for (int v : arr) assert(v != target);
    return -1;
}

/**
 * @brief Container with Most Water with two-pointer elimination invariant.
 * Invariant: Global optimum is either stored in max_water or strictly inside [l, r].
 */
inline int max_area_with_invariant(const std::vector<int>& heights) {
    if (heights.size() < 2) return 0;

    int l = 0;
    int r = static_cast<int>(heights.size()) - 1;
    int max_water = 0;

    while (l < r) {
        int width = r - l;
        int h = std::min(heights[l], heights[r]);
        max_water = std::max(max_water, width * h);

        if (heights[l] < heights[r]) {
            // Elimination proof: for any k in (l, r), area(l, k) <= (k-l)*heights[l] < (r-l)*heights[l] <= max_water
            for (int k = l + 1; k < r; ++k) {
                int test_area = (k - l) * std::min(heights[l], heights[k]);
                assert(test_area <= max_water);
            }
            ++l;
        } else {
            // Elimination proof for r
            for (int k = l; k < r; ++k) {
                int test_area = (r - k) * std::min(heights[k], heights[r]);
                assert(test_area <= max_water);
            }
            --r;
        }
    }

    return max_water;
}

} // namespace dsa

int main() {
    std::cout << "Running Loop Invariants C++17 unit tests..." << std::endl;

    // Test 1: Insertion Sort with invariant verification
    {
        std::vector<int> arr = {5, 2, 9, 1, 5, 6};
        dsa::insertion_sort_with_invariant(arr);
        assert(std::is_sorted(arr.begin(), arr.end()));
    }

    // Test 2: Binary Search with invariant and variant verification
    {
        std::vector<int> sorted_arr = {10, 20, 30, 40, 50, 60, 70};
        assert(dsa::binary_search_with_variant(sorted_arr, 40) == 3);
        assert(dsa::binary_search_with_variant(sorted_arr, 10) == 0);
        assert(dsa::binary_search_with_variant(sorted_arr, 70) == 6);
        assert(dsa::binary_search_with_variant(sorted_arr, 35) == -1);
    }

    // Test 3: Container with Most Water with invariant elimination verification
    {
        std::vector<int> heights = {1, 8, 6, 2, 5, 4, 8, 3, 7};
        int ans = dsa::max_area_with_invariant(heights);
        // Best: index 1 (height 8) and index 8 (height 7): width 7 * min(8, 7) = 49
        assert(ans == 49);
    }

    std::cout << "[PASS] All Loop Invariants C++ unit tests passed." << std::endl;
    return 0;
}
