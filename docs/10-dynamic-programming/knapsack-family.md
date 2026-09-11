---
title: "The Knapsack Problem Family"
difficulty: "Intermediate"
domains: ["Dynamic Programming", "Combinatorial Optimization", "Algorithms"]
prerequisites: ["1D and 2D Dynamic Programming Foundations", "Recursion", "Asymptotic Analysis"]
related_topics: ["Longest Increasing Subsequence", "Edit Distance and Sequence Alignment", "Coin Change", "Subset Sum"]
---

# Knapsack Family

> [!NOTE]
> The **Knapsack Problem Family** comprises core combinatorial optimization benchmarks that allocate bounded capacity $W$ across weighted, valued items. Across variants—**0-1 Knapsack** (at most once), **Unbounded Knapsack** (unlimited copies), and **Bounded Knapsack** (at most $m_i$ copies)—the governing dynamic programming recurrence remains invariant while the state evaluation order changes to enforce multiplicity constraints.
>
> Reference implementations: [C++17 Implementation](../../implementations/cpp/knapsack_family.cpp) | [Python Implementation](../../implementations/python/knapsack_family.py)

> [!TIP]
> **The Loop-Direction Symmetry Rule (1D Rolling Arrays):**
> - **0-1 Knapsack ($W \to w_i$ backwards):** Enforces single-use. When evaluating $dp[c] = \max(dp[c], dp[c-w_i] + v_i)$, $dp[c-w_i]$ has not yet been touched in the current outer loop, preserving values strictly from items $0 \dots i-1$.
> - **Unbounded Knapsack ($w_i \to W$ forwards):** Permits infinite reuse. When evaluating $dp[c]$, $dp[c-w_i]$ may already reflect inclusion of item $i$, naturally compounding copies.
> - **Bounded Knapsack (Binary Power Splitting):** Decomposes count $m_i$ into $O(\log m_i)$ synthetic bundles $\{1, 2, 4, \dots, R\}$, reducing the bounded problem to standard 0-1 Knapsack.

> [!WARNING]
> **Exact-Fill vs. At-Most-Capacity Initialization:**
> - **At-Most-Capacity ($\sum w_i \le W$):** Initialize $dp[0 \dots W] = 0$. Unused capacity incurs zero penalty.
> - **Exact-Fill ($\sum w_i = W$):** Initialize $dp[0] = 0$ and $dp[1 \dots W] = -\infty$. If $dp[c-w] = -\infty$, state $c$ cannot be formed. Returning $\max_{c} dp[c]$ vs. $dp[W]$ distinguishes capacity upper-bounds from rigid knapsack fill.

The **knapsack family** is one of the most important groups of problems in dynamic programming.

The central theme is simple:

> We have items with values and weights, and we want to choose items under a capacity constraint.

But small changes in the rules create very different problems.

Some versions allow:

- each item at most once
- unlimited copies of each item
- only a bounded number of copies

These variations lead to different dynamic programming transitions and iteration orders.

Knapsack is important because it teaches several core ideas at once:

- state definition
- transition design
- feasibility versus optimization
- rolling-array space optimization
- item reconstruction
- why greedy methods can fail
- pseudo-polynomial complexity

This chapter develops the three major variants:

- **0-1 Knapsack**
- **Unbounded Knapsack**
- **Bounded Knapsack**, including **binary power splitting**

We also discuss:

- space optimization
- recovering which items were chosen
- common pitfalls
- the relationship to subset sum and coin change

---

## 1. The basic knapsack model

We are given items, where each item has:

- a **weight**
- a **value**

We are also given a knapsack capacity $ W $.

The goal is to choose items so that:

- total weight does not exceed $ W $
- total value is as large as possible

This is an optimization problem.

---

## 2. Why knapsack matters

Knapsack is more than a single problem.

It is a whole family of modeling patterns used in:

- resource allocation
- budgeting
- scheduling with limits
- subset selection
- partition problems
- combinatorial optimization

It is also one of the clearest examples where:

- a natural greedy strategy may fail
- but dynamic programming succeeds

