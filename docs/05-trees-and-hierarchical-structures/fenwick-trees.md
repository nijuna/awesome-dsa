---
title: "Fenwick Trees (Binary Indexed Trees): Theory, Bitwise Mechanics & Multi-Dimensional Variants"
difficulty: "Intermediate"
domains: ["Data Structures", "Competitive Programming", "Theory", "Systems"]
prerequisites: ["Arrays and Memory Layout", "Bit Manipulation & Two's Complement", "Prefix Sums"]
related_topics: ["Segment Trees", "Sparse Tables", "Range Minimum Query", "CPU Cache and Memory"]
---

# Fenwick Trees (Binary Indexed Trees)

> [!NOTE]
> A **Fenwick Tree** (also known as a **Binary Indexed Tree / BIT**) maintains prefix sums and point updates over a mutable array in $O(\log N)$ time and $O(N)$ space, requiring zero tree pointers and using pure bitwise arithmetic over a flat array.

---

## 1. Why This Matters: The Mutable Prefix Dilemma

Consider maintaining an array of $N$ numbers subject to two operations:
1. `Update(i, delta)`: Add $\Delta$ to element at index $i$.
2. `Query(l, r)`: Compute the sum $\sum_{k=l}^{r} A[k]$.

| Approach | Point Update | Range Sum Query | Memory Overhead | Implementation Complexity |
| :--- | :--- | :--- | :--- | :--- |
| **Flat Array** | **$O(1)$** | $O(N)$ | Zero ($1N$) | Trivial |
| **Prefix Sum Array** | $O(N)$ (cascade updates) | **$O(1)$** | Zero ($1N$) | Trivial |
| **Segment Tree** | $O(\log N)$ | $O(\log N)$ | $4N$ elements | Moderate (Recursive / Node hierarchy) |
| **Fenwick Tree** | **$O(\log N)$** | **$O(\log N)$** | **Zero ($1N$ flat array)** | **Trivial (~10 lines of bitwise code)** |

Invented by **Peter Fenwick in 1994** for data compression entropy coding (CABAC), the Fenwick Tree achieves the optimal balance: $O(\log N)$ for both queries and updates with **the smallest constant factor and memory footprint of any dynamic range structure in computer science**.

---

## 2. Core Intuition & The `i & (-i)` Bitwise Engine

The Fenwick Tree operates by decomposing integers into powers of two using their **binary representations**.

Every positive integer can be uniquely represented as a sum of distinct powers of 2. A Fenwick tree superimposes a hierarchical interval tree over a standard **1-indexed flat array**.

### The Lowest Set Bit (LSB / Two's Complement Trick)
How do we isolate the lowest significant bit of an integer $i$?

$$\text{LSB}(i) = i \ \& \ (-i)$$

In two's complement binary representation, $-i = \sim i + 1$.
* Example: $i = 12 = 00001100_2$
* $\sim i = 11110011_2$
* $-i = \sim i + 1 = 11110100_2$
* $i \ \& \ (-i) = 00001100_2 \ \& \ 11110100_2 = 00000100_2 = 4$

### The Responsibility Invariant
In a 1-indexed Fenwick array `tree[]`:
> `tree[i]` stores the sum of elements in the half-open range **$(i - \text{LSB}(i), i]$**, encompassing exactly $\text{LSB}(i)$ elements ending at index $i$.

```text
Index (Binary)   LSB    Range Covered        Length
---------------------------------------------------
1     (0001_2)    1     (0, 1]  -> A[1]       1
2     (0010_2)    2     (0, 2]  -> A[1..2]    2
3     (0011_2)    1     (2, 3]  -> A[3]       1
4     (0100_2)    4     (0, 4]  -> A[1..4]    4
5     (0101_2)    1     (4, 5]  -> A[5]       1
6     (0110_2)    2     (4, 6]  -> A[5..6]    2
7     (0111_2)    1     (6, 7]  -> A[7]       1
8     (1000_2)    8     (0, 8]  -> A[1..8]    8
```

### Visual Model of Responsibility

