---
title: "All-Pairs Shortest Paths"
difficulty: "Intermediate"
domains: ["Graphs", "Algorithms", "Dynamic Programming", "Shortest Paths"]
prerequisites: ["Graph Fundamentals", "Shortest Paths", "Dijkstra Algorithm", "Bellman-Ford", "Basic Complexity Analysis"]
related_topics: ["Shortest Paths", "Dynamic Programming", "Bit Manipulation", "Graph Diameter", "Transitive Closure"]
---

# All-Pairs Shortest Paths

> [!NOTE]
> The **All-Pairs Shortest Paths (APSP)** problem asks for the minimum-weight directed path between every ordered pair of vertices $(u, v) \in V \times V$. While Single-Source Shortest Path (SSSP) computes a single 1D vector of distances from a source $s$, APSP computes a complete $V \times V$ distance matrix $D$ along with predecessor or next-hop pointers for path reconstruction.
>
> Reference implementations: [C++17 Implementation](../../implementations/cpp/all_pairs_shortest_paths.cpp) | [Python Implementation](../../implementations/python/all_pairs_shortest_paths.py)

> [!TIP]
> **APSP Algorithm Selection Matrix:**
>
> | Graph Regime / Objective | Recommended Algorithm | Time Complexity | Auxiliary Space | Key Strengths |
> | :--- | :--- | :---: | :---: | :--- |
> | **Dense Graphs** ($E \approx V^2$) | **Floyd-Warshall** | $\Theta(V^3)$ | $O(V^2)$ in-place | Extremely compact, cache-friendly flat array DP, handles negative edges |
> | **Sparse Graphs** ($E \ll V^2$) | **Johnson's Algorithm** | $O(V \cdot E + V^2 \log V)$ | $O(V + E)$ | Reweights edges via Bellman-Ford, then executes $|V|$ Dijkstra runs |
> | **Reachability Only (Transitive Closure)** | **Warshall with Bitsets** | $O(V^3 / 64)$ | $O(V^2 / 64)$ | Word-level parallel bitwise OR operations; ultrafast in hardware |
> | **Unweighted Graphs** | **$|V|$ Breadth-First Searches** | $\Theta(V(V + E))$ | $O(V)$ | Optimal for unit-cost road/packet networks |

> [!WARNING]
> **Implementation Pitfalls:**
> 1. **Diagonal Invariant for Negative Cycles**: After Floyd-Warshall terminates, check the main diagonal: if any $D[i][i] < 0$, a negative cycle exists that is reachable from $i$ back to itself.
> 2. **Infinity Sentinel Addition**: If `dist[i][k] == INF` or `dist[k][j] == INF`, bypass the relaxation step. Adding large constants like `LLONG_MAX` will overflow into negative values and create false shortest paths.
> 3. **Potential Telescoping in Johnson's Algorithm**: Remember that Dijkstra runs on reweighted values $\hat{w}(u, v) = w(u, v) + h(u) - h(v)$. You must convert distances back to original metrics: $D[u][v] = \hat{D}[u][v] - h(u) + h(v)$.

---

## 1. Problem Formulation

Let $G = (V, E)$ be a directed weighted graph with vertex set $V = \{0, 1, \dots, n-1\}$, edge set $E$, and weight function $w: E \to \mathbb{R}$.

The **All-Pairs Shortest Path** problem computes the distance matrix $D \in \mathbb{R}^{n \times n}$:

$$
D[u][v] = \begin{cases} 
0 & \text{if } u = v \\
\min_{P: u \rightsquigarrow v} \sum_{e \in P} w(e) & \text{if a path exists} \\
+\infty & \text{if no path exists}
\end{cases}
$$

### Predecessor and Next-Hop Representations:
To reconstruct the optimal path between any pair $(u, v)$:
- **Next-Hop Matrix** $\text{Next}[u][v]$: Stores the immediate neighbor of $u$ on the shortest path toward $v$.
- **Predecessor Matrix** $\Pi[u][v]$: Stores the penultimate vertex immediately preceding $v$ on the shortest path from $u$.

---

## 2. Floyd-Warshall Algorithm (Matrix Dynamic Programming)

