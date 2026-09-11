/**
 * Reference Implementation: All-Pairs Shortest Paths (APSP)
 * Demonstrates:
 * 1. Floyd-Warshall Algorithm (O(V^3) Dense Dynamic Programming with In-Place Storage).
 * 2. Next-Hop Predecessor Matrix & Path Reconstruction.
 * 3. Negative Cycle Detection via Diagonal Invariant (dist[i][i] < 0).
 * 4. Johnson's Algorithm for Sparse Graphs with Potential Reweighting (O(V * E + V^2 log V)).
 * 5. Warshall's Transitive Closure with 64-bit Packed Bitset Acceleration (O(V^3 / 64)).
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <utility>
#include <algorithm>
#include <cstdint>
#include <cassert>

// ============================================================================
// Data Structures and Constants
// ============================================================================
constexpr long long INF = std::numeric_limits<long long>::max() / 4;

struct Edge {
    int to;
    long long weight;
};

struct DirectedEdge {
    int from;
    int to;
    long long weight;
};

struct FloydWarshallResult {
    std::vector<std::vector<long long>> dist;
    std::vector<std::vector<int>> next;
    bool has_negative_cycle;
};

struct JohnsonResult {
    std::vector<std::vector<long long>> dist;
    bool has_negative_cycle;
};

// ============================================================================
// 1. Floyd-Warshall Algorithm with Next-Hop Path Tracking
// ============================================================================
FloydWarshallResult floyd_warshall(
    int n,
    const std::vector<std::vector<long long>>& adj_matrix) {

    std::vector<std::vector<long long>> dist = adj_matrix;
    std::vector<std::vector<int>> next(n, std::vector<int>(n, -1));

    for (int i = 0; i < n; ++i) {
        dist[i][i] = std::min(dist[i][i], 0LL);
        next[i][i] = i;
        for (int j = 0; j < n; ++j) {
            if (i != j && adj_matrix[i][j] != INF) {
                next[i][j] = j;
            }
        }
    }

    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < n; ++j) {
                if (dist[k][j] == INF) continue;
                long long nd = dist[i][k] + dist[k][j];
                if (nd < dist[i][j]) {
                    dist[i][j] = nd;
                    next[i][j] = next[i][k];
                }
            }
        }
    }

    bool has_negative_cycle = false;
    for (int i = 0; i < n; ++i) {
        if (dist[i][i] < 0) {
            has_negative_cycle = true;
            break;
        }
    }

    return {dist, next, has_negative_cycle};
}

std::vector<int> reconstruct_path(
    int u,
    int v,
    const std::vector<std::vector<int>>& next) {

    if (next[u][v] == -1) {
        return {};
    }
    std::vector<int> path = {u};
    int curr = u;
    while (curr != v) {
        curr = next[curr][v];
        if (curr == -1 || path.size() > next.size()) {
            return {}; // Unreachable or trapped in negative cycle
        }
        path.push_back(curr);
    }
    return path;
}

// ============================================================================
// 2. Johnson's Algorithm for Sparse Graphs
// ============================================================================
static bool bellman_ford_potential(
    int n,
    const std::vector<DirectedEdge>& edges,
    std::vector<long long>& h) {

    h.assign(n + 1, INF);
    int super_source = n;
    h[super_source] = 0;

    std::vector<DirectedEdge> ext_edges = edges;
    for (int v = 0; v < n; ++v) {
        ext_edges.push_back({super_source, v, 0});
    }

    for (int i = 0; i < n; ++i) {
        bool changed = false;
        for (const auto& e : ext_edges) {
            if (h[e.from] == INF) continue;
            long long nd = h[e.from] + e.weight;
            if (nd < h[e.to]) {
                h[e.to] = nd;
                changed = true;
            }
        }
        if (!changed) break;
    }

    for (const auto& e : ext_edges) {
        if (h[e.from] == INF) continue;
        if (h[e.from] + e.weight < h[e.to]) {
            return false; // Negative cycle reachable
        }
    }

    h.pop_back(); // Remove super-source potential
    return true;
}

static std::vector<long long> dijkstra(
    int n,
    const std::vector<std::vector<Edge>>& graph,
    int source) {

    std::vector<long long> dist(n, INF);
    dist[source] = 0;

    using State = std::pair<long long, int>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        if (d > dist[u]) continue;

        for (const auto& e : graph[u]) {
            long long nd = d + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                pq.push({nd, e.to});
            }
        }
    }
    return dist;
}

JohnsonResult johnson_apsp(
    int n,
    const std::vector<DirectedEdge>& edges) {

    std::vector<long long> h;
    if (!bellman_ford_potential(n, edges, h)) {
        return {{}, true}; // Negative cycle detected
    }

    std::vector<std::vector<Edge>> reweighted(n);
    for (const auto& e : edges) {
        long long rw = e.weight + h[e.from] - h[e.to];
        reweighted[e.from].push_back({e.to, rw});
    }

    std::vector<std::vector<long long>> dist(n, std::vector<long long>(n, INF));

    for (int s = 0; s < n; ++s) {
        std::vector<long long> d = dijkstra(n, reweighted, s);
        for (int v = 0; v < n; ++v) {
            if (d[v] != INF) {
                dist[s][v] = d[v] - h[s] + h[v];
            }
        }
    }

    return {dist, false};
}

// ============================================================================
// 3. Warshall's Transitive Closure & Bitset-Optimized Version
// ============================================================================
std::vector<std::vector<bool>> warshall_transitive_closure(
    const std::vector<std::vector<bool>>& adj) {

    int n = static_cast<int>(adj.size());
    std::vector<std::vector<bool>> reach = adj;

    for (int i = 0; i < n; ++i) {
        reach[i][i] = true;
    }

    for (int k = 0; k < n; ++k) {
        for (int i = 0; i < n; ++i) {
            if (!reach[i][k]) continue;
            for (int j = 0; j < n; ++j) {
                reach[i][j] = reach[i][j] || reach[k][j];
            }
        }
    }

    return reach;
}

std::vector<std::vector<bool>> bitset_transitive_closure(
    const std::vector<std::vector<bool>>& adj) {

    int n = static_cast<int>(adj.size());
    int words = (n + 63) / 64;
    std::vector<std::vector<uint64_t>> bit_reach(n, std::vector<uint64_t>(words, 0ULL));

    for (int i = 0; i < n; ++i) {
        bit_reach[i][i / 64] |= (1ULL << (i % 64)); // Reflexive loop
        for (int j = 0; j < n; ++j) {
            if (adj[i][j]) {
                bit_reach[i][j / 64] |= (1ULL << (j % 64));
            }
        }
    }

    for (int k = 0; k < n; ++k) {
        int k_word = k / 64;
        uint64_t k_mask = (1ULL << (k % 64));
        for (int i = 0; i < n; ++i) {
            if (bit_reach[i][k_word] & k_mask) {
                for (int w = 0; w < words; ++w) {
                    bit_reach[i][w] |= bit_reach[k][w];
                }
            }
        }
    }

    std::vector<std::vector<bool>> reach(n, std::vector<bool>(n, false));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (bit_reach[i][j / 64] & (1ULL << (j % 64))) {
                reach[i][j] = true;
            }
        }
    }

    return reach;
}

// ============================================================================
// Unit Tests & Invariant Verification
// ============================================================================
int main() {
    // ------------------------------------------------------------------------
    // Test 1: Arthur's Worked 4-Vertex Graph
    // 0 -> 1 (3), 0 -> 2 (8), 1 -> 2 (2), 2 -> 3 (1), 1 -> 3 (7)
    // ------------------------------------------------------------------------
    {
        int n = 4;
        std::vector<std::vector<long long>> adj(n, std::vector<long long>(n, INF));
        adj[0][1] = 3;
        adj[0][2] = 8;
        adj[1][2] = 2;
        adj[2][3] = 1;
        adj[1][3] = 7;

        std::vector<DirectedEdge> edges = {
            {0, 1, 3}, {0, 2, 8}, {1, 2, 2}, {2, 3, 1}, {1, 3, 7}
        };

        auto fw = floyd_warshall(n, adj);
        auto jn = johnson_apsp(n, edges);

        assert(!fw.has_negative_cycle);
        assert(!jn.has_negative_cycle);

        // Optimal distances from 0:
        // 0 -> 0: 0
        // 0 -> 1: 3
        // 0 -> 2: 0 -> 1 -> 2 = 5
        // 0 -> 3: 0 -> 1 -> 2 -> 3 = 6
        assert(fw.dist[0][0] == 0 && jn.dist[0][0] == 0);
        assert(fw.dist[0][1] == 3 && jn.dist[0][1] == 3);
        assert(fw.dist[0][2] == 5 && jn.dist[0][2] == 5);
        assert(fw.dist[0][3] == 6 && jn.dist[0][3] == 6);

        // Verify full matrix equivalence between Floyd-Warshall and Johnson
        for (int u = 0; u < n; ++u) {
            for (int v = 0; v < n; ++v) {
                assert(fw.dist[u][v] == jn.dist[u][v]);
            }
        }

        // Verify Path Reconstruction
        std::vector<int> path_0_3 = reconstruct_path(0, 3, fw.next);
        std::vector<int> expected_0_3 = {0, 1, 2, 3};
        assert(path_0_3 == expected_0_3);

        std::vector<int> path_3_0 = reconstruct_path(3, 0, fw.next);
        assert(path_3_0.empty()); // Unreachable
    }

    // ------------------------------------------------------------------------
    // Test 2: Graph with Negative Edges (No Negative Cycles)
    // ------------------------------------------------------------------------
    {
        int n = 4;
        std::vector<std::vector<long long>> adj(n, std::vector<long long>(n, INF));
        adj[0][1] = 1;
        adj[1][2] = -2;
        adj[2][3] = 3;
        adj[0][3] = 10;

        std::vector<DirectedEdge> edges = {
            {0, 1, 1}, {1, 2, -2}, {2, 3, 3}, {0, 3, 10}
        };

        auto fw = floyd_warshall(n, adj);
        auto jn = johnson_apsp(n, edges);

        assert(!fw.has_negative_cycle);
        assert(!jn.has_negative_cycle);

        assert(fw.dist[0][3] == 2); // 1 - 2 + 3 = 2
        assert(jn.dist[0][3] == 2);
        assert(fw.dist == jn.dist);
    }

    // ------------------------------------------------------------------------
    // Test 3: Negative Cycle Detection
    // 1 -> 2 (-2), 2 -> 3 (-1), 3 -> 1 (-2) => Sum = -5
    // ------------------------------------------------------------------------
    {
        int n = 4;
        std::vector<std::vector<long long>> adj(n, std::vector<long long>(n, INF));
        adj[0][1] = 1;
        adj[1][2] = -2;
        adj[2][3] = -1;
        adj[3][1] = -2;

        std::vector<DirectedEdge> edges = {
            {0, 1, 1}, {1, 2, -2}, {2, 3, -1}, {3, 1, -2}
        };

        auto fw = floyd_warshall(n, adj);
        auto jn = johnson_apsp(n, edges);

        assert(fw.has_negative_cycle);
        assert(jn.has_negative_cycle);
    }

    // ------------------------------------------------------------------------
    // Test 4: Transitive Closure (Standard vs. 64-bit Packed Bitset)
    // ------------------------------------------------------------------------
    {
        int n = 5;
        std::vector<std::vector<bool>> adj(n, std::vector<bool>(n, false));
        adj[0][1] = true;
        adj[1][2] = true;
        adj[2][3] = true;
        adj[4][0] = true; // 4 -> 0 -> 1 -> 2 -> 3

        auto reach_std = warshall_transitive_closure(adj);
        auto reach_bit = bitset_transitive_closure(adj);

        assert(reach_std == reach_bit);
        assert(reach_std[4][3] == true);
        assert(reach_std[3][0] == false);
        assert(reach_std[2][2] == true); // Reflexive
    }

    std::cout << "[PASS] All All-Pairs Shortest Paths C++ unit tests passed.\n";
    return 0;
}
