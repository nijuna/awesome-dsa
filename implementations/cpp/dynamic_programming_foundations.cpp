/**
 * Reference Implementation: 1D and 2D Dynamic Programming Foundations
 * Demonstrates:
 * 1. 1D State Transitions: Fibonacci (Memoization, Tabulation, O(1) Space Optimization).
 * 2. 1D Combinatorial Counting: Climbing Stairs (Prefix Aggregation).
 * 3. 2D Grid Path Counting: Overlapping Subproblems in Two Dimensions.
 * 4. 2D Grid Cost Optimization: Minimum Path Sum on Weighted Lattices.
 * 5. Optimal Solution Reconstruction: Backtracking via Parent Pointers.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <cstdint>
#include <limits>

namespace dp_foundations {

// ============================================================================
// 1. 1D DP: Fibonacci Sequences
// ============================================================================

/**
 * Top-Down Memoization: O(N) Time, O(N) Auxiliary Space & Stack Depth.
 */
long long fibonacci_memo(int n) {
    if (n < 0) {
        throw std::invalid_argument("Fibonacci index must be non-negative");
    }
    if (n <= 1) return n;

    std::vector<long long> memo(n + 1, -1);
    memo[0] = 0;
    memo[1] = 1;

    auto solve = [&](auto&& self, int x) -> long long {
        if (memo[x] != -1) return memo[x];
        return memo[x] = self(self, x - 1) + self(self, x - 2);
    };

    return solve(solve, n);
}

/**
 * Bottom-Up Tabulation: O(N) Time, O(N) Table Space.
 */
long long fibonacci_tab(int n) {
    if (n < 0) {
        throw std::invalid_argument("Fibonacci index must be non-negative");
    }
    if (n <= 1) return n;

    std::vector<long long> dp(n + 1, 0);
    dp[0] = 0;
    dp[1] = 1;

    for (int i = 2; i <= n; ++i) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }

    return dp[n];
}

/**
 * Space-Optimized Tabulation: O(N) Time, O(1) Space.
 */
long long fibonacci_optimized(int n) {
    if (n < 0) {
        throw std::invalid_argument("Fibonacci index must be non-negative");
    }
    if (n <= 1) return n;

    long long prev2 = 0;
    long long prev1 = 1;

    for (int i = 2; i <= n; ++i) {
        long long cur = prev1 + prev2;
        prev2 = prev1;
        prev1 = cur;
    }

    return prev1;
}

// ============================================================================
// 2. 1D Combinatorial Counting: Climbing Stairs
// ============================================================================

/**
 * Computes ways to reach step n taking 1 or 2 steps: O(N) Time, O(1) Space.
 */
long long climbing_stairs(int n) {
    if (n < 0) {
        throw std::invalid_argument("Step count must be non-negative");
    }
    if (n <= 1) return 1;

    long long prev2 = 1; // ways(0)
    long long prev1 = 1; // ways(1)

    for (int i = 2; i <= n; ++i) {
        long long cur = prev1 + prev2;
        prev2 = prev1;
        prev1 = cur;
    }

    return prev1;
}

// ============================================================================
// 3. 2D Grid Path Counting
// ============================================================================

/**
 * Computes unique paths from (0,0) to (rows-1, cols-1) moving only Right or Down.
 * Time: O(rows * cols), Space: O(rows * cols).
 */
long long count_grid_paths(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return 0;

    std::vector<std::vector<long long>> dp(rows, std::vector<long long>(cols, 0));
    dp[0][0] = 1;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (i > 0) dp[i][j] += dp[i - 1][j];
            if (j > 0) dp[i][j] += dp[i][j - 1];
        }
    }

    return dp[rows - 1][cols - 1];
}

/**
 * Space-Optimized 2D Path Counting using single-row buffer: O(cols) Space.
 */
long long count_grid_paths_optimized(int rows, int cols) {
    if (rows <= 0 || cols <= 0) return 0;

    std::vector<long long> dp(cols, 1);

    for (int i = 1; i < rows; ++i) {
        for (int j = 1; j < cols; ++j) {
            dp[j] += dp[j - 1];
        }
    }

    return dp[cols - 1];
}

// ============================================================================
// 4. 2D Grid Optimization: Minimum Path Sum
// ============================================================================

/**
 * Computes minimum cost from (0,0) to (rows-1, cols-1) moving Right or Down.
 * Time: O(rows * cols), Space: O(rows * cols).
 */
long long min_path_sum(const std::vector<std::vector<int>>& cost) {
    if (cost.empty() || cost[0].empty()) return 0;

    int rows = static_cast<int>(cost.size());
    int cols = static_cast<int>(cost[0].size());

    constexpr long long INF = std::numeric_limits<long long>::max() / 2;
    std::vector<std::vector<long long>> dp(rows, std::vector<long long>(cols, INF));
    dp[0][0] = cost[0][0];

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (i == 0 && j == 0) continue;

            long long best = INF;
            if (i > 0) best = std::min(best, dp[i - 1][j]);
            if (j > 0) best = std::min(best, dp[i][j - 1]);
            dp[i][j] = best + cost[i][j];
        }
    }

    return dp[rows - 1][cols - 1];
}

// ============================================================================
// 5. Solution Path Reconstruction (Backtracking via Parent Links)
// ============================================================================

