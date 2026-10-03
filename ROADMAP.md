# The DSA Handbook Master Curriculum Syllabus & Topic Directory

> An architectural syllabus and topic index cataloging all 154 chapters across the 25 core knowledge domains of **The DSA Handbook**. Every chapter includes invariant-first formal specifications, asymptotic proofs, memory models, and verified implementations in Python and C++17.

---

## Phase 1: Foundations & Core Structures

### [00] Learning Paths
*Structured navigational curricula tailored to specific professional backgrounds.*

* [Beginner Learning Path: Zero to Algorithmic Competence](docs/00-learning-paths/beginner-path.md)
* [Competitive Programming Learning Path: From Div 2 to Candidate Master & Beyond](docs/00-learning-paths/competitive-programming-path.md)
* [Technical Interview Learning Path: High-Yield Patterns & Communication Protocol](docs/00-learning-paths/interview-path.md)
* [Systems Engineer Learning Path: Low-Level Architecture & High-Throughput Engines](docs/00-learning-paths/systems-engineer-path.md)
* [Theoretical Computer Science Learning Path: Proofs, Asymptotics & Complexity](docs/00-learning-paths/theory-path.md)

### [01] Mathematical Foundations
*Discrete mathematics, formal proof techniques, combinatorics, and probability.*

* [Combinatorics](docs/01-mathematical-foundations/combinatorics.md)
* [Expected Value & Random Variables in Algorithm Analysis](docs/01-mathematical-foundations/expected-value-and-random-variables.md)
* [Generating Functions: Combinatorial Clotheslines & Recurrence Solvers](docs/01-mathematical-foundations/generating-functions-intuition.md)
* [Linear Algebra for Algorithms: Matrices, Spectral Graphs & XOR Bases](docs/01-mathematical-foundations/linear-algebra-for-algorithms.md)
* [Logic and Proof Techniques](docs/01-mathematical-foundations/logic-and-proof-techniques.md)
* [Number Theory Basics](docs/01-mathematical-foundations/number-theory-basics.md)
* [Probability Basics](docs/01-mathematical-foundations/probability-basics.md)
* [Recurrence Relations](docs/01-mathematical-foundations/recurrence-relations.md)
* [Sets, Functions, and Relations](docs/01-mathematical-foundations/sets-functions-relations.md)
* [Summations and Series](docs/01-mathematical-foundations/summations-and-series.md)

### [02] Analysis & Complexity
*Asymptotics, potential method amortized analysis, Akra-Bazzi, and lower bounds.*

* [The Akra-Bazzi Method: Solving General Divide-and-Conquer Recurrences](docs/02-analysis-and-complexity/akra-bazzi-method.md)
* [Amortized Analysis](docs/02-analysis-and-complexity/amortized-analysis.md)
* [Asymptotic Analysis](docs/02-analysis-and-complexity/asymptotic-analysis.md)
* [Lower Bounds & Adversary Arguments: Proving Algorithmic Limits](docs/02-analysis-and-complexity/lower-bounds-and-adversaries.md)
* [Master Theorem and Beyond](docs/02-analysis-and-complexity/master-theorem-and-beyond.md)
* [Randomized Analysis: Concentration Bounds, Las Vegas & Monte Carlo](docs/02-analysis-and-complexity/randomized-analysis.md)
* [Reductions and Hardness Intuition: P, NP, and Fine-Grained Complexity](docs/02-analysis-and-complexity/reductions-and-hardness-intuition.md)
* [Worst, Average, and Smoothed Analysis: Beyond Pessimism](docs/02-analysis-and-complexity/worst-average-smoothed-analysis.md)

### [03] Machine Model & Performance
*Physical hardware realities: L1/L2/L3 cache hierarchies, branch predictors, SIMD, and memory allocators.*

