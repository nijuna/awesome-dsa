---
title: "1D and 2D Dynamic Programming Foundations"
difficulty: "Beginner to Intermediate"
domains: ["Dynamic Programming", "Algorithm Design Paradigms", "Optimization", "Recursion"]
prerequisites: ["Recursion", "Arrays and Memory Layout", "Asymptotic Analysis", "Divide and Conquer"]
related_topics: ["Longest Increasing Subsequence", "Knapsack Family", "Edit Distance", "Matrix Chain Multiplication"]
---

# 1D and 2D Dynamic Programming Foundations

> [!NOTE]
> A **Dynamic Programming (DP)** algorithm solves complex optimization, counting, or decision problems by decomposing them into overlapping subproblems, solving each subproblem exactly once, and storing its solution in a memo table or DP array to eliminate redundant work.
>
> Reference implementations: [C++17 Implementation](../../implementations/cpp/dynamic_programming_foundations.cpp) | [Python Implementation](../../implementations/python/dynamic_programming_foundations.py)

> [!TIP]
> **The 5-Step DP Formulation Recipe:**
> 1. **State Definition:** Explicitly define $dp[\dots]$ in plain words (e.g., $dp[i]$ is the optimal answer for prefix $A[0 \dots i-1]$).
> 2. **Recurrence Relation:** Express state $dp[i]$ strictly in terms of smaller subproblems (transitions).
> 3. **Base Cases & Sentinels:** Initialize the smallest trivial subproblems without circular dependencies.
> 4. **Topological Evaluation Order:** Iterate or recurse such that when computing state $S$, all dependencies $\text{pred}(S)$ are already solved.
> 5. **Goal & Reconstruction:** Identify which cell holds the target answer, and optionally trace parent pointers backwards to recover the optimal path.

> [!WARNING]
> **Common Dynamic Programming Pitfalls:**
> - **Cyclic Dependencies:** If $A$ depends on $B$ and $B$ depends on $A$, the state graph is not a Directed Acyclic Graph (DAG), and DP will loop infinitely or access uninitialized cells.
> - **Under-specified State:** If future decisions depend on history not captured in the state index, optimal substructure is violated (e.g., traveling salesperson requiring visited set as bitmask).
> - **Off-by-One / Initialization Errors:** Forgetting 0-indexing vs. 1-indexing or misinitializing unreachable states with 0 instead of $\infty$ can invalidate $\min$ transitions.

Dynamic programming, usually abbreviated **DP**, is one of the most powerful algorithm design paradigms.

It is used when a problem has two important features:

- **optimal substructure**
- **overlapping subproblems**

The central idea is:

> Solve each important subproblem once, store its answer, and reuse it instead of recomputing it.

This often turns an exponential brute-force recursion into a polynomial-time algorithm.

Dynamic programming appears in many areas:

- path counting
- shortest and cheapest paths on grids
- sequence comparison
- knapsack problems
- scheduling
- game theory
- optimization over prefixes
- string algorithms

This chapter introduces the core foundations of DP through:

- state design
- recurrence construction
- memoization
- tabulation
- 1D DP
- 2D DP
- path reconstruction
- common pitfalls

The goal is not only to solve a few examples, but to build a reusable way of thinking.

---

## 1. What is dynamic programming?

Dynamic programming is a method for solving problems by breaking them into smaller subproblems and storing intermediate results.

Instead of recomputing the same subproblem many times, we save its answer the first time and reuse it later.

### Main principle

If a problem repeatedly asks for the answers to the same smaller questions, then caching those answers can save enormous time.

---

## 2. Optimal substructure

A problem has **optimal substructure** if an optimal solution can be built from optimal solutions to smaller subproblems.

### Example idea
If the cheapest way to reach a cell in a grid ends by coming from above or from the left, then the best path to that cell depends on the best paths to those earlier cells.

This is the kind of structure DP needs.

---

## 3. Overlapping subproblems

A problem has **overlapping subproblems** if the same smaller subproblem appears many times during recursive solving.

### Example
For Fibonacci:

$$
F(n) = F(n-1) + F(n-2)
$$

A naive recursive computation of $ F(n) $ recomputes values like $ F(n-3) $, $ F(n-4) $, and many others repeatedly.

That overlap is what makes dynamic programming useful.

---

## 4. Dynamic programming versus divide and conquer

Dynamic programming and divide and conquer are related, but they solve different kinds of recursive structure.

### Divide and conquer
Usually works best when the subproblems are mostly disjoint.

### Dynamic programming
Works best when the subproblems overlap heavily.

### Example contrast

