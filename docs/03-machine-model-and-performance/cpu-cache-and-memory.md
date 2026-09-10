---
title: "CPU Cache, Memory Hierarchy & Data Locality"
difficulty: "Intermediate"
domains: ["Systems", "Hardware", "Theory", "Competitive Programming"]
prerequisites: ["Asymptotic Analysis", "Dynamic Arrays"]
related_topics: ["B-Trees", "Matrix Multiplication", "False Sharing", "Benchmarking Pitfalls"]
---

# CPU Cache, Memory Hierarchy & Data Locality

> [!NOTE]
> Modern CPUs execute instructions in nanosecond fractions, but main memory (DRAM) takes tens of nanoseconds to respond. The physical arrangement of data structures in memory often impacts real-world performance far more than asymptotic Big-O factors.

---

## 1. Why This Matters

Classical algorithm analysis uses the **Word RAM model**: accessing any memory address is assumed to take constant $O(1)$ time.

In physical silicon, this assumption is false. A CPU core can execute arithmetic operations in $\sim 0.3\text{ ns}$, but fetching a 64-bit integer from main RAM can take $60\text{ to }100\text{ ns}$ ($\sim 200\times$ slower!). To prevent the CPU from stalling, modern processors deploy a hierarchical cache architecture ($L1, L2, L3$).

Understanding the memory hierarchy explains practical paradoxes:
* Why sequential iteration over an array is up to $50\times$ faster than traversing a linked list of identical size.
* Why B-Trees dominate disk and memory-mapped indexes over binary search trees.
* Why cache-oblivious algorithms outperform standard divide-and-conquer algorithms.

---

## 2. Core Intuition & The Physical Architecture

When your program requests a byte from memory, the CPU hardware does not fetch a single byte. It fetches an entire contiguous chunk called a **Cache Line** (standardized at **64 bytes** across modern x86 and ARM processors).

```text
+-------------------------------------------------------------------------+
|                           CPU CORE (Registers)                          |
|                       Cycle time: ~0.3 ns (3-4 GHz)                     |
+-------------------------------------------------------------------------+
                                     │
                 ┌───────────────────┴───────────────────┐
                 ▼                                       ▼
        +------------------+                    +------------------+
        |  L1 Data Cache   |                    | L1 Inst Cache    |
        |  ~32-64 KB       |                    | ~32-64 KB        |
        |  Latency: ~1 ns  |                    | Latency: ~1 ns   |
        +------------------+                    +------------------+
                 │
                 ▼
        +----------------------------------------------------------+
        |                      L2 Unified Cache                    |
        |                      ~512 KB - 1 MB / core               |
        |                      Latency: ~3-5 ns                    |
        +----------------------------------------------------------+
                 │
                 ▼
        +----------------------------------------------------------+
        |                 L3 Shared Cache (LLC)                    |
        |                 ~16 MB - 64+ MB shared                   |
        |                 Latency: ~12-20 ns                       |
        +----------------------------------------------------------+
                 │
                 ▼
        +----------------------------------------------------------+
        |                     Main Memory (DRAM)                   |
        |                     16 GB - 128+ GB                      |
        |                     Latency: ~60-100 ns                  |
        +----------------------------------------------------------+
```

### The Latency Numbers Every Programmer Should Know (Peter Norvig / Jeff Dean)
* **L1 Cache reference**: $1\text{ ns}$
* **Branch mispredict**: $3\text{ ns}$
* **L2 Cache reference**: $4\text{ ns}$
* **Mutex lock/unlock**: $17\text{ ns}$
* **Main memory (RAM) reference**: $100\text{ ns}$ ($100\times$ slower than L1)
* **Read 1 MB sequentially from memory**: $3,000\text{ ns}$
* **Solid-State Drive (SSD) I/O**: $16,000\text{ ns}$
* **Rotational Disk seek**: $4,000,000\text{ ns}$ ($40,000\times$ slower than RAM)

---

## 3. Spatial Locality vs. Pointer Chasing

### Case Study: Contiguous Array vs. Linked List

Consider iterating through $10^6$ 64-bit integers (`int64_t`, 8 bytes each).

