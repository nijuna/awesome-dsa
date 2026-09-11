---
title: "Disjoint Set Union"
difficulty: "Intermediate to Advanced"
domains: ["Theory", "Algorithms", "Graphs", "Amortized Analysis"]
prerequisites: ["Arrays and Memory Layout", "Trees and Recursion", "Basic Complexity Analysis", "Theoretical vs Practical Performance"]
related_topics: ["Kruskal Minimum Spanning Tree", "Connected Components", "Dynamic Connectivity", "Graph Cycle Detection", "Amortized Analysis"]
---

# Disjoint Set Union

> [!NOTE]
> Disjoint Set Union (DSU), commonly known as Union-Find, maintains a dynamic partition of $n$ elements into disjoint equivalence classes. It supports two primary operations: finding the representative of an element's set and merging two sets. When pairing union by size/rank with path compression, DSU achieves an amortized time complexity of $\Theta(\alpha(n))$ per operation, where $\alpha$ is the extraordinarily slow-growing inverse Ackermann function.
>
> Reference implementations: [C++17 Implementation](../../implementations/cpp/disjoint_set_union.cpp) | [Python Implementation](../../implementations/python/disjoint_set_union.py)

> [!TIP]
> **Core Structural Invariants:**
> - **Partition Invariant**: Every set is structured as a directed rooted tree where edges point upward toward a canonical root representative (`parent[root] == root`).
> - **Height Invariant**: Union by size or rank guarantees that trees remain logarithmically shallow ($O(\log n)$) even before compression.
> - **Self-Flattening Property**: Path compression flattens traversal paths during `find()`, ensuring expensive queries pay for all future queries.

> [!WARNING]
> Standard DSU is strictly a **monotone merging** data structure. It excels at answering *"are these elements currently connected?"* under dynamic edge insertions. It cannot handle arbitrary online edge deletions without advanced machinery (e.g., Rollback DSU for offline intervals, or HDT randomized graphs for online fully dynamic connectivity).

---

## 1. Why This Matters

Many real-world algorithms do not require full graph adjacency lists, path enumerations, or shortest-path trees. Instead, they ask a simpler, fundamental question:

> Do these two items belong to the same connected group or equivalence class?

Examples:
- **Kruskal's Algorithm**: Does adding edge $(u, v)$ create a cycle by connecting vertices already in the same component?
- **Social Networks & Clustering**: Are two users in the same social circle or community?
- **Computer Vision**: In connected-component labeling, which active pixels form a contiguous object?
- **Dynamic Network Analysis**: Has a physical bridge failure split a server cluster into isolated partitions?
- **Constraint Satisfaction**: Do variable equivalence constraints ($x_1 = x_2, x_2 = x_3$) imply $x_1 = x_3$, or contradict an inequality $x_1 \ne x_3$?

An equivalence relation over a set $S$ partitions $S$ into disjoint equivalence classes obeying three axioms:
1. **Reflexivity**: $a \sim a$
2. **Symmetry**: $a \sim b \iff b \sim a$
3. **Transitivity**: $a \sim b \land b \sim c \implies a \sim c$

Disjoint Set Union maintains these equivalence classes under repeated merges using an interface of just two functions:
- `find(x)`: Identify the canonical representative of the set containing $x$.
- `unite(a, b)`: Merge the equivalence classes containing $a$ and $b$.

---

## 2. Compact Parent-Array Forest Representation

Rather than storing explicit set containers (such as hash sets or linked lists), DSU represents equivalence classes as a **forest of rooted trees** stored in a single flat array:

```text
parent[i]
```

Where:
- If `parent[i] == i`, element $i$ is a **canonical root representative**.
- If `parent[i] != i`, element $i$ points to its parent ancestor along the path to the root.

```text
Equivalence Partition:
Set A: {0, 1}                    Set B: {2, 3, 4, 5}

Forest Representation:
        [0] (Root)                       [2] (Root)
         |                              /   \
        [1]                           [3]   [4]
                                             |
                                            [5]

Array Layout:
Index:    0   1   2   3   4   5
parent: [ 0,  0,  2,  2,  2,  4 ]
```

### Why This Representation Is So Efficient:
1. **Ultra-Low Memory Footprint**: Requires only $4$ to $8$ bytes per element (`int parent[]`).
2. **Optimal L1/L2 Cache Locality**: Operations access contiguous vector buffers, avoiding dynamic node allocations and pointer indirection.
3. **Zero Tree Overhead**: No left/right pointers, degree fields, or balance flags per node.

