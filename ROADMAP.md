# Awesome DSA Master Roadmap & Completion Matrix

This roadmap tracks the development of **Awesome DSA** across its 24 core knowledge domains and 3 implementation phases.

Legend:
* [x] **Complete**: Exhaustive coverage matching the 12-section standard.
* [-] **In Progress**: Drafted or active development.
* [ ] **Planned**: Scheduled for rollout.

---

## Phase 1: Core Foundations & Universal Structures

### [00] Learning Paths
- [ ] `beginner-path.md`
- [ ] `interview-path.md`
- [ ] `competitive-programming-path.md`
- [ ] `systems-engineer-path.md`
- [ ] `theory-path.md`

### [01] Mathematical Foundations
- [ ] `logic-and-proof-techniques.md`
- [ ] `sets-functions-relations.md`
- [ ] `summations-and-series.md`
- [ ] `recurrence-relations.md`
- [ ] `combinatorics.md`
- [ ] `probability-basics.md`
- [ ] `expected-value-and-random-variables.md`
- [ ] `generating-functions-intuition.md`
- [ ] `number-theory-basics.md`
- [ ] `linear-algebra-for-algorithms.md`

### [02] Analysis & Complexity
- [ ] `asymptotic-analysis.md`
- [ ] `amortized-analysis.md`
- [ ] `worst-average-smoothed-analysis.md`
- [ ] `lower-bounds-and-adversaries.md`
- [ ] `reductions-and-hardness-intuition.md`
- [ ] `master-theorem-and-beyond.md`
- [ ] `akra-bazzi-method.md`
- [ ] `randomized-analysis.md`

### [03] Machine Model & Performance
- [ ] `ram-model-vs-real-machines.md`
- [x] [`cpu-cache-and-memory.md`](docs/03-machine-model-and-performance/cpu-cache-and-memory.md)
- [ ] `branch-prediction-and-pipelines.md`
- [ ] `memory-allocation-and-fragmentation.md`
- [ ] `locality-and-data-oriented-design.md`
- [ ] `simd-and-vectorization-intuition.md`
- [ ] `benchmarking-pitfalls.md`

### [04] Linear Data Structures
- [ ] `arrays-and-memory-layout.md`
- [x] [`dynamic-arrays-and-strings.md`](docs/04-linear-data-structures/dynamic-arrays-and-strings.md)
- [x] [`linked-lists.md`](docs/04-linear-data-structures/linked-lists.md)
- [ ] `stacks-and-queues.md`
- [ ] `deques.md`
- [x] [`ring-buffers.md`](docs/04-linear-data-structures/ring-buffers.md)
- [ ] `ropes-gap-buffers-piece-tables.md`
- [ ] `bitsets-and-bitvectors.md`
- [x] [`disjoint-set-union.md`](docs/04-linear-data-structures/disjoint-set-union.md)

### [05] Trees & Hierarchical Structures
- [ ] `tree-basics-and-traversals.md`
- [ ] `binary-search-trees.md`
- [x] [`avl-and-red-black-trees.md`](docs/05-trees-and-hierarchical-structures/avl-and-red-black-trees.md)
- [ ] `splay-trees.md`
- [ ] `treaps.md`
- [ ] `scapegoat-and-aa-trees.md`
- [x] [`b-trees-and-b-plus-trees.md`](docs/05-trees-and-hierarchical-structures/b-trees-and-b-plus-trees.md)
- [ ] `order-statistic-trees.md`
- [ ] `interval-trees.md`
- [x] [`segment-trees.md`](docs/05-trees-and-hierarchical-structures/segment-trees.md)
- [x] [`fenwick-trees.md`](docs/05-trees-and-hierarchical-structures/fenwick-trees.md)
- [x] [`tries-and-radix-trees.md`](docs/05-trees-and-hierarchical-structures/tries-and-radix-trees.md)

