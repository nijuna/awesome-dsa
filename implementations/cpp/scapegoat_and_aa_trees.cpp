/**
 * @file scapegoat_and_aa_trees.cpp
 * @brief Reference implementations of Scapegoat Trees and AA Trees.
 *
 * Implements:
 * 1. ScapegoatTree with alpha=2/3 weight balance, logarithmic height bound, and local rebuilding.
 * 2. AATree with Andersson's skew and split operations eliminating Red-Black complexity.
 * 3. Arthur's Two-Layer API and differential testing against std::map.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <cmath>
#include <map>
#include <random>
#include <functional>

namespace dsa {

// ============================================================================
// 1. Scapegoat Tree (alpha = 2/3)
// ============================================================================

template <typename Key, typename Value>
class ScapegoatTree {
private:
    struct Node {
        Key key;
        Value val;
        Node* left = nullptr;
        Node* right = nullptr;

        Node(const Key& k, const Value& v) : key(k), val(v) {}
    };

    Node* root_ = nullptr;
    size_t n_ = 0;   // Current number of elements
    size_t q_ = 0;   // Overestimate of n_ (max nodes since last total rebuild)

    static size_t subtree_size(Node* u) {
        if (!u) return 0;
        return 1 + subtree_size(u->left) + subtree_size(u->right);
    }

    static int max_height(size_t n) {
        if (n == 0) return 0;
        // log_{3/2}(n) = ln(n) / ln(1.5)
        return static_cast<int>(std::floor(std::log(n) / std::log(1.5)));
    }

    void flatten(Node* u, std::vector<Node*>& nodes) {
        if (!u) return;
        flatten(u->left, nodes);
        nodes.push_back(u);
        flatten(u->right, nodes);
    }

    Node* build_balanced(const std::vector<Node*>& nodes, int l, int r) {
        if (l > r) return nullptr;
        int mid = l + (r - l) / 2;
        Node* mid_node = nodes[mid];
        mid_node->left = build_balanced(nodes, l, mid - 1);
        mid_node->right = build_balanced(nodes, mid + 1, r);
        return mid_node;
    }

    Node* rebuild(Node* u) {
        std::vector<Node*> nodes;
        flatten(u, nodes);
        return build_balanced(nodes, 0, static_cast<int>(nodes.size()) - 1);
    }

    void destroy(Node* u) {
        if (!u) return;
        destroy(u->left);
        destroy(u->right);
        delete u;
    }

    bool verify_node(Node* u, const Key* min_k, const Key* max_k) const {
        if (!u) return true;
        if (min_k && !( *min_k < u->key )) return false;
        if (max_k && !( u->key < *max_k )) return false;
        return verify_node(u->left, min_k, &u->key) && verify_node(u->right, &u->key, max_k);
    }

public:
    ScapegoatTree() = default;

    ~ScapegoatTree() {
        destroy(root_);
    }

    ScapegoatTree(const ScapegoatTree&) = delete;
    ScapegoatTree& operator=(const ScapegoatTree&) = delete;

    size_t size() const { return n_; }
    bool empty() const { return n_ == 0; }

    // --- Layer A: Fast Core API ---

    const Key& min() const {
        assert(!empty());
        Node* curr = root_;
        while (curr->left) curr = curr->left;
        return curr->key;
    }

    const Key& max() const {
        assert(!empty());
        Node* curr = root_;
        while (curr->right) curr = curr->right;
        return curr->key;
    }

    // --- Layer B: Safe Adapter API ---

    const Key* try_min() const {
        if (empty()) return nullptr;
        return &min();
    }

    const Key* try_max() const {
        if (empty()) return nullptr;
        return &max();
    }

    const Value* try_find(const Key& key) const {
        Node* curr = root_;
        while (curr) {
            if (key < curr->key) curr = curr->left;
            else if (curr->key < key) curr = curr->right;
            else return &curr->val;
        }
        return nullptr;
    }

    Value* try_find(const Key& key) {
        Node* curr = root_;
        while (curr) {
            if (key < curr->key) curr = curr->left;
            else if (curr->key < key) curr = curr->right;
            else return &curr->val;
        }
        return nullptr;
    }

    void insert(const Key& key, const Value& val) {
        // Track path from root to insertion site
        std::vector<Node**> path;
        Node** curr = &root_;
        int depth = 0;

        while (*curr) {
            path.push_back(curr);
            depth++;
            if (key < (*curr)->key) {
                curr = &((*curr)->left);
            } else if ((*curr)->key < key) {
                curr = &((*curr)->right);
            } else {
                (*curr)->val = val; // Key already exists, update
                return;
            }
        }

        *curr = new Node(key, val);
        n_++;
        q_ = std::max(q_, n_);

        // Check if height exceeds maximum allowable height
        if (depth > max_height(n_)) {
            // Climb up to find the scapegoat: 3 * size(child) > 2 * size(parent)
            // Path holds pointers to ancestors
            int scapegoat_idx = -1;
            for (int i = static_cast<int>(path.size()) - 1; i >= 0; --i) {
                Node* p = *path[i];
                size_t p_size = subtree_size(p);
                size_t left_size = subtree_size(p->left);
                size_t right_size = subtree_size(p->right);
                if (3 * left_size > 2 * p_size || 3 * right_size > 2 * p_size) {
                    scapegoat_idx = i;
                    break;
                }
            }

            if (scapegoat_idx != -1) {
                Node** sg_ptr = path[scapegoat_idx];
                *sg_ptr = rebuild(*sg_ptr);
            } else {
                root_ = rebuild(root_);
            }
        }
    }

    bool erase(const Key& key) {
        Node** curr = &root_;
        while (*curr) {
            if (key < (*curr)->key) {
                curr = &((*curr)->left);
            } else if ((*curr)->key < key) {
                curr = &((*curr)->right);
            } else {
                // Found node to delete
                Node* to_delete = *curr;
                if (!to_delete->left) {
                    *curr = to_delete->right;
                } else if (!to_delete->right) {
                    *curr = to_delete->left;
                } else {
                    // Two children: find successor in right subtree
                    Node** succ = &(to_delete->right);
                    while ((*succ)->left) {
                        succ = &((*succ)->left);
                    }
                    Node* s = *succ;
                    *succ = s->right;
                    s->left = to_delete->left;
                    s->right = to_delete->right;
                    *curr = s;
                }
                delete to_delete;
                n_--;

                // Check if tree needs complete rebuilding: n_ < 2/3 * q_
                if (3 * n_ < 2 * q_) {
                    root_ = rebuild(root_);
                    q_ = n_;
                }
                return true;
            }
        }
        return false;
    }

    bool verify_invariants() const {
        if (!root_) return n_ == 0;
        return verify_node(root_, nullptr, nullptr);
    }
};

// ============================================================================
// 2. AA Tree (Andersson 1993)
// ============================================================================

template <typename Key, typename Value>
class AATree {
private:
    struct Node {
        Key key;
        Value val;
        int level = 1;
        Node* left = nullptr;
        Node* right = nullptr;

        Node(const Key& k, const Value& v, int lvl = 1)
            : key(k), val(v), level(lvl) {}
    };

    Node* root_ = nullptr;
    size_t size_ = 0;

    static Node* skew(Node* t) {
        if (!t || !t->left) return t;
        if (t->left->level == t->level) {
            Node* l = t->left;
            t->left = l->right;
            l->right = t;
            return l;
        }
        return t;
    }

    static Node* split(Node* t) {
        if (!t || !t->right || !t->right->right) return t;
        if (t->level == t->right->right->level) {
            Node* r = t->right;
            t->right = r->left;
            r->left = t;
            r->level++;
            return r;
        }
        return t;
    }

    Node* insert_internal(Node* t, const Key& key, const Value& val, bool& inserted) {
        if (!t) {
            inserted = true;
            return new Node(key, val, 1);
        }
        if (key < t->key) {
            t->left = insert_internal(t->left, key, val, inserted);
        } else if (t->key < key) {
            t->right = insert_internal(t->right, key, val, inserted);
        } else {
            t->val = val;
            inserted = false;
            return t;
        }

        t = skew(t);
        t = split(t);
        return t;
    }

    Node* erase_internal(Node* t, const Key& key, bool& erased) {
        if (!t) return nullptr;

        if (key < t->key) {
            t->left = erase_internal(t->left, key, erased);
        } else if (t->key < key) {
            t->right = erase_internal(t->right, key, erased);
        } else {
            erased = true;
            if (!t->left && !t->right) {
                delete t;
                return nullptr;
            } else if (!t->left) {
                Node* succ = t->right;
                while (succ->left) succ = succ->left;
                t->key = succ->key;
                t->val = succ->val;
                t->right = erase_internal(t->right, succ->key, erased);
            } else {
                Node* pred = t->left;
                while (pred->right) pred = pred->right;
                t->key = pred->key;
                t->val = pred->val;
                t->left = erase_internal(t->left, pred->key, erased);
            }
        }

        // Rebalance after deletion: lower level if necessary (Andersson's decrease_level)
        auto level_of = [](Node* n) -> int { return n ? n->level : 0; };
        int should_be = std::min(level_of(t->left), level_of(t->right)) + 1;
        if (should_be < t->level) {
            t->level = should_be;
            if (t->right && t->right->level > should_be) {
                t->right->level = should_be;
            }
        }

        t = skew(t);
        if (t->right) {
            t->right = skew(t->right);
            if (t->right->right) {
                t->right->right = skew(t->right->right);
            }
        }
        t = split(t);
        if (t->right) {
            t->right = split(t->right);
        }
        return t;
    }

    void destroy(Node* t) {
        if (!t) return;
        destroy(t->left);
        destroy(t->right);
        delete t;
    }

    bool verify_node(Node* t, const Key* min_k, const Key* max_k) const {
        if (!t) return true;
        if (min_k && !( *min_k < t->key )) return false;
        if (max_k && !( t->key < *max_k )) return false;

        // AA Tree Invariants:
        // 1. Leaf level is 1
        if (!t->left && !t->right && t->level != 1) return false;
        // 2. Left child level is parent->level - 1
        if (t->left && t->left->level != t->level - 1) return false;
        // 3. Right child level is parent->level or parent->level - 1
        if (t->right && !(t->right->level == t->level || t->right->level == t->level - 1)) return false;
        // 4. Right grandchild level is < parent->level
        if (t->right && t->right->right && t->right->right->level >= t->level) return false;

        return verify_node(t->left, min_k, &t->key) && verify_node(t->right, &t->key, max_k);
    }

public:
    AATree() = default;
    ~AATree() { destroy(root_); }

    AATree(const AATree&) = delete;
    AATree& operator=(const AATree&) = delete;

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // --- Layer A: Fast Core API ---

    const Key& min() const {
        assert(!empty());
        Node* curr = root_;
        while (curr->left) curr = curr->left;
        return curr->key;
    }

    const Key& max() const {
        assert(!empty());
        Node* curr = root_;
        while (curr->right) curr = curr->right;
        return curr->key;
    }

    // --- Layer B: Safe Adapter API ---

    const Key* try_min() const {
        if (empty()) return nullptr;
        return &min();
    }

    const Key* try_max() const {
        if (empty()) return nullptr;
        return &max();
    }

    const Value* try_find(const Key& key) const {
        Node* curr = root_;
        while (curr) {
            if (key < curr->key) curr = curr->left;
            else if (curr->key < key) curr = curr->right;
            else return &curr->val;
        }
        return nullptr;
    }

    void insert(const Key& key, const Value& val) {
        bool inserted = false;
        root_ = insert_internal(root_, key, val, inserted);
        if (inserted) size_++;
    }

    bool erase(const Key& key) {
        bool erased = false;
        root_ = erase_internal(root_, key, erased);
        if (erased) size_--;
        return erased;
    }

    bool verify_invariants() const {
        if (!root_) return size_ == 0;
        return verify_node(root_, nullptr, nullptr);
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Scapegoat and AA Tree verification..." << std::endl;

    dsa::ScapegoatTree<int, int> sg;
    dsa::AATree<int, int> aa;
    std::map<int, int> oracle;

    std::mt19937_64 rng(42);
    const int OPS = 1000;

    for (int step = 0; step < OPS; ++step) {
        int op = rng() % 4;
        int key = rng() % 250;

        if (op == 0) {
            // Insert
            int val = key * 7;
            sg.insert(key, val);
            aa.insert(key, val);
            oracle[key] = val;
        } else if (op == 1) {
            // Erase
            bool r_sg = sg.erase(key);
            bool r_aa = aa.erase(key);
            bool r_or = oracle.erase(key) > 0;
            assert(r_sg == r_or);
            assert(r_aa == r_or);
        } else if (op == 2) {
            // Find
            const int* v_sg = sg.try_find(key);
            const int* v_aa = aa.try_find(key);
            auto it = oracle.find(key);
            if (it == oracle.end()) {
                assert(v_sg == nullptr);
                assert(v_aa == nullptr);
            } else {
                assert(v_sg != nullptr && *v_sg == it->second);
                assert(v_aa != nullptr && *v_aa == it->second);
            }
        } else {
            // Min & Max
            if (!oracle.empty()) {
                assert(sg.min() == oracle.begin()->first);
                assert(sg.max() == oracle.rbegin()->first);
                assert(aa.min() == oracle.begin()->first);
                assert(aa.max() == oracle.rbegin()->first);
            } else {
                assert(sg.empty());
                assert(aa.empty());
            }
        }

        assert(sg.size() == oracle.size());
        assert(aa.size() == oracle.size());
        assert(sg.verify_invariants());
        assert(aa.verify_invariants());
    }

    std::cout << "[PASS] 1000 differential operations on ScapegoatTree matched std::map." << std::endl;
    std::cout << "[PASS] 1000 differential operations on AATree matched std::map and verified AA invariants." << std::endl;
    std::cout << "All Scapegoat and AA Tree assertions passed successfully!" << std::endl;
    return 0;
}