That makes it a perfect DP flagship topic.

---

## 3. 0-1 Knapsack problem statement

In **0-1 Knapsack**, each item may be taken:

- **0 times**
- or **1 time**

There are no fractional choices and no repeated use.

### Input
For each item $ i $:

- weight $ w_i $
- value $ v_i $

### Goal
Maximize total value subject to:

$$
\sum w_i \le W
$$

where each item is chosen at most once.

---

## 4. Why greedy fails for 0-1 Knapsack

A common first idea is:

> Choose items in descending value-to-weight ratio.

This works for **fractional knapsack**, where items can be split.

But it fails for **0-1 Knapsack**, because taking a locally attractive item can block a better combination of whole items.

### Example intuition
A high ratio item may use enough capacity to prevent taking two slightly lower-ratio items whose combined value is better.

So 0-1 Knapsack is a standard example where:

- greedy looks tempting
- but dynamic programming is needed

---

## 5. State design for 0-1 Knapsack

A classical DP state is:

$$
dp[i][c] = \text{maximum value using the first } i \text{ items with capacity } c
$$

This is a 2D DP.

### Meaning
At state $ (i, c) $, we ask:

> If I only consider the first $ i $ items, and my capacity limit is $ c $, what is the best value I can achieve?

This is a very standard and powerful state definition.

---

## 6. Transition for 0-1 Knapsack

For item $ i $, there are two possibilities:

- do not take it
- take it, if capacity allows

If we number items from $ 0 $ to $ n-1 $, then the transition becomes:

$$
dp[i][c] =
\begin{cases}
dp[i-1][c] & \text{if item } i-1 \text{ is not taken} \\
\max(dp[i-1][c],\; dp[i-1][c-w_{i-1}] + v_{i-1}) & \text{if } w_{i-1} \le c
\end{cases}
$$

### Interpretation
At each item, we decide:

- skip it
- or include it and use the best solution for the remaining capacity among earlier items

---

## 7. Base cases for 0-1 Knapsack

If no items are available, the best value is 0:

$$
dp[0][c] = 0
$$

for all capacities $ c $.

Also, with capacity 0, the best value is 0:

$$
dp[i][0] = 0
$$

These are the natural base cases.

---

## 8. C++17 reference implementation of 0-1 Knapsack, 2D DP

```cpp
#include <vector>
#include <algorithm>

int knapsack_01_2d(const std::vector<int>& weight, const std::vector<int>& value, int W) {
    int n = static_cast<int>(weight.size());
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(W + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int c = 0; c <= W; ++c) {
            dp[i][c] = dp[i - 1][c];
            if (weight[i - 1] <= c) {
                dp[i][c] = std::max(dp[i][c],
                                    dp[i - 1][c - weight[i - 1]] + value[i - 1]);
            }
        }
    }

    return dp[n][W];
}
```

---

## 9. Python reference implementation of 0-1 Knapsack, 2D DP

```python
def knapsack_01_2d(weight, value, W):
    n = len(weight)
    dp = [[0] * (W + 1) for _ in range(n + 1)]

    for i in range(1, n + 1):
        for c in range(W + 1):
            dp[i][c] = dp[i - 1][c]
            if weight[i - 1] <= c:
                dp[i][c] = max(dp[i][c],
                               dp[i - 1][c - weight[i - 1]] + value[i - 1])

    return dp[n][W]
```

---

## 10. Complexity of 0-1 Knapsack

If there are $ n $ items and capacity $ W $, then:

### Time
$$
O(nW)
$$

### Space
$$
O(nW)
$$

This is polynomial in $ n $ and $ W $, but not polynomial in the number of bits needed to represent $ W $.

So this is called **pseudo-polynomial time**.

---

## 11. What pseudo-polynomial means

This is an important concept.

The running time:

$$
O(nW)
$$

depends on the **numerical value** of $ W $, not only on the input length in bits.

If $ W $ is very large, this can be expensive.

So 0-1 Knapsack is not considered efficiently solvable in the usual strong polynomial sense.