* [Benchmarking Pitfalls and Measurement Science](docs/03-machine-model-and-performance/benchmarking-pitfalls.md)
* [Branch Prediction and CPU Pipelines](docs/03-machine-model-and-performance/branch-prediction-and-pipelines.md)
* [CPU Cache, Memory Hierarchy & Data Locality](docs/03-machine-model-and-performance/cpu-cache-and-memory.md)
* [Locality and Data-Oriented Design](docs/03-machine-model-and-performance/locality-and-data-oriented-design.md)
* [Memory Allocation and Fragmentation](docs/03-machine-model-and-performance/memory-allocation-and-fragmentation.md)
* [RAM Model vs Real Machines](docs/03-machine-model-and-performance/ram-model-vs-real-machines.md)
* [SIMD and Vectorization Intuition](docs/03-machine-model-and-performance/simd-and-vectorization-intuition.md)

### [04] Linear Data Structures
*Contiguous and node-based linear memory layouts, ring buffers, and disjoint sets.*

* [Arrays and Memory Layout](docs/04-linear-data-structures/arrays-and-memory-layout.md)
* [Bitsets and Bitvectors](docs/04-linear-data-structures/bitsets-and-bitvectors.md)
* [Deques (Double-Ended Queues)](docs/04-linear-data-structures/deques.md)
* [Disjoint Set Union](docs/04-linear-data-structures/disjoint-set-union.md)
* [Dynamic Arrays and Strings](docs/04-linear-data-structures/dynamic-arrays-and-strings.md)
* [Linked Lists](docs/04-linear-data-structures/linked-lists.md)
* [Ring Buffers](docs/04-linear-data-structures/ring-buffers.md)
* [Ropes, Gap Buffers, and Piece Tables](docs/04-linear-data-structures/ropes-gap-buffers-piece-tables.md)
* [Stacks and Queues](docs/04-linear-data-structures/stacks-and-queues.md)

### [05] Trees & Hierarchical Structures
*Search trees, balanced hierarchies, multiway trees, and prefix trees.*

* [AVL and Red-Black Trees](docs/05-trees-and-hierarchical-structures/avl-and-red-black-trees.md)
* [B-Trees & B+ Trees: Principles, Paging & Storage Engines](docs/05-trees-and-hierarchical-structures/b-trees-and-b-plus-trees.md)
* [Binary Search Trees](docs/05-trees-and-hierarchical-structures/binary-search-trees.md)
* [Fenwick Trees (Binary Indexed Trees): Theory, Bitwise Mechanics & Multi-Dimensional Variants](docs/05-trees-and-hierarchical-structures/fenwick-trees.md)
* [Interval Trees](docs/05-trees-and-hierarchical-structures/interval-trees.md)
* [Order-Statistic Trees](docs/05-trees-and-hierarchical-structures/order-statistic-trees.md)
* [Scapegoat and AA Trees](docs/05-trees-and-hierarchical-structures/scapegoat-and-aa-trees.md)
* [Segment Trees: Interval Decomposition, Lazy Propagation & Range Algebra](docs/05-trees-and-hierarchical-structures/segment-trees.md)
* [Splay Trees](docs/05-trees-and-hierarchical-structures/splay-trees.md)
* [Treaps (Cartesian Trees)](docs/05-trees-and-hierarchical-structures/treaps.md)
* [Tree Basics and Traversals](docs/05-trees-and-hierarchical-structures/tree-basics-and-traversals.md)
* [Tries and Radix Trees](docs/05-trees-and-hierarchical-structures/tries-and-radix-trees.md)

### [06] Heaps, Priority & Selection
*Priority queues, mergeable heaps, linear-time selection, and dynamic medians.*

* [Binary Heaps](docs/06-heaps-priority-and-selection/binary-heaps.md)
* [Binomial Heaps](docs/06-heaps-priority-and-selection/binomial-heaps.md)
* [d-ary Heaps](docs/06-heaps-priority-and-selection/d-ary-heaps.md)
* [Fibonacci Heaps](docs/06-heaps-priority-and-selection/fibonacci-heaps.md)
* [Indexed Priority Queues](docs/06-heaps-priority-and-selection/indexed-priority-queues.md)
* [Median Maintenance](docs/06-heaps-priority-and-selection/median-maintenance.md)
* [Pairing Heaps](docs/06-heaps-priority-and-selection/pairing-heaps.md)
* [Priority Queues in Practice](docs/06-heaps-priority-and-selection/priority-queues-in-practice.md)
* [Quickselect and Median of Medians](docs/06-heaps-priority-and-selection/quickselect-and-median-of-medians.md)