```text
Tree Level 3: [---------------------- tree[8] = A[1..8] ----------------------]
Tree Level 2: [---------- tree[4] = A[1..4] ----------]
Tree Level 1: [--- tree[2] = A[1..2] ---]      [--- tree[6] = A[5..6] ---]
Tree Level 0: [t[1]=A[1]]      [t[3]=A[3]]      [t[5]=A[5]]      [t[7]=A[7]]
Index:             1       2        3       4        5       6        7       8
```

---

## 3. Key Operations & Traversal Direction

### 1. Prefix Query `query(i)`: Moving Downwards by Subtracting LSB
To compute $\sum_{k=1}^{i} A[k]$, accumulate `tree[i]` and jump to the next lower interval by removing the lowest set bit:
$$i \leftarrow i - (i \ \& \ (-i))$$

* Example: Query prefix sum of index 7:
  * $7 = 0111_2 \to \text{add } \text{tree}[7]$ (covers $A[7]$)
  * $7 - 1 = 6 = 0110_2 \to \text{add } \text{tree}[6]$ (covers $A[5..6]$)
  * $6 - 2 = 4 = 0100_2 \to \text{add } \text{tree}[4]$ (covers $A[1..4]$)
  * $4 - 4 = 0 \to \text{Done!}$
  * $\text{Sum} = A[7] + A[5..6] + A[1..4] = A[1..7]$. Total hops: $3 = O(\log N)$.

### 2. Point Update `add(i, delta)`: Moving Upwards by Adding LSB
When element $A[i]$ increases by $\Delta$, update all intervals containing index $i$ by ascending the bitwise parent chain:
$$i \leftarrow i + (i \ \& \ (-i))$$

* Example: Add $\Delta$ to element 3:
  * $3 = 0011_2 \to \text{update } \text{tree}[3]$
  * $3 + 1 = 4 = 0100_2 \to \text{update } \text{tree}[4]$
  * $4 + 4 = 8 = 1000_2 \to \text{update } \text{tree}[8]$
  * Total hops: $O(\log N)$.

---

## 4. Hardware & Memory Reality

* **Zero Pointer Indirection**:
  * A Segment Tree requires storing child indices or pointers (`left_child`, `right_child`), allocating $4N$ integers.
  * A Fenwick Tree uses a single contiguous array of size $N + 1$.
* **Cache Line Residency**:
  * Because $i \ \& \ (-i)$ operations jump across contiguous array indices without chasing heap pointers, traversing a Fenwick Tree produces minimal CPU cache misses and zero allocator fragmentation.
* **SIMD & Branchless Execution**:
  * The core update and query loops contain zero data-dependent branches, allowing modern out-of-order CPUs to pipeline iterations aggressively.

---

## 5. Advanced Variants & Extensions

### 1. Range Update, Point Query (Difference Array Fenwick)
To support adding $\Delta$ to all elements in range $[l, r]$ and querying single element $A[k]$:
* Maintain a Fenwick tree over the **difference array** $D[i] = A[i] - A[i-1]$.
* `range_add(l, r, delta)` $\to$ `add(l, delta)` and `add(r + 1, -delta)`.
* `point_query(k)` $\to$ `query(k)` (reconstructs $A[k] = \sum_{i=1}^{k} D[i]$).

### 2. Range Update, Range Query (Dual Fenwick Trees)
By algebraic expansion of prefix sums over a difference array:
$$\sum_{i=1}^{k} A[i] = \sum_{i=1}^{k} \sum_{j=1}^{i} D[j] = (k + 1) \sum_{i=1}^{k} D[i] - \sum_{i=1}^{k} (i \cdot D[i])$$
* Maintain two Fenwick trees: $B_1$ storing $D[i]$, and $B_2$ storing $i \cdot D[i]$.
* Both range updates and range queries run in $O(\log N)$ time.

### 3. Binary Lifting on Fenwick Tree ($O(\log N)$ Find $K$-th Element)
Find the smallest index $i$ such that $\text{prefix\_sum}(i) \ge K$ in $O(\log N)$ time (instead of $O(\log^2 N)$ binary search):
* Jump in descending powers of 2 from $\lfloor \log_2 N \rfloor$ down to $0$, accumulating intervals greedily without re-querying.

### 4. 2D Fenwick Tree (Matrix Range Queries)
* Nested bitwise loops support $O(\log N \cdot \log M)$ point updates and 2D submatrix sum queries over an $N \times M$ grid with zero pointer overhead.