#### Merge sort
- left half and right half are different subproblems
- little or no overlap
- divide and conquer is natural

#### Fibonacci
- the same smaller values are recomputed repeatedly
- heavy overlap
- dynamic programming is natural

So the key distinction is:

> divide and conquer handles independent subproblems, while dynamic programming handles overlapping subproblems.

---

## 5. Dynamic programming versus greedy algorithms

Greedy algorithms make locally best choices and do not usually reconsider them.

Dynamic programming considers many possibilities systematically and stores the best result for each state.

### Greedy
Fast and elegant when the greedy choice property holds.

### Dynamic programming
More general, but often more expensive.

A good rule is:

- if a safe local rule can be proved, greedy may work
- if local choices interact globally, DP is often needed


### Paradigm Comparison Matrix

| Paradigm | Core Decision Strategy | Best For | Subproblem Relationship | Canonical Examples |
|---|---|---|---|---|
| **Greedy** | Irrevocable local best choice at each step | Problems with proven greedy choice property | Sequential extension (no subproblem re-evaluation) | Kruskal's MST, Dijkstra, Huffman Coding |
| **Divide & Conquer** | Partition into subproblems, solve recursively, combine | Independent partitions | Disjoint / Non-overlapping | Merge Sort, QuickSelect, Karatsuba Multiplication |
| **Dynamic Programming** | Evaluate all valid transitions; cache subproblem solutions | Global optimization, counting, feasibility | Heavily overlapping subproblems | Grid Paths, Knapsack, LIS, Edit Distance |


---

## 6. The DP workflow

A useful DP workflow is:

1. Define the **state**
2. Define the **transition**
3. Define the **base cases**
4. Choose **memoization** or **tabulation**
5. Compute the answer
6. Optionally reconstruct the solution

This is the most important habit to learn.

---

## 7. What is a state?

A **state** describes exactly the information needed to define a subproblem.

Good DP design begins with the question:

> What smaller question am I solving?

Examples:

- `dp[i]` = best answer using the first `i` elements
- `dp[r][c]` = best answer at grid cell `(r, c)`
- `dp[i][j]` = answer for prefix `i` of one sequence and prefix `j` of another

A good state is:

- precise
- minimal
- sufficient for future transitions

---

## 8. What is a transition?

A **transition** explains how to compute one state from smaller states.

It is the recurrence relation of the DP.

### Example
If you can reach grid cell `(r, c)` only from above or left, then:

$$
dp[r][c] = \min(dp[r-1][c], dp[r][c-1]) + cost[r][c]
$$

This says:

- solve smaller states first
- then combine them to obtain the current state

---

## 9. What are base cases?

Base cases are the smallest subproblems whose answers are known directly.

Without correct base cases, the whole recurrence fails.

### Examples
- `dp[0] = 0`
- first row of a grid
- first column of a grid
- empty prefix of a string

In many DP bugs, the recurrence is correct but the base cases are wrong.

---

## 10. Memoization and tabulation

There are two main ways to implement DP.

### A. Memoization
- top-down
- recursive
- compute states only when needed
- store answers in a cache

### B. Tabulation
- bottom-up
- iterative
- compute states in an order where dependencies are already known

Both approaches solve the same recurrence. They differ in evaluation order and implementation style.

---

## 11. Memoization intuition

Memoization starts with the original recursive definition.

Whenever a state is computed:

- store the result
- if the same state is requested again, return the stored value

This preserves recursive clarity while avoiding repeated work.

### Strengths
- easy to write from a recursive definition
- computes only reachable states
- often easier for irregular state spaces

### Weaknesses
- recursion overhead
- possible stack-depth issues
- sometimes less explicit than bottom-up ordering

---

## 12. Tabulation intuition

Tabulation fills a table from smaller states to larger states.

The main question is:

> In what order can I compute the states so that each dependency is already available?

### Strengths
- iterative and often faster
- avoids recursion depth problems
- makes dependency order explicit

### Weaknesses
- sometimes harder to design initially
- may compute unnecessary states

---

## 13. Fibonacci as the first DP example

Fibonacci is simple, but it clearly shows why DP matters.

### Recurrence

$$
F(n) = F(n-1) + F(n-2)
$$

with base cases:

$$
F(0) = 0,\quad F(1) = 1
$$

Naive recursion is exponential because it recomputes the same values many times.

Dynamic programming reduces it to linear time.

---

## 14. C++17 memoized Fibonacci

```cpp
#include <vector>

long long fibonacci_memo(int n) {
    std::vector<long long> memo(n + 1, -1);

    auto solve = [&](auto&& self, int x) -> long long {
        if (x <= 1) return x;
        if (memo[x] != -1) return memo[x];
        return memo[x] = self(self, x - 1) + self(self, x - 2);
    };

    return solve(solve, n);
}
```

