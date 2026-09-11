---
title: "Longest Increasing Subsequence"
difficulty: "Intermediate"
domains: ["Dynamic Programming", "Algorithms", "Binary Search", "Optimization"]
prerequisites: ["1D and 2D Dynamic Programming Foundations", "Binary Search", "Arrays and Memory Layout"]
related_topics: ["Knapsack Family", "Edit Distance and Sequence Alignment", "Longest Common Subsequence", "Segment Trees", "Fenwick Trees"]
---

# Longest Increasing Subsequence

> [!NOTE]
> The **Longest Increasing Subsequence (LIS)** problem seeks the maximum length of a strictly increasing subsequence within an array $a[0 \dots n-1]$. While classical dynamic programming solves this in $O(n^2)$ time by defining $dp[i]$ as the LIS ending at index $i$, patience sorting combined with binary search accelerates computation to $O(n \log n)$ time by maintaining the minimum tail value for every discovered subsequence length.
>
> Reference implementations: [C++17 Implementation](../../implementations/cpp/longest_increasing_subsequence.cpp) | [Python Implementation](../../implementations/python/longest_increasing_subsequence.py)

> [!TIP]
> **The Patience Sorting Invariant ($O(n \log n)$ Optimization):**
> - The array `tails[k]` stores the **smallest possible ending element** of any valid increasing subsequence of length $k+1$.
> - Because a longer subsequence must end with an element strictly greater than the tail of a valid prefix of length $k$, `tails` is strictly monotonically increasing: $\text{tails}[0] < \text{tails}[1] < \dots < \text{tails}[L-1]$.
> - This invariant enables binary search (`std::lower_bound` in C++ / `bisect_left` in Python) in $O(\log n)$ per element.

> [!WARNING]
> **Crucial Conceptual Trap: The `tails` Array Is NOT the Subsequence:**
> The `tails` array stores compressed minimal boundaries for candidate prefixes, **not an actual valid subsequence**. For example, on input `[3, 5, 2]`, `tails` becomes `[2, 5]`. Notice `[2, 5]` is not a valid subsequence of `[3, 5, 2]` because 2 appears after 5! `tails` yields the exact *length*, but recovering the *sequence* requires parent index tracking.

The **Longest Increasing Subsequence**, usually abbreviated **LIS**, is one of the most important classical problems in dynamic programming.

It asks a simple question:

> Given a sequence, what is the longest subsequence whose values are strictly increasing?

This problem is important not only because it appears in practice, but because it teaches a deep algorithmic lesson:

- a natural $ O(n^2) $ dynamic programming solution exists
- a more advanced $ O(n \log n) $ optimization also exists
- careful reconstruction is needed if we want the actual subsequence, not only its length

LIS appears in many contexts:

- sequence analysis
- scheduling and ordering problems
- bioinformatics
- version comparison
- sorting-related reductions
- dynamic programming training

This chapter develops:

- the precise problem definition
- the classical $ O(n^2) $ DP solution
- parent-based reconstruction
- the patience-sorting intuition
- the $ O(n \log n) $ binary-search optimization
- full reconstruction for the optimized method
- common variants and pitfalls

---

## 1. What is a subsequence?

A **subsequence** is obtained by deleting zero or more elements from a sequence **without changing the relative order** of the remaining elements.

### Example

If the sequence is:

```text
[3, 1, 5, 2, 6, 4, 9]
```

then these are subsequences:

- `[3, 5, 6, 9]`
- `[1, 2, 4, 9]`
- `[3, 2, 4]`

But this is **not** a subsequence:

- `[5, 3, 9]`

because it changes the original order.

---

## 2. Increasing subsequence

An **increasing subsequence** is a subsequence whose values increase strictly:

$$
a_{i_1} < a_{i_2} < a_{i_3} < \cdots
$$

with:

$$
i_1 < i_2 < i_3 < \cdots
$$

### Important note
In the standard LIS problem, “increasing” usually means **strictly increasing**.

So equal values do **not** count as increasing.

Later we will briefly mention the non-decreasing variant.

---

## 3. Problem statement

Given an array $ a[0 \dots n-1] $, find:

- the **length** of the longest increasing subsequence
- and often the subsequence itself

### Example

For:

```text
[10, 9, 2, 5, 3, 7, 101, 18]
```

an LIS is:

```text
[2, 3, 7, 18]
```

so the answer length is:

$$
4
$$

Another valid LIS is:

```text
[2, 5, 7, 101]
```

