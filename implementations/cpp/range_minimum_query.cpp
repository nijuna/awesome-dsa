#include <cassert>
#include <algorithm>
#include <iostream>
#include <limits>
#include <stack>
#include <vector>

namespace dsa {

/**
 * @brief Static RMQ via Sparse Table.
 * Preprocessing: O(N log N).
 * Query: O(1).
 * Stores indices with consistent leftmost tie-breaking.
 */
class StaticRMQ {
private:
    std::vector<int> a_;
    std::vector<int> log2_;
    std::vector<std::vector<size_t>> st_; // stores index of minimum

public:
    explicit StaticRMQ(const std::vector<int>& a) : a_(a) {
        size_t n = a_.size();
        log2_.resize(n + 1, 0);
        for (size_t i = 2; i <= n; ++i) {
            log2_[i] = log2_[i / 2] + 1;
        }

        size_t K = (n == 0) ? 0 : log2_[n] + 1;
        st_.assign(K, std::vector<size_t>(n));

        for (size_t i = 0; i < n; ++i) {
            st_[0][i] = i;
        }

        for (size_t k = 1; k < K; ++k) {
            size_t len = 1ULL << k;
            size_t half = len >> 1;
            for (size_t i = 0; i + len <= n; ++i) {
                size_t left_idx = st_[k - 1][i];
                size_t right_idx = st_[k - 1][i + half];
                st_[k][i] = (a_[left_idx] <= a_[right_idx]) ? left_idx : right_idx;
            }
        }
    }

    size_t query_index(size_t l, size_t r) const {
        assert(l <= r && r < a_.size());
        size_t len = r - l + 1;
        int k = log2_[len];
        size_t left_idx = st_[k][l];
        size_t right_idx = st_[k][r - (1ULL << k) + 1];
        return (a_[left_idx] <= a_[right_idx]) ? left_idx : right_idx;
    }

    int query_value(size_t l, size_t r) const {
        return a_[query_index(l, r)];
    }
};

/**
 * @brief Dynamic RMQ via Segment Tree supporting point updates.
 * Preprocessing: O(N).
 * Query: O(log N).
 * Point Update: O(log N).
 */
class DynamicRMQ {
private:
    size_t n_;
    std::vector<int> tree_;

    void build(size_t node, size_t left, size_t right, const std::vector<int>& a) {
        if (left == right) {
            tree_[node] = a[left];
            return;
        }
        size_t mid = left + (right - left) / 2;
        build(2 * node, left, mid, a);
        build(2 * node + 1, mid + 1, right, a);
        tree_[node] = std::min(tree_[2 * node], tree_[2 * node + 1]);
    }

    int query(size_t node, size_t left, size_t right, size_t ql, size_t qr) const {
        if (qr < left || right < ql) return std::numeric_limits<int>::max();
        if (ql <= left && right <= qr) return tree_[node];
        size_t mid = left + (right - left) / 2;
        return std::min(query(2 * node, left, mid, ql, qr),
                        query(2 * node + 1, mid + 1, right, ql, qr));
    }

    void update(size_t node, size_t left, size_t right, size_t idx, int val) {
        if (left == right) {
            tree_[node] = val;
            return;
        }
        size_t mid = left + (right - left) / 2;
        if (idx <= mid) update(2 * node, left, mid, idx, val);
        else update(2 * node + 1, mid + 1, right, idx, val);
        tree_[node] = std::min(tree_[2 * node], tree_[2 * node + 1]);
    }

public:
    explicit DynamicRMQ(const std::vector<int>& a) : n_(a.size()), tree_(4 * std::max(size_t(1), a.size()), std::numeric_limits<int>::max()) {
        if (n_ > 0) build(1, 0, n_ - 1, a);
    }

    int query(size_t l, size_t r) const {
        assert(l <= r && r < n_);
        return query(1, 0, n_ - 1, l, r);
    }

    void update(size_t idx, int val) {
        assert(idx < n_);
        update(1, 0, n_ - 1, idx, val);
    }
};

/**
 * @brief Min-Cartesian Tree builder using monotonic stack in linear O(N) time.
 */
struct CartesianNode {
    int val;
    size_t index;
    int left = -1;
    int right = -1;
    int parent = -1;
};

inline int build_cartesian_tree(const std::vector<int>& a, std::vector<CartesianNode>& nodes) {
    int n = static_cast<int>(a.size());
    nodes.resize(n);
    for (int i = 0; i < n; ++i) {
        nodes[i].val = a[i];
        nodes[i].index = i;
        nodes[i].left = nodes[i].right = nodes[i].parent = -1;
    }

    std::stack<int> st;
    for (int i = 0; i < n; ++i) {
        int last_popped = -1;
        while (!st.empty() && nodes[st.top()].val > a[i]) {
            last_popped = st.top();
            st.pop();
        }
        if (last_popped != -1) {
            nodes[i].left = last_popped;
            nodes[last_popped].parent = i;
        }
        if (!st.empty()) {
            nodes[st.top()].right = i;
            nodes[i].parent = st.top();
        }
        st.push(i);
    }

    int root = -1;
    for (int i = 0; i < n; ++i) {
        if (nodes[i].parent == -1) {
            root = i;
            break;
        }
    }
    return root;
}

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Range Minimum Query (RMQ) C++17 Verification..." << std::endl;

    // 1. Static RMQ (Sparse Table)
    {
        std::vector<int> a = {7, 2, 3, 0, 5, 10, 3, 12, 18};
        StaticRMQ rmq(a);

        assert(rmq.query_value(0, 4) == 0);
        assert(rmq.query_index(0, 4) == 3);

        assert(rmq.query_value(4, 7) == 3);
        assert(rmq.query_index(4, 7) == 6);

        assert(rmq.query_value(1, 2) == 2);
        assert(rmq.query_index(1, 2) == 1);

        assert(rmq.query_value(5, 5) == 10);
        assert(rmq.query_index(5, 5) == 5);
    }

    // 2. Dynamic RMQ (Segment Tree)
    {
        std::vector<int> a = {5, 8, 6, 3, 2, 7};
        DynamicRMQ seg(a);

        assert(seg.query(0, 5) == 2);
        assert(seg.query(0, 2) == 5);

        // Update a[4] from 2 to 9
        seg.update(4, 9);
        assert(seg.query(0, 5) == 3); // minimum is now 3 at index 3

        // Update a[1] to -1
        seg.update(1, -1);
        assert(seg.query(0, 2) == -1);
        assert(seg.query(0, 5) == -1);
    }

    // 3. Cartesian Tree Building
    {
        std::vector<int> a = {9, 3, 7, 1, 8, 12, 10, 20, 15, 18, 5};
        std::vector<CartesianNode> nodes;
        int root = build_cartesian_tree(a, nodes);

        // Root must be the global minimum (value 1 at index 3)
        assert(root == 3);
        assert(nodes[root].val == 1);
    }

    std::cout << "[PASSED] Range Minimum Query (RMQ) C++17 All Tests Passed!" << std::endl;
    return 0;
}
