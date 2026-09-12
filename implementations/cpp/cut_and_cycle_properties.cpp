/**
 * @file cut_and_cycle_properties.cpp
 * @brief Algorithmic validation of the Cut Property and Cycle Property for Spanning Trees.
 *
 * Implements Kruskal's (DSU), Prim's (Priority Queue), and Reverse-Delete algorithms,
 * asserting identical total weights and verifying the fundamental Cut & Cycle properties.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <queue>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <numeric>

namespace dsa {

struct Edge {
    int u;
    int v;
    int weight;
};

class DSU {
private:
    std::vector<int> parent_;
    std::vector<int> rank_;

public:
    explicit DSU(int n) : parent_(n), rank_(n, 0) {
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    int find(int i) {
        if (parent_[i] == i) return i;
        return parent_[i] = find(parent_[i]);
    }

    bool unite(int i, int j) {
        int root_i = find(i);
        int root_j = find(j);
        if (root_i == root_j) return false;

        if (rank_[root_i] < rank_[root_j]) {
            parent_[root_i] = root_j;
        } else if (rank_[root_i] > rank_[root_j]) {
            parent_[root_j] = root_i;
        } else {
            parent_[root_j] = root_i;
            rank_[root_i]++;
        }
        return true;
    }
};

/**
 * @brief Kruskal's MST (Leverages Cut & Cycle properties via DSU).
 */
inline int kruskal_mst(int num_vertices, std::vector<Edge> edges) {
    std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.weight < b.weight;
    });

    DSU dsu(num_vertices);
    int total_weight = 0;
    int edges_count = 0;

    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            total_weight += e.weight;
            edges_count++;
            if (edges_count == num_vertices - 1) break;
        }
    }
    return total_weight;
}

/**
 * @brief Prim's MST (Direct implementation of the Cut Property).
 */
inline int prim_mst(int num_vertices, const std::vector<Edge>& edges) {
    std::vector<std::vector<std::pair<int, int>>> adj(num_vertices);
    for (const auto& e : edges) {
        adj[e.u].push_back({e.v, e.weight});
        adj[e.v].push_back({e.u, e.weight});
    }

    std::vector<bool> in_cut(num_vertices, false);
    // Min-heap: {weight, vertex}
    std::priority_queue<std::pair<int, int>,
                        std::vector<std::pair<int, int>>,
                        std::greater<std::pair<int, int>>> pq;

    pq.push({0, 0});
    int total_weight = 0;
    int visited_count = 0;

    while (!pq.empty() && visited_count < num_vertices) {
        auto [w, u] = pq.top();
        pq.pop();

        if (in_cut[u]) continue;
        in_cut[u] = true;
        total_weight += w;
        visited_count++;

        for (const auto& [v, weight] : adj[u]) {
            if (!in_cut[v]) {
                pq.push({weight, v});
            }
        }
    }
    return total_weight;
}

/**
 * @brief Reverse-Delete MST (Direct implementation of the Cycle Property).
 * Sorts edges descending and deletes the heaviest edge if it belongs to a cycle.
 */
inline int reverse_delete_mst(int num_vertices, std::vector<Edge> edges) {
    // Sort edges in descending order of weight
    std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.weight > b.weight;
    });

    auto is_connected = [&](const std::vector<bool>& active) {
        DSU dsu(num_vertices);
        int components = num_vertices;
        for (size_t i = 0; i < edges.size(); ++i) {
            if (active[i]) {
                if (dsu.unite(edges[i].u, edges[i].v)) {
                    components--;
                }
            }
        }
        return components == 1;
    };

    std::vector<bool> active(edges.size(), true);

    for (size_t i = 0; i < edges.size(); ++i) {
        // Temporarily delete heaviest edge
        active[i] = false;
        // If graph becomes disconnected, this edge was a bridge (not part of a cycle) -> restore it
        if (!is_connected(active)) {
            active[i] = true;
        }
    }

    int total_weight = 0;
    for (size_t i = 0; i < edges.size(); ++i) {
        if (active[i]) total_weight += edges[i].weight;
    }
    return total_weight;
}

} // namespace dsa

int main() {
    std::cout << "Running Cut & Cycle Properties MST verification..." << std::endl;

    // Classic 6-vertex connected graph with distinct weights
    const int V = 6;
    std::vector<dsa::Edge> edges = {
        {0, 1, 4},
        {0, 2, 2},
        {1, 2, 1},
        {1, 3, 5},
        {2, 3, 8},
        {2, 4, 10},
        {3, 4, 2},
        {3, 5, 6},
        {4, 5, 3}
    };

    int kruskal_weight = dsa::kruskal_mst(V, edges);
    int prim_weight = dsa::prim_mst(V, edges);
    int rev_del_weight = dsa::reverse_delete_mst(V, edges);

    assert(kruskal_weight == prim_weight);
    assert(prim_weight == rev_del_weight);
    assert(kruskal_weight == 13); // (1,2):1 + (0,2):2 + (3,4):2 + (4,5):3 + (1,3):5 = 13

    std::cout << "[PASS] Cut Property (Prim) and Cycle Property (Reverse-Delete) matched Kruskal MST weight: "
              << kruskal_weight << "." << std::endl;
    std::cout << "All Cut & Cycle Properties assertions passed successfully!" << std::endl;
    return 0;
}
