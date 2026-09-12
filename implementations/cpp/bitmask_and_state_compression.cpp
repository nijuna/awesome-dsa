/**
 * Reference Implementation: Bitmask and State Compression Dynamic Programming
 * Demonstrates:
 * 1. Held-Karp Travelling Salesperson Problem (TSP): O(n^2 2^n) time, O(n 2^n) space with tour reconstruction.
 * 2. Assignment / Bipartite Matching DP: O(n 2^n) time, O(2^n) space with assignment permutation reconstruction.
 * 3. Submask Iteration: Enumerating all submasks in O(3^n) for subset partitioning / bin packing.
 * 4. Sum Over Subsets (SOS DP): Multidimensional prefix sums on the hypercube in O(n 2^n) time.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <limits>
#include <algorithm>
#include <cassert>

namespace bitmask_dp {

// ============================================================================
// 1. Held-Karp Travelling Salesperson Problem (TSP)
// ============================================================================

/**
 * Held-Karp TSP: Computes the minimum cost Hamiltonian cycle starting and ending at node 0.
 * Time: O(n^2 2^n), Space: O(n 2^n).
 */
long long tsp_held_karp(const std::vector<std::vector<int>>& dist) {
    int n = static_cast<int>(dist.size());
    if (n <= 1) return 0;

    int full = 1 << n;
    const long long INF = std::numeric_limits<long long>::max() / 4;

    std::vector<std::vector<long long>> dp(full, std::vector<long long>(n, INF));
    dp[1][0] = 0; // Start at city 0 with mask containing only city 0 (bit 0 set)

    for (int mask = 1; mask < full; ++mask) {
        for (int u = 0; u < n; ++u) {
            if (!(mask & (1 << u))) continue;
            if (dp[mask][u] == INF) continue;

            for (int v = 0; v < n; ++v) {
                if (mask & (1 << v)) continue;
                int next_mask = mask | (1 << v);
                dp[next_mask][v] = std::min(dp[next_mask][v], dp[mask][u] + dist[u][v]);
            }
        }
    }

    long long ans = INF;
    int all_mask = full - 1;
    for (int u = 1; u < n; ++u) {
        if (dp[all_mask][u] != INF) {
            ans = std::min(ans, dp[all_mask][u] + dist[u][0]);
        }
    }

    return ans;
}

/**
 * Held-Karp TSP with full tour reconstruction.
 * Returns {min_cost, tour_sequence}.
 */
std::pair<long long, std::vector<int>> tsp_held_karp_reconstruct(const std::vector<std::vector<int>>& dist) {
    int n = static_cast<int>(dist.size());
    if (n <= 0) return {0, {}};
    if (n == 1) return {0, {0, 0}};

    int full = 1 << n;
    const long long INF = std::numeric_limits<long long>::max() / 4;

    std::vector<std::vector<long long>> dp(full, std::vector<long long>(n, INF));
    std::vector<std::vector<int>> parent(full, std::vector<int>(n, -1));
    dp[1][0] = 0;

    for (int mask = 1; mask < full; ++mask) {
        for (int u = 0; u < n; ++u) {
            if (!(mask & (1 << u))) continue;
            if (dp[mask][u] == INF) continue;

            for (int v = 0; v < n; ++v) {
                if (mask & (1 << v)) continue;
                int next_mask = mask | (1 << v);
                long long cand = dp[mask][u] + dist[u][v];
                if (cand < dp[next_mask][v]) {
                    dp[next_mask][v] = cand;
                    parent[next_mask][v] = u;
                }
            }
        }
    }

    int all_mask = full - 1;
    long long best_cost = INF;
    int last = -1;

    for (int u = 1; u < n; ++u) {
        if (dp[all_mask][u] != INF) {
            long long cand = dp[all_mask][u] + dist[u][0];
            if (cand < best_cost) {
                best_cost = cand;
                last = u;
            }
        }
    }

    // Backtrack route
    std::vector<int> route;
    int cur = last;
    int mask = all_mask;

    while (cur != -1) {
        route.push_back(cur);
        int prev = parent[mask][cur];
        mask ^= (1 << cur);
        cur = prev;
    }

    std::reverse(route.begin(), route.end());
    route.push_back(0); // Close the cycle
    return {best_cost, route};
}

// ============================================================================
// 2. Assignment / Perfect Matching DP
// ============================================================================

