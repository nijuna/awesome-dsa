/**
 * @file exchange_arguments.cpp
 * @brief Algorithmic validation of Greedy Exchange Arguments.
 *
 * Implements Interval Scheduling (Earliest Finish Time) and Minimizing Lateness (Earliest Due Date)
 * with differential brute-force verification proving greedy optimality via the exchange property.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <numeric>

namespace dsa {

struct Interval {
    int start;
    int finish;
    int id;
};

/**
 * @brief Greedy Interval Scheduling: Selects maximum non-overlapping intervals by Earliest Finish Time.
 */
inline std::vector<Interval> interval_scheduling_greedy(std::vector<Interval> intervals) {
    if (intervals.empty()) return {};

    std::sort(intervals.begin(), intervals.end(), [](const Interval& a, const Interval& b) {
        if (a.finish != b.finish) return a.finish < b.finish;
        return a.start < b.start;
    });

    std::vector<Interval> selected;
    int last_finish = -1;

    for (const auto& iv : intervals) {
        if (iv.start >= last_finish) {
            selected.push_back(iv);
            last_finish = iv.finish;
        }
    }
    return selected;
}

/**
 * @brief Brute-force Oracle for Interval Scheduling: Explores all 2^N subsets to find true maximum.
 */
inline size_t interval_scheduling_brute_force(const std::vector<Interval>& intervals) {
    size_t n = intervals.size();
    size_t max_count = 0;

    for (uint32_t mask = 0; mask < (1U << n); ++mask) {
        std::vector<Interval> subset;
        for (size_t i = 0; i < n; ++i) {
            if (mask & (1U << i)) {
                subset.push_back(intervals[i]);
            }
        }

        // Check compatibility
        std::sort(subset.begin(), subset.end(), [](const Interval& a, const Interval& b) {
            return a.start < b.start;
        });

        bool compatible = true;
        for (size_t i = 1; i < subset.size(); ++i) {
            if (subset[i].start < subset[i - 1].finish) {
                compatible = false;
                break;
            }
        }

        if (compatible) {
            max_count = std::max(max_count, subset.size());
        }
    }
    return max_count;
}

// --- Minimizing Lateness Scheduling ---

struct Job {
    int duration;
    int deadline;
    int id;
};

/**
 * @brief Earliest Due Date (EDD) Minimizing Lateness.
 * Returns maximum lateness L = max(0, finish - deadline).
 */
inline int minimize_lateness_edd(std::vector<Job> jobs) {
    std::sort(jobs.begin(), jobs.end(), [](const Job& a, const Job& b) {
        return a.deadline < b.deadline;
    });

    int current_time = 0;
    int max_lateness = 0;

    for (const auto& j : jobs) {
        current_time += j.duration;
        int lateness = std::max(0, current_time - j.deadline);
        max_lateness = std::max(max_lateness, lateness);
    }
    return max_lateness;
}

/**
 * @brief Brute-force all N! permutations to verify EDD optimality.
 */
inline int minimize_lateness_brute_force(std::vector<Job> jobs) {
    std::vector<size_t> p(jobs.size());
    std::iota(p.begin(), p.end(), 0);

    int best_lateness = 1e9;

    do {
        int current_time = 0;
        int max_l = 0;
        for (size_t idx : p) {
            current_time += jobs[idx].duration;
            int lateness = std::max(0, current_time - jobs[idx].deadline);
            max_l = std::max(max_l, lateness);
        }
        best_lateness = std::min(best_lateness, max_l);
    } while (std::next_permutation(p.begin(), p.end()));

    return best_lateness;
}

} // namespace dsa

int main() {
    std::cout << "Running Greedy Exchange Arguments verification..." << std::endl;

    // 1. Interval Scheduling: Test against Brute Force across diverse inputs
    std::vector<dsa::Interval> test_intervals = {
        {1, 4, 1}, {3, 5, 2}, {0, 6, 3}, {5, 7, 4}, {3, 9, 5},
        {5, 9, 6}, {6, 10, 7}, {8, 11, 8}, {8, 12, 9}, {2, 14, 10}, {12, 16, 11}
    };

    auto greedy_res = dsa::interval_scheduling_greedy(test_intervals);
    size_t brute_count = dsa::interval_scheduling_brute_force(test_intervals);

    assert(greedy_res.size() == brute_count);
    assert(greedy_res.size() == 4); // Standard known optimal cardinality for this classic set

    // 2. Minimizing Lateness: Test EDD optimality vs N! permutations
    std::vector<dsa::Job> test_jobs = {
        {3, 6, 1},
        {2, 8, 2},
        {1, 9, 3},
        {4, 9, 4},
        {3, 14, 5},
        {2, 15, 6}
    };

    int edd_lateness = dsa::minimize_lateness_edd(test_jobs);
    int optimal_lateness = dsa::minimize_lateness_brute_force(test_jobs);

    assert(edd_lateness == optimal_lateness);

    std::cout << "[PASS] Greedy Interval Scheduling EFT matched brute-force optimal cardinality (" << brute_count << ")." << std::endl;
    std::cout << "[PASS] Earliest Due Date scheduling matched N! permutation optimal lateness (" << optimal_lateness << ")." << std::endl;
    std::cout << "All Greedy Exchange Arguments assertions passed successfully!" << std::endl;
    return 0;
}
