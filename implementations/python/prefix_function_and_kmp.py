"""
Reference Implementation: Prefix Function and Knuth-Morris-Pratt (KMP)
Demonstrates:
1. Linear-time prefix function (pi table / border array) computation - O(m)
2. KMP exact string matching with non-backtracking text scan - O(n + m)
3. Overlapping match reporting
4. Smallest period detection (string compression factorization)
5. All border lengths extraction via failure-link chain
6. Prefix occurrence frequency counting across the entire string
7. KMP Deterministic Finite Automaton (DFA) state-transition construction

Language: Python 3
"""

from typing import List
import unittest


# ============================================================================
# 1. Prefix Function (Pi Table)
# ============================================================================

def compute_prefix_function(s: str) -> List[int]:
    """
    Computes the prefix function (pi table / border array) for string s.
    pi[i] is the length of the longest proper prefix of s[0..i] that is also a suffix.
    Time Complexity: O(m) amortized. Space Complexity: O(m).
    """
    n = len(s)
    pi = [0] * n

    for i in range(1, n):
        j = pi[i - 1]

        # Fall back through borders of borders
        while j > 0 and s[i] != s[j]:
            j = pi[j - 1]

        if s[i] == s[j]:
            j += 1

        pi[i] = j

    return pi


# ============================================================================
# 2. KMP Exact String Matching
# ============================================================================

def kmp_search(text: str, pattern: str) -> List[int]:
    """
    Finds all 0-based starting indices where pattern occurs in text.
    Convention: if pattern is empty, returns an empty list.
    Time Complexity: O(n + m). Space Complexity: O(m).
    """
    if not pattern or not text or len(text) < len(pattern):
        return []

    pi = compute_prefix_function(pattern)
    matches: List[int] = []
    j = 0
    m = len(pattern)

    for i, ch in enumerate(text):
        while j > 0 and ch != pattern[j]:
            j = pi[j - 1]

        if ch == pattern[j]:
            j += 1

        if j == m:
            matches.append(i - m + 1)
            # Fallback to allow overlapping matches
            j = pi[j - 1]

    return matches


# ============================================================================
# 3. String Periodicity & Borders
# ============================================================================

def smallest_period(s: str) -> int:
    """
    Computes the length of the smallest period of string s.
    If s is composed of repeated copies of a prefix of length p, returns p.
    Otherwise, returns len(s).
    Time Complexity: O(m). Space Complexity: O(m).
    """
    if not s:
        return 0
    pi = compute_prefix_function(s)
    n = len(s)
    p = n - pi[-1]
    return p if n % p == 0 else n


def all_borders(s: str) -> List[int]:
    """
    Extracts all proper border lengths of string s in strictly decreasing order.
    Follows failure link chain: pi[n-1], pi[pi[n-1]-1], ... down to > 0.
    Time Complexity: O(m). Space Complexity: O(m).
    """
    if not s:
        return []
    pi = compute_prefix_function(s)
    n = len(s)
    borders: List[int] = []

    j = pi[n - 1]
    while j > 0:
        borders.append(j)
        j = pi[j - 1]

    return borders


# ============================================================================
# 4. Counting Occurrences of Each Prefix
# ============================================================================

def count_prefix_occurrences(s: str) -> List[int]:
    """
    Counts how many times each prefix s[0..i] appears as a substring in s.
    Returns list ans of length len(s), where ans[i] is the count for prefix s[0..i].
    Time Complexity: O(m). Space Complexity: O(m).
    """
    n = len(s)
    if n == 0:
        return []

    pi = compute_prefix_function(s)
    count = [0] * (n + 1)

    for x in pi:
        count[x] += 1

    for i in range(n - 1, 0, -1):
        count[pi[i - 1]] += count[i]

    for i in range(1, n + 1):
        count[i] += 1

    return [count[i] for i in range(1, n + 1)]


# ============================================================================
# 5. KMP Finite Automaton
# ============================================================================