```text
ARRAY (Contiguous Memory):
[ 8B ][ 8B ][ 8B ][ 8B ][ 8B ][ 8B ][ 8B ][ 8B ]  <-- 1 Cache Line (64 Bytes = 8 elements)
▲
└─ Fetching element 0 automatically loads elements 1 through 7 into L1 cache for free.
   Hardware prefetchers detect sequential strides and load ahead of time.

LINKED LIST (Heap Scattered Nodes):
[ Data | NextPtr ] ───(pointer hop)───> [ Data | NextPtr ] ───(pointer hop)───> ...
0x1040A0 (Cache Miss!)                  0x78B240 (Cache Miss!)
```

1. **In an Array**: Loading the first 8-byte integer pulls the subsequent 7 integers into the $L1$ cache simultaneously. The CPU hardware prefetcher detects linear strides and fetches the next cache lines before the loop even requests them.
2. **In a Linked List**: Every node is allocated dynamically on the heap at an arbitrary address. Traversing to `node->next` requires a pointer dereference into unknown memory, causing an $L1/L2/L3$ cache miss that forces the CPU pipeline to stall for $\sim 200$ cycles.

---

## 4. Benchmark Demonstration (Python vs. C++)

### C++ Cache Stride Experiment

```cpp
#include <iostream>
#include <vector>
#include <chrono>

void benchmark_stride(size_t stride, size_t size) {
    std::vector<int> arr(size, 1);
    long long sum = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < size; i += stride) {
        sum += arr[i];
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    std::chrono::duration<double, std::milli> elapsed = end - start;
    std::cout << "Stride: " << stride << " | Elapsed: " << elapsed.count() << " ms\n";
}
```

When `stride = 1`, traversing $64\text{ MB}$ takes minimal time because every cache line is utilized 100%. As `stride` increases to $16$ (`16 * sizeof(int) = 64 bytes`), every access hits a brand new cache line, wasting 87.5% of fetched memory bandwidth.

---

## 5. False Sharing in Multithreading

**False Sharing** occurs when two distinct threads running on different CPU cores modify independent variables that happen to share the same 64-byte cache line.

```text
Core 0 writes: counter_A          Core 1 writes: counter_B
          │                                 │
          ▼                                 ▼
   +-----------------------------------------------+
   |        Shared Cache Line (64 Bytes)           |
   |   counter_A (8 bytes)   |  counter_B (8 bytes)|
   +-----------------------------------------------+
```

Even though `counter_A` and `counter_B` are logically independent, the cache coherence protocol (e.g. MESI) constantly invalidates the cache line across cores, converting fast cache writes into slow bus-traffic stalls.

### The Fix: Cache-Line Alignment / Padding
In C++17 and later, align variables to `std::hardware_destructive_interference_size`:

```cpp
#include <new>

struct alignas(64) ThreadSafeCounter {
    uint64_t value;
    // Guaranteed to occupy its own isolated cache line
};
```

---

## 6. Real-World Systems Case Studies

* **B-Trees in PostgreSQL & SQLite**: Rather than storing 2 keys per node like a binary search tree, B-Tree nodes are sized to match disk page sizes ($4\text{ KB}$ or $8\text{ KB}$) or CPU cache lines, maximizing fan-out and minimizing cache misses.
* **Linux Kernel `kfifo`**: A circular ring buffer operating over a contiguous memory buffer. It avoids any dynamic `malloc` during enqueue/dequeue operations, guaranteeing maximum L1 cache residency.
* **Game Engine Entity Component Systems (ECS)**: Traditional Object-Oriented game designs use polymorphic pointers (`Entity* -> Update()`). Modern ECS engines lay components out contiguously in flat arrays (Data-Oriented Design) so physics calculations stream sequentially through L1 cache.

---

## 7. Key Takeaways for DSA Practitioners

1. **Arrays win by default**: Unless you need guaranteed $O(1)$ node splicing where pointers are already known, contiguous arrays (`std::vector`, Python `list`) outperform linked lists in both execution speed and memory compactness.
2. **Mind the pointer tax**: On a 64-bit architecture, every pointer costs 8 bytes. A tree node with `left`, `right`, and `parent` pointers wastes 24 bytes of metadata before storing a single byte of user data.
3. **Structure of Arrays (SoA) vs Array of Structures (AoS)**: If an algorithm only queries one field of a struct across $N$ elements, storing fields in parallel contiguous arrays avoids loading unused struct fields into cache lines.
