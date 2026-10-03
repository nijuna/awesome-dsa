---
title: "Contest and Interview Problem Mapping: Curated Practice Matrix"
difficulty: "All Levels"
domains: ["Practice", "Interviews", "Competitive Programming", "Systems"]
prerequisites: ["Core Data Structures", "Algorithm Design Paradigms"]
related_topics: ["Topic Index", "Roadmap", "Landmark Data Structures"]
---

# Contest and Interview Problem Mapping: Curated Practice Matrix

## 1. Executive Summary & Deliberate Practice Philosophy

Studying data structures and algorithms theoretically is necessary, but **deliberate, feedback-driven practice** is what turns conceptual knowledge into production-grade intuition.

Engineers prepare for distinct evaluation environments:
- **Technical Interviews (FAANG / Big Tech / High-Growth Startups)**: Emphasis on clean API boundaries, edge case rigor, invariant explanation, communicative clarity, and time/space optimal Big-O.
- **Competitive Programming (Codeforces, AtCoder, ICPC)**: Extreme time constraints, high-stress problem reduction, exotic data structure combinations (e.g. CDQ divide-and-conquer, Suffix Automata, Aliens trick), and constant-factor optimization.
- **Systems Engineering Interviews**: Low-level concurrency (lock-free stacks, memory reclamation), storage engine layouts (LSM vs B+Tree), cache replacement, and distributed consensus (Raft, Consistent Hashing).

This chapter provides a **comprehensive curriculum cross-referencing this repository's 154 topics with curated problems across LeetCode, Codeforces, CSES, and Systems Design challenges**.

---

## 2. Platform Difficulty Calibration Matrix

```
+---------------------------------------------------------------------------------+
| Skill Tier   | LeetCode Tier   | Codeforces Rating | CSES Problem Sections      |
|--------------+-----------------+-------------------+----------------------------|
| Beginner     | Easy / Med 1-3  | 800 - 1200        | Introductory & Basic Sort  |
| Intermediate | Medium 4-5      | 1300 - 1600       | Sorting, Basic DP, Graphs  |
| Advanced     | Hard (Standard) | 1700 - 2100       | Range Queries, Tree, Flow  |
| Master       | Hard (Top 1%)   | 2200 - 2600       | Geometry, Strings, Adv DP  |
| Grandmaster  | Contest Winner  | 2700+             | Advanced Techniques / CDQ  |
+---------------------------------------------------------------------------------+
```

---

## 3. Domain-by-Domain Curated Problem Mapping

### 3.1 Linear Structures, Two Pointers & Monotonic Stacks
*Relevant Chapters*: [`arrays-and-memory-layout.md`](../04-linear-data-structures/arrays-and-memory-layout.md), [`monotonic-queue-and-stack.md`](../19-problem-solving-patterns/monotonic-stack-and-queue.md), [`two-pointers.md`](../19-problem-solving-patterns/two-pointers.md), [`sliding-window.md`](../19-problem-solving-patterns/sliding-window.md).

| Platform | Problem Name | Core Mechanism Tested | Difficulty |
| :--- | :--- | :--- | :--- |
| **LeetCode** | 42. Trapping Rain Water | Two Pointers / Monotonic Stack | Hard |
| **LeetCode** | 84. Largest Rectangle in Histogram | Monotonic Increasing Stack | Hard |
| **LeetCode** | 239. Sliding Window Maximum | Monotonic Double-Ended Queue | Hard |
| **CSES** | Nearest Smaller Values | Monotonic Stack Predecessor | 1300 |
| **CSES** | Subarray Sums I & II | Prefix Sums + Hash Map | 1400 |
| **Codeforces** | 1311B. WeirdSort | Inversion Invariant & Array Sorting | 1200 |

---

