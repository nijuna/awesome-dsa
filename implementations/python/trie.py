"""
Reference Implementation: Prefix Trie with Autocomplete & Longest Prefix Match
Demonstrates character-by-character prefix descent, O(L) exact search,
starts_with queries, longest prefix matching (LPM for routing),
autocomplete enumeration, and pruning deletion.

Language: Python 3
"""

from typing import Dict, List, Optional
import unittest


class TrieNode:
    def __init__(self):
        self.is_terminal: bool = False
        self.children: Dict[str, "TrieNode"] = {}


class Trie:
    def __init__(self):
        self._root = TrieNode()
        self._size = 0

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._size == 0

    def insert(self, word: str) -> None:
        curr = self._root
        for ch in word:
            if ch not in curr.children:
                curr.children[ch] = TrieNode()
            curr = curr.children[ch]
        if not curr.is_terminal:
            curr.is_terminal = True
            self._size += 1

    def search(self, word: str) -> bool:
        curr = self._root
        for ch in word:
            if ch not in curr.children:
                return False
            curr = curr.children[ch]
        return curr.is_terminal

    def __contains__(self, word: str) -> bool:
        return self.search(word)

    def starts_with(self, prefix: str) -> bool:
        curr = self._root
        for ch in prefix:
            if ch not in curr.children:
                return False
            curr = curr.children[ch]
        return True

    def longest_prefix_match(self, query: str) -> str:
        """Finds the longest stored key that is a prefix of query."""
        curr = self._root
        longest_len = 0
        current_len = 0

        for ch in query:
            if ch not in curr.children:
                break
            curr = curr.children[ch]
            current_len += 1
            if curr.is_terminal:
                longest_len = current_len

        return query[:longest_len]

    def erase(self, word: str) -> bool:
        def _erase_helper(curr: TrieNode, depth: int) -> Tuple[bool, bool]:
            # Returns (erased, should_delete_child)
            if depth == len(word):
                if not curr.is_terminal:
                    return False, False
                curr.is_terminal = False
                return True, len(curr.children) == 0

            ch = word[depth]
            if ch not in curr.children:
                return False, False

            child = curr.children[ch]
            erased, should_delete_child = _erase_helper(child, depth + 1)
            if should_delete_child:
                del curr.children[ch]

            can_delete_self = not curr.is_terminal and len(curr.children) == 0
            return erased, can_delete_self

        from typing import Tuple
        erased, _ = _erase_helper(self._root, 0)
        if erased:
            self._size -= 1
        return erased

    def words_with_prefix(self, prefix: str) -> List[str]:
        """Autocomplete: Returns all stored words beginning with prefix in sorted order."""
        curr = self._root
        for ch in prefix:
            if ch not in curr.children:
                return []
            curr = curr.children[ch]

        results = []

        def _dfs(node: TrieNode, path: List[str]):
            if node.is_terminal:
                results.append("".join(path))
            for ch in sorted(node.children.keys()):
                path.append(ch)
                _dfs(node.children[ch], path)
                path.pop()

        _dfs(curr, list(prefix))
        return results


class TestTrie(unittest.TestCase):
    def test_trie_basic_operations(self):
        trie = Trie()
        self.assertEqual(len(trie), 0)
        self.assertTrue(trie.is_empty())

        words = ["to", "tea", "ten", "ted", "in", "inn"]
        for w in words:
            trie.insert(w)

        self.assertEqual(len(trie), 6)
        self.assertFalse(trie.is_empty())

        for w in words:
            self.assertTrue(trie.search(w))
            self.assertTrue(w in trie)

        self.assertFalse(trie.search("t"))
        self.assertFalse(trie.search("te"))
        self.assertFalse(trie.search("toast"))

        # Starts with
        self.assertTrue(trie.starts_with("t"))
        self.assertTrue(trie.starts_with("te"))
        self.assertTrue(trie.starts_with("tea"))
        self.assertTrue(trie.starts_with("in"))
        self.assertFalse(trie.starts_with("ta"))

        # Autocomplete
        self.assertEqual(trie.words_with_prefix("te"), ["tea", "ted", "ten"])
        self.assertEqual(trie.words_with_prefix("in"), ["in", "inn"])
        self.assertEqual(trie.words_with_prefix("xyz"), [])

        # Longest Prefix Match
        trie.insert("192.168.")
        trie.insert("192.168.1.")
        self.assertEqual(trie.longest_prefix_match("192.168.1.100"), "192.168.1.")
        self.assertEqual(trie.longest_prefix_match("192.168.2.50"), "192.168.")
        self.assertEqual(trie.longest_prefix_match("10.0.0.1"), "")

        # Deletion
        self.assertTrue(trie.erase("inn"))
        self.assertFalse(trie.search("inn"))
        self.assertTrue(trie.search("in"))

        self.assertTrue(trie.erase("in"))
        self.assertFalse(trie.search("in"))
        self.assertFalse(trie.starts_with("in"))

        self.assertFalse(trie.erase("absent"))


if __name__ == "__main__":
    unittest.main()
