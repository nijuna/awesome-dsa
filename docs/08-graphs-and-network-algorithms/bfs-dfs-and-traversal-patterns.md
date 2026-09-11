---
title: "BFS DFS and Traversal Patterns"
difficulty: "Intermediate"
domains: ["Graphs", "Algorithms", "Search", "Problem Solving"]
prerequisites: ["Arrays and Memory Layout", "Queues", "Stacks", "Recursion", "Basic Complexity Analysis", "Disjoint Set Union"]
related_topics: ["Connected Components", "Topological Sort", "Shortest Paths", "Cycle Detection", "Grid Flood Fill", "State Space Search"]
---

# BFS DFS and Traversal Patterns

> [!NOTE]
> Breadth-First Search (BFS) and Depth-First Search (DFS) are the two fundamental graph traversal patterns. BFS explores outward in concentric distance layers using a FIFO queue and is the standard tool for shortest paths in unweighted graphs. DFS explores deeply along complete paths using recursion or an explicit LIFO stack and is the standard tool for dependency resolution, cycle detection, and discovery/finishing-time analysis.
>
> Reference implementations: [C++17 Implementation](../../implementations/cpp/graph_traversals.cpp) | [Python Implementation](../../implementations/python/graph_traversals.py)

> [!TIP]
> **The Traversal Question at a Glance:**
> The fundamental question in graph exploration is never just *"How do I visit every node?"*, but **"What hidden structure do I want the traversal to reveal?"**
>
> | Pattern | Core Data Structure | Best For | Does It Find Unweighted Shortest Paths? |
> | :--- | :---: | :--- | :---: |
> | **BFS** | FIFO Queue | Distance layers, minimum hops, nearest source | **Yes** |
> | **DFS** | LIFO Stack / Recursion | Structural nesting, cycle detection, finish order | **No** |

> [!WARNING]
> Traversal bugs in production almost always stem from one of three root causes:
> 1. **Late Marking**: Marking nodes as visited upon dequeue rather than enqueue (causing duplicate queue explosions).
> 2. **Disconnected Assumption**: Running traversal from a single root and assuming it covers disconnected components.
> 3. **Call-Stack Exhaustion**: Relying on recursive DFS across deep or degenerate linear graphs.

---

## 1. Why This Matters

A graph is not an array: its nodes are not laid out sequentially in memory, and its topology cannot be indexed directly. To extract meaning from a graph—its components, cycles, connectivity, or bottlenecks—we must **traverse** it.

