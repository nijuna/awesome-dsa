/**
 * Reference Implementation: Graph Representations & Memory Layouts
 * Demonstrates:
 * 1. Adjacency Matrix (O(1) Edge Lookup, O(V^2) Storage).
 * 2. Adjacency List (Vector-of-Vectors, O(V + E) Sparse Storage).
 * 3. Edge List (Flat Array of Edges, O(E) Storage).
 * 4. Compressed Sparse Row (CSR / Forward Star, Contiguous Cache-Optimized O(V + E)).
 * 5. Implicit Grid Graph (Zero-Allocation Neighborhood Traversals).
 * 6. Inter-Representation Conversion Pipelines & Traversal Parity Verification.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <queue>
#include <limits>
#include <algorithm>
#include <utility>
#include <cassert>

constexpr long long NO_EDGE = std::numeric_limits<long long>::max() / 4;

// ============================================================================
// 1. Basic Element Structures
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

// ============================================================================
// 2. Adjacency Matrix Representation
// ============================================================================
class AdjacencyMatrix {
public:
    int n;
    std::vector<std::vector<long long>> mat;

    explicit AdjacencyMatrix(int vertices)
        : n(vertices), mat(vertices, std::vector<long long>(vertices, NO_EDGE)) {
        for (int i = 0; i < n; ++i) {
            mat[i][i] = 0;
        }
    }

    void add_edge(int u, int v, long long weight = 1, bool directed = true) {
        mat[u][v] = weight;
        if (!directed) {
            mat[v][u] = weight;
        }
    }

    bool has_edge(int u, int v) const {
        return mat[u][v] != NO_EDGE && u != v;
    }

    std::vector<int> get_neighbors(int u) const {
        std::vector<int> neighbors;
        for (int v = 0; v < n; ++v) {
            if (u != v && mat[u][v] != NO_EDGE) {
                neighbors.push_back(v);
            }
        }
        return neighbors;
    }
};

// ============================================================================
// 3. Adjacency List Representation
// ============================================================================
class AdjacencyList {
public:
    int n;
    std::vector<std::vector<AdjEdge>> adj;

    explicit AdjacencyList(int vertices) : n(vertices), adj(vertices) {}

    void add_edge(int u, int v, long long weight = 1, bool directed = true) {
        adj[u].push_back({v, weight});
        if (!directed) {
            adj[v].push_back({u, weight});
        }
    }

    bool has_edge(int u, int v) const {
        for (const auto& e : adj[u]) {
            if (e.to == v) return true;
        }
        return false;
    }

    const std::vector<AdjEdge>& get_neighbors(int u) const {
        return adj[u];
    }
};

// ============================================================================
// 4. Edge List Representation
// ============================================================================
class EdgeList {
public:
    int n;
    std::vector<Edge> edges;

    explicit EdgeList(int vertices) : n(vertices) {}

    void add_edge(int u, int v, long long weight = 1) {
        edges.push_back({u, v, weight});
    }
};

// ============================================================================
// 5. Compressed Sparse Row (CSR / Forward Star)
// ============================================================================
class CSRGraph {
public:
    int n;
    std::vector<int> offsets;
    std::vector<int> to;
    std::vector<long long> weights;

    CSRGraph() : n(0) {}

    static CSRGraph from_edges(int vertices, const std::vector<Edge>& edge_list, bool directed = true) {
        CSRGraph g;
        g.n = vertices;
        g.offsets.assign(vertices + 1, 0);

        std::vector<Edge> all_edges;
        for (const auto& e : edge_list) {
            all_edges.push_back(e);
            if (!directed) {
                all_edges.push_back({e.v, e.u, e.weight});
            }
        }

        // Pass 1: Count outgoing degree for each vertex
        for (const auto& e : all_edges) {
            ++g.offsets[e.u + 1];
        }

        // Pass 2: Prefix sum to compute offset boundaries
        for (int i = 1; i <= vertices; ++i) {
            g.offsets[i] += g.offsets[i - 1];
        }

        // Pass 3: Place edges contiguously
        int total_edges = static_cast<int>(all_edges.size());
        g.to.assign(total_edges, 0);
        g.weights.assign(total_edges, 0);
        std::vector<int> cur_head = g.offsets;

        for (const auto& e : all_edges) {
            int pos = cur_head[e.u]++;
            g.to[pos] = e.v;
            g.weights[pos] = e.weight;
        }

        return g;
    }

    int degree(int u) const {
        return offsets[u + 1] - offsets[u];
    }

    std::pair<int, int> neighbor_range(int u) const {
        return {offsets[u], offsets[u + 1]};
    }
};

// ============================================================================
// 6. Implicit Grid Graph Representation
// ============================================================================
class ImplicitGrid {
public:
    int rows;
    int cols;

    ImplicitGrid(int r, int c) : rows(r), cols(c) {}

    std::vector<std::pair<int, int>> get_neighbors4(int r, int c) const {
        static const int dr[4] = {-1, 1, 0, 0};
        static const int dc[4] = {0, 0, -1, 1};

        std::vector<std::pair<int, int>> neighbors;
        for (int k = 0; k < 4; ++k) {
            int nr = r + dr[k];
            int nc = c + dc[k];
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                neighbors.push_back({nr, nc});
            }
        }
        return neighbors;
    }

    std::vector<std::pair<int, int>> get_neighbors8(int r, int c) const {
        static const int dr[8] = {-1, -1, -1, 0, 0, 1, 1, 1};
        static const int dc[8] = {-1, 0, 1, -1, 1, -1, 0, 1};

        std::vector<std::pair<int, int>> neighbors;
        for (int k = 0; k < 8; ++k) {
            int nr = r + dr[k];
            int nc = c + dc[k];
            if (nr >= 0 && nr < rows && nc >= 0 && nc < cols) {
                neighbors.push_back({nr, nc});
            }
        }
        return neighbors;
    }
};

// ============================================================================
// 7. Inter-Representation Conversion Pipelines
// ============================================================================
AdjacencyList edge_list_to_adj_list(const EdgeList& el, bool directed = true) {
    AdjacencyList al(el.n);
    for (const auto& e : el.edges) {
        al.add_edge(e.u, e.v, e.weight, directed);
    }
    return al;
}

EdgeList adj_list_to_edge_list(const AdjacencyList& al) {
    EdgeList el(al.n);
    for (int u = 0; u < al.n; ++u) {
        for (const auto& e : al.adj[u]) {
            el.add_edge(u, e.to, e.weight);
        }
    }
    return el;
}

AdjacencyMatrix adj_list_to_adj_matrix(const AdjacencyList& al) {
    AdjacencyMatrix mat(al.n);
    for (int u = 0; u < al.n; ++u) {
        for (const auto& e : al.adj[u]) {
            mat.add_edge(u, e.to, e.weight, true);
        }
    }
    return mat;
}

AdjacencyList adj_matrix_to_adj_list(const AdjacencyMatrix& mat) {
    AdjacencyList al(mat.n);
    for (int u = 0; u < mat.n; ++u) {
        for (int v = 0; v < mat.n; ++v) {
            if (u != v && mat.mat[u][v] != NO_EDGE) {
                al.add_edge(u, v, mat.mat[u][v], true);
            }
        }
    }
    return al;
}

// ============================================================================
// Unit Tests & Traversal Parity Verification
// ============================================================================
int main() {
    // ------------------------------------------------------------------------
    // Test 1: Worked Example from Chapter
    // Directed graph: 0 -> 1, 0 -> 3, 1 -> 2, 2 -> 3
    // ------------------------------------------------------------------------
    int n = 4;
    EdgeList el(n);
    el.add_edge(0, 1, 10);
    el.add_edge(0, 3, 40);
    el.add_edge(1, 2, 20);
    el.add_edge(2, 3, 30);

    // Conversions
    AdjacencyList al = edge_list_to_adj_list(el, true);
    AdjacencyMatrix mat = adj_list_to_adj_matrix(al);
    CSRGraph csr = CSRGraph::from_edges(n, el.edges, true);

    // Verify Edge Lookups
    assert(mat.has_edge(0, 1) && mat.has_edge(0, 3));
    assert(mat.has_edge(1, 2) && mat.has_edge(2, 3));
    assert(!mat.has_edge(3, 0) && !mat.has_edge(0, 2));

    assert(al.has_edge(0, 1) && al.has_edge(0, 3));
    assert(al.has_edge(1, 2) && al.has_edge(2, 3));
    assert(!al.has_edge(3, 0) && !al.has_edge(0, 2));

    // Verify CSR offsets and neighbor bounds
    // 0: [1, 3] (deg 2)
    // 1: [2]    (deg 1)
    // 2: [3]    (deg 1)
    // 3: []     (deg 0)
    assert(csr.degree(0) == 2);
    assert(csr.degree(1) == 1);
    assert(csr.degree(2) == 1);
    assert(csr.degree(3) == 0);
    assert(csr.offsets[0] == 0 && csr.offsets[1] == 2 && csr.offsets[2] == 3 && csr.offsets[3] == 4 && csr.offsets[4] == 4);

    // Verify BFS Traversal Parity across Matrix, AdjList, and CSR
    auto bfs_matrix = [&](int src) {
        std::vector<int> order;
        std::vector<bool> vis(n, false);
        std::queue<int> q;
        q.push(src);
        vis[src] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order.push_back(u);
            for (int v : mat.get_neighbors(u)) {
                if (!vis[v]) {
                    vis[v] = true;
                    q.push(v);
                }
            }
        }
        return order;
    };

    auto bfs_adj_list = [&](int src) {
        std::vector<int> order;
        std::vector<bool> vis(n, false);
        std::queue<int> q;
        q.push(src);
        vis[src] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order.push_back(u);
            for (const auto& e : al.get_neighbors(u)) {
                if (!vis[e.to]) {
                    vis[e.to] = true;
                    q.push(e.to);
                }
            }
        }
        return order;
    };

    auto bfs_csr = [&](int src) {
        std::vector<int> order;
        std::vector<bool> vis(n, false);
        std::queue<int> q;
        q.push(src);
        vis[src] = true;
        while (!q.empty()) {
            int u = q.front();
            q.pop();
            order.push_back(u);
            auto [start, end] = csr.neighbor_range(u);
            for (int i = start; i < end; ++i) {
                int v = csr.to[i];
                if (!vis[v]) {
                    vis[v] = true;
                    q.push(v);
                }
            }
        }
        return order;
    };

    std::vector<int> order_mat = bfs_matrix(0);
    std::vector<int> order_al = bfs_adj_list(0);
    std::vector<int> order_csr = bfs_csr(0);

    assert(order_mat.size() == 4);
    assert(order_mat == order_al);
    assert(order_mat == order_csr);

    // ------------------------------------------------------------------------
    // Test 2: Matrix <-> List Round-Trip Conversion
    // ------------------------------------------------------------------------
    AdjacencyList roundtrip_al = adj_matrix_to_adj_list(mat);
    AdjacencyMatrix roundtrip_mat = adj_list_to_adj_matrix(roundtrip_al);
    assert(mat.mat == roundtrip_mat.mat);

    // ------------------------------------------------------------------------
    // Test 3: Implicit Grid Neighborhood Bounds
    // ------------------------------------------------------------------------
    ImplicitGrid grid(3, 4); // 3 rows, 4 cols
    // Corner cell (0, 0): 2 orthogonal neighbors
    auto n_corner = grid.get_neighbors4(0, 0);
    assert(n_corner.size() == 2);
    // Interior cell (1, 1): 4 orthogonal neighbors, 8 full neighbors
    auto n_interior4 = grid.get_neighbors4(1, 1);
    auto n_interior8 = grid.get_neighbors8(1, 1);
    assert(n_interior4.size() == 4);
    assert(n_interior8.size() == 8);

    std::cout << "[PASS] All Graph Representations C++ unit tests passed.\n";
    return 0;
}
