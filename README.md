# Awesome DSA [![Awesome](https://awesome.re/badge.svg)](https://awesome.re)

> An intuitive, visual, and production-oriented guide to **Data Structures & Algorithms**.  
> Beyond LeetCode grinding: bridging academic theory, problem-solving patterns, and real-world software systems.

---

## Contents

- [Decision Matrix: Which Data Structure Should I Use?](#decision-matrix-which-data-structure-should-i-use)
- [Data Structures in the Wild (Production Systems)](#data-structures-in-the-wild-production-systems)
- [Interactive Visualizers & Sandboxes](#interactive-visualizers--sandboxes)
- [Core Algorithmic Patterns](#core-algorithmic-patterns)
- [Hardware & Systems Reality Check](#hardware--systems-reality-check)
- [Foundational Courses & Lectures](#foundational-courses--lectures)
- [Books & Reference Texts](#books--reference-texts)
- [Complexity & Cheat Sheets](#complexity--cheat-sheets)
- [Clean Reference Implementations](#clean-reference-implementations)
- [Contributing](#contributing)
- [License](#license)

---

## Decision Matrix: Which Data Structure Should I Use?

Use this quick-lookup guide when architecting software or choosing the right tool for an algorithmic problem.

| Requirement / Query Type | Recommended Data Structure | Time Complexity (Avg) | Real-World Trade-Off |
| :--- | :--- | :--- | :--- |
| Fast key-value lookups without ordering | **Hash Table / Hash Map** | $O(1)$ search, insert | Memory overhead; cache-unfriendly bucket chaining; worst-case $O(N)$ with hash collisions. |
| In-order traversal, range queries ($k_1 \le x \le k_2$) | **Self-Balancing BST (Red-Black / AVL)** | $O(\log N)$ | Pointer chasing hurts CPU cache locality compared to contiguous arrays. |
| Disk/Database indexing with minimal I/O | **B-Tree / B+ Tree** | $O(\log_B N)$ | High fan-out optimizes for page reads; B+ keeps all data in leaves for sequential range scans. |
| High write throughput with sequential disk writes | **LSM-Tree (Log-Structured Merge-Tree)** | $O(1)$ write, $O(\log N)$ read | Append-only commits fast writes; requires background compaction and Bloom filters for read performance. |
| Fast prefix searches / autocomplete | **Trie (Prefix Tree) / Radix Tree** | $O(L)$ ($L = \text{length}$) | Space-heavy if sparse; compressed (Radix) trees minimize node allocations. |
| Constant-time set membership with false positives allowed | **Bloom Filter / Cuckoo Filter** | $O(k)$ ($k = \text{hashes}$) | Saves massive memory/disk lookups; cannot retrieve elements or delete easily (unless Counting/Cuckoo). |
| Dynamic rank queries / K-th smallest in stream | **Order Statistic Tree / Fenwick Tree** | $O(\log N)$ | Extends standard trees or binary representations with subtree size tracking. |
| Range sum / update queries on mutable arrays | **Fenwick Tree (BIT) or Segment Tree** | $O(\log N)$ query/update | Fenwick has smaller memory footprint and simpler code; Segment Tree supports arbitrary associative operations. |
| Streaming cardinality estimation (billions of items) | **HyperLogLog** | $O(1)$ update | Estimates distinct counts within ~1-2% error using only a few kilobytes of RAM. |
| Sliding window min/max or Next Greater Element | **Monotonic Stack / Deque** | $O(1)$ amortized per item | Linear scan maintaining strict monotonic invariant; drops redundant candidates. |
| Disjoint set connectivity / cycle detection in graphs | **Disjoint Set Union (DSU / Union-Find)** | $O(\alpha(N)) \approx O(1)$ | Near-constant time with path compression and union-by-rank. |

---

## Data Structures in the Wild (Production Systems)

Where data structures live inside modern infrastructure:

### Storage & Databases
* **B+ Trees** $\rightarrow$ [PostgreSQL Indexing](https://www.postgresql.org/docs/current/btree-intro.html), [SQLite B-Tree module](https://www.sqlite.org/btreemodule.html), and MySQL InnoDB. Internal nodes store routing keys; leaf nodes form a doubly linked list for fast sequential scans.
* **LSM-Trees** $\rightarrow$ [RocksDB](https://rocksdb.org/), [Apache Cassandra](https://cassandra.apache.org/), and [LevelDB](https://github.com/google/leveldb). Flushes in-memory `MemTable` (often a Skip List) to immutable sorted disk files (`SSTables`).
* **Skip Lists** $\rightarrow$ [Redis Sorted Sets (`ZSET`)](https://redis.io/docs/latest/develop/data-types/sorted-sets/) and LevelDB memory tables. Probabilistic alternative to balanced trees; simpler to implement and lock-free concurrency friendly.

### Operating Systems & Networking
* **Radix Trees / Patricia Tries** $\rightarrow$ [Linux Kernel IP Routing](https://git.kernel.org/) and page cache lookup tables (`xarray`). Space-optimized trees branching on bit chunks of network prefixes.
* **Ring Buffers (Circular Queues)** $\rightarrow$ [Linux Kernel `kfifo`](https://docs.kernel.org/) and high-throughput messaging engines like the [LMAX Disruptor](https://lmax-exchange.github.io/disruptor/). Lock-free single-producer single-consumer ring buffers avoid dynamic memory allocation.
* **Red-Black Trees** $\rightarrow$ [Linux Completely Fair Scheduler (CFS)](https://docs.kernel.org/scheduler/sched-design-CFS.html). Maintains tasks sorted by runtime so the task with least CPU time is picked in $O(1)$ / $O(\log N)$.

### Web & Big Data Systems
* **Bloom Filters** $\rightarrow$ [Google Chrome Safe Browsing](https://developers.google.com/safe-browsing) (checks malicious URLs locally before making network requests) and Apache Cassandra read paths to prevent unnecessary SSTable disk reads.
* **HyperLogLog** $\rightarrow$ [Redis `PFADD`/`PFCOUNT`](https://redis.io/commands/pfcount/) and Reddit view counters. Approximates unique counts across millions of records using negligible memory (~12 KB).
* **Directed Acyclic Graphs (DAGs)** $\rightarrow$ [Git Commit History](https://git-scm.com/book/en/v2/Git-Internals-Git-Objects) and workflow orchestrators like Apache Airflow and Apache Spark execution pipelines.

---

## Interactive Visualizers & Sandboxes

Visualizing state transitions builds lasting intuition faster than static text.

* [VisuAlgo](https://visualgo.net/en) - Comprehensive interactive visualizer covering trees, graphs, sorting, and geometric algorithms.
* [Algorithm Visualizer](https://algorithm-visualizer.org/) - Interactive platform visualizing code execution step-by-step in JavaScript, C++, and Java.
* [USFCA Algorithms in Action](https://www.cs.usfca.edu/~galles/visualization/Algorithms.html) - David Galles' classic interactive web animations for Red-Black, AVL, B-Trees, and hashing.
* [Pathfinding.js Visualizer](https://qiao.github.io/PathFinding.js/visual/) - Interactive 2D grid visualizer for $A^*$, Dijkstra, IDA*, and Breadth-First Search.
* [Sorting.at](https://sorting.at/) - Minimalist, high-framerate visual comparison of sorting algorithms.
* [Red-Black Tree Visualizer](https://www.cs.armstrong.edu/liang/animation/web/RBTree.html) - Step-by-step node rotations and recoloring demonstrations.

---

## Core Algorithmic Patterns

Mastering problem-solving is about recognizing structural patterns, not memorizing solutions.

### 1. Two Pointers & Sliding Window
* **When to use**: Linear collections (arrays, strings) searching for contiguous ranges, target sums, or partitions.
* **Core Insight**: Reduces brute-force $O(N^2)$ checks to $O(N)$ by maintaining left and right boundary invariants.
* **Archetypes**: 3Sum, Container With Most Water, Minimum Size Subarray Sum, Longest Substring Without Repeating Characters.

### 2. Fast & Slow Pointers (Floyd's Cycle-Finding)
* **When to use**: Linked lists and functional graphs where pointers can form loops or identify midpoints.
* **Core Insight**: Pointers moving at speeds 1x and 2x are guaranteed to meet inside a cycle of length $C$ in $O(N)$ steps.
* **Archetypes**: Linked List Cycle Detection, Find the Duplicate Number, Middle of the Linked List.

### 3. Monotonic Stack / Queue
* **When to use**: Finding the next greater/smaller element, or computing largest rectangles.
* **Core Insight**: Push elements while maintaining a strictly increasing or decreasing stack. Discard elements that can never be future optimal answers.
* **Archetypes**: Next Greater Element, Daily Temperatures, Largest Rectangle in Histogram, Trapping Rain Water.

### 4. Top 'K' Elements (Heap / Priority Queue)
* **When to use**: Finding the $k$-th largest or smallest elements in a streaming or static dataset without a full $O(N \log N)$ sort.
* **Core Insight**: Keep a Min-Heap of size $K$. If the next incoming element exceeds the heap's minimum root, pop and push. Runs in $O(N \log K)$.
* **Archetypes**: Kth Largest Element in an Array, Top K Frequent Elements, Merge K Sorted Lists.

### 5. Overlapping Intervals
* **When to use**: Scheduling tasks, calendar collisions, or merging contiguous segments.
* **Core Insight**: Sort intervals by start time. Iterate and compare current start with previous end.
* **Archetypes**: Merge Intervals, Non-overlapping Intervals, Meeting Rooms II.

### 6. Dynamic Programming: State Transitions
* **When to use**: Optimization problems exhibiting **Optimal Substructure** and **Overlapping Subproblems**.
* **Canonical Sub-patterns**:
  * *0/1 Knapsack & Unbounded Knapsack* (subset selection with budget constraints)
  * *Longest Common Subsequence (LCS)* (string alignment, diff engines)
  * *Longest Increasing Subsequence (LIS)* (patience sorting, sequence tracking)
  * *Interval DP* (matrix chain multiplication, burst balloons)

### 7. Graph Traversals & Topological Sort
* **When to use**: Dependency graphs, component connectivity, shortest path in unweighted graphs.
* **Core Insight**:
  * BFS $\rightarrow$ Shortest path on unweighted graphs (level by level).
  * DFS $\rightarrow$ Cycle detection, connectivity, backtracking.
  * Kahn's Algorithm (in-degree tracking) $\rightarrow$ Topological order in DAGs (build systems, course prerequisites).

---

## Hardware & Systems Reality Check

Where pure Big-O theory meets real CPU hardware architecture:

* **Cache Locality vs. Pointer Chasing**:
  * An array (`std::vector` or slice) stores contiguous memory. Traversing it triggers hardware prefetching, hitting the fast L1/L2 CPU cache ($~1$ to $5$ ns).
  * A `LinkedList` scatters nodes across heap memory. Every next pointer hop causes a CPU cache miss, taking upwards of $100$ ns to fetch from main RAM.
  * *Result*: In practice, iterating a contiguous array almost always outperforms a linked list, even for operations where linked lists theoretically have better asymptotic complexity.

* **Amortized Analysis**:
  * Dynamic arrays double their capacity when full ($1 \rightarrow 2 \rightarrow 4 \rightarrow 8 \dots$).
  * The $N$-th insertion might cost $O(N)$ due to reallocating and copying, but dividing total copies over $N$ insertions yields an average of $O(1)$ amortized cost.

* **Memory Overhead of Node Structures**:
  * Each node in a 64-bit doubly linked list requires at least 16 bytes for two pointers (`prev`, `next`) plus memory padding, dwarfing the actual data payload.

---

## Foundational Courses & Lectures

* [MIT 6.006: Introduction to Algorithms (Spring 2020)](https://ocw.mit.edu/courses/6-006-introduction-to-algorithms-spring-2020/) - Profs. Erik Demaine, Srini Devadas. The gold standard in rigorous, conceptual algorithm foundations.
* [Stanford CS161: Design and Analysis of Algorithms](https://web.stanford.edu/class/cs161/) - Tim Roughgarden's lecture notes and problem sets.
* [Algorithms by Robert Sedgewick & Kevin Wayne (Princeton)](https://www.coursera.org/learn/algorithms-part1) - Classical Java-based data structures with practical engineering analysis.
* [Abdul Bari's Algorithms Series](https://www.youtube.com/@abdul_bari) - Universally acclaimed visual whiteboard breakdowns of dynamic programming, greedy methods, and graphs.
* [NeetCode Algorithms & Patterns](https://neetcode.io/) - Clear, structured video breakdowns of standard interview problem archetypes.

---

## Books & Reference Texts

* [Introduction to Algorithms (CLRS)](https://mitpress.mit.edu/9780262046305/introduction-to-algorithms/) - Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest, Clifford Stein. The definitive academic reference encyclopedia.
* [The Algorithm Design Manual](https://www.algorist.com/) - Steven S. Skiena. Focuses on practical problem identification and the famous "Hitchhiker’s Guide to Algorithms" catalog.
* [Grokking Algorithms](https://www.manning.com/books/grokking-algorithms) - Aditya Bhargava. Fully illustrated and approachable; the best conceptual bridge for beginners.
* [Algorithms (4th Edition)](https://algs4.cs.princeton.edu/home/) - Robert Sedgewick, Kevin Wayne. Clean implementations and ties to Java standard libraries.

---

## Complexity & Cheat Sheets

* [Big-O Cheat Sheet](https://www.bigocheatsheet.com/) - Quick reference table covering time and space complexities for all primary data structures and sorting algorithms.
* [Fast IO & Constant Factors](https://usaco.guide/general/fast-io) - Guide to competitive programming I/O bottlenecks and reducing constant-factor overhead.

---

## Clean Reference Implementations

Explore production-grade implementations in various languages:

* [TheAlgorithms (GitHub Organization)](https://github.com/TheAlgorithms) - Open-source algorithm implementations in [Python](https://github.com/TheAlgorithms/Python), [C++](https://github.com/TheAlgorithms/C-Plus-Plus), [Java](https://github.com/TheAlgorithms/Java), [Rust](https://github.com/TheAlgorithms/Rust), and [Go](https://github.com/TheAlgorithms/Go).
* [Sedgewick algs4 Java Code](https://github.com/kevin-wayne/algs4) - Official accompanying code for Princeton's Algorithms textbook.

---

## Contributing

Contributions are welcome! Please read the [Contributing Guidelines](CONTRIBUTING.md) before submitting a Pull Request.

---

## License

To the extent possible under law, the authors have waived all copyright and related or neighboring rights to this work under [CC0 1.0 Universal](LICENSE).
