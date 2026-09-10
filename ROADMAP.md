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
- [ ] `dynamic-arrays-and-strings.md`
- [ ] `linked-lists.md`
- [ ] `stacks-and-queues.md`
- [ ] `deques.md`
- [ ] `ring-buffers.md`
- [ ] `ropes-gap-buffers-piece-tables.md`
- [ ] `bitsets-and-bitvectors.md`
- [ ] `disjoint-set-union.md`

### [05] Trees & Hierarchical Structures
- [ ] `tree-basics-and-traversals.md`
- [ ] `binary-search-trees.md`
- [ ] `avl-and-red-black-trees.md`
- [ ] `splay-trees.md`
- [ ] `treaps.md`
- [ ] `scapegoat-and-aa-trees.md`
- [x] [`b-trees-and-b-plus-trees.md`](docs/05-trees-and-hierarchical-structures/b-trees-and-b-plus-trees.md)
- [ ] `order-statistic-trees.md`
- [ ] `interval-trees.md`
- [ ] `segment-trees.md`
- [ ] `fenwick-trees.md`
- [ ] `tries-and-radix-trees.md`

### [06] Heaps, Priority & Selection
- [ ] `binary-heaps.md`
- [ ] `d-ary-heaps.md`
- [ ] `binomial-heaps.md`
- [ ] `fibonacci-heaps.md`
- [ ] `pairing-heaps.md`
- [ ] `priority-queues-in-practice.md`
- [ ] `selection-algorithms.md`
- [ ] `median-maintenance.md`

### [07] Hashing, Randomization & Probabilistic
- [ ] `hash-functions.md`
- [ ] `hash-tables-and-collisions.md`
- [ ] `robin-hood-cuckoo-and-hopscotch-hashing.md`
- [ ] `consistent-hashing.md`
- [ ] `bloom-and-cuckoo-filters.md`
- [ ] `hyperloglog.md`
- [ ] `skip-lists.md`

### [08] Graphs & Network Algorithms
- [ ] `graph-representations.md`
- [ ] `bfs-dfs-and-traversal-patterns.md`
- [ ] `topological-sort.md`
- [ ] `shortest-paths.md`
- [ ] `all-pairs-shortest-paths.md`
- [ ] `minimum-spanning-trees.md`
- [ ] `strongly-connected-components.md`
- [x] [`lowest-common-ancestor.md`](docs/08-graphs-and-network-algorithms/lowest-common-ancestor.md)

### [09] Algorithm Design Paradigms
- [ ] `recursion-and-backtracking.md`
- [ ] `divide-and-conquer.md`
- [ ] `greedy-algorithms.md`
- [ ] `dynamic-programming-intuition.md`
- [ ] `branch-and-bound.md`
- [ ] `meet-in-the-middle.md`

### [10] Dynamic Programming
- [ ] `1d-and-2d-foundations.md`
- [ ] `knapsack-family.md`
- [ ] `sequences-and-strings.md`
- [ ] `interval-and-matrix-dp.md`
- [ ] `tree-dp.md`
- [ ] `bitmask-and-state-compression.md`

### [19] Problem-Solving Patterns
- [x] [`two-pointers.md`](docs/19-problem-solving-patterns/two-pointers.md)
- [x] [`sliding-window.md`](docs/19-problem-solving-patterns/sliding-window.md)
- [ ] `monotonic-stack-and-queue.md`
- [ ] `interval-scheduling.md`
- [ ] `binary-search-on-answer.md`

---

## Phase 2: Advanced Data Structures & Competitive Programming

### [11] Strings, Text & Pattern Matching
- [ ] `prefix-function-and-kmp.md`
- [ ] `z-algorithm.md`
- [ ] `rabin-karp-and-rolling-hash.md`
- [ ] `aho-corasick.md`
- [ ] `suffix-array.md`
- [ ] `suffix-tree.md`
- [ ] `suffix-automaton.md`
- [ ] `manacher-algorithm.md`

### [12] Range Query & Offline Structures
- [ ] `prefix-sums-and-difference-arrays.md`
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
- [ ] `theoretical-vs-practical-performance.md`
- [ ] `benchmark-design.md`
- [x] [`choosing-the-right-data-structure.md`](docs/22-benchmarking-and-tradeoffs/choosing-the-right-data-structure.md)

### [23] Classics & Papers
- [ ] `classic-papers-reading-list.md`
- [ ] `landmark-data-structures.md`

### [24] Exercises & Problem Sets
- [ ] `topic-index.md`
- [ ] `contest-and-interview-mapping.md`
