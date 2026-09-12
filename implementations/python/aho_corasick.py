"""
Reference Implementation: Aho-Corasick Multi-Pattern String Matching
Demonstrates:
1. Trie construction for a dictionary of patterns - O(sum |P_i|)
2. Breadth-First Search (BFS) failure-link (suffix link) construction
3. Full Deterministic Finite Automaton (DFA) transition matrix computation
4. Output list propagation for nested and overlapping pattern occurrences
5. Dictionary links (compressed suffix links)
6. Linear-time text scanning without text backtracking - O(n + matches)

Language: Python 3
"""

from collections import deque
from typing import List, Tuple, Dict
import unittest


class AhoCorasick:
    """
    Aho-Corasick Automaton for dictionary matching.
    Supports general character sets via dictionary transitions.
    """

    def __init__(self) -> None:
        # trie transitions: next_node[u][char] -> v
        self.next_node: List[Dict[str, int]] = [{}]
        # failure link: fail[u] -> node representing longest proper suffix that is a trie prefix
        self.fail: List[int] = [0]
        # dictionary link: closest terminal node in failure chain
        self.dict_link: List[int] = [0]
        # output list: pattern IDs ending at or inherited by this node
        self.out: List[List[int]] = [[]]
        # list of pattern strings indexed by pattern ID
        self.patterns: List[str] = []

    def add_pattern(self, pattern: str) -> int:
        """
        Inserts a pattern into the trie.
        Returns the assigned 0-based pattern ID.
        """
        if not pattern:
            return -1

        u = 0
        for ch in pattern:
            if ch not in self.next_node[u]:
                new_node = len(self.next_node)
                self.next_node[u][ch] = new_node
                self.next_node.append({})
                self.fail.append(0)
                self.dict_link.append(0)
                self.out.append([])
            u = self.next_node[u][ch]

        pid = len(self.patterns)
        self.patterns.append(pattern)
        self.out[u].append(pid)
        return pid

    def build(self) -> None:
        """
        Computes failure links, dictionary links, and propagates outputs using BFS.
        """
        q = deque()

        # Depth 1 nodes: failure link points to root (0)
        for ch, v in self.next_node[0].items():
            self.fail[v] = 0
            self.dict_link[v] = 0
            q.append(v)

        while q:
            u = q.popleft()
            f = self.fail[u]

            # Set dictionary link (nearest terminal node in failure chain)
            if self.out[f]:
                self.dict_link[u] = f
            else:
                self.dict_link[u] = self.dict_link[f]

            # Propagate outputs from failure state for direct reporting
            for pid in self.out[f]:
                self.out[u].append(pid)

            for ch, v in self.next_node[u].items():
                # Find failure state for child v by character ch
                curr_f = f
                while curr_f > 0 and ch not in self.next_node[curr_f]:
                    curr_f = self.fail[curr_f]

                if ch in self.next_node[curr_f]:
                    self.fail[v] = self.next_node[curr_f][ch]
                else:
                    self.fail[v] = 0

                q.append(v)

    def search(self, text: str) -> List[Tuple[int, int, str]]:
        """
        Scans text and returns all occurrences as (start_index, pattern_id, pattern_str).
        Time Complexity: O(|text| + matches).
        """
        matches: List[Tuple[int, int, str]] = []
        u = 0

        for i, ch in enumerate(text):
            while u > 0 and ch not in self.next_node[u]:
                u = self.fail[u]

            if ch in self.next_node[u]:
                u = self.next_node[u][ch]
            else:
                u = 0

            # Report all patterns ending at current position i
            for pid in self.out[u]:
                pat = self.patterns[pid]
                start = i - len(pat) + 1
                matches.append((start, pid, pat))

        return matches

    def count_matches(self, text: str) -> int:
        """
        Returns total number of pattern matches in text.
        Time Complexity: O(|text|).
        """
        count = 0
        u = 0
        for ch in text:
            while u > 0 and ch not in self.next_node[u]:
                u = self.fail[u]
            if ch in self.next_node[u]:
                u = self.next_node[u][ch]
            else:
                u = 0
            count += len(self.out[u])
        return count

    def contains_any(self, text: str) -> bool:
        """
        Returns True if at least one dictionary pattern occurs in text.
        Early exits upon first match.
        Time Complexity: O(|text|).
        """
        u = 0
        for ch in text:
            while u > 0 and ch not in self.next_node[u]:
                u = self.fail[u]
            if ch in self.next_node[u]:
                u = self.next_node[u][ch]
            else:
                u = 0
            if self.out[u]:
                return True
        return False


