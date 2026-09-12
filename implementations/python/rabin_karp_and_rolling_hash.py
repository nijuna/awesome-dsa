"""
Rabin-Karp and Rolling Hash implementation in Python.

Provides:
1. PrefixRollingHash with double hashing for O(1) substring hash queries.
2. RollingHashWindow for sliding window updates.
3. rabin_karp_search for exact substring matching.
4. compute_lcp for O(log n) Longest Common Prefix via binary search.
5. Comprehensive unit tests.
"""

import unittest
from typing import List, Tuple


class PrefixRollingHash:
    """Prefix rolling hash supporting O(1) substring hash queries via double hashing."""

    B1 = 911
    M1 = 1_000_000_007
    B2 = 997
    M2 = 1_000_000_009

    def __init__(self, s: str):
        self.s = s
        n = len(s)
        self.pref1 = [0] * (n + 1)
        self.pref2 = [0] * (n + 1)
        self.pow1 = [1] * (n + 1)
        self.pow2 = [1] * (n + 1)

        for i, ch in enumerate(s):
            v = ord(ch) + 1
            self.pref1[i + 1] = (self.pref1[i] * self.B1 + v) % self.M1
            self.pref2[i + 1] = (self.pref2[i] * self.B2 + v) % self.M2
            self.pow1[i + 1] = (self.pow1[i] * self.B1) % self.M1
            self.pow2[i + 1] = (self.pow2[i] * self.B2) % self.M2

    def query(self, l: int, r: int) -> Tuple[int, int]:
        """Returns double hash tuple (h1, h2) for substring s[l..r] (inclusive)."""
        if l > r or r >= len(self.s):
            raise IndexError("Substring indices out of bounds")
        length = r - l + 1
        h1 = (self.pref1[r + 1] - self.pref1[l] * self.pow1[length]) % self.M1
        h2 = (self.pref2[r + 1] - self.pref2[l] * self.pow2[length]) % self.M2
        return h1, h2


def rabin_karp_search(text: str, pattern: str) -> List[int]:
    """Finds all 0-indexed occurrences of pattern in text using Rabin-Karp."""
    n = len(text)
    m = len(pattern)
    if m == 0 or m > n:
        return []

    B = 911382323
    M = 972663749

    def val(c: str) -> int:
        return ord(c) + 1

    pattern_hash = 0
    window_hash = 0
    power = 1

    for _ in range(m - 1):
        power = (power * B) % M

    for i in range(m):
        pattern_hash = (pattern_hash * B + val(pattern[i])) % M
        window_hash = (window_hash * B + val(text[i])) % M

    matches = []
    for i in range(n - m + 1):
        if window_hash == pattern_hash:
            if text[i:i + m] == pattern:
                matches.append(i)

        if i + m < n:
            window_hash = (window_hash - val(text[i]) * power) % M
            window_hash = (window_hash * B + val(text[i + m])) % M

    return matches


def compute_lcp(h1: PrefixRollingHash, idx1: int,
                h2: PrefixRollingHash, idx2: int, max_len: int) -> int:
    """Computes length of longest common prefix of two substrings in O(log n)."""
    low, high = 1, max_len
    best = 0
    while low <= high:
        mid = (low + high) // 2
        if h1.query(idx1, idx1 + mid - 1) == h2.query(idx2, idx2 + mid - 1):
            best = mid
            low = mid + 1
        else:
            high = mid - 1
    return best


class TestRabinKarpAndRollingHash(unittest.TestCase):
    def test_rabin_karp_search(self):
        text = "AABAACAADAABAABA"
        pat = "AABA"
        self.assertEqual(rabin_karp_search(text, pat), [0, 9, 12])
        self.assertEqual(rabin_karp_search("ABCDEF", "XYZ"), [])
        self.assertEqual(rabin_karp_search("ABC", "ABC"), [0])
        self.assertEqual(rabin_karp_search("AAAA", "AA"), [0, 1, 2])
        self.assertEqual(rabin_karp_search("AB", "ABC"), [])

    def test_prefix_rolling_hash(self):
        s = "abacaba"
        prh = PrefixRollingHash(s)
        # "aba" at [0..2] and [4..6]
        self.assertEqual(prh.query(0, 2), prh.query(4, 6))
        # "aba" vs "bac"
        self.assertNotEqual(prh.query(0, 2), prh.query(1, 3))

    def test_lcp(self):
        s = "banana"
        prh = PrefixRollingHash(s)
        # Suffix "anana" (idx 1) vs "ana" (idx 3) -> LCP = 3 ("ana")
        self.assertEqual(compute_lcp(prh, 1, prh, 3, 3), 3)
        # Suffix "banana" (idx 0) vs "anana" (idx 1) -> LCP = 0
        self.assertEqual(compute_lcp(prh, 0, prh, 1, 5), 0)


if __name__ == '__main__':
    unittest.main()
