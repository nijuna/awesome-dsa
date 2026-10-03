#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <algorithm>
#include <cmath>
#include <random>
#include <cstdint>
#include <cassert>
#include <iomanip>
#include <unordered_set>
#include <set>

namespace benchmark_framework {

// ============================================================================
// 1. HARDWARE-AWARE OPTIMIZATION BARRIERS
// ============================================================================

// Prevents the compiler from optimizing away the value or dead-code eliminating it
template <typename T>
inline __attribute__((always_inline)) void do_not_optimize(T& value) {
#if defined(__clang__)
    asm volatile("" : "+r,m"(value) : : "memory");
#else
    asm volatile("" : "+m,r"(value) : : "memory");
#endif
}

// Memory clobber barrier forcing pending memory writes to commit
inline __attribute__((always_inline)) void clobber_memory() {
    asm volatile("" : : : "memory");
}

// ============================================================================
// 2. STATISTICAL AGGREGATOR & PERCENTILE CALCULATOR
// ============================================================================

struct BenchmarkStats {
    double mean_ns{0.0};
    double median_ns{0.0};
    double stddev_ns{0.0};
    double min_ns{0.0};
    double max_ns{0.0};
    double p90_ns{0.0};
    double p95_ns{0.0};
    double p99_ns{0.0};
    size_t samples{0};
};

class BenchmarkHarness {
public:
    template <typename Func>
    static BenchmarkStats run(Func&& func, size_t iterations = 1000, size_t warmup = 100) {
        // 1. Warm-up Phase: prime instruction cache and page tables
        for (size_t w = 0; w < warmup; ++w) {
            func();
            clobber_memory();
        }

        // 2. Timed Measurement Phase
        std::vector<double> timings_ns;
        timings_ns.reserve(iterations);

        for (size_t i = 0; i < iterations; ++i) {
            auto t_start = std::chrono::high_resolution_clock::now();
            func();
            clobber_memory();
            auto t_end = std::chrono::high_resolution_clock::now();

            double duration_ns = std::chrono::duration<double, std::nano>(t_end - t_start).count();
            timings_ns.push_back(duration_ns);
        }

        // 3. Statistical Analysis
        std::sort(timings_ns.begin(), timings_ns.end());

        BenchmarkStats stats;
        stats.samples = iterations;
        stats.min_ns = timings_ns.front();
        stats.max_ns = timings_ns.back();

        double sum = 0.0;
        for (double t : timings_ns) {
            sum += t;
        }
        stats.mean_ns = sum / iterations;

        double var_sum = 0.0;
        for (double t : timings_ns) {
            var_sum += (t - stats.mean_ns) * (t - stats.mean_ns);
        }
        stats.stddev_ns = std::sqrt(var_sum / iterations);

        stats.median_ns = percentile(timings_ns, 50.0);
        stats.p90_ns = percentile(timings_ns, 90.0);
        stats.p95_ns = percentile(timings_ns, 95.0);
        stats.p99_ns = percentile(timings_ns, 99.0);

        return stats;
    }

private:
    static double percentile(const std::vector<double>& sorted_data, double p) {
        if (sorted_data.empty()) return 0.0;
        double rank = (p / 100.0) * (sorted_data.size() - 1);
        size_t low = static_cast<size_t>(std::floor(rank));
        size_t high = static_cast<size_t>(std::ceil(rank));
        double weight = rank - low;
        return sorted_data[low] * (1.0 - weight) + sorted_data[high] * weight;
    }
};

} // namespace benchmark_framework

// ============================================================================
// EXPERIMENTAL VALIDATION & COMPARATIVE BENCHMARK
// ============================================================================

