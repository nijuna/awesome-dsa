/**
 * Reference Implementation: Topological Sorting & DAG Dynamic Programming
 * Demonstrates:
 * 1. DFS-based topological sort with 3-color directed cycle detection.
 * 2. Kahn's algorithm (in-degree peeling with FIFO queue).
 * 3. Lexicographical topological sort using a min-priority queue.
 * 4. Longest path / critical path computation in a DAG via topological DP.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <stdexcept>
#include <cassert>
#include <algorithm>

// ============================================================================
// 1. DFS-Based Topological Sort (3-Color Cycle Detection + Reverse Postorder)
// ============================================================================
std::vector<int> topological_sort_dfs(const std::vector<std::vector<int>>& graph) {
    int n = static_cast<int>(graph.size());
    std::vector<int> color(n, 0); // 0 = White (unvisited), 1 = Gray (visiting), 2 = Black (visited)
    std::vector<int> order;
    order.reserve(n);

    std::function<void(int)> dfs = [&](int u) {
        color[u] = 1; // Mark as currently exploring
        for (int v : graph[u]) {
            if (color[v] == 1) {
                throw std::runtime_error("Cycle detected: topological order does not exist");
            }
            if (color[v] == 0) {
                dfs(v);
            }
        }
        color[u] = 2; // Mark as finished
        order.push_back(u); // Postorder finish time
    };

    for (int u = 0; u < n; ++u) {
        if (color[u] == 0) {
            dfs(u);
        }
    }

    std::reverse(order.begin(), order.end());
    return order;
}

// ============================================================================
// 2. Kahn's Algorithm (In-Degree Peeling with FIFO Queue)
// ============================================================================
std::vector<int> topological_sort_kahn(const std::vector<std::vector<int>>& graph) {
    int n = static_cast<int>(graph.size());
    std::vector<int> indegree(n, 0);

    for (int u = 0; u < n; ++u) {
        for (int v : graph[u]) {
            ++indegree[v];
        }
    }

    std::queue<int> q;
    for (int u = 0; u < n; ++u) {
        if (indegree[u] == 0) {
            q.push(u);
        }
    }

    std::vector<int> order;
    order.reserve(n);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        order.push_back(u);

        for (int v : graph[u]) {
            if (--indegree[v] == 0) {
                q.push(v);
            }
        }
    }

    if (static_cast<int>(order.size()) != n) {
        throw std::runtime_error("Cycle detected: topological order does not exist");
    }

    return order;
}

// ============================================================================
// 3. Lexicographically Smallest Topological Sort (Min-Priority Queue)
// ============================================================================
std::vector<int> lexicographically_smallest_topological_sort(
    const std::vector<std::vector<int>>& graph) {
    int n = static_cast<int>(graph.size());
    std::vector<int> indegree(n, 0);

    for (int u = 0; u < n; ++u) {
        for (int v : graph[u]) {
            ++indegree[v];
        }
    }

    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    for (int u = 0; u < n; ++u) {
        if (indegree[u] == 0) {
            pq.push(u);
        }
    }

    std::vector<int> order;
    order.reserve(n);

    while (!pq.empty()) {
        int u = pq.top();
        pq.pop();
        order.push_back(u);

        for (int v : graph[u]) {
            if (--indegree[v] == 0) {
                pq.push(v);
            }
        }
    }

    if (static_cast<int>(order.size()) != n) {
        throw std::runtime_error("Cycle detected: topological order does not exist");
    }

    return order;
}

// ============================================================================
// 4. Longest Path / Critical Path in a Weighted DAG via Topological DP
// ============================================================================
struct WeightedEdge {
    int to;
    int weight;
};

int longest_path_dag(int n, const std::vector<std::vector<WeightedEdge>>& graph) {
    // Build unweighted graph to compute topological sort
    std::vector<std::vector<int>> unweighted(n);
    for (int u = 0; u < n; ++u) {
        for (const auto& e : graph[u]) {
            unweighted[u].push_back(e.to);
        }
    }

    std::vector<int> order = topological_sort_kahn(unweighted);
    std::vector<int> dist(n, 0); // dist[u] = longest path ending at or starting from sources

    for (int u : order) {
        for (const auto& edge : graph[u]) {
            int v = edge.to;
            int w = edge.weight;
            if (dist[u] + w > dist[v]) {
                dist[v] = dist[u] + w;
            }
        }
    }

    int max_path = 0;
    for (int d : dist) {
        max_path = std::max(max_path, d);
    }
    return max_path;
}

// ============================================================================
// Unit Tests
// ============================================================================
int main() {
    // 1. Classical DAG Example:
    // 5 -> 2, 5 -> 0, 4 -> 0, 4 -> 1, 2 -> 3, 3 -> 1
    std::vector<std::vector<int>> dag(6);
    dag[5] = {2, 0};
    dag[4] = {0, 1};
    dag[2] = {3};
    dag[3] = {1};

    std::vector<int> dfs_order = topological_sort_dfs(dag);
    std::vector<int> kahn_order = topological_sort_kahn(dag);
    std::vector<int> lex_order = lexicographically_smallest_topological_sort(dag);

    // Verify validity of topological orders: for every edge u -> v, pos(u) < pos(v)
    auto verify_order = [&](const std::vector<int>& order) {
        assert(order.size() == 6);
        std::vector<int> pos(6);
        for (int i = 0; i < 6; ++i) pos[order[i]] = i;
        for (int u = 0; u < 6; ++u) {
            for (int v : dag[u]) {
                assert(pos[u] < pos[v]);
            }
        }
    };

    verify_order(dfs_order);
    verify_order(kahn_order);
    verify_order(lex_order);

    // For lexicographically smallest: among {4, 5}, 4 is chosen first; then 5, then 0, 2, 3, 1
    std::vector<int> expected_lex = {4, 5, 0, 2, 3, 1};
    assert(lex_order == expected_lex);

    // 2. Cycle Detection Verification
    // Add cycle: 1 -> 5 creates 5 -> 2 -> 3 -> 1 -> 5
    std::vector<std::vector<int>> cyclic_graph = dag;
    cyclic_graph[1].push_back(5);

    bool dfs_caught = false;
    try {
        topological_sort_dfs(cyclic_graph);
    } catch (const std::runtime_error&) {
        dfs_caught = true;
    }
    assert(dfs_caught);

    bool kahn_caught = false;
    try {
        topological_sort_kahn(cyclic_graph);
    } catch (const std::runtime_error&) {
        kahn_caught = true;
    }
    assert(kahn_caught);

    // 3. Longest Path / Critical Path in Weighted DAG
    // 0 --(3)--> 1 --(4)--> 3
    // 0 --(2)--> 2 --(6)--> 3
    // Longest path: 0 -> 2 -> 3 with total length 2 + 6 = 8
    std::vector<std::vector<WeightedEdge>> weighted_dag(4);
    weighted_dag[0].push_back({1, 3});
    weighted_dag[1].push_back({3, 4});
    weighted_dag[0].push_back({2, 2});
    weighted_dag[2].push_back({3, 6});

    int longest = longest_path_dag(4, weighted_dag);
    assert(longest == 8);

    std::cout << "[PASS] All Topological Sort C++ unit tests passed.\n";
    return 0;
}
