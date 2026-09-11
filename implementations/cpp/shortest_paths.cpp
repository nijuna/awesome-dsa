/**
 * Reference Implementation: Single-Source Shortest Paths (SSSP)
 * Demonstrates:
 * 1. Dijkstra's Algorithm (Min-Priority Queue with Stale Pair Pruning).
 * 2. DAG Shortest & Longest Paths via Topological Relaxation (O(V + E)).
 * 3. Bellman-Ford Algorithm with Early-Stopping & Negative Cycle Detection (O(V * E)).
 * 4. Shortest Path Faster Algorithm (SPFA) with Queue-Based Relaxation.
 * 5. Path Reconstruction via Parent Predecessor Pointers.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <utility>
#include <algorithm>
#include <cassert>

// ============================================================================
// Data Structures and Constants
// ============================================================================
struct Edge {
    int to;
    long long weight;
};

struct DirectedEdge {
    int from;
    int to;
    long long weight;
};

constexpr long long INF = std::numeric_limits<long long>::max() / 4;
constexpr long long NEG_INF = std::numeric_limits<long long>::min() / 4;

// ============================================================================
// 1. Dijkstra's Algorithm (Non-Negative Edge Weights)
// ============================================================================
std::vector<long long> dijkstra(
    const std::vector<std::vector<Edge>>& graph,
    int source,
    std::vector<int>* parent_out = nullptr) {

    int n = static_cast<int>(graph.size());
    std::vector<long long> dist(n, INF);
    std::vector<int> parent(n, -1);

    using State = std::pair<long long, int>; // {tentative_dist, vertex}
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    dist[source] = 0;
    pq.push({0, source});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();

        // Prune stale priority queue entries
        if (d > dist[u]) {
            continue;
        }

        for (const auto& e : graph[u]) {
            int v = e.to;
            long long nd = d + e.weight;
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
                pq.push({nd, v});
            }
        }
    }

    if (parent_out) {
        *parent_out = std::move(parent);
    }
    return dist;
}

// ============================================================================
// 2. Path Reconstruction Helper
// ============================================================================
std::vector<int> reconstruct_path(int source, int target, const std::vector<int>& parent) {
    if (source == target) {
        return {source};
    }
    if (parent[target] == -1) {
        return {}; // Target is unreachable
    }

    std::vector<int> path;
    for (int v = target; v != -1; v = parent[v]) {
        path.push_back(v);
    }
    std::reverse(path.begin(), path.end());
    if (path.empty() || path.front() != source) {
        return {};
    }
    return path;
}

// ============================================================================
// 3. DAG Shortest & Longest Paths via Topological Relaxation
// ============================================================================
std::vector<long long> dag_shortest_paths(
    const std::vector<std::vector<Edge>>& graph,
    const std::vector<int>& topo_order,
    int source,
    std::vector<int>* parent_out = nullptr) {

    int n = static_cast<int>(graph.size());
    std::vector<long long> dist(n, INF);
    std::vector<int> parent(n, -1);
    dist[source] = 0;

    for (int u : topo_order) {
        if (dist[u] == INF) {
            continue;
        }
        for (const auto& e : graph[u]) {
            int v = e.to;
            long long nd = dist[u] + e.weight;
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;
            }
        }
    }

    if (parent_out) {
        *parent_out = std::move(parent);
    }
    return dist;
}

std::vector<long long> dag_longest_paths(
    const std::vector<std::vector<Edge>>& graph,
    const std::vector<int>& topo_order,
    int source,
    std::vector<int>* parent_out = nullptr) {

    int n = static_cast<int>(graph.size());
    std::vector<long long> dist(n, NEG_INF);
    std::vector<int> parent(n, -1);
    dist[source] = 0;

    for (int u : topo_order) {
        if (dist[u] == NEG_INF) {
            continue;
        }
        for (const auto& e : graph[u]) {
            int v = e.to;
            long long nd = dist[u] + e.weight;
            if (nd > dist[v]) {
                dist[v] = nd;
                parent[v] = u;
            }
        }
    }

    if (parent_out) {
        *parent_out = std::move(parent);
    }
    return dist;
}

// ============================================================================
// 4. Bellman-Ford Algorithm (Arbitrary Weights & Negative Cycle Detection)
// ============================================================================
bool bellman_ford(
    int n,
    const std::vector<DirectedEdge>& edges,
    int source,
    std::vector<long long>& dist,
    std::vector<int>& parent) {

    dist.assign(n, INF);
    parent.assign(n, -1);
    dist[source] = 0;

    // Up to (n - 1) relaxation rounds
    for (int iter = 0; iter < n - 1; ++iter) {
        bool any_update = false;
        for (const auto& e : edges) {
            if (dist[e.from] == INF) {
                continue;
            }
            long long nd = dist[e.from] + e.weight;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                parent[e.to] = e.from;
                any_update = true;
            }
        }
        if (!any_update) {
            break; // Early convergence
        }
    }

    // n-th check pass for negative cycles reachable from source
    for (const auto& e : edges) {
        if (dist[e.from] == INF) {
            continue;
        }
        if (dist[e.from] + e.weight < dist[e.to]) {
            return false; // Reachable negative cycle exists
        }
    }

    return true;
}

// ============================================================================
// 5. Shortest Path Faster Algorithm (SPFA - Queue-Based Bellman-Ford)
// ============================================================================
bool spfa(
    const std::vector<std::vector<Edge>>& graph,
    int source,
    std::vector<long long>& dist,
    std::vector<int>& parent) {

    int n = static_cast<int>(graph.size());
    dist.assign(n, INF);
    parent.assign(n, -1);

    std::vector<int> in_queue(n, 0);
    std::vector<int> relax_count(n, 0);
    std::queue<int> q;

    dist[source] = 0;
    q.push(source);
    in_queue[source] = 1;

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        in_queue[u] = 0;

        for (const auto& e : graph[u]) {
            int v = e.to;
            if (dist[u] == INF) continue;

            long long nd = dist[u] + e.weight;
            if (nd < dist[v]) {
                dist[v] = nd;
                parent[v] = u;

                if (!in_queue[v]) {
                    q.push(v);
                    in_queue[v] = 1;
                    ++relax_count[v];
                    if (relax_count[v] >= n) {
                        return false; // Reachable negative cycle detected
                    }
                }
            }
        }
    }

    return true;
}

// ============================================================================
// Unit Tests & Verification
// ============================================================================
int main() {
    // ------------------------------------------------------------------------
    // Test 1: Dijkstra on Non-Negative Weighted Graph
    // 0 --(4)--> 1, 0 --(1)--> 2, 2 --(2)--> 1, 1 --(1)--> 3, 2 --(5)--> 3
    // ------------------------------------------------------------------------
    {
        std::vector<std::vector<Edge>> g(4);
        g[0].push_back({1, 4});
        g[0].push_back({2, 1});
        g[2].push_back({1, 2});
        g[1].push_back({3, 1});
        g[2].push_back({3, 5});

        std::vector<int> parent;
        std::vector<long long> dist = dijkstra(g, 0, &parent);

        assert(dist[0] == 0);
        assert(dist[1] == 3); // 0 -> 2 -> 1 (1 + 2 = 3)
        assert(dist[2] == 1);
        assert(dist[3] == 4); // 0 -> 2 -> 1 -> 3 (1 + 2 + 1 = 4)

        std::vector<int> path_to_3 = reconstruct_path(0, 3, parent);
        std::vector<int> expected_path = {0, 2, 1, 3};
        assert(path_to_3 == expected_path);

        // Path to unreachable node in 5-node graph
        g.resize(5);
        dist = dijkstra(g, 0, &parent);
        assert(dist[4] == INF);
        assert(reconstruct_path(0, 4, parent).empty());
    }

    // ------------------------------------------------------------------------
    // Test 2: DAG Shortest and Longest Paths with Negative Weights
    // Topo order: 0, 1, 2, 3
    // 0 --(3)--> 1, 0 --(2)--> 2, 1 --(-4)--> 2, 2 --(5)--> 3
    // ------------------------------------------------------------------------
    {
        std::vector<std::vector<Edge>> g(4);
        g[0].push_back({1, 3});
        g[0].push_back({2, 2});
        g[1].push_back({2, -4});
        g[2].push_back({3, 5});

        std::vector<int> topo = {0, 1, 2, 3};
        std::vector<int> parent_sp, parent_lp;

        std::vector<long long> sp = dag_shortest_paths(g, topo, 0, &parent_sp);
        assert(sp[0] == 0);
        assert(sp[1] == 3);
        assert(sp[2] == -1); // 0 -> 1 -> 2: 3 + (-4) = -1 < 2
        assert(sp[3] == 4);  // -1 + 5 = 4
        assert((reconstruct_path(0, 3, parent_sp) == std::vector<int>{0, 1, 2, 3}));

        std::vector<long long> lp = dag_longest_paths(g, topo, 0, &parent_lp);
        assert(lp[0] == 0);
        assert(lp[1] == 3);
        assert(lp[2] == 2); // 0 -> 2 direct: 2 > -1
        assert(lp[3] == 7); // 2 + 5 = 7
        assert((reconstruct_path(0, 3, parent_lp) == std::vector<int>{0, 2, 3}));
    }

    // ------------------------------------------------------------------------
    // Test 3: Bellman-Ford & SPFA on Arbitrary Weights (No Negative Cycle)
    // ------------------------------------------------------------------------
    {
        std::vector<DirectedEdge> edges = {
            {0, 1, -1},
            {0, 2, 4},
            {1, 2, 3},
            {1, 3, 2},
            {1, 4, 2},
            {3, 2, 5},
            {3, 1, 1},
            {4, 3, -3}
        };
        int n = 5;
        std::vector<long long> dist_bf;
        std::vector<int> parent_bf;
        bool ok_bf = bellman_ford(n, edges, 0, dist_bf, parent_bf);
        assert(ok_bf);
        assert(dist_bf[0] == 0);
        assert(dist_bf[1] == -1);
        assert(dist_bf[2] == 2);
        assert(dist_bf[3] == -2); // 0 -> 1 -> 4 -> 3: -1 + 2 + (-3) = -2
        assert(dist_bf[4] == 1);

        // Convert to adjacency list and test SPFA
        std::vector<std::vector<Edge>> adj(n);
        for (const auto& e : edges) {
            adj[e.from].push_back({e.to, e.weight});
        }
        std::vector<long long> dist_spfa;
        std::vector<int> parent_spfa;
        bool ok_spfa = spfa(adj, 0, dist_spfa, parent_spfa);
        assert(ok_spfa);
        assert(dist_bf == dist_spfa);
    }

    // ------------------------------------------------------------------------
    // Test 4: Negative Cycle Detection (Bellman-Ford & SPFA)
    // Cycle: 1 -> 2 (-2), 2 -> 3 (-1), 3 -> 1 (-2) => Sum = -5
    // ------------------------------------------------------------------------
    {
        int n = 4;
        std::vector<DirectedEdge> edges = {
            {0, 1, 1},
            {1, 2, -2},
            {2, 3, -1},
            {3, 1, -2}
        };

        std::vector<long long> dist_bf;
        std::vector<int> parent_bf;
        bool ok_bf = bellman_ford(n, edges, 0, dist_bf, parent_bf);
        assert(!ok_bf); // Must detect reachable negative cycle

        std::vector<std::vector<Edge>> adj(n);
        for (const auto& e : edges) {
            adj[e.from].push_back({e.to, e.weight});
        }
        std::vector<long long> dist_spfa;
        std::vector<int> parent_spfa;
        bool ok_spfa = spfa(adj, 0, dist_spfa, parent_spfa);
        assert(!ok_spfa); // SPFA must also detect reachable negative cycle
    }

    std::cout << "[PASS] All Shortest Paths C++ unit tests passed.\n";
    return 0;
}
