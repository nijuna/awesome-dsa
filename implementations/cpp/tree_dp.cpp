/**
 * Reference Implementation: Tree Dynamic Programming
 * Demonstrates:
 * 1. Subtree Aggregation: Subtree sizes, heights, and depth sums via bottom-up post-order DFS.
 * 2. Maximum Weight Independent Set (MWIS): Include/exclude DP O(V) and node set reconstruction.
 * 3. Tree Diameter & Centers: Downward path combining O(V) and diameter witness path reconstruction.
 * 4. All-Node Rerooting DP: Two-pass DFS computing sum of distances to all other nodes in O(V) time.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <array>

namespace tree_dp {

// ============================================================================
// 1. Subtree Aggregations (Size and Height)
// ============================================================================

struct TreeAggregates {
    std::vector<int> subtree_size;
    std::vector<int> height;
};

TreeAggregates compute_subtree_size_height(const std::vector<std::vector<int>>& graph, int root = 0) {
    int n = static_cast<int>(graph.size());
    TreeAggregates result{std::vector<int>(n, 0), std::vector<int>(n, 0)};
    if (n == 0) return result;

    auto dfs = [&](auto&& self, int u, int parent) -> void {
        result.subtree_size[u] = 1;
        result.height[u] = 0;

        for (int v : graph[u]) {
            if (v == parent) continue;
            self(self, v, u);
            result.subtree_size[u] += result.subtree_size[v];
            result.height[u] = std::max(result.height[u], result.height[v] + 1);
        }
    };

    dfs(dfs, root, -1);
    return result;
}

// ============================================================================
// 2. Maximum Weight Independent Set (MWIS) on Trees
// ============================================================================

/**
 * Computes the Maximum Weight Independent Set cost: O(V) Time, O(V) Space.
 * dp[u][1] = max weight in subtree Tu if u is included (children must be excluded).
 * dp[u][0] = max weight in subtree Tu if u is excluded (children can be included or excluded).
 */
long long maximum_weight_independent_set(
    const std::vector<std::vector<int>>& graph,
    const std::vector<int>& weight,
    int root = 0) {

    int n = static_cast<int>(graph.size());
    if (n == 0) return 0;

    std::vector<std::array<long long, 2>> dp(n, {0, 0});

    auto dfs = [&](auto&& self, int u, int parent) -> void {
        dp[u][1] = weight[u];
        dp[u][0] = 0;

        for (int v : graph[u]) {
            if (v == parent) continue;
            self(self, v, u);
            dp[u][1] += dp[v][0];
            dp[u][0] += std::max(dp[v][0], dp[v][1]);
        }
    };

    dfs(dfs, root, -1);
    return std::max(dp[root][0], dp[root][1]);
}

/**
 * Computes MWIS and reconstructs the optimal set of node indices.
 * Time: O(V) Time, O(V) Space.
 */
std::pair<long long, std::vector<int>> maximum_weight_independent_set_reconstruct(
    const std::vector<std::vector<int>>& graph,
    const std::vector<int>& weight,
    int root = 0) {

    int n = static_cast<int>(graph.size());
    if (n == 0) return {0, {}};

    std::vector<std::array<long long, 2>> dp(n, {0, 0});

    auto dfs = [&](auto&& self, int u, int parent) -> void {
        dp[u][1] = weight[u];
        dp[u][0] = 0;

        for (int v : graph[u]) {
            if (v == parent) continue;
            self(self, v, u);
            dp[u][1] += dp[v][0];
            dp[u][0] += std::max(dp[v][0], dp[v][1]);
        }
    };

    dfs(dfs, root, -1);

    std::vector<int> chosen;

    auto build = [&](auto&& self, int u, int parent, bool parent_taken) -> void {
        bool take_u = false;
        if (!parent_taken && dp[u][1] >= dp[u][0]) {
            take_u = true;
            chosen.push_back(u);
        }

        for (int v : graph[u]) {
            if (v == parent) continue;
            self(self, v, u, take_u);
        }
    };

    build(build, root, -1, false);
    std::sort(chosen.begin(), chosen.end());

    return {std::max(dp[root][0], dp[root][1]), chosen};
}

// ============================================================================
// 3. Tree Diameter and Centers
// ============================================================================

/**
 * Computes tree diameter in number of edges using downward path DP.
 * Time: O(V) Time, O(V) Space.
 */