### [07] Hashing, Randomization & Probabilistic Structures
*Open addressing, collision resolution, membership filters, and cardinality estimators.*

* [Bloom and Cuckoo Filters](docs/07-hashing-randomization-and-probabilistic/bloom-and-cuckoo-filters.md)
* [Consistent Hashing and Distributed Partitioning](docs/07-hashing-randomization-and-probabilistic/consistent-hashing.md)
* [Hash Functions](docs/07-hashing-randomization-and-probabilistic/hash-functions.md)
* [Hash Tables and Collisions](docs/07-hashing-randomization-and-probabilistic/hash-tables-and-collisions.md)
* [HyperLogLog and Cardinality Estimation](docs/07-hashing-randomization-and-probabilistic/hyperloglog.md)
* [Robin Hood, Cuckoo, and Hopscotch Hashing](docs/07-hashing-randomization-and-probabilistic/robin-hood-cuckoo-and-hopscotch-hashing.md)
* [Skip Lists](docs/07-hashing-randomization-and-probabilistic/skip-lists.md)

### [08] Graphs & Network Algorithms
*Graph traversals, shortest path DAGs, minimum spanning trees, and network flow.*

* [All-Pairs Shortest Paths](docs/08-graphs-and-network-algorithms/all-pairs-shortest-paths.md)
* [BFS DFS and Traversal Patterns](docs/08-graphs-and-network-algorithms/bfs-dfs-and-traversal-patterns.md)
* [Graph Representations](docs/08-graphs-and-network-algorithms/graph-representations.md)
* [Lowest Common Ancestor (LCA): Algorithms, RMQ Reduction & Systems Applications](docs/08-graphs-and-network-algorithms/lowest-common-ancestor.md)
* [Minimum Spanning Trees](docs/08-graphs-and-network-algorithms/minimum-spanning-trees.md)
* [Network Flow and the Push-Relabel Algorithm](docs/08-graphs-and-network-algorithms/network-flow-and-push-relabel.md)
* [Shortest Paths](docs/08-graphs-and-network-algorithms/shortest-paths.md)
* [Strongly Connected Components](docs/08-graphs-and-network-algorithms/strongly-connected-components.md)
* [Topological Sort](docs/08-graphs-and-network-algorithms/topological-sort.md)

### [09] Algorithm Design Paradigms
*Universal algorithmic strategies: divide & conquer, greedy, backtracking, and meet-in-the-middle.*

* [Branch and Bound](docs/09-algorithm-design-paradigms/branch-and-bound.md)
* [Divide and Conquer](docs/09-algorithm-design-paradigms/divide-and-conquer.md)
* [Dynamic Programming Intuition](docs/09-algorithm-design-paradigms/dynamic-programming-intuition.md)
* [Greedy Algorithms](docs/09-algorithm-design-paradigms/greedy-algorithms.md)
* [Meet-in-the-Middle](docs/09-algorithm-design-paradigms/meet-in-the-middle.md)
* [Recursion and Backtracking](docs/09-algorithm-design-paradigms/recursion-and-backtracking.md)

### [10] Dynamic Programming
*Optimal substructure, memoization, tree DP, and state compression.*

* [1D and 2D Dynamic Programming Foundations](docs/10-dynamic-programming/1d-and-2d-foundations.md)
* [Bitmask and State Compression](docs/10-dynamic-programming/bitmask-and-state-compression.md)
* [Edit Distance and Sequence Alignment](docs/10-dynamic-programming/edit-distance-and-sequence-alignment.md)
* [Interval and Matrix DP](docs/10-dynamic-programming/interval-and-matrix-dp.md)
* [The Knapsack Problem Family](docs/10-dynamic-programming/knapsack-family.md)
* [Longest Common Subsequence](docs/10-dynamic-programming/longest-common-subsequence.md)
* [Longest Increasing Subsequence](docs/10-dynamic-programming/longest-increasing-subsequence.md)
* [Tree DP](docs/10-dynamic-programming/tree-dp.md)

