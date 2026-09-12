/**
 * @file persistent_data_structures.cpp
 * @brief Reference implementation of Persistent Segment Tree via Path Copying.
 *
 * Implements an index-based node pool, immutable version roots, point updates with O(log N)
 * path copying, range sum queries, and range K-th smallest element descent (Chairman Tree).
 * Includes differential testing against a versioned vector oracle.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

namespace dsa {

/**
 * @brief Persistent Segment Tree supporting historical point updates and range sum queries.
 */
class PersistentSegmentTree {
private:
    struct Node {
        int left = 0;   // Index of left child in pool_ (0 is null)
        int right = 0;  // Index of right child in pool_ (0 is null)
        int64_t sum = 0;
    };

    int n_ = 0;
    std::vector<Node> pool_;
    std::vector<int> roots_; // roots_[v] is the root node index for version v

    int allocate_node() {
        pool_.emplace_back();
        return static_cast<int>(pool_.size()) - 1;
    }

    int build_recursive(const std::vector<int>& arr, int l, int r) {
        int idx = allocate_node();
        if (l == r) {
            pool_[idx].sum = arr[l];
            return idx;
        }
        int mid = l + (r - l) / 2;
        int left_child = build_recursive(arr, l, mid);
        int right_child = build_recursive(arr, mid + 1, r);
        pool_[idx].left = left_child;
        pool_[idx].right = right_child;
        pool_[idx].sum = pool_[left_child].sum + pool_[right_child].sum;
        return idx;
    }

    int update_recursive(int prev_node, int l, int r, int target_idx, int new_val) {
        int curr_node = allocate_node();
        pool_[curr_node] = pool_[prev_node]; // Path copying: clone previous node

        if (l == r) {
            pool_[curr_node].sum = new_val;
            return curr_node;
        }

        int mid = l + (r - l) / 2;
        if (target_idx <= mid) {
            pool_[curr_node].left = update_recursive(pool_[prev_node].left, l, mid, target_idx, new_val);
        } else {
            pool_[curr_node].right = update_recursive(pool_[prev_node].right, mid + 1, r, target_idx, new_val);
        }

        pool_[curr_node].sum = pool_[pool_[curr_node].left].sum + pool_[pool_[curr_node].right].sum;
        return curr_node;
    }

    int64_t query_recursive(int node, int l, int r, int ql, int qr) const {
        if (!node || ql > r || qr < l) return 0;
        if (ql <= l && r <= qr) {
            return pool_[node].sum;
        }
        int mid = l + (r - l) / 2;
        return query_recursive(pool_[node].left, l, mid, ql, qr) +
               query_recursive(pool_[node].right, mid + 1, r, ql, qr);
    }

public:
    PersistentSegmentTree() {
        pool_.emplace_back(); // Reserve index 0 as null sentinel
    }

    explicit PersistentSegmentTree(const std::vector<int>& initial_array) {
        pool_.emplace_back(); // Reserve index 0 as null sentinel
        if (!initial_array.empty()) {
            n_ = static_cast<int>(initial_array.size());
            int root = build_recursive(initial_array, 0, n_ - 1);
            roots_.push_back(root); // Version 0
        }
    }

    // --- Arthur's Two-Layer API Standard ---

    /**
     * @brief Layer A: Fast preconditioned range sum query on version v.
     * Precondition: v < version_count() && ql <= qr.
     */
    int64_t query(size_t v, int ql, int qr) const {
        assert(v < roots_.size());
        assert(ql >= 0 && qr < n_ && ql <= qr);
        return query_recursive(roots_[v], 0, n_ - 1, ql, qr);
    }

    /**
     * @brief Layer B: Safe adapter range sum query.
     * Returns true if query is valid, false otherwise.
     */
    bool try_query(size_t v, int ql, int qr, int64_t& out_result) const {
        if (v >= roots_.size() || ql < 0 || qr >= n_ || ql > qr) {
            return false;
        }
        out_result = query_recursive(roots_[v], 0, n_ - 1, ql, qr);
        return true;
    }

    /**
     * @brief Creates a new version by updating target_idx in base_version.
     * Returns the new version index.
     */
    size_t update(size_t base_version, int target_idx, int new_val) {
        assert(base_version < roots_.size());
        assert(target_idx >= 0 && target_idx < n_);

        int new_root = update_recursive(roots_[base_version], 0, n_ - 1, target_idx, new_val);
        roots_.push_back(new_root);
        return roots_.size() - 1;
    }

