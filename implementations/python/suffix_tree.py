"""Suffix Tree via Ukkonen's Online O(N) Algorithm.

Implements Arthur's Two-Layer API:
  - Layer 1: Zero-pointer indexed node pool, active-point state machine,
             rule 1 (leaf extensions), rule 2 (edge split), rule 3 (early stop),
             and suffix link traversals.
  - Layer 2: Safe SuffixTreeEngine supporting substring search O(M),
             all occurrence locations O(M + Occ), distinct substring count O(N),
             and differential verification against naive string oracles.
"""

from __future__ import annotations
import random
import unittest
from typing import List, Dict, Optional


GLOBAL_LEAF_END = -1


class Node:
    def __init__(self, start: int, end_idx: int):
        self.start = start
        self.end_idx = end_idx  # -1 for global leaf_end; >= 0 indexes end_values
        self.suffix_link = 0
        self.children: Dict[str, int] = {}
        self.suffix_index = -1


class SuffixTree:
    def __init__(self, text: str):
        self.text = text
        if not self.text or self.text[-1] != '$':
            self.text += '$'

        self.tree: List[Node] = []
        self.end_values: List[int] = []
        self.leaf_end = -1
        self.active_node = 0
        self.active_edge = -1
        self.active_length = 0
        self.remaining_suffix_count = 0

        root_end = self._store_end(-1)
        self.root = self._new_node(-1, root_end)

        for i in range(len(self.text)):
            self._extend(i)

        self._set_suffix_indices(self.root, 0)

    def _get_end(self, end_idx: int) -> int:
        return self.leaf_end if end_idx == GLOBAL_LEAF_END else self.end_values[end_idx]

    def _edge_length(self, node_idx: int) -> int:
        return self._get_end(self.tree[node_idx].end_idx) - self.tree[node_idx].start + 1

    def _new_node(self, start: int, end_idx: int) -> int:
        self.tree.append(Node(start, end_idx))
        return len(self.tree) - 1

    def _store_end(self, val: int) -> int:
        self.end_values.append(val)
        return len(self.end_values) - 1

    def _walk_down(self, curr_node: int) -> bool:
        elen = self._edge_length(curr_node)
        if self.active_length >= elen:
            self.active_edge += elen
            self.active_length -= elen
            self.active_node = curr_node
            return True
        return False

    def _extend(self, phase: int) -> None:
        self.leaf_end = phase
        self.remaining_suffix_count += 1
        last_new_node = -1

        while self.remaining_suffix_count > 0:
            if self.active_length == 0:
                self.active_edge = phase

            ch = self.text[self.active_edge]
            child = self.tree[self.active_node].children.get(ch, -1)

            if child == -1:
                # Rule 2: Leaf creation
                leaf = self._new_node(phase, GLOBAL_LEAF_END)
                self.tree[self.active_node].children[ch] = leaf

                if last_new_node != -1:
                    self.tree[last_new_node].suffix_link = self.active_node
                    last_new_node = -1
            else:
                if self._walk_down(child):
                    continue

                # Rule 3: Character already present along active edge
                next_ch = self.text[self.tree[child].start + self.active_length]
                if next_ch == self.text[phase]:
                    if last_new_node != -1 and self.active_node != self.root:
                        self.tree[last_new_node].suffix_link = self.active_node
                        last_new_node = -1
                    self.active_length += 1
                    break

                # Rule 2: Edge split
                split_end_idx = self._store_end(self.tree[child].start + self.active_length - 1)
                split = self._new_node(self.tree[child].start, split_end_idx)
                self.tree[self.active_node].children[ch] = split

                leaf = self._new_node(phase, GLOBAL_LEAF_END)
                leaf_ch = self.text[phase]
                self.tree[split].children[leaf_ch] = leaf

                self.tree[child].start += self.active_length
                child_ch = self.text[self.tree[child].start]
                self.tree[split].children[child_ch] = child

                if last_new_node != -1:
                    self.tree[last_new_node].suffix_link = split
                last_new_node = split

            self.remaining_suffix_count -= 1
            if self.active_node == self.root and self.active_length > 0:
                self.active_length -= 1
                self.active_edge = phase - self.remaining_suffix_count + 1
            elif self.active_node != self.root:
                self.active_node = self.tree[self.active_node].suffix_link if self.tree[self.active_node].suffix_link > 0 else self.root

    def _set_suffix_indices(self, u: int, label_height: int) -> None:
        if not self.tree[u].children:
            self.tree[u].suffix_index = len(self.text) - label_height
            return
        for child_idx in self.tree[u].children.values():
            self._set_suffix_indices(child_idx, label_height + self._edge_length(child_idx))

    def contains(self, pat: str) -> bool:
        """Tests if pattern is a substring of text in O(M) time."""
        if not pat:
            return True
        curr = self.root
        idx = 0
        m = len(pat)

        while idx < m:
            ch = pat[idx]
            child = self.tree[curr].children.get(ch, -1)
            if child == -1:
                return False

            start = self.tree[child].start
            elen = self._edge_length(child)

            for k in range(elen):
                if idx >= m:
                    break
                if pat[idx] != self.text[start + k]:
                    return False
                idx += 1
            curr = child

        return True

    def locate_all(self, pat: str) -> List[int]:
        """Finds all starting indices of pattern in text in O(M + Occ) time."""
        if not pat:
            return []
        curr = self.root
        idx = 0
        m = len(pat)

        while idx < m:
            ch = pat[idx]
            child = self.tree[curr].children.get(ch, -1)
            if child == -1:
                return []

            start = self.tree[child].start
            elen = self._edge_length(child)

            for k in range(elen):
                if idx >= m:
                    break
                if pat[idx] != self.text[start + k]:
                    return []
                idx += 1
            curr = child

        occurrences: List[int] = []

        def _collect(u: int) -> None:
            if self.tree[u].suffix_index != -1:
                occurrences.append(self.tree[u].suffix_index)
                return
            for v in self.tree[u].children.values():
                _collect(v)

        _collect(curr)
        return sorted(occurrences)

    def count_distinct_substrings(self) -> int:
        """Counts distinct non-empty substrings in O(N) time."""
        def _count_rec(u: int) -> int:
            cnt = 0
            for child in self.tree[u].children.values():
                elen = self._edge_length(child)
                if self._get_end(self.tree[child].end_idx) == len(self.text) - 1:
                    elen = max(0, elen - 1)  # Exclude terminal sentinel '$'
                cnt += elen + _count_rec(child)
            return cnt

        return _count_rec(self.root)


