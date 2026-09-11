---
title: "Longest Common Subsequence"
difficulty: "Intermediate"
domains: ["Dynamic Programming", "String Algorithms", "Sequence Analysis", "Bioinformatics"]
prerequisites: ["1D and 2D Dynamic Programming Foundations", "Recursion", "Arrays and Memory Layout"]
related_topics: ["Edit Distance and Sequence Alignment", "Knapsack Family", "Longest Increasing Subsequence", "Two Pointers"]
---

# Longest Common Subsequence

> [!NOTE]
> The **Longest Common Subsequence (LCS)** problem seeks the longest sequence of elements that appears in the same relative order within two input sequences, though not necessarily contiguously. It is solved via dynamic programming over a 2D prefix grid $dp[i][j]$ in $O(nm)$ time, forming the backbone of sequence comparison, version control diffing, and bioinformatics alignment.
>
> Reference implementations: [C++17 Implementation](../../implementations/cpp/longest_common_subsequence.cpp) | [Python Implementation](../../implementations/python/longest_common_subsequence.py)

> [!TIP]
> **The 2D Sequence-DP Triad:**
> - **Levenshtein Distance:** Minimize edit transformation cost ($\min$ across match/substitute, insert, delete).
> - **Needleman-Wunsch Alignment:** Maximize global alignment score ($\max$ across match/mismatch score and gap penalties).
> - **Longest Common Subsequence (LCS):** Maximize shared ordered structural length ($dp[i-1][j-1]+1$ on match, $\max(dp[i-1][j], dp[i][j-1])$ on mismatch).
>
> **Direct Equivalences:**
> - **Unit-Cost Insertion-Deletion Distance:** $d_{\text{ID}}(X, Y) = n + m - 2 \cdot \text{LCS}(X, Y)$.
> - **Shortest Common Supersequence (SCS) Length:** $|\text{SCS}(X, Y)| = n + m - \text{LCS}(X, Y)$.

