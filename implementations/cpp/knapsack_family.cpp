/**
 * Reference Implementation: The Knapsack Problem Family
 * Demonstrates:
 * 1. 0-1 Knapsack: Classical 2D DP (O(nW) Time, O(nW) Space) & 1D Backward Rolling Array (O(W) Space).
 * 2. 0-1 Knapsack Item Reconstruction via Backtracking.
 * 3. Unbounded Knapsack: 1D Forward Rolling Array (O(nW) Time, O(W) Space) & Choice Tracking Reconstruction.
 * 4. Bounded Knapsack: Binary Power Splitting (O(W sum log m) Time) & Full Multiplicity Reconstruction.
 * 5. Variations: Exact-Fill 0-1 Knapsack (Sentinel Initialization) & Subset Sum Feasibility.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <cassert>
#include <limits>

namespace knapsack {

// ============================================================================
// 1. 0-1 Knapsack (Each Item at Most Once)
// ============================================================================

/**
 * 2D Classical Formulation: O(nW) Time, O(nW) Space.
 * dp[i][c] = max value considering first i items with capacity c.
 */
int knapsack_01_2d(const std::vector<int>& weight, const std::vector<int>& value, int W) {
    int n = static_cast<int>(weight.size());
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(W + 1, 0));

    for (int i = 1; i <= n; ++i) {
        int w = weight[i - 1];
        int v = value[i - 1];
        for (int c = 0; c <= W; ++c) {
            dp[i][c] = dp[i - 1][c];
            if (w <= c) {
                dp[i][c] = std::max(dp[i][c], dp[i - 1][c - w] + v);
            }
        }
    }

    return dp[n][W];
}

/**
 * 1D Space-Optimized Rolling Array: O(nW) Time, O(W) Space.
 * Backward capacity loop (W down to w) prevents item reuse in the same round.
 */
int knapsack_01_1d(const std::vector<int>& weight, const std::vector<int>& value, int W) {
    int n = static_cast<int>(weight.size());
    std::vector<int> dp(W + 1, 0);

    for (int i = 0; i < n; ++i) {
        int w = weight[i];
        int v = value[i];
        for (int c = W; c >= w; --c) {
            dp[c] = std::max(dp[c], dp[c - w] + v);
        }
    }

    return dp[W];
}

/**
 * 0-1 Knapsack Item Reconstruction using full 2D table.
 * Returns {max_value, chosen_indices}.
 */
std::pair<int, std::vector<int>> knapsack_01_reconstruct(
    const std::vector<int>& weight,
    const std::vector<int>& value,
    int W) {

    int n = static_cast<int>(weight.size());
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(W + 1, 0));

    for (int i = 1; i <= n; ++i) {
        int w = weight[i - 1];
        int v = value[i - 1];
        for (int c = 0; c <= W; ++c) {
            dp[i][c] = dp[i - 1][c];
            if (w <= c) {
                dp[i][c] = std::max(dp[i][c], dp[i - 1][c - w] + v);
            }
        }
    }

    std::vector<int> chosen;
    int c = W;
    for (int i = n; i >= 1; --i) {
        if (dp[i][c] != dp[i - 1][c]) {
            chosen.push_back(i - 1);
            c -= weight[i - 1];
        }
    }

    std::reverse(chosen.begin(), chosen.end());
    return {dp[n][W], chosen};
}

// ============================================================================
// 2. Unbounded Knapsack (Unlimited Copies of Each Item)
// ============================================================================

/**
 * 1D Forward Rolling Array: O(nW) Time, O(W) Space.
 * Forward capacity loop (w up to W) allows unrestricted reuse in the same round.
 */
int knapsack_unbounded(const std::vector<int>& weight, const std::vector<int>& value, int W) {
    std::vector<int> dp(W + 1, 0);

    for (size_t i = 0; i < weight.size(); ++i) {
        int w = weight[i];
        int v = value[i];
        for (int c = w; c <= W; ++c) {
            dp[c] = std::max(dp[c], dp[c - w] + v);
        }
    }

    return dp[W];
}

/**
 * Unbounded Knapsack Reconstruction using a 1D choice-tracking array.
 * Returns {max_value, chosen_indices}.
 */
std::pair<int, std::vector<int>> knapsack_unbounded_reconstruct(
    const std::vector<int>& weight,
    const std::vector<int>& value,
    int W) {

    std::vector<int> dp(W + 1, 0);
    std::vector<int> choice(W + 1, -1);

    for (int i = 0; i < static_cast<int>(weight.size()); ++i) {
        int w = weight[i];
        int v = value[i];
        for (int c = w; c <= W; ++c) {
            int candidate = dp[c - w] + v;
            if (candidate > dp[c]) {
                dp[c] = candidate;
                choice[c] = i;
            }
        }
    }

    std::vector<int> chosen;
    int c = W;
    while (c > 0 && choice[c] != -1) {
        int i = choice[c];
        chosen.push_back(i);
        c -= weight[i];
    }

    std::reverse(chosen.begin(), chosen.end());
    return {dp[W], chosen};
}

