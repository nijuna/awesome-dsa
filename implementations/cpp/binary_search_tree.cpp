#include <iostream>
#include <vector>
#include <optional>
#include <cassert>
#include <stdexcept>

namespace dsa {

/**
 * @brief Binary Search Tree (BST) supporting search, insertion, and deletion.
 *
 * Invariants:
 * - For any node x, all keys in left(x) < key(x)
 * - All keys in right(x) > key(x)
 *
 * Complexity:
 * - Average case (balanced/random): O(log n) search, insert, delete.
 * - Worst case (degenerate/skewed): O(n) search, insert, delete.
 */
class BinarySearchTree {
public:
    struct Node {
        int key;
        Node* left;
        Node* right;
        explicit Node(int k) : key(k), left(nullptr), right(nullptr) {}
    };

    BinarySearchTree() : root_(nullptr), size_(0) {}

    ~BinarySearchTree() {
        clear(root_);
    }

    BinarySearchTree(const BinarySearchTree&) = delete;
    BinarySearchTree& operator=(const BinarySearchTree&) = delete;

    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }

    /**
     * @brief Inserts a key into the BST. If duplicate, returns false.
     */
    bool insert(int key) {
        if (!root_) {
            root_ = new Node(key);
            ++size_;
            return true;
        }

        Node* cur = root_;
        while (true) {
            if (key == cur->key) {
                return false; // Duplicate
            } else if (key < cur->key) {
                if (!cur->left) {
                    cur->left = new Node(key);
                    ++size_;
                    return true;
                }
                cur = cur->left;
            } else {
                if (!cur->right) {
                    cur->right = new Node(key);
                    ++size_;
                    return true;
                }
                cur = cur->right;
            }
        }
    }

    /**
     * @brief Searches for a key iteratively.
     */
    bool contains(int key) const {
        Node* cur = root_;
        while (cur) {
            if (key == cur->key) return true;
            cur = (key < cur->key) ? cur->left : cur->right;
        }
        return false;
    }

    /**
     * @brief Removes a key handling all 3 cases: leaf, 1 child, 2 children.
     */
    bool remove(int key) {
        bool removed = false;
        root_ = remove_node(root_, key, removed);
        if (removed) --size_;
        return removed;
    }

    /**
     * @brief Returns in-order traversal of keys (sorted order).
     */
    std::vector<int> inorder() const {
        std::vector<int> result;
        inorder_recursive(root_, result);
        return result;
    }

    std::optional<int> min_value() const {
        if (!root_) return std::nullopt;
        Node* cur = root_;
        while (cur->left) cur = cur->left;
        return cur->key;
    }

    std::optional<int> max_value() const {
        if (!root_) return std::nullopt;
        Node* cur = root_;
        while (cur->right) cur = cur->right;
        return cur->key;
    }

    /**
     * @brief Finds the in-order successor of a key (smallest key strictly greater).
     */
    std::optional<int> successor(int key) const {
        Node* cur = root_;
        Node* succ = nullptr;
        while (cur) {
            if (key < cur->key) {
                succ = cur;
                cur = cur->left;
            } else {
                cur = cur->right;
            }
        }
        if (succ) return succ->key;
        return std::nullopt;
    }

    /**
     * @brief Finds the in-order predecessor of a key (largest key strictly smaller).
     */
    std::optional<int> predecessor(int key) const {
        Node* cur = root_;
        Node* pred = nullptr;
        while (cur) {
            if (key > cur->key) {
                pred = cur;
                cur = cur->right;
            } else {
                cur = cur->left;
            }
        }
        if (pred) return pred->key;
        return std::nullopt;
    }

private:
    Node* root_;
    size_t size_;

    void clear(Node* node) {
        if (!node) return;
        clear(node->left);
        clear(node->right);
        delete node;
    }

    Node* remove_node(Node* node, int key, bool& removed) {
        if (!node) return nullptr;

        if (key < node->key) {
            node->left = remove_node(node->left, key, removed);
        } else if (key > node->key) {
            node->right = remove_node(node->right, key, removed);
        } else {
            removed = true;
            // Case 1 & 2: 0 or 1 child
            if (!node->left) {
                Node* right_child = node->right;
                delete node;
                return right_child;
            } else if (!node->right) {
                Node* left_child = node->left;
                delete node;
                return left_child;
            }

            // Case 3: 2 children - find in-order successor (min in right subtree)
            Node* succ = node->right;
            while (succ->left) {
                succ = succ->left;
            }
            node->key = succ->key;
            node->right = remove_node(node->right, succ->key, removed);
        }
        return node;
    }

    void inorder_recursive(Node* node, std::vector<int>& out) const {
        if (!node) return;
        inorder_recursive(node->left, out);
        out.push_back(node->key);
        inorder_recursive(node->right, out);
    }
};

} // namespace dsa

// ============================================================================
// Unit Tests
// ============================================================================

void test_bst_basic() {
    dsa::BinarySearchTree bst;
    assert(bst.empty());
    assert(bst.size() == 0);

    assert(bst.insert(50));
    assert(bst.insert(30));
    assert(bst.insert(70));
    assert(bst.insert(20));
    assert(bst.insert(40));
    assert(bst.insert(60));
    assert(bst.insert(80));

    // Duplicate rejection
    assert(!bst.insert(50));
    assert(bst.size() == 7);

    // Search
    assert(bst.contains(50));
    assert(bst.contains(20));
    assert(bst.contains(80));
    assert(!bst.contains(99));
    assert(!bst.contains(10));

    // In-order traversal must be sorted
    std::vector<int> expected = {20, 30, 40, 50, 60, 70, 80};
    assert(bst.inorder() == expected);

    assert(bst.min_value().value() == 20);
    assert(bst.max_value().value() == 80);
}

void test_bst_predecessor_successor() {
    dsa::BinarySearchTree bst;
    for (int x : {50, 30, 70, 20, 40, 60, 80}) {
        bst.insert(x);
    }

    assert(bst.successor(40).value() == 50);
    assert(bst.successor(50).value() == 60);
    assert(bst.successor(80) == std::nullopt);

    assert(bst.predecessor(50).value() == 40);
    assert(bst.predecessor(20) == std::nullopt);
    assert(bst.predecessor(80).value() == 70);
}

void test_bst_deletions() {
    dsa::BinarySearchTree bst;
    for (int x : {50, 30, 70, 20, 40, 60, 80}) {
        bst.insert(x);
    }

    // Case 1: Delete leaf node (20)
    assert(bst.remove(20));
    assert(!bst.contains(20));
    assert(bst.size() == 6);
    std::vector<int> exp1 = {30, 40, 50, 60, 70, 80};
    assert(bst.inorder() == exp1);

    // Case 2: Delete node with 1 child (30 now has only right child 40)
    assert(bst.remove(30));
    assert(!bst.contains(30));
    assert(bst.size() == 5);
    std::vector<int> exp2 = {40, 50, 60, 70, 80};
    assert(bst.inorder() == exp2);

    // Case 3: Delete node with 2 children (root 50 has left 40 and right 70)
    assert(bst.remove(50));
    assert(!bst.contains(50));
    assert(bst.size() == 4);
    std::vector<int> exp3 = {40, 60, 70, 80};
    assert(bst.inorder() == exp3);

    // Remove non-existent key
    assert(!bst.remove(999));
    assert(bst.size() == 4);
}

int main() {
    std::cout << "Running Binary Search Tree C++17 unit tests...\n";
    test_bst_basic();
    test_bst_predecessor_successor();
    test_bst_deletions();
    std::cout << "All Binary Search Tree C++17 unit tests passed successfully!\n";
    return 0;
}
