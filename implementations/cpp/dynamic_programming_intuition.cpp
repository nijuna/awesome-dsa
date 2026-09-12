#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <algorithm>

namespace dsa {

/**
 * @brief Archetype 1: Fibonacci Sequence.
 * Demonstrates Naive Recursion vs Memoization vs Tabulation vs Space Optimization.
 */
class FibonacciDemo {
public:
    // 1. Naive Recursion: O(2^N) time, O(N) stack space
    static uint64_t naive_recursive(int n) {
        if (n <= 1) return static_cast<uint64_t>(n);
        return naive_recursive(n - 1) + naive_recursive(n - 2);
    }

    // 2. Top-Down Memoization: O(N) time, O(N) space
    static uint64_t memoized(int n, std::vector<uint64_t>& memo) {
        if (n <= 1) return static_cast<uint64_t>(n);
        if (memo[n] != 0) return memo[n];
        return memo[n] = memoized(n - 1, memo) + memoized(n - 2, memo);
    }

    // 3. Bottom-Up Tabulation: O(N) time, O(N) table space
    static uint64_t tabulated(int n) {
        if (n <= 1) return static_cast<uint64_t>(n);
        std::vector<uint64_t> dp(n + 1, 0);
        dp[1] = 1;
        for (int i = 2; i <= n; ++i) {
            dp[i] = dp[i - 1] + dp[i - 2];
        }
        return dp[n];
    }

    // 4. Space-Optimized Tabulation: O(N) time, O(1) space
    static uint64_t space_optimized(int n) {
        if (n <= 1) return static_cast<uint64_t>(n);
        uint64_t prev2 = 0, prev1 = 1;
        for (int i = 2; i <= n; ++i) {
            uint64_t curr = prev1 + prev2;
            prev2 = prev1;
            prev1 = curr;
        }
        return prev1;
    }
};

/**
 * @brief Archetype 2: 2D Grid Unique Paths.
 * Evaluates state transition on a Directed Acyclic Graph (DAG).
 */
class GridPathsDemo {
public:
    // Tabulation with 2D array: O(M * N) time, O(M * N) space
    static int64_t unique_paths_2d(int m, int n) {
        std::vector<std::vector<int64_t>> dp(m, std::vector<int64_t>(n, 1));
        for (int i = 1; i < m; ++i) {
            for (int j = 1; j < n; ++j) {
                dp[i][j] = dp[i - 1][j] + dp[i][j - 1];
            }
        }
        return dp[m - 1][n - 1];
    }

    // Space-optimized with 1D rolling array: O(M * N) time, O(N) space
    static int64_t unique_paths_1d(int m, int n) {
        std::vector<int64_t> dp(n, 1);
        for (int i = 1; i < m; ++i) {
            for (int j = 1; j < n; ++j) {
                dp[j] += dp[j - 1];
            }
        }
        return dp[n - 1];
    }
};

/**
 * @brief Archetype 3: 0-1 Knapsack Problem.
 * Demonstrates Optimal Substructure and 1D reverse-iteration rolling array.
 */
class KnapsackDemo {
public:
    static int knapsack_1d(const std::vector<int>& weights,
                           const std::vector<int>& values,
                           int capacity) {
        std::vector<int> dp(capacity + 1, 0);
        for (size_t i = 0; i < weights.size(); ++i) {
            int w = weights[i];
            int v = values[i];
            // Reverse iteration ensures each item is used at most once
            for (int cap = capacity; cap >= w; --cap) {
                dp[cap] = std::max(dp[cap], dp[cap - w] + v);
            }
        }
        return dp[capacity];
    }
};

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Dynamic Programming Intuition C++17 Verification..." << std::endl;

    // 1. Fibonacci Equivalence
    {
        int n = 15;
        uint64_t ans_naive = FibonacciDemo::naive_recursive(n);
        std::vector<uint64_t> memo(n + 1, 0);
        uint64_t ans_memo = FibonacciDemo::memoized(n, memo);
        uint64_t ans_tab = FibonacciDemo::tabulated(n);
        uint64_t ans_opt = FibonacciDemo::space_optimized(n);

        assert(ans_naive == 610);
        assert(ans_memo == ans_naive);
        assert(ans_tab == ans_naive);
        assert(ans_opt == ans_naive);

        // Larger n where naive would timeout
        assert(FibonacciDemo::space_optimized(50) == 12586269025ULL);
    }

    // 2. Grid Paths
    {
        assert(GridPathsDemo::unique_paths_2d(3, 7) == 28);
        assert(GridPathsDemo::unique_paths_1d(3, 7) == 28);
        assert(GridPathsDemo::unique_paths_1d(1, 1) == 1);
        assert(GridPathsDemo::unique_paths_1d(3, 3) == 6);
    }

    // 3. 0-1 Knapsack
    {
        std::vector<int> weights = {1, 3, 4, 5};
        std::vector<int> values = {1, 4, 5, 7};
        int capacity = 7;
        // Best: item 1 (w=3, v=4) + item 2 (w=4, v=5) -> w=7, v=9
        assert(KnapsackDemo::knapsack_1d(weights, values, capacity) == 9);
    }

    std::cout << "[PASSED] Dynamic Programming Intuition C++17 All Tests Passed!" << std::endl;
    return 0;
}
