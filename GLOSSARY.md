# Awesome DSA Glossary

A quick reference glossary of canonical terms, complexity classes, and physical computing concepts.

---

### A
* **Amortized Complexity**: The average cost per operation over a worst-case sequence of operations (e.g. dynamic array geometric growth).
* **Associative Operation**: An operation $\star$ where $(a \star b) \star c = a \star (b \star c)$. Required for Segment Trees and Parallel Prefix.
* **AVL Tree**: A self-balancing binary search tree where heights of two child subtrees of any node differ by at most one.

### B
* **B-Tree / B+ Tree**: A self-balancing multiway search tree optimized for systems that read and write large blocks of memory (disk pages).
* **Big-O ($O$)**: An asymptotic upper bound characterizing the limiting behavior of an algorithm.
* **Bloom Filter**: A space-efficient probabilistic data structure used to test whether an element is a member of a set (allows false positives, zero false negatives).

### C
* **Cache Line**: The unit of data transfer between main memory and CPU cache, almost universally 64 bytes on modern x86/ARM architectures.
* **Cache Locality (Spatial/Temporal)**: Spatial locality refers to accessing contiguous memory addresses; temporal locality refers to accessing the same memory address repeatedly.
* **Centroid Decomposition**: A divide-and-conquer technique on trees that repeatedly removes the centroid (node whose removal leaves subtrees of size $\le N/2$) to achieve $O(\log N)$ tree depth.
* **Cuckoo Hashing**: An open-addressing hashing scheme using two hash functions and two tables, guaranteeing $O(1)$ worst-case lookup time.

### D
* **Disjoint Set Union (DSU)**: A data structure that tracks a set of elements partitioned into disjoint subsets, supporting union and find in $O(\alpha(N))$ time.

### F
* **False Sharing**: A performance-degrading issue in multithreaded systems where threads on distinct CPU cores modify independent variables that reside on the same 64-byte cache line.
* **Fenwick Tree (Binary Indexed Tree)**: An array-backed tree supporting prefix sum queries and point updates in $O(\log N)$ time using bit manipulation (`i & (-i)`).

### H
* **Heavy-Light Decomposition (HLD)**: Decomposes a tree into disjoint linear paths, allowing path queries and subtree updates via Segment Trees in $O(\log^2 N)$ or $O(\log N)$ time.
* **HyperLogLog**: A probabilistic algorithm that estimates the cardinality (distinct elements) of a multiset using small fixed memory via logarithmic register hashing.

### L
* **Least Recently Used (LRU)**: A cache eviction policy that discards the least recently accessed items first, commonly implemented with a Hash Map + Doubly Linked List.
* **Log-Structured Merge-Tree (LSM-Tree)**: A write-optimized data structure that buffers writes in memory (MemTable) before writing sorted immutable files (SSTables) to disk.
* **Lowest Common Ancestor (LCA)**: Given two nodes $u$ and $v$ in a rooted tree, the deepest node that is an ancestor of both.

### M
* **Monotonic Stack**: A stack that preserves a strictly increasing or decreasing order of elements to answer "next greater/smaller element" queries in $O(1)$ amortized time.

### R
* **Radix Tree (Patricia Trie)**: A space-optimized trie where each node with only one child is merged with its child.
* **Red-Black Tree**: A balanced binary search tree with nodes colored red or black, enforcing balance such that the longest path is at most twice the shortest path.

### S
* **Skip List**: A probabilistic data structure with layered linked lists that allows $O(\log N)$ search, insertion, and deletion without complex tree rotations.
* **Suffix Automaton**: A directed acyclic word graph (DAWG) representing all substrings of a given string in $O(N)$ states and transitions.
