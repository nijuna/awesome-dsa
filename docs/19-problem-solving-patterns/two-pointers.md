---
title: "Two Pointers: Converging, Partitioning & Fast-Slow Pointers"
difficulty: "Beginner to Intermediate"
domains: ["Theory", "Interview", "Competitive Programming", "Systems"]
prerequisites: ["Arrays and Strings", "Basic Complexity Analysis"]
related_topics: ["Sliding Window", "Binary Search", "Linked Lists", "External Sorting"]
---

# Two Pointers: Converging, Partitioning & Fast-Slow Pointers

> [!NOTE]
> The Two Pointers pattern coordinates two indices across one or more sequences to eliminate redundant search space in $O(N)$ time instead of checking all $O(N^2)$ pairs.

---

## 1. Why This Matters

Many search and partitioning problems on linear data structures require finding a pair of elements, partitioning an array around a pivot, or detecting a cycle:
* Finding two numbers in a sorted array that sum to a target.
* Detecting whether a singly linked list contains a loop without allocating an auxiliary hash set.
* Merging two sorted runs during external merge sort or database index scans.

A brute-force solution checks every combination of indices $(i, j)$ where $0 \le i < j < N$, costing **$O(N^2)$ time**. 

Two Pointers reduces this to **$O(N)$** by using the structural properties of the data (such as monotonicity or sorted order) to prove that moving a pointer **safely discards an entire row or column of potential candidate pairs** without evaluating them individually.

---

## 2. The 4 Fundamental Archetypes

Unlike the Sliding Window (which strictly maintains a contiguous subsegment with internal state), the Two Pointers family encompasses **four distinct movement strategies**:

```text
1. OPPOSITE ENDS (Converging):
   [ L ───►                      ◄─── R ]
   Pointers start at outer boundaries and converge inwards. Used on sorted sequences.

2. FAST & SLOW (Tortoise & Hare):
   [ S ──►      F ────►                 ]
   Pointers move in the same direction at different speeds (1x vs 2x). Used for cycle detection.

3. SAME DIRECTION (Reader & Writer / Partitioning):
   [ W ──►      R ────►                 ]
   One pointer reads ahead while the other writes in-place. Used for deduplication and partitioning.

4. TWO SEQUENCES (Merging / Coordination):
   Array A: [ Ptr_A ──►                 ]
   Array B: [ Ptr_B ──►                 ]
   Pointers advance independently across two distinct collections based on comparison.
```

---

## 3. Formal Invariants & Correctness

### The Monotone Elimination Argument (Opposite Ends)
Why is the converging two-pointer technique on a sorted array $A$ provably correct?

Let $A[0 \dots N-1]$ be sorted in ascending order, searching for $A[i] + A[j] == \text{target}$.
Initialize $l = 0, r = N - 1$.

* If $A[l] + A[r] > \text{target}$:
  * Because $A$ is sorted, for any $k > l$, $A[k] + A[r] \ge A[l] + A[r] > \text{target}$.
  * Therefore, index $r$ **cannot pair with ANY index in $[l, r]$** to form the target sum.
  * **Invariant**: Discarding $r \leftarrow r - 1$ eliminates $r$ from the candidate set with mathematical certainty.
* If $A[l] + A[r] < \text{target}$:
  * By symmetric logic, index $l$ cannot pair with any index in $[l, r]$.
  * Discarding $l \leftarrow l + 1$ eliminates $l$ with certainty.

Each comparison eliminates either one row or one column of the $N \times N$ pair matrix, guaranteeing termination in $\le N$ steps:

```text
Pair Matrix (Sorted Arrays):
      r=0   r=1   r=2   r=3   r=4
l=0 [  .     .     .     .    TOO LARGE -> Discard column r=4 ]
l=1 [  .     .     .     .     x ]
l=2 [  .     .     .     .     x ]
```

---

## 4. Key Operations & Complexity