Floyd-Warshall is a dynamic programming algorithm that processes intermediate vertices incrementally.

### The Subproblem Invariant:
Let $d^{(k)}[i][j]$ represent the length of the shortest path from vertex $i$ to vertex $j$ whose internal intermediate vertices belong exclusively to the prefix subset $\{0, 1, \dots, k-1\}$.

```text
Floyd-Warshall State Transition:
             (Direct / Prior Path via {0 ... k-1})
      [ i ] -----------------------------------------> [ j ]
        \                                             ^
         \                                           /
          \                                         /
     d[i][k]\                                     / d[k][j]
             \                                   /
              v                                 /
             [ k ] (Candidate Intermediate Vertex)
```

```mermaid
graph LR
    i["Source i"] -->|d[i][k]| k["Intermediate k"]
    k -->|d[k][j]| j["Target j"]
    i -.->|"Current d[i][j]"| j
```

### The Recurrence Relation:
For step $k$ (where vertex $k$ is considered as a new intermediate candidate):
1. **Case 1 (Exclude $k$)**: The optimal path does not use vertex $k$. Length is $d^{(k)}[i][j]$.
2. **Case 2 (Include $k$)**: The optimal path decomposes into subpath $i \rightsquigarrow k$ followed by subpath $k \rightsquigarrow j$, both using only intermediates from $\{0, \dots, k-1\}$. Length is $d^{(k)}[i][k] + d^{(k)}[k][j]$.

$$
d^{(k+1)}[i][j] = \min \left( d^{(k)}[i][j], \; d^{(k)}[i][k] + d^{(k)}[k][j] \right)
$$

### In-Place Space Optimization:
Because the values $d^{(k)}[i][k]$ and $d^{(k)}[k][j]$ are invariant with respect to relaxation through $k$ (a shortest simple path cannot traverse $k$ twice without creating a cycle), the DP can be executed **in place** on a single $2D$ matrix $D[i][j]$, dropping space complexity from $O(V^3)$ to $O(V^2)$.

---

## 3. C++17 Reference Implementation: Floyd-Warshall with Path Reconstruction

```cpp
#include <vector>
#include <limits>
#include <algorithm>

constexpr long long INF = std::numeric_limits<long long>::max() / 4;

struct FloydWarshallResult {
    std::vector<std::vector<long long>> dist;
    std::vector<std::vector<int>> next;
    bool has_negative_cycle;
};

FloydWarshallResult floyd_warshall(
    int n,
    const std::vector<std::vector<long long>>& adj_matrix) {

    std::vector<std::vector<long long>> dist = adj_matrix;
    std::vector<std::vector<int>> next(n, std::vector<int>(n, -1));

    // Base Case Initialization
    for (int i = 0; i < n; ++i) {
        dist[i][i] = std::min(dist[i][i], 0LL);
        next[i][i] = i;
        for (int j = 0; j < n; ++j) {
            if (i != j && adj_matrix[i][j] != INF) {
                next[i][j] = j;
            }
        }
    }

    // Main 3-loop dynamic programming
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

    // Negative cycle detection on main diagonal
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

    if (next[u][v] == -1) return {};
    std::vector<int> path = {u};
    int curr = u;
    while (curr != v) {
        curr = next[curr][v];
        if (curr == -1 || path.size() > next.size()) return {};
        path.push_back(curr);
    }
    return path;
}
```

---

## 4. Johnson's Algorithm for Sparse Graphs

When graphs are sparse ($E \ll V^2$), Floyd-Warshall's $O(V^3)$ runtime becomes inefficient compared to running Dijkstra from every vertex ($O(V \cdot E \log V)$). However, Dijkstra requires non-negative edge weights.

**Johnson's algorithm** overcomes this by using a single pass of Bellman-Ford to compute a vertex potential function $h: V \to \mathbb{R}$ that **reweights all edges to non-negative values** without altering the identity of shortest paths.

### Mathematical Foundation: Reweighting via Vertex Potentials
For any potential function $h: V \to \mathbb{R}$, define the reweighted edge cost $\hat{w}(u, v)$:

$$
\hat{w}(u, v) = w(u, v) + h(u) - h(v)
$$

