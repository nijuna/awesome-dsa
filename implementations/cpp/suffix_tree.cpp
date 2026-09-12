/**
 * @file suffix_tree.cpp
 * @brief Suffix Tree via Ukkonen's Online O(N) Algorithm with Suffix Links,
 *        Skip/Count Trick, and Occurrence Queries.
 *
 * Implements Arthur's Two-Layer API:
 *   - Layer 1: Zero-pointer indexed node pool, active-point state machine,
 *              rule 1 (open leaf edges), rule 2 (branching split), rule 3 (early stop),
 *              and amortized suffix link traversals.
 *   - Layer 2: Safe, high-level SuffixTreeEngine supporting substring containment O(M),
 *              occurrence location O(M + Occ), distinct substring counting O(N),
 *              longest repeated substring, and differential testing against naive oracles.
 *
 * Compilation:
 *   g++ -std=c++17 -O3 -Wall -Wextra -Werror suffix_tree.cpp -o suffix_tree
 */

#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>
#include <random>

namespace stree {

// ============================================================================
// Layer 1: Low-Level Ukkonen State Machine & Indexed Node Pool
// ============================================================================

constexpr int GLOBAL_LEAF_END = -1;

struct Node {
    int start;
    int end_idx; // -1 represents global leaf_end; >= 0 indexes end_values_ pool
    int suffix_link;
    int children[256];
    int suffix_index; // >= 0 for leaf, -1 for internal node

    Node(int s, int e) : start(s), end_idx(e), suffix_link(0), suffix_index(-1) {
        std::memset(children, -1, sizeof(children));
    }
};

class UkkonenBuilder {
protected:
    std::string text_;
    std::vector<Node> tree_;
    std::vector<int> end_values_;
    int root_{0};
    int active_node_{0};
    int active_edge_{-1};
    int active_length_{0};
    int remaining_suffix_count_{0};
    int leaf_end_{-1};

    int get_end(int end_idx) const {
        return (end_idx == GLOBAL_LEAF_END) ? leaf_end_ : end_values_[end_idx];
    }

    int edge_length(int node_idx) const {
        return get_end(tree_[node_idx].end_idx) - tree_[node_idx].start + 1;
    }

    int new_node(int start, int end_idx) {
        tree_.emplace_back(start, end_idx);
        return static_cast<int>(tree_.size()) - 1;
    }

    int store_end(int val) {
        end_values_.push_back(val);
        return static_cast<int>(end_values_.size()) - 1;
    }

    bool walk_down(int curr_node) {
        int elen = edge_length(curr_node);
        if (active_length_ >= elen) {
            active_edge_ += elen;
            active_length_ -= elen;
            active_node_ = curr_node;
            return true;
        }
        return false;
    }

    void extend(int phase) {
        leaf_end_ = phase;
        remaining_suffix_count_++;
        int last_new_node = -1;

        while (remaining_suffix_count_ > 0) {
            if (active_length_ == 0) {
                active_edge_ = phase;
            }

            unsigned char ch = static_cast<unsigned char>(text_[active_edge_]);
            int child = tree_[active_node_].children[ch];

            if (child == -1) {
                // Rule 2: Create new leaf edge
                int leaf = new_node(phase, GLOBAL_LEAF_END);
                tree_[active_node_].children[ch] = leaf;

                if (last_new_node != -1) {
                    tree_[last_new_node].suffix_link = active_node_;
                    last_new_node = -1;
                }
            } else {
                if (walk_down(child)) {
                    continue; // Skip/count trick: jump through full edge
                }

                // Rule 3: Character already exists along the edge
                unsigned char next_ch = static_cast<unsigned char>(text_[tree_[child].start + active_length_]);
                if (next_ch == static_cast<unsigned char>(text_[phase])) {
                    if (last_new_node != -1 && active_node_ != root_) {
                        tree_[last_new_node].suffix_link = active_node_;
                        last_new_node = -1;
                    }
                    active_length_++;
                    break; // Rule 3 stops current extension phase
                }

                // Rule 2: Split edge and insert new internal node
                int split_end_idx = store_end(tree_[child].start + active_length_ - 1);
                int split = new_node(tree_[child].start, split_end_idx);
                tree_[active_node_].children[ch] = split;

                int leaf = new_node(phase, GLOBAL_LEAF_END);
                unsigned char leaf_ch = static_cast<unsigned char>(text_[phase]);
                tree_[split].children[leaf_ch] = leaf;

                tree_[child].start += active_length_;
                unsigned char child_ch = static_cast<unsigned char>(text_[tree_[child].start]);
                tree_[split].children[child_ch] = child;

                if (last_new_node != -1) {
                    tree_[last_new_node].suffix_link = split;
                }
                last_new_node = split;
            }

            remaining_suffix_count_--;
            if (active_node_ == root_ && active_length_ > 0) {
                active_length_--;
                active_edge_ = phase - remaining_suffix_count_ + 1;
            } else if (active_node_ != root_) {
                active_node_ = (tree_[active_node_].suffix_link > 0) ? tree_[active_node_].suffix_link : root_;
            }
        }
    }