### 3.2 Trees, Hierarchies & Disjoint-Set Union (DSU)
*Relevant Chapters*: [`binary-search-trees.md`](../05-trees-and-hierarchical-structures/binary-search-trees.md), [`tree-flattening-and-euler-tour.md`](../05-trees-and-hierarchical-structures/tree-basics-and-traversals.md), [`disjoint-set-union.md`](../04-linear-data-structures/disjoint-set-union.md).

| Platform | Problem Name | Core Mechanism Tested | Difficulty |
| :--- | :--- | :--- | :--- |
| **LeetCode** | 236. Lowest Common Ancestor of a Binary Tree | Tree DFS / Binary Lifting | Medium |
| **LeetCode** | 684. Redundant Connection | Disjoint-Set Union Cycle Detection | Medium |
| **LeetCode** | 827. Making A Large Island | 2D DSU Component Merging | Hard |
| **CSES** | Tree Diameter | 2-Pass BFS or Tree DP | 1400 |
| **CSES** | Company Queries I & II | Binary Lifting ($O(\log N)$ Ancestor) | 1600 |
| **CSES** | Subtree Queries | Euler Tour + Fenwick Tree | 1700 |
| **Codeforces** | 1213G. Path Queries | Offline Query Sorting + DSU | 1800 |

---

### 3.3 Graph Algorithms, Shortest Paths & Maximum Flow
*Relevant Chapters*: [`dijkstra.md`](../08-graphs-and-network-algorithms/shortest-paths.md), [`tarjan-scc.md`](../08-graphs-and-network-algorithms/strongly-connected-components.md), [`dinic.md`](../08-graphs-and-network-algorithms/network-flow-and-push-relabel.md), [`push-relabel.md`](../08-graphs-and-network-algorithms/network-flow-and-push-relabel.md).

| Platform | Problem Name | Core Mechanism Tested | Difficulty |
| :--- | :--- | :--- | :--- |
| **LeetCode** | 743. Network Delay Time | Dijkstra with Min-Heap | Medium |
| **LeetCode** | 787. Cheapest Flights Within K Stops | Bellman-Ford / BFS Relaxation | Medium |
| **LeetCode** | 1192. Critical Connections in a Network | Tarjan's Bridge-Finding Algorithm | Hard |
| **CSES** | Shortest Routes I & II | Dijkstra & Floyd-Warshall | 1400 / 1500 |
| **CSES** | Flight Discount | Dijkstra on State-Graph ($V \times 2$) | 1600 |
| **CSES** | Download Speed | Dinic's Maximum Network Flow | 1800 |
| **CSES** | Planets and Kingdoms | Tarjan / Kosaraju SCC | 1700 |
| **Codeforces** | 1082G. Petya and Graph | Max-Flow Min-Cut Reduction | 2200 |

---

### 3.4 Dynamic Programming & Advanced Optimizations
*Relevant Chapters*: [`bitmask-dp.md`](../10-dynamic-programming/bitmask-and-state-compression.md), [`tree-dp.md`](../10-dynamic-programming/tree-dp.md), [`convex-hull-trick.md`](../13-geometric-and-spatial-algorithms/convex-hull.md), [`knuth-and-divide-and-conquer-optimization.md`](../09-algorithm-design-paradigms/divide-and-conquer.md).

| Platform | Problem Name | Core Mechanism Tested | Difficulty |
| :--- | :--- | :--- | :--- |
| **LeetCode** | 312. Burst Balloons | Interval Matrix Chain DP | Hard |
| **LeetCode** | 847. Shortest Path Visiting All Nodes | BFS on Bitmask State Graph | Hard |
| **LeetCode** | 188. Best Time to Buy and Sell Stock IV | State Machine Multi-Transaction DP | Hard |
| **CSES** | Money Sums | Subset Sum 0-1 Knapsack DP | 1400 |
| **CSES** | Projects | Coordinate Compression + DP Binary Search | 1500 |
| **CSES** | Elevator Rides | Bitmask DP ($2^N \times N$) | 1800 |
| **Codeforces** | 319C. Kalila and Dimna in the Logging Industry | Convex Hull Trick ($O(N \log N)$) | 2100 |
| **Codeforces** | 868F. Yet Another Minimization Problem | Divide-and-Conquer DP Optimization | 2300 |

