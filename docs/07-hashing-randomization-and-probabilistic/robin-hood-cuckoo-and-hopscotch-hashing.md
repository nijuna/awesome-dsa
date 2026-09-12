---
title: "Robin Hood, Cuckoo, and Hopscotch Hashing"
difficulty: "Advanced"
domains: ["Hashing, Randomization and Probabilistic", "Advanced Data Structures", "Locality and Data-Oriented Design"]
prerequisites: ["Hash Functions", "Hash Tables and Collisions", "RAM Model vs Real Machines", "Locality and Data-Oriented Design"]
related_topics: ["Consistent Hashing", "Bloom and Cuckoo Filters", "High-Performance Caching"]
---

# Robin Hood, Cuckoo, and Hopscotch Hashing

> [!NOTE]
> **The Modern Open-Addressing Renaissance:**
> Traditional separate chaining (`std::unordered_map` with linked lists) is catastrophic on modern hardware: every collision traverses a pointer chase to a random heap location, incurring 100+ CPU cycle L3/RAM cache miss penalties.
> Modern systems rely on **open addressing**, where all keys reside in a single contiguous array.
> Three brilliant algorithmic designs overcome the historical pitfalls of open addressing (clustering, high variance, cascade deletions):
> 1. **Robin Hood Hashing (Celis, 1986):** Reduces probe length variance by "stealing from the rich and giving to the poor".
> 2. **Cuckoo Hashing (Pagh & Rodler, 2004):** Guarantees **$O(1)$ deterministic worst-case lookup time** using two independent hash tables and eviction chains.
> 3. **Hopscotch Hashing (Herlihy, Shavit, & Tzafrir, 2008):** Guarantees that every element resides within a small bounded neighborhood (e.g. 32 slots) of its initial hash bucket using bitmasks.

> [!TIP]
> **Robin Hood's Early-Exit Invariant:**
> In standard linear probing, an unsuccessful search must probe until it hits an empty slot (which can take dozens of steps at high load factors).
> In Robin Hood Hashing, keys along a probe sequence are kept sorted by their Probe Sequence Length (PSL).
> The instant the search reaches a slot where:
> $$\text{PSL}(\text{current\_slot}) < \text{current\_probe\_distance}$$
> the search **terminates immediately with a miss**! The key cannot possibly exist further down the table.

> [!WARNING]
> **Cuckoo Eviction Loops:**
> When inserting into a Cuckoo hash table, an item can trigger a chain of evictions that forms an infinite directed cycle in the underlying cuckoo graph.
> Every cuckoo implementation must impose a maximum eviction threshold (typically $M = 50$ or $2 \log N$). Exceeding $M$ indicates a cycle and requires **immediate table rehashing** with freshly seeded hash functions.

```mermaid
flowchart TD
    subgraph RobinHood["Robin Hood Hashing (Steal from the Rich)"]
        direction TB
        Incoming["Incoming Key X (PSL = 3)"]
        Occupant["Slot occupied by Key Y (PSL = 1)"]
        Swap["PSL(X) > PSL(Y):\nSwap X into slot!\nY becomes incoming key (PSL = 2)"]
        Incoming --> Occupant --> Swap
    end

    subgraph CuckooHashing["Cuckoo Hashing (Deterministic O(1) Lookup)"]
        direction LR
        K["Key K"] -->|h1(K)| T1["Table 1 Slot"]
        K -->|h2(K)| T2["Table 2 Slot"]
        T1 -.->|"If full: Evict occupant to T2"| T2
    end
```

---

## 1. Core Mental Model & Motivation

On a modern CPU (e.g. Intel Core or AMD Zen), an L1 cache hit takes $\approx 1$ ns (4 cycles), while an off-chip RAM access takes $\approx 70$ ns (250+ cycles).
Pointers are the enemy of throughput.

Open addressing stores all elements in flat contiguous memory:
- Traversing adjacent slots streams sequentially through the CPU hardware prefetcher into the 64-byte L1 cache line.
- However, standard linear probing suffers from **primary clustering**: long runs of occupied slots merge into giant clumps, causing probe lengths and search times to explode as load factor $\alpha \to 1$.

Robin Hood, Cuckoo, and Hopscotch hashing provide three distinct mathematical mechanisms to tame probe variance and guarantee near-instantaneous lookups:

---

## 2. Robin Hood Hashing

### 2.1 The PSL Metric
For any key $k$ located at index $pos$ in a table of size $M$:
$$\text{PSL}(k, pos) = (pos - h(k) + M) \bmod M$$
$\text{PSL}$ (Probe Sequence Length) measures how far a key has drifted from its ideal hash bucket.
- $\text{PSL} = 0$: The key is in its ideal bucket ("rich").
- $\text{PSL} \gg 0$: The key was displaced far down the array ("poor").