/**
 * Assignment Problem: Finds minimum cost to assign n workers to n tasks.
 * worker k = popcount(mask) is assigned to task j not in mask.
 * Time: O(n 2^n), Space: O(2^n).
 */
long long assignment_dp(const std::vector<std::vector<int>>& cost) {
    int n = static_cast<int>(cost.size());
    if (n == 0) return 0;

    int full = 1 << n;
    const long long INF = std::numeric_limits<long long>::max() / 4;

    std::vector<long long> dp(full, INF);
    dp[0] = 0;

    for (int mask = 0; mask < full; ++mask) {
        int worker = __builtin_popcount(mask);
        if (worker >= n) continue;

        for (int task = 0; task < n; ++task) {
            if (mask & (1 << task)) continue;
            int next_mask = mask | (1 << task);
            dp[next_mask] = std::min(dp[next_mask], dp[mask] + cost[worker][task]);
        }
    }

    return dp[full - 1];
}

/**
 * Reconstructs the optimal task assignment vector, where assignment[i] is the task assigned to worker i.
 */
std::pair<long long, std::vector<int>> assignment_dp_reconstruct(const std::vector<std::vector<int>>& cost) {
    int n = static_cast<int>(cost.size());
    if (n == 0) return {0, {}};

    int full = 1 << n;
    const long long INF = std::numeric_limits<long long>::max() / 4;

    std::vector<long long> dp(full, INF);
    std::vector<int> parent_mask(full, -1);
    std::vector<int> chosen_task(full, -1);
    dp[0] = 0;

    for (int mask = 0; mask < full; ++mask) {
        int worker = __builtin_popcount(mask);
        if (worker >= n) continue;

        for (int task = 0; task < n; ++task) {
            if (mask & (1 << task)) continue;
            int next_mask = mask | (1 << task);
            long long cand = dp[mask] + cost[worker][task];
            if (cand < dp[next_mask]) {
                dp[next_mask] = cand;
                parent_mask[next_mask] = mask;
                chosen_task[next_mask] = task;
            }
        }
    }

    std::vector<int> assignment(n, -1);
    int mask = full - 1;
    while (mask > 0) {
        int pm = parent_mask[mask];
        int task = chosen_task[mask];
        int worker = __builtin_popcount(pm);
        assignment[worker] = task;
        mask = pm;
    }

    return {dp[full - 1], assignment};
}

// ============================================================================
// 3. Submask Iteration: Partitioning into Valid Subsets
// ============================================================================

/**
 * Computes minimum number of valid subsets needed to partition a set of elements.
 * is_valid_subset[mask] indicates whether the subset mask forms a valid team / clique / bin.
 * Iterates submasks in O(3^n) total time.
 */
int min_subsets_partition(int n, const std::vector<bool>& is_valid_subset) {
    int full = 1 << n;
    const int INF = 1e9;
    std::vector<int> dp(full, INF);
    dp[0] = 0;

    for (int mask = 1; mask < full; ++mask) {
        // Enumerate all nonempty submasks of mask
        for (int sub = mask; sub > 0; sub = (sub - 1) & mask) {
            if (is_valid_subset[sub] && dp[mask ^ sub] != INF) {
                dp[mask] = std::min(dp[mask], dp[mask ^ sub] + 1);
            }
        }
    }

    return dp[full - 1] == INF ? -1 : dp[full - 1];
}

// ============================================================================
// 4. Sum Over Subsets (SOS DP)
// ============================================================================

/**
 * SOS DP: Computes F[mask] = sum_{sub \subseteq mask} A[sub] in O(n 2^n) time.
 * Operates as multidimensional prefix sums over the n-dimensional Boolean hypercube.
 */
std::vector<long long> sos_dp_sum(std::vector<long long> f, int n) {
    int full = 1 << n;

    for (int bit = 0; bit < n; ++bit) {
        for (int mask = 0; mask < full; ++mask) {
            if (mask & (1 << bit)) {
                f[mask] += f[mask ^ (1 << bit)];
            }
        }
    }

    return f;
}

} // namespace bitmask_dp

// ============================================================================
// Unit Tests
// ============================================================================