---

## 3. The Basic Operations & The Degeneracy Problem

### 3.1 Naive `find(x)`
Follow parent pointers upward until encountering a self-referential root:

```cpp
int find(int x) {
    while (parent[x] != x) {
        x = parent[x];
    }
    return x;
}
```

### 3.2 Naive `unite(a, b)`
Find the representatives $r_a = \text{find}(a)$ and $r_b = \text{find}(b)$. If $r_a \ne r_b$, set `parent[r_b] = r_a`.

```text
The Degenerate Chain Danger:
Uniting 1-2, 2-3, 3-4, 4-5 arbitrarily can produce a linear linked list:
[5] -> [4] -> [3] -> [2] -> [1] -> [0]
Tree Height: O(n)
find(5) Cost: O(n) worst-case time!
```

Without structural balancing heuristics, a sequence of $m$ operations on $n$ elements collapses to $\Omega(m \cdot n)$ time.

---

## 4. Union by Rank and Union by Size

To prevent trees from degenerating into tall chains, DSU enforces a balancing heuristic when joining two roots.

### 4.1 Union by Size
Maintain an array `size[root]` tracking the number of elements in the tree rooted at `root`:
- When uniting $r_a$ and $r_b$, attach the smaller tree under the larger tree.
- Add the size of the smaller tree to the larger tree:

```cpp
bool unite(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) return false;

    if (sz[a] < sz[b]) std::swap(a, b);
    parent[b] = a;
    sz[a] += sz[b];
    return true;
}
```

#### Why Union by Size Works
A node's depth increases by $1$ only when its tree is merged under an equal or strictly larger tree. Consequently, each time a node's depth increases, the size of its containing set at least **doubles**. In a universe of $n$ elements, a set can double at most $\lfloor \log_2 n \rfloor$ times. Thus:

$$
\text{Max Tree Height} \le \lfloor \log_2 n \rfloor
$$

### 4.2 Union by Rank
Maintain an array `rank[root]` representing an upper bound on tree height:
- Attach the lower-rank root under the higher-rank root.
- If both ranks are equal, pick one root as parent and increment its rank by $1$.

```cpp
void unite_by_rank(int a, int b) {
    a = find(a);
    b = find(b);
    if (a == b) return;

    if (rank[a] < rank[b]) std::swap(a, b);
    parent[b] = a;
    if (rank[a] == rank[b]) ++rank[a];
}
```

Both heuristics bound maximum tree height strictly to $O(\log n)$, guaranteeing that `find` and `unite` take $O(\log n)$ worst-case time.

---

## 5. Path Compression: Self-Flattening Forests

While union heuristics prevent trees from growing too tall eagerly, **path compression** repairs long paths lazily during query execution.

### 5.1 Full Path Compression (Recursive)
When traversing from $x$ up to root $r$, rewrite every visited node's parent pointer directly to $r$:

```cpp
int find(int x) {
    if (parent[x] != x) {
        parent[x] = find(parent[x]); // Point directly to canonical root
    }
    return parent[x];
}
```

```
Before find(5):                         After find(5):
       [0] (Root)                              [0] (Root)
        |                                    / / | \
       [1]                                 [1][2][3][4]
        |                                              \
       [2]                                             [5]
        |
       [3]                              Every visited ancestor now
        |                               points directly to root [0]!
       [4]
        |
       [5] <-- Query
```

### 5.2 Two-Pass Iterative Path Compression
For environments where call stack recursion is restricted:

```cpp
int find_iterative(int x) {
    int root = x;
    while (parent[root] != root) {
        root = parent[root];
    }
    while (parent[x] != x) {
        int next = parent[x];
        parent[x] = root;
        x = next;
    }
    return root;
}
```

### 5.3 Path Halving & Path Splitting (Single-Pass Alternatives)
- **Path Halving**: Skip every other node on the path upward:
  ```cpp
  while (parent[x] != x) {
      parent[x] = parent[parent[x]];
      x = parent[x];
  }
  ```
- **Path Splitting**: Make every node point to its grandparent:
  ```cpp
  while (parent[x] != x) {
      int next = parent[x];
      parent[x] = parent[parent[x]];
      x = next;
  }
  ```

---

## 6. The Inverse Ackermann Function $\alpha(n)$

When union by rank/size is combined with path compression, the amortized cost per operation drops from $O(\log n)$ to:

$$
\Theta(\alpha(n))
$$

where $\alpha(n)$ is the **inverse Ackermann function**.

### How Slow Does $\alpha(n)$ Grow?
The Ackermann function $A(i, j)$ is a notoriously fast-growing non-primitive recursive function:
- $A(1, j) = 2j$
- $A(2, j) = 2^j$
- $A(3, j) = 2^{2^{\cdot^{\cdot^2}}}$ (Tower of powers of height $j$)
- $A(4, 1) = 2^{2^{2^2}} = 65,536$
- $A(4, 2) = 2^{65536} \approx 10^{19729}$ (vastly exceeds the estimated $10^{80}$ atoms in the observable universe!)

The inverse Ackermann function $\alpha(n)$ is defined as:

$$
\alpha(n) = \min \{ k \mid A(k, 1) \ge n \}
$$

$$
\alpha(n) \le 4 \quad \text{for any } n \le 10^{19729}
$$

For every imaginable input size in computer science, $\alpha(n) \le 4$. While theoretically non-constant, in practical systems engineering DSU runs in **effective constant time ($O(1)$)** per operation.

---

## 7. Amortized Complexity Proof Sketch (Tarjan, 1975)

Why does path compression combined with union by rank produce $\Theta(\alpha(n))$ instead of merely $O(\log n)$?

### 1. Rank Invariants
Under union by rank:
1. `rank[x]` is assigned when $x$ is created and only increases when $x$ is a root. Once $x$ becomes a child, `rank[x]` is immutable.
2. For every non-root node $x$, $\text{rank}[x] < \text{rank}[\text{parent}[x]]$. Ranks strictly increase along paths to roots.
3. The number of nodes of rank $r$ is at most $n / 2^r$.

### 2. Rank Grouping
Tarjan partitioned ranks into intervals or "groups" defined by the Ackermann progression:
- Group 0: $\{0\}$
- Group 1: $\{1\}$
- Group 2: $\{2, 3\}$
- Group 3: $\{4, 5, \dots, 15\}$
- Group $k$: $\{B(k-1), \dots, B(k) - 1\}$ where $B(k) \approx A(k, 1)$.

The total number of rank groups for a universe of size $n$ is at most $\alpha(n)$.

### 3. Accounting Charges
During a query `find(x)`, edges along the traversal path are categorized into two types:
- **Boundary Edges**: Edges $(u, \text{parent}[u])$ where $u$ and $\text{parent}[u]$ belong to *different* rank groups. Since there are at most $\alpha(n)$ rank groups, at most $\alpha(n)$ boundary edges exist along any path. The query pays directly for these edges: $O(\alpha(n))$.
- **Internal Edges**: Edges $(u, \text{parent}[u])$ where $u$ and $\text{parent}[u]$ belong to the *same* rank group. Each time an internal edge is traversed, path compression moves $u$ to point to a new parent with a strictly higher rank. Node $u$ can move to a higher-rank parent within its own rank group only a limited number of times before its parent's rank crosses into the next group.

Summing the charges across all nodes proves that the total work across $m$ operations on $n$ elements is strictly bounded by:

$$
O(m \cdot \alpha(n))
$$

---

## 8. Summary of Complexity Across Variants

| DSU Heuristic Configuration | Worst-Case `find` | Worst-Case `unite` | Amortized Cost per Op ($m$ ops) |
| :--- | :---: | :---: | :---: |
| **Naive (No Heuristics)** | $O(n)$ | $O(n)$ | $O(n)$ |
| **Union by Size/Rank Only** | $O(\log n)$ | $O(\log n)$ | $O(\log n)$ |
| **Path Compression Only** | $O(n)$ | $O(n)$ | $O(\log_{1 + m/n} n)$ |
| **Union by Size/Rank + Path Compression** | $O(\log n)$ | $O(\log n)$ | **$\Theta(\alpha(n))$** |

---

## 9. Rollback DSU: Undoing Unions

Standard DSU is monotonic: once two sets are merged, they cannot be unmerged. Furthermore, path compression writes to multiple ancestor pointers, making state tracking difficult to reverse.

In **offline dynamic connectivity** (e.g., answering connectivity queries while edges are inserted and deleted over time via a segment tree over time), we must support:
- `unite(u, v)`
- `checkpoint()`
- `rollback_to(checkpoint)`

