---
title: "Lowest Common Ancestor (LCA): Algorithms, RMQ Reduction & Systems Applications"
difficulty: "Intermediate to Advanced"
domains: ["Graphs", "Trees", "Competitive Programming", "Theory"]
prerequisites: ["Trees & Traversals", "Binary Search", "Disjoint Set Union", "Sparse Tables"]
related_topics: ["Heavy-Light Decomposition", "Euler Tour Technique", "Range Minimum Query", "Centroid Decomposition"]
---

# Lowest Common Ancestor (LCA): Algorithms, RMQ Reduction & Systems Applications

> [!NOTE]
> Given a rooted tree $T$ and two nodes $u$ and $v$, the **Lowest Common Ancestor (LCA)** is the deepest node that is an ancestor of both $u$ and $v$. It is the fundamental building block for tree distance calculations, path queries, network routing hierarchies, and phylogenetic trees.

---

## 1. Why This Matters

Tree path queries are pervasive in both algorithmic contests and real-world systems:
* **Tree Path Distances**: The distance between any two nodes $u, v$ in a weighted tree is computed in $O(1)$ after finding their LCA:
  $$\text{dist}(u, v) = \text{dist}(\text{root}, u) + \text{dist}(\text{root}, v) - 2 \cdot \text{dist}(\text{root}, \text{LCA}(u, v))$$
* **Tree Path Aggregations**: Querying maximum edge weight, sum, or bottlenecks along the unique path between $u$ and $v$ reduces to querying path $(u \to \text{LCA})$ and path $(v \to \text{LCA})$.
* **Filesystem & Git Internals**: Finding the common merge base of two Git branches (`git merge-base branchA branchB`) is an LCA problem over the Directed Acyclic Graph (DAG) of commit snapshots.
* **Network IP Multicast**: Routing packets efficiently from source to multiple receivers along tree branches without redundant transmission.

---

## 2. Core Intuition & Visual Mental Model

```text
                  1 (Root, Depth 0)
                /   \
 (Depth 1)     2     3
              / \     \
 (Depth 2)   4   5     6
            /     \
 (Depth 3) 7       8

* LCA(7, 8) = 2  (Deepest shared ancestor)
* LCA(7, 4) = 4  (A node can be an ancestor of itself)
* LCA(8, 6) = 1  (Only the root connects them)
```

---

## 3. The 4 Canonical Approaches: Architectural Tradeoffs

Different engineering scenarios require different algorithmic tradeoffs between **preprocessing time**, **query latency**, and **memory footprint**:

| Technique | Preprocessing Time | Query Latency | Auxiliary Memory | Best For |
| :--- | :--- | :--- | :--- | :--- |
| **1. Naive Pointer Walk** | $O(N)$ (depths only) | $O(N)$ | $O(N)$ | Single one-off query on unweighted trees. |
| **2. Binary Lifting** | $O(N \log N)$ | $O(\log N)$ | $O(N \log N)$ | **General Workhorse**: Supports dynamic path aggregations (e.g. max edge on path). |
| **3. Euler Tour + RMQ** | $O(N \log N)$ or $O(N)$ | **$O(1)$** | $O(N \log N)$ or $O(N)$ | **Ultra-low latency queries**: Millions of read queries on static trees. |
| **4. Tarjan's Offline (DSU)** | $O(N + Q \cdot \alpha(N))$ | **$O(\alpha(N)) \approx O(1)$** | $O(N + Q)$ | Batch query processing when all queries are known upfront. |

---

## 4. Deep Dive 1: Binary Lifting ($O(N \log N)$ Preprocessing, $O(\log N)$ Query)

### Mathematical Invariant
Let $\text{up}[u][i]$ be the $2^i$-th ancestor of node $u$. If no such ancestor exists, $\text{up}[u][i] = 0$ (or root).

The dynamic programming recurrence is:
$$\text{up}[u][i] = \text{up}\big[\text{up}[u][i-1]\big][i-1]$$
*(The $2^i$-th ancestor is the $2^{i-1}$-th ancestor of the $2^{i-1}$-th ancestor).*