So the LIS is not always unique.

---

## 4. Subsequence versus subarray

This is one of the most important distinctions.

### Subsequence
Elements keep their order, but do not need to be adjacent.

### Subarray
Elements must occupy a contiguous block.

For LIS, we are dealing with **subsequences**, not contiguous segments.

That is why the problem is richer than a simple sliding-window task.

---

## 5. Why LIS matters

LIS is a foundational topic because it teaches several important ideas at once:

- state definition in dynamic programming
- transition design
- parent reconstruction
- asymptotic improvement through binary search
- the difference between computing a value and recovering a structure

It is also a good example of a problem that begins as DP and then becomes something more refined.

---

## 6. First dynamic programming idea

A natural DP question is:

> What is the length of the longest increasing subsequence that **ends at position** $ i $?

This is the key local viewpoint.

Let:

$$
dp[i] = \text{length of the LIS ending exactly at index } i
$$

Then the overall answer is:

$$
\max_i dp[i]
$$

This is a very standard and elegant state definition.

---

## 7. The $ O(n^2) $ recurrence

If the subsequence ends at position $ i $, then the previous element must come from some earlier position $ j < i $ with:

$$
a[j] < a[i]
$$

So:

$$
dp[i] = 1 + \max \{ dp[j] \mid j < i \text{ and } a[j] < a[i] \}
$$

If no such $ j $ exists, then:

$$
dp[i] = 1
$$

because the subsequence containing only $ a[i] $ is valid.

---

## 8. Why this recurrence is correct

Any increasing subsequence ending at $ i $ must choose its previous element from an earlier smaller value.

Among all such choices, we want the one giving the largest possible subsequence length.

So the recurrence simply says:

- try every valid predecessor
- extend the best one by $ a[i] $

This is a classic example of **optimal substructure**.

---

## 9. Bottom-up computation order

Since $ dp[i] $ depends only on earlier positions $ j < i $, we compute the table left to right:

```text
dp[0], dp[1], dp[2], ...
```

For each $ i $, scan all earlier $ j $.

This gives:

- $ n $ choices of $ i $
- up to $ n $ earlier positions $ j $

So the total time is:

$$
O(n^2)
$$

---

## 10. C++17 reference implementation of LIS length in $ O(n^2) $

```cpp
#include <vector>
#include <algorithm>

int lis_length_n2(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    if (n == 0) return 0;

    std::vector<int> dp(n, 1);
    int best = 1;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            if (a[j] < a[i]) {
                dp[i] = std::max(dp[i], dp[j] + 1);
            }
        }
        best = std::max(best, dp[i]);
    }

    return best;
}
```

---

## 11. Python reference implementation of LIS length in $ O(n^2) $

```python
def lis_length_n2(a):
    n = len(a)
    if n == 0:
        return 0

    dp = [1] * n
    best = 1

    for i in range(n):
        for j in range(i):
            if a[j] < a[i]:
                dp[i] = max(dp[i], dp[j] + 1)
        best = max(best, dp[i])

    return best
```

---

## 12. Reconstructing the actual subsequence in $ O(n^2) $

To reconstruct the subsequence, we store a **parent** pointer.

Let:

- `parent[i]` = the previous index used before $ i $ in the best subsequence ending at $ i $

Whenever we improve `dp[i]` using `j`, we set:

```text
parent[i] = j
```

After filling the DP table:

1. find an index `end_idx` where `dp[end_idx]` is maximum
2. follow parent pointers backward
3. reverse the collected values

---

## 13. C++17 LIS reconstruction in $ O(n^2) $

```cpp
#include <vector>
#include <algorithm>

std::vector<int> lis_sequence_n2(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    if (n == 0) return {};

    std::vector<int> dp(n, 1), parent(n, -1);
    int best_len = 1;
    int best_end = 0;

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < i; ++j) {
            if (a[j] < a[i] && dp[j] + 1 > dp[i]) {
                dp[i] = dp[j] + 1;
                parent[i] = j;
            }
        }
        if (dp[i] > best_len) {
            best_len = dp[i];
            best_end = i;
        }
    }

    std::vector<int> seq;
    for (int cur = best_end; cur != -1; cur = parent[cur]) {
        seq.push_back(a[cur]);
    }
    std::reverse(seq.begin(), seq.end());
    return seq;
}
```

---

## 14. Python LIS reconstruction in $ O(n^2) $

