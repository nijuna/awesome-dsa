/**
 * @file induction.cpp
 * @brief Algorithmic realizations of mathematical induction proofs.
 *
 * Implements the constructive L-Tromino tiling algorithm for 2^n x 2^n deficient grids,
 * and structural induction verification for full binary trees.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cmath>

namespace dsa {

/**
 * @brief Grid for L-Tromino tiling.
 * 0 indicates untiled cell; -1 indicates missing hole; positive integers indicate tile IDs.
 */
class TrominoBoard {
private:
    int n_; // dimension size 2^k
    std::vector<std::vector<int>> grid_;
    int tile_id_ = 1;

    void tile_recursive(int top, int left, int size, int hole_r, int hole_c) {
        if (size == 1) return;

        int current_tile = tile_id_++;
        int half = size / 2;
        int mid_r = top + half;
        int mid_c = left + half;

        // Check which quadrant contains the hole
        bool hole_top_left = (hole_r < mid_r && hole_c < mid_c);
        bool hole_top_right = (hole_r < mid_r && hole_c >= mid_c);
        bool hole_bottom_left = (hole_r >= mid_r && hole_c < mid_c);
        bool hole_bottom_right = (hole_r >= mid_r && hole_c >= mid_c);

        // Place central L-tromino covering the 3 intact quadrants
        grid_[mid_r - 1][mid_c - 1] = hole_top_left ? grid_[mid_r - 1][mid_c - 1] : current_tile;
        grid_[mid_r - 1][mid_c] = hole_top_right ? grid_[mid_r - 1][mid_c] : current_tile;
        grid_[mid_r][mid_c - 1] = hole_bottom_left ? grid_[mid_r][mid_c - 1] : current_tile;
        grid_[mid_r][mid_c] = hole_bottom_right ? grid_[mid_r][mid_c] : current_tile;

        // Recurse on all 4 quadrants (each now has exactly 1 hole)
        tile_recursive(top, left, half,
                       hole_top_left ? hole_r : mid_r - 1,
                       hole_top_left ? hole_c : mid_c - 1);

        tile_recursive(top, mid_c, half,
                       hole_top_right ? hole_r : mid_r - 1,
                       hole_top_right ? hole_c : mid_c);

        tile_recursive(mid_r, left, half,
                       hole_bottom_left ? hole_r : mid_r,
                       hole_bottom_left ? hole_c : mid_c - 1);

        tile_recursive(mid_r, mid_c, half,
                       hole_bottom_right ? hole_r : mid_r,
                       hole_bottom_right ? hole_c : mid_c);
    }

public:
    TrominoBoard(int k, int missing_r, int missing_c) : n_(1 << k), grid_(n_, std::vector<int>(n_, 0)) {
        assert(missing_r >= 0 && missing_r < n_);
        assert(missing_c >= 0 && missing_c < n_);
        grid_[missing_r][missing_c] = -1; // Missing cell
        tile_recursive(0, 0, n_, missing_r, missing_c);
    }

    bool verify_tiling(int missing_r, int missing_c) const {
        // Assert exactly one -1 at (missing_r, missing_c)
        if (grid_[missing_r][missing_c] != -1) return false;

        // Count occurrences of each tile ID: must be exactly 3 per tromino
        std::vector<int> counts(tile_id_, 0);
        for (int r = 0; r < n_; ++r) {
            for (int c = 0; c < n_; ++c) {
                if (r == missing_r && c == missing_c) continue;
                int id = grid_[r][c];
                if (id <= 0 || id >= tile_id_) return false;
                counts[id]++;
            }
        }

        for (int id = 1; id < tile_id_; ++id) {
            if (counts[id] != 3) return false;
        }

        return true;
    }

    int dimension() const { return n_; }
};

// --- Structural Induction Verification on Binary Trees ---

struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;
    explicit TreeNode(int v) : val(v), left(nullptr), right(nullptr) {}
};

/**
 * @brief Verifies the structural induction property for Full Binary Trees:
 * Leaves = Internal Nodes + 1 (L = I + 1).
 */
inline std::pair<int, int> count_leaves_and_internals(const TreeNode* root) {
    if (!root) return {0, 0};
    if (!root->left && !root->right) {
        // Base case: Leaf node
        return {1, 0}; // 1 leaf, 0 internal
    }

    // Inductive step: Composite node
    auto left_counts = count_leaves_and_internals(root->left);
    auto right_counts = count_leaves_and_internals(root->right);

    int total_leaves = left_counts.first + right_counts.first;
    int total_internals = 1 + left_counts.second + right_counts.second;
    return {total_leaves, total_internals};
}

} // namespace dsa

int main() {
    std::cout << "Running Mathematical Induction algorithmic verification..." << std::endl;

    // 1. Tromino Tiling constructive induction on 4x4 (k=2) and 8x8 (k=3) boards
    for (int k = 1; k <= 3; ++k) {
        int dim = 1 << k;
        // Test with arbitrary hole locations
        for (int hr = 0; hr < dim; hr += (dim > 2 ? 2 : 1)) {
            for (int hc = 0; hc < dim; hc += (dim > 2 ? 2 : 1)) {
                dsa::TrominoBoard board(k, hr, hc);
                assert(board.verify_tiling(hr, hc));
            }
        }
    }

    // 2. Structural Induction Verification on Full Binary Tree
    // Construct a full binary tree with 3 internal nodes and 4 leaves
    // Tree structure:
    //         (1)
    //        /   |
    //      (2)   (3)
    //     /   |
    //   (4)   (5)
    dsa::TreeNode n1(1), n2(2), n3(3), n4(4), n5(5);
    n1.left = &n2;
    n1.right = &n3;
    n2.left = &n4;
    n2.right = &n5;

    auto counts = dsa::count_leaves_and_internals(&n1);
    int leaves = counts.first;
    int internals = counts.second;

    assert(internals == 2);
    assert(leaves == 3);
    assert(leaves == internals + 1); // L = I + 1 verified!

    std::cout << "[PASS] Tromino tiling inductive decomposition verified across 2^k grids." << std::endl;
    std::cout << "[PASS] Structural induction tree theorem (L = I + 1) verified." << std::endl;
    std::cout << "All Mathematical Induction assertions passed successfully!" << std::endl;
    return 0;
}