This is one reason the problem is central in complexity discussions.

---

## 12. Space optimization for 0-1 Knapsack

In the recurrence, row $ i $ depends only on row $ i-1 $.

So we can compress the 2D table into a 1D array:

$$
dp[c] = \text{best value for capacity } c
$$

### Important detail
For 0-1 Knapsack, capacities must be iterated **backward**:

```text
for c from W down to weight[i]
```

This prevents using the same item more than once in the same iteration.

That detail is essential.

---

## 13. C++17 0-1 Knapsack, 1D rolling array

```cpp
#include <vector>
#include <algorithm>

int knapsack_01_1d(const std::vector<int>& weight, const std::vector<int>& value, int W) {
    int n = static_cast<int>(weight.size());
    std::vector<int> dp(W + 1, 0);

    for (int i = 0; i < n; ++i) {
        for (int c = W; c >= weight[i]; --c) {
            dp[c] = std::max(dp[c], dp[c - weight[i]] + value[i]);
        }
    }

    return dp[W];
}
```

---

## 14. Python 0-1 Knapsack, 1D rolling array

```python
def knapsack_01_1d(weight, value, W):
    dp = [0] * (W + 1)

    for w, v in zip(weight, value):
        for c in range(W, w - 1, -1):
            dp[c] = max(dp[c], dp[c - w] + v)

    return dp[W]
```

---

## 15. Why backward iteration is necessary in 0-1 Knapsack

Suppose we process one item and iterate capacities upward.

Then when computing `dp[c]`, the state `dp[c - w]` may already include the same item from this same round.

That would incorrectly allow multiple copies.

Backward iteration avoids this by ensuring each item contributes at most once per round.

This is one of the most important implementation details in the chapter.

---

## 16. Item reconstruction for 0-1 Knapsack

Often we want not only the best value, but also the chosen items.

The easiest reconstruction method uses the full 2D table.

### Idea
Start from `dp[n][W]` and walk backward.

For item $ i $:

- if `dp[i][c] == dp[i-1][c]`, then item $ i-1 $ was not taken
- otherwise, it was taken, and we move to:
  - `i - 1`
  - capacity `c - weight[i-1]`

This gives the chosen item set.

---

## 17. C++17 0-1 Knapsack reconstruction

```cpp
#include <vector>
#include <algorithm>

std::pair<int, std::vector<int>> knapsack_01_reconstruct(
    const std::vector<int>& weight,
    const std::vector<int>& value,
    int W) {

    int n = static_cast<int>(weight.size());
    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(W + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int c = 0; c <= W; ++c) {
            dp[i][c] = dp[i - 1][c];
            if (weight[i - 1] <= c) {
                dp[i][c] = std::max(dp[i][c],
                                    dp[i - 1][c - weight[i - 1]] + value[i - 1]);
            }
        }
    }

    std::vector<int> chosen;
    int c = W;

    for (int i = n; i >= 1; --i) {
        if (dp[i][c] != dp[i - 1][c]) {
            chosen.push_back(i - 1);
            c -= weight[i - 1];
        }
    }

    std::reverse(chosen.begin(), chosen.end());
    return {dp[n][W], chosen};
}
```

---

## 18. Python 0-1 Knapsack reconstruction

```python
def knapsack_01_reconstruct(weight, value, W):
    n = len(weight)
    dp = [[0] * (W + 1) for _ in range(n + 1)]

    for i in range(1, n + 1):
        for c in range(W + 1):
            dp[i][c] = dp[i - 1][c]
            if weight[i - 1] <= c:
                dp[i][c] = max(dp[i][c],
                               dp[i - 1][c - weight[i - 1]] + value[i - 1])

    chosen = []
    c = W
    for i in range(n, 0, -1):
        if dp[i][c] != dp[i - 1][c]:
            chosen.append(i - 1)
            c -= weight[i - 1]

    chosen.reverse()
    return dp[n][W], chosen
```

---

## 19. Unbounded Knapsack problem statement

In **Unbounded Knapsack**, each item may be taken any number of times.

