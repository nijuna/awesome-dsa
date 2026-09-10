/**
 * Benchmark Harness: Fenwick Tree vs. Segment Tree
 * Measures physical performance, throughput, and memory footprint on modern hardware.
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <iomanip>

// --- Fenwick Tree ---
template <typename T = long long>
class FenwickTree {
private:
    std::size_t n_;
    std::vector<T> tree_;

public:
    explicit FenwickTree(std::size_t n) : n_(n), tree_(n + 1, 0) {}

    explicit FenwickTree(const std::vector<T>& arr) : n_(arr.size()), tree_(arr.size() + 1, 0) {
        for (std::size_t i = 1; i <= n_; ++i) {
            tree_[i] += arr[i - 1];
            std::size_t parent = i + (i & -i);
            if (parent <= n_) tree_[parent] += tree_[i];
        }
    }

    void add(std::size_t i, T delta) {
        for (; i <= n_; i += i & -i) tree_[i] += delta;
    }

    T query(std::size_t i) const {
        T sum = 0;
        for (; i > 0; i -= i & -i) sum += tree_[i];
        return sum;
    }

    T range_query(std::size_t l, std::size_t r) const {
        if (l > r) return 0;
        return query(r) - query(l - 1);
    }
};

// --- Segment Tree ---
template <typename T = long long>
class SegmentTree {
private:
    std::size_t n_;
    std::vector<T> tree_;

    void build(const std::vector<T>& arr, std::size_t node, std::size_t start, std::size_t end) {
        if (start == end) {
            tree_[node] = arr[start];
            return;
        }
        std::size_t mid = start + (end - start) / 2;
        build(arr, 2 * node, start, mid);
        build(arr, 2 * node + 1, mid + 1, end);
        tree_[node] = tree_[2 * node] + tree_[2 * node + 1];
    }

public:
    explicit SegmentTree(const std::vector<T>& arr)
        : n_(arr.size()), tree_(4 * arr.size(), 0) {
        if (n_ > 0) build(arr, 1, 0, n_ - 1);
    }

    void update(std::size_t node, std::size_t start, std::size_t end, std::size_t idx, T val) {
        if (start == end) {
            tree_[node] += val;
            return;
        }
        std::size_t mid = start + (end - start) / 2;
        if (idx <= mid) update(2 * node, start, mid, idx, val);
        else update(2 * node + 1, mid + 1, end, idx, val);
        tree_[node] = tree_[2 * node] + tree_[2 * node + 1];
    }

    void add(std::size_t idx, T val) {
        update(1, 0, n_ - 1, idx, val);
    }

    T query(std::size_t node, std::size_t start, std::size_t end, std::size_t l, std::size_t r) {
        if (r < start || end < l) return 0;
        if (l <= start && end <= r) return tree_[node];
        std::size_t mid = start + (end - start) / 2;
        return query(2 * node, start, mid, l, r) + query(2 * node + 1, mid + 1, end, l, r);
    }

    T range_query(std::size_t l, std::size_t r) {
        return query(1, 0, n_ - 1, l, r);
    }
};

int main() {
    const std::size_t N = 1000000;    // 1,000,000 elements
    const std::size_t OPS = 2000000;  // 2,000,000 operations

    std::cout << "===============================================================\n";
    std::cout << " BENCHMARK: Fenwick Tree vs. Segment Tree (N = " << N << ", Ops = " << OPS << ")\n";
    std::cout << "===============================================================\n\n";

    std::vector<long long> initial(N, 1);

    // Pre-generate random operations to avoid measuring RNG latency
    std::mt19937_64 rng(42);
    std::vector<std::size_t> indices(OPS);
    std::vector<std::pair<std::size_t, std::size_t>> ranges(OPS);
    for (std::size_t i = 0; i < OPS; ++i) {
        indices[i] = rng() % N;
        std::size_t a = rng() % N;
        std::size_t b = rng() % N;
        if (a > b) std::swap(a, b);
        ranges[i] = {a, b};
    }

    // 1. Benchmark Fenwick Tree
    auto start_build_bit = std::chrono::high_resolution_clock::now();
    FenwickTree<long long> bit(initial);
    auto end_build_bit = std::chrono::high_resolution_clock::now();

    auto start_upd_bit = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < OPS; ++i) {
        bit.add(indices[i] + 1, 5); // 1-indexed
    }
    auto end_upd_bit = std::chrono::high_resolution_clock::now();

    volatile long long bit_sum = 0;
    auto start_qry_bit = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < OPS; ++i) {
        bit_sum += bit.range_query(ranges[i].first + 1, ranges[i].second + 1);
    }
    auto end_qry_bit = std::chrono::high_resolution_clock::now();

    // 2. Benchmark Segment Tree
    auto start_build_st = std::chrono::high_resolution_clock::now();
    SegmentTree<long long> st(initial);
    auto end_build_st = std::chrono::high_resolution_clock::now();

    auto start_upd_st = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < OPS; ++i) {
        st.add(indices[i], 5); // 0-indexed
    }
    auto end_upd_st = std::chrono::high_resolution_clock::now();

    volatile long long st_sum = 0;
    auto start_qry_st = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < OPS; ++i) {
        st_sum += st.range_query(ranges[i].first, ranges[i].second);
    }
    auto end_qry_st = std::chrono::high_resolution_clock::now();

    // Durations
    double bit_build_ms = std::chrono::duration<double, std::milli>(end_build_bit - start_build_bit).count();
    double bit_upd_ms = std::chrono::duration<double, std::milli>(end_upd_bit - start_upd_bit).count();
    double bit_qry_ms = std::chrono::duration<double, std::milli>(end_qry_bit - start_qry_bit).count();

    double st_build_ms = std::chrono::duration<double, std::milli>(end_build_st - start_build_st).count();
    double st_upd_ms = std::chrono::duration<double, std::milli>(end_upd_st - start_upd_st).count();
    double st_qry_ms = std::chrono::duration<double, std::milli>(end_qry_st - start_qry_st).count();

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "| Metric | Fenwick Tree | Segment Tree | Fenwick Speedup |\n";
    std::cout << "| :--- | :--- | :--- | :---: |\n";
    std::cout << "| Memory Allocation | " << (N * sizeof(long long)) / (1024 * 1024) << " MB (1N) | "
              << (4 * N * sizeof(long long)) / (1024 * 1024) << " MB (4N) | **4.0x less RAM** |\n";
    std::cout << "| Build Time (N=1M) | " << bit_build_ms << " ms | " << st_build_ms << " ms | **"
              << (st_build_ms / bit_build_ms) << "x faster** |\n";
    std::cout << "| Point Updates (2M ops) | " << bit_upd_ms << " ms | " << st_upd_ms << " ms | **"
              << (st_upd_ms / bit_upd_ms) << "x faster** |\n";
    std::cout << "| Range Queries (2M ops) | " << bit_qry_ms << " ms | " << st_qry_ms << " ms | **"
              << (st_qry_ms / bit_qry_ms) << "x faster** |\n";

    return 0;
}
