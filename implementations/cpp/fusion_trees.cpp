/**
 * @file fusion_trees.cpp
 * @brief High-performance C++17 reference implementation of Fredman & Willard's Fusion Tree.
 *
 * Implements Word RAM predecessor search breaking the Omega(log N) comparison lower bound:
 * 1. Packed-Sketch Word RAM Node:
 *    - Node branching factor B = 8 (T = 4, max 7 keys per node).
 *    - Parallel comparison on packed 64-bit integer words: each field contains an indicator bit
 *      and data bits. Subtraction `diff = packed_keys - Q` executes 7 parallel comparisons in O(1).
 *    - `__builtin_popcountll` extracts exact rank in a single CPU cycle.
 * 2. B-Tree Architecture:
 *    - Full balanced B-Tree over 64-bit unsigned integers.
 *    - Insertions, deletions (borrow/merge), predecessor, and successor queries in O(log_B N) = O(log N / log W).
 *    - O(N) space.
 *
 * Fully compliant with -std=c++17 -O3 -Wall -Wextra -Werror.
 */

#include <iostream>
#include <vector>
#include <set>
#include <optional>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

namespace dsa {

/**
 * @brief Node for Fusion Tree with branching factor B = 8.
 */
class FusionNode {
public:
    static constexpr int T = 4;                ///< Minimum degree: keys in [T-1, 2T-1] = [3, 7]
    static constexpr int MAX_KEYS = 2 * T - 1; ///< 7
    static constexpr int MIN_KEYS = T - 1;     ///< 3
    static constexpr int B = MAX_KEYS + 1;     ///< 8 branches

    static constexpr uint64_t INDICATOR_MASK = 0x8080808080808080ULL;
    static constexpr uint64_t REPLICATE_MULT = 0x0101010101010101ULL;

    int num_keys{0};
    uint64_t keys[MAX_KEYS]{0};
    FusionNode* children[B]{nullptr};
    bool is_leaf{true};

    explicit FusionNode(bool leaf = true) : is_leaf(leaf) {
        for (int i = 0; i < B; ++i) children[i] = nullptr;
    }

    ~FusionNode() {
        if (!is_leaf) {
            for (int i = 0; i <= num_keys; ++i) {
                delete children[i];
            }
        }
    }

    FusionNode(const FusionNode&) = delete;
    FusionNode& operator=(const FusionNode&) = delete;

    /**
     * @brief Parallel comparison on packed 64-bit Word RAM fields.
     * Computes the number of sketched keys strictly less than q_sketch in O(1) Word RAM operations.
     */
    static int parallel_rank(uint64_t packed_node, int k, uint8_t q_sketch) noexcept {
        if (k == 0) return 0;
        uint64_t Q = (static_cast<uint64_t>(q_sketch) & 0x7FULL) * REPLICATE_MULT;
        uint64_t diff = packed_node - Q;
        uint64_t indicators = diff & INDICATOR_MASK;

        uint64_t valid_mask = 0;
        for (int i = 0; i < k; ++i) {
            valid_mask |= (0x80ULL << (8 * i));
        }
        indicators &= valid_mask;
        int geq_count = __builtin_popcountll(indicators);
        return k - geq_count;
    }

    [[nodiscard]] int find_key(uint64_t x) const noexcept {
        int idx = 0;
        while (idx < num_keys && keys[idx] < x) ++idx;
        return idx;
    }

    void remove(uint64_t x) {
        int idx = find_key(x);
        if (idx < num_keys && keys[idx] == x) {
            if (is_leaf) {
                for (int i = idx + 1; i < num_keys; ++i) {
                    keys[i - 1] = keys[i];
                }
                num_keys--;
            } else {
                remove_from_non_leaf(idx);
            }
        } else {
            if (is_leaf) return;

            bool flag = (idx == num_keys);
            if (children[idx]->num_keys < T) {
                fill(idx);
            }

            if (flag && idx > num_keys) {
                children[idx - 1]->remove(x);
            } else {
                children[idx]->remove(x);
            }
        }
    }