---

## 15. Python memoized Fibonacci

```python
def fibonacci_memo(n):
    memo = [-1] * (n + 1)

    def solve(x):
        if x <= 1:
            return x
        if memo[x] != -1:
            return memo[x]
        memo[x] = solve(x - 1) + solve(x - 2)
        return memo[x]

    return solve(n)
```

---

## 16. C++17 tabulated Fibonacci

```cpp
long long fibonacci_tab(int n) {
    if (n <= 1) return n;

    std::vector<long long> dp(n + 1, 0);
    dp[1] = 1;

    for (int i = 2; i <= n; ++i) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }

    return dp[n];
}
```

---

## 17. Python tabulated Fibonacci

```python
def fibonacci_tab(n):
    if n <= 1:
        return n

    dp = [0] * (n + 1)
    dp[1] = 1

    for i in range(2, n + 1):
        dp[i] = dp[i - 1] + dp[i - 2]

    return dp[n]
```

---

## 18. Space optimization

Sometimes a DP table stores more information than we truly need.

If each state depends only on a few recent earlier states, we may compress space.

### Fibonacci example
Instead of keeping the whole table, we only need the previous two values.

This reduces space from:

$$
O(n)
$$

to:

$$
O(1)
$$

### General lesson
Always ask:

> Which earlier states are actually needed after this step?

---

## 19. C++17 space-optimized Fibonacci

```cpp
long long fibonacci_optimized(int n) {
    if (n <= 1) return n;

    long long prev2 = 0;
    long long prev1 = 1;

    for (int i = 2; i <= n; ++i) {
        long long cur = prev1 + prev2;
        prev2 = prev1;
        prev1 = cur;
    }

    return prev1;
}
```

---

## 20. Python space-optimized Fibonacci

```python
def fibonacci_optimized(n):
    if n <= 1:
        return n

    prev2, prev1 = 0, 1
    for _ in range(2, n + 1):
        prev2, prev1 = prev1, prev1 + prev2
    return prev1
```

---

## 21. 1D DP pattern

A **1D DP** usually has the form:

```text
dp[i] = answer for prefix or position i
```

This is common when the problem moves along:

- an array
- a line
- a sequence
- a timeline

### Typical examples
- Fibonacci
- climbing stairs
- minimum cost to reach position `i`
- maximum sum ending at `i`

The important feature is that one index is enough to describe the state.

---

## 22. Example: climbing stairs

### Problem
If you can climb either 1 step or 2 steps at a time, how many ways are there to reach step `n`?

### State
Let:

$$
dp[i] = \text{number of ways to reach step } i
$$

### Transition

$$
dp[i] = dp[i-1] + dp[i-2]
$$

because the last move came from either:

- step $ i-1 $, or
- step $ i-2 $

### Base cases

$$
dp[0] = 1,\quad dp[1] = 1
$$

---

## 23. C++17 climbing stairs

```cpp
long long climbing_stairs(int n) {
    if (n <= 1) return 1;

    std::vector<long long> dp(n + 1, 0);
    dp[0] = 1;
    dp[1] = 1;

    for (int i = 2; i <= n; ++i) {
        dp[i] = dp[i - 1] + dp[i - 2];
    }

    return dp[n];
}
```

---

## 24. Python climbing stairs

```python
def climbing_stairs(n):
    if n <= 1:
        return 1

    dp = [0] * (n + 1)
    dp[0] = 1
    dp[1] = 1

    for i in range(2, n + 1):
        dp[i] = dp[i - 1] + dp[i - 2]

    return dp[n]
```

---

## 25. 2D DP pattern

A **2D DP** usually has the form:

```text
dp[i][j] = answer for state described by two indices
```

This is common when the problem depends on two dimensions such as:

- row and column in a grid
- two prefixes of strings
- starting and ending positions
- item index and remaining capacity

### Typical examples
- grid path counting
- minimum path sum
- edit distance
- longest common subsequence
- knapsack tables

---

## 26. Example: counting paths in a grid

### Problem
How many ways are there to move from the top-left corner to the bottom-right corner of an $ r \times c $ grid if you can only move:

- right
- down

### State
Let:

$$
dp[i][j] = \text{number of ways to reach cell } (i, j)
$$

### Transition

$$
dp[i][j] = dp[i-1][j] + dp[i][j-1]
$$

because the last move came from:

- above, or
- left

### Base case
The start cell has one way:

$$
dp[0][0] = 1
$$