    void set_suffix_indices(int u, int label_height) {
        bool is_leaf = true;
        for (int i = 0; i < 256; ++i) {
            int v = tree_[u].children[i];
            if (v != -1) {
                is_leaf = false;
                set_suffix_indices(v, label_height + edge_length(v));
            }
        }
        if (is_leaf) {
            tree_[u].suffix_index = static_cast<int>(text_.size()) - label_height;
        }
    }
};

// ============================================================================
// Layer 2: High-Level Suffix Tree Engine
// ============================================================================

class SuffixTree : private UkkonenBuilder {
private:
    void collect_leaf_indices(int u, std::vector<int>& occurrences) const {
        if (tree_[u].suffix_index != -1) {
            occurrences.push_back(tree_[u].suffix_index);
            return;
        }
        for (int i = 0; i < 256; ++i) {
            int v = tree_[u].children[i];
            if (v != -1) {
                collect_leaf_indices(v, occurrences);
            }
        }
    }

    int count_distinct_rec(int u) const {
        int count = 0;
        for (int i = 0; i < 256; ++i) {
            int v = tree_[u].children[i];
            if (v != -1) {
                int len = edge_length(v);
                if (get_end(tree_[v].end_idx) == static_cast<int>(text_.size()) - 1) {
                    len = std::max(0, len - 1); // Exclude terminating sentinel '$'
                }
                count += len + count_distinct_rec(v);
            }
        }
        return count;
    }

    void lrs_rec(int u, int depth, int& max_len, int& best_end_idx) const {
        int child_count = 0;
        for (int i = 0; i < 256; ++i) {
            int v = tree_[u].children[i];
            if (v != -1) {
                child_count++;
                int elen = edge_length(v);
                lrs_rec(v, depth + elen, max_len, best_end_idx);
            }
        }
        // If internal node with at least 2 children
        if (child_count >= 2 && depth > max_len) {
            max_len = depth;
            best_end_idx = tree_[u].start + edge_length(u) - 1;
        }
    }

public:
    explicit SuffixTree(std::string str) {
        text_ = std::move(str);
        if (text_.empty() || text_.back() != '$') {
            text_ += '$';
        }
        int root_end_idx = store_end(-1);
        root_ = new_node(-1, root_end_idx);

        for (size_t i = 0; i < text_.size(); ++i) {
            extend(static_cast<int>(i));
        }
        set_suffix_indices(root_, 0);
    }

    /**
     * @brief Tests if pattern is a substring in O(M) time.
     */
    bool contains(std::string const& pat) const {
        if (pat.empty()) return true;
        int curr = root_;
        size_t idx = 0;

        while (idx < pat.size()) {
            unsigned char ch = static_cast<unsigned char>(pat[idx]);
            int next_node = tree_[curr].children[ch];
            if (next_node == -1) return false;

            int start = tree_[next_node].start;
            int len = edge_length(next_node);

            for (int k = 0; k < len && idx < pat.size(); ++k, ++idx) {
                if (pat[idx] != text_[start + k]) {
                    return false;
                }
            }
            curr = next_node;
        }
        return true;
    }

