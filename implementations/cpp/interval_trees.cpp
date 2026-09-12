/**
 * @file interval_trees.cpp
 * @brief Reference implementation of Augmented Interval Trees with Subtree Max Augmentation.
 *
 * Implements balanced Interval Tree (augmented Treap) for dynamic interval storage,
 * point stabbing queries, single-overlap search pruning, and all-overlaps reporting.
 * Follows Arthur's Two-Layer API and differential testing against linear vector oracle.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

namespace dsa {

struct Interval {
    int64_t low = 0;
    int64_t high = 0;

    Interval() = default;
    Interval(int64_t l, int64_t h) : low(l), high(h) {
        assert(low <= high);
    }

    bool overlaps(const Interval& o) const {
        return low <= o.high && o.low <= high;
    }

    bool operator==(const Interval& o) const {
        return low == o.low && high == o.high;
    }

    bool operator<(const Interval& o) const {
        if (low != o.low) return low < o.low;
        return high < o.high;
    }
};

/**
 * @brief Augmented Balanced Interval Tree.
 * Each node stores an interval [low, high] and max_high of its subtree.
 */
class IntervalTree {
private:
    struct Node {
        Interval intv;
        int64_t max_high;
        uint32_t priority;
        Node* left = nullptr;
        Node* right = nullptr;

        Node(const Interval& i, uint32_t prio)
            : intv(i), max_high(i.high), priority(prio) {}
    };

    Node* root_ = nullptr;
    size_t size_ = 0;
    std::mt19937 rng_{42};

    static int64_t get_max_high(Node* n) {
        return n ? n->max_high : INT64_MIN;
    }

    static void update_node(Node* n) {
        if (!n) return;
        n->max_high = n->intv.high;
        if (n->left) n->max_high = std::max(n->max_high, n->left->max_high);
        if (n->right) n->max_high = std::max(n->max_high, n->right->max_high);
    }

    Node* rotate_right(Node* y) {
        Node* x = y->left;
        y->left = x->right;
        x->right = y;
        update_node(y);
        update_node(x);
        return x;
    }

    Node* rotate_left(Node* x) {
        Node* y = x->right;
        x->right = y->left;
        y->left = x;
        update_node(x);
        update_node(y);
        return y;
    }

    Node* insert_internal(Node* t, const Interval& intv, uint32_t prio, bool& inserted) {
        if (!t) {
            inserted = true;
            return new Node(intv, prio);
        }

        if (intv == t->intv) {
            inserted = false;
            return t;
        }

        if (intv < t->intv) {
            t->left = insert_internal(t->left, intv, prio, inserted);
            if (t->left->priority > t->priority) {
                t = rotate_right(t);
            }
        } else {
            t->right = insert_internal(t->right, intv, prio, inserted);
            if (t->right->priority > t->priority) {
                t = rotate_left(t);
            }
        }

        update_node(t);
        return t;
    }

    Node* erase_internal(Node* t, const Interval& intv, bool& erased) {
        if (!t) return nullptr;

        if (intv < t->intv) {
            t->left = erase_internal(t->left, intv, erased);
        } else if (t->intv < intv) {
            t->right = erase_internal(t->right, intv, erased);
        } else {
            // Found node to erase
            erased = true;
            if (!t->left && !t->right) {
                delete t;
                return nullptr;
            } else if (!t->left) {
                Node* r = t->right;
                delete t;
                return r;
            } else if (!t->right) {
                Node* l = t->left;
                delete t;
                return l;
            } else {
                // Rotate with higher priority child
                if (t->left->priority > t->right->priority) {
                    t = rotate_right(t);
                    t->right = erase_internal(t->right, intv, erased);
                } else {
                    t = rotate_left(t);
                    t->left = erase_internal(t->left, intv, erased);
                }
            }
        }

        update_node(t);
        return t;
    }

    void find_all_internal(Node* t, const Interval& q, std::vector<Interval>& out) const {
        if (!t) return;

        // 1. Search left if left subtree can contain an overlap
        if (t->left && t->left->max_high >= q.low) {
            find_all_internal(t->left, q, out);
        }

        // 2. Check current node
        if (t->intv.overlaps(q)) {
            out.push_back(t->intv);
        }

        // 3. Search right only if right subtree could have an interval with low <= q.high
        if (t->right && t->intv.low <= q.high) {
            find_all_internal(t->right, q, out);
        }
    }

    void destroy(Node* t) {
        if (!t) return;
        destroy(t->left);
        destroy(t->right);
        delete t;
    }

    bool verify_node(Node* t, const Interval* min_i, const Interval* max_i) const {
        if (!t) return true;
        if (min_i && !( *min_i < t->intv )) return false;
        if (max_i && !( t->intv < *max_i )) return false;

        int64_t expected_max = t->intv.high;
        if (t->left) expected_max = std::max(expected_max, t->left->max_high);
        if (t->right) expected_max = std::max(expected_max, t->right->max_high);
        if (t->max_high != expected_max) return false;

        return verify_node(t->left, min_i, &t->intv) && verify_node(t->right, &t->intv, max_i);
    }

public:
    IntervalTree() = default;

    ~IntervalTree() {
        destroy(root_);
    }

    IntervalTree(const IntervalTree&) = delete;
    IntervalTree& operator=(const IntervalTree&) = delete;

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // --- Layer A: Fast Core API (Preconditioned) ---

