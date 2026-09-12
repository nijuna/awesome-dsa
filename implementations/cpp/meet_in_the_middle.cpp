#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <queue>

namespace dsa {

/**
 * @brief Helper to generate all subset sums of a given array slice.
 */
inline std::vector<int64_t> generate_subset_sums(const std::vector<int64_t>& arr) {
    size_t n = arr.size();
    size_t total_subsets = 1ULL << n;
    std::vector<int64_t> sums;
    sums.reserve(total_subsets);

    for (size_t mask = 0; mask < total_subsets; ++mask) {
        int64_t current_sum = 0;
        for (size_t i = 0; i < n; ++i) {
            if (mask & (1ULL << i)) {
                current_sum += arr[i];
            }
        }
        sums.push_back(current_sum);
    }
    return sums;
}

/**
 * @brief Meet-in-the-middle Subset Sum (Exact match).
 * Complexity: O(2^(n/2) * (n/2)) to generate, O(2^(n/2) log(2^(n/2))) to sort and search.
 */
inline bool subset_sum_exact(const std::vector<int64_t>& nums, int64_t target) {
    size_t n = nums.size();
    if (n == 0) return target == 0;

    size_t mid = n / 2;
    std::vector<int64_t> left(nums.begin(), nums.begin() + mid);
    std::vector<int64_t> right(nums.begin() + mid, nums.end());

    std::vector<int64_t> left_sums = generate_subset_sums(left);
    std::vector<int64_t> right_sums = generate_subset_sums(right);

    std::sort(right_sums.begin(), right_sums.end());

    for (int64_t s_left : left_sums) {
        int64_t needed = target - s_left;
        if (std::binary_search(right_sums.begin(), right_sums.end(), needed)) {
            return true;
        }
    }
    return false;
}

/**
 * @brief Meet-in-the-middle Maximum Subset Sum <= W (Knapsack capacity variant).
 * Returns the largest subset sum that does not exceed capacity W.
 */
inline int64_t max_subset_sum_le(const std::vector<int64_t>& nums, int64_t W) {
    size_t n = nums.size();
    if (n == 0) return 0;

    size_t mid = n / 2;
    std::vector<int64_t> left(nums.begin(), nums.begin() + mid);
    std::vector<int64_t> right(nums.begin() + mid, nums.end());

    std::vector<int64_t> left_sums = generate_subset_sums(left);
    std::vector<int64_t> right_sums = generate_subset_sums(right);

    std::sort(right_sums.begin(), right_sums.end());

    int64_t best = 0;
    for (int64_t s_left : left_sums) {
        if (s_left > W) continue;
        int64_t rem = W - s_left;
        // Find largest right sum <= rem
        auto it = std::upper_bound(right_sums.begin(), right_sums.end(), rem);
        if (it != right_sums.begin()) {
            --it;
            int64_t total = s_left + *it;
            if (total > best) {
                best = total;
            }
        }
    }
    return best;
}

/**
 * @brief 4-Sum Quadruplets Count: A + B + C + D == target.
 * Given 4 arrays, returns count of tuples (i, j, k, l) such that A[i] + B[j] + C[k] + D[l] == target.
 * Reduces complexity from O(n^4) to O(n^2).
 */
inline int64_t four_sum_count(const std::vector<int>& A, const std::vector<int>& B,
                              const std::vector<int>& C, const std::vector<int>& D,
                              int target = 0) {
    std::unordered_map<int, int64_t> ab_sums;
    for (int a : A) {
        for (int b : B) {
            ab_sums[a + b]++;
        }
    }

    int64_t count = 0;
    for (int c : C) {
        for (int d : D) {
            int needed = target - (c + d);
            auto it = ab_sums.find(needed);
            if (it != ab_sums.end()) {
                count += it->second;
            }
        }
    }
    return count;
}

/**
 * @brief Bidirectional Breadth-First Search on an unweighted undirected graph.
 * Returns shortest path distance between start and target, or -1 if unreachable.
 */
inline int bidirectional_bfs(int n, const std::vector<std::vector<int>>& adj, int start, int target) {
    if (start == target) return 0;

    std::vector<int> dist_start(n, -1);
    std::vector<int> dist_target(n, -1);

    std::queue<int> q_start, q_target;

    q_start.push(start);
    dist_start[start] = 0;

    q_target.push(target);
    dist_target[target] = 0;

    while (!q_start.empty() && !q_target.empty()) {
        // Expand smaller frontier to keep branching balanced
        if (q_start.size() <= q_target.size()) {
            int u = q_start.front();
            q_start.pop();

            for (int v : adj[u]) {
                if (dist_target[v] != -1) {
                    return dist_start[u] + 1 + dist_target[v];
                }
                if (dist_start[v] == -1) {
                    dist_start[v] = dist_start[u] + 1;
                    q_start.push(v);
                }
            }
        } else {
            int u = q_target.front();
            q_target.pop();

            for (int v : adj[u]) {
                if (dist_start[v] != -1) {
                    return dist_target[u] + 1 + dist_start[v];
                }
                if (dist_target[v] == -1) {
                    dist_target[v] = dist_target[u] + 1;
                    q_target.push(v);
                }
            }
        }
    }
    return -1;
}

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Meet-in-the-Middle C++17 Verification..." << std::endl;

    // 1. Subset Sum Exact
    {
        std::vector<int64_t> nums = {3, 34, 4, 12, 5, 2};
        assert(subset_sum_exact(nums, 9));   // 4 + 5 = 9
        assert(subset_sum_exact(nums, 14));  // 5 + 4 + 3 + 2 = 14
        assert(!subset_sum_exact(nums, 30));
        assert(subset_sum_exact(nums, 0));   // empty set = 0
    }

    // 2. Subset Sum Max <= W
    {
        std::vector<int64_t> nums = {10, 20, 30, 40, 50};
        assert(max_subset_sum_le(nums, 65) == 60);  // 10 + 20 + 30 or 20 + 40
        assert(max_subset_sum_le(nums, 100) == 100); // 10 + 40 + 50
        assert(max_subset_sum_le(nums, 5) == 0);
    }

    // 3. 4-Sum Count
    {
        std::vector<int> A = { 1,  2};
        std::vector<int> B = {-2, -1};
        std::vector<int> C = {-1,  2};
        std::vector<int> D = { 0,  2};
        // Target = 0
        // (0, 0, 0, 0) -> 1 + (-2) + (-1) + 2 = 0
        // (1, 1, 0, 0) -> 2 + (-1) + (-1) + 0 = 0
        int64_t cnt = four_sum_count(A, B, C, D, 0);
        assert(cnt == 2);
    }

    // 4. Bidirectional BFS
    {
        // Graph: 0 - 1 - 2 - 3 - 4
        int n = 5;
        std::vector<std::vector<int>> adj(n);
        adj[0] = {1};
        adj[1] = {0, 2};
        adj[2] = {1, 3};
        adj[3] = {2, 4};
        adj[4] = {3};

        assert(bidirectional_bfs(n, adj, 0, 4) == 4);
        assert(bidirectional_bfs(n, adj, 1, 3) == 2);
        assert(bidirectional_bfs(n, adj, 2, 2) == 0);

        // Disconnected graph
        std::vector<std::vector<int>> disc_adj(6);
        disc_adj[0] = {1}; disc_adj[1] = {0};
        disc_adj[2] = {3}; disc_adj[3] = {2};
        assert(bidirectional_bfs(6, disc_adj, 0, 3) == -1);
    }

    std::cout << "[PASSED] Meet-in-the-Middle C++17 All Tests Passed!" << std::endl;
    return 0;
}