int tree_diameter(const std::vector<std::vector<int>>& graph, int root = 0) {
    int n = static_cast<int>(graph.size());
    if (n <= 1) return 0;

    int diameter = 0;

    auto dfs = [&](auto&& self, int u, int parent) -> int {
        int best1 = 0, best2 = 0;

        for (int v : graph[u]) {
            if (v == parent) continue;
            int child_down = self(self, v, u) + 1;

            if (child_down > best1) {
                best2 = best1;
                best1 = child_down;
            } else if (child_down > best2) {
                best2 = child_down;
            }
        }

        diameter = std::max(diameter, best1 + best2);
        return best1;
    };

    dfs(dfs, root, -1);
    return diameter;
}

/**
 * Reconstructs a full diameter path (ordered list of nodes from endpoint to endpoint).
 * Time: O(V) Time, O(V) Space.
 */
std::vector<int> tree_diameter_path(const std::vector<std::vector<int>>& graph, int root = 0) {
    int n = static_cast<int>(graph.size());
    if (n == 0) return {};
    if (n == 1) return {0};

    int best_diameter = 0;
    int best_inflection = root;
    int branch1_child = -1;
    int branch2_child = -1;

    std::vector<int> longest_down(n, 0);
    std::vector<int> best_child(n, -1);

    auto dfs = [&](auto&& self, int u, int parent) -> void {
        int b1 = 0, b2 = 0;
        int c1 = -1, c2 = -1;

        for (int v : graph[u]) {
            if (v == parent) continue;
            self(self, v, u);
            int d = longest_down[v] + 1;
            if (d > b1) {
                b2 = b1; c2 = c1;
                b1 = d;  c1 = v;
            } else if (d > b2) {
                b2 = d;  c2 = v;
            }
        }

        longest_down[u] = b1;
        best_child[u] = c1;

        if (b1 + b2 > best_diameter) {
            best_diameter = b1 + b2;
            best_inflection = u;
            branch1_child = c1;
            branch2_child = c2;
        }
    };

    dfs(dfs, root, -1);

    // Build branch 1 path
    std::vector<int> path1;
    for (int curr = branch1_child; curr != -1; curr = best_child[curr]) {
        path1.push_back(curr);
    }
    std::reverse(path1.begin(), path1.end());

    // Build branch 2 path
    std::vector<int> path2;
    for (int curr = branch2_child; curr != -1; curr = best_child[curr]) {
        path2.push_back(curr);
    }

    std::vector<int> full_path;
    full_path.reserve(path1.size() + 1 + path2.size());
    full_path.insert(full_path.end(), path1.begin(), path1.end());
    full_path.push_back(best_inflection);
    full_path.insert(full_path.end(), path2.begin(), path2.end());

    return full_path;
}

/**
 * Finds the center node(s) of the tree (1 or 2 nodes minimizing max distance to all others).
 * Center lies at the middle of the diameter path.
 */
std::vector<int> tree_centers(const std::vector<std::vector<int>>& graph) {
    int n = static_cast<int>(graph.size());
    if (n == 0) return {};
    if (n == 1) return {0};

    auto path = tree_diameter_path(graph, 0);
    int len = static_cast<int>(path.size());

    if (len % 2 == 1) {
        return {path[len / 2]};
    } else {
        std::vector<int> centers = {path[len / 2 - 1], path[len / 2]};
        std::sort(centers.begin(), centers.end());
        return centers;
    }
}

// ============================================================================
// 4. All-Node Rerooting DP (Sum of Distances to All Nodes)
// ============================================================================

/**
 * Computes sum of distances from every node u to all other nodes in O(V) time.
 * Pass 1: Bottom-up post-order DFS computing subtree size and subtree downward distances.
 * Pass 2: Top-down pre-order DFS computing all-node distance sums:
 *         ans[v] = ans[u] - size[v] + (n - size[v]).
 */
std::vector<long long> sum_of_distances_all_nodes(const std::vector<std::vector<int>>& graph, int root = 0) {
    int n = static_cast<int>(graph.size());
    if (n == 0) return {};
    if (n == 1) return {0};

    std::vector<int> size(n, 0);
    std::vector<long long> down(n, 0), ans(n, 0);

    auto dfs1 = [&](auto&& self, int u, int parent) -> void {
        size[u] = 1;
        down[u] = 0;

        for (int v : graph[u]) {
            if (v == parent) continue;
            self(self, v, u);
            size[u] += size[v];
            down[u] += down[v] + size[v];
        }
    };

    auto dfs2 = [&](auto&& self, int u, int parent) -> void {
        for (int v : graph[u]) {
            if (v == parent) continue;
            ans[v] = ans[u] - size[v] + (n - size[v]);
            self(self, v, u);
        }
    };

    dfs1(dfs1, root, -1);
    ans[root] = down[root];
    dfs2(dfs2, root, -1);

    return ans;
}

} // namespace tree_dp

