/**
 * Reference Implementation: Greedy Algorithms & Paradigm Mechanics
 * Demonstrates:
 * 1. Interval Scheduling (Earliest Finish Time Heuristic, O(N log N)).
 * 2. Fractional Knapsack (Value-to-Weight Density Greedy Choice, O(N log N)).
 * 3. Huffman Coding (Prefix-Free Tree Construction via Min-Heap, O(N log N)).
 * 4. Coin Change Analysis (Greedy Optimality on Canonical Systems vs. Failure Counterexamples).
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <string>
#include <map>
#include <cmath>
#include <cassert>

// ============================================================================
// 1. Interval Scheduling (Earliest Finish Time)
// ============================================================================
struct Interval {
    int id;
    int start;
    int finish;
};

std::vector<Interval> interval_scheduling(std::vector<Interval> intervals) {
    // Greedy choice: sort by finish time ascending
    std::sort(intervals.begin(), intervals.end(), [](const Interval& a, const Interval& b) {
        if (a.finish != b.finish) return a.finish < b.finish;
        return a.start < b.start;
    });

    std::vector<Interval> chosen;
    int current_finish = -1;

    for (const auto& interval : intervals) {
        if (interval.start >= current_finish) {
            chosen.push_back(interval);
            current_finish = interval.finish;
        }
    }

    return chosen;
}

// ============================================================================
// 2. Fractional Knapsack (Density Sorting)
// ============================================================================
struct KnapsackItem {
    int id;
    double value;
    double weight;
};

struct FractionalResult {
    double total_value;
    std::vector<std::pair<int, double>> taken; // {item_id, fraction}
};

FractionalResult fractional_knapsack(double capacity, std::vector<KnapsackItem> items) {
    // Greedy choice: sort by value-to-weight ratio descending
    std::sort(items.begin(), items.end(), [](const KnapsackItem& a, const KnapsackItem& b) {
        return (a.value / a.weight) > (b.value / b.weight);
    });

    double total_value = 0.0;
    double remaining = capacity;
    std::vector<std::pair<int, double>> taken;

    for (const auto& item : items) {
        if (remaining <= 0.0) break;

        if (item.weight <= remaining) {
            // Take item completely
            total_value += item.value;
            remaining -= item.weight;
            taken.push_back({item.id, 1.0});
        } else {
            // Take fractional portion
            double fraction = remaining / item.weight;
            total_value += item.value * fraction;
            taken.push_back({item.id, fraction});
            remaining = 0.0;
        }
    }

    return {total_value, taken};
}

// ============================================================================
// 3. Huffman Coding (Bottom-Up Min-Heap Tree Construction)
// ============================================================================
struct HuffmanNode {
    char symbol;
    int freq;
    HuffmanNode* left;
    HuffmanNode* right;

    HuffmanNode(char s, int f) : symbol(s), freq(f), left(nullptr), right(nullptr) {}
    HuffmanNode(int f, HuffmanNode* l, HuffmanNode* r)
        : symbol('\0'), freq(f), left(l), right(r) {}
};

struct HuffmanNodeComparator {
    bool operator()(const HuffmanNode* a, const HuffmanNode* b) const {
        return a->freq > b->freq; // Min-heap ordered by frequency
    }
};

class HuffmanEncoder {
public:
    HuffmanNode* root;
    std::map<char, std::string> code_table;

    explicit HuffmanEncoder(const std::map<char, int>& frequencies) : root(nullptr) {
        build_tree(frequencies);
        if (root) {
            std::string prefix = "";
            generate_codes(root, prefix);
        }
    }

    ~HuffmanEncoder() {
        free_tree(root);
    }

private:
    void build_tree(const std::map<char, int>& frequencies) {
        std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, HuffmanNodeComparator> pq;

        for (const auto& [symbol, freq] : frequencies) {
            pq.push(new HuffmanNode(symbol, freq));
        }

        if (pq.empty()) return;

        // Special case: single unique symbol
        if (pq.size() == 1) {
            HuffmanNode* single = pq.top();
            pq.pop();
            root = new HuffmanNode(single->freq, single, nullptr);
            return;
        }

        while (pq.size() > 1) {
            HuffmanNode* left = pq.top();
            pq.pop();
            HuffmanNode* right = pq.top();
            pq.pop();

            HuffmanNode* parent = new HuffmanNode(left->freq + right->freq, left, right);
            pq.push(parent);
        }

        root = pq.top();
    }

    void generate_codes(HuffmanNode* node, const std::string& prefix) {
        if (!node) return;

        if (!node->left && !node->right) {
            code_table[node->symbol] = prefix.empty() ? "0" : prefix;
            return;
        }

        generate_codes(node->left, prefix + "0");
        generate_codes(node->right, prefix + "1");
    }

    void free_tree(HuffmanNode* node) {
        if (!node) return;
        free_tree(node->left);
        free_tree(node->right);
        delete node;
    }
};

// ============================================================================
// 4. Coin Change: Canonical Greedy vs. Counterexample Demonstration
// ============================================================================
int greedy_coin_change(int amount, const std::vector<int>& denominations) {
    // Assumes denominations are sorted in descending order
    int count = 0;
    int remaining = amount;
    for (int coin : denominations) {
        if (remaining <= 0) break;
        int take = remaining / coin;
        count += take;
        remaining -= take * coin;
    }
    return (remaining == 0) ? count : -1;
}

// ============================================================================
// Unit Tests & Invariant Verification
// ============================================================================
int main() {
    // ------------------------------------------------------------------------
    // Test 1: Interval Scheduling Verification
    // ------------------------------------------------------------------------
    {
        std::vector<Interval> intervals = {
            {1, 1, 4},
            {2, 3, 5},
            {3, 0, 6},
            {4, 5, 7},
            {5, 3, 9},
            {6, 5, 9},
            {7, 6, 10},
            {8, 8, 11},
            {9, 8, 12},
            {10, 2, 14},
            {11, 12, 16}
        };

        auto chosen = interval_scheduling(intervals);

        // Optimal earliest finish selection: {1-4}, {5-7}, {8-11}, {12-16} => 4 intervals
        assert(chosen.size() == 4);
        assert(chosen[0].id == 1 && chosen[0].finish == 4);
        assert(chosen[1].id == 4 && chosen[1].finish == 7);
        assert(chosen[2].id == 8 && chosen[2].finish == 11);
        assert(chosen[3].id == 11 && chosen[3].finish == 16);

        // Verify mutual disjointness
        for (size_t i = 1; i < chosen.size(); ++i) {
            assert(chosen[i].start >= chosen[i - 1].finish);
        }
    }

    // ------------------------------------------------------------------------
    // Test 2: Fractional Knapsack Verification
    // ------------------------------------------------------------------------
    {
        std::vector<KnapsackItem> items = {
            {1, 60.0, 10.0},  // density = 6.0
            {2, 100.0, 20.0}, // density = 5.0
            {3, 120.0, 30.0}  // density = 4.0
        };
        double capacity = 50.0;

        auto res = fractional_knapsack(capacity, items);

        // Take item 1 fully (10 kg, $60), item 2 fully (20 kg, $100), item 3 (20/30 = 2/3, $80)
        // Total value = 60 + 100 + 80 = 240.0
        assert(std::abs(res.total_value - 240.0) < 1e-6);
        assert(res.taken.size() == 3);
        assert(res.taken[0].first == 1 && std::abs(res.taken[0].second - 1.0) < 1e-6);
        assert(res.taken[1].first == 2 && std::abs(res.taken[1].second - 1.0) < 1e-6);
        assert(res.taken[2].first == 3 && std::abs(res.taken[2].second - 2.0 / 3.0) < 1e-6);
    }

    // ------------------------------------------------------------------------
    // Test 3: Huffman Coding Prefix-Free Tree Verification
    // Arthur's Example: A:5, B:9, C:12, D:13, E:16, F:45
    // ------------------------------------------------------------------------
    {
        std::map<char, int> freqs = {
            {'A', 5}, {'B', 9}, {'C', 12}, {'D', 13}, {'E', 16}, {'F', 45}
        };

        HuffmanEncoder encoder(freqs);
        const auto& codes = encoder.code_table;

        assert(codes.size() == 6);

        // Verify Prefix-Free Property: no codeword is a prefix of another
        for (const auto& [s1, c1] : codes) {
            for (const auto& [s2, c2] : codes) {
                if (s1 != s2) {
                    assert(c1.find(c2) != 0); // c2 is not a prefix of c1
                    assert(c2.find(c1) != 0); // c1 is not a prefix of c2
                }
            }
        }

        // Most frequent character 'F' (freq 45) must receive a shorter code than least frequent 'A' (freq 5)
        assert(codes.at('F').length() < codes.at('A').length());
    }

    // ------------------------------------------------------------------------
    // Test 4: Coin Change Canonical Optimality vs. Non-Canonical Failure
    // ------------------------------------------------------------------------
    {
        // Canonical US currency: {25, 10, 5, 1}
        std::vector<int> us_coins = {25, 10, 5, 1};
        int coins_63 = greedy_coin_change(63, us_coins);
        // 63 = 25*2 + 10*1 + 1*3 => 2 + 1 + 3 = 6 coins
        assert(coins_63 == 6);

        // Counterexample: non-canonical currency {4, 3, 1} for amount 6
        std::vector<int> non_canonical = {4, 3, 1};
        int greedy_res = greedy_coin_change(6, non_canonical);
        // Greedy takes 4, remaining 2, takes 1, 1 => 4 + 1 + 1 = 3 coins
        assert(greedy_res == 3);
        // But optimal is 3 + 3 = 2 coins! Greedy fails.
    }

    std::cout << "[PASS] All Greedy Algorithms C++ unit tests passed.\n";
    return 0;
}
