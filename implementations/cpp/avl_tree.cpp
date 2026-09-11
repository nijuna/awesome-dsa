/**
 * Reference Implementation: Self-Balancing AVL Tree
 * Demonstrates strict balance factor maintenance (BF in {-1, 0, 1}),
 * LL/RR/LR/RL rotations, deterministic O(log n) search/insert/erase,
 * and height bound guarantees.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <optional>
#include <cassert>
#include <cstddef>
#include <cmath>

template <typename Key, typename Value>
class AVLTree {
public:
    struct Node {
        Key key;
        Value val;
        int height;
        Node* left;
        Node* right;

        Node(const Key& k, const Value& v)
            : key(k), val(v), height(1), left(nullptr), right(nullptr) {}
    };

private:
    Node* root_;
    std::size_t size_;

    static int get_height(const Node* n) noexcept {
        return n ? n->height : 0;
    }

    static int get_balance_factor(const Node* n) noexcept {
        return n ? get_height(n->left) - get_height(n->right) : 0;
    }

    static void update_height(Node* n) noexcept {
        if (n) {
            n->height = 1 + std::max(get_height(n->left), get_height(n->right));
        }
    }

    // Right rotation around y:
    //      y             x
    //     / \           / \ .
    //    x   C   ==>   A   y
    //   / \               / \ .
    //  A   B             B   C
    static Node* rotate_right(Node* y) noexcept {
        Node* x = y->left;
        Node* b = x->right;

        x->right = y;
        y->left = b;

        update_height(y);
        update_height(x);

        return x;
    }

    // Left rotation around x:
    //    x               y
    //   / \             / \ .
    //  A   y    ==>    x   C
    //     / \         / \ .
    //    B   C       A   B
    static Node* rotate_left(Node* x) noexcept {
        Node* y = x->right;
        Node* b = y->left;

        y->left = x;
        x->right = b;

        update_height(x);
        update_height(y);

        return y;
    }

    // Rebalance a node if balance factor is violated
    static Node* rebalance(Node* n) noexcept {
        update_height(n);
        int bf = get_balance_factor(n);

        // Left heavy
        if (bf > 1) {
            // LR case: rotate left at child first
            if (get_balance_factor(n->left) < 0) {
                n->left = rotate_left(n->left);
            }
            // LL case: rotate right
            return rotate_right(n);
        }

        // Right heavy
        if (bf < -1) {
            // RL case: rotate right at child first
            if (get_balance_factor(n->right) > 0) {
                n->right = rotate_right(n->right);
            }
            // RR case: rotate left
            return rotate_left(n);
        }

        return n;
    }

    Node* insert_internal(Node* n, const Key& key, const Value& val, bool& inserted) {
        if (!n) {
            inserted = true;
            return new Node(key, val);
        }

        if (key < n->key) {
            n->left = insert_internal(n->left, key, val, inserted);
        } else if (key > n->key) {
            n->right = insert_internal(n->right, key, val, inserted);
        } else {
            // Key exists: update value without increasing size
            n->val = val;
            inserted = false;
            return n;
        }

        return rebalance(n);
    }

    static Node* find_min(Node* n) noexcept {
        while (n && n->left) {
            n = n->left;
        }
        return n;
    }

    Node* erase_internal(Node* n, const Key& key, bool& erased) {
        if (!n) {
            erased = false;
            return nullptr;
        }

        if (key < n->key) {
            n->left = erase_internal(n->left, key, erased);
        } else if (key > n->key) {
            n->right = erase_internal(n->right, key, erased);
        } else {
            // Node found
            erased = true;
            if (!n->left || !n->right) {
                Node* temp = n->left ? n->left : n->right;
                delete n;
                return temp;
            }

            // Node with two children: replace with in-order successor
            Node* successor = find_min(n->right);
            n->key = successor->key;
            n->val = successor->val;
            n->right = erase_internal(n->right, successor->key, erased);
        }

        return rebalance(n);
    }

    static void destroy_tree(Node* n) noexcept {
        if (n) {
            destroy_tree(n->left);
            destroy_tree(n->right);
            delete n;
        }
    }

    static void inorder_internal(const Node* n, std::vector<Key>& out) {
        if (!n) return;
        inorder_internal(n->left, out);
        out.push_back(n->key);
        inorder_internal(n->right, out);
    }

    static bool verify_avl_invariants(const Node* n) {
        if (!n) return true;
        int bf = get_balance_factor(n);
        if (bf < -1 || bf > 1) return false;
        int expected_h = 1 + std::max(get_height(n->left), get_height(n->right));
        if (n->height != expected_h) return false;
        return verify_avl_invariants(n->left) && verify_avl_invariants(n->right);
    }

public:
    AVLTree() : root_(nullptr), size_(0) {}

    ~AVLTree() {
        destroy_tree(root_);
    }

    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    AVLTree(AVLTree&& other) noexcept : root_(other.root_), size_(other.size_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    AVLTree& operator=(AVLTree&& other) noexcept {
        if (this != &other) {
            destroy_tree(root_);
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] int height() const noexcept { return get_height(root_); }

    void insert(const Key& key, const Value& val) {
        bool inserted = false;
        root_ = insert_internal(root_, key, val, inserted);
        if (inserted) {
            ++size_;
        }
    }

    bool erase(const Key& key) {
        bool erased = false;
        root_ = erase_internal(root_, key, erased);
        if (erased) {
            --size_;
        }
        return erased;
    }

    std::optional<Value> find(const Key& key) const {
        Node* curr = root_;
        while (curr) {
            if (key < curr->key) {
                curr = curr->left;
            } else if (key > curr->key) {
                curr = curr->right;
            } else {
                return curr->val;
            }
        }
        return std::nullopt;
    }

    bool contains(const Key& key) const noexcept {
        return find(key).has_value();
    }

    std::vector<Key> inorder_keys() const {
        std::vector<Key> out;
        out.reserve(size_);
        inorder_internal(root_, out);
        return out;
    }

    [[nodiscard]] bool is_valid_avl() const {
        return verify_avl_invariants(root_);
    }
};

int main() {
    AVLTree<int, std::string> tree;
    assert(tree.empty());
    assert(tree.size() == 0);
    assert(tree.height() == 0);
    assert(tree.is_valid_avl());

    // 1. Sequential insertion (testing RR rotation cascade)
    for (int i = 1; i <= 100; ++i) {
        tree.insert(i, "val_" + std::to_string(i));
        assert(tree.is_valid_avl());
    }
    assert(tree.size() == 100);
    // AVL bound: h < 1.44 * log2(102) ≈ 9.6 -> height must be <= 9
    assert(tree.height() <= 9);

    // 2. Inorder sorted check
    std::vector<int> keys = tree.inorder_keys();
    assert(keys.size() == 100);
    for (int i = 0; i < 100; ++i) {
        assert(keys[i] == i + 1);
    }

    // 3. Lookup checks
    for (int i = 1; i <= 100; ++i) {
        auto val = tree.find(i);
        assert(val.has_value());
        assert(*val == "val_" + std::to_string(i));
    }
    assert(!tree.find(0).has_value());
    assert(!tree.find(101).has_value());

    // 4. Update existing key
    tree.insert(50, "updated_50");
    assert(tree.size() == 100);
    assert(tree.find(50).value() == "updated_50");

    // 5. Deletions: leaf, single child, two children
    assert(tree.erase(1));  // min leaf
    assert(tree.is_valid_avl());
    assert(tree.size() == 99);
    assert(!tree.contains(1));

    assert(tree.erase(50)); // two children replacement
    assert(tree.is_valid_avl());
    assert(tree.size() == 98);
    assert(!tree.contains(50));

    // Erase non-existent
    assert(!tree.erase(999));
    assert(tree.size() == 98);

    // 6. Delete all remaining elements
    for (int i = 2; i <= 100; ++i) {
        if (i != 50) {
            assert(tree.erase(i));
            assert(tree.is_valid_avl());
        }
    }
    assert(tree.empty());
    assert(tree.size() == 0);
    assert(tree.height() == 0);

    // 7. Reverse insertion (testing LL rotation cascade)
    for (int i = 100; i >= 1; --i) {
        tree.insert(i, "rev_" + std::to_string(i));
        assert(tree.is_valid_avl());
    }
    assert(tree.size() == 100);
    assert(tree.height() <= 9);

    // 8. Zig-Zag insertions (testing LR and RL rotations)
    AVLTree<int, int> zigzag;
    zigzag.insert(30, 30);
    zigzag.insert(10, 10);
    zigzag.insert(20, 20); // Triggers LR rotation
    assert(zigzag.is_valid_avl());
    assert(zigzag.height() == 2);

    zigzag.insert(50, 50);
    zigzag.insert(40, 40); // Triggers RL rotation
    assert(zigzag.is_valid_avl());

    std::cout << "[PASS] All AVL Tree C++ unit tests passed.\n";
    return 0;
}