> [!WARNING]
> **Subsequence vs. Substring & Reconstruction Memory Trade-offs:**
> - **Subsequence $\neq$ Substring:** Subsequences allow arbitrary gaps; substrings require unbroken contiguity. Mismatches in LCS retain previous optimal prefix values ($\max(dp[i-1][j], dp[i][j-1])$), whereas substring DP resets to 0 immediately.
> - **Reconstruction vs. Space Optimization:** A 2-row rolling array achieves $O(nm)$ time in $O(\min(n, m))$ space for scalar length calculation, but discards state history. Recovering the actual string requires either the full $O(nm)$ table or linear-space divide-and-conquer checkpointing (**Hirschberg's Algorithm**).

The **Longest Common Subsequence**, usually abbreviated **LCS**, is one of the most important classical problems in dynamic programming.

It asks a simple but powerful question:

> Given two sequences, what is the longest subsequence that appears in both of them?

LCS is foundational because it teaches:

- 2D dynamic programming on prefixes
- the difference between subsequences and substrings
- reconstruction of an actual optimal answer
- the relationship between matching structure and edit operations

It also connects naturally to:

- edit distance
- sequence alignment
- diff tools
- version comparison
- bioinformatics
- plagiarism detection
- sequence similarity analysis

This chapter develops:

- the LCS problem definition
- the standard $ O(nm) $ dynamic programming recurrence
- full subsequence reconstruction
- rolling-array space optimization for length only
- the relationship between LCS and edit distance
- common variants and pitfalls

---

## 1. What is a subsequence?

A **subsequence** is obtained by deleting zero or more elements from a sequence without changing the order of the remaining elements.

### Example

If the sequence is:

```text
A B C B D A B
```

then these are subsequences:

- `A B D`
- `B C B A`
- `A B C B`

But this is not a subsequence:

- `B A C`

because the order does not match the original sequence.

---

## 2. Common subsequence

A **common subsequence** of two sequences is a subsequence that appears in both.

### Example

For:

```text
X = A B C B D A B
Y = B D C A B A
```

some common subsequences are:

- `B C A`
- `B D A`
- `B C B`
- `B D A B`

The goal of LCS is to find one with maximum possible length.

---

## 3. Subsequence versus substring

This is a very important distinction that frequently confuses beginners.

| Property | Subsequence | Substring (or Subarray) |
|---|---|---|
| **Definition** | Elements derived by deleting zero or more elements without changing order | Elements must form an unbroken, contiguous slice |
| **Contiguity** | Non-contiguous elements allowed | Strict contiguity required |
| **Example on `"ABCDE"`** | `"ACE"` is a valid subsequence | `"ACE"` is **not** a substring (`"BCD"` is) |
| **Total count for length $n$** | $2^n$ possible subsequences | $\frac{n(n+1)}{2}$ non-empty substrings |
| **DP Recurrence Behavior** | Retains cumulative optimal prefix scores ($\max(dp[i-1][j], dp[i][j-1])$) | Resets to 0 immediately upon character mismatch |

So LCS is fundamentally distinct from the longest common substring problem. In LCS, characters do not need to appear side-by-side in either parent sequence; they merely need to occur in matching chronological order.

---

## 4. Problem statement

Given two sequences $ X $ and $ Y $, find:

- the **length** of their longest common subsequence
- and often one actual longest common subsequence itself

If:

- $ X $ has length $ n $
- $ Y $ has length $ m $

then the classical DP solution runs in:

$$
O(nm)
$$

time.

---

## 5. Why LCS matters

LCS is important because it captures **ordered similarity**.

Two sequences may differ in many places, but still preserve a long common ordered structure.

This makes LCS useful in:

- comparing document revisions
- finding stable structure in noisy data
- DNA and protein sequence comparison
- computing differences between files
- measuring similarity without requiring exact alignment everywhere

It is also one of the cleanest examples of 2D dynamic programming.

---

## 6. Prefix-based state definition

The classical DP state is:

$$
dp[i][j] = \text{length of the LCS of } X[0 \dots i-1] \text{ and } Y[0 \dots j-1]
$$

This means:

- first $ i $ characters of $ X $
- first $ j $ characters of $ Y $

This prefix viewpoint is exactly the same style used in edit distance and global alignment.

---

## 7. Base cases

If either sequence is empty, then the longest common subsequence has length 0.

So:

$$
dp[i][0] = 0
$$

for all $ i $, and

$$
dp[0][j] = 0
$$

for all $ j $.

Also:

$$
dp[0][0] = 0
$$

This initializes the top row and left column.

---

## 8. Transition when the last characters match

If:

$$
X[i-1] = Y[j-1]
$$

then that matching character can be part of an LCS.

So:

$$
dp[i][j] = dp[i-1][j-1] + 1
$$

### Intuition
We extend the best common subsequence of the shorter prefixes by this new matching symbol.

---

## 9. Transition when the last characters do not match

If:

$$
X[i-1] \ne Y[j-1]
$$

then the last character of at least one prefix is not used in the LCS.

So we try:

- ignoring the last character of $ X $
- ignoring the last character of $ Y $

This gives:

$$
dp[i][j] = \max(dp[i-1][j], dp[i][j-1])
$$

---

## 10. Full recurrence

Putting the two cases together:

$$
dp[i][j] =
\begin{cases}
dp[i-1][j-1] + 1 & \text{if } X[i-1] = Y[j-1] \\
\max(dp[i-1][j], dp[i][j-1]) & \text{if } X[i-1] \ne Y[j-1]
\end{cases}
$$

This is the standard LCS recurrence.

---

## 11. Why the recurrence is correct

At state $ (i, j) $, there are two main cases.

### If the last characters match
Then an optimal common subsequence can end with that symbol, so we add 1 to the optimal answer for the shorter prefixes.

### If the last characters do not match
Then the final character of at least one prefix is not used in the optimal answer, so we compare:
- dropping the last character of $ X $
- dropping the last character of $ Y $

These cases cover all possibilities.

This is a classical example of:

- optimal substructure
- overlapping subproblems
- 2D prefix DP

---

## 12. Evaluation order

Because each state depends only on:

- `dp[i-1][j-1]`
- `dp[i-1][j]`
- `dp[i][j-1]`

we can fill the table row by row or column by column.

A common order is:

```text
for i from 1 to n:
    for j from 1 to m:
        compute dp[i][j]
```

This works because all required smaller states are already known.

---

## 13. C++17 reference implementation of LCS length

```cpp
#include <vector>
#include <string>
#include <algorithm>

int lcs_length(const std::string& x, const std::string& y) {
    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (x[i - 1] == y[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    return dp[n][m];
}
```

---

## 14. Python reference implementation of LCS length

```python
def lcs_length(x, y):
    n, m = len(x), len(y)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            if x[i - 1] == y[j - 1]:
                dp[i][j] = dp[i - 1][j - 1] + 1
            else:
                dp[i][j] = max(dp[i - 1][j], dp[i][j - 1])

    return dp[n][m]
```

---

## 15. Worked example

Consider:

```text
X = A B C B D A B
Y = B D C A B A
```

The LCS length is:

$$
4
$$

One valid LCS is:

```text
B C B A
```

Another may be:

```text
B D A B
```

So the LCS is not always unique.

That is an important point for reconstruction.

---

## 16. Reconstructing an actual LCS

The DP table gives the length, but we often want the actual subsequence.

We reconstruct by backtracking from:

$$
dp[n][m]
$$

### Rule
- if `x[i-1] == y[j-1]`, that character belongs to the LCS
- otherwise move toward the neighboring state with the same optimal value

Then reverse the collected characters.

---

## 17. C++17 LCS reconstruction

```cpp
#include <vector>
#include <string>
#include <algorithm>

std::string lcs_sequence(const std::string& x, const std::string& y) {
    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (x[i - 1] == y[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
            }
        }
    }

    std::string result;
    int i = n, j = m;

    while (i > 0 && j > 0) {
        if (x[i - 1] == y[j - 1]) {
            result.push_back(x[i - 1]);
            --i;
            --j;
        } else if (dp[i - 1][j] >= dp[i][j - 1]) {
            --i;
        } else {
            --j;
        }
    }

    std::reverse(result.begin(), result.end());
    return result;
}
```

---

## 18. Python LCS reconstruction

```python
def lcs_sequence(x, y):
    n, m = len(x), len(y)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            if x[i - 1] == y[j - 1]:
                dp[i][j] = dp[i - 1][j - 1] + 1
            else:
                dp[i][j] = max(dp[i - 1][j], dp[i][j - 1])

    result = []
    i, j = n, m

    while i > 0 and j > 0:
        if x[i - 1] == y[j - 1]:
            result.append(x[i - 1])
            i -= 1
            j -= 1
        elif dp[i - 1][j] >= dp[i][j - 1]:
            i -= 1
        else:
            j -= 1

    result.reverse()
    return ''.join(result)
```

---

## 19. Why multiple LCS answers can exist

The DP recurrence can create ties whenever:

$$
dp[i-1][j] = dp[i][j-1]
$$

When this equality occurs during backtracing, moving either up (excluding $X[i-1]$) or left (excluding $Y[j-1]$) leads to an equally valid optimal subsequence.

### Concrete Example: Tie Demonstration

Consider:

```text
X = ABCD
Y = ACBD
```

Both strings have length 4. Their optimal LCS has length **3**. However, there are two distinct common subsequences of maximum length 3:
1. `ABD` (selecting characters at indices 0, 1, 3 in $X$ and 0, 2, 3 in $Y$)
2. `ACD` (selecting characters at indices 0, 2, 3 in $X$ and 0, 1, 3 in $Y$)

When the backtracker reaches cell $(3, 3)$ (comparing prefixes `"ABC"` and `"ACB"`):
- $X[2] = \text{'C'}$ and $Y[2] = \text{'B'}$ do not match.
- $dp[2][3] = 2$ (LCS of `"AB"` and `"ACB"` is `"AB"`, length 2).
- $dp[3][2] = 2$ (LCS of `"ABC"` and `"AC"` is `"AC"`, length 2).

Since $dp[2][3] = dp[3][2] = 2$, a tie occurs. Neither branch is mathematically wrong; both branch choices yield optimal global subsequences.

---

## 20. Deterministic tie handling

When implementing sequence reconstruction in production libraries or competitive programming, arbitrary non-deterministic branching makes unit testing difficult. Therefore, a deterministic tie-breaking policy should always be codified:

- **Prefer Up (`dp[i-1][j] >= dp[i][j-1]`):** Prioritizes pruning the prefix of $X$ first (reconstructs `ABD` in the example above).
- **Prefer Left (`dp[i-1][j] < dp[i][j-1]`):** Prioritizes pruning the prefix of $Y$ first (reconstructs `ACD` in the example above).

The key engineering takeaway is:
> **Optimality is unique in scalar length ($L$), but non-unique in combinatorial witness sequences.** A robust test suite validates that any reconstructed witness is a valid subsequence of both inputs and has length exactly equal to $dp[n][m]$.

---

## 21. Rolling-array optimization for LCS length

If we only want the LCS length, not the sequence itself, then we do not need the full table.

Each row depends only on:

- the previous row
- the current row so far

So we can reduce space from:

$$
O(nm)
$$

to:

$$
O(m)
$$

using two rows.

---

## 22. C++17 rolling-array LCS length

```cpp
#include <vector>
#include <string>
#include <algorithm>

int lcs_length_rolling(const std::string& x, const std::string& y) {
    int n = static_cast<int>(x.size());
    int m = static_cast<int>(y.size());

    std::vector<int> prev(m + 1, 0), cur(m + 1, 0);

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (x[i - 1] == y[j - 1]) {
                cur[j] = prev[j - 1] + 1;
            } else {
                cur[j] = std::max(prev[j], cur[j - 1]);
            }
        }
        std::swap(prev, cur);
        std::fill(cur.begin(), cur.end(), 0);
    }

    return prev[m];
}
```

---

## 23. Python rolling-array LCS length

```python
def lcs_length_rolling(x, y):
    n, m = len(x), len(y)
    prev = [0] * (m + 1)
    cur = [0] * (m + 1)

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            if x[i - 1] == y[j - 1]:
                cur[j] = prev[j - 1] + 1
            else:
                cur[j] = max(prev[j], cur[j - 1])
        prev, cur = cur, prev
        for j in range(m + 1):
            cur[j] = 0

    return prev[m]
```

---

## 24. Why rolling arrays do not easily reconstruct the sequence

Rolling arrays discard earlier rows as soon as the current row is computed. While this achieves optimal $O(\min(n, m))$ memory when evaluating the scalar LCS length, it permanently destroys the transition history required to walk backward from $(n, m)$ to $(0, 0)$.

| Goal | Time Complexity | Auxiliary Space | Storage Choice | Notes |
|---|---|---|---|---|
| **LCS Length Only** | $O(nm)$ | $O(\min(n, m))$ | Rolling 2 rows (`prev` and `cur`) | Optimal for filtering, scoring, and metrics |
| **One Actual Optimal LCS** | $O(nm)$ | $O(nm)$ | Full $(n+1) \times (m+1)$ table | Standard textbook backtracing |
| **Subsequence in Linear Space** | $O(nm)$ | $O(\min(n, m))$ | Hirschberg's divide-and-conquer | Uses DP forward and backward to find midpoint split |

This mirrors the exact storage trade-off encountered in global alignment and edit distance:
- **Lower memory footprint** $\iff$ **Loss of direct backtrace pointer history**.

---

## 25. LCS versus edit distance

LCS and edit distance are closely related, but they ask different questions.

### LCS
Maximize the length of shared ordered structure.

### Edit distance
Minimize the cost of transforming one string into another.

They are different viewpoints on sequence similarity.

---

## 26. Relationship between LCS and insertion-deletion distance

If the only allowed operations are:

- insert
- delete

and substitution is not allowed as a single step, then the edit distance between two strings can be expressed using LCS.

If:

- $ n = |X| $
- $ m = |Y| $
- $ L = \text{LCS length} $

then the minimum number of insertions and deletions needed is:

$$
(n - L) + (m - L) = n + m - 2L
$$

### Why?
- delete symbols from $ X $ that are not in the chosen common subsequence
- insert symbols needed to reach $ Y $

This is a very important conceptual connection.

---

## 27. LCS versus longest common substring

These are often confused.

### Longest common subsequence
Characters stay in order, but gaps are allowed.

### Longest common substring
Characters must be contiguous in both strings.

Example:

```text
X = ABCDEF
Y = AXYCDEF
```

- a common subsequence could skip characters
- a common substring must appear as one consecutive block

So the DP definitions are different.

---

## 28. Generalizing beyond strings

LCS is not limited to strings.

It works for any sequences where equality comparison makes sense, such as:

- arrays of integers
- token streams
- lines in files
- biological symbol sequences

So the same DP idea applies quite broadly.

---

## 29. Diff and version comparison intuition

LCS is closely related to how file-difference tools think.

If two versions of a text share a long common subsequence of lines or tokens, then the unchanged structure can be preserved, and the differences can be described around it.

This is one reason LCS is historically important in comparison tools.

---

## 30. Variants of LCS

Important variants include:

- longest common substring
- shortest common supersequence
- three-sequence LCS
- weighted LCS
- constrained LCS

The standard two-sequence LCS is the foundational starting point.

---

## 31. Shortest common supersequence connection

A **shortest common supersequence** is the shortest sequence that contains both input sequences as subsequences.

If:

- $ n = |X| $
- $ m = |Y| $
- $ L = \text{LCS length} $

then the shortest common supersequence length is:

$$
n + m - L
$$

This is another important structural connection.

---

## 32. Complexity summary

If the sequence lengths are $ n $ and $ m $, then:

### Full-table LCS
- **Time:** $ O(nm) $
- **Space:** $ O(nm) $

### Rolling-array LCS length
- **Time:** $ O(nm) $
- **Space:** $ O(m) $

These are standard complexities for classical LCS DP.

---

## 33. Common mistakes

### Mistake 1: confusing subsequence with substring
This is the most common conceptual error.

### Mistake 2: defining the wrong state
Be clear that `dp[i][j]` refers to the first `i` and first `j` characters.

### Mistake 3: incorrect base row or base column
If one sequence is empty, LCS length must be 0.

### Mistake 4: reconstructing without consistent tie handling
Multiple correct LCS answers may exist.

### Mistake 5: expecting rolling arrays to reconstruct the sequence directly
Usually they cannot.

### Mistake 6: mixing LCS with edit distance formulas
They are related, but not identical problems.

---

## 34. Comparison with nearby sequence DP problems

| Problem | Goal | Key recurrence style | Objective |
|---|---|---|---|
| LCS | preserve common ordered structure | match -> diagonal + 1, else max of up and left | maximize length |
| Edit distance | transform one sequence into another | min of delete, insert, substitute | minimize cost |
| Needleman-Wunsch | align full sequences | max of diagonal, up, left with scores | maximize score |
| Longest common substring | preserve contiguous common block | reset on mismatch | maximize length |

This comparison helps place LCS within the broader sequence-DP family.

---

## 35. Proof intuition summary

The LCS recurrence is correct because:

- if the last characters match, they can extend an optimal solution for the shorter prefixes
- if they do not match, at least one of those last characters is excluded from an optimal solution

So examining:
- the diagonal on match
- the up and left neighbors on mismatch

covers all possibilities.

---

## 36. Worked table intuition

Suppose:

```text
X = ABCD
Y = ACBD
```

Then:

- `A` matches `A`
- `B` and `C` appear in different positions
- `D` matches `D`

An LCS has length:

$$
3
$$

such as:

```text
ABD
```

or

```text
ACD
```

This is a good small example for seeing ties and multiple valid reconstructions.

---

## 37. Summary

The Longest Common Subsequence problem asks for the longest ordered sequence shared by two input sequences.

The classical dynamic programming solution uses the state:

$$
dp[i][j] = \text{LCS length of the first } i \text{ and first } j \text{ symbols}
$$

with recurrence:

$$
dp[i][j] =
\begin{cases}
dp[i-1][j-1] + 1 & \text{if the last symbols match} \\
\max(dp[i-1][j], dp[i][j-1]) & \text{otherwise}
\end{cases}
$$

LCS is a foundational sequence-DP problem because it teaches:

- 2D prefix states
- reconstruction
- ties and non-uniqueness
- rolling-array optimization
- relationships to edit distance and supersequence problems

It is one of the clearest bridges between abstract dynamic programming and practical sequence comparison.

---

## 38. Practice prompts

1. What is the difference between a subsequence and a substring?
2. What does $ dp[i][j] $ represent in LCS?
3. Why is the base row and base column equal to 0?
4. Why do matching last characters use the diagonal state?
5. Why do mismatching last characters lead to the max of up and left?
6. Why can multiple different LCS answers exist?
7. Why do rolling arrays help for length but not for full reconstruction?
8. How is LCS related to insertion-deletion edit distance?

---

## 39. Suggested next topics

A natural continuation after LCS is:

- interval DP
- tree DP
- bitmask and state-compression DP
- shortest common supersequence
- advanced sequence DP