> [!WARNING]
> **Rollback DSU and full path compression do not mix.** Path compression rewrites multiple parent pointers dynamically along the path, which makes exact historical undo expensive. Rollback DSU intentionally relies on **union by size/rank alone** (without path compression), achieving deterministic $O(1)$ stack-based undo while keeping tree operations at $O(\log n)$.

```cpp
class RollbackDSU {
    struct Operation { int child, parent, old_parent_size; };
    std::vector<int> parent, size;
    std::vector<Operation> history;

public:
    int find(int x) const {
        while (parent[x] != x) x = parent[x]; // No path compression!
        return x;
    }

    bool unite(int a, int b) {
        a = find(a); b = find(b);
        if (a == b) return false;
        if (size[a] < size[b]) std::swap(a, b);

        history.push_back({b, a, size[a]});
        parent[b] = a;
        size[a] += size[b];
        return true;
    }

    void rollback_to(std::size_t cp) {
        while (history.size() > cp) {
            auto [child, par, old_sz] = history.back();
            history.pop_back();
            parent[child] = child;
            size[par] = old_sz;
        }
    }
};
```

---

## 10. Potential / Parity DSU (Weighted DSU)

Some problems require tracking **relative relationships** between elements in the same component.

### Parity / Bipartite Graph DSU
Suppose we receive constraints indicating whether two vertices share the same color ($0$) or opposite colors ($1$).
- Store `parity[x]`: relative color difference between $x$ and `parent[x]`.
- During `find(x)`, path compression updates parity using XOR:
  ```cpp
  parity[x] ^= parity[original_parent];
  ```
- When adding a constraint $(a, b, \text{rel})$:
  - If $a$ and $b$ are already in the same component, check consistency:
    $$
    (\text{parity}[a] \oplus \text{parity}[b]) == \text{rel}
    $$
    If false, an **odd cycle** is detected (the graph is not bipartite!).
  - If in different components, merge roots and set:
    $$
    \text{parity}[r_b] = \text{parity}[a] \oplus \text{parity}[b] \oplus \text{rel}
    $$

---

## 11. Systems Applications & Benchmark Reality

1. **Kruskal's Minimum Spanning Tree**:
   Sort $E$ edges in $O(E \log E)$. For each edge $(u, v)$, execute `unite(u, v)`. DSU checks cycle formation in practically $O(1)$ time, making sorting the only bottleneck.
2. **Percolation Theory (Monte Carlo Simulations)**:
   In a grid of $N \times N$ sites, sites open randomly. DSU models porous media flow by tracking when open pores on the top row connect to the bottom row.
3. **Connected-Component Image Labeling**:
   Single-pass raster scanning assigns provisional region labels to 8-connected foreground pixels and unions touching provisional labels, compressing them into final object masks.

---

## 12. Curated Problems & Exercises

### 1. Kruskal's Minimum Spanning Tree
- **Pattern**: Greedy edge sorting + DSU cycle prevention.
- **Why It Matters**: Canonical textbook application demonstrating that set partitioning eliminates DFS/BFS cycle checks.

### 2. LeetCode 684 — Redundant Connection
- **Pattern**: Dynamic cycle detection in an undirected graph.
- **Key Insight**: The first edge $(u, v)$ where `dsu.same(u, v) == true` is the redundant edge that creates the cycle.

### 3. Bipartite Constraint Verification
- **Pattern**: Parity DSU tracking relative 2-coloring.
- **Key Insight**: Detecting odd cycles on-the-fly during edge insertions using path-compressed XOR potentials.

---

## 13. Related Topics & Further Reading

### Internal Documentation
- **[Dynamic Arrays and Strings](dynamic-arrays-and-strings.md)**: Contiguous memory layout underlying parent arrays.
- **[Theoretical vs Practical Performance](../22-benchmarking-and-tradeoffs/theoretical-vs-practical-performance.md)**: Why compact arrays outperform pointer-heavy tree structures.
- **[Lowest Common Ancestor](../08-graphs-and-network-algorithms/lowest-common-ancestor.md)**: Tree lifting and hierarchical queries.

### Seminal References
- Tarjan, R. E. (1975). *Efficiency of a good but not linear set union algorithm*. Journal of the ACM (JACM), 22(2), 215-225.
- Hopcroft, J. E., & Ullman, J. D. (1973). *Set merging algorithms*. SIAM Journal on Computing, 2(4), 294-303.
- Fredman, M. L., & Saks, M. E. (1989). *The cell probe complexity of dynamic data structures*. STOC '89.