```python
def lis_sequence_n2(a):
    n = len(a)
    if n == 0:
        return []

    dp = [1] * n
    parent = [-1] * n
    best_len = 1
    best_end = 0

    for i in range(n):
        for j in range(i):
            if a[j] < a[i] and dp[j] + 1 > dp[i]:
                dp[i] = dp[j] + 1
                parent[i] = j

        if dp[i] > best_len:
            best_len = dp[i]
            best_end = i

    seq = []
    cur = best_end
    while cur != -1:
        seq.append(a[cur])
        cur = parent[cur]

    seq.reverse()
    return seq
```

---

## 15. Worked example for the $ O(n^2) $ DP

Consider:

```text
[3, 1, 5, 2, 6, 4, 9]
```

We compute `dp[i]`:

- `dp[0] = 1` for `[3]`
- `dp[1] = 1` for `[1]`
- `dp[2] = 2` from `[3,5]` or `[1,5]`
- `dp[3] = 2` from `[1,2]`
- `dp[4] = 3` from `[3,5,6]` or `[1,5,6]`
- `dp[5] = 3` from `[1,2,4]`
- `dp[6] = 4` from `[1,2,4,9]` or `[3,5,6,9]`

So the LIS length is:

$$
4
$$

This example shows how each answer depends on earlier local answers.

---

## 16. Complexity of the classical DP

### Time
For each $ i $, we may inspect all earlier $ j < i $, so:

$$
O(n^2)
$$

### Space
We store:

- `dp`
- optionally `parent`

So space is:

$$
O(n)
$$

This is already quite efficient for moderate input sizes.

---

## 17. Why we want something faster

For very large $ n $, $ O(n^2) $ may be too slow.

So a natural question is:

> Can we do better than testing every earlier predecessor?

The answer is yes.

A remarkable optimization reduces the time to:

$$
O(n \log n)
$$

But the idea changes.

Instead of storing the full best subsequence ending at every position, we maintain a compact summary of possible subsequence endings.

---

## 18. Tails array intuition

The $ O(n \log n) $ method keeps an array often called `tails`.

### Meaning
`tails[len]` will represent the **smallest possible ending value** of an increasing subsequence of length `len + 1` found so far.

This is the key idea.

### Why smaller ending values are better
If two increasing subsequences have the same length, the one with the smaller final value is more flexible for future extension.

For example:

- ending with `4` is better than ending with `10`
if both subsequences have the same length, because more future numbers can extend `4`.

---

## 19. Example of tails behavior

Process:

```text
[3, 1, 5, 2, 6, 4, 9]
```

Step by step:

- `3` -> `tails = [3]`
- `1` -> replace first element -> `tails = [1]`
- `5` -> extend -> `tails = [1, 5]`
- `2` -> replace `5` -> `tails = [1, 2]`
- `6` -> extend -> `tails = [1, 2, 6]`
- `4` -> replace `6` -> `tails = [1, 2, 4]`
- `9` -> extend -> `tails = [1, 2, 4, 9]`

The final length of `tails` is the LIS length:

$$
4
$$

### Important warning
The `tails` array itself is **not always an actual LIS**.
It is a compressed summary that gives the correct length.

---

## 20. Why binary search appears

The `tails` array is always sorted.

Why?

Because the minimum possible tail for a longer subsequence must be strictly larger than the minimum possible tail for a shorter one.

So when processing a new value $ x $, we can use binary search to find:

- the first position where `tails[pos] >= x`

Then:

- replace `tails[pos]` with `x`, or
- append `x` if no such position exists

This gives:

$$
O(\log n)
$$

work per element, for total:

$$
O(n \log n)
$$

---

## 21. Why the $ O(n \log n) $ method works

The crucial invariant is:

> For each length $ L $, we keep the smallest possible tail value of any increasing subsequence of length $ L $.

This does not lose optimality, because a smaller tail is always at least as good for future extension as a larger tail of the same length.

So we are compressing many candidate subsequences into the best tail representative for each length.

That is the core proof intuition.

---

## 22. C++17 LIS length in $ O(n \log n) $

```cpp
#include <vector>
#include <algorithm>

int lis_length_nlogn(const std::vector<int>& a) {
    std::vector<int> tails;

    for (int x : a) {
        auto it = std::lower_bound(tails.begin(), tails.end(), x);
        if (it == tails.end()) {
            tails.push_back(x);
        } else {
            *it = x;
        }
    }

    return static_cast<int>(tails.size());
}
```

---

## 23. Python LIS length in $ O(n \log n) $

