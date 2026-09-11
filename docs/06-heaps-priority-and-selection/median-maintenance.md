---
title: "Median Maintenance"
difficulty: "Intermediate to Advanced"
domains: ["Algorithms", "Streaming", "Heaps", "Decision Making"]
prerequisites: ["Binary Heaps", "Priority Queues in Practice", "Arrays and Memory Layout", "Basic Complexity Analysis", "Sorting and Order Statistics"]
related_topics: ["Binary Heaps", "d-ary Heaps", "Priority Queues in Practice", "Sliding Window Techniques", "Order Statistics", "Streaming Algorithms"]
---

# Median Maintenance

> [!NOTE]
> Median maintenance is the problem of tracking the median of a dynamically growing stream using sublinear work per update. The standard solution coordinates two complementary heaps: a max-heap for the lower half and a min-heap for the upper half, maintaining balance and ordering invariants so that the median is always available in $O(1)$ time after $O(\log n)$ insertion.
>
> Reference implementations: [C++17 Implementation](../../implementations/cpp/median_maintenance.cpp) | [Python Implementation](../../implementations/python/median_maintenance.py)

> [!TIP]
> **The Two Invariants at a Glance:**
> - **Ordering Invariant**: $\max(low) \le \min(high)$ — every element in the lower half is strictly less than or equal to every element in the upper half.
> - **Size Balance Invariant**: $||low| - |high|| \le 1$ — the two heaps differ in size by at most one element, placing the median directly at the heap boundary.

> [!WARNING]
> A heap does not store full sorted order. Median maintenance works only because we maintain two carefully coordinated heaps with strict cross-heap invariants. If those invariants drift, the reported median becomes incorrect immediately.

---

## 1. Why This Matters

Suppose numbers arrive continuously one at a time:
- Stock tick prices in quantitative market making
- Request latency metrics in high-throughput microservices
- IoT sensor telemetry (temperature, voltage, pressure)
- Packet delays in networking monitoring engines
- Real-time scoring systems and streaming analytics

After each new value arrives, we want the exact current median.

### The Naive Approaches Fail:
1. **Append and Sort**:
   Append each new item to an array and re-sort:
   $$
   O(n \log n)
   $$
   per update. On a stream of $10^6$ elements, this requires $10^{12}$ operations, crashing real-time throughput.
2. **Insertion into a Sorted Array**:
   Use binary search to find the insertion point ($O(\log n)$), then shift subsequent elements rightward:
   $$
   O(n)
   $$
   per update due to memory copying. On large streams, memory bandwidth saturation throttles ingestion.

Median maintenance solves this online in:
- **$O(\log n)$ insertion**
- **$O(1)$ median query**

using two heaps. It is one of the cleanest examples of an online streaming algorithm built from simple data structures and rigorous cross-structure invariants.

---

## 2. What Is the Median?

For a sorted sequence:

$$
a_0 \le a_1 \le \cdots \le a_{n-1}
$$

the median is the middle element.

### Odd $n$
If $n$ is odd, the median is unique and unambiguously the exact center:

$$
a_{\lfloor n/2 \rfloor}
$$

### Even $n$
If $n$ is even, two middle values exist ($a_{n/2 - 1}$ and $a_{n/2}$). Different domains apply distinct conventions:
1. **Lower Median**:
   $$
   a_{n/2 - 1}
   $$
2. **Upper Median**:
   $$
   a_{n/2}
   $$
3. **Arithmetic Mean of Both Middle Values**:
   $$
   \frac{a_{n/2 - 1} + a_{n/2}}{2}
   $$

The dual-heap architecture supports all three conventions natively by inspecting the tops of the two heaps.

---

## 3. Core Idea: Split the Stream into Two Halves

We maintain:
- A **max-heap** `low` containing the lower half of values.
- A **min-heap** `high` containing the upper half of values.

```text
Lower Half (Max-Heap)              Upper Half (Min-Heap)
[ ... smaller elements ... ]  |  [ ... larger elements ... ]
            low.top()         |         high.top()
               \              |             /
                +------ Median Boundary ---+
```

### Why These Specific Heaps?
- `low` provides $O(1)$ access to the **largest element** among the smaller half.
- `high` provides $O(1)$ access to the **smallest element** among the larger half.

Those two boundary elements completely determine the median. We do not care about the relative order of elements deep inside `low` or deep inside `high`; we only care about the values at their interface.

---

## 4. The Two Fundamental Invariants

The correctness of the algorithm relies on preserving two invariants after every insertion:

### 4.1 Ordering Invariant
Every element in `low` must be less than or equal to every element in `high`:

$$
\max(low) \le \min(high)
$$

whenever both heaps are non-empty. This guarantees that no element in the "lower half" is secretly greater than an element in the "upper half."

### 4.2 Size Balance Invariant
The sizes of the two heaps must never differ by more than 1:

$$
\left|\,|low| - |high|\,\right| \le 1
$$

### The Standard Implementation Convention
In production code, a standard convention is to allow `low` to hold the extra element when $n$ is odd:

$$
|low| = |high| \quad \text{or} \quad |low| = |high| + 1
$$

Under this convention:
- **Odd $n$**: The median is simply `low.top()`.
- **Even $n$**: The lower median is `low.top()`, upper median is `high.top()`, and mean median is `(low.top() + high.top()) / 2.0`.

---

## 5. Online Insertion Algorithm

When a new number $x$ arrives:

### Step 1: Routing (Which Heap Receives $x$?)
- If `low` is empty or $x \le \text{low.top()}$, insert $x$ into `low`.
- Otherwise, insert $x$ into `high`.

### Step 2: Balancing (Restoring Size Invariant)
After routing, one heap may have too many elements:
- If $|low| > |high| + 1$:
  Pop the top of `low` and push it into `high`.
- If $|high| > |low|$:
  Pop the top of `high` and push it into `low`.

```cpp
void add(int x) {
    // 1. Route
    if (low.empty() || x <= low.top()) {
        low.push(x);
    } else {
        high.push(x);
    }

    // 2. Rebalance
    if (low.size() > high.size() + 1) {
        high.push(low.top());
        low.pop();
    } else if (high.size() > low.size()) {
        low.push(high.top());
        high.pop();
    }
}
```

### Complexity
- Routing: $1$ heap insertion $\implies O(\log n)$.
- Rebalancing (at most 1 transfer): $1$ pop + $1$ push $\implies O(\log n)$.
- Total Update Latency: $O(\log n)$.
- Median Query: $O(1)$ directly inspecting heap roots.

---

## 6. Step-by-Step Trace

Let the stream arrive in the order: `[5, 2, 10, 4, 8]`

```
1. Insert 5:
   low is empty -> push 5 to low.
   low: [5] (size 1) | high: [] (size 0)
   Median = 5.0

2. Insert 2:
   2 <= low.top() (5) -> push 2 to low.
   low: [5, 2] (size 2) | high: [] (size 0)
   Rebalance: low has 2, high has 0 (|low| > |high| + 1) -> move 5 from low to high.
   low: [2] (size 1) | high: [5] (size 1)
   Median = (2 + 5) / 2.0 = 3.5

3. Insert 10:
   10 > low.top() (2) -> push 10 to high.
   low: [2] (size 1) | high: [5, 10] (size 2)
   Rebalance: high has 2, low has 1 (|high| > |low|) -> move 5 from high to low.
   low: [5, 2] (size 2) | high: [10] (size 1)
   Median = low.top() = 5.0

4. Insert 4:
   4 <= low.top() (5) -> push 4 to low.
   low: [5, 4, 2] (size 3) | high: [10] (size 1)
   Rebalance: low has 3, high has 1 -> move 5 from low to high.
   low: [4, 2] (size 2) | high: [5, 10] (size 2)
   Median = (4 + 5) / 2.0 = 4.5

5. Insert 8:
   8 > low.top() (4) -> push 8 to high.
   low: [4, 2] (size 2) | high: [5, 8, 10] (size 3)
   Rebalance: high has 3, low has 2 -> move 5 from high to low.
   low: [5, 4, 2] (size 3) | high: [8, 10] (size 2)
   Median = low.top() = 5.0
```

---

## 7. Mathematical Correctness Proof

### Theorem
*The two-heap maintenance algorithm correctly reports the median after every update in $O(1)$ time.*

### Proof
1. **Base Case**: For $n=1$, the single element $x$ is placed in `low`. `low.top() = x`, which is the median.
2. **Inductive Invariant Preservation**:
   Assume the invariants hold before inserting $x$:
   - $\max(low) \le \min(high)$
   - $|low| - |high| \in \{0, 1\}$
   
   **Case A**: $x \le \max(low)$. Placing $x$ in `low` preserves $\max(low \cup \{x\}) = \max(low) \le \min(high)$.
   If $|low|$ becomes $|high| + 2$, moving the maximum element $m = \max(low)$ to `high` yields:
   $$
   \max(low') \le m \le \min(high) \le \min(high')
   $$
   The ordering invariant is strictly preserved, and $|low'| = |high'| + 1$.
   
   **Case B**: $x > \max(low)$. Placing $x$ in `high` preserves $\max(low) \le \min(high \cup \{x\})$.
   If $|high|$ becomes $|low| + 1$, moving the minimum element $m = \min(high)$ to `low` yields:
   $$
   \max(low') \le m \le \min(high')
   $$
   The ordering invariant holds, and $|low'| = |high'| + 1$.
3. **Median Extraction**:
   Since all elements in `low` are $\le$ all elements in `high`, and their sizes differ by at most 1:
   - For odd $n = 2k + 1$: $|low| = k + 1$ and $|high| = k$. Exactly $k$ elements are $< \text{low.top()}$ and $k$ elements are $\ge \text{low.top()}$. Thus `low.top()` is by definition element $a_k$, the exact median.
   - For even $n = 2k$: $|low| = k$ and $|high| = k$. The middle two elements are $\max(low) = \text{low.top()}$ and $\min(high) = \text{high.top()}$. Their average is the exact mean median. $\blacksquare$

