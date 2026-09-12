/**
 * @file mo_algorithm.cpp
 * @brief Mo's Algorithm for Offline Range Queries with Block Decomposition,
 *        Zig-Zag Alternation, Hilbert Curve Ordering, and Differential Verification.
 *
 * Implements Arthur's Two-Layer API:
 *   - Layer 1: Query ordering predicates (Block Zig-Zag, Hilbert curve 1D projection),
 *              active window [cur_L, cur_R] maintenance, incremental add/remove transitions.
 *   - Layer 2: Safe MoEngine providing:
 *              1. solveDistinctElements() [Count distinct items in O((N + Q) * sqrt(N))]
 *              2. solveSumOfSquaredFrequencies() [Codeforces 86D Powerful Array]
 *              3. naiveDistinctQueries() [O(Q * N) verification oracle]
 *              4. naivePowerfulQueries() [O(Q * N) verification oracle]
 *
 * Compilation:
 *   g++ -std=c++17 -O3 -Wall -Wextra -Werror mo_algorithm.cpp -o mo_algo
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <unordered_set>
#include <unordered_map>
#include <random>

namespace mo {

// ============================================================================
// Layer 1: Query Representation, Comparators & Hilbert Ordering
// ============================================================================

struct Query {
    int id;
    int l;
    int r;
    int block;
    int64_t hilbert_ord{0};

    Query(int q_id, int left, int right, int b_size = 1)
        : id(q_id), l(left), r(right), block(left / b_size) {}
};

/**
 * @brief Computes 1D coordinate along a 2D Hilbert curve using bit manipulation.
 */
inline int64_t hilbert_order(int x, int y, int pow, int rotate) {
    if (pow == 0) return 0;
    int hpow = 1 << (pow - 1);
    int seg = (x < hpow) ? ((y < hpow) ? 0 : 3) : ((y < hpow) ? 1 : 2);
    seg = (seg + rotate) & 3;
    const int rotate_delta[4] = {3, 0, 0, 1};
    int nx = x & (x ^ hpow), ny = y & (y ^ hpow);
    int nrot = (rotate + rotate_delta[seg]) & 3;
    int64_t sub_square_size = int64_t(1) << (2 * pow - 2);
    int64_t ans = seg * sub_square_size;
    int64_t add = hilbert_order(nx, ny, pow - 1, nrot);
    ans += (seg == 1 || seg == 2) ? add : (sub_square_size - add - 1);
    return ans;
}

// Zig-Zag comparator: sorts by block, then alternates R ascending/descending
struct MoBlockComparator {
    bool operator()(Query const& a, Query const& b) const {
        if (a.block != b.block) {
            return a.block < b.block;
        }
        return (a.block & 1) ? (a.r < b.r) : (a.r > b.r);
    }
};

// Hilbert comparator: global optimal Manhattan distance order
struct MoHilbertComparator {
    bool operator()(Query const& a, Query const& b) const {
        return a.hilbert_ord < b.hilbert_ord;
    }
};

// ============================================================================
// Layer 2: High-Level Mo Engine
// ============================================================================

class MoEngine {
public:
    /**
     * @brief Solves Distinct Elements Query (DQUERY) in O((N + Q) * sqrt(N)).
     * Returns count of distinct values in range [L, R] for each query.
     */
    static std::vector<int> solveDistinctElements(std::vector<int> const& arr,
                                                  std::vector<std::pair<int, int>> const& raw_queries,
                                                  bool use_hilbert = true) {
        int n = static_cast<int>(arr.size());
        int q_count = static_cast<int>(raw_queries.size());
        if (q_count == 0) return {};

        int block_size = std::max(1, static_cast<int>(n / std::sqrt(q_count)));
        std::vector<Query> queries;
        queries.reserve(q_count);

        for (int i = 0; i < q_count; ++i) {
            Query q(i, raw_queries[i].first, raw_queries[i].second, block_size);
            if (use_hilbert) {
                q.hilbert_ord = hilbert_order(q.l, q.r, 21, 0);
            }
            queries.push_back(q);
        }

        if (use_hilbert) {
            std::sort(queries.begin(), queries.end(), MoHilbertComparator());
        } else {
            std::sort(queries.begin(), queries.end(), MoBlockComparator());
        }

        // Coordinate frequency table
        // Find max element to size frequency table or use hash map
        int max_val = 0;
        for (int x : arr) if (x > max_val) max_val = x;

        std::vector<int> freq(max_val + 1, 0);
        int current_distinct = 0;
        std::vector<int> answers(q_count);

        auto add = [&](int idx) {
            int val = arr[idx];
            if (freq[val] == 0) {
                current_distinct++;
            }
            freq[val]++;
        };

        auto remove = [&](int idx) {
            int val = arr[idx];
            freq[val]--;
            if (freq[val] == 0) {
                current_distinct--;
            }
        };

        int cur_l = 0;
        int cur_r = -1;

        for (auto const& q : queries) {
            while (cur_l > q.l) add(--cur_l);
            while (cur_r < q.r) add(++cur_r);
            while (cur_l < q.l) remove(cur_l++);
            while (cur_r > q.r) remove(cur_r--);
            answers[q.id] = current_distinct;
        }

        return answers;
    }

