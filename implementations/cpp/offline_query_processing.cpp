/**
 * @file offline_query_processing.cpp
 * @brief Offline Query Processing Paradigms: CDQ Divide-and-Conquer for 3D Partial Orders
 *        and Offline Sweep-Line with Fenwick Tree for Range Queries.
 *
 * Implements Arthur's Two-Layer API:
 *   - Layer 1: Fenwick tree (BIT) with rollback, in-place dimension merging,
 *              3D partial order dominance counting, right-endpoint sweep line.
 *   - Layer 2: Safe OfflineQueryEngine supporting:
 *              1. solve3DPartialOrder() [CDQ Divide-and-Conquer O(N log^2 N)]
 *              2. solveDistinctRangeQueries() [Offline Sweep Line O((N + Q) log N)]
 *              3. naive3DPartialOrder() [O(N^2) differential oracle]
 *              4. naiveDistinctQueries() [O(Q * N) differential oracle]
 *
 * Compilation:
 *   g++ -std=c++17 -O3 -Wall -Wextra -Werror offline_query_processing.cpp -o offline_query
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdint>
#include <cassert>
#include <unordered_map>
#include <unordered_set>
#include <random>

namespace offline {

// ============================================================================
// Layer 1: Fenwick Tree (BIT) with Fast Rollback & Primitives
// ============================================================================

class RollbackFenwickTree {
private:
    int size_{0};
    std::vector<int> tree_;

public:
    RollbackFenwickTree() = default;
    explicit RollbackFenwickTree(int n) : size_(n), tree_(n + 1, 0) {}

    void add(int idx, int delta) {
        for (; idx <= size_; idx += idx & -idx) {
            tree_[idx] += delta;
        }
    }

    int query(int idx) const {
        int sum = 0;
        for (; idx > 0; idx -= idx & -idx) {
            sum += tree_[idx];
        }
        return sum;
    }

    int queryRange(int l, int r) const {
        if (l > r) return 0;
        return query(r) - query(l - 1);
    }
};

struct Element3D {
    int id;
    int a;
    int b;
    int c;
    int count{1};
    int ans{0};

    bool operator==(Element3D const& o) const {
        return a == o.a && b == o.b && c == o.c;
    }
};

// ============================================================================
// Layer 2: High-Level Offline Query Engine
// ============================================================================

class OfflineQueryEngine {
private:
    static void cdqRec(std::vector<Element3D>& elems, std::vector<Element3D>& temp,
                       int left, int right, int max_c, RollbackFenwickTree& bit) {
        if (left >= right) return;

        int mid = left + (right - left) / 2;
        cdqRec(elems, temp, left, mid, max_c, bit);
        cdqRec(elems, temp, mid + 1, right, max_c, bit);

        // Merge two halves sorted by b, while querying c in Fenwick tree
        int i = left;
        int j = mid + 1;
        int k = left;

        while (i <= mid && j <= right) {
            if (elems[i].b <= elems[j].b) {
                bit.add(elems[i].c, elems[i].count);
                temp[k++] = elems[i++];
            } else {
                elems[j].ans += bit.query(elems[j].c);
                temp[k++] = elems[j++];
            }
        }

        while (j <= right) {
            elems[j].ans += bit.query(elems[j].c);
            temp[k++] = elems[j++];
        }

        // Roll back Fenwick tree modifications made by left half
        for (int p = left; p < i; ++p) {
            bit.add(elems[p].c, -elems[p].count);
        }

        while (i <= mid) {
            temp[k++] = elems[i++];
        }

        for (int p = left; p <= right; ++p) {
            elems[p] = temp[p];
        }
    }

public:
    /**
     * @brief Solves 3D Partial Order (Dominance Counting) via CDQ Divide-and-Conquer in O(N log^2 N).
     * For each element i, counts elements j (j != i) such that a_j <= a_i, b_j <= b_i, and c_j <= c_i.
     */
    static std::vector<int> solve3DPartialOrder(std::vector<Element3D> points) {
        int n = static_cast<int>(points.size());
        if (n == 0) return {};

        // Sort by (a, b, c) primarily
        std::sort(points.begin(), points.end(), [](Element3D const& x, Element3D const& y) {
            if (x.a != y.a) return x.a < y.a;
            if (x.b != y.b) return x.b < y.b;
            return x.c < y.c;
        });

        // Deduplicate identical points and sum frequencies
        std::vector<Element3D> unique_pts;
        unique_pts.reserve(n);
        for (int i = 0; i < n; ++i) {
            if (!unique_pts.empty() && points[i] == unique_pts.back()) {
                unique_pts.back().count++;
            } else {
                unique_pts.push_back(points[i]);
            }
        }

        // Coordinate compression on dimension c
        std::vector<int> c_vals;
        c_vals.reserve(unique_pts.size());
        for (auto const& p : unique_pts) c_vals.push_back(p.c);
        std::sort(c_vals.begin(), c_vals.end());
        c_vals.erase(std::unique(c_vals.begin(), c_vals.end()), c_vals.end());

        for (auto& p : unique_pts) {
            p.c = static_cast<int>(std::lower_bound(c_vals.begin(), c_vals.end(), p.c) - c_vals.begin()) + 1;
        }

        int max_c = static_cast<int>(c_vals.size());
        RollbackFenwickTree bit(max_c + 2);
        std::vector<Element3D> temp(unique_pts.size());

        cdqRec(unique_pts, temp, 0, static_cast<int>(unique_pts.size()) - 1, max_c, bit);

        // Account for identical points: identical copies dominate each other
        std::vector<int> results(n, 0);
        for (auto const& p : unique_pts) {
            // Self + strictly smaller: (p.ans + p.count - 1)
            int dominance = p.ans + (p.count - 1);
            // Assign to original matching points
            for (int i = 0; i < n; ++i) {
                if (points[i].a == p.a && points[i].b == p.b &&
                    c_vals[p.c - 1] == points[i].c) {
                    results[points[i].id] = dominance;
                }
            }
        }

        return results;
    }

    /**
     * @brief Solves Distinct Elements Range Query (DQUERY) via Offline Sweep-Line in O((N + Q) log N).
     */
    static std::vector<int> solveDistinctRangeQueries(std::vector<int> const& arr,
                                                      std::vector<std::pair<int, int>> const& raw_queries) {
        int n = static_cast<int>(arr.size());
        int q_count = static_cast<int>(raw_queries.size());
        if (q_count == 0) return {};

        struct OfflineQ {
            int id;
            int l;
            int r;
        };

        std::vector<std::vector<OfflineQ>> queries_at_r(n);
        for (int i = 0; i < q_count; ++i) {
            queries_at_r[raw_queries[i].second].push_back({i, raw_queries[i].first, raw_queries[i].second});
        }

        RollbackFenwickTree bit(n + 1);
        std::unordered_map<int, int> last_pos;
        std::vector<int> answers(q_count);

        for (int r = 0; r < n; ++r) {
            int val = arr[r];
            if (last_pos.find(val) != last_pos.end()) {
                bit.add(last_pos[val] + 1, -1); // 1-based indexing for BIT
            }
            last_pos[val] = r;
            bit.add(r + 1, 1);

            for (auto const& q : queries_at_r[r]) {
                answers[q.id] = bit.queryRange(q.l + 1, q.r + 1);
            }
        }

        return answers;
    }

    // ========================================================================
    // Differential Verification Oracles
    // ========================================================================

    static std::vector<int> naive3DPartialOrder(std::vector<Element3D> const& points) {
        size_t n = points.size();
        std::vector<int> results(n, 0);
        for (size_t i = 0; i < n; ++i) {
            int count = 0;
            for (size_t j = 0; j < n; ++j) {
                if (i == j) continue;
                if (points[j].a <= points[i].a &&
                    points[j].b <= points[i].b &&
                    points[j].c <= points[i].c) {
                    count++;
                }
            }
            results[points[i].id] = count;
        }
        return results;
    }

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
};

} // namespace offline

