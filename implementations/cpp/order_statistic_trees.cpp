/**
 * @file order_statistic_trees.cpp
 * @brief Reference implementation of an Order-Statistic Tree (augmented balanced BST).
 *
 * Implements find_by_order(k) (select) and order_of_key(x) (rank) in O(log n) time
 * using an augmented Treap structure.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <random>
#include <cstdint>
#include <cassert>
#include <stdexcept>
#include <algorithm>

namespace dsa {

template <typename T>
class OrderStatisticTree {
private:
    struct Node {
        T key;
        uint32_t priority;
        size_t size;
        Node* left;
        Node* right;

        Node(const T& k, uint32_t p)
            : key(k), priority(p), size(1), left(nullptr), right(nullptr) {}
    };

    Node* root = nullptr;
    std::mt19937 rng;

    static size_t get_size(Node* n) {
        return n ? n->size : 0;
    }

    static void update_size(Node* n) {
        if (n) {
            n->size = 1 + get_size(n->left) + get_size(n->right);
        }
    }

    Node* rotate_right(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        x->right = y;
        update_size(y);
        update_size(x);
        return x;
    }

    Node* rotate_left(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        update_size(x);
        update_size(y);
        return y;
    }

    Node* insert_helper(Node* node, const T& key) {
        if (!node) {
            return new Node(key, rng());
        }
        if (key < node->key) {
            node->left = insert_helper(node->left, key);
            if (node->left->priority > node->priority) {
                node = rotate_right(node);
            }
        } else if (node->key < key) {
            node->right = insert_helper(node->right, key);
            if (node->right->priority > node->priority) {
                node = rotate_left(node);
            }
        }
        // If key == node->key, do not duplicate (set semantics)
        update_size(node);
        return node;
    }

    Node* erase_helper(Node* node, const T& key) {
        if (!node) return nullptr;

        if (key < node->key) {
            node->left = erase_helper(node->left, key);
        } else if (node->key < key) {
            node->right = erase_helper(node->right, key);
        } else {
            // Found node to delete
            if (!node->left && !node->right) {
                delete node;
                return nullptr;
            } else if (!node->left) {
                Node* temp = node->right;
                delete node;
                return temp;
            } else if (!node->right) {
                Node* temp = node->left;
                delete node;
                return temp;
            } else {
                // Both children exist: rotate child with higher priority to root
                if (node->left->priority > node->right->priority) {
                    node = rotate_right(node);
                    node->right = erase_helper(node->right, key);
                } else {
                    node = rotate_left(node);
                    node->left = erase_helper(node->left, key);
                }
            }
        }
        update_size(node);
        return node;
    }

    const T& select_helper(Node* node, size_t k) const {
        assert(node != nullptr);
        size_t left_size = get_size(node->left);
        if (k == left_size) {
            return node->key;
        } else if (k < left_size) {
            return select_helper(node->left, k);
        } else {
            return select_helper(node->right, k - left_size - 1);
        }
    }

    void destroy(Node* node) {
        if (node) {
            destroy(node->left);
            destroy(node->right);
            delete node;
        }
    }

public:
    OrderStatisticTree(uint32_t seed = 42) : rng(seed) {}

    ~OrderStatisticTree() {
        destroy(root);
    }

    // Disable copy for simplicity, allow move
    OrderStatisticTree(const OrderStatisticTree&) = delete;
    OrderStatisticTree& operator=(const OrderStatisticTree&) = delete;

    size_t size() const {
        return get_size(root);
    }

    bool empty() const {
        return root == nullptr;
    }

    void insert(const T& key) {
        root = insert_helper(root, key);
    }

    void erase(const T& key) {
        root = erase_helper(root, key);
    }

    /**
     * @brief Returns the k-th smallest element (0-indexed) in O(log n) time.
     */
    const T& find_by_order(size_t k) const {
        if (k >= size()) {
            throw std::out_of_range("Rank k exceeds tree size");
        }
        return select_helper(root, k);
    }

    /**
     * @brief Returns count of elements in the tree strictly smaller than key in O(log n) time.
     */
    size_t order_of_key(const T& key) const {
        size_t rank = 0;
        Node* curr = root;
        while (curr) {
            if (key <= curr->key) {
                curr = curr->left;
            } else {
                rank += get_size(curr->left) + 1;
                curr = curr->right;
            }
        }
        return rank;
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Order-Statistic Tree C++17 unit tests..." << std::endl;

    dsa::OrderStatisticTree<int> ost(12345);

    // Test 1: Empty and size
    assert(ost.empty());
    assert(ost.size() == 0);

    // Test 2: Insert elements: {10, 20, 5, 15, 30}
    ost.insert(10);
    ost.insert(20);
    ost.insert(5);
    ost.insert(15);
    ost.insert(30);
    assert(ost.size() == 5);

    // Sorted keys should be: [5, 10, 15, 20, 30]
    // Test find_by_order (0-indexed select)
    assert(ost.find_by_order(0) == 5);
    assert(ost.find_by_order(1) == 10);
    assert(ost.find_by_order(2) == 15);
    assert(ost.find_by_order(3) == 20);
    assert(ost.find_by_order(4) == 30);

    // Test order_of_key (rank)
    assert(ost.order_of_key(5) == 0);
    assert(ost.order_of_key(10) == 1);
    assert(ost.order_of_key(12) == 2); // 5, 10 are smaller
    assert(ost.order_of_key(15) == 2);
    assert(ost.order_of_key(35) == 5); // all 5 are smaller

    // Test 3: Deletion
    ost.erase(15);
    assert(ost.size() == 4);
    // Remaining: [5, 10, 20, 30]
    assert(ost.find_by_order(2) == 20);
    assert(ost.order_of_key(20) == 2);

    // Test 4: Randomized stress verification against sorted vector oracle
    {
        dsa::OrderStatisticTree<int> stress_ost(999);
        std::vector<int> oracle;
        std::mt19937 gen(42);
        std::uniform_int_distribution<int> dist(1, 10000);

        for (int i = 0; i < 2000; ++i) {
            int val = dist(gen);
            if (std::find(oracle.begin(), oracle.end(), val) == oracle.end()) {
                stress_ost.insert(val);
                oracle.push_back(val);
                std::sort(oracle.begin(), oracle.end());
            }
        }

        assert(stress_ost.size() == oracle.size());
        for (size_t k = 0; k < oracle.size(); k += 50) {
            assert(stress_ost.find_by_order(k) == oracle[k]);
            assert(stress_ost.order_of_key(oracle[k]) == k);
        }
    }

    std::cout << "[PASS] All Order-Statistic Tree C++ unit tests passed." << std::endl;
    return 0;
}