This changes the recurrence and the iteration order.

### Goal
Maximize total value subject to capacity $ W $, but now we may reuse items.

This version models situations like:

- unlimited supply of item types
- repeated use of actions or resources
- rod-cutting style optimization

---

## 20. State design for Unbounded Knapsack

A common 1D state is again:

$$
dp[c] = \text{maximum value achievable with capacity } c
$$

The difference is in the transition.

Since items can be reused, if we take item $ i $, then we may still take it again later.

So:

$$
dp[c] = \max(dp[c],\; dp[c - w_i] + v_i)
$$

and now the order of iteration must allow repeated use.

---

## 21. Forward iteration for Unbounded Knapsack

For Unbounded Knapsack, capacities are iterated **forward**:

```text
for c from weight[i] up to W
```

Why?

Because when computing `dp[c]`, we want `dp[c - weight[i]]` to be allowed to already include the current item.

That is exactly what unbounded use means.

This is the mirror image of the 0-1 case.

---

## 22. C++17 Unbounded Knapsack

```cpp
#include <vector>
#include <algorithm>

int knapsack_unbounded(const std::vector<int>& weight, const std::vector<int>& value, int W) {
    std::vector<int> dp(W + 1, 0);

    for (int i = 0; i < static_cast<int>(weight.size()); ++i) {
        for (int c = weight[i]; c <= W; ++c) {
            dp[c] = std::max(dp[c], dp[c - weight[i]] + value[i]);
        }
    }

    return dp[W];
}
```

---

## 23. Python Unbounded Knapsack

```python
def knapsack_unbounded(weight, value, W):
    dp = [0] * (W + 1)

    for w, v in zip(weight, value):
        for c in range(w, W + 1):
            dp[c] = max(dp[c], dp[c - w] + v)

    return dp[W]
```

---

## 24. Why 0-1 and unbounded differ only by loop direction

This is a beautiful and important DP lesson.

The recurrence forms look similar, but the meaning changes because of the iteration order.

### 0-1 Knapsack
Backward capacity loop:
- prevents reusing the same item

### Unbounded Knapsack
Forward capacity loop:
- allows reusing the same item

A tiny implementation detail completely changes the problem being solved.


| Variant | 1D Capacity Order | Recurrence Invariant | Why It Works |
|---|---|---|---|
| **0-1 Knapsack** | High to low ($W \to w_i$) | Uses previous-row values $dp[c - w_i]$ | Prevents re-using item $i$ within the same round |
| **Unbounded Knapsack** | Low to high ($w_i \to W$) | Uses current-row updated values $dp[c - w_i]$ | Actively allows compounding re-use within the same round |


---

## 25. Reconstructing items in Unbounded Knapsack

Reconstruction can be done by storing a choice array.

For each capacity $ c $, record which item last improved `dp[c]`.

Then while capacity remains positive:

- take that item
- subtract its weight
- continue

This gives one optimal multiset of items.

---

## 26. C++17 Unbounded Knapsack reconstruction

```cpp
#include <vector>
#include <algorithm>

std::pair<int, std::vector<int>> knapsack_unbounded_reconstruct(
    const std::vector<int>& weight,
    const std::vector<int>& value,
    int W) {

    std::vector<int> dp(W + 1, 0);
    std::vector<int> choice(W + 1, -1);

    for (int i = 0; i < static_cast<int>(weight.size()); ++i) {
        for (int c = weight[i]; c <= W; ++c) {
            int candidate = dp[c - weight[i]] + value[i];
            if (candidate > dp[c]) {
                dp[c] = candidate;
                choice[c] = i;
            }
        }
    }

    std::vector<int> chosen;
    int c = W;
    while (c > 0 && choice[c] != -1) {
        int i = choice[c];
        chosen.push_back(i);
        c -= weight[i];
    }

    std::reverse(chosen.begin(), chosen.end());
    return {dp[W], chosen};
}
```

---

## 27. Python Unbounded Knapsack reconstruction

