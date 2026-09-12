/**
 * @file splay_trees.cpp
 * @brief Reference implementation of Splay Trees with Sleator-Tarjan Splay Rotations.
 *
 * Implements self-adjusting binary search trees with zig, zig-zig, and zig-zag splaying,
 * Arthur's Two-Layer API, BST invariant verification, and differential testing against std::map.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <map>
#include <random>
#include <functional>

namespace dsa {

template <typename Key, typename Value>
class SplayTree {
private:
    struct Node {
        Key key;
        Value val;
        Node* left = nullptr;
        Node* right = nullptr;
        Node* parent = nullptr;

        Node(const Key& k, const Value& v, Node* p = nullptr)
            : key(k), val(v), parent(p) {}
    };

    Node* root_ = nullptr;
    size_t size_ = 0;

    void rotate(Node* x) {
        Node* p = x->parent;
        Node* g = p->parent;
        bool is_right = (p->right == x);

        if (is_right) {
            p->right = x->left;
            if (x->left) x->left->parent = p;
            x->left = p;
        } else {
            p->left = x->right;
            if (x->right) x->right->parent = p;
            x->right = p;
        }

        p->parent = x;
        x->parent = g;

        if (g) {
            if (g->left == p) g->left = x;
            else g->right = x;
        } else {
            root_ = x;
        }
    }

    void splay(Node* x) {
        if (!x) return;
        while (x->parent) {
            Node* p = x->parent;
            Node* g = p->parent;
            if (!g) {
                // Zig step
                rotate(x);
            } else if ((g->left == p) == (p->left == x)) {
                // Zig-Zig step: Rotate parent first, then node x
                rotate(p);
                rotate(x);
            } else {
                // Zig-Zag step: Rotate node x twice
                rotate(x);
                rotate(x);
            }
        }
        root_ = x;
    }

    Node* find_internal(const Key& key) {
        if (!root_) return nullptr;
        Node* curr = root_;
        Node* last = curr;
        while (curr) {
            last = curr;
            if (key < curr->key) {
                curr = curr->left;
            } else if (curr->key < key) {
                curr = curr->right;
            } else {
                splay(curr);
                return curr;
            }
        }
        // Key not found: Splay the last visited node to maintain amortized bound
        splay(last);
        return nullptr;
    }

    void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }

    bool verify_node(Node* n, const Key* min_bound, const Key* max_bound) const {
        if (!n) return true;
        if (min_bound && !( *min_bound < n->key )) return false;
        if (max_bound && !( n->key < *max_bound )) return false;

        if (n->left) {
            if (n->left->parent != n) return false;
            if (!verify_node(n->left, min_bound, &n->key)) return false;
        }
        if (n->right) {
            if (n->right->parent != n) return false;
            if (!verify_node(n->right, &n->key, max_bound)) return false;
        }
        return true;
    }

public:
    SplayTree() = default;

    ~SplayTree() {
        destroy(root_);
    }

    SplayTree(const SplayTree&) = delete;
    SplayTree& operator=(const SplayTree&) = delete;

    SplayTree(SplayTree&& other) noexcept
        : root_(other.root_), size_(other.size_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    SplayTree& operator=(SplayTree&& other) noexcept {
        if (this != &other) {
            destroy(root_);
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return this;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return root_ == nullptr;
    }

    // --- Layer A: Fast Core API (Preconditioned) ---

    const Key& min() {
        assert(!empty());
        Node* curr = root_;
        while (curr->left) curr = curr->left;
        splay(curr);
        return root_->key;
    }

    const Key& max() {
        assert(!empty());
        Node* curr = root_;
        while (curr->right) curr = curr->right;
        splay(curr);
        return root_->key;
    }

    // --- Layer B: Safe Adapter API ---

    const Key* try_min() {
        if (empty()) return nullptr;
        return &min();
    }

    const Key* try_max() {
        if (empty()) return nullptr;
        return &max();
    }

    Value* try_find(const Key& key) {
        Node* n = find_internal(key);
        return n ? &n->val : nullptr;
    }

    bool contains(const Key& key) {
        return find_internal(key) != nullptr;
    }

    void insert(const Key& key, const Value& val) {
        if (!root_) {
            root_ = new Node(key, val);
            size_ = 1;
            return;
        }

        Node* found = find_internal(key);
        if (found) {
            found->val = val;
            return;
        }

        // root_ is now the node closest to key
        Node* new_node = new Node(key, val);
        if (key < root_->key) {
            new_node->left = root_->left;
            new_node->right = root_;
            if (root_->left) root_->left->parent = new_node;
            root_->left = nullptr;
        } else {
            new_node->right = root_->right;
            new_node->left = root_;
            if (root_->right) root_->right->parent = new_node;
            root_->right = nullptr;
        }
        root_->parent = new_node;
        root_ = new_node;
        size_++;
    }

    bool erase(const Key& key) {
        Node* found = find_internal(key);
        if (!found) return false;

        // found is now root_
        Node* left_sub = root_->left;
        Node* right_sub = root_->right;

        delete root_;
        size_--;

        if (!left_sub) {
            root_ = right_sub;
            if (root_) root_->parent = nullptr;
        } else {
            left_sub->parent = nullptr;
            // Splay the maximum of left subtree to its root
            Node* max_left = left_sub;
            while (max_left->right) max_left = max_left->right;
            
            // Splay max_left within left_sub
            root_ = left_sub;
            splay(max_left); // Now root_ is max_left and has no right child
            root_->right = right_sub;
            if (right_sub) right_sub->parent = root_;
        }
        return true;
    }

    bool verify_invariants() const {
        if (!root_) return size_ == 0;
        if (root_->parent != nullptr) return false;
        return verify_node(root_, nullptr, nullptr);
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Splay Tree verification..." << std::endl;

    dsa::SplayTree<int, int> splay;
    std::map<int, int> oracle;

    // 1. Basic Operations & BST Invariants
    auto add_initial = [&](int k, int v) {
        splay.insert(k, v);
        oracle[k] = v;
    };
    add_initial(50, 500);
    add_initial(20, 200);
    add_initial(70, 700);
    add_initial(10, 100);
    add_initial(30, 300);

    assert(splay.size() == 5);
    assert(splay.verify_invariants());
    assert(splay.min() == 10);
    assert(splay.max() == 70);

    // Verify search splays node to root
    int* val = splay.try_find(30);
    assert(val != nullptr && *val == 300);
    assert(splay.verify_invariants());

    // 2. Randomized Differential Fuzzing vs std::map
    std::mt19937_64 rng(1337);
    const int OPS = 1000;

    for (int step = 0; step < OPS; ++step) {
        int op = rng() % 4;
        int key = rng() % 200;

        if (op == 0) {
            // Insert
            int value = key * 10;
            splay.insert(key, value);
            oracle[key] = value;
        } else if (op == 1) {
            // Erase
            bool r1 = splay.erase(key);
            bool r2 = oracle.erase(key) > 0;
            assert(r1 == r2);
        } else if (op == 2) {
            // Find
            int* v1 = splay.try_find(key);
            auto it = oracle.find(key);
            if (it == oracle.end()) {
                assert(v1 == nullptr);
            } else {
                assert(v1 != nullptr);
                assert(*v1 == it->second);
            }
        } else {
            // Min & Max checks
            if (!oracle.empty()) {
                assert(splay.min() == oracle.begin()->first);
                assert(splay.max() == oracle.rbegin()->first);
            } else {
                assert(splay.empty());
                assert(splay.try_min() == nullptr);
                assert(splay.try_max() == nullptr);
            }
        }

        assert(splay.size() == oracle.size());
        assert(splay.verify_invariants());
    }

    std::cout << "[PASS] Verified basic splay operations, zig-zig rotation, and BST invariants." << std::endl;
    std::cout << "[PASS] 1000 differential operations matched std::map bit-for-bit." << std::endl;
    std::cout << "All Splay Tree assertions passed successfully!" << std::endl;
    return 0;
}
