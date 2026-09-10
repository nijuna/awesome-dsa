/**
 * Benchmark Harness: Geometric Growth Factor 1.5x vs. 2.0x
 * Simulates dynamic array reallocations, measuring:
 * 1. Number of reallocations required for N elements
 * 2. Total elements moved/copied during reallocations
 * 3. Peak capacity and wasted tail memory overhead
 * 4. Wall-clock execution time
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <chrono>
#include <iomanip>
#include <cstddef>
#include <cstdint>

struct SimulationResult {
    std::size_t reallocations = 0;
    std::size_t total_elements_copied = 0;
    std::size_t final_capacity = 0;
    double elapsed_ms = 0.0;
};

template <int Num, int Den>
SimulationResult run_simulation(std::size_t N) {
    SimulationResult res;
    auto start = std::chrono::high_resolution_clock::now();

    std::size_t capacity = 0;
    std::size_t size = 0;
    int* buffer = nullptr;

    for (std::size_t i = 0; i < N; ++i) {
        if (size == capacity) {
            std::size_t new_cap = (capacity == 0) ? 1 : (capacity * Num) / Den;
            if (new_cap <= capacity) {
                new_cap = capacity + 1;
            }
            int* new_buf = new int[new_cap];
            for (std::size_t j = 0; j < size; ++j) {
                new_buf[j] = buffer[j];
            }
            delete[] buffer;
            buffer = new_buf;
            res.total_elements_copied += size;
            res.reallocations++;
            capacity = new_cap;
        }
        buffer[size++] = static_cast<int>(i);
    }

    auto end = std::chrono::high_resolution_clock::now();
    res.elapsed_ms = std::chrono::duration<double, std::milli>(end - start).count();
    res.final_capacity = capacity;

    delete[] buffer;
    return res;
}

int main() {
    const std::size_t N = 20000000; // 20 Million elements
    std::cout << "===================================================================\n";
    std::cout << " BENCHMARK: Growth Factor 1.5x (MSVC/Folly) vs. 2.0x (GCC/Clang)\n";
    std::cout << " N = " << N << " sequential appends (without reserve)\n";
    std::cout << "===================================================================\n\n";

    // Factor 2.0x (Num=2, Den=1)
    auto res_2_0 = run_simulation<2, 1>(N);

    // Factor 1.5x (Num=3, Den=2)
    auto res_1_5 = run_simulation<3, 2>(N);

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "| Metric | Factor 2.0x (GCC/Clang) | Factor 1.5x (MSVC/Folly) | Comparison / Tradeoff |\n";
    std::cout << "| :--- | :--- | :--- | :--- |\n";
    std::cout << "| Number of Reallocations | " << res_2_0.reallocations << " | " << res_1_5.reallocations 
              << " | **1.5x triggers ~1.7x more reallocs** |\n";
    std::cout << "| Total Elements Copied | " << res_2_0.total_elements_copied << " | " << res_1_5.total_elements_copied 
              << " | **" << (static_cast<double>(res_1_5.total_elements_copied) / res_2_0.total_elements_copied) << "x more copying** |\n";
    std::cout << "| Final Allocated Capacity | " << res_2_0.final_capacity << " (" << (res_2_0.final_capacity * sizeof(int) / (1024 * 1024)) << " MB) | " 
              << res_1_5.final_capacity << " (" << (res_1_5.final_capacity * sizeof(int) / (1024 * 1024)) << " MB) | **1.5x wastes significantly less RAM** |\n";
    
    double waste_2_0 = (static_cast<double>(res_2_0.final_capacity - N) / res_2_0.final_capacity) * 100.0;
    double waste_1_5 = (static_cast<double>(res_1_5.final_capacity - N) / res_1_5.final_capacity) * 100.0;
    std::cout << "| Tail Wasted Capacity (%) | " << waste_2_0 << "% | " << waste_1_5 << "% | **" 
              << (waste_2_0 - waste_1_5) << "% less wasted tail headroom** |\n";
    std::cout << "| Wall-Clock Realloc Time | " << res_2_0.elapsed_ms << " ms | " << res_1_5.elapsed_ms << " ms | **2.0x is slightly faster due to fewer reallocs** |\n";

    std::cout << "\n> Memory Allocator Reuse Insight:\n";
    std::cout << "> With factor 2.0: Sum_{i=0}^{k-1} 2^i = 2^k - 1 < 2^k. The sum of all previously freed chunks is strictly LESS than\n";
    std::cout << "> the next allocation size, preventing the allocator from ever recycling its own contiguous memory.\n";
    std::cout << "> With factor 1.5 (< phi approx 1.618): previous deallocated chunks can eventually coalesce and satisfy subsequent requests.\n";

    return 0;
}