// ============================================================================
// Verification Suite
// ============================================================================

void run_unit_tests() {
    using namespace offline;

    // Test 1: 3D Partial Order
    // Points: (1, 1, 1), (2, 2, 2), (3, 3, 3), (1, 2, 3)
    std::vector<Element3D> pts = {
        {0, 1, 1, 1, 1, 0},
        {1, 2, 2, 2, 1, 0},
        {2, 3, 3, 3, 1, 0},
        {3, 1, 2, 3, 1, 0}
    };

    auto cdq_res = OfflineQueryEngine::solve3DPartialOrder(pts);
    auto naive_cdq = OfflineQueryEngine::naive3DPartialOrder(pts);
    assert(cdq_res == naive_cdq);
    // Point 0 dominated by 0 others: 0
    // Point 1 dominated by Point 0: 1
    // Point 2 dominated by Points 0, 1, 3: 3
    // Point 3 dominated by Point 0: 1
    std::vector<int> expected_cdq = {0, 1, 3, 1};
    assert(cdq_res == expected_cdq);

    // Test 2: Offline Distinct Range Queries (DQUERY)
    std::vector<int> arr = {1, 1, 2, 1, 3};
    std::vector<std::pair<int, int>> queries = {
        {0, 4}, {1, 3}, {2, 4}, {0, 1}
    };
    auto distinct_res = OfflineQueryEngine::solveDistinctRangeQueries(arr, queries);
    std::vector<int> exp_distinct = {3, 2, 3, 1};
    assert(distinct_res == exp_distinct);
}

void run_differential_stress_tests() {
    using namespace offline;
    std::mt19937 rng(42);

    for (int trial = 0; trial < 40; ++trial) {
        int n = 50;
        std::vector<Element3D> pts;
        pts.reserve(n);
        for (int i = 0; i < n; ++i) {
            pts.push_back({i, static_cast<int>(rng() % 30),
                              static_cast<int>(rng() % 30),
                              static_cast<int>(rng() % 30), 1, 0});
        }

        auto cdq = OfflineQueryEngine::solve3DPartialOrder(pts);
        auto naive_cdq = OfflineQueryEngine::naive3DPartialOrder(pts);
        assert(cdq == naive_cdq);
    }

    for (int trial = 0; trial < 40; ++trial) {
        int n = 70;
        int q = 50;
        std::vector<int> arr(n);
        for (int i = 0; i < n; ++i) arr[i] = (rng() % 25) + 1;

        std::vector<std::pair<int, int>> queries;
        for (int i = 0; i < q; ++i) {
            int l = rng() % n;
            int r = rng() % n;
            if (l > r) std::swap(l, r);
            queries.emplace_back(l, r);
        }

        auto sweep_res = OfflineQueryEngine::solveDistinctRangeQueries(arr, queries);
        auto naive_res = OfflineQueryEngine::naiveDistinctQueries(arr, queries);
        assert(sweep_res == naive_res);
    }
}

int main() {
    std::cout << "[Verification] Running Offline Query Processing test suite...\n";
    run_unit_tests();
    run_differential_stress_tests();
    std::cout << "[Verification] All Offline Query Processing differential tests passed successfully!\n";
    return 0;
}