---

## 6. Canonical Implementations

### Python (Clean & Generic)

```python
class FenwickTree:
    def __init__(self, n: int):
        self.n = n
        self.tree = [0] * (n + 1)

    def add(self, i: int, delta: int):
        """Adds delta to 1-based index i in O(log N)."""
        while i <= self.n:
            self.tree[i] += delta
            i += i & (-i)

    def query(self, i: int) -> int:
        """Returns prefix sum A[1..i] in O(log N)."""
        total = 0
        while i > 0:
            total += self.tree[i]
            i -= i & (-i)
        return total

    def range_query(self, l: int, r: int) -> int:
        """Returns range sum A[l..r] in O(log N)."""
        if l > r:
            return 0
        return self.query(r) - self.query(l - 1)
```

### Modern C++ (Fast & Cache-Aligned)

```cpp
#include <vector>
#include <cstddef>

template <typename T = long long>
class FenwickTree {
private:
    std::size_t n_;
    std::vector<T> tree_;

public:
    explicit FenwickTree(std::size_t n) : n_(n), tree_(n + 1, 0) {}

    // Linear-time O(N) constructor from initial values
    explicit FenwickTree(const std::vector<T>& arr) : n_(arr.size()), tree_(arr.size() + 1, 0) {
        for (std::size_t i = 1; i <= n_; ++i) {
            tree_[i] += arr[i - 1];
            std::size_t parent = i + (i & -i);
            if (parent <= n_) {
                tree_[parent] += tree_[i];
            }
        }
    }

    void add(std::size_t i, T delta) {
        for (; i <= n_; i += i & -i) {
            tree_[i] += delta;
        }
    }

    T query(std::size_t i) const {
        T sum = 0;
        for (; i > 0; i -= i & -i) {
            sum += tree_[i];
        }
        return sum;
    }

    T range_query(std::size_t l, std::size_t r) const {
        if (l > r) return 0;
        return query(r) - query(l - 1);
    }
};
```

---

## 7. When NOT to Use Fenwick Trees

1. **Non-Invertible Operations**: Fenwick trees rely on subtraction to compute range queries: $\text{sum}(l, r) = \text{query}(r) - \text{query}(l - 1)$. For operations like Range Maximum Query (RMQ) or GCD, there is no inverse operation ($\max(A, B) - B$ is meaningless). Use a **Segment Tree** or **Sparse Table** instead.
2. **Arbitrary Interval Updates**: Range assignments (e.g. set all $A[l..r] = V$) require lazy propagation, which is natural in **Segment Trees** but complex in Fenwick Trees.

---

## 8. Common Pitfalls & Edge Cases

1. **Zero-Indexing Infinite Loop**: Fenwick trees must be **1-indexed**. If you call `add(0, delta)` or `query(0)`, $0 \ \& \ (-0) = 0$, causing an infinite loop. Always offset 0-based inputs by $+1$.
2. **Bounds Overflow in Point Updates**: In C/C++, ensure the update loop terminates strictly at `i <= n_`.
3. **Integer Overflow on Sums**: Accumulating $10^5$ elements of size $10^9$ overflows 32-bit signed integers. Always use `int64_t` or `long long` for `tree_` and `sum`.

---

## 9. Curated Problem Mappings

* [LeetCode 307: Range Sum Query Mutable](https://leetcode.com/problems/range-sum-query-mutable/) *(Canonical point update, range sum)*
* [LeetCode 315: Count of Smaller Numbers After Self](https://leetcode.com/problems/count-of-smaller-numbers-after-self/) *(Coordinate compression + Fenwick inversion counting)*
* [CSES 1648: Dynamic Range Sum Queries](https://cses.fi/problemset/task/1648) *(Standard CP benchmark)*
* [CSES 1651: Range Update Queries](https://cses.fi/problemset/task/1651) *(Difference array Fenwick)*
* [Codeforces 383C: Propagating tree](https://codeforces.com/problemset/problem/383/C) *(Euler tour on trees + Fenwick Tree)*

---

## 10. References & Landmark Papers

* **Fenwick, P. M. (1994)**: *"A New Data Structure for Cumulative Frequency Tables"*. Software: Practice and Experience.
