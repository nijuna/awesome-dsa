/**
 * @file logic_and_proof_techniques.cpp
 * @brief Computational verification of logical equivalences, induction, and loop invariants.
 *
 * Implements truth-table verification of logical laws (De Morgan, contraposition),
 * automated inductive step checkers, and formally instrumented algorithms with runtime invariant assertions.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <functional>
#include <cassert>
#include <cstdint>
#include <algorithm>

namespace dsa {

/**
 * @brief Verifies if a boolean expression f(P, Q) is a tautology (True for all assignments).
 */
inline bool is_tautology_2vars(const std::function<bool(bool, bool)>& expr) {
    const bool states[2] = {false, true};
    for (bool P : states) {
        for (bool Q : states) {
            if (!expr(P, Q)) return false;
        }
    }
    return true;
}

/**
 * @brief Logical implication: P => Q is equivalent to (!P || Q).
 */
inline constexpr bool implies(bool p, bool q) {
    return !p || q;
}

/**
 * @brief Automated verification of weak mathematical induction for claim P(n) on range [base, max_n].
 * Verifies base case P(base) and inductive step P(k) => P(k+1) for all k.
 */
inline bool verify_induction(uint32_t base, uint32_t max_n, const std::function<bool(uint32_t)>& P) {
    // 1. Base case verification
    if (!P(base)) return false;

    // 2. Inductive step transmission check
    for (uint32_t k = base; k < max_n; ++k) {
        bool hyp = P(k);
        bool next = P(k + 1);
        if (!implies(hyp, next)) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Binary search instrumented with explicit formal loop invariant checks.
 * Invariant: If target exists in arr, it must lie within index range [low, high].
 */
inline int binary_search_with_invariant(const std::vector<int>& arr, int target) {
    int low = 0;
    int high = static_cast<int>(arr.size()) - 1;

    // Helper lambda to check invariant
    auto check_invariant = [&](int l, int h) {
        for (size_t i = 0; i < arr.size(); ++i) {
            if (arr[i] == target) {
                // Target must be within [l, h]
                assert(static_cast<int>(i) >= l && static_cast<int>(i) <= h);
            }
        }
    };

    // Initialization check
    check_invariant(low, high);

    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (arr[mid] == target) {
            return mid;
        } else if (arr[mid] < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
        // Maintenance check
        check_invariant(low, high);
    }

    // Termination check: Target not found, meaning it cannot exist in array
    for (int val : arr) {
        assert(val != target);
    }
    return -1;
}

/**
 * @brief Dutch National Flag 3-way partition instrumented with loop invariant.
 * Invariant partitions array into:
 *   [0 ... low-1]: strictly 0
 *   [low ... mid-1]: strictly 1
 *   [mid ... high]: unclassified
 *   [high+1 ... n-1]: strictly 2
 */
inline void dutch_national_flag_with_invariant(std::vector<int>& nums) {
    int low = 0;
    int mid = 0;
    int high = static_cast<int>(nums.size()) - 1;

    auto verify_dutch_invariant = [&](int l, int m, int h) {
        for (int i = 0; i < l; ++i) assert(nums[i] == 0);
        for (int i = l; i < m; ++i) assert(nums[i] == 1);
        for (size_t i = static_cast<size_t>(h + 1); i < nums.size(); ++i) assert(nums[i] == 2);
    };

    // Initialization
    verify_dutch_invariant(low, mid, high);

    while (mid <= high) {
        if (nums[mid] == 0) {
            std::swap(nums[low], nums[mid]);
            ++low;
            ++mid;
        } else if (nums[mid] == 1) {
            ++mid;
        } else {
            std::swap(nums[mid], nums[high]);
            --high;
        }
        // Maintenance
        verify_dutch_invariant(low, mid, high);
    }

    // Termination: unclassified interval [mid, high] is empty (mid > high)
    verify_dutch_invariant(low, mid, high);
}

} // namespace dsa

int main() {
    std::cout << "Running Logic and Proof Techniques C++17 unit tests..." << std::endl;

    // Test 1: De Morgan's Laws equivalence verification
    // !(P && Q) <=> (!P || !Q)
    assert(dsa::is_tautology_2vars([](bool p, bool q) {
        return (!(p && q)) == (!p || !q);
    }));

    // !(P || Q) <=> (!P && !Q)
    assert(dsa::is_tautology_2vars([](bool p, bool q) {
        return (!(p || q)) == (!p && !q);
    }));

    // Test 2: Contrapositive Equivalence
    // (P => Q) <=> (!Q => !P)
    assert(dsa::is_tautology_2vars([](bool p, bool q) {
        return dsa::implies(p, q) == dsa::implies(!q, !p);
    }));

    // Test 3: Implication identity
    // (P => Q) <=> (!P || Q)
    assert(dsa::is_tautology_2vars([](bool p, bool q) {
        return dsa::implies(p, q) == (!p || q);
    }));

    // Test 4: Verification of induction: Sum of first n integers claim P(n): sum_{i=1}^n i == n(n+1)/2
    assert(dsa::verify_induction(1, 1000, [](uint32_t n) {
        uint64_t sum = 0;
        for (uint32_t i = 1; i <= n; ++i) sum += i;
        uint64_t formula = static_cast<uint64_t>(n) * (n + 1) / 2;
        return sum == formula;
    }));

    // Test 5: Verification of induction: Sum of squares P(n): sum i^2 == n(n+1)(2n+1)/6
    assert(dsa::verify_induction(1, 500, [](uint32_t n) {
        uint64_t sum = 0;
        for (uint32_t i = 1; i <= n; ++i) sum += static_cast<uint64_t>(i) * i;
        uint64_t formula = static_cast<uint64_t>(n) * (n + 1) * (2 * n + 1) / 6;
        return sum == formula;
    }));

    // Test 6: Binary search loop invariant execution
    std::vector<int> sorted_arr = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    assert(dsa::binary_search_with_invariant(sorted_arr, 23) == 5);
    assert(dsa::binary_search_with_invariant(sorted_arr, 2) == 0);
    assert(dsa::binary_search_with_invariant(sorted_arr, 91) == 9);
    assert(dsa::binary_search_with_invariant(sorted_arr, 40) == -1);

    // Test 7: Dutch National Flag loop invariant execution
    std::vector<int> colors = {2, 0, 2, 1, 1, 0, 2, 1, 0, 0, 1, 2};
    dsa::dutch_national_flag_with_invariant(colors);
    assert(std::is_sorted(colors.begin(), colors.end()));

    std::cout << "[PASS] All Logic and Proof Techniques C++ unit tests passed." << std::endl;
    return 0;
}