### [06] Heaps, Priority & Selection
- [x] [`binary-heaps.md`](docs/06-heaps-priority-and-selection/binary-heaps.md)
- [x] [`d-ary-heaps.md`](docs/06-heaps-priority-and-selection/d-ary-heaps.md)
- [ ] `binomial-heaps.md`
- [x] [`fibonacci-heaps.md`](docs/06-heaps-priority-and-selection/fibonacci-heaps.md)
- [x] [`pairing-heaps.md`](docs/06-heaps-priority-and-selection/pairing-heaps.md)
- [x] [`priority-queues-in-practice.md`](docs/06-heaps-priority-and-selection/priority-queues-in-practice.md)
- [x] [`quickselect-and-median-of-medians.md`](docs/06-heaps-priority-and-selection/quickselect-and-median-of-medians.md)
- [x] [`median-maintenance.md`](docs/06-heaps-priority-and-selection/median-maintenance.md)
- [x] [`indexed-priority-queues.md`](docs/06-heaps-priority-and-selection/indexed-priority-queues.md)

### [07] Hashing, Randomization & Probabilistic
- [ ] `hash-functions.md`
- [x] [`hash-tables-and-collisions.md`](docs/07-hashing-randomization-and-probabilistic/hash-tables-and-collisions.md)
- [ ] `robin-hood-cuckoo-and-hopscotch-hashing.md`
- [ ] `consistent-hashing.md`
- [x] [`bloom-and-cuckoo-filters.md`](docs/07-hashing-randomization-and-probabilistic/bloom-and-cuckoo-filters.md)
- [ ] `hyperloglog.md`
- [x] [`skip-lists.md`](docs/07-hashing-randomization-and-probabilistic/skip-lists.md)

### [08] Graphs & Network Algorithms
- [x] [`graph-representations.md`](docs/08-graphs-and-network-algorithms/graph-representations.md)
- [x] [`bfs-dfs-and-traversal-patterns.md`](docs/08-graphs-and-network-algorithms/bfs-dfs-and-traversal-patterns.md)
- [x] [`topological-sort.md`](docs/08-graphs-and-network-algorithms/topological-sort.md)
- [x] [`shortest-paths.md`](docs/08-graphs-and-network-algorithms/shortest-paths.md)
- [x] [`all-pairs-shortest-paths.md`](docs/08-graphs-and-network-algorithms/all-pairs-shortest-paths.md)
- [x] [`minimum-spanning-trees.md`](docs/08-graphs-and-network-algorithms/minimum-spanning-trees.md)
- [x] [`strongly-connected-components.md`](docs/08-graphs-and-network-algorithms/strongly-connected-components.md)
- [x] [`lowest-common-ancestor.md`](docs/08-graphs-and-network-algorithms/lowest-common-ancestor.md)

### [09] Algorithm Design Paradigms
- [ ] `recursion-and-backtracking.md`
- [x] [`divide-and-conquer.md`](docs/09-algorithm-design-paradigms/divide-and-conquer.md)
- [x] [`greedy-algorithms.md`](docs/09-algorithm-design-paradigms/greedy-algorithms.md)
- [ ] `dynamic-programming-intuition.md`
- [ ] `branch-and-bound.md`
- [ ] `meet-in-the-middle.md`

### [10] Dynamic Programming
- [x] [`1d-and-2d-foundations.md`](docs/10-dynamic-programming/1d-and-2d-foundations.md)
- [x] [`longest-increasing-subsequence.md`](docs/10-dynamic-programming/longest-increasing-subsequence.md)
- [x] [`knapsack-family.md`](docs/10-dynamic-programming/knapsack-family.md)
- [x] [`edit-distance-and-sequence-alignment.md`](docs/10-dynamic-programming/edit-distance-and-sequence-alignment.md)
- [x] [`longest-common-subsequence.md`](docs/10-dynamic-programming/longest-common-subsequence.md)
- [x] [`interval-and-matrix-dp.md`](docs/10-dynamic-programming/interval-and-matrix-dp.md)
- [x] [`tree-dp.md`](docs/10-dynamic-programming/tree-dp.md)
- [x] [`bitmask-and-state-compression.md`](docs/10-dynamic-programming/bitmask-and-state-compression.md)

### [19] Problem-Solving Patterns
- [x] [`two-pointers.md`](docs/19-problem-solving-patterns/two-pointers.md)
- [x] [`sliding-window.md`](docs/19-problem-solving-patterns/sliding-window.md)
- [x] [`monotonic-stack-and-queue.md`](docs/19-problem-solving-patterns/monotonic-stack-and-queue.md)
- [x] [`interval-scheduling.md`](docs/19-problem-solving-patterns/interval-scheduling.md)
- [x] [`binary-search-on-answer.md`](docs/19-problem-solving-patterns/binary-search-on-answer.md)

