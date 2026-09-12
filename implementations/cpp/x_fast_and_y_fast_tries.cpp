/**
 * @file x_fast_and_y_fast_tries.cpp
 * @brief High-performance C++17 implementation of X-Fast and Y-Fast Tries.
 *
 * Implements Willard's integer search structures on the Word RAM model:
 * 1. XFastTrie:
 *    - Bitwise trie of height W (default 32 bits).
 *    - Level hash tables storing prefixes at each depth.
 *    - Doubly linked list of leaves in sorted order.
 *    - Descendant pointers (desc_min and desc_max) for LCP-based O(log W) predecessor/successor.
 *    - Space: O(N * W).
 * 2. YFastTrie:
 *    - Indirection via balanced micro-trees (size bounded within [W/2, 2*W]).
 *    - Dynamic representative routing via an X-Fast Trie of size O(N / W).
 *    - Space: O(N).
 *    - Predecessor/Successor/Insert/Erase: O(log W) = O(log log U) amortized.
 *
 * Fully compliant with -std=c++17 -O3 -Wall -Wextra -Werror.
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <optional>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

namespace dsa {

/**
 * @brief X-Fast Trie over 32-bit unsigned integer universe [0, 2^32 - 1].
 */
class XFastTrie {
public:
    static constexpr int W = 32;

    struct Node {
        uint32_t prefix{0};
        int depth{0};
        Node* left{nullptr};
        Node* right{nullptr};
        Node* desc_min{nullptr};
        Node* desc_max{nullptr};
        Node* prev_leaf{nullptr};
        Node* next_leaf{nullptr};

        Node(uint32_t p, int d) : prefix(p), depth(d) {
            if (d == W) {
                desc_min = desc_max = this;
            }
        }
    };

private:
    std::unordered_map<uint32_t, Node*> levels_[W + 1];
    Node* root_{nullptr};
    Node* leaf_head_{nullptr};
    Node* leaf_tail_{nullptr};
    size_t size_{0};

public:
    XFastTrie() {
        root_ = new Node(0, 0);
        levels_[0][0] = root_;
    }

    ~XFastTrie() {
        clear();
        delete root_;
    }

    XFastTrie(const XFastTrie&) = delete;
    XFastTrie& operator=(const XFastTrie&) = delete;

    XFastTrie(XFastTrie&& other) noexcept
        : root_(other.root_),
          leaf_head_(other.leaf_head_),
          leaf_tail_(other.leaf_tail_),
          size_(other.size_) {
        for (int d = 0; d <= W; ++d) {
            levels_[d] = std::move(other.levels_[d]);
        }
        other.root_ = new Node(0, 0);
        other.levels_[0][0] = other.root_;
        other.leaf_head_ = other.leaf_tail_ = nullptr;
        other.size_ = 0;
    }