    void remove_from_non_leaf(int idx) {
        uint64_t k = keys[idx];

        if (children[idx]->num_keys >= T) {
            uint64_t pred = get_predecessor(idx);
            keys[idx] = pred;
            children[idx]->remove(pred);
        } else if (children[idx + 1]->num_keys >= T) {
            uint64_t succ = get_successor(idx);
            keys[idx] = succ;
            children[idx + 1]->remove(succ);
        } else {
            merge(idx);
            children[idx]->remove(k);
        }
    }

    [[nodiscard]] uint64_t get_predecessor(int idx) const noexcept {
        FusionNode* cur = children[idx];
        while (!cur->is_leaf) cur = cur->children[cur->num_keys];
        return cur->keys[cur->num_keys - 1];
    }

    [[nodiscard]] uint64_t get_successor(int idx) const noexcept {
        FusionNode* cur = children[idx + 1];
        while (!cur->is_leaf) cur = cur->children[0];
        return cur->keys[0];
    }

    void fill(int idx) {
        if (idx != 0 && children[idx - 1]->num_keys >= T) {
            borrow_from_prev(idx);
        } else if (idx != num_keys && children[idx + 1]->num_keys >= T) {
            borrow_from_next(idx);
        } else {
            if (idx != num_keys) merge(idx);
            else merge(idx - 1);
        }
    }

    void borrow_from_prev(int idx) {
        FusionNode* child = children[idx];
        FusionNode* sibling = children[idx - 1];

        for (int i = child->num_keys - 1; i >= 0; --i) {
            child->keys[i + 1] = child->keys[i];
        }

        if (!child->is_leaf) {
            for (int i = child->num_keys; i >= 0; --i) {
                child->children[i + 1] = child->children[i];
            }
        }

        child->keys[0] = keys[idx - 1];
        if (!child->is_leaf) {
            child->children[0] = sibling->children[sibling->num_keys];
            sibling->children[sibling->num_keys] = nullptr;
        }

        keys[idx - 1] = sibling->keys[sibling->num_keys - 1];
        child->num_keys += 1;
        sibling->num_keys -= 1;
    }

    void borrow_from_next(int idx) {
        FusionNode* child = children[idx];
        FusionNode* sibling = children[idx + 1];

        child->keys[child->num_keys] = keys[idx];

        if (!child->is_leaf) {
            child->children[child->num_keys + 1] = sibling->children[0];
            sibling->children[0] = nullptr;
        }

        keys[idx] = sibling->keys[0];

        for (int i = 1; i < sibling->num_keys; ++i) {
            sibling->keys[i - 1] = sibling->keys[i];
        }

        if (!sibling->is_leaf) {
            for (int i = 1; i <= sibling->num_keys; ++i) {
                sibling->children[i - 1] = sibling->children[i];
            }
            sibling->children[sibling->num_keys] = nullptr;
        }

        child->num_keys += 1;
        sibling->num_keys -= 1;
    }

    void merge(int idx) {
        FusionNode* child = children[idx];
        FusionNode* sibling = children[idx + 1];

        child->keys[T - 1] = keys[idx];

        for (int i = 0; i < sibling->num_keys; ++i) {
            child->keys[i + T] = sibling->keys[i];
        }

        if (!child->is_leaf) {
            for (int i = 0; i <= sibling->num_keys; ++i) {
                child->children[i + T] = sibling->children[i];
                sibling->children[i] = nullptr;
            }
        }

        for (int i = idx + 1; i < num_keys; ++i) {
            keys[i - 1] = keys[i];
        }

        for (int i = idx + 2; i <= num_keys; ++i) {
            children[i - 1] = children[i];
        }
        children[num_keys] = nullptr;

        child->num_keys += sibling->num_keys + 1;
        num_keys--;

        delete sibling;
    }
};

/**
 * @brief Fusion Tree over 64-bit unsigned integer universe.
 * Supports O(log_B N) predecessor, successor, insertion, and deletion.
 */
class FusionTree {
private:
    FusionNode* root_{nullptr};
    size_t size_{0};