int main() {
    using namespace bitmask_dp;

    // Test 1: Held-Karp TSP on a 4-city graph
    // Distances:
    // 0 -> 1: 10, 0 -> 2: 15, 0 -> 3: 20
    // 1 -> 0: 10, 1 -> 2: 35, 1 -> 3: 25
    // 2 -> 0: 15, 2 -> 1: 35, 2 -> 3: 30
    // 3 -> 0: 20, 3 -> 1: 25, 3 -> 2: 30
    // Optimal cycle: 0 -> 1 -> 3 -> 2 -> 0: 10 + 25 + 30 + 15 = 80
    {
        std::vector<std::vector<int>> dist = {
            {0, 10, 15, 20},
            {10, 0, 35, 25},
            {15, 35, 0, 30},
            {20, 25, 30, 0}
        };

        long long cost = tsp_held_karp(dist);
        assert(cost == 80);

        auto [rec_cost, tour] = tsp_held_karp_reconstruct(dist);
        assert(rec_cost == 80);
        assert(tour.size() == 5);
        assert(tour.front() == 0 && tour.back() == 0);

        // Verify cycle cost manually
        long long check = 0;
        for (size_t i = 0; i + 1 < tour.size(); ++i) {
            check += dist[tour[i]][tour[i + 1]];
        }
        assert(check == 80);
    }

    // Test 2: Assignment Problem
    // 3 workers, 3 tasks
    // Worker 0: [9, 2, 7]
    // Worker 1: [6, 4, 3]
    // Worker 2: [5, 8, 1]
    // Optimal: W0 -> T1 (cost 2), W1 -> T0 (cost 6), W2 -> T2 (cost 1) => total = 9
    // (or W0 -> T1 (2), W1 -> T2 (3), W2 -> T0 (5) => total = 10)
    {
        std::vector<std::vector<int>> cost = {
            {9, 2, 7},
            {6, 4, 3},
            {5, 8, 1}
        };

        long long min_c = assignment_dp(cost);
        assert(min_c == 9);

        auto [rec_cost, assign] = assignment_dp_reconstruct(cost);
        assert(rec_cost == 9);
        assert(assign.size() == 3);
        assert(assign[0] == 1);
        assert(assign[1] == 0);
        assert(assign[2] == 2);
    }

    // Test 3: Submask Iteration for Subset Partitioning
    // 4 elements, valid subsets are individual elements or pair {0, 1} and pair {2, 3}
    {
        int n = 4;
        std::vector<bool> valid(1 << n, false);
        for (int i = 0; i < n; ++i) valid[1 << i] = true;
        valid[(1 << 0) | (1 << 1)] = true; // {0, 1}
        valid[(1 << 2) | (1 << 3)] = true; // {2, 3}

        int parts = min_subsets_partition(n, valid);
        assert(parts == 2); // {0, 1} and {2, 3}
    }

    // Test 4: Sum Over Subsets (SOS DP)
    // n = 3 (8 masks: 000 .. 111)
    // A[mask] = 1 for all mask
    // F[mask] should equal 2^(popcount(mask))
    {
        int n = 3;
        std::vector<long long> a(1 << n, 1);
        auto f = sos_dp_sum(a, n);

        for (int mask = 0; mask < (1 << n); ++mask) {
            int pc = __builtin_popcount(mask);
            long long expected = 1LL << pc;
            assert(f[mask] == expected);
        }

        // Test with custom values:
        // A = [1, 2, 4, 8] for n=2
        // F[0] = A[0] = 1
        // F[1] = A[0]+A[1] = 3
        // F[2] = A[0]+A[2] = 5
        // F[3] = A[0]+A[1]+A[2]+A[3] = 15
        std::vector<long long> a2 = {1, 2, 4, 8};
        auto f2 = sos_dp_sum(a2, 2);
        assert(f2[0] == 1);
        assert(f2[1] == 3);
        assert(f2[2] == 5);
        assert(f2[3] == 15);
    }

    // Test 5: Edge cases (n = 1)
    {
        std::vector<std::vector<int>> dist1 = {{0}};
        assert(tsp_held_karp(dist1) == 0);

        std::vector<std::vector<int>> cost1 = {{42}};
        assert(assignment_dp(cost1) == 42);
        auto [c1, a1] = assignment_dp_reconstruct(cost1);
        assert(c1 == 42 && a1.size() == 1 && a1[0] == 0);
    }

    std::cout << "[PASS] All Bitmask and State Compression C++ unit tests passed." << std::endl;
    return 0;
}