// ============================================================================
// Unit Tests
// ============================================================================

int main() {
    using namespace tree_dp;

    // Test 1: Star graph with center 0 and leaves 1, 2, 3
    //       0
    //     / | /
    //    1  2  3
    {
        std::vector<std::vector<int>> star(4);
        star[0] = {1, 2, 3};
        star[1] = {0};
        star[2] = {0};
        star[3] = {0};

        auto agg = compute_subtree_size_height(star, 0);
        assert(agg.subtree_size[0] == 4);
        assert(agg.subtree_size[1] == 1);
        assert(agg.height[0] == 1);

        // MWIS: weights = [10, 5, 5, 5] -> pick {1, 2, 3} (sum 15) vs {0} (sum 10)
        std::vector<int> weights = {10, 5, 5, 5};
        assert(maximum_weight_independent_set(star, weights, 0) == 15);

        auto [w, chosen] = maximum_weight_independent_set_reconstruct(star, weights, 0);
        assert(w == 15);
        std::vector<int> expected_chosen = {1, 2, 3};
        assert(chosen == expected_chosen);

        // If root weight is 20 -> pick {0}
        std::vector<int> weights2 = {20, 5, 5, 5};
        auto [w2, chosen2] = maximum_weight_independent_set_reconstruct(star, weights2, 0);
        assert(w2 == 20);
        std::vector<int> expected_chosen2 = {0};
        assert(chosen2 == expected_chosen2);

        // Diameter of star is 2 edges (e.g. 1 - 0 - 2)
        assert(tree_diameter(star, 0) == 2);
        auto path = tree_diameter_path(star, 0);
        assert(static_cast<int>(path.size()) == 3);
        assert(path[1] == 0);

        auto centers = tree_centers(star);
        assert(centers.size() == 1);
        assert(centers[0] == 0);

        // Rerooting distance sum:
        // From 0: 1 + 1 + 1 = 3
        // From 1: 1 (to 0) + 2 (to 2) + 2 (to 3) = 5
        auto dist_sums = sum_of_distances_all_nodes(star, 0);
        assert(dist_sums[0] == 3);
        assert(dist_sums[1] == 5);
        assert(dist_sums[2] == 5);
        assert(dist_sums[3] == 5);
    }

    // Test 2: Line graph 0 - 1 - 2 - 3 - 4
    // Diameter = 4 edges, Center = 2
    {
        int n = 5;
        std::vector<std::vector<int>> line(n);
        for (int i = 0; i < n - 1; ++i) {
            line[i].push_back(i + 1);
            line[i + 1].push_back(i);
        }

        assert(tree_diameter(line, 0) == 4);
        auto path = tree_diameter_path(line, 0);
        assert(static_cast<int>(path.size()) == 5);

        auto centers = tree_centers(line);
        assert(centers.size() == 1);
        assert(centers[0] == 2);

        // From 2: 2 + 1 + 0 + 1 + 2 = 6
        // From 0: 0 + 1 + 2 + 3 + 4 = 10
        auto dist_sums = sum_of_distances_all_nodes(line, 0);
        assert(dist_sums[2] == 6);
        assert(dist_sums[0] == 10);
        assert(dist_sums[4] == 10);
    }

    // Test 3: Even length line 0 - 1 - 2 - 3 (centers = {1, 2})
    {
        std::vector<std::vector<int>> line4(4);
        line4[0] = {1}; line4[1] = {0, 2}; line4[2] = {1, 3}; line4[3] = {2};

        assert(tree_diameter(line4, 0) == 3);
        auto centers = tree_centers(line4);
        assert(centers.size() == 2);
        assert(centers[0] == 1);
        assert(centers[1] == 2);
    }

    // Test 4: Single-node tree
    {
        std::vector<std::vector<int>> single(1);
        auto agg = compute_subtree_size_height(single, 0);
        assert(agg.subtree_size[0] == 1);
        assert(agg.height[0] == 0);

        std::vector<int> w = {42};
        assert(maximum_weight_independent_set(single, w, 0) == 42);
        auto [max_w, ch] = maximum_weight_independent_set_reconstruct(single, w, 0);
        assert(max_w == 42 && ch.size() == 1 && ch[0] == 0);

        assert(tree_diameter(single, 0) == 0);
        auto centers = tree_centers(single);
        assert(centers.size() == 1 && centers[0] == 0);

        auto dist = sum_of_distances_all_nodes(single, 0);
        assert(dist.size() == 1 && dist[0] == 0);
    }

    std::cout << "[PASS] All Tree DP C++ unit tests passed." << std::endl;
    return 0;
}
