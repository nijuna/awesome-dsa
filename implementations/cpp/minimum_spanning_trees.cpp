/**
 * Reference Implementation: Minimum Spanning Trees (MST)
 * Demonstrates:
 * 1. Kruskal's Algorithm (Global Edge Sorting + Disjoint Set Union).
 * 2. Prim's Algorithm (Frontier Expansion with Min-Priority Queue).
 * 3. Borůvka's Algorithm (Concurrent Component Contraction).
 * 4. Maximum Spanning Tree & Minimum Bottleneck Spanning Tree verification.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <queue>
#include <limits>
#include <utility>
#include <cassert>

// ============================================================================
// Data Structures
// ============================================================================
struct Edge {
    int u;
    int v;
    long long weight;
};

struct AdjEdge {
    int to;
    long long weight;
};

struct MstResult {
    long long total_weight;
    std::vector<Edge> edges;
    bool is_connected;
};

struct DSU {
    std::vector<int> parent;
    std::vector<int> rank;

    explicit DSU(int n) : parent(n), rank(n, 0) {
        std::iota(parent.begin(), parent.end(), 0);
    }

    int find(int x) {
        if (parent[x] != x) {
            parent[x] = find(parent[x]);
        }
        return parent[x];
    }

    bool unite(int a, int b) {
        a = find(a);
        b = find(b);
        if (a == b) return false;
        if (rank[a] < rank[b]) std::swap(a, b);
        parent[b] = a;
        if (rank[a] == rank[b]) ++rank[a];
        return true;
    }
};

// ============================================================================
// 1. Kruskal's Algorithm
// ============================================================================
MstResult kruskal_mst(int n, std::vector<Edge> edges) {
    if (n <= 1) {
        return {0, {}, true};
    }

    std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
        return a.weight < b.weight;
    });

    DSU dsu(n);
    long long total_weight = 0;
    std::vector<Edge> mst;
    mst.reserve(n - 1);

    for (const auto& e : edges) {
        if (dsu.unite(e.u, e.v)) {
            total_weight += e.weight;
            mst.push_back(e);
            if (static_cast<int>(mst.size()) == n - 1) {
                break;
            }
        }
    }

    bool connected = (static_cast<int>(mst.size()) == n - 1);
    return {total_weight, mst, connected};
}

// ============================================================================
// 2. Prim's Algorithm
// ============================================================================
MstResult prim_mst(int n, const std::vector<std::vector<AdjEdge>>& graph, int start = 0) {
    if (n <= 1) {
        return {0, {}, true};
    }

    std::vector<int> visited(n, 0);
    std::vector<long long> best(n, std::numeric_limits<long long>::max());
    std::vector<int> parent(n, -1);

    using State = std::pair<long long, int>; // {edge_weight, vertex}
    std::priority_queue<State, std::vector<State>, std::greater<State>> pq;

    best[start] = 0;
    pq.push({0, start});

    long long total_weight = 0;
    std::vector<Edge> mst;
    mst.reserve(n - 1);

    while (!pq.empty()) {
        auto [w, u] = pq.top();
        pq.pop();

        if (visited[u]) {
            continue; // Stale heap entry
        }
        visited[u] = 1;

        if (parent[u] != -1) {
            total_weight += w;
            mst.push_back({parent[u], u, w});
        }

        for (const auto& e : graph[u]) {
            int v = e.to;
            if (!visited[v] && e.weight < best[v]) {
                best[v] = e.weight;
                parent[v] = u;
                pq.push({best[v], v});
            }
        }
    }

    bool connected = (static_cast<int>(mst.size()) == n - 1);
    return {total_weight, mst, connected};
}

// ============================================================================
// 3. Borůvka's Algorithm
// ============================================================================
MstResult boruvka_mst(int n, const std::vector<Edge>& edges) {
    if (n <= 1) {
        return {0, {}, true};
    }

    DSU dsu(n);
    int components = n;
    long long total_weight = 0;
    std::vector<Edge> mst;
    mst.reserve(n - 1);

    while (components > 1) {
        std::vector<int> best_edge(n, -1);

        for (int i = 0; i < static_cast<int>(edges.size()); ++i) {
            int u = dsu.find(edges[i].u);
            int v = dsu.find(edges[i].v);
            if (u == v) continue;

            if (best_edge[u] == -1 || edges[i].weight < edges[best_edge[u]].weight) {
                best_edge[u] = i;
            }
            if (best_edge[v] == -1 || edges[i].weight < edges[best_edge[v]].weight) {
                best_edge[v] = i;
            }
        }

        bool merged_any = false;

        for (int i = 0; i < n; ++i) {
            int ei = best_edge[i];
            if (ei == -1) continue;

            const auto& e = edges[ei];
            if (dsu.unite(e.u, e.v)) {
                total_weight += e.weight;
                mst.push_back(e);
                --components;
                merged_any = true;
            }
        }

        if (!merged_any) {
            break;
        }
    }

    bool connected = (static_cast<int>(mst.size()) == n - 1);
    return {total_weight, mst, connected};
}

// ============================================================================
// 4. Maximum Spanning Tree
// ============================================================================
MstResult maximum_spanning_tree(int n, std::vector<Edge> edges) {
    for (auto& e : edges) {
        e.weight = -e.weight;
    }
    MstResult res = kruskal_mst(n, edges);
    res.total_weight = -res.total_weight;
    for (auto& e : res.edges) {
        e.weight = -e.weight;
    }
    return res;
}

// ============================================================================
// Unit Tests & Invariant Verification
// ============================================================================
int main() {
    // ------------------------------------------------------------------------
    // Test 1: Standard 4-Vertex Graph (Arthur's Example)
    // A(0), B(1), C(2), D(3)
    // AB=1, BC=2, CD=3, AC=4, BD=5
    // ------------------------------------------------------------------------
    {
        int n = 4;
        std::vector<Edge> edges = {
            {0, 1, 1},
            {1, 2, 2},
            {2, 3, 3},
            {0, 2, 4},
            {1, 3, 5}
        };

        // Adjacency representation for Prim
        std::vector<std::vector<AdjEdge>> adj(n);
        for (const auto& e : edges) {
            adj[e.u].push_back({e.v, e.weight});
            adj[e.v].push_back({e.u, e.weight});
        }

        MstResult res_k = kruskal_mst(n, edges);
        MstResult res_p = prim_mst(n, adj, 0);
        MstResult res_b = boruvka_mst(n, edges);

        assert(res_k.is_connected);
        assert(res_p.is_connected);
        assert(res_b.is_connected);

        assert(res_k.total_weight == 6);
        assert(res_p.total_weight == 6);
        assert(res_b.total_weight == 6);

        assert(res_k.edges.size() == 3);
        assert(res_p.edges.size() == 3);
        assert(res_b.edges.size() == 3);

        // Maximum Spanning Tree: edges with weights 5, 4, 3 => weight = 12
        MstResult res_max = maximum_spanning_tree(n, edges);
        assert(res_max.is_connected);
        assert(res_max.total_weight == 12);
    }

    // ------------------------------------------------------------------------
    // Test 2: Disconnected Graph
    // ------------------------------------------------------------------------
    {
        int n = 5;
        // Component 1: {0, 1}, Component 2: {2, 3, 4}
        std::vector<Edge> edges = {
            {0, 1, 10},
            {2, 3, 5},
            {3, 4, 7},
            {2, 4, 8}
        };

        std::vector<std::vector<AdjEdge>> adj(n);
        for (const auto& e : edges) {
            adj[e.u].push_back({e.v, e.weight});
            adj[e.v].push_back({e.u, e.weight});
        }

        MstResult res_k = kruskal_mst(n, edges);
        MstResult res_p = prim_mst(n, adj, 0);
        MstResult res_b = boruvka_mst(n, edges);

        assert(!res_k.is_connected);
        assert(!res_p.is_connected);
        assert(!res_b.is_connected);
    }

    // ------------------------------------------------------------------------
    // Test 3: Trivial Single-Vertex Graph
    // ------------------------------------------------------------------------
    {
        int n = 1;
        std::vector<Edge> edges = {};
        std::vector<std::vector<AdjEdge>> adj(1);

        MstResult res_k = kruskal_mst(n, edges);
        MstResult res_p = prim_mst(n, adj, 0);
        MstResult res_b = boruvka_mst(n, edges);

        assert(res_k.is_connected && res_k.total_weight == 0 && res_k.edges.empty());
        assert(res_p.is_connected && res_p.total_weight == 0 && res_p.edges.empty());
        assert(res_b.is_connected && res_b.total_weight == 0 && res_b.edges.empty());
    }

    // ------------------------------------------------------------------------
    // Test 4: Complete Graph K4 with Non-Distinct Weights
    // ------------------------------------------------------------------------
    {
        int n = 4;
        std::vector<Edge> edges = {
            {0, 1, 2}, {0, 2, 2}, {0, 3, 3},
            {1, 2, 2}, {1, 3, 4}, {2, 3, 2}
        };
        std::vector<std::vector<AdjEdge>> adj(n);
        for (const auto& e : edges) {
            adj[e.u].push_back({e.v, e.weight});
            adj[e.v].push_back({e.u, e.weight});
        }

        MstResult res_k = kruskal_mst(n, edges);
        MstResult res_p = prim_mst(n, adj, 0);
        MstResult res_b = boruvka_mst(n, edges);

        assert(res_k.is_connected && res_p.is_connected && res_b.is_connected);
        assert(res_k.total_weight == 6); // 3 edges of weight 2
        assert(res_p.total_weight == 6);
        assert(res_b.total_weight == 6);
    }

    std::cout << "[PASS] All Minimum Spanning Tree C++ unit tests passed.\n";
    return 0;
}
