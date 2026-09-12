#include <iostream>
#include <vector>
#include <random>
#include <cassert>
#include <stdexcept>
#include <optional>

namespace dsa {

/**
 * @brief Randomized Cartesian Tree (Treap).
 * Combines Binary Search Tree on keys with Max-Heap on random priorities.
 * Provides expected O(log n) search, insert, and delete using split and merge.
 */
class Treap {
public:
    struct Node {
        int key;
        uint32_t priority;
        size_t size;
        Node* left;
        Node* right;

        Node(int k, uint32_t p)
            : key(k), priority(p), size(1), left(nullptr), right(nullptr) {}
    };

    Treap() : root_(nullptr), rng_(1337) {}

    explicit Treap(uint32_t seed) : root_(nullptr), rng_(seed) {}

    ~Treap() {
        clear(root_);
    }

    Treap(const Treap&) = delete;
    Treap& operator=(const Treap&) = delete;

    bool empty() const { return root_ == nullptr; }
    size_t size() const { return get_size(root_); }

    /**
     * @brief Inserts a key. If duplicate, returns false.
     */
    bool insert(int key) {
        if (contains(key)) return false;

        uint32_t p = rng_();
        Node* new_node = new Node(key, p);

        Node* left = nullptr;
        Node* right = nullptr;
        split(root_, key, left, right);

        root_ = merge(merge(left, new_node), right);
        return true;
    }

    /**
     * @brief Erases a key. Returns true if removed, false if not found.
     */
    bool erase(int key) {
        if (!contains(key)) return false;

        Node* left = nullptr;
        Node* mid = nullptr;
        Node* right = nullptr;

        // Split into < key and >= key
        split(root_, key - 1, left, right);
        // Split right into == key and > key
        split(right, key, mid, right);

        // Delete mid node
        delete mid;

        // Merge left and right back
        root_ = merge(left, right);
        return true;
    }

    /**
     * @brief Checks if a key exists in the Treap.
     */
    bool contains(int key) const {
        Node* cur = root_;
        while (cur) {
            if (cur->key == key) return true;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return false;
    }

    /**
     * @brief Finds the k-th smallest element (0-indexed).
     */
    std::optional<int> kth_element(size_t k) const {
        if (k >= size()) return std::nullopt;
        Node* cur = root_;
        while (cur) {
            size_t left_sz = get_size(cur->left);
            if (k == left_sz) {
                return cur->key;
            } else if (k < left_sz) {
                cur = cur->left;
            } else {
                k -= (left_sz + 1);
                cur = cur->right;
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Returns sorted keys via in-order traversal.
     */
    std::vector<int> inorder() const {
        std::vector<int> result;
        inorder_recursive(root_, result);
        return result;
    }

    /**
     * @brief Verifies that both BST and Heap invariants hold throughout the tree.
     */
    bool verify_invariants() const {
        return verify_node(root_, nullptr, nullptr);
    }

private:
    Node* root_;
    std::mt19937 rng_;

    static size_t get_size(Node* node) {
        return node ? node->size : 0;
    }

    static void update_size(Node* node) {
        if (node) {
            node->size = 1 + get_size(node->left) + get_size(node->right);
        }
    }

    void clear(Node* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

    /**
     * @brief Splits tree rooted at node into left (keys <= key) and right (keys > key).
     */
    void split(Node* node, int key, Node*& left, Node*& right) {
        if (!node) {
            left = nullptr;
            right = nullptr;
            return;
        }

        if (node->key <= key) {
            split(node->right, key, node->right, right);
            left = node;
        } else {
            split(node->left, key, left, node->left);
            right = node;
        }
        update_size(node);
    }

    /**
     * @brief Merges two treaps where all keys in left < all keys in right.
     */
    Node* merge(Node* left, Node* right) {
        if (!left) return right;
        if (!right) return left;

        if (left->priority >= right->priority) {
            left->right = merge(left->right, right);
            update_size(left);
            return left;
        } else {
            right->left = merge(left, right->left);
            update_size(right);
            return right;
        }
    }

    void inorder_recursive(Node* node, std::vector<int>& out) const {
        if (!node) return;
        inorder_recursive(node->left, out);
        out.push_back(node->key);
        inorder_recursive(node->right, out);
    }

    bool verify_node(Node* node, Node* min_node, Node* max_node) const {
        if (!node) return true;

        // BST invariant
        if (min_node && node->key <= min_node->key) return false;
        if (max_node && node->key >= max_node->key) return false;

        // Max-heap invariant
        if (node->left && node->left->priority > node->priority) return false;
        if (node->right && node->right->priority > node->priority) return false;

        // Subtree size check
        if (node->size != 1 + get_size(node->left) + get_size(node->right)) return false;

        return verify_node(node->left, min_node, node) &&
               verify_node(node->right, node, max_node);
    }
};

} // namespace dsa

// ============================================================================
// Unit Tests
// ============================================================================

void test_treap_basic() {
    dsa::Treap tr(42);
    assert(tr.empty());
    assert(tr.size() == 0);

    std::vector<int> keys = {50, 30, 70, 20, 40, 60, 80};
    for (int k : keys) {
        assert(tr.insert(k));
    }
    assert(tr.size() == 7);
    assert(!tr.insert(50)); // Duplicate rejected

    assert(tr.verify_invariants());

    // Search
    for (int k : keys) {
        assert(tr.contains(k));
    }
    assert(!tr.contains(999));
    assert(!tr.contains(10));

    // In-order traversal
    std::vector<int> expected = {20, 30, 40, 50, 60, 70, 80};
    assert(tr.inorder() == expected);

    // K-th element (0-indexed)
    assert(tr.kth_element(0).value() == 20);
    assert(tr.kth_element(3).value() == 50);
    assert(tr.kth_element(6).value() == 80);
    assert(tr.kth_element(7) == std::nullopt);
}

void test_treap_erase() {
    dsa::Treap tr(42);
    for (int k : {50, 30, 70, 20, 40, 60, 80}) {
        tr.insert(k);
    }

    assert(tr.erase(20));
    assert(!tr.contains(20));
    assert(tr.size() == 6);
    assert(tr.verify_invariants());

    assert(tr.erase(50)); // Root or internal node
    assert(!tr.contains(50));
    assert(tr.size() == 5);
    assert(tr.verify_invariants());

    std::vector<int> expected = {30, 40, 60, 70, 80};
    assert(tr.inorder() == expected);

    assert(!tr.erase(999));
}

int main() {
    std::cout << "Running Treap C++17 unit tests...\n";
    test_treap_basic();
    test_treap_erase();
    std::cout << "All Treap C++17 unit tests passed successfully!\n";
    return 0;
}