// ============================================================================
// 3. Bounded Knapsack (Multiplicity m_i via Binary Power Splitting)
// ============================================================================

struct Bundle {
    int w, v;
    int original_idx;
    int multiplicity;
};

/**
 * Bounded Knapsack with Binary Power Splitting: O(W * sum(log count_i)) Time.
 * Decomposes count_i into powers {1, 2, 4, ..., remainder}.
 */
int knapsack_bounded_binary_split(
    const std::vector<int>& weight,
    const std::vector<int>& value,
    const std::vector<int>& count,
    int W) {

    std::vector<std::pair<int, int>> expanded;

    for (size_t i = 0; i < weight.size(); ++i) {
        int k = count[i];
        int power = 1;
        while (power <= k) {
            expanded.push_back({weight[i] * power, value[i] * power});
            k -= power;
            power <<= 1;
        }
        if (k > 0) {
            expanded.push_back({weight[i] * k, value[i] * k});
        }
    }

    std::vector<int> dp(W + 1, 0);
    for (const auto& [item_w, item_v] : expanded) {
        for (int c = W; c >= item_w; --c) {
            dp[c] = std::max(dp[c], dp[c - item_w] + item_v);
        }
    }

    return dp[W];
}

/**
 * Bounded Knapsack Multiplicity Reconstruction.
 * Returns {max_value, counts} where counts[i] is the number of copies chosen of item i.
 */
std::pair<int, std::vector<int>> knapsack_bounded_reconstruct(
    const std::vector<int>& weight,
    const std::vector<int>& value,
    const std::vector<int>& count,
    int W) {

    std::vector<Bundle> expanded;
    int n = static_cast<int>(weight.size());

    for (int i = 0; i < n; ++i) {
        int k = count[i];
        int power = 1;
        while (power <= k) {
            expanded.push_back({weight[i] * power, value[i] * power, i, power});
            k -= power;
            power <<= 1;
        }
        if (k > 0) {
            expanded.push_back({weight[i] * k, value[i] * k, i, k});
        }
    }

    int m = static_cast<int>(expanded.size());
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(W + 1, 0));

    for (int i = 1; i <= m; ++i) {
        int ew = expanded[i - 1].w;
        int ev = expanded[i - 1].v;
        for (int c = 0; c <= W; ++c) {
            dp[i][c] = dp[i - 1][c];
            if (ew <= c) {
                dp[i][c] = std::max(dp[i][c], dp[i - 1][c - ew] + ev);
            }
        }
    }

    std::vector<int> used(n, 0);
    int c = W;
    for (int i = m; i >= 1; --i) {
        if (dp[i][c] != dp[i - 1][c]) {
            used[expanded[i - 1].original_idx] += expanded[i - 1].multiplicity;
            c -= expanded[i - 1].w;
        }
    }

    return {dp[m][W], used};
}

// ============================================================================
// 4. Knapsack Variations: Exact-Fill & Subset Sum Feasibility
// ============================================================================

/**
 * Exact-Fill 0-1 Knapsack: Returns max value achieving exactly weight W,
 * or -1 if impossible. Uses -INF sentinels.
 */
int knapsack_01_exact_fill(const std::vector<int>& weight, const std::vector<int>& value, int W) {
    constexpr int NEG_INF = -1000000000;
    std::vector<int> dp(W + 1, NEG_INF);
    dp[0] = 0;

    for (size_t i = 0; i < weight.size(); ++i) {
        int w = weight[i];
        int v = value[i];
        for (int c = W; c >= w; --c) {
            if (dp[c - w] != NEG_INF) {
                dp[c] = std::max(dp[c], dp[c - w] + v);
            }
        }
    }

    return (dp[W] >= 0) ? dp[W] : -1;
}

/**
 * Subset Sum Feasibility: Returns true if any subset sums exactly to target S.
 */
bool subset_sum_feasible(const std::vector<int>& nums, int target) {
    if (target < 0) return false;
    std::vector<bool> dp(target + 1, false);
    dp[0] = true;

    for (int x : nums) {
        for (int s = target; s >= x; --s) {
            if (dp[s - x]) {
                dp[s] = true;
            }
        }
    }

    return dp[target];
}

} // namespace knapsack

// ============================================================================
// Comprehensive Unit Verification
// ============================================================================