```text
u ──(2^(i-1) steps)──► up[u][i-1] ──(2^(i-1) steps)──► up[u][i]
└──────────────────────── 2^i steps ───────────────────────┘
```

### The Query Algorithm in 2 Steps:
1. **Equalize Depths**: If $\text{depth}(u) < \text{depth}(v)$, swap them. Lift $u$ upward in powers of 2 until $\text{depth}(u) == \text{depth}(v)$.
2. **Jump Simultaneously**: If $u == v$, return $u$. Otherwise, iterate $i = \lfloor \log_2 N \rfloor$ down to $0$. If $\text{up}[u][i] \ne \text{up}[v][i]$, both jump simultaneously: $u \leftarrow \text{up}[u][i]$ and $v \leftarrow \text{up}[v][i]$.
3. The parent of $u$ (i.e. $\text{up}[u][0]$) is the LCA!

---

## 5. Deep Dive 2: Euler Tour + RMQ ($O(1)$ Query via Sparse Table)

By recording a tree traversal tour, the tree LCA problem can be **isomorphically reduced to a static Range Minimum Query (RMQ)** problem.

### The Reduction Steps:
1. Perform an Euler Tour (DFS) that visits each edge twice. Record the node label and its depth every time a node is entered or re-entered from a child.
   * Tour length: Exactly $2N - 1$.
2. Maintain `first[u]`: the index of the first occurrence of node $u$ in the tour array.
3. **The Core Theorem**: For any two nodes $u$ and $v$, the LCA is the node with the **minimum depth** in the Euler tour between index $\min(\text{first}[u], \text{first}[v])$ and $\max(\text{first}[u], \text{first}[v])$.

```text
Euler Tour Nodes:  [ 1,  2,  4,  7,  4,  2,  5,  8,  5,  2,  1,  3,  6,  3,  1 ]
Depths:            [ 0,  1,  2,  3,  2,  1,  2,  3,  2,  1,  0,  1,  2,  1,  0 ]
                     ▲                       ▲
              first[7] = 3             first[8] = 7

Between index 3 and 7, the minimum depth is 1 (at index 5), corresponding to Node 2!
Hence, LCA(7, 8) = 2.
```

Using a **Sparse Table** over the depth array, range minimum queries are answered in strict **$O(1)$ time**.

---

## 6. Deep Dive 3: Tarjan's Offline Algorithm ($O(N + Q)$ with Union-Find)

When all $Q$ queries are known before execution, Tarjan's algorithm answers all queries in a single DFS traversal using **Disjoint Set Union (DSU)**:

1. Maintain node states: `UNVISITED`, `VISITING`, `VISITED`.
2. When visiting node $u$:
   * Create a singleton DSU set with representative $u$.
   * Recursively visit all children. After each child $v$ finishes, union $v$'s set into $u$'s set, making $u$ the set representative.
   * For every query $(u, w)$: if $w$ is already `VISITED`, then $\text{LCA}(u, w) = \text{Find}(w)$.
3. Overall time: $O(N + Q \cdot \alpha(N)) \approx O(N + Q)$ total time.

---

## 7. Hardware & Memory Cache Reality

In Binary Lifting, the lookup table stores $N \times \lceil \log_2 N \rceil$ node indices.

### Memory Layout Optimization (Row-Major vs Column-Major)
* **Bad**: `int up[LOGN][N];`  
  Accessing `up[i][u]` followed by `up[i-1][u]` jumps $N \times 4$ bytes across memory, causing repeated CPU L1/L2 cache misses during query resolution.
* **Optimal**: `int up[N][LOGN];` (or contiguous flattened `vector<int> up(N * LOGN)`)  
  All $2^i$ ancestors for node $u$ reside contiguously within the same 64-byte cache line:
  $$\text{sizeof}(\text{int}) \times 16\text{ levels} = 64\text{ bytes (Exactly 1 CPU Cache Line!)}$$
  **Result**: Loading node $u$'s ancestors fetches the entire ancestor jump table in a single hardware memory burst.

---

## 8. Canonical Implementations

### Python (Binary Lifting)