def build_kmp_automaton(
    pattern: str, alphabet: str = "abcdefghijklmnopqrstuvwxyz"
) -> List[List[int]]:
    """
    Builds the KMP Deterministic Finite Automaton (DFA).
    aut[state][char_index] -> next_state.
    Time Complexity: O(m * |Sigma|). Space Complexity: O(m * |Sigma|).
    """
    m = len(pattern)
    pi = compute_prefix_function(pattern)
    alpha_size = len(alphabet)
    char_to_idx = {ch: idx for idx, ch in enumerate(alphabet)}

    aut: List[List[int]] = [[0] * alpha_size for _ in range(m + 1)]

    for i in range(m + 1):
        for c, ch in enumerate(alphabet):
            if i > 0 and (i == m or ch != pattern[i]):
                aut[i][c] = aut[pi[i - 1]][c]
            else:
                aut[i][c] = i + (1 if i < m and ch == pattern[i] else 0)

    return aut


# ============================================================================
# Unit Tests
# ============================================================================

class TestKMP(unittest.TestCase):

    def test_prefix_function_examples(self):
        # Arthur's example 1: ababac -> [0, 0, 1, 2, 3, 0]
        self.assertEqual(compute_prefix_function("ababac"), [0, 0, 1, 2, 3, 0])

        # Arthur's example 2: aabaaab -> [0, 1, 0, 1, 2, 2, 3]
        self.assertEqual(compute_prefix_function("aabaaab"), [0, 1, 0, 1, 2, 2, 3])

        # ababa -> [0, 0, 1, 2, 3]
        self.assertEqual(compute_prefix_function("ababa"), [0, 0, 1, 2, 3])

        # Empty & single char
        self.assertEqual(compute_prefix_function(""), [])
        self.assertEqual(compute_prefix_function("a"), [0])

    def test_kmp_search_exact_and_overlapping(self):
        # Standard match
        self.assertEqual(kmp_search("ababcababa", "ababa"), [5])

        # Overlapping matches: "aaaaa" with "aaa" -> [0, 1, 2]
        self.assertEqual(kmp_search("aaaaa", "aaa"), [0, 1, 2])

        # Multiple distinct occurrences
        self.assertEqual(kmp_search("banana", "a"), [1, 3, 5])
        self.assertEqual(kmp_search("banana", "an"), [1, 3])

        # No match
        self.assertEqual(kmp_search("abcdef", "xyz"), [])

    def test_boundary_cases(self):
        self.assertEqual(kmp_search("", "abc"), [])
        self.assertEqual(kmp_search("abc", ""), [])
        self.assertEqual(kmp_search("abc", "abcdef"), [])

    def test_smallest_period_and_borders(self):
        self.assertEqual(smallest_period("abababab"), 2)
        self.assertEqual(smallest_period("abcabcabc"), 3)
        self.assertEqual(smallest_period("abcdef"), 6)
        self.assertEqual(smallest_period(""), 0)

        self.assertEqual(all_borders("ababa"), [3, 1])
        self.assertEqual(all_borders("abcdef"), [])
        self.assertEqual(all_borders(""), [])

    def test_count_prefix_occurrences(self):
        # In "ababa":
        # "a" (len 1) -> 3
        # "ab" (len 2) -> 2
        # "aba" (len 3) -> 2
        # "abab" (len 4) -> 1
        # "ababa" (len 5) -> 1
        self.assertEqual(count_prefix_occurrences("ababa"), [3, 2, 2, 1, 1])
        self.assertEqual(count_prefix_occurrences(""), [])

    def test_automaton_search(self):
        pattern = "ababa"
        text = "ababcabababacababa"
        alphabet = "abc"
        char_to_idx = {ch: idx for idx, ch in enumerate(alphabet)}
        aut = build_kmp_automaton(pattern, alphabet)

        state = 0
        matches = []
        m = len(pattern)
        for i, ch in enumerate(text):
            state = aut[state][char_to_idx[ch]]
            if state == m:
                matches.append(i - m + 1)

        self.assertEqual(matches, kmp_search(text, pattern))


if __name__ == "__main__":
    unittest.main()
