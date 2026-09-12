"""
Reference Implementation: Suffix Arrays & LCP Array
Demonstrates:
1. Suffix Array construction via Prefix Doubling - O(n log^2 n)
2. Kasai's algorithm for linear-time LCP Array construction - O(n)
3. Binary Search Substring Search & Range Query - O(m log n)
4. Distinct Substrings Counting - O(n) after sa and lcp
5. Longest Repeated Substring extraction - O(n) after sa and lcp

Language: Python 3
"""

from typing import List, Tuple
import unittest


# ============================================================================
# 1. Suffix Array Construction (Prefix Doubling)
# ============================================================================

def build_suffix_array(s: str) -> List[int]:
    """
    Builds the Suffix Array using the Prefix Doubling technique.
    sa[r] is the starting index of the suffix having sorted rank r.
    Time Complexity: O(n log^2 n) (or O(n log n) with counting sort).
    Space Complexity: O(n).
    """
    n = len(s)
    if n == 0:
        return []

    sa = list(range(n))
    rank = [ord(c) for c in s]
    tmp = [0] * n
    length = 1

    while length < n:
        # Sort suffixes according to pair (rank[i], rank[i + length])
        sa.sort(key=lambda i: (rank[i], rank[i + length] if i + length < n else -1))

        tmp[sa[0]] = 0
        for i in range(1, n):
            prev = sa[i - 1]
            curr = sa[i]
            prev_key = (rank[prev], rank[prev + length] if prev + length < n else -1)
            curr_key = (rank[curr], rank[curr + length] if curr + length < n else -1)
            tmp[curr] = tmp[prev] + (1 if prev_key < curr_key else 0)

        rank[:] = tmp
        if rank[sa[-1]] == n - 1:
            break
        length <<= 1

    return sa


# ============================================================================
# 2. Kasai's LCP Array Construction
# ============================================================================

def build_lcp_array(s: str, sa: List[int]) -> List[int]:
    """
    Computes the Longest Common Prefix (LCP) array using Kasai's algorithm.
    lcp[r] = length of the longest common prefix of suffixes sa[r] and sa[r - 1].
    Convention: lcp[0] = 0.
    Time Complexity: O(n). Space Complexity: O(n).
    """
    n = len(s)
    if n == 0:
        return []

    rank = [0] * n
    lcp = [0] * n

    for i, pos in enumerate(sa):
        rank[pos] = i

    h = 0
    for i in range(n):
        r = rank[i]
        if r == 0:
            continue

        j = sa[r - 1]
        while i + h < n and j + h < n and s[i + h] == s[j + h]:
            h += 1

        lcp[r] = h
        if h > 0:
            h -= 1

    return lcp


# ============================================================================
# 3. Substring Search & Range Query via Binary Search
# ============================================================================

def contains_substring(s: str, sa: List[int], pattern: str) -> bool:
    """
    Checks whether pattern occurs as a substring in s via binary search on sa.
    Time Complexity: O(m log n) where m = len(pattern). Space Complexity: O(1).
    """
    n = len(s)
    m = len(pattern)
    if m == 0:
        return True
    if n == 0 or n < m:
        return False

    low, high = 0, n - 1
    while low <= high:
        mid = (low + high) // 2
        start = sa[mid]
        frag = s[start:start + m]

        if frag == pattern:
            return True
        elif frag < pattern:
            low = mid + 1
        else:
            high = mid - 1

    return False


def find_substring_range(s: str, sa: List[int], pattern: str) -> Tuple[int, int]:
    """
    Finds the range [L, R] of indices in the suffix array whose suffixes start with pattern.
    If pattern does not appear, returns (-1, -1).
    Time Complexity: O(m log n). Space Complexity: O(1).
    """
    n = len(s)
    m = len(pattern)
    if m == 0:
        return (0, n - 1)
    if n == 0 or n < m:
        return (-1, -1)

    # Lower bound in sa
    low, high = 0, n - 1
    first_pos = -1
    while low <= high:
        mid = (low + high) // 2
        start = sa[mid]
        frag = s[start:start + m]

        if frag >= pattern:
            if frag == pattern:
                first_pos = mid
            high = mid - 1
        else:
            low = mid + 1

    if first_pos == -1:
        return (-1, -1)

    # Upper bound in sa
    low, high = first_pos, n - 1
    last_pos = first_pos
    while low <= high:
        mid = (low + high) // 2
        start = sa[mid]
        frag = s[start:start + m]

        if frag <= pattern:
            if frag == pattern:
                last_pos = mid
            low = mid + 1
        else:
            high = mid - 1

    return (first_pos, last_pos)


def count_occurrences(s: str, sa: List[int], pattern: str) -> int:
    """
    Counts the number of times pattern appears in string s.
    Time Complexity: O(m log n). Space Complexity: O(1).
    """
    l, r = find_substring_range(s, sa, pattern)
    if l == -1:
        return 0
    return r - l + 1