---

## 27. C++17 grid path counting

```cpp
#include <vector>

long long count_grid_paths(int rows, int cols) {
    std::vector<std::vector<long long>> dp(rows, std::vector<long long>(cols, 0));
    dp[0][0] = 1;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (i > 0) dp[i][j] += dp[i - 1][j];
            if (j > 0) dp[i][j] += dp[i][j - 1];
        }
    }

    return dp[rows - 1][cols - 1];
}
```

---

## 28. Python grid path counting

```python
def count_grid_paths(rows, cols):
    dp = [[0] * cols for _ in range(rows)]
    dp[0][0] = 1

    for i in range(rows):
        for j in range(cols):
            if i > 0:
                dp[i][j] += dp[i - 1][j]
            if j > 0:
                dp[i][j] += dp[i][j - 1]

    return dp[rows - 1][cols - 1]
```

---

## 29. Example: minimum path sum in a grid

Now suppose each cell has a nonnegative cost, and we want the minimum total cost path from the top-left to the bottom-right.

### State

$$
dp[i][j] = \text{minimum cost to reach cell } (i, j)
$$

### Transition

$$
dp[i][j] = \min(dp[i-1][j], dp[i][j-1]) + cost[i][j]
$$

This is a classic 2D DP optimization problem.

---

## 30. C++17 minimum path sum

```cpp
#include <vector>
#include <algorithm>

long long min_path_sum(const std::vector<std::vector<int>>& cost) {
    int rows = static_cast<int>(cost.size());
    int cols = static_cast<int>(cost[0].size());

    std::vector<std::vector<long long>> dp(rows, std::vector<long long>(cols, 0));
    dp[0][0] = cost[0][0];

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (i == 0 && j == 0) continue;

            long long best = (1LL << 60);
            if (i > 0) best = std::min(best, dp[i - 1][j]);
            if (j > 0) best = std::min(best, dp[i][j - 1]);
            dp[i][j] = best + cost[i][j];
        }
    }

    return dp[rows - 1][cols - 1];
}
```

---

## 31. Python minimum path sum

```python
def min_path_sum(cost):
    rows, cols = len(cost), len(cost[0])
    dp = [[0] * cols for _ in range(rows)]
    dp[0][0] = cost[0][0]

    for i in range(rows):
        for j in range(cols):
            if i == 0 and j == 0:
                continue

            best = 10**18
            if i > 0:
                best = min(best, dp[i - 1][j])
            if j > 0:
                best = min(best, dp[i][j - 1])

            dp[i][j] = best + cost[i][j]

    return dp[rows - 1][cols - 1]
```

---

## 32. Reconstructing a solution path

DP often computes only the optimal value, but sometimes we also want the actual solution.

This can be done by storing:

- a parent pointer
- a choice table
- or enough information to backtrack from the final state

### Grid example
If `dp[i][j]` came from:

- `dp[i-1][j]`, store "up"
- `dp[i][j-1]`, store "left"

Then starting from the goal, we can reconstruct the chosen path backward.

---

## 33. C++17 path reconstruction for minimum path sum

```cpp
#include <vector>
#include <string>
#include <algorithm>

std::vector<std::pair<int, int>> reconstruct_min_path(
    const std::vector<std::vector<int>>& cost) {
    int rows = static_cast<int>(cost.size());
    int cols = static_cast<int>(cost[0].size());

    std::vector<std::vector<long long>> dp(rows, std::vector<long long>(cols, 0));
    std::vector<std::vector<std::pair<int, int>>> parent(
        rows, std::vector<std::pair<int, int>>(cols, {-1, -1}));

    dp[0][0] = cost[0][0];

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            if (i == 0 && j == 0) continue;

            long long from_up = (i > 0 ? dp[i - 1][j] : (1LL << 60));
            long long from_left = (j > 0 ? dp[i][j - 1] : (1LL << 60));

            if (from_up <= from_left) {
                dp[i][j] = from_up + cost[i][j];
                parent[i][j] = {i - 1, j};
            } else {
                dp[i][j] = from_left + cost[i][j];
                parent[i][j] = {i, j - 1};
            }
        }
    }

    std::vector<std::pair<int, int>> path;
    int i = rows - 1, j = cols - 1;

    while (i != -1 && j != -1) {
        path.push_back({i, j});
        auto [pi, pj] = parent[i][j];
        i = pi;
        j = pj;
    }

    std::reverse(path.begin(), path.end());
    return path;
}
```

---

## 34. Python path reconstruction for minimum path sum

