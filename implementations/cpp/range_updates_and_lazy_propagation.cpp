#include <cassert>
#include <algorithm>
#include <iostream>
#include <vector>
#include <limits>

namespace dsa {

/**
 * @brief Segment Tree with Lazy Propagation supporting Range Add and Range Set updates.
 * Range queries: Sum and Min.
 * Time complexity: O(log N) for both range updates and range queries.
 */
class LazySegmentTree {
private:
    struct Node {
        int64_t sum = 0;
        int64_t min_val = std::numeric_limits<int64_t>::max();
        int64_t lazy_add = 0;
        int64_t lazy_set = 0;
        bool has_set = false;
    };

    size_t n_;
    std::vector<Node> tree_;

    void apply_set(size_t node, size_t l, size_t r, int64_t val) {
        tree_[node].has_set = true;
        tree_[node].lazy_set = val;
        tree_[node].lazy_add = 0; // Set overrides any previous adds
        tree_[node].sum = val * static_cast<int64_t>(r - l + 1);
        tree_[node].min_val = val;
    }

    void apply_add(size_t node, size_t l, size_t r, int64_t val) {
        if (tree_[node].has_set) {
            tree_[node].lazy_set += val;
        } else {
            tree_[node].lazy_add += val;
        }
        tree_[node].sum += val * static_cast<int64_t>(r - l + 1);
        tree_[node].min_val += val;
    }

    void push_down(size_t node, size_t l, size_t r) {
        if (l == r) return;
        size_t mid = l + (r - l) / 2;
        size_t left_child = 2 * node;
        size_t right_child = 2 * node + 1;

        if (tree_[node].has_set) {
            apply_set(left_child, l, mid, tree_[node].lazy_set);
            apply_set(right_child, mid + 1, r, tree_[node].lazy_set);
            tree_[node].has_set = false;
            tree_[node].lazy_set = 0;
        }

        if (tree_[node].lazy_add != 0) {
            apply_add(left_child, l, mid, tree_[node].lazy_add);
            apply_add(right_child, mid + 1, r, tree_[node].lazy_add);
            tree_[node].lazy_add = 0;
        }
    }

    void push_up(size_t node) {
        tree_[node].sum = tree_[2 * node].sum + tree_[2 * node + 1].sum;
        tree_[node].min_val = std::min(tree_[2 * node].min_val, tree_[2 * node + 1].min_val);
    }

    void build(size_t node, size_t l, size_t r, const std::vector<int64_t>& a) {
        if (l == r) {
            tree_[node].sum = a[l];
            tree_[node].min_val = a[l];
            return;
        }
        size_t mid = l + (r - l) / 2;
        build(2 * node, l, mid, a);
        build(2 * node + 1, mid + 1, r, a);
        push_up(node);
    }

    void range_add(size_t node, size_t l, size_t r, size_t ql, size_t qr, int64_t val) {
        if (ql <= l && r <= qr) {
            apply_add(node, l, r, val);
            return;
        }
        push_down(node, l, r);
        size_t mid = l + (r - l) / 2;
        if (ql <= mid) range_add(2 * node, l, mid, ql, qr, val);
        if (qr > mid) range_add(2 * node + 1, mid + 1, r, ql, qr, val);
        push_up(node);
    }

    void range_set(size_t node, size_t l, size_t r, size_t ql, size_t qr, int64_t val) {
        if (ql <= l && r <= qr) {
            apply_set(node, l, r, val);
            return;
        }
        push_down(node, l, r);
        size_t mid = l + (r - l) / 2;
        if (ql <= mid) range_set(2 * node, l, mid, ql, qr, val);
        if (qr > mid) range_set(2 * node + 1, mid + 1, r, ql, qr, val);
        push_up(node);
    }

    int64_t query_sum(size_t node, size_t l, size_t r, size_t ql, size_t qr) {
        if (ql <= l && r <= qr) return tree_[node].sum;
        push_down(node, l, r);
        size_t mid = l + (r - l) / 2;
        int64_t res = 0;
        if (ql <= mid) res += query_sum(2 * node, l, mid, ql, qr);
        if (qr > mid) res += query_sum(2 * node + 1, mid + 1, r, ql, qr);
        return res;
    }

    int64_t query_min(size_t node, size_t l, size_t r, size_t ql, size_t qr) {
        if (ql <= l && r <= qr) return tree_[node].min_val;
        push_down(node, l, r);
        size_t mid = l + (r - l) / 2;
        int64_t res = std::numeric_limits<int64_t>::max();
        if (ql <= mid) res = std::min(res, query_min(2 * node, l, mid, ql, qr));
        if (qr > mid) res = std::min(res, query_min(2 * node + 1, mid + 1, r, ql, qr));
        return res;
    }

public:
    explicit LazySegmentTree(const std::vector<int64_t>& a)
        : n_(a.size()), tree_(4 * std::max(size_t(1), a.size())) {
        if (n_ > 0) build(1, 0, n_ - 1, a);
    }

    void range_add(size_t l, size_t r, int64_t val) {
        assert(l <= r && r < n_);
        range_add(1, 0, n_ - 1, l, r, val);
    }

    void range_set(size_t l, size_t r, int64_t val) {
        assert(l <= r && r < n_);
        range_set(1, 0, n_ - 1, l, r, val);
    }

    int64_t query_sum(size_t l, size_t r) {
        assert(l <= r && r < n_);
        return query_sum(1, 0, n_ - 1, l, r);
    }

    int64_t query_min(size_t l, size_t r) {
        assert(l <= r && r < n_);
        return query_min(1, 0, n_ - 1, l, r);
    }
};

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Range Updates and Lazy Propagation C++17 Verification..." << std::endl;

    std::vector<int64_t> a = {1, 2, 3, 4, 5, 6, 7, 8};
    LazySegmentTree tree(a);

    // Initial checks
    assert(tree.query_sum(0, 7) == 36);
    assert(tree.query_min(0, 7) == 1);
    assert(tree.query_sum(2, 4) == 12); // 3 + 4 + 5 = 12

    // Range Add: add +10 to [1, 3] -> a becomes [1, 12, 13, 14, 5, 6, 7, 8]
    tree.range_add(1, 3, 10);
    assert(tree.query_sum(1, 3) == 39);
    assert(tree.query_min(0, 3) == 1);
    assert(tree.query_min(1, 3) == 12);
    assert(tree.query_sum(0, 7) == 66);

    // Range Set: set [2, 5] to 0 -> a becomes [1, 12, 0, 0, 0, 0, 7, 8]
    tree.range_set(2, 5, 0);
    assert(tree.query_sum(2, 5) == 0);
    assert(tree.query_min(2, 5) == 0);
    assert(tree.query_sum(0, 7) == 28); // 1 + 12 + 0 + 0 + 0 + 0 + 7 + 8 = 28
    assert(tree.query_min(0, 7) == 0);

    // Range Add on top of Range Set: add +5 to [3, 6] -> a becomes [1, 12, 0, 5, 5, 5, 12, 8]
    tree.range_add(3, 6, 5);
    assert(tree.query_sum(3, 6) == 27); // 5 + 5 + 5 + 12 = 27
    assert(tree.query_min(3, 6) == 5);
    assert(tree.query_min(0, 7) == 0);
    assert(tree.query_sum(0, 7) == 48);

    std::cout << "[PASSED] Range Updates and Lazy Propagation C++17 All Tests Passed!" << std::endl;
    return 0;
}