---

## Phase 2: Advanced Data Structures & Competitive Programming

### [11] Strings, Text & Pattern Matching
- [x] [`prefix-function-and-kmp.md`](docs/11-strings-text-and-pattern-matching/prefix-function-and-kmp.md)
- [x] [`z-algorithm.md`](docs/11-strings-text-and-pattern-matching/z-algorithm.md)
- [ ] `rabin-karp-and-rolling-hash.md`
- [x] [`aho-corasick.md`](docs/11-strings-text-and-pattern-matching/aho-corasick.md)
- [x] [`suffix-array.md`](docs/11-strings-text-and-pattern-matching/suffix-arrays-and-lcp.md)
- [ ] `suffix-tree.md`
- [ ] `suffix-automaton.md`
- [ ] `manacher-algorithm.md`

### [12] Range Query & Offline Structures
- [x] [`prefix-sums-and-difference-arrays.md`](docs/12-range-query-and-offline-structures/prefix-sums-and-difference-arrays.md)
- [ ] `sparse-tables.md`
- [ ] `sqrt-decomposition.md`
- [ ] `mo-algorithm.md`
- [ ] `offline-query-processing.md`
- [ ] `range-minimum-query.md`
- [ ] `range-updates-and-lazy-propagation.md`

### [13] Geometric & Spatial Algorithms
- [ ] `computational-geometry-basics.md`
- [ ] `line-sweep.md`
- [ ] `convex-hull.md`
- [ ] `closest-pair-of-points.md`
- [ ] `kd-trees.md`
- [ ] `r-trees.md`

### [14] Advanced Data Structures
- [ ] `van-emde-boas-trees.md`
- [ ] `x-fast-and-y-fast-tries.md`
- [ ] `fusion-trees.md`
- [ ] `persistent-data-structures.md`
- [ ] `link-cut-trees.md`
- [ ] `wavelet-trees.md`
- [ ] `succinct-data-structures.md`

---

## Phase 3: Systems, Concurrency, Proofs & Production

### [15] External Memory & Streaming
- [ ] `io-model-and-external-memory.md`
- [ ] `external-sorting.md`
- [ ] `lsm-trees.md`
- [ ] `streaming-models.md`
- [ ] `count-min-sketch.md`

### [16] Parallel, Concurrent & Lock-Free
- [ ] `parallel-algorithm-basics.md`
- [ ] `concurrent-queues-and-stacks.md`
- [ ] `lock-free-and-wait-free-basics.md`
- [ ] `aba-problem.md`
- [ ] `hazard-pointers-and-epoch-reclamation.md`
- [ ] `work-stealing-deques.md`

### [17] Cryptographic & Merkle Structures
- [ ] `merkle-trees.md`
- [ ] `authenticated-data-structures.md`

### [18] Systems Case Studies
- [ ] `storage-engines-breakdown.md`
- [ ] `linux-kernel-internals.md`
- [ ] `high-performance-caching.md`
- [ ] `distributed-state-engines.md`

### [20] Proof Techniques & Correctness
- [ ] `induction.md`
- [ ] `loop-invariants.md`
- [ ] `exchange-arguments.md`
- [ ] `cut-and-cycle-properties.md`

### [21] Implementation Engineering
- [ ] `api-design-for-data-structures.md`
- [ ] `testing-data-structures.md`
- [ ] `fuzzing-and-property-testing.md`

### [22] Benchmarking & Tradeoffs
- [x] [`theoretical-vs-practical-performance.md`](docs/22-benchmarking-and-tradeoffs/theoretical-vs-practical-performance.md)
- [ ] `benchmark-design.md`
- [x] [`choosing-the-right-data-structure.md`](docs/22-benchmarking-and-tradeoffs/choosing-the-right-data-structure.md)

### [23] Classics & Papers
- [ ] `classic-papers-reading-list.md`
- [ ] `landmark-data-structures.md`

### [24] Exercises & Problem Sets
- [ ] `topic-index.md`
- [ ] `contest-and-interview-mapping.md`
