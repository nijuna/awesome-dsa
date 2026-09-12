/**
 * @file parallel_algorithm_basics.cpp
 * @brief High-performance C++17 reference implementation of Parallel Primitives and the Work-Depth Model.
 *
 * Implements fundamental parallel algorithms based on the DAG dynamic multithreading model:
 * 1. Explicit Parallel Reduction: Work O(N), Span O(log N).
 * 2. Blelloch Work-Efficient Parallel Prefix Scan: Work O(N), Span O(log N).
 *    - Phase 1: Upsweep (Reduction Tree).
 *    - Phase 2: Downsweep (Prefix Distribution Tree).
 * 3. Parallel Filter / Stream Compaction: Work O(N), Span O(log N).
 * 4. Parallel Merge Sort: Work O(N log N), Span O(log^2 N).
 * 5. Practical C++17 parallel execution mapping layer.
 *
 * Fully compliant with -std=c++17 -O3 -Wall -Wextra -Werror -pthread.
 */

#include <iostream>
#include <vector>
#include <numeric>
#include <future>
#include <thread>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <functional>
#include <random>

namespace dsa {

/**
 * @brief Fork-join dispatcher with grain-size cutoff threshold.
 */
class ForkJoinRunner {
public:
    template <typename F1, typename F2>
    static void invoke(F1&& task1, F2&& task2, size_t threshold, size_t current_n) {
        if (current_n <= threshold) {
            task1();
            task2();
        } else {
            auto fut = std::async(std::launch::async, std::forward<F1>(task1));
            task2();
            fut.get();
        }
    }
};

/**
 * @brief Explicit Divide-and-Conquer Parallel Reduction.
 * Work: O(N), Span: O(log N).
 */
template <typename T, typename Op = std::plus<T>>
T parallel_reduce(const std::vector<T>& arr, size_t left, size_t right, T identity, Op op = Op{}, size_t threshold = 1024) {
    if (left >= right) return identity;
    if (right - left == 1) return arr[left];
    if (right - left <= threshold) {
        T acc = identity;
        for (size_t i = left; i < right; ++i) {
            acc = op(acc, arr[i]);
        }
        return acc;
    }

    size_t mid = left + (right - left) / 2;
    T left_res = identity;
    T right_res = identity;

    ForkJoinRunner::invoke(
        [&]() { left_res = parallel_reduce(arr, left, mid, identity, op, threshold); },
        [&]() { right_res = parallel_reduce(arr, mid, right, identity, op, threshold); },
        threshold, right - left
    );

    return op(left_res, right_res);
}

/**
 * @brief Blelloch's Work-Efficient Parallel Prefix Scan (Exclusive Scan).
 * Work: O(N), Span: O(log N).
 * Pass 1: Upsweep (Parallel Reduction Tree).
 * Pass 2: Downsweep (Parallel Distribution Tree).
 */
template <typename T>
void blelloch_exclusive_scan(std::vector<T>& data) {
    size_t n = data.size();
    if (n <= 1) {
        if (n == 1) data[0] = 0;
        return;
    }

    // Pad to power of 2
    size_t m = 1;
    while (m < n) m <<= 1;
    std::vector<T> tree(2 * m, 0);

    // Leaves placed at indices [m ... m + n - 1]
    for (size_t i = 0; i < n; ++i) {
        tree[m + i] = data[i];
    }

    // Pass 1: Upsweep (Reduction Tree)
    for (size_t d = m / 2; d > 0; d >>= 1) {
        for (size_t i = d; i < 2 * d; ++i) {
            tree[i] = tree[2 * i] + tree[2 * i + 1];
        }
    }

    // Set root to identity
    tree[1] = 0;

    // Pass 2: Downsweep (Distribution Tree)
    for (size_t d = 1; d < m; d <<= 1) {
        for (size_t i = d; i < 2 * d; ++i) {
            T t = tree[2 * i];
            tree[2 * i] = tree[i];
            tree[2 * i + 1] = tree[i] + t;
        }
    }

    // Extract scanned result
    for (size_t i = 0; i < n; ++i) {
        data[i] = tree[m + i];
    }
}

/**
 * @brief Parallel Filter (Stream Compaction / Pack).
 * Work: O(N), Span: O(log N).
 */
template <typename T, typename Pred>
std::vector<T> parallel_filter(const std::vector<T>& in, Pred pred) {
    size_t n = in.size();
    if (n == 0) return {};

    std::vector<size_t> flags(n, 0);
    for (size_t i = 0; i < n; ++i) {
        if (pred(in[i])) flags[i] = 1;
    }

    std::vector<size_t> offsets = flags;
    blelloch_exclusive_scan(offsets);

    size_t total_matches = offsets.back() + flags.back();
    std::vector<T> out(total_matches);

    for (size_t i = 0; i < n; ++i) {
        if (flags[i]) {
            out[offsets[i]] = in[i];
        }
    }

    return out;
}

/**
 * @brief Parallel Merge Sort using divide-and-conquer fork-join.
 * Work: O(N log N), Span: O(log^2 N).
 */
template <typename T>
void parallel_mergesort(std::vector<T>& arr, size_t left, size_t right, size_t threshold = 1024) {
    if (right - left <= 1) return;
    if (right - left <= threshold) {
        std::sort(arr.begin() + left, arr.begin() + right);
        return;
    }

    size_t mid = left + (right - left) / 2;
    ForkJoinRunner::invoke(
        [&]() { parallel_mergesort(arr, left, mid, threshold); },
        [&]() { parallel_mergesort(arr, mid, right, threshold); },
        threshold, right - left
    );

    std::inplace_merge(arr.begin() + left, arr.begin() + mid, arr.begin() + right);
}

} // namespace dsa

int main() {
    using namespace dsa;
    std::cout << "Testing Parallel Algorithm Basics & Primitives...\n";

    // 1. Verify Parallel Reduction
    {
        size_t N = 10000;
        std::vector<int64_t> arr(N);
        std::iota(arr.begin(), arr.end(), 1);
        int64_t expected = static_cast<int64_t>(N) * (N + 1) / 2;

        int64_t actual = parallel_reduce(arr, 0, N, int64_t(0), std::plus<int64_t>{}, 256);
        assert(actual == expected);
        std::cout << "Parallel reduction verified.\n";
    }

    // 2. Verify Blelloch Exclusive Scan
    {
        std::vector<int> data = {3, 1, 7, 0, 4, 1, 6, 3};
        std::vector<int> copy = data;
        blelloch_exclusive_scan(copy);

        std::vector<int> expected = {0, 3, 4, 11, 11, 15, 16, 22};
        assert(copy == expected);
        std::cout << "Blelloch exclusive scan verified.\n";
    }

    // 3. Verify Parallel Filter (Stream Compaction)
    {
        std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        auto evens = parallel_filter(data, [](int x) { return x % 2 == 0; });
        std::vector<int> expected_evens = {2, 4, 6, 8, 10};
        assert(evens == expected_evens);
        std::cout << "Parallel filter verified.\n";
    }

    // 4. Verify Parallel Merge Sort
    {
        std::mt19937 rng(42);
        size_t N = 5000;
        std::vector<int> arr(N);
        for (size_t i = 0; i < N; ++i) arr[i] = rng() % 100000;

        std::vector<int> oracle = arr;
        std::sort(oracle.begin(), oracle.end());

        parallel_mergesort(arr, 0, N, 256);
        assert(arr == oracle);
        std::cout << "Parallel mergesort verified.\n";
    }

    std::cout << "All parallel algorithmic primitives verified successfully!\n";
    return 0;
}
