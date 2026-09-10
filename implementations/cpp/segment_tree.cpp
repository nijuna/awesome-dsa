/**
 * Reference Implementation: Segment Tree with Lazy Propagation
 * Production-ready modern C++17 implementation with unit tests.
 */

#include <iostream>
#include <vector>
#include <cassert>
#include <algorithm>

template <typename T = long long>
class LazySegmentTree {
private:
    std::size_t n_;
    std::vector<T> tree_;
    std::vector<T> lazy_;

    void build(const std::vector<T>& arr, std::size_t node, std::size_t start, std::size_t end) {
        if (start == end) {
            tree_[node] = arr[start];
            return;
        }
        std::size_t mid = start + (end - start) / 2;
        std::size_t left = 2 * node;
        std::size_t right = 2 * node + 1;
        build(arr, left, start, mid);
        build(arr, right, mid + 1, end);
        tree_[node] = tree_[left] + tree_[right];
    }

    void push_down(std::size_t node, std::size_t start, std::size_t end) {
        if (lazy_[node] != 0) {
            T val = lazy_[node];
            std::size_t mid = start + (end - start) / 2;
            std::size_t left = 2 * node;
            std::size_t right = 2 * node + 1;

            // Apply to child tree node sums
            tree_[left] += val * (mid - start + 1);
            tree_[right] += val * (end - mid);

            // Propagate lazy tags
            lazy_[left] += val;
            lazy_[right] += val;

            lazy_[node] = 0;
        }
    }

    void update_range(std::size_t node, std::size_t start, std::size_t end,
                      std::size_t l, std::size_t r, T val) {
        if (l <= start && end <= r) {
            tree_[node] += val * (end - start + 1);
            lazy_[node] += val;
            return;
        }

        push_down(node, start, end);
        std::size_t mid = start + (end - start) / 2;
        std::size_t left = 2 * node;
        std::size_t right = 2 * node + 1;

        if (l <= mid) {
            update_range(left, start, mid, l, r, val);
        }
        if (r > mid) {
            update_range(right, mid + 1, end, l, r, val);
        }

        tree_[node] = tree_[left] + tree_[right];
    }

    T query_range(std::size_t node, std::size_t start, std::size_t end,
                  std::size_t l, std::size_t r) {
        if (l <= start && end <= r) {
            return tree_[node];
        }

        push_down(node, start, end);
        std::size_t mid = start + (end - start) / 2;
        std::size_t left = 2 * node;
        std::size_t right = 2 * node + 1;
        T total = 0;

        if (l <= mid) {
            total += query_range(left, start, mid, l, r);
        }
        if (r > mid) {
            total += query_range(right, mid + 1, end, l, r);
        }

        return total;
    }

public:
    explicit LazySegmentTree(const std::vector<T>& arr)
        : n_(arr.size()), tree_(4 * arr.size(), 0), lazy_(4 * arr.size(), 0) {
        if (n_ > 0) {
            build(arr, 1, 0, n_ - 1);
        }
    }

    void range_update(std::size_t l, std::size_t r, T val) {
        if (n_ == 0 || l > r || l >= n_) return;
        r = std::min(r, n_ - 1);
        update_range(1, 0, n_ - 1, l, r, val);
    }

    T range_query(std::size_t l, std::size_t r) {
        if (n_ == 0 || l > r || l >= n_) return 0;
        r = std::min(r, n_ - 1);
        return query_range(1, 0, n_ - 1, l, r);
    }
};

void run_tests() {
    std::vector<long long> arr = {1, 2, 3, 4, 5};
    LazySegmentTree<long long> st(arr);

    assert(st.range_query(0, 4) == 15);
    assert(st.range_query(1, 3) == 9); // 2 + 3 + 4

    // Add 10 to range [1, 3] -> arr becomes {1, 12, 13, 14, 5}
    st.range_update(1, 3, 10);
    assert(st.range_query(1, 3) == 39);
    assert(st.range_query(0, 4) == 45);
    assert(st.range_query(0, 0) == 1);
    assert(st.range_query(4, 4) == 5);

    // Edge case: single element
    std::vector<long long> single = {100};
    LazySegmentTree<long long> st_single(single);
    assert(st_single.range_query(0, 0) == 100);
    st_single.range_update(0, 0, 50);
    assert(st_single.range_query(0, 0) == 150);

    std::cout << "[PASS] All Lazy Segment Tree C++ unit tests passed.\n";
}

int main() {
    run_tests();
    return 0;
}