# ============================================================================
# 4. Distinct Substrings & Longest Repeated Substring
# ============================================================================

def count_distinct_substrings(s: str, sa: List[int], lcp: List[int]) -> int:
    """
    Counts total distinct substrings in string s.
    Formula: sum_{i=0}^{n-1} (n - sa[i]) - sum_{i=0}^{n-1} lcp[i].
    Time Complexity: O(n) after sa and lcp are built.
    """
    n = len(s)
    if n == 0:
        return 0

    total = 0
    for i in range(n):
        total += (n - sa[i]) - lcp[i]

    return total


def longest_repeated_substring(s: str, sa: List[int], lcp: List[int]) -> str:
    """
    Extracts the longest repeated substring in s.
    Finds the maximum value in the LCP array.
    Time Complexity: O(n) after sa and lcp are built.
    """
    n = len(s)
    if n == 0:
        return ""

    best_len = 0
    best_pos = 0

    for i in range(1, n):
        if lcp[i] > best_len:
            best_len = lcp[i]
            best_pos = sa[i]

    return s[best_pos:best_pos + best_len]


# ============================================================================
# Unit Tests
# ============================================================================

class TestSuffixArrays(unittest.TestCase):

    def test_arthur_banana_example(self):
        s = "banana"
        sa = build_suffix_array(s)
        self.assertEqual(sa, [5, 3, 1, 0, 4, 2])

        lcp = build_lcp_array(s, sa)
        self.assertEqual(lcp, [0, 1, 3, 0, 0, 2])

        distinct = count_distinct_substrings(s, sa, lcp)
        self.assertEqual(distinct, 15)

        lrs = longest_repeated_substring(s, sa, lcp)
        self.assertEqual(lrs, "ana")

        # Substring searches
        self.assertTrue(contains_substring(s, sa, "ana"))
        self.assertTrue(contains_substring(s, sa, "banana"))
        self.assertTrue(contains_substring(s, sa, "a"))
        self.assertTrue(contains_substring(s, sa, "nan"))
        self.assertFalse(contains_substring(s, sa, "xyz"))
        self.assertFalse(contains_substring(s, sa, "bananass"))

        self.assertEqual(count_occurrences(s, sa, "a"), 3)
        self.assertEqual(count_occurrences(s, sa, "ana"), 2)
        self.assertEqual(count_occurrences(s, sa, "banana"), 1)
        self.assertEqual(count_occurrences(s, sa, "nan"), 1)
        self.assertEqual(count_occurrences(s, sa, "apple"), 0)

    def test_repeated_characters(self):
        s = "aaaa"
        sa = build_suffix_array(s)
        self.assertEqual(sa, [3, 2, 1, 0])

        lcp = build_lcp_array(s, sa)
        self.assertEqual(lcp, [0, 1, 2, 3])

        self.assertEqual(count_distinct_substrings(s, sa, lcp), 4)
        self.assertEqual(longest_repeated_substring(s, sa, lcp), "aaa")
        self.assertEqual(count_occurrences(s, sa, "aa"), 3)

    def test_distinct_characters(self):
        s = "abcdef"
        sa = build_suffix_array(s)
        self.assertEqual(sa, [0, 1, 2, 3, 4, 5])

        lcp = build_lcp_array(s, sa)
        self.assertEqual(lcp, [0, 0, 0, 0, 0, 0])

        self.assertEqual(count_distinct_substrings(s, sa, lcp), 21)
        self.assertEqual(longest_repeated_substring(s, sa, lcp), "")

    def test_edge_cases(self):
        self.assertEqual(build_suffix_array(""), [])
        self.assertEqual(build_lcp_array("", []), [])
        self.assertEqual(count_distinct_substrings("", [], []), 0)
        self.assertEqual(longest_repeated_substring("", [], []), "")

        single = "z"
        sa_single = build_suffix_array(single)
        self.assertEqual(sa_single, [0])
        lcp_single = build_lcp_array(single, sa_single)
        self.assertEqual(lcp_single, [0])
        self.assertEqual(count_distinct_substrings(single, sa_single, lcp_single), 1)
        self.assertEqual(longest_repeated_substring(single, sa_single, lcp_single), "")

    def test_random_cross_validation(self):
        import random
        random.seed(42)

        for _ in range(30):
            n = random.randint(15, 35)
            s = "".join(random.choices("abcd", k=n))

            sa = build_suffix_array(s)

            # Brute-force suffix sort
            naive_sa = sorted(range(n), key=lambda i: s[i:])
            self.assertEqual(sa, naive_sa)

            lcp = build_lcp_array(s, sa)
            for i in range(1, n):
                h = 0
                p1, p2 = sa[i - 1], sa[i]
                while p1 + h < n and p2 + h < n and s[p1 + h] == s[p2 + h]:
                    h += 1
                self.assertEqual(lcp[i], h)


if __name__ == "__main__":
    unittest.main()
