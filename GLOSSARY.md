# The DSA Handbook Glossary

An exhaustive, publication-grade reference glossary of canonical data structures, algorithms, complexity definitions, CPU cache models, and concurrency primitives.

---

### A
* **ABA Problem**: A memory synchronization hazard in lock-free programming where a thread reads a pointer value $A$, is preempted, and another thread modifies the pointer to $B$ and back to $A$. A subsequent Compare-And-Swap (CAS) succeeds erroneously because the value matches, despite the underlying memory state having changed.
* **Aho-Corasick Automaton**: A multi-pattern string matching algorithm that constructs a trie augmented with failure transitions (suffix links). Matches all occurrences of a dictionary of patterns against a stream of text in $O(N + M + Z)$ time, where $Z$ is the count of matches.
* **Akra-Bazzi Method**: A mathematical generalization of the Master Theorem that solves general divide-and-conquer recurrences with unequal partition sizes and perturbation terms: $T(x) = g(x) + \sum_{i=1}^k a_i T(b_i x + h_i(x))$.
* **Amortized Complexity**: The average execution cost per operation over an arbitrary worst-case sequence of operations. Evaluated through aggregate analysis, the accounting method (banker's method), or the physicist's potential function method.
* **Associative Operation**: A binary algebraic operation $\star$ satisfying $(a \star b) \star c = a \star (b \star c)$. Associativity is the mandatory mathematical precondition for parallel prefix scans, segment trees, and monoidal decompositions.
* **AVL Tree**: A self-balancing binary search tree enforcing the strict invariant that for every node, the heights of its left and right subtrees differ by at most 1. Guarantees worst-case $O(\log N)$ search, insertion, and deletion via single and double rotations.

---

### B
* **B-Tree / B+ Tree**: Self-balancing multiway search trees designed for external block-storage devices (hard drives, NVMe SSDs). B-Trees maximize node fan-out to align with disk page boundaries, minimizing I/O transfers. In B+ Trees, all payload data resides exclusively in leaf nodes linked sequentially for high-throughput range scans.
* **Big-O ($O$), Big-$\Omega$, Big-$\Theta$**: Foundational asymptotic notations characterizing function growth rates. $O(g(n))$ establishes an asymptotic upper bound, $\Omega(g(n))$ an asymptotic lower bound, and $\Theta(g(n))$ a tight bound within constant factors.
* **Binary Indexed Tree (Fenwick Tree)**: An array-backed tree structure supporting prefix sum aggregations and point updates in $O(\log N)$ time and $O(N)$ space using two's-complement bit manipulation (`i & (-i)`).
* **Binomial Heap**: A priority queue implemented as a collection of binomial trees of distinct orders, supporting efficient heap merging ($O(\log N)$) alongside standard priority queue operations.
* **Bloom Filter**: A space-efficient probabilistic data structure used to test set membership with bounded false positives and zero false negatives. Uses $k$ independent hash functions over an $m$-bit array.
* **Branch Predictor**: A CPU execution engine unit that speculatively predicts the outcome of conditional branch instructions to keep hardware pipelines saturated. Branch mispredictions incur expensive pipeline flush penalties (~15–20 clock cycles).
* **Brent's Theorem**: A foundational theorem in parallel algorithm analysis stating that an algorithm with work $W$ and span (depth) $D$ executes on $p$ parallel processors in time $T_p \le \frac{W - D}{p} + D$.

---

### C
* **Cache Line**: The atomic block of data transferred between main system memory (DRAM) and CPU cache levels (L1, L2, L3), standardly 64 bytes on modern x86-64 and ARM processors.
* **Cache Locality (Spatial & Temporal)**: Hardware-conscious access properties. Spatial locality refers to accessing memory locations adjacent to recently accessed addresses (maximizing 64-byte line utilization); temporal locality refers to re-accessing the same address in short intervals.
* **Centroid Decomposition**: A divide-and-conquer technique on arbitrary trees that recursively identifies and removes the tree centroid (a node whose removal leaves subtrees of size $\le N/2$), constructing a centroid tree of depth at most $O(\log N)$.
* **Compare-And-Swap (CAS)**: A hardware-level atomic read-modify-write instruction (`lock cmpxchg` on x86) that compares a memory location against an expected value, updating it to a new value only if identical.
* **Consistent Hashing**: A distributed hash partitioning technique mapping keys and servers onto a continuous $2^{32}$ or $2^{64}$ circular ring. Minimizes key remapping during server additions or removals to $O(K/N)$.
* **Convex Hull**: The minimal convex boundary enclosing a planar point set $S$, computed in $O(N \log N)$ time via Graham Scan or Andrew's Monotone Chain algorithm.
* **Count-Min Sketch**: A sublinear-space probabilistic data structure serving as a frequency table of events in a streaming data sequence, providing $(\epsilon, \delta)$ additive error bounds using $d$ hash functions over $w$ counters.
* **Cuckoo Hashing**: An open-addressing dictionary scheme using multiple hash tables and hash functions, guaranteeing $O(1)$ worst-case lookup and deletion by evicting colliding keys along alternating displacement paths.

---

### D
* **D-ary Heap**: A generalization of the binary heap where each node has $d$ children. Decreases heap depth to $O(\log_d N)$ and optimizes CPU cache line usage during sift-up operations, commonly used in Dijkstra's algorithm.
* **De Bruijn Sequence**: A cyclic sequence of alphabet size $k$ containing every possible substring of length $n$ exactly once, leveraged in fast constant-time bit manipulation algorithms (e.g. finding least significant bit).
* **Dijkstra's Algorithm**: A greedy graph algorithm solving single-source shortest path problems on graphs with non-negative edge weights in $O((V + E) \log V)$ with binary heaps or $O(E + V \log V)$ with Fibonacci heaps.
* **Disjoint Set Union (DSU / Union-Find)**: A tree-backed data structure maintaining partitioned equivalence classes, supporting union and find queries in near-constant $O(\alpha(N))$ amortized time via path compression and union by rank.
* **Dynamic Programming (DP)**: An algorithmic optimization paradigm that solves optimization problems by recursively decomposing them into overlapping subproblems exhibiting optimal substructure, caching results via memoization or tabulation.

---

### E
* **Epoch-Based Reclamation (EBR)**: A lock-free safe memory reclamation protocol where global execution is divided into discrete epochs. Memory retired by a thread in epoch $e$ is safely reclaimed only after all active threads have acknowledged entering at least epoch $e+1$.
* **Eulerian Trail / Tour**: A trail in a graph that traverses every edge exactly once. Exists in a connected graph if and only if exactly zero or two vertices have odd degree.
* **External Memory Model (I/O Model)**: The Aggarwal-Vitter computational complexity framework measuring algorithm efficiency by the number of block transfers (I/Os) of size $B$ between fast internal memory of capacity $M$ and unbounded slow secondary storage.

---

### F
* **False Sharing**: A severe multithreaded performance penalty occurring when threads executing on distinct physical CPU cores concurrently modify independent variables that share the same 64-byte cache line, triggering repeated cache invalidations across cores.
* **Fibonacci Heap**: A collection of min-heap-ordered trees supporting $O(1)$ amortized insertion, find-min, and decrease-key operations, alongside $O(\log N)$ amortized delete-min. Critical for optimal theoretical asymptotic graph bounds.
* **Floyd-Warshall Algorithm**: An $O(V^3)$ dynamic programming algorithm that computes all-pairs shortest paths on directed graphs with positive or negative edge weights (provided no negative cycles exist).
* **FM-Index**: A succinct full-text index combining the Burrows-Wheeler Transform (BWT) with compact rank/select bitvector primitives, supporting exact pattern queries in $O(M)$ time proportional solely to pattern length.
* **Fusion Tree**: A specialized search tree operating on the Word RAM model that breaks the comparison-based lower bound of $\Omega(\log N)$, achieving $O(\log N / \log w)$ predecessor query time using bit-parallel parallel comparisons.

---

### G
* **Geometric Resizing**: The strategy of allocating memory in geometric ratios (scaling factor $c > 1$, typically $1.5$ or $2$) when a dynamic array saturates, mathematically ensuring $O(1)$ amortized insertion time.
* **Graham Scan**: An $O(N \log N)$ algorithm for computing the 2D convex hull of a set of points by sorting vertices by polar angle and applying a monotonic stack with cross-product turn tests.
* **Greedy Exchange Argument**: A formal proof methodology establishing the global optimality of a greedy heuristic by showing that any arbitrary optimal solution can be incrementally transformed into the greedy solution without deteriorating the objective value.

---

### H
* **Hazard Pointer**: A non-blocking memory reclamation mechanism where reader threads publish memory addresses they are actively inspecting into single-writer global hazard pointers, preventing reclaiming threads from freeing concurrent memory.
* **Heavy-Light Decomposition (HLD)**: A structural decomposition of arbitrary trees into vertex-disjoint linear paths based on subtree sizes, mapping any tree path query into at most $O(\log N)$ contiguous segments on a segment tree.
* **HyperLogLog (HLL)**: A space-efficient probabilistic algorithm estimating multiset cardinality with low relative error ($\approx 1.04 / \sqrt{m}$) using register buckets tracking the position of the leading zero in hashed keys.

---

### I
* **Induction (Mathematical & Structural)**: A rigorous deductive proof method demonstrating that a property $P(n)$ holds universally across all natural numbers or recursively defined structures (trees, DAGs) via a verifiable base case and inductive step.
* **Interval Tree**: An augmented binary search tree storing dynamic collections of closed intervals $[l, r]$, answering all intervals intersecting a query interval in $O(\log N + k)$ time where $k$ is the output count.
* **Intrusive Data Structure**: A memory layout paradigm (dominant in systems engines and the Linux kernel) where pointer nodes (`list_head`, `rb_node`) are embedded directly into user data structures, eliminating heap wrapping allocations and pointer indirection.
* **Invariant**: A mathematical predicate or system condition that holds strictly true throughout program execution, loop iterations, or state transitions, serving as the formal foundation for algorithm correctness proofs.

---

### K
* **Knuth-Morris-Pratt (KMP) Algorithm**: A linear-time ($O(N + M)$) string matching algorithm that computes a prefix function (longest proper prefix that is also a suffix) to advance the pattern without backtracking the text pointer.
* **KD-Tree ($k$-dimensional Tree)**: A space-partitioning binary tree organizing points in $k$-dimensional Euclidean space by recursively partitioning coordinate planes along alternating dimensions, enabling multi-dimensional range and nearest-neighbor search.

---

### L
* **Least Recently Used (LRU)**: A cache replacement policy that evicts the item accessed least recently. Implemented in $O(1)$ time per lookup and insertion using an intrusive doubly linked list coupled with a hash map.
* **Link-Cut Tree**: A dynamic tree structure representing a forest of rooted trees that supports path aggregations, dynamic edge additions (link), and edge deletions (cut) in $O(\log N)$ amortized time via splay-tree auxiliary paths.
* **Lock-Free**: A non-blocking concurrency progress guarantee ensuring that across all threads executing in a concurrent system, at least one thread completes its operation in a bounded number of execution steps.
* **Log-Structured Merge-Tree (LSM-Tree)**: A write-optimized storage architecture buffering mutations in an append-only in-memory table (MemTable) before flushing immutable, sorted runs (SSTables) sequentially to disk, optimized via tiered or leveled compaction.
* **Lowest Common Ancestor (LCA)**: In a rooted tree, the deepest node that is an ancestor of both vertices $u$ and $v$. Solvable in $O(1)$ query time after $O(N \log N)$ binary lifting or $O(N)$ Euler tour RMQ preprocessing.

---

### M
* **Manacher's Algorithm**: An optimal $O(N)$ algorithm for finding all maximal palindromic substrings within a string by exploiting palindromic symmetry to skip redundant character comparisons.
* **Master Theorem**: A cookbook theorem providing closed-form asymptotic solutions for divide-and-conquer recurrences of the canonical form $T(n) = a T(n/b) + f(n)$.
* **Merkle Tree**: A cryptographic hash tree where leaf nodes store hashes of data blocks and non-leaf nodes store hashes of their concatenated children, providing $O(\log N)$ tamper-evident audit proofs of data inclusion.
* **Mo's Algorithm**: An offline query reordering technique that sorts range queries $[L, R]$ on a static array in $O((N + Q) \sqrt{N})$ time using square root block decomposition or space-filling Hilbert curves.
* **Monotonic Stack / Queue**: A linear structure preserving a strictly ascending or descending ordering of elements, computing next-greater/smaller element queries or sliding-window extrema in $O(1)$ amortized time.

---

### N
* **Network Flow & Max-Flow Min-Cut**: An algorithmic discipline computing maximum feasible commodity flow through directed edge-capacitated networks; the Max-Flow Min-Cut theorem proves maximum network flow precisely equals the capacity of the bottleneck cut.
* **NP-Completeness**: The complexity class of decision problems in NP to which any problem in NP can be reduced in polynomial time. If any NP-complete problem has a polynomial-time algorithm, then $P = NP$.

---

### O
* **Open Addressing**: A collision resolution paradigm in hash tables where all key-value entries reside directly inside a contiguous bucket array, resolved via systematic probing sequences (linear, quadratic, or double hashing).
* **Order Statistic Tree**: An augmented balanced binary search tree storing subtree sizes at each node, enabling $O(\log N)$ time determination of the $k$-th smallest element and finding the rank of any given key.

---

### P
* **Pairing Heap**: A self-adjusting priority queue that provides exceptional empirical performance with simple pointer manipulation; supports $O(1)$ heap merge and insertion, and $O(\log N)$ amortized extraction.
* **Patricia Trie (Radix Tree)**: A space-compressed trie where every node with a single child is collapsed with its descendant, converting single-character branching chains into edge strings.
* **Persistent Data Structure**: A data structure that preserves previous versions of itself under state mutations. Partially persistent structures permit queries on all historical versions but writes only to the latest; fully persistent structures permit branches and writes to any version.
* **Potential Method**: An amortized analysis technique defining an energy potential function $\Phi(D) \ge 0$ over data structure states, computing amortized costs as $\hat{c}_i = c_i + \Phi(D_i) - \Phi(D_{i-1})$.
* **Push-Relabel Algorithm**: An efficient network flow algorithm operating via localized operations: pushing excess preflow along admissible edges and relabeling vertex heights, achieving $O(V^2 E)$ or $O(V^3)$ runtimes.

---

### Q
* **Quickselect**: An expected $O(N)$ selection algorithm that identifies the $k$-th smallest element in an unsorted sequence by recursively partitioning around a chosen pivot element.

---

### R
* **R-Tree**: A height-balanced multiway spatial index grouping geometric objects into hierarchically bounded Minimum Bounding Rectangles (MBRs), optimized for multi-dimensional spatial queries.
* **Rabin-Karp Algorithm**: A string-searching algorithm utilizing polynomial rolling hashes to identify pattern occurrences in text in $O(N + M)$ expected time.
* **Rank and Select**: Fundamental bitvector query primitives: $\text{rank}_b(i)$ returns the number of occurrences of bit $b$ up to index $i$; $\text{select}_b(k)$ returns the array index of the $k$-th occurrence of bit $b$.
* **Red-Black Tree**: A balanced binary search tree adhering to color invariants (black root, red nodes have black children, equal black-height along all root-to-leaf paths), bounding maximum height to $2 \log_2(N + 1)$.
* **Ring Buffer (Circular FIFO)**: A contiguous fixed-capacity buffer utilizing modular index arithmetic to execute $O(1)$ FIFO queue operations without data shifting, optimal for single-producer single-consumer lock-free pipelines.
* **Robin Hood Hashing**: An open-addressing hash table optimization that minimizes probe sequence variance by allowing inserted elements to displace existing elements that have traversed fewer probing steps.

---

### S
* **Segment Tree**: A binary tree over an array where each node maintains an associative summary of an interval. Supports $O(\log N)$ point and range queries, extensible to $O(\log N)$ range updates via lazy propagation.
* **Skip List**: A probabilistic multi-level linked list that offers $O(\log N)$ search, insertion, and deletion without requiring complex tree rotations or balance factors.
* **Sparse Table**: A static lookup table answering Range Minimum Queries (RMQ) in $O(1)$ time after $O(N \log N)$ preprocessing, leveraging power-of-two overlapping intervals for idempotent operations.
* **Splay Tree**: A self-adjusting binary search tree that rotates recently accessed nodes to the root via splay steps, ensuring $O(\log N)$ amortized operation times and satisfying dynamic optimality properties.
* **Suffix Automaton (DAWG)**: A minimal directed acyclic word graph representing the complete set of substrings of a string in $O(N)$ states and transitions, facilitating linear-time text analysis.

---

### T
* **Tarjan's Strongly Connected Components Algorithm**: A depth-first search algorithm partitioning directed graphs into maximal strongly connected subgraphs in $O(V + E)$ linear time using DFS discovery times and ancestor reachability low-links.
* **Treap**: A randomized balanced binary search tree where each node maintains a search key and an independently assigned random heap priority, guaranteeing $O(\log N)$ expected tree height.
* **Trie (Prefix Tree)**: An ordered search tree where keys are represented as paths with edges corresponding to individual characters, yielding $O(L)$ prefix lookup time proportional to key length.
* **Two Pointers**: An algorithmic traversal paradigm maintaining two positional indices that traverse linear sequences monotonically, solving constraint satisfaction and range problems in $O(N)$ time.

---

### U
* **Ukkonen's Algorithm**: An online, linear-time ($O(N)$) algorithm for constructing suffix trees by streaming text characters sequentially and managing an active traversal point with suffix links.

---

### V
* **van Emde Boas Tree (vEB Tree)**: A recursive integer priority queue that achieves $O(\log \log U)$ predecessor, successor, insertion, and deletion over a fixed universe $U$ using recursive square-root universe decomposition.
* **Vector Clock**: A distributed algorithm that captures causal event dependencies across distributed nodes without synchronized physical clocks, identifying concurrency and race conditions.

---

### W
* **Wait-Free**: The strongest non-blocking concurrency progress guarantee: every thread is guaranteed to complete its operation within a bounded number of execution steps, completely immune to thread contention or starvation.
* **Wavelet Tree**: A succinct data structure that recursively partitions an alphabet over a sequence using bitvectors, answering range quantile, rank, and select queries in $O(\log |\Sigma|)$ time.
* **Work-Stealing Deque**: A concurrency load-balancing structure (e.g. Chase-Lev deque) where owner threads push and pop tasks from the deque head in LIFO order, while idle worker threads steal tasks from the deque tail in FIFO order.

---

### X
* **X-Fast Trie**: A bitwise trie over an integer universe $U$ that stores pointers in level-wise hash tables, accelerating predecessor and successor lookups to $O(\log \log U)$ time on the Word RAM model with $O(N \log U)$ space.

---

### Y
* **Y-Fast Trie**: An optimization over the X-Fast Trie that groups elements into balanced binary search trees of size $O(\log U)$, reducing space complexity to optimal $O(N)$ while preserving $O(\log \log U)$ query times.

---

### Z
* **Z-Algorithm**: A linear-time ($O(N)$) string analysis algorithm computing an array $Z$ where $Z[i]$ represents the length of the longest substring starting at index $i$ that matches a prefix of the string.