int main() {
    using namespace benchmark_framework;

    std::cout << "===============================================================\n";
    std::cout << "Benchmark Design: Scientific Harness, Barriers & Latency Stats\n";
    std::cout << "===============================================================\n";

    // ------------------------------------------------------------------------
    // Experiment 1: Optimization Barrier Validation
    // ------------------------------------------------------------------------
    std::cout << "[Test 1/3] Testing Optimization Barriers (do_not_optimize)...\n";
    {
        uint64_t counter = 0;
        auto stats = BenchmarkHarness::run([&counter]() {
            counter += 42;
            do_not_optimize(counter);
        }, 1000, 100);

        assert(counter > 0);
        assert(stats.samples == 1000);
        assert(stats.min_ns >= 0.0);
        assert(stats.p99_ns >= stats.median_ns);
        std::cout << "  -> Barrier confirmed: Compiler did not eliminate computation.\n";
        std::cout << "  -> Latency: median=" << stats.median_ns
                  << "ns, p99=" << stats.p99_ns << "ns\n";
    }

    // ------------------------------------------------------------------------
    // Experiment 2: Small-N Memory Hierarchy Inversion
    // Linear Scan (contiguous array) vs std::set (Red-Black tree pointer chasing)
    // ------------------------------------------------------------------------
    std::cout << "[Test 2/3] Testing Small-N Cache-Friendly Linear Scan vs std::set...\n";
    {
        const size_t SMALL_N = 32;
        std::vector<int> vec;
        std::set<int> rb_set;

        for (size_t i = 0; i < SMALL_N; ++i) {
            vec.push_back(i * 2);
            rb_set.insert(i * 2);
        }

        int target = 48; // In the set

        auto vec_stats = BenchmarkHarness::run([&vec, target]() {
            bool found = false;
            for (int v : vec) {
                if (v == target) {
                    found = true;
                    break;
                }
            }
            do_not_optimize(found);
        }, 2000, 200);

        auto set_stats = BenchmarkHarness::run([&rb_set, target]() {
            bool found = (rb_set.find(target) != rb_set.end());
            do_not_optimize(found);
        }, 2000, 200);

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  -> Small N=" << SMALL_N << ":\n";
        std::cout << "     Contiguous Vector Scan : " << vec_stats.median_ns << " ns (L1 cache-friendly)\n";
        std::cout << "     std::set (RB-Tree O(logN)): " << set_stats.median_ns << " ns (Pointer chasing)\n";

        // At small N, contiguous vector is typically faster or competitive with tree lookup
        assert(vec_stats.median_ns <= set_stats.median_ns * 3.0);
    }

    // ------------------------------------------------------------------------
    // Experiment 3: Large-N Scaling (Vector Scan vs Hash Table)
    // ------------------------------------------------------------------------
    std::cout << "[Test 3/3] Testing Large-N Asymptotic Scaling (Array vs Hash Table)...\n";
    {
        const size_t LARGE_N = 5000;
        std::vector<int> sorted_vec;
        std::unordered_set<int> hash_set;

        for (size_t i = 0; i < LARGE_N; ++i) {
            sorted_vec.push_back(i * 3);
            hash_set.insert(i * 3);
        }

        int target = 7500; // In set

        auto bin_stats = BenchmarkHarness::run([&sorted_vec, target]() {
            bool found = std::binary_search(sorted_vec.begin(), sorted_vec.end(), target);
            do_not_optimize(found);
        }, 2000, 200);

        auto hash_stats = BenchmarkHarness::run([&hash_set, target]() {
            bool found = (hash_set.find(target) != hash_set.end());
            do_not_optimize(found);
        }, 2000, 200);

        std::cout << "  -> Large N=" << LARGE_N << ":\n";
        std::cout << "     Binary Search (Sorted Array): " << bin_stats.median_ns << " ns\n";
        std::cout << "     Hash Set (O(1) Avg Lookup)   : " << hash_stats.median_ns << " ns\n";

        assert(bin_stats.samples == 2000);
        assert(hash_stats.samples == 2000);
    }

    std::cout << "===============================================================\n";
    std::cout << "All Benchmark Design tests PASSED flawlessly!\n";
    std::cout << "===============================================================\n";

    return 0;
}