```python
import math

class TreeLCA:
    def __init__(self, n: int, adj: list[list[int]], root: int = 1):
        self.n = n
        self.log = math.ceil(math.log2(max(n, 2))) + 1
        self.depth = [0] * (n + 1)
        self.up = [[0] * self.log for _ in range(n + 1)]
        self._dfs(root, root, 0, adj)

    def _dfs(self, u: int, p: int, d: int, adj: list[list[int]]):
        self.depth[u] = d
        self.up[u][0] = p
        for i in range(1, self.log):
            self.up[u][i] = self.up[self.up[u][i - 1]][i - 1]
        for v in adj[u]:
            if v != p:
                self._dfs(v, u, d + 1, adj)

    def query(self, u: int, v: int) -> int:
        if self.depth[u] < self.depth[v]:
            u, v = v, u

        # Step 1: Lift u to the same depth as v
        diff = self.depth[u] - self.depth[v]
        for i in range(self.log):
            if (diff >> i) & 1:
                u = self.up[u][i]

        if u == v:
            return u

        # Step 2: Jump together
        for i in range(self.log - 1, -1, -1):
            if self.up[u][i] != self.up[v][i]:
                u = self.up[u][i]
                v = self.up[v][i]

        return self.up[u][0]
```

### Modern C++ (Cache-Optimized Binary Lifting)

```cpp
#include <vector>
#include <cmath>
#include <algorithm>

class BinaryLiftingLCA {
private:
    int n_, log_;
    std::vector<int> depth_;
    std::vector<std::vector<int>> up_;

    void dfs(int u, int p, int d, const std::vector<std::vector<int>>& adj) {
        depth_[u] = d;
        up_[u][0] = p;
        for (int i = 1; i < log_; ++i) {
            up_[u][i] = up_[up_[u][i - 1]][i - 1];
        }
        for (int v : adj[u]) {
            if (v != p) dfs(v, u, d + 1, adj);
        }
    }

public:
    BinaryLiftingLCA(int n, int root, const std::vector<std::vector<int>>& adj)
        : n_(n), log_(std::ceil(std::log2(std::max(n, 2))) + 1),
          depth_(n + 1, 0), up_(n + 1, std::vector<int>(log_, 0)) {
        dfs(root, root, 0, adj);
    }

    int query(int u, int v) const {
        if (depth_[u] < depth_[v]) std::swap(u, v);

        int diff = depth_[u] - depth_[v];
        for (int i = 0; i < log_; ++i) {
            if ((diff >> i) & 1) u = up_[u][i];
        }
        if (u == v) return u;

        for (int i = log_ - 1; i >= 0; --i) {
            if (up_[u][i] != up_[v][i]) {
                u = up_[u][i];
                v = up_[v][i];
            }
        }
        return up_[u][0];
    }
};
```

---

## 9. Common Pitfalls & Edge Cases

1. **Stack Overflow on Deep Trees**: In Python and C++, running recursive DFS on degenerate trees (skewed chains of $N = 10^5$ nodes) will cause a call stack overflow. In production or competitive contests, increase recursion limit or use an iterative DFS with an explicit stack.
2. **Off-by-One in Logarithm**: Ensure `log_ = ceil(log2(N)) + 1`. If `log_` is too small, jumping $2^i$ will truncate powers of 2 for nodes near tree depth boundaries.
3. **Graph with Multiple Components**: If the input is a forest rather than a single connected tree, remember to run DFS from every unvisited root component.

---

## 10. Curated Problem Sets

* [CSES 1688: Company Queries II](https://cses.fi/problemset/task/1688) *(The pure canonical LCA problem)*
* [LeetCode 236: Lowest Common Ancestor of a Binary Tree](https://leetcode.com/problems/lowest-common-ancestor-of-a-binary-tree/) *(Medium)*
* [Codeforces 1304E: 1-Trees and Queries](https://codeforces.com/problemset/problem/1304/E) *(Distance between nodes under dynamic shortcut edges)*
* [SPOJ LCA: Lowest Common Ancestor](https://www.spoj.com/problems/LCA/) *(Classic RMQ benchmark)*