# ============================================================================
# Differential Oracles
# ============================================================================

def naive_locate(text: str, pat: str) -> List[int]:
    res = []
    if not pat or not text:
        return res
    pos = text.find(pat)
    while pos != -1:
        res.append(pos)
        pos = text.find(pat, pos + 1)
    return res


def naive_distinct_substrings(text: str) -> int:
    subs = set()
    n = len(text)
    for i in range(n):
        for j in range(i + 1, n + 1):
            subs.add(text[i:j])
    return len(subs)


# ============================================================================
# Unit & Differential Tests
# ============================================================================

class TestSuffixTree(unittest.TestCase):
    def test_basic_banana(self):
        st = SuffixTree("banana")
        self.assertTrue(st.contains("banana"))
        self.assertTrue(st.contains("anana"))
        self.assertTrue(st.contains("nan"))
        self.assertTrue(st.contains("ana"))
        self.assertTrue(st.contains("a"))
        self.assertFalse(st.contains("band"))
        self.assertFalse(st.contains("ananas"))

        self.assertEqual(st.locate_all("ana"), [1, 3])
        self.assertEqual(st.locate_all("a"), [1, 3, 5])
        self.assertEqual(st.count_distinct_substrings(), 15)

    def test_differential_random(self):
        rng = random.Random(42)
        alphabet = "abc"

        for _ in range(30):
            n = 15
            s = "".join(rng.choice(alphabet) for _ in range(n))
            tree = SuffixTree(s)

            # 1. Distinct substrings count
            self.assertEqual(tree.count_distinct_substrings(), naive_distinct_substrings(s))

            # 2. Pattern search and location
            for length in range(1, 4):
                for i in range(n - length + 1):
                    pat = s[i:i + length]
                    self.assertTrue(tree.contains(pat))
                    self.assertEqual(tree.locate_all(pat), naive_locate(s, pat))


if __name__ == "__main__":
    unittest.main()