Traversal is the computational engine behind:
- **Unweighted Shortest Paths & Routing**: Navigation grids, peer-to-peer gossip protocols, hop-count routing.
- **Dependency & Build Systems**: Compiler symbol resolution, package manager installations, topological task scheduling.
- **Structural Decomposition**: Connected components, biconnected components, bridges, and articulation points.
- **State-Space Exploration**: Solving combinatorial puzzles (Rubik's cube, Word Ladder, 15-puzzle, chess tree pruning).
- **Percolation & Image Processing**: Connected-component labeling, flood fill, region growing.

Both BFS and DFS visit every reachable vertex in $O(V + E)$ time when using adjacency lists. Yet their exploration mechanics produce radically different views of the underlying topology.

---

## 2. The Core Mental Model: Frontier vs. Path

Every traversal algorithm maintains an active set of discovered but unprocessed vertices (the **frontier**) and a set of **visited** vertices to prevent infinite loops in cyclic graphs.

The distinction between BFS and DFS comes down to a single algorithmic policy:

```
Frontier Processing Policy:
- BFS (FIFO Queue):  Always expand the OLDEST discovered vertex first.
  -> Produces expanding concentric wavefronts sorted strictly by hop distance.

- DFS (LIFO Stack):  Always expand the MOST RECENTLY discovered vertex first.
  -> Plunges down a single trajectory until it hits a dead end, then backtracks.
```

```
BFS Exploration (Wavefront):            DFS Exploration (Trajectory):
       [0] (Source)                            [0] (Source)
      /   \                                     |
    [1]   [2]   <-- Layer 1                    [1]
    / \   / \                                   |
  [3] [4][5] [6]<-- Layer 2                    [3]
                                                |
                                               [5] (Dead end -> Backtrack)
```

---

## 3. Graph Representation & Complexity Baseline

Throughout this chapter, assume a graph with $V$ vertices and $E$ edges represented via an adjacency list:

```text
adj[u] = [v1, v2, ..., vk]
```

### Space & Time Invariants
- **Adjacency Matrix**: Space $O(V^2)$, Traversal Time $O(V^2)$. Impractical for large sparse graphs ($V = 10^6, E = 10^7$).
- **Adjacency List**: Space $O(V + E)$, Traversal Time $O(V + E)$. Every vertex enters the queue/stack at most once, and every directed edge is traversed exactly once.

---

## 4. Breadth-First Search (BFS)

### 4.1 Queue Discipline & Layer Invariant
BFS starts at a source $s$ at distance $0$. It enqueues $s$, then repeatedly dequeues a vertex $u$ and inspects all adjacent neighbors $v$. If $v$ is unvisited, it is assigned distance $\text{dist}[v] = \text{dist}[u] + 1$, marked visited, and pushed to the back of the queue.

```text
Layer Invariant:
At any point during BFS, the queue contains vertices from at most two consecutive
distance layers: { d, d, ..., d, d+1, d+1, ..., d+1 }.
```

Because the queue is strictly monotonic with respect to distance, vertices are finalized in non-decreasing order of their shortest-path hop count.

```cpp
BFSResult bfs(const std::vector<std::vector<int>>& adj, int start) {
    int n = static_cast<int>(adj.size());
    std::vector<int> dist(n, -1), parent(n, -1);
    std::queue<int> q;

    dist[start] = 0;
    q.push(start); // Rule: Mark upon ENQUEUE

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v : adj[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
                q.push(v); // Mark immediately upon discovery
            }
        }
    }
    return {dist, parent};
}
```

### 4.2 The Critical Discipline: Mark on Enqueue, NOT Dequeue!
A notorious bug in naive BFS implementations is marking a node as visited when it is *popped* from the queue rather than when it is *pushed*.

```text
The Dequeue-Marking Trap:
Suppose node V has 1,000 neighbors in the same layer that all link to node W.
- If marked upon ENQUEUE: W is marked by the first neighbor; the remaining 999 ignore it.
  Queue growth: 1 entry.
- If marked upon DEQUEUE: All 1,000 neighbors observe W as "unvisited" and push it!
  Queue growth: 1,000 duplicate entries of W!
Result: Memory blow-up and exponential slowdown.
```

---

## 5. Shortest Paths & Path Reconstruction

Because BFS discovers vertices in non-decreasing order of distance, the first time node $v$ is reached from $u$, the path from $s$ to $v$ via $u$ is guaranteed to be a **shortest path** in unweighted edge count.

To reconstruct the path, follow parent pointers from $v$ back to $s$ and reverse:

```cpp
std::vector<int> reconstruct_path(int target, const std::vector<int>& parent) {
    if (parent[target] == -1 && target != start_node) return {}; // Unreachable
    std::vector<int> path;
    for (int curr = target; curr != -1; curr = parent[curr]) {
        path.push_back(curr);
    }
    std::reverse(path.begin(), path.end());
    return path;
}
```

---

## 6. Advanced BFS Variations

### 6.1 Multi-Source BFS
When tracking distances to the *nearest* facility among many (e.g., nearest fire station in a city grid, distance to nearest zero in a binary matrix):
- Do **not** execute independent BFS runs from each source ($O(K \cdot (V + E))$).
- **Initialize the queue with all $K$ sources simultaneously** with distance 0:

```cpp
std::vector<int> multi_source_bfs(const std::vector<std::vector<int>>& adj, 
                                 const std::vector<int>& sources) {
    std::vector<int> dist(adj.size(), -1);
    std::queue<int> q;

    for (int s : sources) {
        dist[s] = 0;
        q.push(s);
    }
    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : adj[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }
    return dist; // Runs in exactly O(V + E)
}
```

### 6.2 0-1 BFS with `std::deque`
When edge weights are restricted to $\{0, 1\}$ (e.g., moving on a grid with free teleportation vs. standard cost 1):
- Running Dijkstra costs $O(E \log V)$.
- **0-1 BFS runs in $O(V + E)$** using a double-ended queue (`std::deque`):
  - Traversed edge weight $0 \implies$ push neighbor to the **front** (`push_front`).
  - Traversed edge weight $1 \implies$ push neighbor to the **back** (`push_back`).

This maintains strict monotonic distance ordering in the deque without any heap overhead.

### 6.3 Bidirectional BFS
When finding the shortest path between known start $s$ and target $t$ in a graph with uniform branching factor $b$:
- Forward BFS explores $O(b^d)$ nodes up to distance $d$.
- Bidirectional BFS runs two simultaneous wavefronts from $s$ and $t$, stopping when the frontiers intersect.
- Space and time drop to $O(2 \cdot b^{d/2}) = O(b^{d/2})$, an exponential reduction in state exploration.

---

## 7. Depth-First Search (DFS)

### 7.1 Recursion & Explicit Stack Mechanics
DFS explores as deeply as possible along each branch before backtracking.

```cpp
void dfs(int u, const std::vector<std::vector<int>>& adj, 
         std::vector<bool>& visited, std::vector<int>& order) {
    visited[u] = true;
    order.push_back(u); // Preorder discovery

    for (int v : adj[u]) {
        if (!visited[v]) {
            dfs(v, adj, visited, order);
        }
    }
}
```

> [!WARNING]
> Recursive DFS is conceptually elegant, but on deep, adversarial, or linear-chain graphs ($V = 10^5$), it can easily overflow the default 8 MB operating system thread call stack. For production services or untrusted user graphs, **iterative DFS using an explicit heap-allocated stack (`std::stack<int>`)** is strictly safer.

---

## 8. Discovery and Finishing Timestamps ($t_{\text{in}} / t_{\text{out}}$)

In advanced algorithms, DFS is augmented with a global clock:
- `tin[u]`: Timestamp when node $u$ is first discovered (preorder).
- `tout[u]`: Timestamp when all descendants of $u$ have been completely explored (postorder).

```text
Ancestor Theorem:
Node u is an ancestor of node v in the DFS tree if and only if:
tin[u] < tin[v]  and  tout[u] > tout[v]
(The lifetime interval [tin[v], tout[v]] is strictly nested inside [tin[u], tout[u]]).
```

This simple timestamp property powers $O(1)$ ancestor queries in trees (Binary Lifting LCA) and provides the topological order in DAGs (reverse $t_{\text{out}}$ ordering).

---

## 9. DFS Edge Classifications (Directed Graphs)

When DFS executes on a directed graph, every edge $(u, v)$ belongs to exactly one of four structural categories:

```
                  [ u ] (Current Node)
                 /  |  \
   Tree Edge   /    |    \  Cross Edge (to visited sibling branch)
             v      |      v
           [ v ]    |     [ w ] (Finished earlier)
             |      |
             v      | Forward Edge (to descendant already reachable)
           [ k ] <--+
             |
             +---- Back Edge ----> Ancestor in active recursion stack! (CYCLE)
```

1. **Tree Edge**: Discovers an unvisited node $v$ (`color[v] == White`).
2. **Back Edge**: Points to an ancestor currently in the active recursion call stack (`color[v] == Gray`). **A directed graph contains a cycle if and only if DFS encounters a back edge.**
3. **Forward Edge**: Points to a finished descendant in the same DFS tree (`color[v] == Black`, $\text{tin}[u] < \text{tin}[v]$).
4. **Cross Edge**: Points to a previously finished node in an independent branch ($\text{tin}[u] > \text{tin}[v]$).

---

## 10. Cycle Detection Patterns

### 10.1 Undirected Graph Cycle Detection
In an undirected graph, every edge is bidirectional. A cycle exists if DFS reaches an already-visited vertex that is **not the immediate parent**:

```cpp
bool has_cycle_undirected(int u, int parent, const std::vector<std::vector<int>>& adj, 
                          std::vector<bool>& visited) {
    visited[u] = true;
    for (int v : adj[u]) {
        if (!visited[v]) {
            if (has_cycle_undirected(v, u, adj, visited)) return true;
        } else if (v != parent) {
            return true; // Visited non-parent neighbor confirms cycle!
        }
    }
    return false;
}
```

### 10.2 Directed Graph Cycle Detection (Three-Color State)
A single boolean `visited` flag is insufficient for directed graphs (a cross-edge to a finished node is not a cycle!). Use the three-color scheme:
- **White (0)**: Unvisited.
- **Gray (1)**: Currently in active recursion stack.
- **Black (2)**: Completely explored and exited.

```cpp
bool has_cycle_directed(int u, const std::vector<std::vector<int>>& adj, 
                        std::vector<int>& color) {
    color[u] = 1; // Gray

    for (int v : adj[u]) {
        if (color[v] == 1) return true; // Found Back-Edge to active ancestor!
        if (color[v] == 0 && has_cycle_directed(v, adj, color)) return true;
    }

    color[u] = 2; // Black
    return false;
}
```

---

## 11. Traversing Disconnected Graphs (The Outer Loop)

Never assume a graph is connected from vertex 0! Real-world graphs (dependency graphs, road networks, social clusters) frequently contain isolated components.

Always wrap traversals in an outer loop over all vertices:

```cpp
std::vector<std::vector<int>> find_all_components(int n, const std::vector<std::vector<int>>& adj) {
    std::vector<bool> visited(n, false);
    std::vector<std::vector<int>> components;

    for (int i = 0; i < n; ++i) {
        if (!visited[i]) {
            // Run BFS or DFS to harvest entire connected component
            components.push_back(collect_component(i, adj, visited));
        }
    }
    return components;
}
```

---

## 12. Grid Traversal & Implicit State-Space Search

In competitive programming and systems engineering, graphs are often not explicitly provided as adjacency lists; they are **implicit state spaces**:
- **2D Grid Maze**: Cells are vertices; 4-directional transitions are edges.
- **Rubik's Cube / 15-Puzzle**: Board states are vertices; legal slider/rotational moves are edges.
- **Word Ladder**: Dictionary words are vertices; single-character substitutions are edges.

```
Grid Navigation Invariant:
int dr[] = {-1, 1, 0, 0}; // Up, Down, Left, Right
int dc[] = {0, 0, -1, 1};

for (int i = 0; i < 4; ++i) {
    int nr = r + dr[i], nc = c + dc[i];
    if (nr >= 0 && nr < rows && nc >= 0 && nc < cols && !visited[nr][nc]) {
        // Valid move
    }
}
```

- **Use BFS when**: You need the minimum number of transformation steps, the shortest escape path, or the nearest target.
- **Use DFS when**: You need exhaustive region coloring (Flood Fill), connected island counting, or existence verification.

---

## 13. Comprehensive Traversal Decision Framework

| Problem Requirement | Optimal Traversal | Systems Rationale |
| :--- | :---: | :--- |
| **Shortest Path in Unweighted Graph** | **BFS** | Discovers nodes in strict order of edge hops; first arrival is optimal. |
| **State-Space Minimum Move Puzzle** | **BFS** | Guarantees minimum move sequence without searching full graph. |
| **0/1 Weighted Shortest Path** | **0-1 BFS** | Deque front/back pushes run in $O(V + E)$ without heap overhead. |
| **Nearest Target Among Multiple Sources** | **Multi-Source BFS** | Enqueueing all sources simultaneously expands uniform global wavefront. |
| **Topological Sort on DAG** | **DFS** | Reverse of postorder finishing times ($t_{\text{out}}$) gives valid topological order. |
| **Directed Cycle Detection** | **DFS** | Three-color state tracking identifies back-edges to active ancestors. |
| **Connected-Component Counting** | **Either (or DSU)** | Both visit entire reachable component in $O(V + E)$. |
| **Grid Flood Fill (Area Painting)** | **DFS** | Simpler call stack, lower memory overhead than queue on large grids. |

---

## 14. Curated Problems & Exercises

### 1. LeetCode 102 — Binary Tree Level Order Traversal
- **Pattern**: Standard FIFO BFS with queue size snapshots per layer.

### 2. LeetCode 200 — Number of Islands
- **Pattern**: Outer loop grid traversal with DFS/BFS component marking.

### 3. LeetCode 207 — Course Schedule
- **Pattern**: Directed cycle detection using 3-color DFS or Kahn's BFS.

### 4. LeetCode 127 — Word Ladder
- **Pattern**: Implicit state-space search using bidirectional BFS.

### 5. Codeforces 1063B — Labyrinth
- **Pattern**: 0-1 BFS using `std::deque` to minimize directional turn penalties.

---

## 15. Related Topics & Further Reading

### Internal Documentation
- **[Disjoint Set Union](../04-linear-data-structures/disjoint-set-union.md)**: Incremental component maintenance.
- **[Lowest Common Ancestor](lowest-common-ancestor.md)**: Tree traversal and discovery intervals.
- **[Priority Queues in Practice](../06-heaps-priority-and-selection/priority-queues-in-practice.md)**: Heap backends for weighted shortest paths (Dijkstra).

### Seminal References
- Tarjan, R. E. (1972). *Depth-first search and linear graph algorithms*. SIAM Journal on Computing, 1(2), 146-160.
- Moore, E. F. (1959). *The shortest path through a maze*. Proc. Int. Symp. Switching Theory.
