---
title: "Master Topic Index & Global DSA Taxonomy"
difficulty: "All Levels"
domains: ["Reference", "Index", "Taxonomy"]
prerequisites: ["None"]
related_topics: ["Contest and Interview Mapping", "Roadmap", "Classic Papers Reading List"]
---

# Master Topic Index & Global DSA Taxonomy

## 1. Executive Summary & Repository Organization

The **Awesome-DSA** repository is an exhaustive, publication-grade knowledge base encompassing **154 topics** across **25 specialized domains**. Every chapter adheres to strict formal standards: mathematical definitions, formal invariants, ASCII/Mermaid layout diagrams, asymptotic complexity derivations, dual C++17 and Python 3 reference implementations, and differential test suites.

This master index provides:
1. **Alphabetical Concept Index**: Rapid lookup of data structures, algorithms, theorems, and mechanisms.
2. **Curated Glossary of Canonical Systems & Algorithm Acronyms**.
3. **Domain Taxonomy Matrix**: Comprehensive cross-referencing of all 25 domains with direct relative file links.

---

## 2. Master Glossary of Industry & Algorithmic Acronyms

| Acronym | Full Definition | Primary Subsystem / Domain |
| :--- | :--- | :--- |
| **ADS** | Authenticated Data Structures | Cryptographic / Merkle Proofs |
| **ARC** | Adaptive Replacement Cache | Systems Caching (Megiddo & Modha) |
| **AVL** | Adelson-Velsky and Landis Tree | Balanced Search Trees |
| **BWT** | Burrows-Wheeler Transform | Text Compression & Suffix Indexing |
| **CAS** | Compare-And-Swap | Lock-Free Concurrency |
| **CDQ** | Chen Danqi Divide-and-Conquer | Offline Multi-Dimensional Processing |
| **CFS** | Completely Fair Scheduler | Linux Kernel Task Scheduling |
| **CRDT** | Conflict-free Replicated Data Type | Distributed State Synchronization |
| **DCE** | Dead Code Elimination | Compiler Optimization & Benchmarking |
| **DP** | Dynamic Programming | Algorithmic Paradigms |
| **DSU** | Disjoint-Set Union (Union-Find) | Graph Connectivity & Equivalence |
| **EBR** | Epoch-Based Reclamation | Safe Concurrent Memory Reclamation |
| **FAA** | Fetch-And-Add | Wait-Free Atomic Primitive |
| **FLP** | Fischer-Lynch-Paterson Impossibility | Distributed Consensus |
| **FM-Index** | Ferragina-Manzini Index | Compressed Full-Text Substring Search |
| **HLL** | HyperLogLog | Cardinality Estimation Sketch |
| **IPC** | Instructions Per Cycle | Hardware Micro-Architecture |
| **KMP** | Knuth-Morris-Pratt | String Pattern Matching |
| **LCP** | Longest Common Prefix | Suffix Array String Processing |
| **LLC** | Last-Level Cache (L3) | CPU Memory Hierarchy |
| **LSM** | Log-Structured Merge-Tree | Database Storage Engines |
| **MST** | Minimum Spanning Tree | Graph Theory |
| **PACELC** | Partition, Availability, Consistency, Else Latency, Consistency | Distributed Systems Theorem |
| **PRAM** | Parallel Random Access Machine | Parallel Algorithm Analysis |
| **RAF** | Read Amplification Factor | Storage Engine Performance Metric |
| **RCU** | Read-Copy-Update | Linux Kernel Synchronization |
| **RUM** | Read, Update, Memory Conjecture | Storage Engine Trade-Off Space |
| **SAF** | Space Amplification Factor | Storage Engine Performance Metric |
| **SAM** | Suffix Automaton | Directed Acyclic Word Graph |
| **SCC** | Strongly Connected Components | Directed Graph Algorithms |
| **SLRU** | Segmented Least Recently Used | Multi-Tiered Cache Eviction |
| **SMR** | Safe Memory Reclamation | Concurrency (Hazard Pointers / EBR) |
| **SPMC** | Single-Producer Multi-Consumer | Work-Stealing Queues |
| **TLB** | Translation Lookaside Buffer | Virtual Memory Paging |
| **VFS** | Virtual File System | Operating System Kernel |
| **WAF** | Write Amplification Factor | Storage Engine Performance Metric |

---

## 3. Alphabetical Master Topic Directory

