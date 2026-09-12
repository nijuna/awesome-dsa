#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <cassert>
#include <stdexcept>

namespace dsa {

/**
 * @brief Precomputes floor(log2(i)) for i = 1..n in O(n) time.
 */
inline std::vector<int> build_log_table(int n) {
    if (n <= 0) return {};
    std::vector<int> log_table(n + 1, 0);
    for (int i = 2; i <= n; ++i) {
        log_table[i] = log_table[i / 2] + 1;
    }
    return log_table;
}

/**
 * @brief Sparse Table for Range Minimum Query (RMQ) on static arrays.
 *
 * Precomputation: O(n log n) time and space.
 * Query: O(1) time using idempotence min(x, x) = x.
 */
class SparseTableMin {
public:
    explicit SparseTableMin(const std::vector<int>& a) {
        n_ = static_cast<int>(a.size());
        if (n_ == 0) return;

        log_ = build_log_table(n_);
        int k_max = log_[n_] + 1;
        st_.assign(k_max, std::vector<int>(n_));
        st_[0] = a;

        for (int k = 1; k < k_max; ++k) {
            int len = 1 << k;
            int half = len >> 1;
            for (int i = 0; i + len <= n_; ++i) {
                st_[k][i] = std::min(st_[k - 1][i], st_[k - 1][i + half]);
            }
        }
    }

    int query(int l, int r) const {
        if (n_ == 0 || l < 0 || r >= n_ || l > r) {
            throw std::out_of_range("Invalid query range");
        }
        int len = r - l + 1;
        int k = log_[len];
        return std::min(st_[k][l], st_[k][r - (1 << k) + 1]);
    }

    bool empty() const { return n_ == 0; }
    int size() const { return n_; }

private:
    int n_{0};
    std::vector<int> log_;
    std::vector<std::vector<int>> st_;
};

/**
 * @brief Sparse Table for Range Maximum Query on static arrays.
 *
 * Query: O(1) time.
 */
class SparseTableMax {
public:
    explicit SparseTableMax(const std::vector<int>& a) {
        n_ = static_cast<int>(a.size());
        if (n_ == 0) return;

        log_ = build_log_table(n_);
        int k_max = log_[n_] + 1;
        st_.assign(k_max, std::vector<int>(n_));
        st_[0] = a;

        for (int k = 1; k < k_max; ++k) {
            int len = 1 << k;
            int half = len >> 1;
            for (int i = 0; i + len <= n_; ++i) {
                st_[k][i] = std::max(st_[k - 1][i], st_[k - 1][i + half]);
            }
        }
    }

    int query(int l, int r) const {
        if (n_ == 0 || l < 0 || r >= n_ || l > r) {
            throw std::out_of_range("Invalid query range");
        }
        int len = r - l + 1;
        int k = log_[len];
        return std::max(st_[k][l], st_[k][r - (1 << k) + 1]);
    }

private:
    int n_{0};
    std::vector<int> log_;
    std::vector<std::vector<int>> st_;
};

/**
 * @brief Sparse Table for Range Greatest Common Divisor (GCD) queries.
 *
 * Query: O(1) gcd evaluations.
 */
class SparseTableGCD {
public:
    explicit SparseTableGCD(const std::vector<int>& a) {
        n_ = static_cast<int>(a.size());
        if (n_ == 0) return;

        log_ = build_log_table(n_);
        int k_max = log_[n_] + 1;
        st_.assign(k_max, std::vector<int>(n_));
        st_[0] = a;

        for (int k = 1; k < k_max; ++k) {
            int len = 1 << k;
            int half = len >> 1;
            for (int i = 0; i + len <= n_; ++i) {
                st_[k][i] = std::gcd(st_[k - 1][i], st_[k - 1][i + half]);
            }
        }
    }