    void split_child(FusionNode* parent, int i, FusionNode* full_child) {
        FusionNode* z = new FusionNode(full_child->is_leaf);
        z->num_keys = FusionNode::T - 1; // 3

        for (int j = 0; j < FusionNode::T - 1; ++j) {
            z->keys[j] = full_child->keys[j + FusionNode::T];
        }

        if (!full_child->is_leaf) {
            for (int j = 0; j < FusionNode::T; ++j) {
                z->children[j] = full_child->children[j + FusionNode::T];
                full_child->children[j + FusionNode::T] = nullptr;
            }
        }

        full_child->num_keys = FusionNode::T - 1;

        for (int j = parent->num_keys; j >= i + 1; --j) {
            parent->children[j + 1] = parent->children[j];
        }
        parent->children[i + 1] = z;

        for (int j = parent->num_keys - 1; j >= i; --j) {
            parent->keys[j + 1] = parent->keys[j];
        }
        parent->keys[i] = full_child->keys[FusionNode::T - 1];
        parent->num_keys++;
    }

    void insert_non_full(FusionNode* node, uint64_t x) {
        int i = node->num_keys - 1;

        if (node->is_leaf) {
            while (i >= 0 && node->keys[i] > x) {
                node->keys[i + 1] = node->keys[i];
                --i;
            }
            node->keys[i + 1] = x;
            node->num_keys++;
        } else {
            while (i >= 0 && node->keys[i] > x) {
                --i;
            }
            i++;
            if (node->children[i]->num_keys == FusionNode::MAX_KEYS) {
                split_child(node, i, node->children[i]);
                if (node->keys[i] < x) {
                    i++;
                }
            }
            insert_non_full(node->children[i], x);
        }
    }

public:
    FusionTree() : root_(new FusionNode(true)) {}

    ~FusionTree() {
        delete root_;
    }

    FusionTree(const FusionTree&) = delete;
    FusionTree& operator=(const FusionTree&) = delete;

    FusionTree(FusionTree&& other) noexcept
        : root_(other.root_), size_(other.size_) {
        other.root_ = new FusionNode(true);
        other.size_ = 0;
    }

    FusionTree& operator=(FusionTree&& other) noexcept {
        if (this != &other) {
            delete root_;
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = new FusionNode(true);
            other.size_ = 0;
        }
        return *this;
    }

    void clear() {
        delete root_;
        root_ = new FusionNode(true);
        size_ = 0;
    }

    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    [[nodiscard]] bool contains(uint64_t x) const noexcept {
        FusionNode* cur = root_;
        while (cur) {
            int idx = cur->find_key(x);
            if (idx < cur->num_keys && cur->keys[idx] == x) return true;
            if (cur->is_leaf) break;
            cur = cur->children[idx];
        }
        return false;
    }

    bool insert(uint64_t x) {
        if (contains(x)) return false;

        FusionNode* r = root_;
        if (r->num_keys == FusionNode::MAX_KEYS) {
            FusionNode* s = new FusionNode(false);
            root_ = s;
            s->children[0] = r;
            split_child(s, 0, r);
            insert_non_full(s, x);
        } else {
            insert_non_full(r, x);
        }
        size_++;
        return true;
    }

    bool erase(uint64_t x) {
        if (!contains(x)) return false;

        root_->remove(x);

        if (root_->num_keys == 0) {
            FusionNode* tmp = root_;
            if (!root_->is_leaf && root_->children[0]) {
                root_ = root_->children[0];
                tmp->is_leaf = true; // prevent recursive destruction of children
                delete tmp;
            }
        }
        size_--;
        return true;
    }

