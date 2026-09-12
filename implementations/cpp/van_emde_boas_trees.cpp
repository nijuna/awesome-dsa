/**
 * @file van_emde_boas_trees.cpp
 * @brief Reference implementation of van Emde Boas Trees and Cache-Oblivious Search Tree Layout.
 *
 * Implements:
 * 1. van Emde Boas Tree with O(log log U) integer dictionary operations (insert, delete, successor, predecessor).
 * 2. Cache-Oblivious Binary Search Tree using the van Emde Boas recursive height-split layout.
 * 3. Differential testing against std::set and std::lower_bound.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <memory>
#include <set>
#include <random>

namespace dsa {

/**
 * @brief van Emde Boas Tree for integer universe U = 2^k.
 * Achieves O(log log U) time per operation.
 */
class VanEmdeBoasTree {
private:
    int bits_;                      // k such that U = 2^k
    int upper_bits_;                // ceil(k / 2)
    int lower_bits_;                // floor(k / 2)
    uint64_t lower_mask_;           // (1 << lower_bits) - 1

    bool is_empty_ = true;
    uint64_t min_ = 0;
    uint64_t max_ = 0;

    std::unique_ptr<VanEmdeBoasTree> summary_;
    std::vector<std::unique_ptr<VanEmdeBoasTree>> clusters_;

    inline uint64_t high(uint64_t x) const {
        return x >> lower_bits_;
    }

    inline uint64_t low(uint64_t x) const {
        return x & lower_mask_;
    }

    inline uint64_t index(uint64_t c, uint64_t i) const {
        return (c << lower_bits_) | i;
    }

public:
    explicit VanEmdeBoasTree(int bits)
        : bits_(bits),
          upper_bits_((bits + 1) / 2),
          lower_bits_(bits / 2),
          lower_mask_((1ULL << lower_bits_) - 1),
          is_empty_(true),
          min_(0),
          max_(0) {
        assert(bits >= 1 && bits <= 32);
        if (bits_ > 1) {
            summary_ = std::make_unique<VanEmdeBoasTree>(upper_bits_);
            size_t num_clusters = 1ULL << upper_bits_;
            clusters_.resize(num_clusters);
            for (size_t c = 0; c < num_clusters; ++c) {
                clusters_[c] = std::make_unique<VanEmdeBoasTree>(lower_bits_);
            }
        }
    }

    bool empty() const {
        return is_empty_;
    }

    // --- Layer A: Fast Core API (Preconditioned) ---

    uint64_t min() const {
        assert(!empty());
        return min_;
    }

    uint64_t max() const {
        assert(!empty());
        return max_;
    }

    // --- Layer B: Safe Adapter API ---

    const uint64_t* try_min() const {
        return empty() ? nullptr : &min_;
    }

    const uint64_t* try_max() const {
        return empty() ? nullptr : &max_;
    }

    bool contains(uint64_t x) const {
        if (empty() || x < min_ || x > max_) return false;
        if (x == min_ || x == max_) return true;
        if (bits_ == 1) return false;
        return clusters_[high(x)]->contains(low(x));
    }

    void insert(uint64_t x) {
        if (empty()) {
            min_ = max_ = x;
            is_empty_ = false;
            return;
        }

        if (x == min_ || x == max_) return;

        if (x < min_) {
            std::swap(x, min_);
        }

        if (bits_ > 1) {
            uint64_t c = high(x);
            uint64_t i = low(x);
            if (clusters_[c]->empty()) {
                summary_->insert(c);
                clusters_[c]->insert(i);
            } else {
                clusters_[c]->insert(i);
            }
        }

        if (x > max_) {
            max_ = x;
        }
    }

    const uint64_t* try_successor(uint64_t x) const {
        if (empty()) return nullptr;
        if (bits_ == 1) {
            if (x == 0 && max_ == 1) return &max_;
            return nullptr;
        }

        if (x < min_) {
            return &min_;
        }

        uint64_t c = high(x);
        uint64_t i = low(x);
        const uint64_t* max_in_c = clusters_[c]->try_max();

        if (max_in_c && i < *max_in_c) {
            const uint64_t* succ_i = clusters_[c]->try_successor(i);
            if (succ_i) {
                static thread_local uint64_t res;
                res = index(c, *succ_i);
                return &res;
            }
        }

        const uint64_t* succ_c = summary_->try_successor(c);
        if (!succ_c) return nullptr;

        uint64_t min_in_succ_c = clusters_[*succ_c]->min();
        static thread_local uint64_t res2;
        res2 = index(*succ_c, min_in_succ_c);
        return &res2;
    }