```python
def knapsack_unbounded_reconstruct(weight, value, W):
    dp = [0] * (W + 1)
    choice = [-1] * (W + 1)

    for i, (w, v) in enumerate(zip(weight, value)):
        for c in range(w, W + 1):
            candidate = dp[c - w] + v
            if candidate > dp[c]:
                dp[c] = candidate
                choice[c] = i

    chosen = []
    c = W
    while c > 0 and choice[c] != -1:
        i = choice[c]
        chosen.append(i)
        c -= weight[i]

    chosen.reverse()
    return dp[W], chosen
```

---

## 28. Bounded Knapsack problem statement

In **Bounded Knapsack**, each item $ i $ may be used at most $ m_i $ times.

So it lies between:

- 0-1 Knapsack
- Unbounded Knapsack

This is also called **multiple knapsack** in some texts.

### Challenge
A direct expansion into $ m_i $ copies can be too slow if multiplicities are large.

So we want a smarter method.

---

## 29. Binary power splitting idea

A standard optimization is **binary power splitting**.

Suppose an item may be taken up to $ m $ times.

Instead of creating $ m $ identical copies, we split that quantity into groups of sizes:

```text
1, 2, 4, 8, ...
```

plus a final remainder if needed.

For example, if $ m = 13 $, we split it into:

```text
1, 2, 4, 6
```

because:

$$
1 + 2 + 4 + 6 = 13
$$

Each group becomes a synthetic 0-1 item with:

- weight multiplied by the group size
- value multiplied by the group size

This reduces the number of expanded items from $ m $ to $ O(\log m) $.

---

## 30. Why binary splitting works

Any number from 0 up to $ m $ can be represented as a sum of those split groups.

So choosing up to $ m $ copies of the original item is equivalent to choosing some subset of the split bundles.

That converts bounded knapsack into a 0-1 knapsack on a larger but much more manageable item set.

This is a very important engineering technique.

---

## 31. C++17 bounded knapsack with binary splitting

```cpp
#include <vector>
#include <algorithm>

int knapsack_bounded_binary_split(
    const std::vector<int>& weight,
    const std::vector<int>& value,
    const std::vector<int>& count,
    int W) {

    struct Item {
        int w, v;
    };

    std::vector<Item> expanded;

    for (int i = 0; i < static_cast<int>(weight.size()); ++i) {
        int k = count[i];
        int power = 1;
        while (power <= k) {
            expanded.push_back({weight[i] * power, value[i] * power});
            k -= power;
            power <<= 1;
        }
        if (k > 0) {
            expanded.push_back({weight[i] * k, value[i] * k});
        }
    }

    std::vector<int> dp(W + 1, 0);
    for (const auto& item : expanded) {
        for (int c = W; c >= item.w; --c) {
            dp[c] = std::max(dp[c], dp[c - item.w] + item.v);
        }
    }

    return dp[W];
}
```

---

## 32. Python bounded knapsack with binary splitting

```python
def knapsack_bounded_binary_split(weight, value, count, W):
    expanded = []

    for w, v, k in zip(weight, value, count):
        power = 1
        while power <= k:
            expanded.append((w * power, v * power))
            k -= power
            power <<= 1
        if k > 0:
            expanded.append((w * k, v * k))

    dp = [0] * (W + 1)
    for ew, ev in expanded:
        for c in range(W, ew - 1, -1):
            dp[c] = max(dp[c], dp[c - ew] + ev)

    return dp[W]
```

---

## 33. Reconstruction for bounded knapsack

Reconstruction becomes slightly more involved after binary splitting, because the chosen bundles must be mapped back to counts of original items.

A practical approach is:

1. store metadata with each expanded bundle:
   - original item index
   - bundle size
2. run ordinary 0-1 reconstruction on the expanded items
3. aggregate selected bundle sizes by original item

This produces the number of times each original item was chosen.

---

## 34. C++17 bounded knapsack reconstruction sketch