---

## 8. Sliding-Window Median (Window Size $k$)

> [!WARNING]
> The simple two-heap solution solves the **insert-only** streaming median. The **sliding-window median** is significantly harder because standard binary heaps do not support efficient arbitrary deletion ($O(n)$ search) when elements exit the window.

In a sliding window of size $k$, every step involves:
1. Adding the new element entering the right side of the window.
2. Removing the oldest element exiting the left side of the window.

### Strategy: Two Heaps with Hash-Map Lazy Deletion

Instead of searching through the heap to delete the expired element immediately, we defer the deletion:
1. Maintain an auxiliary hash table `delayed[val]` counting how many instances of `val` are scheduled for deletion.
2. Track the count of *valid* (non-deleted) elements in each heap: `low_valid` and `high_valid`.
3. When an element $x$ expires:
   - Increment `delayed[x]`.
   - Decrement `low_valid` if $x \le \text{low.top()}$, else decrement `high_valid`.
4. **Pruning**: Whenever an element scheduled for deletion reaches the root of either heap (`low.top()` or `high.top()`), pop it off and decrement its entry in `delayed`.
5. Rebalance based on `low_valid` and `high_valid`.

```cpp
void prune(std::priority_queue<int>& heap, std::unordered_map<int, int>& delayed) {
    while (!heap.empty() && delayed[heap.top()] > 0) {
        --delayed[heap.top()];
        heap.pop();
    }
}
```

### Complexity
- Each element is pushed once and popped at most once from each heap.
- Amortized time per sliding step: **$O(\log k)$**.
- Space: $O(k)$ active elements.

---

## 9. Alternative Architectures

| Approach | Running Median (Insert-Only) | Sliding Window (Insert + Delete) | Space | Implementation Complexity |
| :--- | :---: | :---: | :---: | :---: |
| **Dual Heaps** | **$O(\log n)$ update, $O(1)$ query** | Requires lazy deletion hash-map | $O(n)$ or $O(k)$ | **Very Low** (STL priority_queue) |
| **Augmented Red-Black Tree** | $O(\log n)$ update, $O(\log n)$ query | $O(\log k)$ update, $O(\log k)$ query | $O(n)$ | High (order-statistic tree) |
| **Dual Multisets (`std::multiset`)** | $O(\log n)$ update, $O(1)$ query | $O(\log k)$ update, $O(1)$ query | $O(n)$ | Moderate (pointer tree overhead) |
| **Quantile Sketch (t-digest / GK)** | $O(1)$ update, approximate query | $O(1)$ update, approximate query | **$O(1)$ bounded** | Moderate (approximate statistics) |

---

## 10. Practical Engineering Considerations

1. **Integer Overflow During Averaging**:
   When computing the mean of two 32-bit signed integers in C/C++, never write:
   ```cpp
   // WRONG: can overflow if low.top() + high.top() > INT_MAX
   int median = (low.top() + high.top()) / 2;
   ```
   Instead, cast to a wider 64-bit integer or double:
   ```cpp
   // CORRECT:
   double median = (static_cast<double>(low.top()) + static_cast<double>(high.top())) / 2.0;
   ```
2. **Handling Floating-Point Streams**:
   If tracking floating-point metrics (`double`), `NaN` values violate strict weak ordering in `std::priority_queue`, causing undefined behavior. Always sanitize or filter `NaN` values before pushing to the heaps.
3. **Memory Footprint**:
   For an unbounded stream, $n$ grows indefinitely. If the stream runs for days, dual heaps will consume unbounded RAM. For long-running observability systems, either:
   - Bound the stream with a **sliding window** ($k = 10,000$).
   - Use an **approximate quantile sketch** (such as $t$-digest or DDSketch) that maintains fixed, bounded memory footprint.

---

## 11. Curated Problems

### 1. LeetCode 295 — Find Median from Data Stream
- **Difficulty**: Hard
- **Pattern**: Exact dual-heap balancing over an unbounded stream.
- **Key Insight**: Maintain $|low| - |high| \in \{0, 1\}$ and route based on `low.top()`.

### 2. LeetCode 480 — Sliding Window Median
- **Difficulty**: Hard
- **Pattern**: Dual heaps coupled with hash-map lazy deletion.
- **Key Insight**: Defer deletions until expired nodes reach the top of either heap, maintaining valid size counters to drive rebalancing.

---

## 12. Related Topics & Further Reading

### Internal Documentation
- **[Binary Heaps](binary-heaps.md)**: Array representations, Floyd's build-heap algorithm.
- **[d-ary Heaps](d-ary-heaps.md)**: Multi-way branching factors for cache tuning.
- **[Priority Queues in Practice](priority-queues-in-practice.md)**: Hardware tradeoffs, event loops, and benchmark analysis.
- **[Sliding Window Techniques](../19-problem-solving-patterns/sliding-window.md)**: Two-pointer windows and rolling aggregation patterns.