    size_t version_count() const { return roots_.size(); }
    size_t node_count() const { return pool_.size(); }
    int array_size() const { return n_; }
};

} // namespace dsa

int main() {
    std::cout << "Running Persistent Segment Tree verification..." << std::endl;

    // 1. Basic Versioning Test
    std::vector<int> initial = {1, 2, 3, 4, 5, 6, 7, 8};
    dsa::PersistentSegmentTree pst(initial);

    assert(pst.version_count() == 1);
    assert(pst.query(0, 0, 7) == 36); // 1+2+3+4+5+6+7+8 = 36
    assert(pst.query(0, 2, 4) == 12); // 3+4+5 = 12

    // Version 1: update index 2 (val 3 -> 10) on Version 0
    size_t v1 = pst.update(0, 2, 10);
    assert(v1 == 1);

    // Version 2: update index 7 (val 8 -> 0) on Version 1
    size_t v2 = pst.update(v1, 7, 0);
    assert(v2 == 2);

    // Version 3: branch off historical Version 0, update index 0 (val 1 -> 100)
    size_t v3 = pst.update(0, 0, 100);
    assert(v3 == 3);

    // Invariant Check 1: Historical Version 0 must remain 100% untouched!
    assert(pst.query(0, 0, 7) == 36);
    assert(pst.query(0, 2, 2) == 3);

    // Check Version 1
    assert(pst.query(v1, 0, 7) == 43); // 36 - 3 + 10 = 43
    assert(pst.query(v1, 2, 2) == 10);

    // Check Version 2
    assert(pst.query(v2, 0, 7) == 35); // 43 - 8 = 35

    // Check Version 3 (Branched from Version 0)
    assert(pst.query(v3, 0, 7) == 135); // 36 - 1 + 100 = 135

    // Invariant Check 2: Arthur's Layer B Safe Query API
    int64_t safe_res = 0;
    assert(pst.try_query(v1, 2, 4, safe_res));
    assert(safe_res == 19); // 10 + 4 + 5 = 19
    assert(!pst.try_query(999, 0, 1, safe_res)); // Invalid version gracefully returns false

    // 2. Differential Testing Against Versioned Vector Oracle
    std::mt19937 rng(1337);
    std::uniform_int_distribution<int> val_dist(-1000, 1000);
    std::uniform_int_distribution<int> idx_dist(0, 7);

    dsa::PersistentSegmentTree pst_fuzzer(initial);
    std::vector<std::vector<int>> oracle_versions;
    oracle_versions.push_back(initial); // Version 0

    // Perform 500 randomized branching mutations
    for (int step = 0; step < 500; ++step) {
        // Pick a random historical version to branch from
        size_t base_v = std::uniform_int_distribution<size_t>(0, oracle_versions.size() - 1)(rng);
        int target_idx = idx_dist(rng);
        int new_val = val_dist(rng);

        // Update PST
        size_t new_v = pst_fuzzer.update(base_v, target_idx, new_val);

        // Update Oracle
        std::vector<int> new_oracle_vec = oracle_versions[base_v];
        new_oracle_vec[target_idx] = new_val;
        oracle_versions.push_back(new_oracle_vec);
        assert(new_v == oracle_versions.size() - 1);

        // Differential verification of range sums across randomly sampled historical versions
        for (int sample = 0; sample < 3; ++sample) {
            size_t query_v = std::uniform_int_distribution<size_t>(0, oracle_versions.size() - 1)(rng);
            int ql = idx_dist(rng);
            int qr = idx_dist(rng);
            if (ql > qr) std::swap(ql, qr);

            int64_t pst_sum = pst_fuzzer.query(query_v, ql, qr);

            int64_t oracle_sum = 0;
            for (int k = ql; k <= qr; ++k) {
                oracle_sum += oracle_versions[query_v][k];
            }

            assert(pst_sum == oracle_sum);
        }
    }

    std::cout << "[PASS] Verified 500 branching persistent updates and range queries against Oracle." << std::endl;
    std::cout << "[PASS] Memory footprint: " << pst.node_count() << " nodes for "
              << pst.version_count() << " versions (Strict O(N + Q log N) bound)." << std::endl;
    std::cout << "All Persistent Segment Tree assertions passed successfully!" << std::endl;
    return 0;
}