### 2.2 The Robin Hood Invariant
During insertion, when probing slot $pos$:
- If slot $pos$ is empty: place key and terminate.
- If slot $pos$ contains key $y$ with $\text{PSL}(y, pos) < \text{PSL}(x, pos)$:
  - Swap $x$ and $y$!
  - Key $x$ takes slot $pos$ (enriching the poor).
  - Key $y$ now becomes the displaced key to be inserted further down with $\text{PSL}(y, pos + 1) = \text{PSL}(y, pos) + 1$.
- Repeat until an empty slot is filled.

### 2.3 Backward-Shift Deletion (Zero Tombstones!)
A major weakness of standard open addressing is tombstone accumulation.
Robin Hood hashing completely avoids tombstones:
1. Clear the target slot $pos$.
2. Inspect the next slot $next = (pos + 1) \bmod M$.
3. If slot $next$ is empty or has $\text{PSL} == 0$, stop.
4. Otherwise, shift the element from $next$ backwards into $pos$, decrement its PSL by 1, set $pos = next$, and repeat!
This maintains the contiguous PSL-ordered invariant and eliminates tombstone bloat entirely.

---

## 3. Cuckoo Hashing

```mermaid
flowchart LR
    Insert["Insert key x"] --> CheckT1{"Is Table 1 [h1(x)] empty?"}
    CheckT1 -->|Yes| PlaceT1["Place x in Table 1"]
    CheckT1 -->|No| EvictT1["Place x in Table 1\nEvict current occupant y\ny must move to Table 2 [h2(y)]"]
    EvictT1 --> CheckT2{"Is Table 2 [h2(y)] empty?"}
    CheckT2 -->|Yes| PlaceT2["Place y in Table 2"]
    CheckT2 -->|No| EvictT2["Place y in Table 2\nEvict current occupant z\nz moves to Table 1 [h1(z)]"]
    EvictT2 --> CycleCheck{"Evictions > MAX_LOOP ?"}
    CycleCheck -->|No| EvictT1
    CycleCheck -->|Yes (Cycle Detected)| Rehash["REHASH Entire Table with New Seeds"]
```

### 3.1 Worst-Case $O(1)$ Lookup Guarantee
In Cuckoo Hashing:
$$\text{Lookup}(k): \quad \text{Check } T_1[h_1(k)] \quad \text{and} \quad T_2[h_2(k)]$$
- At most **two memory reads** are performed.
- Both memory addresses can be computed in parallel and prefetched concurrently!
- Zero worst-case probe chains; lookups are strictly $O(1)$.

### 3.2 The Cuckoo Graph
Consider a bipartite graph where nodes are table slots and each key $k$ is an undirected edge $(h_1(k), h_2(k))$.
- A valid placement of keys corresponds to orienting each edge toward the slot that holds it.
- A connected component can be successfully placed if and only if it contains at most one cycle (a pseudo-tree).
- When an insertion introduces an edge that creates a second cycle in a component, placement becomes mathematically impossible, triggering a **rehash**.
- For 2 hash functions, the theoretical maximum load factor is $\alpha \approx 50\%$. Using 3 hash functions increases the threshold to $\alpha \approx 91\%$.

---

## 4. Hopscotch Hashing

Hopscotch hashing enforces that for any key $k$, its actual position in the table is within a small bounded **neighborhood** $H$ of its ideal hash bucket $h(k)$ (typically $H = 32$ or $64$, fitting in a single 32-bit/64-bit integer bitmask):
$$\text{Actual Position}(k) \in [h(k), \quad h(k) + H - 1]$$

### 4.1 Neighborhood Bitmask (`hop_info`)
Each bucket $i$ stores an $H$-bit unsigned integer `hop_info`:
- Bit $j$ of `hop_info[i]` is 1 if the item at slot $(i + j) \bmod M$ hashes to bucket $i$.
- To search for $k$: examine only the set bits in `hop_info[h(k)]`.
- Search requires scanning at most $H$ slots, all located within the same or adjacent cache lines!

### 4.2 Hopscotch Insertion Displacement
1. Hash $k$ to $i = h(k)$.
2. Linearly probe starting at $i$ to find the first empty slot $empty$.
3. If $empty$ is within $H - 1$ of $i$, insert $k$ at $empty$, set bit in `hop_info[i]`, and finish.
4. If $empty$ is too far ($empty \ge i + H$):
   - Find an element $y$ between $empty - H + 1$ and $empty - 1$ whose hash bucket $h(y)$ allows it to be displaced into $empty$.
   - Swap $y$ into $empty$, update $y$'s hop bitmask, and make $y$'s old slot the new $empty$.
   - Repeat until $empty$ is within $x$'s neighborhood!

---

## 5. Algorithmic Complexity Comparison