    int query(int l, int r) const {
        if (n_ == 0 || l < 0 || r >= n_ || l > r) {
            throw std::out_of_range("Invalid query range");
        }
        int len = r - l + 1;
        int k = log_[len];
        return std::gcd(st_[k][l], st_[k][r - (1 << k) + 1]);
    }

private:
    int n_{0};
    std::vector<int> log_;
    std::vector<std::vector<int>> st_;
};

/**
 * @brief Sparse Table storing indices of minimum elements (ArgMin).
 * Useful for LCA reductions and Cartesian tree building.
 */
class SparseTableArgMin {
public:
    explicit SparseTableArgMin(const std::vector<int>& a) : a_(a) {
        n_ = static_cast<int>(a.size());
        if (n_ == 0) return;

        log_ = build_log_table(n_);
        int k_max = log_[n_] + 1;
        st_.assign(k_max, std::vector<int>(n_));

        for (int i = 0; i < n_; ++i) {
            st_[0][i] = i;
        }

        for (int k = 1; k < k_max; ++k) {
            int len = 1 << k;
            int half = len >> 1;
            for (int i = 0; i + len <= n_; ++i) {
                int left_idx = st_[k - 1][i];
                int right_idx = st_[k - 1][i + half];
                st_[k][i] = (a_[left_idx] <= a_[right_idx] ? left_idx : right_idx);
            }
        }
    }

    int query_index(int l, int r) const {
        if (n_ == 0 || l < 0 || r >= n_ || l > r) {
            throw std::out_of_range("Invalid query range");
        }
        int len = r - l + 1;
        int k = log_[len];
        int left_idx = st_[k][l];
        int right_idx = st_[k][r - (1 << k) + 1];
        return (a_[left_idx] <= a_[right_idx] ? left_idx : right_idx);
    }

    int query_value(int l, int r) const {
        return a_[query_index(l, r)];
    }

private:
    std::vector<int> a_;
    int n_{0};
    std::vector<int> log_;
    std::vector<std::vector<int>> st_;
};

} // namespace dsa

// ============================================================================
// Unit Tests
// ============================================================================

void test_rmq_min() {
    std::vector<int> a = {5, 2, 4, 7, 1, 3, 6};
    dsa::SparseTableMin st(a);

    assert(st.query(0, 6) == 1);
    assert(st.query(0, 1) == 2);
    assert(st.query(1, 3) == 2); // {2, 4, 7}
    assert(st.query(1, 5) == 1); // {2, 4, 7, 1, 3}
    assert(st.query(4, 4) == 1);
    assert(st.query(5, 6) == 3);

    // Negative values
    std::vector<int> neg = {10, -5, 3, -8, 20};
    dsa::SparseTableMin st_neg(neg);
    assert(st_neg.query(0, 4) == -8);
    assert(st_neg.query(0, 2) == -5);
    assert(st_neg.query(1, 2) == -5);
    assert(st_neg.query(4, 4) == 20);
}

void test_rmq_max() {
    std::vector<int> a = {3, 9, 2, 8, 1, 7};
    dsa::SparseTableMax st(a);

    assert(st.query(0, 5) == 9);
    assert(st.query(2, 4) == 8);
    assert(st.query(0, 0) == 3);
    assert(st.query(4, 5) == 7);
}

void test_gcd() {
    std::vector<int> a = {24, 18, 42, 60, 100};
    dsa::SparseTableGCD st(a);

    assert(st.query(0, 1) == 6);  // gcd(24, 18) = 6
    assert(st.query(0, 2) == 6);  // gcd(24, 18, 42) = 6
    assert(st.query(0, 3) == 6);  // gcd(24, 18, 42, 60) = 6
    assert(st.query(3, 4) == 20); // gcd(60, 100) = 20
    assert(st.query(0, 4) == 2);  // gcd(all) = 2
}

void test_argmin() {
    std::vector<int> a = {5, 2, 4, 2, 1, 3, 1};
    dsa::SparseTableArgMin st(a);

    // With <= tie-breaking, earliest minimum index is preferred
    assert(st.query_index(0, 3) == 1); // a[1] = 2
    assert(st.query_index(4, 6) == 4); // a[4] = 1
    assert(st.query_value(0, 6) == 1);
    assert(st.query_index(0, 6) == 4);
}

void test_single_element() {
    std::vector<int> single = {42};
    dsa::SparseTableMin st(single);
    assert(st.query(0, 0) == 42);

    dsa::SparseTableArgMin st_arg(single);
    assert(st_arg.query_index(0, 0) == 0);
    assert(st_arg.query_value(0, 0) == 42);
}

int main() {
    std::cout << "Running Sparse Table C++17 unit tests...\n";
    test_rmq_min();
    test_rmq_max();
    test_gcd();
    test_argmin();
    test_single_element();
    std::cout << "All Sparse Table C++17 unit tests passed successfully!\n";
    return 0;
}