```python
from bisect import bisect_left

def lis_length_nlogn(a):
    tails = []

    for x in a:
        pos = bisect_left(tails, x)
        if pos == len(tails):
            tails.append(x)
        else:
            tails[pos] = x

    return len(tails)
```

---

## 24. Strictly increasing versus non-decreasing

The binary search detail matters.

### Strictly increasing LIS
Use the first position where:

```text
tails[pos] >= x
```

That is `lower_bound` in C++ and `bisect_left` in Python.

### Non-decreasing subsequence
Use the first position where:

```text
tails[pos] > x
```

That is `upper_bound` in C++ and `bisect_right` in Python.

This small change controls whether equal values are allowed.

---

## 25. Why reconstruction is harder in $ O(n \log n) $

The optimized method gives the length easily, but not the actual sequence directly.

This is because `tails` stores summary information, not complete subsequences.

To reconstruct an LIS, we need extra arrays:

- `parent[i]` = previous index in the subsequence ending at `i`
- `tail_index[len]` = the array index of the last element of the best subsequence of length `len + 1`

Then when we place element `a[i]` at some position `pos`:

- its parent becomes the index stored for `pos - 1`
- it becomes the new representative for length `pos + 1`

After processing all elements, backtrack from the index for the longest length.

---

## 26. C++17 LIS reconstruction in $ O(n \log n) $

```cpp
#include <vector>
#include <algorithm>

std::vector<int> lis_sequence_nlogn(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    if (n == 0) return {};

    std::vector<int> tail_values;
    std::vector<int> tail_index;
    std::vector<int> parent(n, -1);

    for (int i = 0; i < n; ++i) {
        int x = a[i];
        int pos = static_cast<int>(
            std::lower_bound(tail_values.begin(), tail_values.end(), x) - tail_values.begin()
        );

        if (pos == static_cast<int>(tail_values.size())) {
            tail_values.push_back(x);
            tail_index.push_back(i);
        } else {
            tail_values[pos] = x;
            tail_index[pos] = i;
        }

        if (pos > 0) {
            parent[i] = tail_index[pos - 1];
        }
    }

    std::vector<int> seq;
    int cur = tail_index.back();
    while (cur != -1) {
        seq.push_back(a[cur]);
        cur = parent[cur];
    }
    std::reverse(seq.begin(), seq.end());
    return seq;
}
```

---

## 27. Python LIS reconstruction in $ O(n \log n) $

```python
from bisect import bisect_left

def lis_sequence_nlogn(a):
    n = len(a)
    if n == 0:
        return []

    tail_values = []
    tail_index = []
    parent = [-1] * n

    for i, x in enumerate(a):
        pos = bisect_left(tail_values, x)

        if pos == len(tail_values):
            tail_values.append(x)
            tail_index.append(i)
        else:
            tail_values[pos] = x
            tail_index[pos] = i

        if pos > 0:
            parent[i] = tail_index[pos - 1]

    seq = []
    cur = tail_index[-1]
    while cur != -1:
        seq.append(a[cur])
        cur = parent[cur]

    seq.reverse()
    return seq
```

---

## 28. Worked reconstruction intuition

Suppose we process:

```text
[10, 9, 2, 5, 3, 7, 101, 18]
```

The algorithm updates representatives of subsequence lengths.

For example:

- `2` becomes a good tail for length 1
- `3` becomes a better tail than `5` for length 2
- `7` becomes a tail for length 3
- `18` may replace `101` as a better tail for length 4

Even though `18` replaces `101`, the longest length remains 4.

Parent pointers remember **which earlier index** led to each extension, so the actual subsequence can still be recovered.

This is why summary values plus parent tracking are enough.

---

## 29. Patience sorting intuition

The $ O(n \log n) $ LIS method is often explained using **patience sorting**.

### Analogy
Imagine placing numbers on piles.

For each new number:
- place it on the leftmost pile whose top is at least that number
- if no such pile exists, start a new pile

The number of piles at the end equals the LIS length.

### Why this helps
This pile-top process is another way to visualize the `tails` array.

You do not need the full card-game interpretation to use the algorithm, but the picture is helpful.

---

## 30. Why the optimized method is not "just greedy"

The $ O(n \log n) $ algorithm may look greedy because it makes local replacements.

But conceptually it is better understood as:

- maintaining optimal representatives for subsequence lengths
- using binary search to update them efficiently

It is not a simple greedy proof problem in the same way as interval scheduling or MST.

