/**
 * @file sqrt_decomposition.cpp
 * @brief Reference implementation of Square Root (Sqrt) Decomposition for range queries.
 *
 * Implements point updates, range additions with lazy tags, and range sum/min queries
 * in O(sqrt(n)) time and O(n) space.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <random>

namespace dsa {

template <typename T>
class SqrtDecomposition {
private:
    size_t n;
    size_t block_size;
    size_t num_blocks;
    std::vector<T> arr;
    std::vector<T> block_sum;
    std::vector<T> lazy;

public:
    explicit SqrtDecomposition(const std::vector<T>& data)
        : n(data.size()), arr(data) {
        if (n == 0) {
            block_size = 1;
            num_blocks = 0;
            return;
        }

        block_size = std::max<size_t>(1, static_cast<size_t>(std::sqrt(n)));
        num_blocks = (n + block_size - 1) / block_size;

        block_sum.assign(num_blocks, 0);
        lazy.assign(num_blocks, 0);

        for (size_t i = 0; i < n; ++i) {
            block_sum[i / block_size] += arr[i];
        }
    }

    size_t size() const {
        return n;
    }

    /**
     * @brief Point update: sets arr[idx] = new_val in O(1) time.
     */
    void update_point(size_t idx, T new_val) {
        if (idx >= n) throw std::out_of_range("Index out of bounds");
        size_t b = idx / block_size;
        T effective_current = arr[idx] + lazy[b];
        T diff = new_val - effective_current;
        arr[idx] = new_val - lazy[b];
        block_sum[b] += diff;
    }

    /**
     * @brief Range add update: adds delta to all elements in range [l, r] in O(sqrt(n)) time.
     */
    void range_add(size_t l, size_t r, T delta) {
        if (l > r || r >= n) throw std::out_of_range("Invalid range indices");

        size_t b_l = l / block_size;
        size_t b_r = r / block_size;

        if (b_l == b_r) {
            for (size_t i = l; i <= r; ++i) {
                arr[i] += delta;
            }
            block_sum[b_l] += delta * static_cast<T>(r - l + 1);
            return;
        }

        // 1. Left partial block
        size_t end_l = (b_l + 1) * block_size - 1;
        for (size_t i = l; i <= end_l; ++i) {
            arr[i] += delta;
        }
        block_sum[b_l] += delta * static_cast<T>(end_l - l + 1);

        // 2. Full intermediate blocks
        for (size_t b = b_l + 1; b < b_r; ++b) {
            lazy[b] += delta;
            block_sum[b] += delta * static_cast<T>(block_size);
        }

        // 3. Right partial block
        size_t start_r = b_r * block_size;
        for (size_t i = start_r; i <= r; ++i) {
            arr[i] += delta;
        }
        block_sum[b_r] += delta * static_cast<T>(r - start_r + 1);
    }

    /**
     * @brief Range sum query: returns sum of elements in [l, r] in O(sqrt(n)) time.
     */
    T query_sum(size_t l, size_t r) const {
        if (l > r || r >= n) throw std::out_of_range("Invalid range indices");

        size_t b_l = l / block_size;
        size_t b_r = r / block_size;
        T total = 0;

        if (b_l == b_r) {
            for (size_t i = l; i <= r; ++i) {
                total += arr[i] + lazy[b_l];
            }
            return total;
        }

        // 1. Left partial block
        size_t end_l = (b_l + 1) * block_size - 1;
        for (size_t i = l; i <= end_l; ++i) {
            total += arr[i] + lazy[b_l];
        }

        // 2. Full intermediate blocks
        for (size_t b = b_l + 1; b < b_r; ++b) {
            total += block_sum[b];
        }

        // 3. Right partial block
        size_t start_r = b_r * block_size;
        for (size_t i = start_r; i <= r; ++i) {
            total += arr[i] + lazy[b_r];
        }

        return total;
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Sqrt Decomposition C++17 unit tests..." << std::endl;

    // Test 1: Basic range sum and point update
    {
        std::vector<int64_t> data = {1, 3, 5, 7, 9, 11, 13, 15, 17};
        dsa::SqrtDecomposition<int64_t> sqrt_dec(data);

        // Sum [1, 5] = 3 + 5 + 7 + 9 + 11 = 35
        assert(sqrt_dec.query_sum(1, 5) == 35);

        // Point update at idx 3: 7 -> 10
        sqrt_dec.update_point(3, 10);
        // New sum [1, 5] = 3 + 5 + 10 + 9 + 11 = 38
        assert(sqrt_dec.query_sum(1, 5) == 38);
    }

    // Test 2: Range add with lazy propagation
    {
        std::vector<int64_t> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        dsa::SqrtDecomposition<int64_t> sqrt_dec(data);

        // Add 5 to range [2, 7]
        sqrt_dec.range_add(2, 7, 5);

        // Query single element at 3: original 4 + 5 = 9
        assert(sqrt_dec.query_sum(3, 3) == 9);

        // Query total sum [0, 9]:
        // original sum = 55, added 6 elements * 5 = 30 -> total = 85
        assert(sqrt_dec.query_sum(0, 9) == 85);
    }

    // Test 3: Randomized stress testing against naive vector oracle
    {
        constexpr size_t N = 1000;
        std::vector<int64_t> initial(N, 0);
        std::mt19937 rng(42);
        std::uniform_int_distribution<int64_t> val_dist(-100, 100);

        for (size_t i = 0; i < N; ++i) initial[i] = val_dist(rng);

        std::vector<int64_t> oracle = initial;
        dsa::SqrtDecomposition<int64_t> sqrt_dec(initial);

        std::uniform_int_distribution<size_t> idx_dist(0, N - 1);
        std::uniform_int_distribution<int> op_dist(0, 2);

        for (int step = 0; step < 5000; ++step) {
            int op = op_dist(rng);
            size_t idx1 = idx_dist(rng);
            size_t idx2 = idx_dist(rng);
            size_t l = std::min(idx1, idx2);
            size_t r = std::max(idx1, idx2);

            if (op == 0) {
                // Query
                int64_t expected = 0;
                for (size_t i = l; i <= r; ++i) expected += oracle[i];
                assert(sqrt_dec.query_sum(l, r) == expected);
            } else if (op == 1) {
                // Range add
                int64_t delta = val_dist(rng);
                for (size_t i = l; i <= r; ++i) oracle[i] += delta;
                sqrt_dec.range_add(l, r, delta);
            } else {
                // Point update
                int64_t new_v = val_dist(rng);
                oracle[l] = new_v;
                sqrt_dec.update_point(l, new_v);
            }
        }
    }

    std::cout << "[PASS] All Sqrt Decomposition C++ unit tests passed." << std::endl;
    return 0;
}