---

### 3.5 Range Queries, Fenwick & Segment Trees
*Relevant Chapters*: [`fenwick-tree.md`](../05-trees-and-hierarchical-structures/fenwick-trees.md), [`segment-tree.md`](../05-trees-and-hierarchical-structures/segment-trees.md), [`heavy-light-decomposition.md`](../08-graphs-and-network-algorithms/lowest-common-ancestor.md), [`mo-and-cdq-divide-and-conquer.md`](../12-range-query-and-offline-structures/mo-algorithm.md).

| Platform | Problem Name | Core Mechanism Tested | Difficulty |
| :--- | :--- | :--- | :--- |
| **LeetCode** | 307. Range Sum Query - Mutable | Fenwick Tree / Segment Tree | Medium |
| **LeetCode** | 315. Count of Smaller Numbers After Self | Inversion Counting via BIT / Merge Sort | Hard |
| **LeetCode** | 218. The Skyline Problem | Sweep-Line + Max Priority Queue | Hard |
| **CSES** | Range Update Queries | Difference Array or Lazy Segment Tree | 1500 |
| **CSES** | Hotel Queries | Segment Tree Binary Search (Walk on Tree) | 1700 |
| **CSES** | Path Queries | Heavy-Light Decomposition (HLD) | 2000 |
| **Codeforces** | 86D. Powerful array | Mo's Algorithm with $\sqrt{N}$ Blocks | 2200 |
| **Codeforces** | 1093E. Intersection of Permutations | 2D Fenwick / CDQ Divide-and-Conquer | 2400 |

---

### 3.6 String Algorithms, Suffix Structures & Automata
*Relevant Chapters*: [`knuth-morris-pratt.md`](../11-strings-text-and-pattern-matching/prefix-function-and-kmp.md), [`aho-corasick.md`](../11-strings-text-and-pattern-matching/aho-corasick.md), [`ukkonen-suffix-tree.md`](../11-strings-text-and-pattern-matching/suffix-tree.md), [`suffix-automaton.md`](../11-strings-text-and-pattern-matching/suffix-automaton.md).

| Platform | Problem Name | Core Mechanism Tested | Difficulty |
| :--- | :--- | :--- | :--- |
| **LeetCode** | 28. Find the Index of the First Occurrence | KMP / Robin-Karp Rolling Hash | Easy/Med |
| **LeetCode** | 214. Shortest Palindrome | KMP Failure Function on $S + \# + S^R$ | Hard |
| **LeetCode** | 1032. Stream of Characters | Aho-Corasick or Reverse Trie | Hard |
| **CSES** | String Matching | KMP / Z-Algorithm | 1300 |
| **CSES** | Finding Borders | KMP $\pi$-array Traversal | 1400 |
| **CSES** | Word Combinations | Aho-Corasick + DP | 1900 |
| **CSES** | Suffix Array Construction | $O(N \log N)$ Doubling Suffix Array | 2100 |
| **Codeforces** | 700E. Cool Slogans | Suffix Automaton + Segment Tree Merging | 2700 |

---

### 3.7 Computational Geometry & Spatial Search
*Relevant Chapters*: [`convex-hull.md`](../13-geometric-and-spatial-algorithms/convex-hull.md), [`line-sweep.md`](../13-geometric-and-spatial-algorithms/line-sweep.md), [`k-d-trees.md`](../13-geometric-and-spatial-algorithms/kd-trees.md).

