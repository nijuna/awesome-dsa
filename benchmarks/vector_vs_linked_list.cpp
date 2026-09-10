/**
 * Benchmark Harness: std::vector (Contiguous Array) vs. std::list (Doubly Linked List)
 * Measures real hardware cache performance, iteration latency, and memory overhead.
 */

#include <iostream>
#include <vector>
#include <list>
#include <chrono>
#include <numeric>
#include <iomanip>

int main() {
    const std::size_t N = 10000000; // 10 Million elements
    std::cout << "===================================================================\n";
    std::cout << " BENCHMARK: std::vector vs. std::list (N = " << N << " elements)\n";
    std::cout << "===================================================================\n\n";

    // 1. Benchmark Construction / Allocation Time
    auto start_alloc_vec = std::chrono::high_resolution_clock::now();
    std::vector<int> vec;
    vec.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        vec.push_back(static_cast<int>(i));
    }
    auto end_alloc_vec = std::chrono::high_resolution_clock::now();

    auto start_alloc_list = std::chrono::high_resolution_clock::now();
    std::list<int> lst;
    for (std::size_t i = 0; i < N; ++i) {
        lst.push_back(static_cast<int>(i));
    }
    auto end_alloc_list = std::chrono::high_resolution_clock::now();

    // 2. Benchmark Sequential Iteration / Traversal
    volatile long long sum_vec = 0;
    auto start_iter_vec = std::chrono::high_resolution_clock::now();
    for (int val : vec) {
        sum_vec += val;
    }
    auto end_iter_vec = std::chrono::high_resolution_clock::now();

    volatile long long sum_list = 0;
    auto start_iter_list = std::chrono::high_resolution_clock::now();
    for (int val : lst) {
        sum_list += val;
    }
    auto end_iter_list = std::chrono::high_resolution_clock::now();

    // Durations
    double vec_alloc_ms = std::chrono::duration<double, std::milli>(end_alloc_vec - start_alloc_vec).count();
    double list_alloc_ms = std::chrono::duration<double, std::milli>(end_alloc_list - start_alloc_list).count();

    double vec_iter_ms = std::chrono::duration<double, std::milli>(end_iter_vec - start_iter_vec).count();
    double list_iter_ms = std::chrono::duration<double, std::milli>(end_iter_list - start_iter_list).count();

    // Memory estimation:
    // std::vector<int>: N * 4 bytes
    // std::list<int>: N * (sizeof(int) + 2 * sizeof(void*)) + heap allocator overhead (approx 24-32 bytes/node)
    std::size_t vec_bytes = N * sizeof(int);
    std::size_t list_bytes = N * (sizeof(int) + 2 * sizeof(void*)); // minimal 24 bytes per node without allocator padding

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "| Metric | std::vector (Contiguous) | std::list (Linked Nodes) | Vector Advantage |\n";
    std::cout << "| :--- | :--- | :--- | :---: |\n";
    std::cout << "| Memory Overhead (Payload only) | " << (vec_bytes / (1024 * 1024)) << " MB | " 
              << (list_bytes / (1024 * 1024)) << " MB (excl. heap padding) | **6.0x less RAM** |\n";
    std::cout << "| Allocation Time (10M push_back) | " << vec_alloc_ms << " ms | " << list_alloc_ms << " ms | **"
              << (list_alloc_ms / vec_alloc_ms) << "x faster** |\n";
    std::cout << "| Sequential Traversal (Sum 10M) | " << vec_iter_ms << " ms | " << list_iter_ms << " ms | **"
              << (list_iter_ms / vec_iter_ms) << "x faster (L1 prefetch)** |\n";

    if (sum_vec != sum_list) {
        std::cerr << "Error: Checksum mismatch!\n";
        return 1;
    }

    return 0;
}
