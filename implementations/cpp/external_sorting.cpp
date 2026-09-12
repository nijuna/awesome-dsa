/**
 * @file external_sorting.cpp
 * @brief Reference implementation of Two-Phase Multi-Way External Merge Sort.
 *
 * Implements Phase 1 (Run generation of size M) and Phase 2 (K-way merge via Min-Heap),
 * verifying correct sorted ordering and permutation invariance.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

namespace dsa {

struct HeapEntry {
    int value;
    size_t run_idx;
    size_t element_idx;

    bool operator>(const HeapEntry& other) const {
        return value > other.value;
    }
};

/**
 * @brief Simulates Multi-Way External Merge Sort.
 * @param input The unsorted dataset (size N).
 * @param M RAM capacity (elements that can be sorted in memory at once).
 * @return The completely sorted dataset.
 */
inline std::vector<int> external_merge_sort(const std::vector<int>& input, size_t M) {
    assert(M > 0);
    size_t n = input.size();
    if (n <= M) {
        std::vector<int> sorted = input;
        std::sort(sorted.begin(), sorted.end());
        return sorted;
    }

    // --- Phase 1: Run Formation ---
    std::vector<std::vector<int>> runs;
    for (size_t i = 0; i < n; i += M) {
        size_t end_idx = std::min(i + M, n);
        std::vector<int> chunk(input.begin() + i, input.begin() + end_idx);
        std::sort(chunk.begin(), chunk.end());
        runs.push_back(std::move(chunk));
    }

    // --- Phase 2: K-Way Merge ---
    size_t k = runs.size();
    std::priority_queue<HeapEntry, std::vector<HeapEntry>, std::greater<HeapEntry>> min_heap;

    // Initialize heap with the first element of each run
    for (size_t r = 0; r < k; ++r) {
        if (!runs[r].empty()) {
            min_heap.push({runs[r][0], r, 0});
        }
    }

    std::vector<int> output;
    output.reserve(n);

    while (!min_heap.empty()) {
        HeapEntry top = min_heap.top();
        min_heap.pop();

        output.push_back(top.value);

        // Advance in the corresponding run
        size_t next_elem_idx = top.element_idx + 1;
        if (next_elem_idx < runs[top.run_idx].size()) {
            min_heap.push({runs[top.run_idx][next_elem_idx], top.run_idx, next_elem_idx});
        }
    }

    return output;
}

} // namespace dsa

int main() {
    std::cout << "Running Multi-Way External Merge Sort verification..." << std::endl;

    const size_t N = 10000;
    const size_t M = 250; // RAM can only hold 250 elements at a time -> 40 runs

    std::mt19937 rng(42);
    std::uniform_int_distribution<int> dist(-50000, 50000);

    std::vector<int> dataset(N);
    for (auto& x : dataset) x = dist(rng);

    // Run External Sort
    std::vector<int> sorted_result = dsa::external_merge_sort(dataset, M);

    // 1. Size invariance
    assert(sorted_result.size() == N);

    // 2. Ordering check
    for (size_t i = 1; i < N; ++i) {
        assert(sorted_result[i - 1] <= sorted_result[i]);
    }

    // 3. Oracle comparison against std::sort
    std::vector<int> oracle_sorted = dataset;
    std::sort(oracle_sorted.begin(), oracle_sorted.end());
    assert(sorted_result == oracle_sorted);

    std::cout << "[PASS] Successfully sorted " << N << " items with simulated RAM capacity M = " << M << "." << std::endl;
    std::cout << "[PASS] K-way merge result strictly matches std::sort oracle." << std::endl;
    std::cout << "All Multi-Way External Merge Sort assertions passed successfully!" << std::endl;
    return 0;
}