int main() {
    using namespace knapsack;

    // Test 1: Classical 0-1 Knapsack
    // Items: (w=2, v=3), (w=3, v=4), (w=4, v=5), (w=5, v=6), W=5
    // Optimal: items 0 & 1 (w=2+3=5, v=3+4=7)
    std::vector<int> w1 = {2, 3, 4, 5};
    std::vector<int> v1 = {3, 4, 5, 6};
    int W1 = 5;

    assert(knapsack_01_2d(w1, v1, W1) == 7);
    assert(knapsack_01_1d(w1, v1, W1) == 7);

    auto [val1, items1] = knapsack_01_reconstruct(w1, v1, W1);
    assert(val1 == 7);
    assert(items1.size() == 2);
    assert(items1[0] == 0 && items1[1] == 1);

    // Test 2: Greedy Failure Case for 0-1 Knapsack
    // Items: (w=6, v=10, ratio=1.67), (w=5, v=8, ratio=1.60), (w=5, v=8, ratio=1.60), W=10
    // Greedy ratio picks (w=6, v=10), remaining capacity 4 -> cannot fit anything else. Total: 10
    // Optimal: pick both w=5 items: total weight 10, total value 16!
    std::vector<int> w_greedy = {6, 5, 5};
    std::vector<int> v_greedy = {10, 8, 8};
    assert(knapsack_01_1d(w_greedy, v_greedy, 10) == 16);

    // Test 3: Unbounded Knapsack
    // Single item w=3, v=5, W=9 -> 0-1 gives 5, unbounded gives 15 (3 copies)
    std::vector<int> w_single = {3};
    std::vector<int> v_single = {5};
    assert(knapsack_01_1d(w_single, v_single, 9) == 5);
    assert(knapsack_unbounded(w_single, v_single, 9) == 15);

    auto [unb_val, unb_items] = knapsack_unbounded_reconstruct(w_single, v_single, 9);
    assert(unb_val == 15);
    assert(unb_items.size() == 3);
    assert(unb_items[0] == 0 && unb_items[1] == 0 && unb_items[2] == 0);

    // Test 4: Bounded Knapsack with Binary Power Splitting
    // Item: w=3, v=5, count=2, W=9 -> can take at most 2 copies -> value 10
    std::vector<int> counts_single = {2};
    assert(knapsack_bounded_binary_split(w_single, v_single, counts_single, 9) == 10);

    auto [bnd_val, bnd_used] = knapsack_bounded_reconstruct(w_single, v_single, counts_single, 9);
    assert(bnd_val == 10);
    assert(bnd_used[0] == 2);

    // Complex bounded knapsack test
    // Items:
    // 0: w=2, v=3, count=3 (take up to 3)
    // 1: w=3, v=4, count=2 (take up to 2)
    // 2: w=4, v=6, count=1 (take up to 1)
    // W = 8
    std::vector<int> w_bnd = {2, 3, 4};
    std::vector<int> v_bnd = {3, 4, 6};
    std::vector<int> c_bnd = {3, 2, 1};
    int W_bnd = 8;
    int bnd_res = knapsack_bounded_binary_split(w_bnd, v_bnd, c_bnd, W_bnd);
    auto [rec_bnd_val, rec_bnd_counts] = knapsack_bounded_reconstruct(w_bnd, v_bnd, c_bnd, W_bnd);
    assert(bnd_res == rec_bnd_val);

    // Validate weight and value from reconstructed counts
    int total_w = 0, total_v = 0;
    for (size_t i = 0; i < w_bnd.size(); ++i) {
        assert(rec_bnd_counts[i] <= c_bnd[i]);
        total_w += rec_bnd_counts[i] * w_bnd[i];
        total_v += rec_bnd_counts[i] * v_bnd[i];
    }
    assert(total_w <= W_bnd);
    assert(total_v == bnd_res);

    // Test 5: Exact-Fill 0-1 Knapsack
    // Items: w={3, 4, 7}, v={10, 15, 30}
    // Target W=7: can form by {7} (v=30) or {3,4} (v=25) -> max 30
    // Target W=8: impossible -> -1
    std::vector<int> w_exact = {3, 4, 7};
    std::vector<int> v_exact = {10, 15, 30};
    assert(knapsack_01_exact_fill(w_exact, v_exact, 7) == 30);
    assert(knapsack_01_exact_fill(w_exact, v_exact, 8) == -1);

    // Test 6: Subset Sum Feasibility
    std::vector<int> nums = {3, 34, 4, 12, 5, 2};
    assert(subset_sum_feasible(nums, 9) == true);  // 4 + 5 = 9
    assert(subset_sum_feasible(nums, 30) == false);
    assert(subset_sum_feasible(nums, 0) == true);

    // Test 7: Zero capacity / empty items
    assert(knapsack_01_1d({}, {}, 10) == 0);
    assert(knapsack_01_1d({1, 2}, {10, 20}, 0) == 0);

    std::cout << "[PASS] All Knapsack Family C++ unit tests passed." << std::endl;
    return 0;
}