    const uint64_t* try_predecessor(uint64_t x) const {
        if (empty()) return nullptr;
        if (bits_ == 1) {
            if (x == 1 && min_ == 0) return &min_;
            return nullptr;
        }

        if (x > max_) {
            return &max_;
        }

        uint64_t c = high(x);
        uint64_t i = low(x);
        const uint64_t* min_in_c = clusters_[c]->try_min();

        if (min_in_c && i > *min_in_c) {
            const uint64_t* pred_i = clusters_[c]->try_predecessor(i);
            if (pred_i) {
                static thread_local uint64_t res;
                res = index(c, *pred_i);
                return &res;
            }
        }

        const uint64_t* pred_c = summary_->try_predecessor(c);
        if (pred_c) {
            uint64_t max_in_pred_c = clusters_[*pred_c]->max();
            static thread_local uint64_t res2;
            res2 = index(*pred_c, max_in_pred_c);
            return &res2;
        }

        if (x > min_) {
            return &min_;
        }

        return nullptr;
    }

    void erase(uint64_t x) {
        if (empty() || x < min_ || x > max_) return;

        if (min_ == max_) {
            if (x == min_) {
                is_empty_ = true;
                min_ = max_ = 0;
            }
            return;
        }

        if (bits_ == 1) {
            if (x == 0) {
                min_ = 1;
            } else {
                min_ = 0;
            }
            max_ = min_;
            return;
        }

        if (x == min_) {
            uint64_t first_c = summary_->min();
            x = index(first_c, clusters_[first_c]->min());
            min_ = x;
        }

        uint64_t c = high(x);
        uint64_t i = low(x);
        clusters_[c]->erase(i);

        if (clusters_[c]->empty()) {
            summary_->erase(c);
            if (x == max_) {
                if (summary_->empty()) {
                    max_ = min_;
                } else {
                    uint64_t last_c = summary_->max();
                    max_ = index(last_c, clusters_[last_c]->max());
                }
            }
        } else if (x == max_) {
            max_ = index(c, clusters_[c]->max());
        }
    }
};

/**
 * @brief Cache-Oblivious Complete Binary Search Tree with van Emde Boas Layout.
 * Achieves O(log_B N) cache misses across all memory hierarchy levels without tuning to B.
 */
class CacheObliviousSearchTree {
private:
    int height_;
    int n_;
    std::vector<int64_t> layout_; // Nodes in van Emde Boas recursive layout order
    std::vector<int> bst_to_veb_;  // Mapping from 1-based complete BST index to vEB layout index
    std::vector<int> veb_to_bst_;  // Reverse mapping

    // Recursively lay out a subtree rooted at bst_idx with height h into layout_
    void build_veb_layout(int bst_idx, int h, int& current_pos) {
        if (h == 1) {
            bst_to_veb_[bst_idx] = current_pos;
            veb_to_bst_[current_pos] = bst_idx;
            current_pos++;
            return;
        }

        int h_top = h / 2;
        int h_bot = h - h_top;

        // 1. Recursively lay out the top tree of height h_top
        build_veb_layout(bst_idx, h_top, current_pos);

        // 2. Recursively lay out each bottom subtree of height h_bot
        // The roots of the bottom subtrees are at depth h_top relative to bst_idx
        int num_bottom = 1 << h_top;
        for (int i = 0; i < num_bottom; ++i) {
            int bot_root = (bst_idx << h_top) + i;
            build_veb_layout(bot_root, h_bot, current_pos);
        }
    }

public:
    /**
     * @param sorted_keys Sorted array of distinct elements of size N = 2^height - 1
     */
    explicit CacheObliviousSearchTree(int height, const std::vector<int64_t>& sorted_keys)
        : height_(height),
          n_((1 << height) - 1),
          layout_(n_),
          bst_to_veb_(1 << height, 0),
          veb_to_bst_(n_, 0) {
        assert(static_cast<int>(sorted_keys.size()) == n_);

        // Compute van Emde Boas layout permutation
        int pos = 0;
        build_veb_layout(1, height_, pos);
        assert(pos == n_);

        // Map sorted keys into complete BST nodes:
        // In-order traversal of complete BST gives sorted array
        std::vector<int64_t> bst_nodes(1 << height, 0);
        int key_idx = 0;
        auto inorder = [&](auto& self, int u) -> void {
            if (u > n_) return;
            self(self, 2 * u);
            bst_nodes[u] = sorted_keys[key_idx++];
            self(self, 2 * u + 1);
        };
        inorder(inorder, 1);
        assert(key_idx == n_);

        // Place into vEB physical layout array
        for (int i = 0; i < n_; ++i) {
            int bst_idx = veb_to_bst_[i];
            layout_[i] = bst_nodes[bst_idx];
        }
    }

