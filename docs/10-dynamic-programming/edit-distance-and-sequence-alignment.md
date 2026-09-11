---
title: "Edit Distance and Sequence Alignment"
difficulty: "Intermediate"
domains: ["Dynamic Programming", "String Algorithms", "Bioinformatics", "Sequence Analysis"]
prerequisites: ["1D and 2D Dynamic Programming Foundations", "Recursion", "Arrays and Memory Layout"]
related_topics: ["Longest Common Subsequence", "Knapsack Family", "Two Pointers", "String Hashing"]
---

# Edit Distance and Sequence Alignment

> [!NOTE]
> **Edit Distance** measures sequence dissimilarity by quantifying the minimum number of character operations (insertions, deletions, substitutions) required to transform string $A$ into string $B$. Conversely, **Needleman-Wunsch Global Alignment** scores sequence similarity by maximizing matches and penalizing gaps and mismatches. Both operate over 2D prefix lattices $dp[i][j]$ in $O(nm)$ time.
>
> Reference implementations: [C++17 Implementation](../../implementations/cpp/edit_distance_and_alignment.cpp) | [Python Implementation](../../implementations/python/edit_distance_and_alignment.py)

> [!TIP]
> **Duality Between Distance Minimization and Alignment Maximization:**
> - **Levenshtein Edit Distance (Cost Minimization):** Matches cost 0; substitutions cost 1; insertions and deletions cost 1. Objective: $\min(dp[i-1][j]+1, dp[i][j-1]+1, dp[i-1][j-1] + \text{cost})$.
> - **Needleman-Wunsch Alignment (Score Maximization):** Matches yield $+s$; mismatches penalize $-p$; gaps penalize $-g$. Objective: $\max(dp[i-1][j]-g, dp[i][j-1]-g, dp[i-1][j-1] + \text{score})$.
> - **Space Optimization:** Since row $i$ depends strictly on row $i-1$, both admit two-row rolling array optimizations reducing space from $O(nm)$ to $O(m)$.

