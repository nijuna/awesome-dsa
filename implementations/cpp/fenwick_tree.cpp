/**
 * Reference Implementation: Fenwick Tree (Binary Indexed Tree / BIT)
 * Production-ready modern C++17 implementation with unit tests.
 */

#include <iostream>
#include <vector>
#include <cstddef>
#include <cassert>

template <typename T = long long>
class FenwickTree {
private:
    std::size_t n_;
    std::vector<T> tree_;

public:
    explicit FenwickTree(std::size_t n) : n_(n), tree_(n + 1, 0) {}

    // Linear time O(N) constructor from initial values
    explicit FenwickTree(const std::vector<T>& arr) : n_(arr.size()), tree_(arr.size() + 1, 0) {
        for (std::size_t i = 1; i <= n_; ++i) {
            tree_[i] += arr[i - 1];
            std::size_t parent = i + (i & -i);
            if (parent <= n_) {
                tree_[parent] += tree_[i];
            }
        }
    }

    void add(std::size_t i, T delta) {
        for (; i <= n_; i += i & -i) {
            tree_[i] += delta;
        }
    }

    T query(std::size_t i) const {
        T sum = 0;
        for (; i > 0; i -= i & -i) {
            sum += tree_[i];
        }
        return sum;
    }

    T range_query(std::size_t l, std::size_t r) const {
        if (l > r) return 0;
        return query(r) - query(l - 1);
    }
};

void run_tests() {
    std::vector<long long> arr = {1, 3, 5, 7, 9, 11};
    FenwickTree<long long> bit(arr);

    assert(bit.query(1) == 1);
    assert(bit.query(3) == 9); // 1 + 3 + 5
    assert(bit.range_query(2, 4) == 15); // 3 + 5 + 7
    assert(bit.range_query(1, 6) == 36);

    // Update index 3 (value 5 -> 5 + 6 = 11)
    bit.add(3, 6);
    assert(bit.query(3) == 15);
    assert(bit.range_query(2, 4) == 21);
    assert(bit.range_query(1, 6) == 42);

    // Edge case: single element
    FenwickTree<long long> single(1);
    single.add(1, 100);
    assert(single.query(1) == 100);
    assert(single.range_query(1, 1) == 100);
    assert(single.range_query(2, 1) == 0);

    std::cout << "[PASS] All Fenwick Tree C++ unit tests passed.\n";
}

int main() {
    run_tests();
    return 0;
}