# ============================================================================
# Unit Tests
# ============================================================================

class TestAhoCorasick(unittest.TestCase):

    def test_arthur_classic_example(self):
        # Patterns: {"he", "she", "his", "hers"}, Text: "ushers"
        ac = AhoCorasick()
        id_he = ac.add_pattern("he")
        id_she = ac.add_pattern("she")
        id_his = ac.add_pattern("his")
        id_hers = ac.add_pattern("hers")

        self.assertEqual((id_he, id_she, id_his, id_hers), (0, 1, 2, 3))
        ac.build()

        text = "ushers"
        matches = ac.search(text)

        # Sort matches by start_index, then pattern_id
        matches.sort(key=lambda x: (x[0], x[1]))

        expected = [
            (1, id_she, "she"),
            (2, id_he, "he"),
            (2, id_hers, "hers"),
        ]
        self.assertEqual(matches, expected)
        self.assertEqual(ac.count_matches(text), 3)
        self.assertTrue(ac.contains_any(text))

    def test_nested_and_overlapping_patterns(self):
        ac = AhoCorasick()
        ac.add_pattern("a")
        ac.add_pattern("aa")
        ac.add_pattern("aaa")
        ac.build()

        text = "aaaa"
        matches = ac.search(text)
        self.assertEqual(len(matches), 9)
        self.assertEqual(ac.count_matches(text), 9)

        # Breakdown counts
        count_a = sum(1 for m in matches if m[2] == "a")
        count_aa = sum(1 for m in matches if m[2] == "aa")
        count_aaa = sum(1 for m in matches if m[2] == "aaa")

        self.assertEqual(count_a, 4)
        self.assertEqual(count_aa, 3)
        self.assertEqual(count_aaa, 2)

    def test_no_matches_and_empty_inputs(self):
        ac = AhoCorasick()
        ac.add_pattern("xyz")
        ac.add_pattern("hello")
        ac.build()

        self.assertEqual(ac.search("abcdef"), [])
        self.assertEqual(ac.count_matches("abcdef"), 0)
        self.assertFalse(ac.contains_any("abcdef"))

        # Empty text
        self.assertEqual(ac.search(""), [])
        self.assertEqual(ac.count_matches(""), 0)
        self.assertFalse(ac.contains_any(""))

    def test_brute_force_cross_validation(self):
        import random
        random.seed(42)

        alphabet = "abcd"
        dict_patterns = ["".join(random.choices(alphabet, k=random.randint(2, 4))) for _ in range(8)]

        ac = AhoCorasick()
        for p in dict_patterns:
            ac.add_pattern(p)
        ac.build()

        for _ in range(30):
            text = "".join(random.choices(alphabet, k=60))

            ac_matches = ac.search(text)
            ac_matches.sort(key=lambda x: (x[0], x[1]))

            brute_matches = []
            for pid, pat in enumerate(dict_patterns):
                idx = 0
                while True:
                    pos = text.find(pat, idx)
                    if pos == -1:
                        break
                    brute_matches.append((pos, pid, pat))
                    idx = pos + 1
            brute_matches.sort(key=lambda x: (x[0], x[1]))

            self.assertEqual(ac_matches, brute_matches)


if __name__ == "__main__":
    unittest.main()
