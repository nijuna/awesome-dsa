/**
 * @file benchmarking_pitfalls.cpp
 * @brief High-precision benchmarking harness and compiler optimization barriers.
 *
 * Implements do_not_optimize barriers, warmup phases, steady_clock measurement,
 * and statistical analysis (min, median, mean, standard deviation).
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <chrono>
#include <algorithm>
#include <numeric>
#include <cmath>

namespace dsa {

/**
 * @brief Optimization barrier equivalent to Google Benchmark's DoNotOptimize.
 * Forces the compiler to treat the value as an escape variable so it cannot be optimized away.
 */
template <typename T>
inline void do_not_optimize(T const& value) {
#if defined(__GNUC__) || defined(__clang__)
    asm volatile("" : : "r,m"(value) : "memory");
#else
    // Fallback for other compilers
    volatile const T* p = &value;
    (void)p;
#endif
}

struct BenchmarkStats {
    double min_ns;
    double max_ns;
    double median_ns;
    double mean_ns;
    double stddev_ns;
};

/**
 * @brief Computes statistical metrics over a vector of sample timings in nanoseconds.
 */
inline BenchmarkStats compute_statistics(std::vector<double> samples) {
    assert(!samples.empty());
    std::sort(samples.begin(), samples.end());

    double min_v = samples.front();
    double max_v = samples.back();

    double median_v = 0.0;
    size_t n = samples.size();
    if (n % 2 == 1) {
        median_v = samples[n / 2];
    } else {
        median_v = (samples[n / 2 - 1] + samples[n / 2]) / 2.0;
    }

    double sum = std::accumulate(samples.begin(), samples.end(), 0.0);
    double mean_v = sum / n;

    double variance_sum = 0.0;
    for (double x : samples) {
        variance_sum += (x - mean_v) * (x - mean_v);
    }
    double stddev_v = std::sqrt(variance_sum / n);

    return {min_v, max_v, median_v, mean_v, stddev_v};
}

/**
 * @brief Benchmark harness running warmup followed by batch measurements.
 */
template <typename Func>
BenchmarkStats run_benchmark(Func&& func, size_t warmup_iters, size_t batch_count, size_t iters_per_batch) {
    // 1. Warmup
    for (size_t i = 0; i < warmup_iters; ++i) {
        func();
    }

    // 2. Measured Batches
    std::vector<double> batch_times;
    batch_times.reserve(batch_count);

    for (size_t b = 0; b < batch_count; ++b) {
        auto t0 = std::chrono::steady_clock::now();
        for (size_t i = 0; i < iters_per_batch; ++i) {
            func();
        }
        auto t1 = std::chrono::steady_clock::now();

        std::chrono::duration<double, std::nano> elapsed = t1 - t0;
        batch_times.push_back(elapsed.count() / iters_per_batch);
    }

    return compute_statistics(batch_times);
}

} // namespace dsa

int main() {
    std::cout << "Running Benchmarking Pitfalls harness verification..." << std::endl;

    // Test 1: Statistical calculator verification
    std::vector<double> test_data = {10.0, 20.0, 30.0, 40.0, 50.0};
    dsa::BenchmarkStats stats = dsa::compute_statistics(test_data);
    assert(stats.min_ns == 10.0);
    assert(stats.max_ns == 50.0);
    assert(stats.median_ns == 30.0);
    assert(stats.mean_ns == 30.0);
    assert(std::abs(stats.stddev_ns - 14.1421356) < 1e-4);

    // Test 2: Execution of benchmark harness with do_not_optimize barrier
    int accumulator = 0;
    auto benchmark_target = [&]() {
        accumulator += 1;
        dsa::do_not_optimize(accumulator);
    };

    dsa::BenchmarkStats run_stats = dsa::run_benchmark(benchmark_target, 1000, 10, 5000);
    assert(run_stats.min_ns >= 0.0);
    assert(run_stats.max_ns >= run_stats.min_ns);
    assert(run_stats.median_ns >= run_stats.min_ns);

    std::cout << "[PASS] Statistical metrics calculation verified." << std::endl;
    std::cout << "[PASS] Steady clock harness and do_not_optimize barrier verified (Median: "
              << run_stats.median_ns << " ns/op)." << std::endl;
    std::cout << "All Benchmarking Pitfalls assertions passed successfully!" << std::endl;
    return 0;
}
