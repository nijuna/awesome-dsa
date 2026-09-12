"""
Suffix Automaton (DAWG) in Python.

Implements online linear-time construction with state cloning,
distinct substring count, pattern matching, longest common substring,
and differential oracle testing with unittests.
"""

import random
import unittest
from typing import Dict, List, Optional, Set


class State:
    def __init__(self, length: int = 0, link: int = -1):
        self.len = length
        self.link = link
        self.next: Dict[str, int] = {}
        self.is_clone = False


class SuffixAutomaton:
    """Minimal DFA recognizing all substrings of a string in linear time and space."""

    def __init__(self, text: Optional[str] = None):
        self.st: List[State] = [State(0, -1)]  # Root is index 0
        self.last = 0
        if text:
            for char in text:
                self.extend(char)

    def extend(self, c: str) -> None:
        cur = len(self.st)
        self.st.append(State(self.st[self.last].len + 1))

        p = self.last
        while p != -1 and c not in self.st[p].next:
            self.st[p].next[c] = cur
            p = self.st[p].link

        if p == -1:
            self.st[cur].link = 0
        else:
            q = self.st[p].next[c]
            if self.st[p].len + 1 == self.st[q].len:
                self.st[cur].link = q
            else:
                clone = len(self.st)
                cloned_state = State(self.st[p].len + 1, self.st[q].link)
                cloned_state.next = dict(self.st[q].next)
                cloned_state.is_clone = True
                self.st.append(cloned_state)

                while p != -1 and self.st[p].next.get(c) == q:
                    self.st[p].next[c] = clone
                    p = self.st[p].link

                self.st[q].link = clone
                self.st[cur].link = clone

        self.last = cur

    def contains(self, pattern: str) -> bool:
        """Checks if pattern is a substring in O(|P|) time."""
        curr = 0
        for c in pattern:
            if c not in self.st[curr].next:
                return False
            curr = self.st[curr].next[c]
        return True

    def count_distinct_substrings(self) -> int:
        """Computes total distinct substrings in O(N) time."""
        return sum(self.st[v].len - self.st[self.st[v].link].len for v in range(1, len(self.st)))

    def longest_common_substring(self, other: str) -> str:
        """Finds longest common substring between indexed text and other in O(|other|) time."""
        v = 0
        current_len = 0
        best_len = 0
        best_pos = 0

        for i, c in enumerate(other):
            while v != 0 and c not in self.st[v].next:
                v = self.st[v].link
                current_len = self.st[v].len
            if c in self.st[v].next:
                v = self.st[v].next[c]
                current_len += 1
            if current_len > best_len:
                best_len = current_len
                best_pos = i

        if best_len == 0:
            return ""
        return other[best_pos - best_len + 1 : best_pos + 1]


class TestSuffixAutomaton(unittest.TestCase):
    def test_basic_properties(self):
        sam = SuffixAutomaton("aab")
        # Substrings: "", "a", "aa", "aab", "ab", "b" -> 5 non-empty
        self.assertEqual(sam.count_distinct_substrings(), 5)
        self.assertTrue(sam.contains("a"))
        self.assertTrue(sam.contains("aa"))
        self.assertTrue(sam.contains("aab"))
        self.assertTrue(sam.contains("ab"))
        self.assertTrue(sam.contains("b"))
        self.assertFalse(sam.contains("ba"))
        self.assertFalse(sam.contains("aaa"))

    def test_longest_common_substring(self):
        sam = SuffixAutomaton("algorithm_wizardry")
        lcs = sam.longest_common_substring("super_algorithm_engine")
        self.assertEqual(lcs, "algorithm_")

    def test_differential_oracle(self):
        rng = random.Random(42)
        alphabet = "abc"
        for _ in range(50):
            length = rng.randint(1, 20)
            text = "".join(rng.choice(alphabet) for _ in range(length))

            sam = SuffixAutomaton(text)

            # Ground truth oracle
            oracle_subs: Set[str] = set()
            for i in range(len(text)):
                for j in range(i + 1, len(text) + 1):
                    oracle_subs.add(text[i:j])

            self.assertEqual(sam.count_distinct_substrings(), len(oracle_subs))
            for sub in oracle_subs:
                self.assertTrue(sam.contains(sub))

            self.assertFalse(sam.contains(text + "z"))


if __name__ == "__main__":
    unittest.main()