```cpp
#include <vector>
#include <algorithm>

struct Bundle {
    int w, v;
    int original;
    int multiplicity;
};

std::pair<int, std::vector<int>> knapsack_bounded_reconstruct(
    const std::vector<int>& weight,
    const std::vector<int>& value,
    const std::vector<int>& count,
    int W) {

    std::vector<Bundle> expanded;
    int n = static_cast<int>(weight.size());

    for (int i = 0; i < n; ++i) {
        int k = count[i];
        int power = 1;
        while (power <= k) {
            expanded.push_back({weight[i] * power, value[i] * power, i, power});
            k -= power;
            power <<= 1;
        }
        if (k > 0) {
            expanded.push_back({weight[i] * k, value[i] * k, i, k});
        }
    }

    int m = static_cast<int>(expanded.size());
    std::vector<std::vector<int>> dp(m + 1, std::vector<int>(W + 1, 0));

    for (int i = 1; i <= m; ++i) {
        for (int c = 0; c <= W; ++c) {
            dp[i][c] = dp[i - 1][c];
            if (expanded[i - 1].w <= c) {
                dp[i][c] = std::max(dp[i][c],
                                    dp[i - 1][c - expanded[i - 1].w] + expanded[i - 1].v);
            }
        }
    }

    std::vector<int> used(n, 0);
    int c = W;
    for (int i = m; i >= 1; --i) {
        if (dp[i][c] != dp[i - 1][c]) {
            used[expanded[i - 1].original] += expanded[i - 1].multiplicity;
            c -= expanded[i - 1].w;
        }
    }

    return {dp[m][W], used};
}
```

---

## 35. Python bounded knapsack reconstruction sketch

```python
def knapsack_bounded_reconstruct(weight, value, count, W):
    expanded = []
    n = len(weight)

    for i, (w, v, k) in enumerate(zip(weight, value, count)):
        power = 1
        while power <= k:
            expanded.append((w * power, v * power, i, power))
            k -= power
            power <<= 1
        if k > 0:
            expanded.append((w * k, v * k, i, k))

    m = len(expanded)
    dp = [[0] * (W + 1) for _ in range(m + 1)]

    for i in range(1, m + 1):
        ew, ev, _, _ = expanded[i - 1]
        for c in range(W + 1):
            dp[i][c] = dp[i - 1][c]
            if ew <= c:
                dp[i][c] = max(dp[i][c], dp[i - 1][c - ew] + ev)

    used = [0] * n
    c = W
    for i in range(m, 0, -1):
        if dp[i][c] != dp[i - 1][c]:
            ew, ev, idx, mult = expanded[i - 1]
            used[idx] += mult
            c -= ew

    return dp[m][W], used
```

---

## 36. Relationship to subset sum

**Subset sum** asks:

> Can we choose some items whose weights sum exactly to a target?

This is closely related to 0-1 Knapsack.

Instead of maximizing value, we track feasibility.

### Example DP idea

$$
dp[i][s] = \text{whether sum } s \text{ is achievable using first } i \text{ items}
$$

So subset sum is a knapsack-style DP with boolean states rather than value-maximizing states.

---

## 37. Relationship to coin change

Coin change comes in multiple forms.

### Minimum coins
Find the fewest number of coins needed to form a target amount.

### Number of ways
Count how many combinations form a target amount.

These are closely related to **unbounded knapsack**, because each coin denomination can often be used repeatedly.

So the knapsack family is part of a broader pattern of capacity-based dynamic programming.


### Knapsack Family Relationship Matrix

```text
+---------------------+-------------------------------+-----------------------------------------+
| Knapsack Problem    | Decision / Goal               | Direct Computational Cousin             |
+---------------------+-------------------------------+-----------------------------------------+
| 0-1 Knapsack        | Maximize value (at most once) | Subset Sum (Feasibility test, boolean)  |
| Unbounded Knapsack  | Maximize value (reuse items)  | Coin Change (Min coins / total ways)    |
| Bounded Knapsack    | Multiplicity limit m_i        | Binary Splitting -> 0-1 Knapsack        |
+---------------------+-------------------------------+-----------------------------------------+
```


---

## 38. Rolling arrays and when they help

