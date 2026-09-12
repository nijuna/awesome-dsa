/**
 * @file testing_data_structures.cpp
 * @brief Differential testing framework and self-verifying AVL tree candidate vs std::set oracle.
 *
 * Implements an AVL Tree candidate data structure with internal invariant verification,
 * and a differential testing harness executing randomized operations against std::set.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <set>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

namespace dsa {

/**
 * @brief Self-verifying AVL Tree implementation.
 */
class AVLTree {
private:
    struct Node {
        int key;
        int height;
        Node* left;
        Node* right;

        explicit Node(int val) : key(val), height(1), left(nullptr), right(nullptr) {}
    };

    Node* root_ = nullptr;
    size_t size_ = 0;

    int get_height(Node* n) const {
        return n ? n->height : 0;
    }

    int get_balance(Node* n) const {
        return n ? get_height(n->left) - get_height(n->right) : 0;
    }

    void update_height(Node* n) {
        if (n) {
            n->height = 1 + std::max(get_height(n->left), get_height(n->right));
        }
    }

    Node* rotate_right(Node* y) {
        Node* x = y->left;
        Node* t2 = x->right;

        x->right = y;
        y->left = t2;

        update_height(y);
        update_height(x);

        return x;
    }

    Node* rotate_left(Node* x) {
        Node* y = x->right;
        Node* t2 = y->left;

        y->left = x;
        x->right = t2;

        update_height(x);
        update_height(y);

        return y;
    }

    Node* balance(Node* node) {
        if (!node) return nullptr;
        update_height(node);

        int balance_factor = get_balance(node);

        // Left heavy
        if (balance_factor > 1) {
            if (get_balance(node->left) < 0) {
                node->left = rotate_left(node->left);
            }
            return rotate_right(node);
        }

        // Right heavy
        if (balance_factor < -1) {
            if (get_balance(node->right) > 0) {
                node->right = rotate_right(node->right);
            }
            return rotate_left(node);
        }

        return node;
    }

    Node* insert(Node* node, int key, bool& inserted) {
        if (!node) {
            inserted = true;
            return new Node(key);
        }

        if (key < node->key) {
            node->left = insert(node->left, key, inserted);
        } else if (key > node->key) {
            node->right = insert(node->right, key, inserted);
        } else {
            inserted = false; // Key already present
            return node;
        }

        return balance(node);
    }

    Node* find_min(Node* node) const {
        while (node && node->left) {
            node = node->left;
        }
        return node;
    }

    Node* erase(Node* node, int key, bool& erased) {
        if (!node) {
            erased = false;
            return nullptr;
        }

        if (key < node->key) {
            node->left = erase(node->left, key, erased);
        } else if (key > node->key) {
            node->right = erase(node->right, key, erased);
        } else {
            erased = true;
            if (!node->left || !node->right) {
                Node* temp = node->left ? node->left : node->right;
                if (!temp) {
                    // No child
                    delete node;
                    return nullptr;
                } else {
                    // One child
                    *node = *temp;
                    delete temp;
                }
            } else {
                // Two children
                Node* succ = find_min(node->right);
                node->key = succ->key;
                node->right = erase(node->right, succ->key, erased);
            }
        }

        return balance(node);
    }

    void destroy(Node* node) {
        if (!node) return;
        destroy(node->left);
        destroy(node->right);
        delete node;
    }

    void in_order(Node* node, std::vector<int>& out) const {
        if (!node) return;
        in_order(node->left, out);
        out.push_back(node->key);
        in_order(node->right, out);
    }

    bool verify_node_invariants(Node* node, int64_t min_key, int64_t max_key) const {
        if (!node) return true;

        if (node->key <= min_key || node->key >= max_key) return false;

        int lh = get_height(node->left);
        int rh = get_height(node->right);

        if (node->height != 1 + std::max(lh, rh)) return false;
        if (std::abs(lh - rh) > 1) return false;

        return verify_node_invariants(node->left, min_key, node->key) &&
               verify_node_invariants(node->right, node->key, max_key);
    }

public:
    AVLTree() = default;
    ~AVLTree() {
        destroy(root_);
    }

    // Non-copyable for simplicity
    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    bool insert(int key) {
        bool inserted = false;
        root_ = insert(root_, key, inserted);
        if (inserted) ++size_;
        return inserted;
    }

    bool erase(int key) {
        bool erased = false;
        root_ = erase(root_, key, erased);
        if (erased) --size_;
        return erased;
    }

    bool contains(int key) const {
        Node* curr = root_;
        while (curr) {
            if (key < curr->key) curr = curr->left;
            else if (key > curr->key) curr = curr->right;
            else return true;
        }
        return false;
    }

    size_t size() const {
        return size_;
    }

    bool empty() const {
        return size_ == 0;
    }

    std::vector<int> to_vector() const {
        std::vector<int> out;
        out.reserve(size_);
        in_order(root_, out);
        return out;
    }

    /**
     * @brief Invariant validation method.
     * Verifies strict BST ordering, height tracking, and balance factors across all nodes.
     */
    bool verify_invariants() const {
        return verify_node_invariants(root_, -9223372036854775807LL, 9223372036854775807LL);
    }
};

/**
 * @brief Differential testing engine comparing AVLTree against std::set oracle.
 */
inline void run_differential_tests(uint64_t seed, int operations_count) {
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<int> op_dist(0, 99);
    std::uniform_int_distribution<int> key_dist(-1000, 1000);

    AVLTree candidate;
    std::set<int> oracle;

    for (int step = 0; step < operations_count; ++step) {
        int op = op_dist(rng);
        int key = key_dist(rng);

        if (op < 40) {
            // 40% Insert
            bool cand_res = candidate.insert(key);
            bool orac_res = oracle.insert(key).second;
            assert(cand_res == orac_res);
        } else if (op < 70) {
            // 30% Erase
            bool cand_res = candidate.erase(key);
            bool orac_res = (oracle.erase(key) > 0);
            assert(cand_res == orac_res);
        } else if (op < 90) {
            // 20% Contains
            bool cand_res = candidate.contains(key);
            bool orac_res = (oracle.find(key) != oracle.end());
            assert(cand_res == orac_res);
        } else {
            // 10% Traversal check
            assert(candidate.size() == oracle.size());
            assert(candidate.empty() == oracle.empty());
            std::vector<int> cand_vec = candidate.to_vector();
            std::vector<int> orac_vec(oracle.begin(), oracle.end());
            assert(cand_vec == orac_vec);
        }

        // Deep internal invariant check after every mutation
        assert(candidate.verify_invariants());
    }
}

} // namespace dsa

int main() {
    std::cout << "Running Data Structure Differential & Invariant Testing..." << std::endl;

    // Boundary / Unit tests
    dsa::AVLTree tree;
    assert(tree.empty());
    assert(tree.size() == 0);
    assert(!tree.contains(42));
    assert(!tree.erase(42));

    // Single element insert / erase
    assert(tree.insert(10));
    assert(!tree.empty());
    assert(tree.size() == 1);
    assert(tree.contains(10));
    assert(tree.verify_invariants());

    assert(!tree.insert(10)); // duplicate
    assert(tree.size() == 1);

    assert(tree.erase(10));
    assert(tree.empty());
    assert(tree.size() == 0);
    assert(tree.verify_invariants());

    // Run differential stress test with 30,000 randomized operations against std::set
    uint64_t seed = 42424242ULL;
    dsa::run_differential_tests(seed, 30000);

    std::cout << "Completed 30,000 differential operations with zero discrepancies!" << std::endl;
    std::cout << "All AVL invariants verified successfully!" << std::endl;
    return 0;
}