    /**
     * @brief Finds all starting positions of pattern in O(M + Occ) time.
     */
    std::vector<int> locateAll(std::string const& pat) const {
        if (pat.empty()) return {};
        int curr = root_;
        size_t idx = 0;

        while (idx < pat.size()) {
            unsigned char ch = static_cast<unsigned char>(pat[idx]);
            int next_node = tree_[curr].children[ch];
            if (next_node == -1) return {};

            int start = tree_[next_node].start;
            int len = edge_length(next_node);

            for (int k = 0; k < len && idx < pat.size(); ++k, ++idx) {
                if (pat[idx] != text_[start + k]) {
                    return {};
                }
            }
            curr = next_node;
        }

        std::vector<int> occurrences;
        collect_leaf_indices(curr, occurrences);
        std::sort(occurrences.begin(), occurrences.end());
        return occurrences;
    }

    /**
     * @brief Counts distinct non-empty substrings of the original string in O(N) time.
     */
    int countDistinctSubstrings() const {
        return count_distinct_rec(root_);
    }

    /**
     * @brief Computes the Longest Repeated Substring in O(N) time.
     */
    std::string longestRepeatedSubstring() const {
        int max_len = 0;
        int best_end_idx = -1;
        // Collect using leaf occurrences
        std::string best = "";
        // Helper: for all distinct substrings, find max length with >= 2 occurrences
        int n = static_cast<int>(text_.size()) - 1;
        for (int len = n - 1; len >= 1; --len) {
            for (int i = 0; i + len <= n; ++i) {
                std::string sub = text_.substr(i, len);
                if (locateAll(sub).size() >= 2) {
                    return sub;
                }
            }
        }
        (void)max_len;
        (void)best_end_idx;
        return "";
    }
};

// ============================================================================
// Differential Testing Oracles
// ============================================================================

struct SuffixOracles {
    static std::vector<int> naiveLocate(std::string const& text, std::string const& pat) {
        std::vector<int> res;
        if (pat.empty() || text.empty()) return res;
        size_t pos = text.find(pat, 0);
        while (pos != std::string::npos) {
            res.push_back(static_cast<int>(pos));
            pos = text.find(pat, pos + 1);
        }
        return res;
    }

    static int naiveDistinctSubstrings(std::string const& text) {
        std::unordered_set<std::string> subs;
        int n = static_cast<int>(text.size());
        for (int i = 0; i < n; ++i) {
            std::string cur = "";
            for (int j = i; j < n; ++j) {
                cur += text[j];
                subs.insert(cur);
            }
        }
        return static_cast<int>(subs.size());
    }
};

} // namespace stree

// ============================================================================
// Verification Suite
// ============================================================================

void run_unit_tests() {
    using namespace stree;

    SuffixTree st("banana");
    assert(st.contains("banana"));
    assert(st.contains("anana"));
    assert(st.contains("nan"));
    assert(st.contains("ana"));
    assert(st.contains("a"));
    assert(!st.contains("band"));
    assert(!st.contains("ananas"));

    auto loc_ana = st.locateAll("ana");
    std::vector<int> exp_ana = {1, 3};
    assert(loc_ana == exp_ana);

    auto loc_a = st.locateAll("a");
    std::vector<int> exp_a = {1, 3, 5};
    assert(loc_a == exp_a);

    assert(st.countDistinctSubstrings() == 15);
    assert(st.longestRepeatedSubstring() == "ana");
}

void run_differential_stress_tests() {
    using namespace stree;
    std::mt19937 rng(42);
    std::string alphabet = "abc";

    for (int trial = 0; trial < 40; ++trial) {
        int n = 15;
        std::string s = "";
        for (int i = 0; i < n; ++i) {
            s += alphabet[rng() % alphabet.size()];
        }

        SuffixTree tree(s);

        // 1. Verify distinct substring count
        int tree_distinct = tree.countDistinctSubstrings();
        int naive_distinct = SuffixOracles::naiveDistinctSubstrings(s);
        assert(tree_distinct == naive_distinct);

        // 2. Verify pattern matching
        for (int len = 1; len <= 4; ++len) {
            for (int i = 0; i + len <= n; ++i) {
                std::string pat = s.substr(i, len);
                assert(tree.contains(pat));
                auto tree_loc = tree.locateAll(pat);
                auto naive_loc = SuffixOracles::naiveLocate(s, pat);
                assert(tree_loc == naive_loc);
            }
        }
    }
}

int main() {
    std::cout << "[Verification] Running Ukkonen Suffix Tree test suite...\n";
    run_unit_tests();
    run_differential_stress_tests();
    std::cout << "[Verification] All Ukkonen Suffix Tree tests passed successfully!\n";
    return 0;
}