This is one reason LIS is such a rich topic: it sits at the boundary between DP and structural optimization.

---

## 31. Comparing the two main LIS methods

### $ O(n^2) $ DP
- easier to derive
- clearer as a first solution
- directly shows the state transition

### $ O(n \log n) $ method
- faster
- more subtle
- requires the tails invariant
- reconstruction is trickier

In teaching, it is usually best to learn the quadratic DP first.


| Method | Time Complexity | Space Complexity | Primary Role & Trade-offs |
|---|---:|---:|---|
| **Classical DP (Ending Index)** | $O(n^2)$ | $O(n)$ | First-principles clarity; simple parent pointers; easily extensible to arbitrary DAG state transitions |
| **Patience Sorting / Binary Search** | $O(n \log n)$ | $O(n)$ | Asymptotically optimal for large $n$; requires tail predecessor index tracking for reconstruction |


---

## 32. Common variants

### A. Longest non-decreasing subsequence
Allow equal values, so use `<=` in the recurrence and `upper_bound` style logic in the optimized method.

### B. Count of LIS
Instead of only length, count how many longest increasing subsequences exist.

### C. LIS in higher dimensions
Sometimes values are pairs or tuples, and sorting plus LIS becomes a powerful reduction.

### D. Minimum deletions to sort
The minimum number of deletions needed to make a sequence strictly increasing is:

$$
n - \text{LIS length}
$$

These variants make LIS widely useful.

---

## 33. Common mistakes

### Mistake 1: confusing subsequence with subarray
LIS allows skipped elements.

### Mistake 2: using the wrong comparison
Strictly increasing needs `<`, not `<=`.

### Mistake 3: assuming `tails` is always the final answer sequence
It usually is not.

### Mistake 4: reconstructing from values instead of indices
Use parent pointers and indices, not only values.

### Mistake 5: using `upper_bound` when `lower_bound` is needed
For standard strictly increasing LIS, use `lower_bound` or `bisect_left`.

### Mistake 6: forgetting the empty-array case
The LIS length of an empty sequence is 0.

---

## 34. Complexity summary

### Classical DP
- **Time:** $ O(n^2) $
- **Space:** $ O(n) $

### Optimized method
- **Time:** $ O(n \log n) $
- **Space:** $ O(n) $

The optimized method is asymptotically better, but the classical DP is often easier to understand and sometimes fully sufficient for moderate input sizes.

---

## 35. Proof intuition summary

### For the $ O(n^2) $ DP
The recurrence is correct because every increasing subsequence ending at $ i $ must extend some earlier smaller value.

### For the $ O(n \log n) $ method
The algorithm is correct because for each possible length, keeping the smallest possible tail preserves maximum flexibility for future extensions.

This is the central invariant.

---

## 36. Applications

### A. Sequence analysis
LIS measures ordered growth patterns in data.

### B. Scheduling reductions
Some ordering and compatibility problems reduce to LIS after sorting.

### C. Version and ranking analysis
LIS can measure consistency between orderings.

### D. Computational geometry and multidimensional reductions
Problems on pairs or envelopes often reduce to sorting plus LIS.

---

## 37. Summary

The Longest Increasing Subsequence problem asks for the longest subsequence of a sequence whose values are strictly increasing.

The classical solution uses dynamic programming:

$$
dp[i] = \text{LIS length ending at } i
$$

This gives an:

$$
O(n^2)
$$

algorithm and supports straightforward reconstruction with parent pointers.

A more advanced method maintains the smallest possible tail for each subsequence length and uses binary search, giving:

$$
O(n \log n)
$$

time.

LIS is an important flagship problem because it teaches:

- state definition
- transition design
- reconstruction
- asymptotic optimization
- the difference between a direct DP and a more compressed structural method

---

## 38. Practice prompts

1. What is the difference between a subsequence and a subarray?
2. Why is
   $$
   dp[i] = 1 + \max(dp[j])
   $$
   over valid earlier $ j $ the right recurrence?
3. Why does the classical LIS DP take $ O(n^2) $ time?
4. What does the `tails` array represent?
5. Why does replacing a larger tail with a smaller one preserve future possibilities?
6. Why is reconstruction harder in the $ O(n \log n) $ method?
7. What changes if we want a non-decreasing subsequence instead?

---

## 39. Suggested next topics

A natural continuation after LIS is:

- knapsack family
- edit distance and sequence alignment
- longest common subsequence
- interval DP
- DP optimization techniques