/**
 * Reconstructs the exact sequence of grid coordinates forming the minimum path.
 * Returns vector of (row, col) pairs from (0,0) to (rows-1, cols-1).
 */
std::vector<std::pair<int, int>> reconstruct_min_path(
    const std::vector<std::vector<int>>& cost) {
    if (cost.empty() || cost[0].empty()) return {};

    int rows = static_cast<int>(cost.size());
    int cols = static_cast<int>(cost[0].size());

    constexpr long long INF = std::numeric_limits<long long>::max() / 2;
    std::vector<std::vector<long long>> dp(rows, std::vector<long long>(cols, INF));
    std::vector<std::vector<std::pair<int, int>>> parent(
        rows, std::vector<std::pair<int, int>>(cols, {-1, -1}));

    dp[0][0] = cost[0][0];

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (i == 0 && j == 0) continue;

            long long from_up = (i > 0 ? dp[i - 1][j] : INF);
            long long from_left = (j > 0 ? dp[i][j - 1] : INF);

            if (from_up <= from_left) {
                dp[i][j] = from_up + cost[i][j];
                parent[i][j] = {i - 1, j};
            } else {
                dp[i][j] = from_left + cost[i][j];
                parent[i][j] = {i, j - 1};
            }
        }
    }

    std::vector<std::pair<int, int>> path;
    int cur_r = rows - 1;
    int cur_c = cols - 1;

    while (cur_r != -1 && cur_c != -1) {
        path.push_back({cur_r, cur_c});
        auto [pr, pc] = parent[cur_r][cur_c];
        cur_r = pr;
        cur_c = pc;
    }

    std::reverse(path.begin(), path.end());
    return path;
}

} // namespace dp_foundations

// ============================================================================
// Comprehensive Unit Verification
// ============================================================================

int main() {
    using namespace dp_foundations;

    // 1. Fibonacci Verification across all approaches
    const std::vector<int> fib_indices = {0, 1, 2, 3, 4, 5, 10, 20, 30, 45};
    const std::vector<long long> expected_fib = {
        0LL, 1LL, 1LL, 2LL, 3LL, 5LL, 55LL, 6765LL, 832040LL, 1134903170LL
    };

    for (size_t i = 0; i < fib_indices.size(); ++i) {
        int n = fib_indices[i];
        long long expected = expected_fib[i];
        assert(fibonacci_memo(n) == expected);
        assert(fibonacci_tab(n) == expected);
        assert(fibonacci_optimized(n) == expected);
    }

    // Negative Fibonacci check
    try {
        fibonacci_memo(-1);
        assert(false);
    } catch (const std::invalid_argument&) {}

    // 2. Climbing Stairs Verification
    assert(climbing_stairs(0) == 1);
    assert(climbing_stairs(1) == 1);
    assert(climbing_stairs(2) == 2);
    assert(climbing_stairs(3) == 3);
    assert(climbing_stairs(4) == 5);
    assert(climbing_stairs(5) == 8);

    // 3. Grid Path Counting
    assert(count_grid_paths(1, 1) == 1);
    assert(count_grid_paths(1, 5) == 1);
    assert(count_grid_paths(5, 1) == 1);
    assert(count_grid_paths(3, 3) == 6);
    assert(count_grid_paths(3, 7) == 28);
    assert(count_grid_paths(5, 5) == 70);

    assert(count_grid_paths_optimized(3, 3) == 6);
    assert(count_grid_paths_optimized(3, 7) == 28);
    assert(count_grid_paths_optimized(5, 5) == 70);

    // Degenerate grid
    assert(count_grid_paths(0, 5) == 0);
    assert(count_grid_paths_optimized(5, 0) == 0);

    // 4. Minimum Path Sum & Path Reconstruction
    std::vector<std::vector<int>> grid1 = {
        {1, 3, 1},
        {1, 5, 1},
        {4, 2, 1}
    };
    // Optimal path: 1 -> 3 -> 1 -> 1 -> 1 = 7
    long long min_cost1 = min_path_sum(grid1);
    assert(min_cost1 == 7);

    auto path1 = reconstruct_min_path(grid1);
    std::vector<std::pair<int, int>> expected_path1 = {
        {0, 0}, {0, 1}, {0, 2}, {1, 2}, {2, 2}
    };
    assert(path1 == expected_path1);

    // Verify cost sum along reconstructed path
    long long verified_sum = 0;
    for (const auto& [r, c] : path1) {
        verified_sum += grid1[r][c];
    }
    assert(verified_sum == min_cost1);

    // Single cell grid
    std::vector<std::vector<int>> single_cell = {{42}};
    assert(min_path_sum(single_cell) == 42);
    auto single_path = reconstruct_min_path(single_cell);
    assert(single_path.size() == 1);
    assert(single_path[0] == std::make_pair(0, 0));

    // Single row grid
    std::vector<std::vector<int>> single_row = {{2, 4, 1, 3}};
    assert(min_path_sum(single_row) == 10);
    auto row_path = reconstruct_min_path(single_row);
    assert(row_path.size() == 4);

    // Single column grid
    std::vector<std::vector<int>> single_col = {{2}, {4}, {1}, {3}};
    assert(min_path_sum(single_col) == 10);
    auto col_path = reconstruct_min_path(single_col);
    assert(col_path.size() == 4);

    std::cout << "[PASS] All Dynamic Programming Foundations C++ unit tests passed." << std::endl;
    return 0;
}
