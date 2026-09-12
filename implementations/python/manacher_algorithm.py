"""
Manacher's Algorithm for Linear-Time Palindrome Analysis.

Computes longest palindromic substring, all palindrome radii,
and total palindromic substring count in strictly O(n) time.
"""

import unittest
from typing import List, Tuple


def manacher_radii(s: str) -> List[int]:
    """Computes Manacher's palindrome radii array for transformed string."""
    if not s:
        return []

    t = "^#" + "#".join(s) + "#$"
    n = len(t)
    p = [0] * n
    c = 0
    r = 0

    for i in range(1, n - 1):
        i_mirror = 2 * c - i
        if i < r:
            p[i] = min(r - i, p[i_mirror])

        while t[i + p[i] + 1] == t[i - p[i] - 1]:
            p[i] += 1

        if i + p[i] > r:
            c = i
            r = i + p[i]

    return p


def longest_palindromic_substring(s: str) -> str:
    """Finds the longest palindromic substring in strictly O(n) time."""
    if not s:
        return ""

    p = manacher_radii(s)
    max_len = 0
    center_idx = 0
    for i in range(1, len(p) - 1):
        if p[i] > max_len:
            max_len = p[i]
            center_idx = i

    start = (center_idx - max_len) // 2
    return s[start : start + max_len]


def count_palindromic_substrings(s: str) -> int:
    """Counts total palindromic substrings in s in O(n) time."""
    if not s:
        return 0
    p = manacher_radii(s)
    return sum((radius + 1) // 2 for radius in p[1:-1])


class TestManacherAlgorithm(unittest.TestCase):
    def test_longest_palindrome_odd(self):
        s = "babad"
        lps = longest_palindromic_substring(s)
        self.assertIn(lps, ["bab", "aba"])

    def test_longest_palindrome_even(self):
        s = "cbbd"
        self.assertEqual(longest_palindromic_substring(s), "bb")

    def test_entire_palindrome(self):
        s = "racecar"
        self.assertEqual(longest_palindromic_substring(s), "racecar")
        self.assertEqual(count_palindromic_substrings(s), 10)

    def test_all_identical(self):
        s = "aaaa"
        self.assertEqual(longest_palindromic_substring(s), "aaaa")
        self.assertEqual(count_palindromic_substrings(s), 10)

    def test_single_and_empty(self):
        self.assertEqual(longest_palindromic_substring("a"), "a")
        self.assertEqual(count_palindromic_substrings("a"), 1)
        self.assertEqual(longest_palindromic_substring(""), "")
        self.assertEqual(count_palindromic_substrings(""), 0)


if __name__ == "__main__":
    unittest.main()