| Hash Table Architecture | Lookup (Average) | Lookup (Worst-Case) | Insert (Average) | Insert (Worst-Case) | Max Practical Load Factor ($\alpha$) |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **Separate Chaining** | $O(1)$ | $O(N)$ | $O(1)$ | $O(1)$ | $\alpha > 100\%$ (Heap overhead) |
| **Linear Probing** | $O(1)$ | $O(N)$ | $O(1)$ | $O(N)$ | $\alpha \approx 60\% - 70\%$ |
| **Robin Hood Hashing** | **$O(1)$ (Low var)** | $O(\log N)$ | $O(1)$ | $O(N)$ | **$\alpha \approx 90\% - 95\%$** |
| **Cuckoo Hashing** | **$O(1)$** | **$O(1)$ (Deterministic)**| $O(1)$ amortized | $O(N)$ (Rehash) | $\alpha \approx 50\%$ (2-way), $91\%$ (3-way) |
| **Hopscotch Hashing** | **$O(1)$** | **$O(H) = O(1)$** | $O(1)$ amortized | $O(N)$ | $\alpha \approx 90\%$ |

---

## 6. High-Performance Engineering & CPU Cache Dynamics

### 6.1 Hardware Prefetching Friendly
Robin Hood and Hopscotch hashing are optimized for sequential spatial locality:
- Linear probes follow the CPU streaming prefetcher directly.
- In contrast, quadratic probing and double hashing jump unpredictably across the array, causing an L1/L2 miss on nearly every collision step.

### 6.2 SIMD Group Probing (Google SwissTable / Rust `hashbrown`)
Modern production open-addressing tables (such as Google's `absl::flat_hash_map` and Rust's `hashbrown`) combine Robin Hood concepts with 16-byte SIMD metadata control bytes:
- An auxiliary 1-byte-per-slot control array stores top 7 bits of the hash (or empty/sentinel markers).
- An AVX2 / SSE instruction (`_mm_cmpeq_epi8`) inspects **16 slots simultaneously in a single CPU cycle**, eliminating branch mispredictions.

---

## 7. Edge Cases & Failure Modes

1. **Cuckoo Eviction Limit:** Cuckoo insertion must abort when recursion/loop count exceeds $50$ to avoid infinite ping-pong cycles.
2. **Robin Hood Wrap-Around Arithmetic:** Modulo arithmetic for circular arrays: $(pos - h(k) + M) \bmod M$.
3. **Empty Table Searches:** Searching in a sparsely populated or empty table must terminate on the first empty slot.

---

## 8. Reference Implementation Architecture

Both C++17 and Python 3 reference implementations implement:
- **`RobinHoodHashMap<Key, Value>`**:
  - Open addressing with entry displacement swapping
  - Early-exit miss detection
  - Backward-shift deletion without tombstones
  - Arthur's Two-Layer API (`try_find`, `insert`, `erase`, `size`, `load_factor`)
- **`CuckooHashMap<Key, Value>`**:
  - Two independent hash functions (seeded Murmur/splitmix variants)
  - Strict deterministic $O(1)$ worst-case lookup
  - Eviction loop detection with automatic table rehashing
  - Arthur's Two-Layer API

---

## 9. Differential Testing & Oracle Verification Strategy

- Both Robin Hood and Cuckoo implementations are differential fuzz-tested against `std::unordered_map` / Python `dict` across 1,000 randomized operations.
- Correctness of key-value associations, update semantics, deletion handling, and load factor expansion are strictly asserted after every step.

---

## 10. Engineering Pitfalls & Optimization Notes

> [!CAUTION]
> **Tombstone Clutter in Linear Hashing vs Backward-Shift:**
> Never use `DELETED` sentinel tombstones in Robin Hood tables if backward-shift deletion is feasible.
> Tombstones degrade subsequent lookups and require periodic garbage collection rehashes. Backward-shift maintains compact runs and keeps average PSL minimal.

---

## 11. Real-World Applications & Industry Context

1. **Rust Standard Library (`std::collections::HashMap`):** Powered by `hashbrown`, an open-addressing table based on SwissTable and Robin Hood principles.
2. **High-Frequency Trading (HFT):** Cuckoo hashing is widely deployed in matching engines and order book lookups where the 99.9th percentile tail latency must never exceed 2 memory reads.
3. **Network Switch Flow Tables:** Hardware Ternary CAMs and router packet processors implement Cuckoo and Hopscotch hashing to guarantee wire-speed 100Gbps IP packet forwarding.

---

## 12. Curated Academic References

1. **Celis, Pedro (1986):** *Robin Hood Hashing*. PhD thesis, University of Waterloo, Computer Science Department, Technical Report CS-86-14.
2. **Pagh, Rasmus & Rodler, Flemming Friche (2004):** *Cuckoo hashing*. Journal of Algorithms, 51(2), pp. 122–144.
3. **Herlihy, Maurice, Shavit, Nir, & Tzafrir, Moran (2008):** *Hopscotch hashing*. DISC 2008, Lecture Notes in Computer Science, vol. 5218, pp. 350–364.
