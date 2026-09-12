"""
Z-Algorithm and String Borders

This module provides production-grade reference implementations of:
- z_function(s): Computes the Z-array in O(n) amortized time.
- z_search(text, pattern, separator): Exact pattern matching via concatenation.
- border_lengths(s): All proper border lengths of a string.
- smallest_period(s): Smallest exact repetition period.
- count_prefix_occurrences(s): Counts total occurrences of each prefix of length 1..n.
"""

from typing import List
import unittest


def z_function(s: str) -> List[int]:
    """
    Computes the Z-function for a given string s.

    For a string s of length n, z[i] is the length of the longest substring
    starting at index i that matches a prefix of s.
    By standard convention, z[0] = 0.

    Time Complexity: O(n) amortized time, where n = len(s).
    Space Complexity: O(n) to store the result list.
    """
    n = len(s)
    z = [0] * n
    if n == 0:
        return z

    l = 0
    r = 0
    for i in range(1, n):
        if i <= r:
            z[i] = min(r - i + 1, z[i - l])

        while i + z[i] < n and s[z[i]] == s[i + z[i]]:
            z[i] += 1

        if i + z[i] - 1 > r:
            l = i
            r = i + z[i] - 1

    return z


def z_search(text: str, pattern: str, separator: str = "#") -> List[int]:
    """
    Finds all 0-based starting indices of pattern occurrences in text using the Z-algorithm.

    Constructs pattern + separator + text.
    Any position i in the text portion where z[i] >= len(pattern) indicates
    an exact match starting at text index (i - len(pattern) - 1).

    Time Complexity: O(n + m) where n = len(text), m = len(pattern).
    Space Complexity: O(n + m) for combined string and Z-array.
    """
    if not pattern:
        return []
    m = len(pattern)
    n = len(text)
    if m > n:
        return []

    combined = f"{pattern}{separator}{text}"
    z = z_function(combined)
    total_len = len(combined)
    matches = []

    for i in range(m + 1, total_len):
        if z[i] >= m:
            matches.append(i - m - 1)

    return matches


def border_lengths(s: str) -> List[int]:
    """
    Finds all proper border lengths of a string s.

    A border is both a proper prefix and proper suffix of s.
    Using the Z-function, a suffix starting at position i is a border iff i + z[i] == len(s).

    Time Complexity: O(n).
    Space Complexity: O(n).
    """
    n = len(s)
    if n <= 1:
        return []

    z = z_function(s)
    borders = [z[i] for i in range(1, n) if i + z[i] == n]
    borders.sort()
    return borders


def smallest_period(s: str) -> int:
    """
    Finds the smallest exact repetition period of string s.

    A string has period p if characters repeat every p positions and len(s) % p == 0.
    In terms of Z-values, candidate p is a period iff p + z[p] == len(s).

    Time Complexity: O(n).
    Space Complexity: O(n).
    """
    n = len(s)
    if n == 0:
        return 0

    z = z_function(s)
    for p in range(1, n):
        if p + z[p] == n and n % p == 0:
            return p
    return n


def count_prefix_occurrences(s: str) -> List[int]:
    """
    Counts total occurrences of each prefix of length 1..n inside s.

    Returns a list of length n + 1 where ans[len] is the frequency of prefix s[0..len-1].

    Time Complexity: O(n).
    Space Complexity: O(n).
    """
    n = len(s)
    if n == 0:
        return [0]

    z = z_function(s)
    count = [0] * (n + 1)

    for i in range(1, n):
        if z[i] > 0:
            count[z[i]] += 1

    # Suffix sums: a match of length L contributes to all prefix lengths <= L
    for length in range(n - 1, 0, -1):
        count[length] += count[length + 1]

    # Prefix of length 1..n always appears at index 0
    for length in range(1, n + 1):
        count[length] += 1

    return count


# ============================================================================
# Unit Tests
# ============================================================================

class TestZAlgorithm(unittest.TestCase):
    def test_empty_and_single(self):
        self.assertEqual(z_function(""), [])
        self.assertEqual(smallest_period(""), 0)

        self.assertEqual(z_function("a"), [0])
        self.assertEqual(border_lengths("a"), [])
        self.assertEqual(smallest_period("a"), 1)

    def test_canonical_ababa(self):
        s = "ababa"
        self.assertEqual(z_function(s), [0, 0, 3, 0, 1])
        self.assertEqual(border_lengths(s), [1, 3])
        self.assertEqual(smallest_period(s), 5)

        prefix_counts = count_prefix_occurrences(s)
        self.assertEqual(prefix_counts[1], 3)  # "a" appears at 0, 2, 4
        self.assertEqual(prefix_counts[2], 2)  # "ab" appears at 0, 2
        self.assertEqual(prefix_counts[3], 2)  # "aba" appears at 0, 2
        self.assertEqual(prefix_counts[4], 1)  # "abab" appears at 0
        self.assertEqual(prefix_counts[5], 1)  # "ababa" appears at 0

    def test_all_identical_chars(self):
        s = "aaaaa"
        self.assertEqual(z_function(s), [0, 4, 3, 2, 1])
        self.assertEqual(border_lengths(s), [1, 2, 3, 4])
        self.assertEqual(smallest_period(s), 1)

        prefix_counts = count_prefix_occurrences(s)
        self.assertEqual(prefix_counts[1], 5)
        self.assertEqual(prefix_counts[2], 4)
        self.assertEqual(prefix_counts[3], 3)
        self.assertEqual(prefix_counts[4], 2)
        self.assertEqual(prefix_counts[5], 1)

    def test_pattern_matching(self):
        text = "abacaba"
        pattern = "aba"
        self.assertEqual(z_search(text, pattern), [0, 4])

        # Overlapping occurrences
        text = "aaaaa"
        pattern = "aa"
        self.assertEqual(z_search(text, pattern), [0, 1, 2, 3])

        # Pattern not found
        self.assertEqual(z_search("abcdef", "xyz"), [])

        # Pattern longer than text
        self.assertEqual(z_search("abc", "abcdef"), [])

        # Empty pattern
        self.assertEqual(z_search("abc", ""), [])

    def test_periods(self):
        self.assertEqual(smallest_period("abcabcabc"), 3)
        self.assertEqual(smallest_period("abababab"), 2)
        self.assertEqual(smallest_period("abcdef"), 6)
        self.assertEqual(smallest_period("abcab"), 5)

    def test_z_invariants(self):
        s = "aabcaabxaabcaab"
        z = z_function(s)
        n = len(s)
        for i in range(1, n):
            length = z[i]
            if length > 0:
                self.assertEqual(s[:length], s[i:i + length])
            if i + length < n:
                self.assertNotEqual(s[length], s[i + length])


if __name__ == "__main__":
    unittest.main()