| Archetype | Time Complexity | Space Complexity | Primary Prerequisite |
| :--- | :--- | :--- | :--- |
| **Opposite Ends (Converging)** | $O(N)$ (or $O(N \log N)$ if sort needed) | $O(1)$ | Sequence must be sorted or exhibit monotonicity. |
| **Fast & Slow (Floyd's Cycle)** | $O(N)$ | $O(1)$ | Functional graph or linked list pointer traversal. |
| **Reader / Writer (In-place)** | $O(N)$ | $O(1)$ | In-place array mutation allowed. |
| **Two Sequences (Merge)** | $O(N + M)$ | $O(1)$ auxiliary | Both input sequences must be sorted. |

---

## 5. Hardware & Cache Reality

* **Contiguous Cache Streaming**:
  * In the **Reader/Writer** and **Two Sequences** archetypes, both pointers advance forward sequentially. The CPU hardware prefetcher detects linear strides and continuously preloads downstream cache lines, resulting in near-zero memory stalls.
* **Dual-Direction Streaming (Converging)**:
  * Two pointers moving inward stream from two cache lines (one at the front, one at the back). Modern CPUs possess multiple independent prefetch units per core, easily sustaining two concurrent streams without cache thrashing.
* **Pointer Chasing in Linked Lists (Tortoise & Hare)**:
  * In linked lists, fast-slow pointers incur pointer-chasing latency. The fast pointer (`f = f->next->next`) jumps two heap nodes, causing frequent L1 cache misses. However, space complexity remains strictly $O(1)$ compared to storing node addresses in an auxiliary `std::unordered_set` ($O(N)$ memory).

---

## 6. Canonical Implementations

### Template 1: Opposite Ends (Two-Sum in Sorted Array)

#### Python
```python
def two_sum_sorted(nums: list[int], target: int) -> list[int]:
    left, right = 0, len(nums) - 1

    while left < right:
        current_sum = nums[left] + nums[right]
        if current_sum == target:
            return [left, right]
        elif current_sum < target:
            left += 1
        else:
            right -= 1

    return []
```

#### Modern C++
```cpp
#include <vector>
#include <optional>

std::optional<std::pair<int, int>> two_sum_sorted(const std::vector<int>& nums, int target) {
    int left = 0;
    int right = static_cast<int>(nums.size()) - 1;

    while (left < right) {
        int current_sum = nums[left] + nums[right];
        if (current_sum == target) {
            return std::make_pair(left, right);
        } else if (current_sum < target) {
            ++left;
        } else {
            --right;
        }
    }
    return std::nullopt;
}
```

---

### Template 2: Fast & Slow Pointers (Floyd's Cycle-Finding)

#### Python
```python
class ListNode:
    def __init__(self, val=0, next=None):
        self.val = val
        self.next = next

def has_cycle(head: ListNode | None) -> bool:
    slow = head
    fast = head

    while fast and fast.next:
        slow = slow.next
        fast = fast.next.next
        if slow == fast:
            return True

    return False
```

#### Modern C++
```cpp
struct ListNode {
    int val;
    ListNode* next;
    ListNode(int x) : val(x), next(nullptr) {}
};

bool has_cycle(ListNode* head) {
    ListNode* slow = head;
    ListNode* fast = head;

    while (fast != nullptr && fast->next != nullptr) {
        slow = slow->next;
        fast = fast->next->next;
        if (slow == fast) return true;
    }
    return false;
}
```

---

### Template 3: Reader & Writer (In-Place Array Deduplication)

```cpp
#include <vector>

int remove_duplicates_sorted(std::vector<int>& nums) {
    if (nums.empty()) return 0;

    int write = 1;
    for (int read = 1; read < static_cast<int>(nums.size()); ++read) {
        if (nums[read] != nums[read - 1]) {
            nums[write] = nums[read];
            ++write;
        }
    }
    return write; // New length of deduplicated prefix
}
```

---

## 7. Real-World Systems Case Studies

### 1. Database Storage Engines: Multi-Way Merge & Compaction
* In **LSM-Trees (e.g. RocksDB, Cassandra)**, immutable disk files (SSTables) contain pre-sorted key-value pairs.
* During **background compaction**, the engine merges multiple sorted SSTables into a new sorted level using coordinated pointer iterators, identical to the Two Sequences merging pattern.

### 2. External Merge Sort
* When datasets exceed physical RAM (e.g. sorting a $500\text{ GB}$ file with $16\text{ GB}$ of RAM), the engine sorts in-memory chunks, writes them to disk runs, and merges them using sequential stream pointers, maximizing disk sequential read throughput.

### 3. Memory Compaction in Garbage Collectors
* The **Mark-Compact garbage collection algorithm** uses the Two Pointers Reader/Writer pattern (e.g. Edwards' two-finger compaction) to slide live memory objects to one side of the heap, eliminating fragmentation without extra memory allocations.

---

## 8. Common Pitfalls & Edge Cases

1. **Infinite Loops in Converging Pointers**: Forgetting to increment `left` or decrement `right` inside conditional branches causes frozen loops.
2. **Strict vs Non-Strict Inequality**: Using `while (left <= right)` instead of `while (left < right)` when looking for distinct pairs allows a single element at `left == right` to pair with itself.
3. **Handling Duplicate Pairs**: In problems like 3Sum, after finding a valid pair, both pointers must skip over identical adjacent values (`while (nums[l] == nums[l+1]) l++;`) to avoid generating duplicate output tuples.
4. **Fast Pointer Null Dereference**: In cycle detection, always verify `while (fast && fast->next)` before accessing `fast->next->next`.

---

## 9. Curated Problem Sets

### [LeetCode 11: Container With Most Water](https://leetcode.com/problems/container-with-most-water/)
- **Difficulty**: Medium
- **Pattern**: Converging pointers
- **Why this matters**: Proves greedy elimination: always advance the pointer with the shorter height because advancing the taller pointer can never yield a larger area.

### [LeetCode 15: 3Sum](https://leetcode.com/problems/3sum/)
- **Difficulty**: Medium
- **Pattern**: Sorting + Converging pointers
- **Why this matters**: The canonical extension of 2Sum to $O(N^2)$ via an outer loop and inner two-pointer search.

### [LeetCode 142: Linked List Cycle II](https://leetcode.com/problems/linked-list-cycle-ii/)
- **Difficulty**: Medium
- **Pattern**: Floyd's Cycle Detection + Mathematical Phase Alignment
- **Why this matters**: Shows the classic mathematical proof: after slow and fast meet, resetting one pointer to `head` and advancing both at 1x speed causes them to meet exactly at the cycle entrance.

### [LeetCode 26: Remove Duplicates from Sorted Array](https://leetcode.com/problems/remove-duplicates-from-sorted-array/)
- **Difficulty**: Easy
- **Pattern**: Reader & Writer in-place compaction
- **Why this matters**: Foundational for memory compaction and in-place array algorithms.

### [LeetCode 88: Merge Sorted Array](https://leetcode.com/problems/merge-sorted-array/)
- **Difficulty**: Easy
- **Pattern**: Backward converging two-pointer merge
- **Why this matters**: Teaches merging from the *end* of the destination array to avoid overwriting elements in-place.

---

## 10. Related Topics

* [Sliding Window](sliding-window.md)
* [Dynamic Arrays & Memory Layout](../04-linear-data-structures/dynamic-arrays-and-strings.md)
* [Linked Lists](../04-linear-data-structures/linked-lists.md)
* [Binary Search on Answer](binary-search-on-answer.md)