    /**
     * @brief Solves Powerful Array (Sum of freq[x]^2 * x) in O((N + Q) * sqrt(N)).
     */
    static std::vector<int64_t> solvePowerfulArray(std::vector<int> const& arr,
                                                   std::vector<std::pair<int, int>> const& raw_queries) {
        int n = static_cast<int>(arr.size());
        int q_count = static_cast<int>(raw_queries.size());
        if (q_count == 0) return {};

        int block_size = std::max(1, static_cast<int>(n / std::sqrt(q_count)));
        std::vector<Query> queries;
        queries.reserve(q_count);

        for (int i = 0; i < q_count; ++i) {
            Query q(i, raw_queries[i].first, raw_queries[i].second, block_size);
            q.hilbert_ord = hilbert_order(q.l, q.r, 21, 0);
            queries.push_back(q);
        }

        std::sort(queries.begin(), queries.end(), MoHilbertComparator());

        int max_val = 0;
        for (int x : arr) if (x > max_val) max_val = x;

        std::vector<int64_t> freq(max_val + 1, 0);
        int64_t current_sum = 0;
        std::vector<int64_t> answers(q_count);

        auto add = [&](int idx) {
            int64_t val = arr[idx];
            int64_t f = freq[val];
            // Subtract previous f^2 * val, add (f + 1)^2 * val: delta = (2*f + 1) * val
            current_sum += (2 * f + 1) * val;
            freq[val] = f + 1;
        };

        auto remove = [&](int idx) {
            int64_t val = arr[idx];
            int64_t f = freq[val];
            // Subtract f^2 * val, add (f - 1)^2 * val: delta = (1 - 2*f) * val
            current_sum -= (2 * f - 1) * val;
            freq[val] = f - 1;
        };

        int cur_l = 0;
        int cur_r = -1;

        for (auto const& q : queries) {
            while (cur_l > q.l) add(--cur_l);
            while (cur_r < q.r) add(++cur_r);
            while (cur_l < q.l) remove(cur_l++);
            while (cur_r > q.r) remove(cur_r--);
            answers[q.id] = current_sum;
        }

        return answers;
    }

    // ========================================================================
    // Differential Verification Oracles
    // ========================================================================

    static std::vector<int> naiveDistinctQueries(std::vector<int> const& arr,
                                                 std::vector<std::pair<int, int>> const& queries) {
        std::vector<int> results;
        results.reserve(queries.size());
        for (auto const& q : queries) {
            std::unordered_set<int> seen;
            for (int i = q.first; i <= q.second; ++i) {
                seen.insert(arr[i]);
            }
            results.push_back(static_cast<int>(seen.size()));
        }
        return results;
    }

    static std::vector<int64_t> naivePowerfulQueries(std::vector<int> const& arr,
                                                     std::vector<std::pair<int, int>> const& queries) {
        std::vector<int64_t> results;
        results.reserve(queries.size());
        for (auto const& q : queries) {
            std::unordered_map<int, int64_t> freq;
            for (int i = q.first; i <= q.second; ++i) {
                freq[arr[i]]++;
            }
            int64_t total = 0;
            for (auto const& pair : freq) {
                total += pair.second * pair.second * pair.first;
            }
            results.push_back(total);
        }
        return results;
    }
};

} // namespace mo

// ============================================================================
// Verification Suite
// ============================================================================

void run_unit_tests() {
    using namespace mo;

    // Array: [1, 1, 2, 1, 3]
    std::vector<int> arr = {1, 1, 2, 1, 3};
    std::vector<std::pair<int, int>> queries = {
        {0, 4}, // [1, 1, 2, 1, 3] -> 3 distinct {1, 2, 3}
        {1, 3}, // [1, 2, 1]       -> 2 distinct {1, 2}
        {2, 4}, // [2, 1, 3]       -> 3 distinct {2, 1, 3}
        {0, 1}  // [1, 1]          -> 1 distinct {1}
    };

    auto res_distinct = MoEngine::solveDistinctElements(arr, queries);
    std::vector<int> exp_distinct = {3, 2, 3, 1};
    assert(res_distinct == exp_distinct);

    // Powerful Array: sum of freq^2 * val
    // Q0 [0, 4]: 1 occurs 3 times (9*1=9), 2 occurs 1 time (1*2=2), 3 occurs 1 time (1*3=3) -> 9+2+3 = 14
    // Q1 [1, 3]: 1 occurs 2 times (4*1=4), 2 occurs 1 time (1*2=2) -> 4+2 = 6
    auto res_power = MoEngine::solvePowerfulArray(arr, queries);
    std::vector<int64_t> exp_power = {14, 6, 6, 4};
    assert(res_power == exp_power);
}

void run_differential_stress_tests() {
    using namespace mo;
    std::mt19937 rng(1337);

    for (int trial = 0; trial < 40; ++trial) {
        int n = 80;
        int q = 60;
        std::vector<int> arr(n);
        for (int i = 0; i < n; ++i) {
            arr[i] = (rng() % 30) + 1; // values 1..30
        }

        std::vector<std::pair<int, int>> queries;
        for (int i = 0; i < q; ++i) {
            int l = rng() % n;
            int r = rng() % n;
            if (l > r) std::swap(l, r);
            queries.emplace_back(l, r);
        }

        // 1. Verify Distinct Elements with Block Zig-Zag and Hilbert orders
        auto res_block = MoEngine::solveDistinctElements(arr, queries, false);
        auto res_hilbert = MoEngine::solveDistinctElements(arr, queries, true);
        auto naive_distinct = MoEngine::naiveDistinctQueries(arr, queries);

        assert(res_block == naive_distinct);
        assert(res_hilbert == naive_distinct);

        // 2. Verify Powerful Array query
        auto res_power = MoEngine::solvePowerfulArray(arr, queries);
        auto naive_power = MoEngine::naivePowerfulQueries(arr, queries);
        assert(res_power == naive_power);
    }
}

int main() {
    std::cout << "[Verification] Running Mo's Algorithm test suite...\n";
    run_unit_tests();
    run_differential_stress_tests();
    std::cout << "[Verification] All Mo's Algorithm differential tests passed successfully!\n";
    return 0;
}