| Platform | Problem Name | Core Mechanism Tested | Difficulty |
| :--- | :--- | :--- | :--- |
| **LeetCode** | 587. Erect the Fence | Andrew's Monotone Chain Convex Hull | Hard |
| **LeetCode** | 892. Surface Area of 3D Shapes | Orthogonal Coordinate Geometry | Medium |
| **CSES** | Point Location Test | Cross Product / 2D Orientation Predicate | 1400 |
| **CSES** | Line Segment Intersection | Orientation Predicates & Collinearity | 1600 |
| **CSES** | Polygon Area | Shoelace Formula (__int128 exact) | 1500 |
| **CSES** | Convex Hull | Andrew's Monotone Chain | 1700 |
| **CSES** | Minimum Euclidean Distance | Line Sweep / Divide-and-Conquer | 1900 |

---

### 3.8 Systems Design, Concurrency & High-Throughput Engineering
*Relevant Chapters*: [`concurrent-queues-and-stacks.md`](../16-parallel-concurrent-and-lock-free/concurrent-queues-and-stacks.md), [`storage-engines-breakdown.md`](../18-systems-case-studies/storage-engines-breakdown.md), [`linux-kernel-internals.md`](../18-systems-case-studies/linux-kernel-internals.md), [`high-performance-caching.md`](../18-systems-case-studies/high-performance-caching.md), [`distributed-state-engines.md`](../18-systems-case-studies/distributed-state-engines.md).

| Real-World Scenario | Core Architectural Problem | Reference Chapter |
| :--- | :--- | :--- |
| **High-Throughput Key-Value Store** | LSM-Tree vs B+Tree Slotted Pages & WAF/RAF | [`storage-engines-breakdown.md`](../18-systems-case-studies/storage-engines-breakdown.md) |
| **Lock-Free MPSC Job Scheduler** | Michael-Scott Queue with Sentinel Node & CAS | [`concurrent-queues-and-stacks.md`](../16-parallel-concurrent-and-lock-free/concurrent-queues-and-stacks.md) |
| **Safe Node Reclamation in Concurrency** | Hazard Pointers vs Epoch-Based Reclamation | [`hazard-pointers-and-epoch-reclamation.md`](../16-parallel-concurrent-and-lock-free/hazard-pointers-and-epoch-reclamation.md) |
| **Multi-Core Work Stealing Engine** | Chase-Lev SPMC Deque with Memory Fences | [`work-stealing-deques.md`](../16-parallel-concurrent-and-lock-free/work-stealing-deques.md) |
| **Scan-Resistant In-Memory Cache** | W-TinyLFU with 4-bit Count-Min Sketch | [`high-performance-caching.md`](../18-systems-case-studies/high-performance-caching.md) |
| **Distributed Sharded Cluster** | Consistent Hashing Ring with Virtual Nodes | [`distributed-state-engines.md`](../18-systems-case-studies/distributed-state-engines.md) |
| **Cryptographic Audit Proof Service** | Merkle Trees with RFC 6962 Domain Separation | [`merkle-trees.md`](../17-cryptographic-and-merkle-like-structures/merkle-trees.md) |

---

## 4. The Deliberate Practice Protocol

```
Step 1: The Cold Attempt (20 - 30 minutes)
- Read problem statement carefully. Identify constraints.
- Formulate invariant hypothesis. Trace small examples on paper.
- DO NOT look at hints or editorial!

Step 2: The Blocked Pivot (30 - 45 minutes)
- If stuck after 30 minutes:
  1. Simplify input: What if N is small? What if the graph is a tree?
  2. Invert perspective: Can we solve the problem backwards from the goal?
  3. Classify paradigm: Is it Greedy, DP, Network Flow, or Sweep-Line?

Step 3: The Educational Unblock
- Read ONLY the high-level tag or the first sentence of editorial.
- Attempt full implementation independently.

Step 4: The Master Review (Post-Solve)
- Never move on immediately after getting "Accepted"!
- Inspect top solutions: Did someone achieve a cleaner invariant?
- Can memory footprint be halved? Can recursion be eliminated?
- Record key takeaway in personal engineering notebook.
```
