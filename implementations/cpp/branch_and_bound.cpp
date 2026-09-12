/**
 * @file branch_and_bound.cpp
 * @brief Reference implementation of Branch and Bound for 0/1 Knapsack Optimization.
 *
 * Implements Best-First Search Branch and Bound with fractional knapsack bounding,
 * greedy warm starting, and differential verification against an exact DP oracle.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

namespace dsa {

struct KnapsackItem {
    int64_t value = 0;
    int64_t weight = 0;

    KnapsackItem() = default;
    KnapsackItem(int64_t v, int64_t w) : value(v), weight(w) {
        assert(v >= 0 && w >= 0);
    }
};

class BranchAndBoundKnapsack {
private:
    struct InternalItem {
        int64_t value;
        int64_t weight;
        double density;

        bool operator<(const InternalItem& o) const {
            return density > o.density; // Sort descending
        }
    };

    struct SearchNode {
        int level = 0;
        int64_t value = 0;
        int64_t weight = 0;
        double bound = 0.0;

        bool operator<(const SearchNode& o) const {
            return bound < o.bound; // Max-heap: highest bound first
        }
    };

    static double compute_bound(int level, int64_t current_val, int64_t current_weight,
                                int64_t capacity, const std::vector<InternalItem>& items) {
        if (current_weight >= capacity) return 0.0;

        double bound = static_cast<double>(current_val);
        int64_t rem_cap = capacity - current_weight;
        size_t n = items.size();

        for (size_t i = level; i < n; ++i) {
            if (items[i].weight <= rem_cap) {
                rem_cap -= items[i].weight;
                bound += static_cast<double>(items[i].value);
            } else {
                bound += static_cast<double>(rem_cap) * items[i].density;
                break;
            }
        }
        return bound;
    }

public:
    static int64_t solve(const std::vector<KnapsackItem>& raw_items, int64_t capacity) {
        if (capacity <= 0 || raw_items.empty()) return 0;

        // 1. Filter and prepare items sorted by value density
        std::vector<InternalItem> items;
        int64_t total_weight = 0;
        int64_t total_value = 0;

        for (const auto& it : raw_items) {
            if (it.weight == 0) {
                total_value += it.value;
            } else if (it.weight <= capacity && it.value > 0) {
                items.push_back({it.value, it.weight, static_cast<double>(it.value) / it.weight});
                total_weight += it.weight;
            }
        }

        // If all candidate items fit, return sum directly
        if (total_weight <= capacity) {
            for (const auto& it : items) total_value += it.value;
            return total_value;
        }

        std::sort(items.begin(), items.end());

        // 2. Greedy Warm-Start for Incumbent (z*)
        int64_t incumbent = 0;
        int64_t greedy_weight = 0;
        for (const auto& it : items) {
            if (greedy_weight + it.weight <= capacity) {
                greedy_weight += it.weight;
                incumbent += it.value;
            }
        }

        // 3. Best-First Search Priority Queue
        std::priority_queue<SearchNode> pq;
        SearchNode root;
        root.level = 0;
        root.value = 0;
        root.weight = 0;
        root.bound = compute_bound(0, 0, 0, capacity, items);

        if (root.bound > incumbent) {
            pq.push(root);
        }

        int n = static_cast<int>(items.size());

        while (!pq.empty()) {
            SearchNode curr = pq.top();
            pq.pop();

            // Pruning check: if optimistic bound cannot beat incumbent, discard
            if (curr.bound <= static_cast<double>(incumbent)) {
                continue;
            }

            if (curr.level == n) {
                continue;
            }

            const auto& next_item = items[curr.level];

            // Branch 1: Include item (if it fits)
            if (curr.weight + next_item.weight <= capacity) {
                SearchNode left;
                left.level = curr.level + 1;
                left.value = curr.value + next_item.value;
                left.weight = curr.weight + next_item.weight;
                left.bound = compute_bound(left.level, left.value, left.weight, capacity, items);

                if (left.value > incumbent) {
                    incumbent = left.value;
                }
                if (left.bound > static_cast<double>(incumbent)) {
                    pq.push(left);
                }
            }

            // Branch 2: Exclude item
            SearchNode right;
            right.level = curr.level + 1;
            right.value = curr.value;
            right.weight = curr.weight;
            right.bound = compute_bound(right.level, right.value, right.weight, capacity, items);

            if (right.bound > static_cast<double>(incumbent)) {
                pq.push(right);
            }
        }

        return incumbent + total_value;
    }
};

/**
 * @brief Classical exact 0/1 Knapsack Dynamic Programming Oracle.
 */
class DynamicProgrammingOracle {
public:
    static int64_t solve(const std::vector<KnapsackItem>& items, int64_t capacity) {
        if (capacity <= 0 || items.empty()) return 0;

        int64_t zero_weight_val = 0;
        std::vector<KnapsackItem> filtered;
        for (const auto& it : items) {
            if (it.weight == 0) zero_weight_val += it.value;
            else if (it.weight <= capacity) filtered.push_back(it);
        }

        std::vector<int64_t> dp(capacity + 1, 0);

        for (const auto& it : filtered) {
            for (int64_t w = capacity; w >= it.weight; --w) {
                dp[w] = std::max(dp[w], dp[w - it.weight] + it.value);
            }
        }

        return dp[capacity] + zero_weight_val;
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Branch and Bound verification..." << std::endl;

    // 1. Classical Textbook Instance
    // Items: (v=40, w=2), (v=42, w=5), (v=25, w=7), (v=12, w=3) with W = 10
    std::vector<dsa::KnapsackItem> textbook = {
        {40, 2}, {42, 5}, {25, 7}, {12, 3}
    };
    int64_t cap1 = 10;
    int64_t bnb_res1 = dsa::BranchAndBoundKnapsack::solve(textbook, cap1);
    int64_t dp_res1 = dsa::DynamicProgrammingOracle::solve(textbook, cap1);

    assert(bnb_res1 == dp_res1);
    assert(bnb_res1 == 94); // items 1, 2, 4 -> 40 + 42 + 12 = 94, weight 2 + 5 + 3 = 10

    // 2. Randomized Differential Testing vs Exact DP Oracle
    std::mt19937 rng(42);
    const int TRIALS = 50;

    for (int t = 0; t < TRIALS; ++t) {
        int n = 15;
        int64_t capacity = (rng() % 150) + 20;

        std::vector<dsa::KnapsackItem> items;
        for (int i = 0; i < n; ++i) {
            int64_t v = (rng() % 50) + 1;
            int64_t w = (rng() % 30) + 1;
            items.emplace_back(v, w);
        }

        int64_t bnb_ans = dsa::BranchAndBoundKnapsack::solve(items, capacity);
        int64_t dp_ans = dsa::DynamicProgrammingOracle::solve(items, capacity);

        assert(bnb_ans == dp_ans);
    }

    // 3. Edge Cases
    assert(dsa::BranchAndBoundKnapsack::solve({}, 100) == 0);
    assert(dsa::BranchAndBoundKnapsack::solve(textbook, 0) == 0);

    std::cout << "[PASS] Textbook instance matched exact optimal value: 94." << std::endl;
    std::cout << "[PASS] 50 randomized instances matched exact Dynamic Programming oracle." << std::endl;
    std::cout << "All Branch and Bound assertions passed successfully!" << std::endl;
    return 0;
}