### A
- **Aho-Corasick Automaton**: Multiple string pattern matching in linear time. [`aho-corasick.md`](../11-strings-text-and-pattern-matching/aho-corasick.md)
- **Akra-Bazzi Method**: Solving general divide-and-conquer recurrences with non-uniform branch splits. [`akra-bazzi-method.md`](../02-analysis-and-complexity/akra-bazzi-method.md)
- **Amortized Analysis**: Aggregate, accounting, and physicist potential method derivations. [`amortized-analysis.md`](../02-analysis-and-complexity/amortized-analysis.md)
- **Andrew's Monotone Chain**: $O(N \log N)$ exact integer convex hull algorithm. [`convex-hull.md`](../13-geometric-and-spatial-algorithms/convex-hull.md)
- **Authenticated Data Structures**: Merkle treaps and cryptographic membership/non-membership proofs. [`authenticated-data-structures.md`](../17-cryptographic-and-merkle-like-structures/authenticated-data-structures.md)
- **AVL Tree**: Strict height-balanced binary search tree with single and double rotations. [`avl-trees.md`](../05-trees-and-hierarchical-structures/avl-trees.md)

### B
- **B-Trees & B+ Trees**: Slotted-page external memory balanced search trees. [`b-trees-and-variants.md`](../05-trees-and-hierarchical-structures/b-trees-and-variants.md)
- **Benchmark Design**: Optimization barriers, statistical percentiles, and hardware counter profiling. [`benchmark-design.md`](../22-benchmarking-and-tradeoffs/benchmark-design.md)
- **Binary Search & Variants**: Exact match, lower bound, upper bound, and float bisection. [`binary-search-and-variants.md`](../09-algorithm-design-paradigms/binary-search-and-variants.md)
- **Bit Manipulation Tricks**: Popcount, trailing zeros, bitwise subsets, and SWAR. [`bit-manipulation-tricks.md`](../03-machine-model-and-performance/bit-manipulation-tricks.md)
- **Bitcask**: Append-only log storage engine with in-memory keydir. [`storage-engines-breakdown.md`](../18-systems-case-studies/storage-engines-breakdown.md)
- **Bloom Filters**: Space-efficient probabilistic set membership. [`bloom-filters.md`](../07-hashing-randomization-and-probabilistic/bloom-filters.md)

### C
- **Cache-Oblivious Algorithms**: Matrix multiplication, van Emde Boas layouts, and funnelsort. [`cache-oblivious-algorithms.md`](../15-external-memory-cache-oblivious-and-streaming/cache-oblivious-algorithms.md)
- **CDQ Divide-and-Conquer**: Offline multi-dimensional partial order range querying. [`mo-and-cdq-divide-and-conquer.md`](../12-range-query-and-offline-structures/mo-and-cdq-divide-and-conquer.md)
- **CFS Completely Fair Scheduler**: Linux kernel augmented red-black tree with cached leftmost node. [`linux-kernel-internals.md`](../18-systems-case-studies/linux-kernel-internals.md)
- **Chase-Lev Deque**: Single-producer multi-consumer dynamic circular work-stealing deque. [`work-stealing-deques.md`](../16-parallel-concurrent-and-lock-free/work-stealing-deques.md)
- **Consistent Hashing**: Dynamo ring with virtual nodes and minimal key migration. [`distributed-state-engines.md`](../18-systems-case-studies/distributed-state-engines.md)
- **Count-Min Sketch**: Sub-linear frequency estimation with periodic time-decay resets. [`count-min-sketch.md`](../07-hashing-randomization-and-probabilistic/count-min-sketch.md)
- **Cuckoo Hashing**: Constant worst-case lookup with multiple hash functions and eviction cycles. [`cuckoo-hashing.md`](../07-hashing-randomization-and-probabilistic/cuckoo-hashing.md)

### D
- **Dijkstra's Algorithm**: Single-source shortest path with binary and Fibonacci heaps. [`dijkstra.md`](../08-graphs-and-network-algorithms/dijkstra.md)
- **Disjoint-Set Union (DSU)**: Path compression and union-by-rank with $\alpha(n)$ amortized bound. [`disjoint-set-union.md`](../04-linear-data-structures/disjoint-set-union.md)
- **Dynamic Programming Optimization**: Convex Hull Trick, Knuth, Divide & Conquer, and Aliens (WQS) binary search. [`convex-hull-trick.md`](../10-dynamic-programming/convex-hull-trick.md)

### E
- **Epoch-Based Reclamation (EBR)**: 3-epoch circular binning for lock-free memory reclamation. [`hazard-pointers-and-epoch-reclamation.md`](../16-parallel-concurrent-and-lock-free/hazard-pointers-and-epoch-reclamation.md)
- **Euler Tour Technique & Tree Flattening**: Subtree queries via interval mapping on linear arrays. [`tree-flattening-and-euler-tour.md`](../05-trees-and-hierarchical-structures/tree-flattening-and-euler-tour.md)