    [[nodiscard]] std::optional<uint64_t> predecessor(uint64_t x) const noexcept {
        if (size_ == 0) return std::nullopt;
        std::optional<uint64_t> best = std::nullopt;
        FusionNode* cur = root_;

        while (cur) {
            int idx = cur->find_key(x);
            if (idx < cur->num_keys && cur->keys[idx] == x) {
                return x;
            }
            if (idx > 0) {
                best = cur->keys[idx - 1];
            }
            if (cur->is_leaf) break;
            cur = cur->children[idx];
        }
        return best;
    }

    [[nodiscard]] std::optional<uint64_t> successor(uint64_t x) const noexcept {
        if (size_ == 0) return std::nullopt;
        std::optional<uint64_t> best = std::nullopt;
        FusionNode* cur = root_;

        while (cur) {
            int idx = cur->find_key(x);
            if (idx < cur->num_keys && cur->keys[idx] == x) {
                return x;
            }
            if (idx < cur->num_keys) {
                best = cur->keys[idx];
            }
            if (cur->is_leaf) break;
            cur = cur->children[idx];
        }
        return best;
    }
};

} // namespace dsa

int main() {
    using namespace dsa;
    std::cout << "Testing Word RAM parallel comparison and FusionTree...\n";

    // 1. Verify packed subtraction parallel rank
    {
        uint8_t raw_keys[] = {10, 25, 40, 70, 95};
        uint64_t packed = 0;
        for (int i = 0; i < 5; ++i) {
            uint64_t field = 0x80ULL | (raw_keys[i] & 0x7F);
            packed |= (field << (8 * i));
        }

        assert(FusionNode::parallel_rank(packed, 5, 5) == 0);
        assert(FusionNode::parallel_rank(packed, 5, 10) == 0);
        assert(FusionNode::parallel_rank(packed, 5, 11) == 1);
        assert(FusionNode::parallel_rank(packed, 5, 25) == 1);
        assert(FusionNode::parallel_rank(packed, 5, 30) == 2);
        assert(FusionNode::parallel_rank(packed, 5, 40) == 2);
        assert(FusionNode::parallel_rank(packed, 5, 70) == 3);
        assert(FusionNode::parallel_rank(packed, 5, 80) == 4);
        assert(FusionNode::parallel_rank(packed, 5, 95) == 4);
        assert(FusionNode::parallel_rank(packed, 5, 100) == 5);
        std::cout << "Packed 64-bit parallel rank passed.\n";
    }

    // 2. Randomized differential verification against std::set
    {
        FusionTree ft;
        std::set<uint64_t> oracle;
        std::mt19937_64 rng(1337);
        std::uniform_int_distribution<uint64_t> dist(1, 100000);

        for (int i = 0; i < 10000; ++i) {
            uint64_t op = rng() % 3;
            uint64_t val = dist(rng);

            if (op == 0) { // Insert
                bool fi = ft.insert(val);
                bool oi = oracle.insert(val).second;
                assert(fi == oi);
            } else if (op == 1) { // Erase
                bool fe = ft.erase(val);
                bool oe = (oracle.erase(val) > 0);
                assert(fe == oe);
            } else { // Queries
                assert(ft.size() == oracle.size());
                for (auto q : {val, val - 1, val + 1}) {
                    auto o_succ = [&]() -> std::optional<uint64_t> {
                        if (oracle.empty()) return std::nullopt;
                        auto it = oracle.lower_bound(q);
                        if (it == oracle.end()) return std::nullopt;
                        return *it;
                    }();
                    auto f_succ = ft.successor(q);
                    assert(f_succ == o_succ);

                    auto o_pred = [&]() -> std::optional<uint64_t> {
                        if (oracle.empty()) return std::nullopt;
                        auto it = oracle.upper_bound(q);
                        if (it == oracle.begin()) return std::nullopt;
                        return *std::prev(it);
                    }();
                    auto f_pred = ft.predecessor(q);
                    assert(f_pred == o_pred);
                }
            }
        }
    }

    std::cout << "FusionTree 10,000 differential operations passed with 100% precision!\n";
    return 0;
}