Consider any directed path $P = \langle v_0, v_1, \dots, v_k \rangle$ from $v_0$ to $v_k$. The reweighted path cost telescopes:

$$
\hat{w}(P) = \sum_{i=1}^k \hat{w}(v_{i-1}, v_i) = \sum_{i=1}^k \left( w(v_{i-1}, v_i) + h(v_{i-1}) - h(v_i) \right) = w(P) + h(v_0) - h(v_k)
$$

Because $h(v_0) - h(v_k)$ is a constant independent of the specific path taken between $v_0$ and $v_k$, **any path minimizing $\hat{w}(P)$ also minimizes $w(P)$**.

### Finding Valid Potentials ($h$):
To guarantee $\hat{w}(u, v) \ge 0$, we require:

$$
w(u, v) + h(u) - h(v) \ge 0 \iff h(v) \le h(u) + w(u, v)
$$

This is the standard triangle inequality of shortest paths!
1. Add an auxiliary super-source $s^* \notin V$ with directed edges $(s^*, v)$ of weight $0$ to every $v \in V$.
2. Run Bellman-Ford from $s^*$. If a negative cycle is detected, terminate.
3. Set $h(v) = \text{dist}(s^*, v)$ for each $v \in V$.
4. By definition of shortest paths, $h(v) \le h(u) + w(u, v)$, guaranteeing $\hat{w}(u, v) \ge 0$.

### C++17 Reference Implementation: Johnson's Algorithm

```cpp
#include <vector>
#include <queue>
#include <limits>

struct DirectedEdge {
    int from;
    int to;
    long long weight;
};

struct Edge {
    int to;
    long long weight;
};

std::vector<std::vector<long long>> johnson_apsp(
    int n,
    const std::vector<DirectedEdge>& edges) {

    // Step 1: Bellman-Ford from super-source
    std::vector<long long> h(n + 1, INF);
    int super_source = n;
    h[super_source] = 0;

    std::vector<DirectedEdge> ext_edges = edges;
    for (int v = 0; v < n; ++v) ext_edges.push_back({super_source, v, 0});

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

    // Step 2: Negative cycle validation
    for (const auto& e : ext_edges) {
        if (h[e.from] == INF) continue;
        if (h[e.from] + e.weight < h[e.to]) {
            return {}; // Reachable negative cycle detected
        }
    }

    // Step 3: Reweight edges to non-negative values
    std::vector<std::vector<Edge>> reweighted(n);
    for (const auto& e : edges) {
        long long rw = e.weight + h[e.from] - h[e.to];
        reweighted[e.from].push_back({e.to, rw});
    }

    // Step 4: Run Dijkstra from every vertex
    std::vector<std::vector<long long>> dist(n, std::vector<long long>(n, INF));

    for (int s = 0; s < n; ++s) {
        std::vector<long long> d(n, INF);
        d[s] = 0;
        using State = std::pair<long long, int>;
        std::priority_queue<State, std::vector<State>, std::greater<State>> pq;
        pq.push({0, s});

        while (!pq.empty()) {
            auto [cur_d, u] = pq.top();
            pq.pop();
            if (cur_d > d[u]) continue;

            for (const auto& e : reweighted[u]) {
                long long nd = cur_d + e.weight;
                if (nd < d[e.to]) {
                    d[e.to] = nd;
                    pq.push({nd, e.to});
                }
            }
        }

        // Step 5: Convert reweighted distances back to original weights
        for (int v = 0; v < n; ++v) {
            if (d[v] != INF) {
                dist[s][v] = d[v] - h[s] + h[v];
            }
        }
    }

    return dist;
}
```

---

## 5. Warshall's Algorithm & Bitset Transitive Closure

When edge weights are irrelevant and the only query is **reachability** ($u \rightsquigarrow v$), the problem reduces to the **Transitive Closure**.

### Boolean Recurrence:
Let $R[i][j]$ be a boolean matrix indicating if $j$ is reachable from $i$:

$$
R[i][j] \leftarrow R[i][j] \lor (R[i][k] \land R[k][j])
$$