### F
- **Fenwick Tree (Binary Indexed Tree)**: Prefix sums, point updates, and range queries in $O(\log n)$. [`fenwick-tree.md`](../12-range-query-and-offline-structures/fenwick-tree.md)
- **Fibonacci Heap**: Amortized $O(1)$ decrease-key and insert with cascading cuts. [`fibonacci-heap.md`](../06-heaps-priority-and-selection/fibonacci-heap.md)
- **Flow Networks & Min-Cut**: Dinic's blocking flow and Push-Relabel algorithm with gap heuristic. [`dinic.md`](../08-graphs-and-network-algorithms/dinic.md), [`push-relabel.md`](../08-graphs-and-network-algorithms/push-relabel.md)
- **Fusion Trees**: Fredman-Willard $O(\log_W N)$ word-level parallelism predecessor search. [`fusion-trees.md`](../14-advanced-data-structures/fusion-trees.md)

### H
- **Hazard Pointers**: Thread-local pointer publication for bounded lock-free memory reclamation. [`hazard-pointers-and-epoch-reclamation.md`](../16-parallel-concurrent-and-lock-free/hazard-pointers-and-epoch-reclamation.md)
- **Heavy-Light Decomposition (HLD)**: Decomposing tree paths into continuous array intervals. [`heavy-light-decomposition.md`](../12-range-query-and-offline-structures/heavy-light-decomposition.md)
- **HyperLogLog**: Near-zero RAM cardinality estimation via leading-zero register tracking. [`hyperloglog.md`](../07-hashing-randomization-and-probabilistic/hyperloglog.md)

### K
- **K-D Tree**: Multi-dimensional orthogonal range search and nearest neighbor search. [`k-d-trees.md`](../13-geometric-and-spatial-algorithms/k-d-trees.md)
- **Knuth-Morris-Pratt (KMP)**: Deterministic linear-time single string pattern matching. [`knuth-morris-pratt.md`](../11-strings-text-and-pattern-matching/knuth-morris-pratt.md)

### L
- **Link-Cut Tree**: Dynamic tree connectivity, path aggregations, and root manipulation via splay trees. [`link-cut-tree.md`](../14-advanced-data-structures/link-cut-tree.md)
- **Linux Kernel Intrusive Lists**: `struct list_head` and `container_of` offset-of pointer arithmetic. [`linux-kernel-internals.md`](../18-systems-case-studies/linux-kernel-internals.md)
- **Lock-Free Queue (Michael-Scott)**: Sentinel node queue with cooperative helping CAS. [`concurrent-queues-and-stacks.md`](../16-parallel-concurrent-and-lock-free/concurrent-queues-and-stacks.md)
- **LSM-Tree**: Log-Structured Merge-Tree with tiered and leveled compaction. [`storage-engines-breakdown.md`](../18-systems-case-studies/storage-engines-breakdown.md)

### M
- **Manacher's Algorithm**: $O(N)$ linear-time discovery of all palindromic substrings. [`manacher-algorithm.md`](../11-strings-text-and-pattern-matching/manacher-algorithm.md)
- **Merkle Trees**: RFC 6962 cryptographic audit paths and tamper-proof log proofs. [`merkle-trees.md`](../17-cryptographic-and-merkle-like-structures/merkle-trees.md)
- **Mo's Algorithm**: Offline range queries with $\sqrt{N}$ block decomposition and Hilbert curves. [`mo-and-cdq-divide-and-conquer.md`](../12-range-query-and-offline-structures/mo-and-cdq-divide-and-conquer.md)

### P
- **Persistent Segment Tree**: Functional history tracking via immutable path copying. [`persistent-segment-tree.md`](../12-range-query-and-offline-structures/persistent-segment-tree.md)
- **PRAM & Work-Depth**: Brent's theorem, parallel slackness, and fork-join DAG scheduling. [`pram-and-work-depth.md`](../16-parallel-concurrent-and-lock-free/pram-and-work-depth.md)

### R
- **Raft Consensus**: Leader election, replicated append-only logs, and quorum commit safety. [`distributed-state-engines.md`](../18-systems-case-studies/distributed-state-engines.md)
- **Read-Copy-Update (RCU)**: Lockless reader scaling via quiescent state grace period tracking. [`linux-kernel-internals.md`](../18-systems-case-studies/linux-kernel-internals.md)
- **Red-Black Tree**: Isomorphic binary representation of 2-3-4 B-trees. [`red-black-trees.md`](../05-trees-and-hierarchical-structures/red-black-trees.md)