### [19] Problem-Solving Patterns
*Canonical algorithmic patterns: two pointers, sliding window, and monotonic structures.*

* [Binary Search on Answer](docs/19-problem-solving-patterns/binary-search-on-answer.md)
* [Interval Scheduling](docs/19-problem-solving-patterns/interval-scheduling.md)
* [Monotonic Stack and Queue](docs/19-problem-solving-patterns/monotonic-stack-and-queue.md)
* [Sliding Window](docs/19-problem-solving-patterns/sliding-window.md)
* [Two Pointers: Converging, Partitioning & Fast-Slow Pointers](docs/19-problem-solving-patterns/two-pointers.md)

---

## Phase 2: Advanced Structures & Text Processing

### [11] Strings, Text & Pattern Matching
*Exact string matching, border arrays, suffix data structures, and text automata.*

* [Aho-Corasick](docs/11-strings-text-and-pattern-matching/aho-corasick.md)
* [Manacher's Algorithm](docs/11-strings-text-and-pattern-matching/manacher-algorithm.md)
* [Prefix Function and KMP](docs/11-strings-text-and-pattern-matching/prefix-function-and-kmp.md)
* [Rabin-Karp and Rolling Hash](docs/11-strings-text-and-pattern-matching/rabin-karp-and-rolling-hash.md)
* [Suffix Arrays and LCP](docs/11-strings-text-and-pattern-matching/suffix-arrays-and-lcp.md)
* [Suffix Automaton (DAWG)](docs/11-strings-text-and-pattern-matching/suffix-automaton.md)
* [Suffix Tree: Ukkonen's Online Linear-Time Construction and Suffix Links](docs/11-strings-text-and-pattern-matching/suffix-tree.md)
* [Z-Algorithm and String Borders](docs/11-strings-text-and-pattern-matching/z-algorithm.md)

### [12] Range Queries & Offline Structures
*Static and dynamic range query engines, lazy propagation, and Mo's query decomposition.*

* [Mo's Algorithm: Query Square Root Decomposition, Hilbert Curves, and Offline Range Queries](docs/12-range-query-and-offline-structures/mo-algorithm.md)
* [Offline Query Processing: CDQ Divide-and-Conquer, Sweep-Line Structures, and Multi-Dimensional Reductions](docs/12-range-query-and-offline-structures/offline-query-processing.md)
* [Prefix Sums and Difference Arrays](docs/12-range-query-and-offline-structures/prefix-sums-and-difference-arrays.md)
* [Range Minimum Query](docs/12-range-query-and-offline-structures/range-minimum-query.md)
* [Range Updates and Lazy Propagation](docs/12-range-query-and-offline-structures/range-updates-and-lazy-propagation.md)
* [Sparse Tables](docs/12-range-query-and-offline-structures/sparse-tables.md)
* [Square Root (Sqrt) Decomposition](docs/12-range-query-and-offline-structures/sqrt-decomposition.md)

### [13] Geometric & Spatial Algorithms
*Computational geometry primitives, convex hulls, line sweep, and spatial partitioning.*

* [Closest Pair of Points: Divide-and-Conquer and Spatial Pruning](docs/13-geometric-and-spatial-algorithms/closest-pair-of-points.md)
* [Computational Geometry Basics: Primitives, Exact Predicates, and Intersections](docs/13-geometric-and-spatial-algorithms/computational-geometry-basics.md)
* [Convex Hull: Andrew's Monotone Chain, Graham Scan, and Jarvis March](docs/13-geometric-and-spatial-algorithms/convex-hull.md)
* [KD-Trees: Multidimensional Space Partitioning and Nearest-Neighbor Search](docs/13-geometric-and-spatial-algorithms/kd-trees.md)
* [Line Sweep and Segment Intersections: Shamos-Hoey and Bentley-Ottmann](docs/13-geometric-and-spatial-algorithms/line-sweep.md)
* [R-Trees: Minimum Bounding Rectangles, Quadratic Split, and Spatial Indexing](docs/13-geometric-and-spatial-algorithms/r-trees.md)

### [14] Advanced Data Structures
*Word-RAM models, integer priority structures, dynamic trees, and succinct bitvectors.*

* [Fusion Trees: Breaking the Comparison Lower Bound on the Word RAM](docs/14-advanced-data-structures/fusion-trees.md)
* [Link-Cut Trees](docs/14-advanced-data-structures/link-cut-trees.md)
* [Persistent Segment Trees and Path Copying](docs/14-advanced-data-structures/persistent-data-structures.md)
* [Succinct Data Structures: Rank and Select on Bitvectors](docs/14-advanced-data-structures/succinct-data-structures.md)
* [van Emde Boas Trees and Cache-Oblivious Layouts](docs/14-advanced-data-structures/van-emde-boas-trees.md)
* [Wavelet Trees: Succinct Multiset Representation & Range Quantile Engine](docs/14-advanced-data-structures/wavelet-trees.md)
* [X-Fast and Y-Fast Tries: Bitwise Predecessor Search on the Word RAM](docs/14-advanced-data-structures/x-fast-and-y-fast-tries.md)

---

## Phase 3: Systems, Concurrency, Hardware & Verification

### [15] External Memory, Cache-Oblivious & Streaming
*I/O-complexity models, multi-way external sorting, LSM storage engines, and streaming sketches.*

* [Count-Min Sketch and Frequency Estimation](docs/15-external-memory-cache-oblivious-and-streaming/count-min-sketch.md)
* [Multi-Way External Merge Sort](docs/15-external-memory-cache-oblivious-and-streaming/external-sorting.md)
* [The External Memory (I/O) Model](docs/15-external-memory-cache-oblivious-and-streaming/io-model-and-external-memory.md)
* [Log-Structured Merge-Trees (LSM-Trees): Write-Optimized Storage & Leveled Compaction](docs/15-external-memory-cache-oblivious-and-streaming/lsm-trees.md)
* [Streaming Models & Algorithms: Single-Pass Sublinear Analytics](docs/15-external-memory-cache-oblivious-and-streaming/streaming-models.md)

### [16] Parallel, Concurrent & Lock-Free Structures
*Work-depth parallel models, non-blocking synchronization, ABA resolution, and hazard pointers.*

* [The ABA Problem & CAS Pitfalls: Memory Recycling in Lock-Free Systems](docs/16-parallel-concurrent-and-lock-free/aba-problem.md)
* [Concurrent Queues, Stacks & Safe Memory Reclamation](docs/16-parallel-concurrent-and-lock-free/concurrent-queues-and-stacks.md)
* [Safe Memory Reclamation: Hazard Pointers & Epoch-Based Reclamation](docs/16-parallel-concurrent-and-lock-free/hazard-pointers-and-epoch-reclamation.md)
* [Lock-Free and Wait-Free Basics: Progress Guarantees & Consensus](docs/16-parallel-concurrent-and-lock-free/lock-free-and-wait-free-basics.md)
* [Parallel Algorithm Basics: The Work-Depth Model & Algorithmic Primitives](docs/16-parallel-concurrent-and-lock-free/parallel-algorithm-basics.md)
* [Work-Stealing Deques: The Chase-Lev Lock-Free Deque](docs/16-parallel-concurrent-and-lock-free/work-stealing-deques.md)

### [17] Cryptographic & Merkle Structures
*Verifiable data structures, authenticated dictionaries, and cryptographic proof systems.*

* [Authenticated Data Structures: Verifiable Outsourced Storage & Non-Membership Proofs](docs/17-cryptographic-and-merkle-like-structures/authenticated-data-structures.md)
* [Merkle Trees: Cryptographic Hash Trees & Audit Proofs](docs/17-cryptographic-and-merkle-like-structures/merkle-trees.md)

### [18] Systems Case Studies
*Production implementations in the Linux kernel, PostgreSQL, RocksDB, and distributed state machines.*

* [Distributed State Engines: Consistent Hashing, Vector Clocks & Raft Consensus](docs/18-systems-case-studies/distributed-state-engines.md)
* [High-Performance Caching: 2Q, ARC & Window TinyLFU](docs/18-systems-case-studies/high-performance-caching.md)
* [Linux Kernel Internals: Intrusive Structures, CFS Scheduler & RCU](docs/18-systems-case-studies/linux-kernel-internals.md)
* [Storage Engine Breakdown: LSM-Trees, B-Trees & Bitcask](docs/18-systems-case-studies/storage-engines-breakdown.md)

### [20] Proof Techniques & Correctness
*Mathematical induction, loop invariants, exchange arguments, and matroid properties.*

* [Cut and Cycle Properties in Graphs and Spanning Trees](docs/20-proof-techniques-and-correctness/cut-and-cycle-properties.md)
* [Greedy Exchange Arguments](docs/20-proof-techniques-and-correctness/exchange-arguments.md)
* [Mathematical Induction in Algorithm Correctness](docs/20-proof-techniques-and-correctness/induction.md)
* [Loop Invariants and Correctness](docs/20-proof-techniques-and-correctness/loop-invariants.md)

### [21] Implementation Engineering
*Industrial software engineering: robust API design, property-based testing, and fuzzing.*

* [API Design for Data Structures](docs/21-implementation-engineering/api-design-for-data-structures.md)
* [Fuzzing and Property-Based Testing](docs/21-implementation-engineering/fuzzing-and-property-testing.md)
* [Testing Data Structures](docs/21-implementation-engineering/testing-data-structures.md)

### [22] Benchmarking & Tradeoffs
*Empirical performance measurement, hardware performance counters, and selection matrices.*

* [Benchmark Design: Scientific Methodology & Performance Engineering](docs/22-benchmarking-and-tradeoffs/benchmark-design.md)
* [Choosing the Right Data Structure](docs/22-benchmarking-and-tradeoffs/choosing-the-right-data-structure.md)
* [Theoretical vs Practical Performance](docs/22-benchmarking-and-tradeoffs/theoretical-vs-practical-performance.md)

### [23] Classics & Landmark Papers
*Foundational academic literature and the historical genesis of canonical data structures.*

* [Classic Papers Reading List: The Foundational Canon of Data Structures & Algorithms](docs/23-history-papers-and-classics/classic-papers-reading-list.md)
* [Landmark Data Structures: Architectural Evolution & Historical Milestones](docs/23-history-papers-and-classics/landmark-data-structures.md)

### [24] Exercises & Problem Sets
*Categorized practice taxonomies mapped to competitive programming and technical interviews.*

* [Contest and Interview Problem Mapping: Curated Practice Matrix](docs/24-exercises-and-curated-problems/contest-and-interview-mapping.md)
* [Master Topic Index & Global DSA Taxonomy](docs/24-exercises-and-curated-problems/topic-index.md)

---

## Future Research Horizons & Expansion Targets

The handbook actively tracks ongoing frontiers in data structures, hardware paradigms, and modern algorithms. Potential targets for future monograph expansion include:

1. **Non-Volatile Memory (NVM) & Persistent Memory Structures**:
   * Crash-consistent search trees (FAST-FAIR, NV-Tree, P-B+Tree, FPTree).
   * Cache-line flush (`clwb`, `clflushopt`) and memory fence (`sfence`) durability primitives.
   * Lock-free persistent logs and persistent memory allocators (PMDK).

2. **Learned Index Structures & Machine-Learned DSA**:
   * Recursive Model Indexes (RMI) replacing traditional B-Trees and Radix Trees.
   * Updatable learned indexes (ALEX, PGM-Index, RadixSpline).
   * Learned Bloom filters and neural cardinality estimators.

3. **Hardware Transactional Memory (HTM) & Modern Concurrency**:
   * Restricted Transactional Memory (Intel TSX) hybrid synchronization.
   * Wait-free universal constructions and fast lock-free priority queues.
   * NUMA-aware data placement and flat combining algorithms.

4. **High-Dimensional Vector Search & Approximate Nearest Neighbors (ANN)**:
   * Hierarchical Navigable Small World (HNSW) graphs.
   * Inverted File with Product Quantization (IVF-PQ) and ScaNN.
   * Disk-native vector indexing (DiskANN) for trillion-scale embedding spaces.

5. **Quantum Algorithmic Primitives**:
   * Quantum amplitude amplification and Grover search complexity boundaries.
   * Quantum walks on graphs and quantum Fourier transform primitives.