### 64-Bit Bitset Packing Acceleration:
Instead of iterating through every column $j$ with scalar boolean operations, each row $R[i]$ can be represented as a bit-vector of 64-bit unsigned integers (`uint64_t`). If $R[i][k]$ is true, row $i$ updates via a word-level bitwise OR:

$$
R[i] \leftarrow R[i] \mid R[k]
$$

This accelerates transitive closure by a factor of 64 in runtime:

$$
O\left(\frac{V^3}{64}\right)
$$

```cpp
std::vector<std::vector<bool>> bitset_transitive_closure(
    const std::vector<std::vector<bool>>& adj) {

    int n = static_cast<int>(adj.size());
    int words = (n + 63) / 64;
    std::vector<std::vector<uint64_t>> bit_reach(n, std::vector<uint64_t>(words, 0ULL));

    for (int i = 0; i < n; ++i) {
        bit_reach[i][i / 64] |= (1ULL << (i % 64)); // Reflexive
        for (int j = 0; j < n; ++j) {
            if (adj[i][j]) bit_reach[i][j / 64] |= (1ULL << (j % 64));
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
            if (bit_reach[i][j / 64] & (1ULL << (j % 64))) reach[i][j] = true;
        }
    }
    return reach;
}
```

---

## 6. Comprehensive Complexity Comparison

| Algorithm | Graph Topology | Edge Weights | Negative Cycles | Time Complexity | Auxiliary Space |
| :--- | :--- | :---: | :---: | :---: | :---: |
| **Floyd-Warshall** | Dense ($E \approx V^2$) | Arbitrary | **Detects** ($D[i][i] < 0$) | $\Theta(V^3)$ | $O(V^2)$ in-place |
| **Johnson's Algorithm** | Sparse ($E \ll V^2$) | Arbitrary | **Detects** (via Bellman-Ford) | $O(V \cdot E + V^2 \log V)$ | $O(V + E)$ |
| **Repeated Dijkstra** | Any | Non-negative ($w \ge 0$) | Cannot handle | $O(V(V + E) \log V)$ | $O(V + E)$ |
| **Warshall Transitive Closure** | Any | Unweighted | N/A | $\Theta(V^3)$ | $O(V^2)$ |
| **Bitset Transitive Closure** | Dense | Unweighted | N/A | $\Theta(V^3 / 64)$ | $O(V^2 / 64)$ |

---

## 7. Systems Applications & Graph Metrics

1. **Graph Diameter & Radius**:
   - The **eccentricity** of vertex $u$ is $\epsilon(u) = \max_{v \in V} D[u][v]$.
   - The **diameter** is $\max_{u \in V} \epsilon(u)$ (longest shortest path).
   - The **radius** is $\min_{u \in V} \epsilon(u)$ (center of the graph).
2. **All-Pairs Routing Tables**: Precomputing static forwarding tables for autonomous network systems.
3. **Compiler Alias Analysis & Type Inheritance**: Utilizing transitive closure over class hierarchies and pointer graphs.
4. **Detecting Arbitrage Opportunities**: Constructing exchange-rate logs and identifying negative cycles across entire currency pairs.

---

## 8. Practice Problems & Engineering Challenges

1. **Network Diameter in Highway System**: Given city coordinates and toll roads, compute the highway diameter using Floyd-Warshall and reconstruct the longest diameter route.
2. **Dense Matrix Cache Locality**: Optimize Floyd-Warshall's inner loop ordering (`k -> i -> j` vs `i -> k -> j`) and measure L1/L2 cache misses using performance counters.
3. **Johnson vs. Floyd-Warshall Benchmark**: Profile runtime transitions on random graphs as edge density increases from $E = 2V$ to $E = V(V-1)/2$.
4. **Min-Mean Weight Cycle**: Find the directed cycle whose average edge weight is minimal using Karp's algorithm or binary search over APSP potentials.

---

## 9. Next Steps & Suggested Reading

- **Network Flow** (`network-flow.md`): Ford-Fulkerson, Edmonds-Karp, and Dinic's blocking flow algorithms.
- **Bipartite Matching** (`bipartite-matching.md`): Hopcroft-Karp algorithm and Konig's theorem.
- **Dynamic Programming on Trees**: Optimizing tree paths and diameters in $O(V)$ time.