### S
- **Segment Tree**: Point update, range query, and lazy propagation range updates in $O(\log n)$. [`segment-tree.md`](../12-range-query-and-offline-structures/segment-tree.md)
- **Skip List**: Probabilistic balanced dictionary using geometric coin flipping. [`skip-lists.md`](../04-linear-data-structures/skip-lists.md)
- **Splay Tree**: Self-adjusting binary search tree with amortized $O(\log n)$ access lemma. [`splay-trees.md`](../05-trees-and-hierarchical-structures/splay-trees.md)
- **Succinct Rank/Select Bitvectors**: Jacobson $o(N)$ summary directories supporting $O(1)$ rank. [`succinct-structures.md`](../14-advanced-data-structures/succinct-structures.md)
- **Suffix Automaton (SAM)**: Minimal directed acyclic word graph encoding all string substrings. [`suffix-automaton.md`](../11-strings-text-and-pattern-matching/suffix-automaton.md)

### T
- **Treiber Stack**: Lock-free concurrent LIFO stack with CAS and ABA tagged pointers. [`concurrent-queues-and-stacks.md`](../16-parallel-concurrent-and-lock-free/concurrent-queues-and-stacks.md), [`aba-problem.md`](../16-parallel-concurrent-and-lock-free/aba-problem.md)
- **Treap**: Randomized search tree combining binary search tree keys with heap priorities. [`treaps.md`](../05-trees-and-hierarchical-structures/treaps.md)

### U
- **Ukkonen's Algorithm**: Online $O(N)$ linear-time suffix tree construction. [`ukkonen-suffix-tree.md`](../11-strings-text-and-pattern-matching/ukkonen-suffix-tree.md)

### V
- **van Emde Boas Tree**: Predecessor and successor queries in $O(\log \log U)$ time. [`van-emde-boas-tree.md`](../14-advanced-data-structures/van-emde-boas-tree.md)
- **Vector Clocks**: Partial order causality tracking and concurrent divergence resolution. [`distributed-state-engines.md`](../18-systems-case-studies/distributed-state-engines.md)

### W
- **Window TinyLFU (W-TinyLFU)**: 4-bit Count-Min admission filter with segmented LRU main cache. [`high-performance-caching.md`](../18-systems-case-studies/high-performance-caching.md)

---

## 4. The 25 Domains Overview

```
+---------------------------------------------------------------------------------+
| Domain                                  | Core Specialization                  |
|-----------------------------------------+---------------------------------------|
| 00-learning-paths                       | Role-oriented mastery paths           |
| 01-mathematical-foundations             | Discrete math, probability & bounds   |
| 02-analysis-and-complexity              | Asymptotics, master theorem & models  |
| 03-machine-model-and-performance        | CPU caches, memory layout & bits      |
| 04-linear-data-structures               | Vectors, lists, stacks & queues       |
| 05-trees-and-hierarchical-structures    | AVL, Red-Black, Splay, B-Trees & DSU  |
| 06-heaps-priority-and-selection         | Binary, Binomial, Fibonacci & Pairing |
| 07-hashing-randomization-and-probabilistic | Robin Hood, Cuckoo, Bloom & HLL    |
| 08-graphs-and-network-algorithms        | Shortest path, MST, Flows & Cuts      |
| 09-algorithm-design-paradigms           | Divide-and-conquer, Greedy & Search   |
| 10-dynamic-programming                  | Bitmask, Tree, Interval & Convex Hull |
| 11-strings-text-and-pattern-matching    | KMP, Aho-Corasick, Suffix Tree & SAM  |
| 12-range-query-and-offline-structures   | Segment Trees, Fenwick, HLD & Mo      |
| 13-geometric-and-spatial-algorithms     | Convex Hull, Sweep-line, K-D & Voronoi|
| 14-advanced-data-structures             | vEB, Fusion, Link-Cut & Succinct      |
| 15-external-memory-and-streaming        | I/O Model, Buffer Trees & Oblivious   |
| 16-parallel-concurrent-and-lock-free    | Treiber, M&S, Hazard Pointers & Chase |
| 17-cryptographic-and-merkle-structures  | SHA-256 Merkle & Authenticated Treap  |
| 18-systems-case-studies                 | Storage Engines, Linux, Caching, Raft |
| 19-problem-solving-patterns             | Sliding Window, Two Pointers, Monotone|
| 20-proof-techniques-and-correctness     | Induction, Invariants & Cut-Cycle     |
| 21-implementation-engineering           | API Design, Sanitizers & Fuzzing      |
| 22-benchmarking-and-tradeoffs           | Hardware Barriers & Statistical Stats |
| 23-history-papers-and-classics          | 40-Paper Canon & Landmark Chronology  |
| 24-exercises-and-curated-problems       | Global Taxonomy & Problem Mapping     |
+---------------------------------------------------------------------------------+
```