A full 2D table is useful for:

- clarity
- reconstruction
- teaching

But if we only need the optimal value, 1D rolling arrays often reduce memory from:

$$
O(nW)
$$

to:

$$
O(W)
$$

This is a major practical improvement when capacity is large.

### Important caution
The correct loop direction depends on the variant:

- 0-1 -> backward
- unbounded -> forward

---

## 39. Common mistakes

### Mistake 1: using greedy for 0-1 Knapsack
Value-to-weight ratio is not generally correct here.

### Mistake 2: using the wrong loop direction
This is one of the most common bugs.

### Mistake 3: forgetting pseudo-polynomial behavior
$ O(nW) $ can still be very large if $ W $ is large.

### Mistake 4: reconstructing from a 1D DP without enough extra information
If reconstruction is needed, store parent or choice information, or use a 2D table.

### Mistake 5: expanding bounded multiplicities naively
If counts are large, direct expansion may be too slow.

### Mistake 6: mixing exact fill with at-most-capacity versions
Be clear whether the problem asks:
- best value with total weight at most $ W $
- or exact total weight $ W $

Those are different formulations.

---

## 40. Comparison table

| Variant | Item usage | Typical DP state | Loop direction | Time complexity |
|---|---|---|---|---:|
| 0-1 Knapsack | each item at most once | `dp[i][c]` or `dp[c]` | backward in 1D | $ O(nW) $ |
| Unbounded Knapsack | unlimited copies | `dp[c]` | forward | $ O(nW) $ |
| Bounded Knapsack | up to $ m_i $ copies | often via binary splitting to 0-1 | backward after expansion | $ O(W \sum \log m_i) $ after splitting |

---

## 41. Worked contrast among the three variants

Suppose an item has:

- weight = 3
- value = 5

and capacity is 9.

### 0-1 Knapsack
We may take it at most once.
Best contribution from that item type: value 5.

### Unbounded Knapsack
We may take it three times.
Best contribution from that item type alone: value 15.

### Bounded Knapsack with count 2
We may take it at most twice.
Best contribution from that item type alone: value 10.

This simple example shows how the usage rule changes the recurrence.

---

## 42. Proof intuition summary

### 0-1 Knapsack
At each item and capacity, the optimal solution either excludes the item or includes it once.

### Unbounded Knapsack
At each capacity, the optimal solution may include an item and still remain free to use it again.

### Bounded Knapsack with binary splitting
The allowed copy counts can be represented by sums of binary-sized bundles, converting the problem into a 0-1 instance on synthetic items.

These are all examples of state-and-transition reasoning.

---

## 43. Summary

The knapsack family studies optimization under capacity constraints.

The three main variants are:

- **0-1 Knapsack**, where each item is used at most once
- **Unbounded Knapsack**, where items may be reused freely
- **Bounded Knapsack**, where each item has a limited multiplicity

These problems are central in dynamic programming because they teach:

- state definition
- recurrence design
- loop-order correctness
- rolling-array optimization
- reconstruction
- pseudo-polynomial complexity

The most important practical lessons are:

- greedy fails for 0-1 knapsack
- backward iteration is necessary for 0-1 rolling DP
- forward iteration is necessary for unbounded knapsack
- binary power splitting makes bounded knapsack efficient

Knapsack is one of the best examples of how small rule changes produce different DP formulations.

---

## 44. Practice prompts

1. Why does greedy fail for 0-1 Knapsack?
2. What does the state $ dp[i][c] $ mean?
3. Why is 0-1 Knapsack pseudo-polynomial rather than strongly polynomial?
4. Why must the 1D capacity loop go backward in 0-1 Knapsack?
5. Why must the 1D capacity loop go forward in Unbounded Knapsack?
6. What problem does binary power splitting solve in bounded knapsack?
7. Why is reconstruction easier from a 2D table than from a compressed 1D array?

---

## 45. Suggested next topics

A natural continuation after the knapsack family is:

- edit distance and sequence alignment
- longest common subsequence
- interval DP
- digit DP
- DP optimization techniques