> [!WARNING]
> **Reconstruction vs. Space Optimization Trade-off:**
> While two-row rolling arrays compute the scalar distance/score in $O(m)$ auxiliary space, recovering the full edit transcript or aligned strings requires storing either the full $(n+1) \times (m+1)$ table or using divide-and-conquer checkpointing (**Hirschberg's Algorithm**, which achieves $O(nm)$ time and $O(m)$ space).

Edit distance and sequence alignment are among the most important applications of dynamic programming.

They ask a natural question:

> How similar are two sequences, and how can we transform or align one with the other?

These problems appear in many areas:

- spell checking
- version comparison
- DNA and protein analysis
- plagiarism detection
- natural language processing
- error correction
- diff tools

This chapter studies two central formulations:

- **Levenshtein distance**, which measures the minimum number of edits needed to transform one string into another
- **Needleman-Wunsch global alignment**, which scores a full alignment between two sequences

These topics are closely related.

Both use:

- 2D dynamic programming on sequence prefixes
- local transitions from neighboring states
- back-tracing to recover the actual edit script or alignment

We will also discuss:

- rolling-array space optimization
- alignment versus distance viewpoints
- related sequence metrics such as Hamming distance and longest common subsequence

---

## 1. Problem motivation

Suppose we have two strings:

```text
kitten
sitting
```

They are clearly similar, but not identical.

A natural question is:

- how many edits are needed to change one into the other?
- which edits are those?
- how should the characters be aligned?

Dynamic programming gives a clean way to answer all of these.

---

## 2. Sequence comparison as dynamic programming

Sequence comparison problems often have the same structure:

- compare prefixes of the two sequences
- define a state on those prefixes
- decide whether the last step is:
  - match
  - substitution
  - insertion
  - deletion

This leads naturally to a 2D DP table.

A typical state is:

$$
dp[i][j] = \text{best answer for the first } i \text{ characters of one sequence and the first } j \text{ characters of the other}
$$

This prefix-based viewpoint is the key pattern.

---

## 3. Levenshtein distance

The **Levenshtein distance** between two strings is the minimum number of single-character edits needed to transform one string into the other.

The allowed edits are:

- **insert**
- **delete**
- **substitute**

A match has cost 0.

A substitution has cost 1 if the characters differ.

This is one of the standard definitions of edit distance.

---

## 4. Formal problem statement for edit distance

Given strings $ a $ and $ b $, compute the minimum number of operations required to transform $ a $ into $ b $, where each operation is one of:

- insert one character
- delete one character
- substitute one character for another

The answer is a nonnegative integer.

### Example

```text
kitten -> sitting
```

One optimal sequence is:

- substitute `k` with `s`
- substitute `e` with `i`
- insert `g`

So the Levenshtein distance is:

$$
3
$$

---

## 5. State definition for Levenshtein distance

Let:

$$
dp[i][j] = \text{minimum edit distance between } a[0 \dots i-1] \text{ and } b[0 \dots j-1]
$$

In other words:

- the first $ i $ characters of $ a $
- the first $ j $ characters of $ b $

This is the natural 2D prefix state.

---

## 6. Base cases for edit distance

If one string is empty, the only way to transform it into the other is by repeated insertions or deletions.

So:

$$
dp[0][j] = j
$$

because we must insert $ j $ characters.

And:

$$
dp[i][0] = i
$$

because we must delete $ i $ characters.

Also:

$$
dp[0][0] = 0
$$

These base cases define the top row and left column.

---

## 7. Transition for Levenshtein distance

To compute $ dp[i][j] $, consider the last step.

### Case 1: delete from $ a $
Delete the last character of the current prefix of $ a $:

$$
dp[i-1][j] + 1
$$

### Case 2: insert into $ a $
Insert the last character of the current prefix of $ b $:

$$
dp[i][j-1] + 1
$$

### Case 3: match or substitute
Align $ a[i-1] $ with $ b[j-1] $:

$$
dp[i-1][j-1] + cost
$$

where:

$$
cost =
\begin{cases}
0 & \text{if } a[i-1] = b[j-1] \\
1 & \text{otherwise}
\end{cases}
$$

So the recurrence is:

$$
dp[i][j] =
\min
\left(
dp[i-1][j] + 1,\;
dp[i][j-1] + 1,\;
dp[i-1][j-1] + cost
\right)
$$

---

## 8. Why the recurrence is correct

Any optimal transformation from the first $ i $ characters of $ a $ to the first $ j $ characters of $ b $ must end in exactly one of these ways:

- delete the final character of the current $ a $-prefix
- insert the final character needed for the current $ b $-prefix
- align the two final characters, either as a match or substitution

These three cases cover all possibilities, so taking the minimum gives the optimal answer.

This is a standard example of optimal substructure.

---

## 9. C++17 reference implementation of Levenshtein distance

```cpp
#include <vector>
#include <string>
#include <algorithm>

int levenshtein_distance(const std::string& a, const std::string& b) {
    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 0; i <= n; ++i) dp[i][0] = i;
    for (int j = 0; j <= m; ++j) dp[0][j] = j;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            });
        }
    }

    return dp[n][m];
}
```

---

## 10. Python reference implementation of Levenshtein distance

```python
def levenshtein_distance(a, b):
    n, m = len(a), len(b)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(n + 1):
        dp[i][0] = i
    for j in range(m + 1):
        dp[0][j] = j

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            cost = 0 if a[i - 1] == b[j - 1] else 1
            dp[i][j] = min(
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            )

    return dp[n][m]
```

---

## 11. Worked edit distance example

Consider:

```text
a = horse
b = ros
```

One optimal transformation is:

- delete `h`
- substitute `r` for `o`
- delete `e`

So the distance is:

$$
3
$$

The DP table systematically checks all shorter prefix pairs and builds up the optimal answer.

---

## 12. Full edit transcript reconstruction

Computing the minimum distance is useful, but often we also want the actual sequence of edit operations.

This can be reconstructed by back-tracing from $ dp[n][m] $ to $ dp[0][0] $.

At each state $ (i, j) $, we decide which transition produced the optimal value:

- delete
- insert
- match
- substitute

Then we reverse the collected operations.

---

## 13. A simple edit operation format

A practical operation record might store:

- operation type
- source character if relevant
- target character if relevant

For example:

```text
match k
substitute e -> i
delete t
insert g
```

The exact format can vary, but the back-trace idea is the same.

---

## 14. C++17 edit transcript reconstruction

```cpp
#include <vector>
#include <string>
#include <algorithm>

struct EditOp {
    std::string type;
    char from_char;
    char to_char;
};

std::pair<int, std::vector<EditOp>> levenshtein_with_ops(
    const std::string& a,
    const std::string& b) {

    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 0; i <= n; ++i) dp[i][0] = i;
    for (int j = 0; j <= m; ++j) dp[0][j] = j;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            });
        }
    }

    std::vector<EditOp> ops;
    int i = n, j = m;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            if (dp[i][j] == dp[i - 1][j - 1] + cost) {
                if (cost == 0) {
                    ops.push_back({"match", a[i - 1], b[j - 1]});
                } else {
                    ops.push_back({"substitute", a[i - 1], b[j - 1]});
                }
                --i;
                --j;
                continue;
            }
        }

        if (i > 0 && dp[i][j] == dp[i - 1][j] + 1) {
            ops.push_back({"delete", a[i - 1], '\0'});
            --i;
        } else {
            ops.push_back({"insert", '\0', b[j - 1]});
            --j;
        }
    }

    std::reverse(ops.begin(), ops.end());
    return {dp[n][m], ops};
}
```

---

## 15. Python edit transcript reconstruction

```python
def levenshtein_with_ops(a, b):
    n, m = len(a), len(b)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(n + 1):
        dp[i][0] = i
    for j in range(m + 1):
        dp[0][j] = j

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            cost = 0 if a[i - 1] == b[j - 1] else 1
            dp[i][j] = min(
                dp[i - 1][j] + 1,
                dp[i][j - 1] + 1,
                dp[i - 1][j - 1] + cost
            )

    ops = []
    i, j = n, m

    while i > 0 or j > 0:
        if i > 0 and j > 0:
            cost = 0 if a[i - 1] == b[j - 1] else 1
            if dp[i][j] == dp[i - 1][j - 1] + cost:
                if cost == 0:
                    ops.append(("match", a[i - 1], b[j - 1]))
                else:
                    ops.append(("substitute", a[i - 1], b[j - 1]))
                i -= 1
                j -= 1
                continue

        if i > 0 and dp[i][j] == dp[i - 1][j] + 1:
            ops.append(("delete", a[i - 1], None))
            i -= 1
        else:
            ops.append(("insert", None, b[j - 1]))
            j -= 1

    ops.reverse()
    return dp[n][m], ops
```

---

## 16. From edit distance to alignment

Edit distance gives a cost.

Alignment gives a structure.

Instead of asking only:

> How many edits are needed?

alignment asks:

> Which symbols should be matched with which, and where should gaps be inserted?

This is especially important in biological sequence analysis.

---

## 17. What is sequence alignment?

A **sequence alignment** places two sequences in rows, allowing gap symbols, so that corresponding positions can be compared.

### Example

```text
G-ATTACA
GCA-TGCU
```

Here the dash `-` represents a gap.

An alignment can reward matches and penalize:

- mismatches
- gaps

Different scoring systems create different optimal alignments.

---

## 18. Global alignment and Needleman-Wunsch

**Needleman-Wunsch** is the classical dynamic programming algorithm for **global alignment**.

Global alignment means:

- align the full length of both sequences
- from beginning to end

This is appropriate when the sequences are believed to be globally related.

The algorithm is based on the same prefix-DP structure as edit distance.

---

## 19. Scoring model for Needleman-Wunsch

A simple global alignment score uses:

- match reward
- mismatch penalty
- gap penalty

For example:

- match = $ +1 $
- mismatch = $ -1 $
- gap = $ -1 $

The exact scoring system depends on the application.

The goal is to **maximize** total alignment score.

This is a key difference from edit distance, which usually **minimizes** cost.

---

## 20. State definition for global alignment

Let:

$$
dp[i][j] = \text{best alignment score for } a[0 \dots i-1] \text{ and } b[0 \dots j-1]
$$

This is again a prefix-based 2D state.

The difference is that now we maximize score rather than minimize edit count.

---

## 21. Base cases for Needleman-Wunsch

If one sequence prefix is aligned against an empty sequence, the only option is a chain of gaps.

So:

$$
dp[i][0] = i \cdot gap
$$

and:

$$
dp[0][j] = j \cdot gap
$$

Also:

$$
dp[0][0] = 0
$$

These initialize the first row and column.

---

## 22. Transition for Needleman-Wunsch

At state $ (i, j) $, the final alignment step must be one of:

### A. Align $ a[i-1] $ with $ b[j-1] $
This gives:

$$
dp[i-1][j-1] + s(a[i-1], b[j-1])
$$

where $ s $ is the match or mismatch score.

### B. Align $ a[i-1] $ with a gap
This gives:

$$
dp[i-1][j] + gap
$$

### C. Align a gap with $ b[j-1] $
This gives:

$$
dp[i][j-1] + gap
$$

So:

$$
dp[i][j] =
\max
\left(
dp[i-1][j-1] + s(a[i-1], b[j-1]),
\;
dp[i-1][j] + gap,
\;
dp[i][j-1] + gap
\right)
$$

---

## 23. Why Needleman-Wunsch works

An optimal global alignment of the first $ i $ and $ j $ characters must end in exactly one of these three ways:

- character-character alignment
- character-gap alignment
- gap-character alignment

These cases are complete and mutually meaningful.

So the DP recurrence explores all valid final moves and chooses the highest-scoring one.

---

## 24. C++17 reference implementation of Needleman-Wunsch

```cpp
#include <vector>
#include <string>
#include <algorithm>

int needleman_wunsch_score(
    const std::string& a,
    const std::string& b,
    int match_score = 1,
    int mismatch_penalty = -1,
    int gap_penalty = -1) {

    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 0; i <= n; ++i) dp[i][0] = i * gap_penalty;
    for (int j = 0; j <= m; ++j) dp[0][j] = j * gap_penalty;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int diag = dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? match_score : mismatch_penalty);
            int up = dp[i - 1][j] + gap_penalty;
            int left = dp[i][j - 1] + gap_penalty;
            dp[i][j] = std::max({diag, up, left});
        }
    }

    return dp[n][m];
}
```

---

## 25. Python reference implementation of Needleman-Wunsch

```python
def needleman_wunsch_score(a, b, match_score=1, mismatch_penalty=-1, gap_penalty=-1):
    n, m = len(a), len(b)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(n + 1):
        dp[i][0] = i * gap_penalty
    for j in range(m + 1):
        dp[0][j] = j * gap_penalty

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            diag = dp[i - 1][j - 1] + (match_score if a[i - 1] == b[j - 1] else mismatch_penalty)
            up = dp[i - 1][j] + gap_penalty
            left = dp[i][j - 1] + gap_penalty
            dp[i][j] = max(diag, up, left)

    return dp[n][m]
```

---

## 26. Alignment back-trace

Just as with edit distance, we often want the actual alignment, not only the score.

We reconstruct it by walking backward from $ dp[n][m] $.

At each state:

- if we came diagonally, align the two characters
- if we came from above, align a character in $ a $ with a gap
- if we came from the left, align a gap with a character in $ b $

Then reverse the collected aligned symbols.

---

## 27. C++17 global alignment reconstruction

```cpp
#include <vector>
#include <string>
#include <algorithm>
#include <utility>

struct AlignmentResult {
    int score;
    std::string aligned_a;
    std::string aligned_b;
};

AlignmentResult needleman_wunsch_align(
    const std::string& a,
    const std::string& b,
    int match_score = 1,
    int mismatch_penalty = -1,
    int gap_penalty = -1) {

    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<std::vector<int>> dp(n + 1, std::vector<int>(m + 1, 0));

    for (int i = 0; i <= n; ++i) dp[i][0] = i * gap_penalty;
    for (int j = 0; j <= m; ++j) dp[0][j] = j * gap_penalty;

    for (int i = 1; i <= n; ++i) {
        for (int j = 1; j <= m; ++j) {
            int diag = dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? match_score : mismatch_penalty);
            int up = dp[i - 1][j] + gap_penalty;
            int left = dp[i][j - 1] + gap_penalty;
            dp[i][j] = std::max({diag, up, left});
        }
    }

    std::string aligned_a, aligned_b;
    int i = n, j = m;

    while (i > 0 || j > 0) {
        if (i > 0 && j > 0) {
            int diag = dp[i - 1][j - 1] + (a[i - 1] == b[j - 1] ? match_score : mismatch_penalty);
            if (dp[i][j] == diag) {
                aligned_a.push_back(a[i - 1]);
                aligned_b.push_back(b[j - 1]);
                --i;
                --j;
                continue;
            }
        }

        if (i > 0 && dp[i][j] == dp[i - 1][j] + gap_penalty) {
            aligned_a.push_back(a[i - 1]);
            aligned_b.push_back('-');
            --i;
        } else {
            aligned_a.push_back('-');
            aligned_b.push_back(b[j - 1]);
            --j;
        }
    }

    std::reverse(aligned_a.begin(), aligned_a.end());
    std::reverse(aligned_b.begin(), aligned_b.end());

    return {dp[n][m], aligned_a, aligned_b};
}
```

---

## 28. Python global alignment reconstruction

```python
def needleman_wunsch_align(a, b, match_score=1, mismatch_penalty=-1, gap_penalty=-1):
    n, m = len(a), len(b)
    dp = [[0] * (m + 1) for _ in range(n + 1)]

    for i in range(n + 1):
        dp[i][0] = i * gap_penalty
    for j in range(m + 1):
        dp[0][j] = j * gap_penalty

    for i in range(1, n + 1):
        for j in range(1, m + 1):
            diag = dp[i - 1][j - 1] + (match_score if a[i - 1] == b[j - 1] else mismatch_penalty)
            up = dp[i - 1][j] + gap_penalty
            left = dp[i][j - 1] + gap_penalty
            dp[i][j] = max(diag, up, left)

    aligned_a = []
    aligned_b = []
    i, j = n, m

    while i > 0 or j > 0:
        if i > 0 and j > 0:
            diag = dp[i - 1][j - 1] + (match_score if a[i - 1] == b[j - 1] else mismatch_penalty)
            if dp[i][j] == diag:
                aligned_a.append(a[i - 1])
                aligned_b.append(b[j - 1])
                i -= 1
                j -= 1
                continue

        if i > 0 and dp[i][j] == dp[i - 1][j] + gap_penalty:
            aligned_a.append(a[i - 1])
            aligned_b.append('-')
            i -= 1
        else:
            aligned_a.append('-')
            aligned_b.append(b[j - 1])
            j -= 1

    aligned_a.reverse()
    aligned_b.reverse()
    return dp[n][m], ''.join(aligned_a), ''.join(aligned_b)
```

---

## 29. Edit distance versus global alignment

These problems are closely related, but they are not identical.

### Edit distance
- minimizes number or cost of edit operations
- natural for transformation tasks

### Global alignment
- maximizes alignment score
- natural for sequence comparison

The same 2D DP structure appears in both, but the interpretation changes.

This is a powerful lesson:

> one DP table pattern can support multiple meanings depending on the scoring model

---

## 30. Two-row rolling-array optimization

Both edit distance and alignment use a full $ (n+1) \times (m+1) $ table in the simplest implementation.

But each row depends only on:

- the previous row
- the current row so far

So if we only need the final score or distance, we can reduce memory from:

$$
O(nm)
$$

to:

$$
O(m)
$$

using two rows.

This is a standard rolling-array optimization.

---

## 31. C++17 two-row Levenshtein distance

```cpp
#include <vector>
#include <string>
#include <algorithm>

int levenshtein_distance_rolling(const std::string& a, const std::string& b) {
    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<int> prev(m + 1), cur(m + 1);

    for (int j = 0; j <= m; ++j) prev[j] = j;

    for (int i = 1; i <= n; ++i) {
        cur[0] = i;
        for (int j = 1; j <= m; ++j) {
            int cost = (a[i - 1] == b[j - 1]) ? 0 : 1;
            cur[j] = std::min({
                prev[j] + 1,
                cur[j - 1] + 1,
                prev[j - 1] + cost
            });
        }
        std::swap(prev, cur);
    }

    return prev[m];
}
```

---

## 32. Python two-row Levenshtein distance

```python
def levenshtein_distance_rolling(a, b):
    n, m = len(a), len(b)
    prev = list(range(m + 1))
    cur = [0] * (m + 1)

    for i in range(1, n + 1):
        cur[0] = i
        for j in range(1, m + 1):
            cost = 0 if a[i - 1] == b[j - 1] else 1
            cur[j] = min(
                prev[j] + 1,
                cur[j - 1] + 1,
                prev[j - 1] + cost
            )
        prev, cur = cur, prev

    return prev[m]
```

---

## 33. C++17 two-row Needleman-Wunsch score

```cpp
#include <vector>
#include <string>
#include <algorithm>

int needleman_wunsch_score_rolling(
    const std::string& a,
    const std::string& b,
    int match_score = 1,
    int mismatch_penalty = -1,
    int gap_penalty = -1) {

    int n = static_cast<int>(a.size());
    int m = static_cast<int>(b.size());

    std::vector<int> prev(m + 1), cur(m + 1);

    for (int j = 0; j <= m; ++j) prev[j] = j * gap_penalty;

    for (int i = 1; i <= n; ++i) {
        cur[0] = i * gap_penalty;
        for (int j = 1; j <= m; ++j) {
            int diag = prev[j - 1] + (a[i - 1] == b[j - 1] ? match_score : mismatch_penalty);
            int up = prev[j] + gap_penalty;
            int left = cur[j - 1] + gap_penalty;
            cur[j] = std::max({diag, up, left});
        }
        std::swap(prev, cur);
    }

    return prev[m];
}
```

---

## 34. Python two-row Needleman-Wunsch score

```python
def needleman_wunsch_score_rolling(a, b, match_score=1, mismatch_penalty=-1, gap_penalty=-1):
    n, m = len(a), len(b)
    prev = [j * gap_penalty for j in range(m + 1)]
    cur = [0] * (m + 1)

    for i in range(1, n + 1):
        cur[0] = i * gap_penalty
        for j in range(1, m + 1):
            diag = prev[j - 1] + (match_score if a[i - 1] == b[j - 1] else mismatch_penalty)
            up = prev[j] + gap_penalty
            left = cur[j - 1] + gap_penalty
            cur[j] = max(diag, up, left)
        prev, cur = cur, prev

    return prev[m]
```

---

## 35. Space optimization versus reconstruction

Rolling arrays are excellent when we need only the final score.

But full reconstruction usually needs more information.

### If you want only the distance or score
Two rows are often enough.

### If you want the full transcript or alignment
You usually keep:

- the full table
- or an explicit parent-direction table

This is a common DP trade-off between memory and recoverability.

---

## 36. Related sequence metrics

Many other sequence-comparison measures are related.

### Hamming distance
Counts differing positions, but only for strings of equal length and without insertions or deletions.

### Longest common subsequence
Maximizes preserved subsequence length instead of minimizing edits.

### Smith-Waterman local alignment
Finds the best local matching region instead of forcing a full global alignment.

### Damerau-Levenshtein distance
Adds adjacent transposition as an allowed edit.

These problems are part of the same broader family of sequence DP.

---

## 37. Hamming distance versus edit distance

This distinction is important.

### Hamming distance
- strings must have equal length
- only substitutions are considered
- no insertions or deletions

### Edit distance
- strings may have different lengths
- insertions and deletions are allowed
- more flexible and more general

So Hamming distance is simpler, but edit distance is more expressive.

---

## 38. Longest common subsequence connection

The **Longest Common Subsequence** problem is closely related to edit distance.

LCS asks:

> What is the longest subsequence present in both sequences?

This also uses 2D DP on prefixes.

So once a learner understands edit distance and alignment, LCS becomes much easier to understand.

That makes this chapter a strong bridge to later sequence DP topics.

---

## 39. Worked alignment example

Suppose:

```text
a = GATTACA
b = GCATGCU
```

A global alignment may insert gaps and mismatches to maximize total score.

The exact optimal alignment depends on the scoring system, but the important idea is this:

- the DP table scores every prefix pair
- the back-trace recovers one optimal full alignment

This is the same structure as edit transcript recovery, but expressed as aligned rows.

---

## 40. Complexity summary

If the sequence lengths are $ n $ and $ m $, then:

### Full-table edit distance
- **Time:** $ O(nm) $
- **Space:** $ O(nm) $

### Full-table Needleman-Wunsch
- **Time:** $ O(nm) $
- **Space:** $ O(nm) $

### Rolling-array score-only versions
- **Time:** $ O(nm) $
- **Space:** $ O(m) $

These are standard and practical complexities for sequence DP.

---

## 41. Common mistakes

### Mistake 1: defining the wrong prefix state
Be clear that `dp[i][j]` usually refers to the first `i` and first `j` characters.

### Mistake 2: incorrect base row or base column
These represent alignment against an empty prefix and must be initialized carefully.

### Mistake 3: mixing minimization and maximization viewpoints
Edit distance minimizes cost.
Needleman-Wunsch maximizes score.

### Mistake 4: reconstructing with ambiguous tie handling
There may be multiple optimal transcripts or alignments.
One deterministic tie rule is fine, but it should be consistent.

### Mistake 5: expecting rolling arrays to support easy full reconstruction
They usually do not without extra machinery.

### Mistake 6: confusing edit distance with Hamming distance
Insertions and deletions make edit distance more general.

---

## 42. Comparison table

| Problem | Goal | Allowed operations | DP objective | Typical complexity |
|---|---|---|---|---:|
| Levenshtein distance | transform one string into another | insert, delete, substitute | minimize cost | $ O(nm) $ |
| Needleman-Wunsch | globally align two sequences | match, mismatch, gap | maximize score | $ O(nm) $ |
| Hamming distance | compare equal-length strings | substitution only | count mismatches | $ O(n) $ |
| LCS | maximize common subsequence | skip unmatched characters | maximize length | $ O(nm) $ |

---

## 43. Proof intuition summary

### Levenshtein distance
The last edit must be an insertion, deletion, or match-substitution, so the recurrence covers all possibilities.

### Needleman-Wunsch
The final alignment column must be character-character, character-gap, or gap-character, so the recurrence again covers all possibilities.

Both are classic examples of:

- optimal substructure
- overlapping subproblems
- prefix-based 2D dynamic programming

---

## 44. Summary

Edit distance and sequence alignment are foundational dynamic programming problems on pairs of sequences.

The two central formulations are:

- **Levenshtein distance**, which minimizes edit cost
- **Needleman-Wunsch**, which maximizes global alignment score

Both rely on the same core pattern:

$$
dp[i][j] = \text{best answer for the first } i \text{ characters and the first } j \text{ characters}
$$

Important practical ideas include:

- correct base-row and base-column initialization
- full back-tracing to recover edits or alignments
- rolling-array optimization when only the final score is needed
- understanding the difference between distance and alignment viewpoints

This chapter is a major bridge from introductory DP into richer sequence and bioinformatics-style dynamic programming.

---

## 45. Practice prompts

1. What does the state $dp[i][j]$ mean in edit distance?
2. Why are the base cases $dp[0][j] = j$ and $dp[i][0] = i$?
3. What are the three transition cases in Levenshtein distance?
4. How does Needleman-Wunsch global alignment differ conceptually from Levenshtein edit distance?
5. Why does two-row rolling array optimization reduce space complexity from $O(nm)$ to $O(m)$?
6. Why is full back-tracing incompatible with simple two-row rolling arrays without checkpointing techniques like Hirschberg's algorithm?
7. What is the difference between global alignment (Needleman-Wunsch) and local alignment (Smith-Waterman)?

---

## 46. Suggested next topics

A natural continuation after edit distance and sequence alignment is:

- **Longest Common Subsequence (LCS)**
- **Smith-Waterman Local Sequence Alignment**
- **Hirschberg's Linear-Space Alignment Algorithm**
- **Interval and Matrix Chain Dynamic Programming**
- **Tree Dynamic Programming**
- **Bitmask and State Compression Dynamic Programming**