```python
def reconstruct_min_path(cost):
    rows, cols = len(cost), len(cost[0])
    dp = [[0] * cols for _ in range(rows)]
    parent = [[(-1, -1)] * cols for _ in range(rows)]

    dp[0][0] = cost[0][0]

    for i in range(rows):
        for j in range(cols):
            if i == 0 and j == 0:
                continue

            from_up = dp[i - 1][j] if i > 0 else 10**18
            from_left = dp[i][j - 1] if j > 0 else 10**18

            if from_up <= from_left:
                dp[i][j] = from_up + cost[i][j]
                parent[i][j] = (i - 1, j)
            else:
                dp[i][j] = from_left + cost[i][j]
                parent[i][j] = (i, j - 1)

    path = []
    i, j = rows - 1, cols - 1
    while i != -1 and j != -1:
        path.append((i, j))
        i, j = parent[i][j]

    path.reverse()
    return path
```

---

## 35. Ordering states correctly

In bottom-up DP, state order matters.

You must compute each state only after all states it depends on are already available.

### Examples
- Fibonacci: increasing `i`
- grid DP: row-major or column-major, if each cell depends only on earlier cells
- string DP: increasing prefix lengths

A wrong order gives incorrect answers even if the recurrence is correct.

---

## 36. Choosing between memoization and tabulation

A useful rule of thumb is:

### Prefer memoization when:
- the recurrence is easy to express recursively
- only some states are reachable
- the state space is irregular

### Prefer tabulation when:
- the dependency order is clear
- you want better constant factors
- recursion depth may be large
- you want easier space optimization

Both are valuable tools.

---

## 37. Time and space complexity in DP

To estimate DP complexity, ask two questions:

### Time
How many states are there, and how much work is done per state?

### Space
How many state values must be stored at once?

### Example
If there are $ O(nm) $ states and each takes $ O(1) $ work, then total time is:

$$
O(nm)
$$

This simple counting method is one of the most useful DP habits.

---

## 38. Common mistakes

### Mistake 1: defining the wrong state
If the state does not contain enough information, transitions will be invalid.

### Mistake 2: incorrect base cases
A recurrence can be mathematically correct but still fail if initialization is wrong.

### Mistake 3: wrong iteration order
Bottom-up DP must respect dependency order.

### Mistake 4: double counting
In counting problems, it is easy to count the same structure more than once.

### Mistake 5: forgetting unreachable states
Some states may need special sentinel values such as infinity or negative infinity.

### Mistake 6: using DP when a simpler method exists
Not every problem with recurrence structure needs full DP.

---

## 39. How to recognize DP problems

Dynamic programming is promising when:

- brute force recursion repeats the same subproblems
- the problem asks for optimum, count, or feasibility
- the answer for a larger instance depends on smaller instances
- state transitions can be written clearly
- caching repeated work would save time

Typical question forms include:

- maximum or minimum value
- number of ways
- can it be done
- best answer using first `i` items
- best answer at position `(i, j)`

---

## 40. 1D versus 2D foundations

### 1D DP
Use one index when the subproblem depends on one main dimension.

Examples:
- Fibonacci
- climbing stairs
- linear cost optimization

### 2D DP
Use two indices when the subproblem depends on two dimensions.

Examples:
- grids
- pairwise sequence prefixes
- item index and capacity
- interval endpoints in some formulations

This chapter focuses on these two foundational patterns because many larger DP topics grow directly from them.

---

## 41. Summary

Dynamic programming solves problems with:

- **optimal substructure**
- **overlapping subproblems**

The main workflow is:

1. define the state
2. define the transition
3. define the base cases
4. choose memoization or tabulation
5. compute and possibly reconstruct the solution

Foundational DP patterns include:

- **1D DP**, such as Fibonacci and climbing stairs
- **2D DP**, such as grid path counting and minimum path sum

Important practical skills include:

- identifying overlapping subproblems
- choosing a good state
- ordering computation correctly
- optimizing space when possible
- reconstructing actual solutions, not only values

Dynamic programming is one of the central tools of algorithm design because it turns repeated recursive work into structured efficient computation.

---

## 42. Practice prompts

1. What are the two main properties that suggest dynamic programming?
2. What is the difference between memoization and tabulation?
3. Why is Fibonacci a classic DP example?
4. What makes a good DP state?
5. Why does grid path counting naturally lead to a 2D DP table?
6. How can space sometimes be reduced in DP?
7. Why is iteration order important in bottom-up DP?

---

## 43. Suggested next topics

A natural continuation after 1D and 2D DP foundations is:

- longest increasing subsequence
- knapsack family
- edit distance and sequence alignment
- longest common subsequence
- interval DP