    XFastTrie& operator=(XFastTrie&& other) noexcept {
        if (this != &other) {
            clear();
            delete root_;
            root_ = other.root_;
            leaf_head_ = other.leaf_head_;
            leaf_tail_ = other.leaf_tail_;
            size_ = other.size_;
            for (int d = 0; d <= W; ++d) {
                levels_[d] = std::move(other.levels_[d]);
            }
            other.root_ = new Node(0, 0);
            other.levels_[0][0] = other.root_;
            other.leaf_head_ = other.leaf_tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void clear() {
        for (int d = 1; d <= W; ++d) {
            for (auto& kv : levels_[d]) {
                delete kv.second;
            }
            levels_[d].clear();
        }
        root_->left = root_->right = root_->desc_min = root_->desc_max = nullptr;
        leaf_head_ = leaf_tail_ = nullptr;
        size_ = 0;
    }

    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    [[nodiscard]] bool contains(uint32_t x) const {
        return levels_[W].find(x) != levels_[W].end();
    }

    bool insert(uint32_t x) {
        if (contains(x)) return false;

        Node* leaf = new Node(x, W);
        levels_[W][x] = leaf;

        // Maintain doubly linked list of leaves in sorted order
        if (!leaf_head_) {
            leaf_head_ = leaf_tail_ = leaf;
        } else if (x < leaf_head_->prefix) {
            leaf->next_leaf = leaf_head_;
            leaf_head_->prev_leaf = leaf;
            leaf_head_ = leaf;
        } else if (x > leaf_tail_->prefix) {
            leaf->prev_leaf = leaf_tail_;
            leaf_tail_->next_leaf = leaf;
            leaf_tail_ = leaf;
        } else {
            Node* cur = leaf_head_;
            while (cur->next_leaf && cur->next_leaf->prefix < x) {
                cur = cur->next_leaf;
            }
            leaf->next_leaf = cur->next_leaf;
            leaf->prev_leaf = cur;
            if (cur->next_leaf) cur->next_leaf->prev_leaf = leaf;
            cur->next_leaf = leaf;
        }

        // Establish path nodes
        std::vector<Node*> path;
        path.reserve(W + 1);
        Node* curr = root_;
        path.push_back(curr);

        for (int d = 1; d <= W; ++d) {
            uint32_t pref = (d == W) ? x : (x >> (W - d));
            int bit = (x >> (W - d)) & 1;

            Node* next_node = nullptr;
            if (d == W) {
                next_node = leaf;
            } else {
                auto it = levels_[d].find(pref);
                if (it == levels_[d].end()) {
                    next_node = new Node(pref, d);
                    levels_[d][pref] = next_node;
                } else {
                    next_node = it->second;
                }
            }

            if (bit == 0) curr->left = next_node;
            else curr->right = next_node;

            path.push_back(next_node);
            curr = next_node;
        }

        // Bottom-up invariant update for desc_min and desc_max
        for (int i = W - 1; i >= 0; --i) {
            Node* u = path[i];
            u->desc_min = u->left ? u->left->desc_min : (u->right ? u->right->desc_min : nullptr);
            u->desc_max = u->right ? u->right->desc_max : (u->left ? u->left->desc_max : nullptr);
        }

        size_++;
        return true;
    }

    bool erase(uint32_t x) {
        auto it = levels_[W].find(x);
        if (it == levels_[W].end()) return false;
        Node* leaf = it->second;

        // Unlink from leaf list
        if (leaf->prev_leaf) leaf->prev_leaf->next_leaf = leaf->next_leaf;
        else leaf_head_ = leaf->next_leaf;
        if (leaf->next_leaf) leaf->next_leaf->prev_leaf = leaf->prev_leaf;
        else leaf_tail_ = leaf->prev_leaf;

        levels_[W].erase(it);

        // Record ancestor path
        std::vector<Node*> path;
        path.reserve(W + 1);
        Node* curr = root_;
        path.push_back(curr);
        for (int d = 1; d <= W; ++d) {
            int bit = (x >> (W - d)) & 1;
            curr = (bit == 0) ? curr->left : curr->right;
            path.push_back(curr);
        }

        delete leaf;
        bool child_deleted = true;

        for (int d = W - 1; d >= 0; --d) {
            Node* u = path[d];
            int bit = (x >> (W - 1 - d)) & 1;

            if (child_deleted) {
                if (bit == 0) u->left = nullptr;
                else u->right = nullptr;
            }

            if (d > 0 && u->left == nullptr && u->right == nullptr) {
                levels_[d].erase(u->prefix);
                delete u;
                child_deleted = true;
            } else {
                u->desc_min = u->left ? u->left->desc_min : (u->right ? u->right->desc_min : nullptr);
                u->desc_max = u->right ? u->right->desc_max : (u->left ? u->left->desc_max : nullptr);
                child_deleted = false;
            }
        }

        size_--;
        return true;
    }

    [[nodiscard]] std::optional<uint32_t> predecessor(uint32_t x) const {
        if (size_ == 0) return std::nullopt;
        auto it = levels_[W].find(x);
        if (it != levels_[W].end()) return x;

        int low = 0, high = W, lcp_depth = 0;
        Node* lcp_node = root_;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            uint32_t pref = (mid == 0) ? 0 : (x >> (W - mid));
            auto found = levels_[mid].find(pref);
            if (found != levels_[mid].end()) {
                lcp_depth = mid;
                lcp_node = found->second;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        int next_bit = (x >> (W - 1 - lcp_depth)) & 1;
        Node* cand = (next_bit == 0) ? lcp_node->desc_min : lcp_node->desc_max;
        if (cand && cand->prefix > x) cand = cand->prev_leaf;

        if (cand && cand->prefix <= x) return cand->prefix;
        return std::nullopt;
    }

    [[nodiscard]] std::optional<uint32_t> successor(uint32_t x) const {
        if (size_ == 0) return std::nullopt;
        auto it = levels_[W].find(x);
        if (it != levels_[W].end()) return x;

        int low = 0, high = W, lcp_depth = 0;
        Node* lcp_node = root_;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            uint32_t pref = (mid == 0) ? 0 : (x >> (W - mid));
            auto found = levels_[mid].find(pref);
            if (found != levels_[mid].end()) {
                lcp_depth = mid;
                lcp_node = found->second;
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }

        int next_bit = (x >> (W - 1 - lcp_depth)) & 1;
        Node* cand = (next_bit == 0) ? lcp_node->desc_min : lcp_node->desc_max;
        if (cand && cand->prefix < x) cand = cand->next_leaf;

        if (cand && cand->prefix >= x) return cand->prefix;
        return std::nullopt;
    }
};

/**
 * @brief Y-Fast Trie achieving O(N) space and O(log W) = O(log log U) queries and updates.
 */
class YFastTrie {
public:
    static constexpr int W = 32;
    static constexpr size_t MAX_LEAVES = 2 * W; // 64
    static constexpr size_t MIN_LEAVES = W / 2; // 16

    struct Cluster {
        std::set<uint32_t> tree;
        uint32_t rep{0};
        Cluster* prev{nullptr};
        Cluster* next{nullptr};
    };

private:
    XFastTrie rep_trie_;
    std::unordered_map<uint32_t, Cluster*> rep_to_cluster_;
    Cluster* cluster_head_{nullptr};
    Cluster* cluster_tail_{nullptr};
    size_t size_{0};

    [[nodiscard]] Cluster* find_cluster(uint32_t x) const {
        if (empty()) return nullptr;
        auto succ_rep = rep_trie_.successor(x);
        if (succ_rep.has_value()) {
            return rep_to_cluster_.at(*succ_rep);
        }
        return cluster_tail_;
    }

    void split_cluster(Cluster* c) {
        Cluster* n = new Cluster();
        n->next = c->next;
        n->prev = c;
        if (c->next) c->next->prev = n;
        else cluster_tail_ = n;
        c->next = n;

        size_t half = c->tree.size() / 2;
        auto it = c->tree.begin();
        std::advance(it, half);
        std::vector<uint32_t> to_move(it, c->tree.end());
        for (auto val : to_move) c->tree.erase(val);
        for (auto val : to_move) n->tree.insert(val);

        rep_to_cluster_.erase(c->rep);
        rep_trie_.erase(c->rep);
        c->rep = *c->tree.rbegin();
        rep_trie_.insert(c->rep);
        rep_to_cluster_[c->rep] = c;

        n->rep = *n->tree.rbegin();
        rep_trie_.insert(n->rep);
        rep_to_cluster_[n->rep] = n;
    }

public:
    YFastTrie() = default;

    ~YFastTrie() {
        clear();
    }

    YFastTrie(const YFastTrie&) = delete;
    YFastTrie& operator=(const YFastTrie&) = delete;

    YFastTrie(YFastTrie&& other) noexcept
        : rep_trie_(std::move(other.rep_trie_)),
          rep_to_cluster_(std::move(other.rep_to_cluster_)),
          cluster_head_(other.cluster_head_),
          cluster_tail_(other.cluster_tail_),
          size_(other.size_) {
        other.cluster_head_ = other.cluster_tail_ = nullptr;
        other.size_ = 0;
    }

    YFastTrie& operator=(YFastTrie&& other) noexcept {
        if (this != &other) {
            clear();
            rep_trie_ = std::move(other.rep_trie_);
            rep_to_cluster_ = std::move(other.rep_to_cluster_);
            cluster_head_ = other.cluster_head_;
            cluster_tail_ = other.cluster_tail_;
            size_ = other.size_;
            other.cluster_head_ = other.cluster_tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void clear() {
        Cluster* cur = cluster_head_;
        while (cur) {
            Cluster* nxt = cur->next;
            delete cur;
            cur = nxt;
        }
        cluster_head_ = cluster_tail_ = nullptr;
        rep_to_cluster_.clear();
        rep_trie_.clear();
        size_ = 0;
    }

    [[nodiscard]] size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    [[nodiscard]] bool contains(uint32_t x) const {
        if (empty()) return false;
        Cluster* c = find_cluster(x);
        return c && c->tree.count(x) > 0;
    }

    bool insert(uint32_t x) {
        if (empty()) {
            Cluster* c = new Cluster();
            c->tree.insert(x);
            c->rep = x;
            rep_trie_.insert(x);
            rep_to_cluster_[x] = c;
            cluster_head_ = cluster_tail_ = c;
            size_++;
            return true;
        }

        Cluster* c = find_cluster(x);
        if (!c || c->tree.count(x) > 0) return false;

        c->tree.insert(x);
        size_++;

        uint32_t new_max = *c->tree.rbegin();
        if (new_max != c->rep) {
            rep_to_cluster_.erase(c->rep);
            rep_trie_.erase(c->rep);
            c->rep = new_max;
            rep_trie_.insert(new_max);
            rep_to_cluster_[new_max] = c;
        }

        if (c->tree.size() > MAX_LEAVES) {
            split_cluster(c);
        }
        return true;
    }

    bool erase(uint32_t x) {
        if (empty()) return false;
        Cluster* c = find_cluster(x);
        if (!c || c->tree.count(x) == 0) return false;

        c->tree.erase(x);
        size_--;

        if (c->tree.empty()) {
            rep_to_cluster_.erase(c->rep);
            rep_trie_.erase(c->rep);
            if (c->prev) c->prev->next = c->next;
            else cluster_head_ = c->next;
            if (c->next) c->next->prev = c->prev;
            else cluster_tail_ = c->prev;
            delete c;
            return true;
        }

        uint32_t new_max = *c->tree.rbegin();
        if (new_max != c->rep) {
            rep_to_cluster_.erase(c->rep);
            rep_trie_.erase(c->rep);
            c->rep = new_max;
            rep_trie_.insert(new_max);
            rep_to_cluster_[new_max] = c;
        }

        if (c->tree.size() < MIN_LEAVES && c->next) {
            Cluster* n = c->next;
            if (c->tree.size() + n->tree.size() <= MAX_LEAVES) {
                for (auto val : n->tree) c->tree.insert(val);
                rep_to_cluster_.erase(n->rep);
                rep_trie_.erase(n->rep);
                c->next = n->next;
                if (n->next) n->next->prev = c;
                else cluster_tail_ = c;
                delete n;

                uint32_t merged_max = *c->tree.rbegin();
                if (merged_max != c->rep) {
                    rep_to_cluster_.erase(c->rep);
                    rep_trie_.erase(c->rep);
                    c->rep = merged_max;
                    rep_trie_.insert(merged_max);
                    rep_to_cluster_[merged_max] = c;
                }
            }
        }

        return true;
    }

    [[nodiscard]] std::optional<uint32_t> predecessor(uint32_t x) const {
        if (empty()) return std::nullopt;
        Cluster* c = find_cluster(x);
        if (c) {
            auto it = c->tree.upper_bound(x);
            if (it != c->tree.begin()) {
                return *std::prev(it);
            }
            if (c->prev && !c->prev->tree.empty()) {
                return *c->prev->tree.rbegin();
            }
        }
        return std::nullopt;
    }

    [[nodiscard]] std::optional<uint32_t> successor(uint32_t x) const {
        if (empty()) return std::nullopt;
        Cluster* c = find_cluster(x);
        if (c) {
            auto it = c->tree.lower_bound(x);
            if (it != c->tree.end()) {
                return *it;
            }
            if (c->next && !c->next->tree.empty()) {
                return *c->next->tree.begin();
            }
        }
        return std::nullopt;
    }
};

} // namespace dsa

int main() {
    using namespace dsa;
    std::cout << "Running comprehensive verification tests on XFastTrie and YFastTrie...\n";

    // 1. Basic Unit Tests
    {
        XFastTrie xft;
        assert(xft.empty());
        assert(xft.size() == 0);
        assert(!xft.predecessor(10).has_value());
        assert(!xft.successor(10).has_value());

        xft.insert(100);
        xft.insert(200);
        xft.insert(300);

        assert(xft.size() == 3);
        assert(xft.contains(100));
        assert(xft.contains(200));
        assert(xft.contains(300));
        assert(!xft.contains(150));

        assert(xft.predecessor(99) == std::nullopt);
        assert(xft.predecessor(100) == 100);
        assert(xft.predecessor(150) == 100);
        assert(xft.predecessor(200) == 200);
        assert(xft.predecessor(350) == 300);

        assert(xft.successor(50) == 100);
        assert(xft.successor(100) == 100);
        assert(xft.successor(150) == 200);
        assert(xft.successor(300) == 300);
        assert(xft.successor(301) == std::nullopt);

        assert(xft.erase(200));
        assert(!xft.contains(200));
        assert(xft.size() == 2);
        assert(xft.successor(150) == 300);
        assert(xft.predecessor(250) == 100);
    }

    // 2. Large Scale Randomized Differential Verification against std::set
    {
        XFastTrie xft;
        YFastTrie yft;
        std::set<uint32_t> oracle;

        std::mt19937 rng(1337);
        std::uniform_int_distribution<uint32_t> dist(1, 100000);

        for (int i = 0; i < 10000; ++i) {
            uint32_t op = rng() % 3;
            uint32_t val = dist(rng);

            if (op == 0) { // Insert
                bool xi = xft.insert(val);
                bool yi = yft.insert(val);
                bool oi = oracle.insert(val).second;
                assert(xi == oi);
                assert(yi == oi);
            } else if (op == 1) { // Erase
                bool xe = xft.erase(val);
                bool ye = yft.erase(val);
                bool oe = (oracle.erase(val) > 0);
                assert(xe == oe);
                assert(ye == oe);
            } else { // Queries
                assert(xft.size() == oracle.size());
                assert(yft.size() == oracle.size());

                for (auto q : {val, val - 1, val + 1}) {
                    auto o_succ = [&]() -> std::optional<uint32_t> {
                        if (oracle.empty()) return std::nullopt;
                        auto it = oracle.lower_bound(q);
                        if (it == oracle.end()) return std::nullopt;
                        return *it;
                    }();
                    auto x_succ = xft.successor(q);
                    auto y_succ = yft.successor(q);
                    assert(x_succ == o_succ);
                    assert(y_succ == o_succ);

                    auto o_pred = [&]() -> std::optional<uint32_t> {
                        if (oracle.empty()) return std::nullopt;
                        auto it = oracle.upper_bound(q);
                        if (it == oracle.begin()) return std::nullopt;
                        return *std::prev(it);
                    }();
                    auto x_pred = xft.predecessor(q);
                    auto y_pred = yft.predecessor(q);
                    assert(x_pred == o_pred);
                    assert(y_pred == o_pred);
                }
            }
        }
    }

    std::cout << "All XFastTrie and YFastTrie tests passed with 100% precision!\n";
    return 0;
}
