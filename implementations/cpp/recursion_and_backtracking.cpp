#include <cassert>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

namespace dsa {

/**
 * @brief N-Queens solver using backtracking with O(1) conflict checking.
 */
class NQueens {
private:
    int n_;
    std::vector<std::vector<std::string>> solutions_;
    std::vector<int> col_placement_; // col_placement_[row] = col
    std::vector<bool> used_cols_;
    std::vector<bool> used_diag1_;   // r + c
    std::vector<bool> used_diag2_;   // r - c + n - 1

    void backtrack(int row) {
        if (row == n_) {
            // Found a valid solution, construct board
            std::vector<std::string> board(n_, std::string(n_, '.'));
            for (int r = 0; r < n_; ++r) {
                board[r][col_placement_[r]] = 'Q';
            }
            solutions_.push_back(std::move(board));
            return;
        }

        for (int c = 0; c < n_; ++c) {
            int d1 = row + c;
            int d2 = row - c + n_ - 1;

            if (used_cols_[c] || used_diag1_[d1] || used_diag2_[d2]) {
                continue; // Pruned
            }

            // Choose
            col_placement_[row] = c;
            used_cols_[c] = true;
            used_diag1_[d1] = true;
            used_diag2_[d2] = true;

            // Explore
            backtrack(row + 1);

            // Unchoose
            used_cols_[c] = false;
            used_diag1_[d1] = false;
            used_diag2_[d2] = false;
        }
    }

public:
    explicit NQueens(int n)
        : n_(n), col_placement_(n, -1),
          used_cols_(n, false), used_diag1_(2 * n, false), used_diag2_(2 * n, false) {}

    std::vector<std::vector<std::string>> solve() {
        solutions_.clear();
        if (n_ <= 0) return solutions_;
        backtrack(0);
        return solutions_;
    }

    int count_solutions() {
        return static_cast<int>(solve().size());
    }
};

/**
 * @brief Generate all subsets (power set) of a vector using backtracking.
 */
inline void subsets_helper(const std::vector<int>& nums, size_t idx,
                           std::vector<int>& current,
                           std::vector<std::vector<int>>& result) {
    if (idx == nums.size()) {
        result.push_back(current);
        return;
    }

    // Branch 1: Exclude nums[idx]
    subsets_helper(nums, idx + 1, current, result);

    // Branch 2: Include nums[idx]
    current.push_back(nums[idx]);
    subsets_helper(nums, idx + 1, current, result);
    current.pop_back(); // Unchoose
}

inline std::vector<std::vector<int>> generate_subsets(const std::vector<int>& nums) {
    std::vector<std::vector<int>> result;
    std::vector<int> current;
    subsets_helper(nums, 0, current, result);
    return result;
}

/**
 * @brief Generate all subsets with duplicates handled (Subsets II).
 */
inline void subsets_with_dup_helper(const std::vector<int>& nums, size_t start,
                                    std::vector<int>& current,
                                    std::vector<std::vector<int>>& result) {
    result.push_back(current);
    for (size_t i = start; i < nums.size(); ++i) {
        if (i > start && nums[i] == nums[i - 1]) {
            continue; // Skip duplicate branch
        }
        current.push_back(nums[i]);
        subsets_with_dup_helper(nums, i + 1, current, result);
        current.pop_back();
    }
}

inline std::vector<std::vector<int>> generate_subsets_unique(std::vector<int> nums) {
    std::sort(nums.begin(), nums.end());
    std::vector<std::vector<int>> result;
    std::vector<int> current;
    subsets_with_dup_helper(nums, 0, current, result);
    return result;
}

/**
 * @brief Generate all permutations using backtracking and in-place swapping.
 */
inline void permutations_helper(std::vector<int>& nums, size_t start,
                                std::vector<std::vector<int>>& result) {
    if (start == nums.size()) {
        result.push_back(nums);
        return;
    }
    for (size_t i = start; i < nums.size(); ++i) {
        std::swap(nums[start], nums[i]);       // Choose
        permutations_helper(nums, start + 1, result); // Explore
        std::swap(nums[start], nums[i]);       // Unchoose
    }
}

inline std::vector<std::vector<int>> generate_permutations(std::vector<int> nums) {
    std::vector<std::vector<int>> result;
    permutations_helper(nums, 0, result);
    return result;
}

/**
 * @brief Combination Sum: Find all unique combinations where candidates sum to target.
 * Candidates can be reused unlimited times.
 */
inline void combination_sum_helper(const std::vector<int>& candidates, int target,
                                   size_t start, std::vector<int>& current,
                                   std::vector<std::vector<int>>& result) {
    if (target == 0) {
        result.push_back(current);
        return;
    }
    for (size_t i = start; i < candidates.size(); ++i) {
        if (candidates[i] > target) {
            break; // Pruned because array is sorted
        }
        current.push_back(candidates[i]);
        combination_sum_helper(candidates, target - candidates[i], i, current, result);
        current.pop_back();
    }
}

inline std::vector<std::vector<int>> combination_sum(std::vector<int> candidates, int target) {
    std::sort(candidates.begin(), candidates.end());
    std::vector<std::vector<int>> result;
    std::vector<int> current;
    combination_sum_helper(candidates, target, 0, current, result);
    return result;
}

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Recursion and Backtracking C++17 Verification..." << std::endl;

    // 1. N-Queens Verification
    {
        NQueens q1(1);
        assert(q1.count_solutions() == 1);

        NQueens q2(2);
        assert(q2.count_solutions() == 0);

        NQueens q3(3);
        assert(q3.count_solutions() == 0);

        NQueens q4(4);
        assert(q4.count_solutions() == 2);
        auto sols4 = q4.solve();
        assert(sols4.size() == 2);
        assert(sols4[0].size() == 4);

        NQueens q8(8);
        assert(q8.count_solutions() == 92);
    }

    // 2. Subset Generation
    {
        std::vector<int> nums = {1, 2, 3};
        auto subsets = generate_subsets(nums);
        assert(subsets.size() == 8);

        std::vector<int> dups = {1, 2, 2};
        auto unique_subsets = generate_subsets_unique(dups);
        assert(unique_subsets.size() == 6); // [], [1], [1,2], [1,2,2], [2], [2,2]
    }

    // 3. Permutation Generation
    {
        std::vector<int> nums = {1, 2, 3};
        auto perms = generate_permutations(nums);
        assert(perms.size() == 6);

        std::vector<int> single = {5};
        auto single_perms = generate_permutations(single);
        assert(single_perms.size() == 1);
        assert((single_perms[0] == std::vector<int>{5}));
    }

    // 4. Combination Sum with Pruning
    {
        std::vector<int> cands = {2, 3, 6, 7};
        auto res = combination_sum(cands, 7);
        // [2, 2, 3] and [7]
        assert(res.size() == 2);
        assert((res[0] == std::vector<int>{2, 2, 3}));
        assert((res[1] == std::vector<int>{7}));

        auto res_empty = combination_sum({2}, 1);
        assert(res_empty.empty());
    }

    std::cout << "[PASSED] Recursion and Backtracking C++17 All Tests Passed!" << std::endl;
    return 0;
}
