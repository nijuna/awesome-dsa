/**
 * @file branch_prediction_and_pipelines.cpp
 * @brief Reference implementations and hardware-aware primitives exploring CPU branch prediction and pipelines.
 *
 * Implements branchless selection, clamping, min/max, branchless binary search,
 * and a 2-bit saturating counter branch predictor simulation.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <random>
#include <chrono>

namespace dsa {

/**
 * @brief Branchy conditional accumulation (subject to branch misprediction on random data).
 */
inline int64_t conditional_sum_branchy(const std::vector<int>& data, int threshold) {
    int64_t sum = 0;
    for (int x : data) {
        if (x >= threshold) {
            sum += x;
        }
    }
    return sum;
}

/**
 * @brief Branchless conditional accumulation using arithmetic multiplication / conditional select.
 */
inline int64_t conditional_sum_branchless(const std::vector<int>& data, int threshold) {
    int64_t sum = 0;
    for (int x : data) {
        // (x >= threshold) evaluates to 0 or 1; multiplies without jumping
        sum += static_cast<int64_t>(x) * (x >= threshold);
    }
    return sum;
}

/**
 * @brief Standard branchy select.
 */
inline int select_branchy(bool cond, int a, int b) {
    return cond ? a : b;
}

/**
 * @brief Branchless selection via bitwise mask.
 * When cond is true, mask is ~0 (all 1s). When false, mask is 0.
 */
inline int select_branchless(bool cond, int a, int b) {
    uint32_t mask = static_cast<uint32_t>(0) - static_cast<uint32_t>(cond ? 1 : 0);
    return static_cast<int>((static_cast<uint32_t>(a) & mask) | (static_cast<uint32_t>(b) & ~mask));
}

/**
 * @brief Branchless minimum for 32-bit integers without comparison jump.
 */
inline int min_branchless(int a, int b) {
    int64_t diff = static_cast<int64_t>(a) - static_cast<int64_t>(b);
    int64_t mask = diff >> 63; // -1 if a < b, 0 if a >= b
    return static_cast<int>(b + (diff & mask));
}

/**
 * @brief Branchless maximum for 32-bit integers without comparison jump.
 */
inline int max_branchless(int a, int b) {
    int64_t diff = static_cast<int64_t>(a) - static_cast<int64_t>(b);
    int64_t mask = diff >> 63; // -1 if a < b, 0 if a >= b
    return static_cast<int>(a - (diff & mask));
}

/**
 * @brief Branchless clamping between [low, high].
 */
inline int clamp_branchless(int x, int low, int high) {
    return max_branchless(low, min_branchless(x, high));
}

/**
 * @brief Branchless binary search (lower bound).
 * Avoids unpredictable mid-point conditional jumps by using conditional pointer advancement.
 */
inline int branchless_lower_bound(const std::vector<int>& arr, int target) {
    int n = static_cast<int>(arr.size());
    if (n == 0) return 0;

    int base = 0;
    while (n > 1) {
        int half = n / 2;
        // Compiles to CMOV on x86-64 without conditional jump
        base = (arr[base + half] < target) ? base + half : base;
        n -= half;
    }
    return (arr[base] < target) ? base + 1 : base;
}

/**
 * @brief 2-bit saturating counter branch predictor simulation.
 * States:
 *  0: Strongly Not Taken (SNT)
 *  1: Weakly Not Taken (WNT)
 *  2: Weakly Taken (WT)
 *  3: Strongly Taken (ST)
 */
class BimodalPredictor {
private:
    std::vector<uint8_t> table_;
    size_t mask_;

public:
    explicit BimodalPredictor(size_t table_size = 1024) {
        // Ensure power of 2
        size_t size = 1;
        while (size < table_size) size <<= 1;
        table_.assign(size, 2); // default Weakly Taken
        mask_ = size - 1;
    }

    bool predict(uint64_t pc) const {
        size_t idx = pc & mask_;
        return table_[idx] >= 2;
    }

    void update(uint64_t pc, bool taken) {
        size_t idx = pc & mask_;
        uint8_t state = table_[idx];
        if (taken) {
            if (state < 3) table_[idx] = state + 1;
        } else {
            if (state > 0) table_[idx] = state - 1;
        }
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Branch Prediction & CPU Pipeline tests..." << std::endl;

    // Test 1: Branchless selection equivalence
    for (int a : {-100, -1, 0, 1, 42, 9999}) {
        for (int b : {-500, 0, 7, 100, 12345}) {
            assert(dsa::select_branchless(true, a, b) == dsa::select_branchy(true, a, b));
            assert(dsa::select_branchless(false, a, b) == dsa::select_branchy(false, a, b));
            assert(dsa::min_branchless(a, b) == std::min(a, b));
        }
    }

    // Test 2: Clamp verification
    for (int x = -100; x <= 100; ++x) {
        int clamped_ref = std::clamp(x, -20, 50);
        int clamped_val = dsa::clamp_branchless(x, -20, 50);
        assert(clamped_ref == clamped_val);
    }

    // Test 3: Branchless lower bound verification
    std::vector<int> sorted_arr = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
    for (int target = 0; target <= 100; ++target) {
        auto it = std::lower_bound(sorted_arr.begin(), sorted_arr.end(), target);
        int expected_idx = static_cast<int>(std::distance(sorted_arr.begin(), it));
        int actual_idx = dsa::branchless_lower_bound(sorted_arr, target);
        assert(expected_idx == actual_idx);
    }

    // Empty array and single element tests
    assert(dsa::branchless_lower_bound({}, 42) == 0);
    assert(dsa::branchless_lower_bound({10}, 5) == 0);
    assert(dsa::branchless_lower_bound({10}, 10) == 0);
    assert(dsa::branchless_lower_bound({10}, 15) == 1);

    // Test 4: Conditional accumulation equivalence
    std::mt19937 rng(1337);
    std::uniform_int_distribution<int> dist(0, 255);
    std::vector<int> data(10000);
    for (auto& x : data) x = dist(rng);

    int64_t sum1 = dsa::conditional_sum_branchy(data, 128);
    int64_t sum2 = dsa::conditional_sum_branchless(data, 128);
    assert(sum1 == sum2);

    // Test 5: 2-bit Saturating Counter Simulation
    dsa::BimodalPredictor predictor(64);
    uint64_t branch_pc = 0x4010a0;

    // Pattern: 100 consecutive 'Taken' branches (e.g. inner loop iterations)
    size_t correct_predictions = 0;
    for (int i = 0; i < 100; ++i) {
        bool pred = predictor.predict(branch_pc);
        if (pred == true) correct_predictions++;
        predictor.update(branch_pc, true);
    }
    // After warming up, accuracy should be >= 98%
    assert(correct_predictions >= 98);

    // One anomalous exit: Not Taken
    predictor.update(branch_pc, false);
    // Due to hysteresis, the next prediction should still be Taken (transition from ST to WT)
    assert(predictor.predict(branch_pc) == true);

    std::cout << "All Branch Prediction & CPU Pipeline assertions passed successfully!" << std::endl;
    return 0;
}