    const Interval& find_overlap(const Interval& q) const {
        const Interval* res = try_find_overlap(q);
        assert(res != nullptr);
        return *res;
    }

    void insert(int64_t low, int64_t high) {
        Interval intv(low, high);
        bool inserted = false;
        root_ = insert_internal(root_, intv, rng_(), inserted);
        if (inserted) size_++;
    }

    bool erase(int64_t low, int64_t high) {
        Interval intv(low, high);
        bool erased = false;
        root_ = erase_internal(root_, intv, erased);
        if (erased) size_--;
        return erased;
    }

    // --- Layer B: Safe Adapter API ---

    const Interval* try_find_overlap(const Interval& q) const {
        Node* curr = root_;
        while (curr) {
            if (curr->intv.overlaps(q)) {
                return &curr->intv;
            }
            if (curr->left && curr->left->max_high >= q.low) {
                curr = curr->left;
            } else {
                curr = curr->right;
            }
        }
        return nullptr;
    }

    std::vector<Interval> find_all_overlaps(const Interval& q) const {
        std::vector<Interval> out;
        find_all_internal(root_, q, out);
        return out;
    }

    std::vector<Interval> stabbing_query(int64_t point) const {
        return find_all_overlaps(Interval(point, point));
    }

    bool verify_invariants() const {
        if (!root_) return size_ == 0;
        return verify_node(root_, nullptr, nullptr);
    }
};

/**
 * @brief Naive vector oracle for differential verification.
 */
class NaiveIntervalOracle {
private:
    std::vector<Interval> intervals_;

public:
    void insert(int64_t low, int64_t high) {
        Interval intv(low, high);
        if (std::find(intervals_.begin(), intervals_.end(), intv) == intervals_.end()) {
            intervals_.push_back(intv);
        }
    }

    bool erase(int64_t low, int64_t high) {
        Interval intv(low, high);
        auto it = std::find(intervals_.begin(), intervals_.end(), intv);
        if (it != intervals_.end()) {
            intervals_.erase(it);
            return true;
        }
        return false;
    }

    bool has_overlap(const Interval& q) const {
        for (const auto& i : intervals_) {
            if (i.overlaps(q)) return true;
        }
        return false;
    }

    std::vector<Interval> find_all_overlaps(const Interval& q) const {
        std::vector<Interval> out;
        for (const auto& i : intervals_) {
            if (i.overlaps(q)) out.push_back(i);
        }
        std::sort(out.begin(), out.end());
        return out;
    }

    size_t size() const { return intervals_.size(); }
};

} // namespace dsa

int main() {
    std::cout << "Running Interval Tree verification..." << std::endl;

    dsa::IntervalTree itree;
    dsa::NaiveIntervalOracle oracle;

    // 1. Classical Textbook Interval Set (CLRS Fig 14.4)
    std::vector<std::pair<int64_t, int64_t>> textbook = {
        {16, 21}, {8, 9}, {25, 30}, {5, 8}, {15, 23},
        {17, 19}, {26, 26}, {0, 3}, {6, 10}, {19, 20}
    };

    for (const auto& p : textbook) {
        itree.insert(p.first, p.second);
        oracle.insert(p.first, p.second);
    }

    assert(itree.size() == 10);
    assert(itree.verify_invariants());

    // Query [22, 25]: Should find [15, 23]
    dsa::Interval q1(22, 25);
    const dsa::Interval* found1 = itree.try_find_overlap(q1);
    assert(found1 != nullptr && found1->overlaps(q1));

    // Stabbing query at point 7: Should hit [5, 8], [6, 10]
    auto stab7 = itree.stabbing_query(7);
    assert(stab7.size() == 2);

    // 2. Randomized Differential Fuzzing vs Naive Oracle
    std::mt19937 rng(1337);
    const int OPS = 1000;

    for (int step = 0; step < OPS; ++step) {
        int op = rng() % 4;
        int64_t l = rng() % 100;
        int64_t h = l + (rng() % 30);

        if (op == 0) {
            // Insert
            itree.insert(l, h);
            oracle.insert(l, h);
        } else if (op == 1) {
            // Erase
            bool r1 = itree.erase(l, h);
            bool r2 = oracle.erase(l, h);
            assert(r1 == r2);
        } else if (op == 2) {
            // Single overlap query
            dsa::Interval q(l, h);
            const dsa::Interval* res = itree.try_find_overlap(q);
            bool expected_overlap = oracle.has_overlap(q);
            if (expected_overlap) {
                assert(res != nullptr);
                assert(res->overlaps(q));
            } else {
                assert(res == nullptr);
            }
        } else {
            // All overlaps query
            dsa::Interval q(l, h);
            auto all_tree = itree.find_all_overlaps(q);
            auto all_oracle = oracle.find_all_overlaps(q);
            std::sort(all_tree.begin(), all_tree.end());
            assert(all_tree == all_oracle);
        }

        assert(itree.size() == oracle.size());
        assert(itree.verify_invariants());
    }

    std::cout << "[PASS] Textbook intervals and stabbing query verified." << std::endl;
    std::cout << "[PASS] 1000 differential operations matched naive oracle bit-for-bit." << std::endl;
    std::cout << "All Interval Tree assertions passed successfully!" << std::endl;
    return 0;
}