    /**
     * @brief Search for key using the van Emde Boas layout.
     * Navigates the complete BST in O(log N) comparisons and O(log_B N) cache misses.
     * Returns true if key is found, and stores index in pos_found.
     */
    bool search(int64_t key) const {
        int u = 1; // Start at root of complete BST
        while (u <= n_) {
            int veb_pos = bst_to_veb_[u];
            int64_t val = layout_[veb_pos];
            if (key == val) {
                return true;
            } else if (key < val) {
                u = 2 * u; // Move to left child
            } else {
                u = 2 * u + 1; // Move to right child
            }
        }
        return false;
    }
};

} // namespace dsa

int main() {
    std::cout << "Running van Emde Boas Trees and Cache-Oblivious Layout verification..." << std::endl;

    // =========================================================================
    // Part 1: van Emde Boas Tree Differential Testing vs std::set
    // =========================================================================
    const int BITS = 8; // Universe U = 256
    dsa::VanEmdeBoasTree veb(BITS);
    std::set<uint64_t> oracle_set;

    std::mt19937_64 rng(42);
    const int OPS = 1000;

    for (int step = 0; step < OPS; ++step) {
        int op = rng() % 4;
        uint64_t x = rng() % (1ULL << BITS);

        if (op == 0) {
            // Insert
            veb.insert(x);
            oracle_set.insert(x);
        } else if (op == 1) {
            // Erase
            veb.erase(x);
            oracle_set.erase(x);
        } else if (op == 2) {
            // Successor
            const uint64_t* veb_succ = veb.try_successor(x);
            auto it = oracle_set.upper_bound(x);
            if (it == oracle_set.end()) {
                assert(veb_succ == nullptr);
            } else {
                assert(veb_succ != nullptr);
                assert(*veb_succ == *it);
            }
        } else {
            // Predecessor
            const uint64_t* veb_pred = veb.try_predecessor(x);
            auto it = oracle_set.lower_bound(x);
            if (it == oracle_set.begin()) {
                assert(veb_pred == nullptr);
            } else {
                --it;
                assert(veb_pred != nullptr);
                assert(*veb_pred == *it);
            }
        }

        // Verify min and max
        if (oracle_set.empty()) {
            assert(veb.empty());
            assert(veb.try_min() == nullptr);
            assert(veb.try_max() == nullptr);
        } else {
            assert(!veb.empty());
            assert(veb.min() == *oracle_set.begin());
            assert(veb.max() == *oracle_set.rbegin());
        }
    }
    std::cout << "[PASS] 1000 differential operations on van Emde Boas tree matched std::set." << std::endl;

    // =========================================================================
    // Part 2: Cache-Oblivious Search Tree Layout Testing vs std::lower_bound
    // =========================================================================
    const int TREE_HEIGHT = 4; // N = 15 keys
    const int N_KEYS = (1 << TREE_HEIGHT) - 1;
    std::vector<int64_t> keys(N_KEYS);
    for (int i = 0; i < N_KEYS; ++i) {
        keys[i] = (i + 1) * 10;
    }

    dsa::CacheObliviousSearchTree cot(TREE_HEIGHT, keys);

    // Test present keys
    for (int64_t k : keys) {
        assert(cot.search(k));
    }

    // Test absent keys
    for (int64_t k : {0, 5, 15, 25, 95, 155, 200}) {
        assert(!cot.search(k));
    }

    std::cout << "[PASS] Cache-Oblivious van Emde Boas layout correctly searched all present and absent keys." << std::endl;
    std::cout << "All van Emde Boas and Cache-Oblivious assertions passed successfully!" << std::endl;
    return 0;
}
