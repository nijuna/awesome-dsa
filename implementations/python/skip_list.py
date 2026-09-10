"""
Reference Implementation: Skip List (Probabilistic Alternative to Balanced BST)
Demonstrates multi-level forward pointers, geometric height distribution (p = 0.5),
O(log n) search/insert/delete, and unittests.
"""

from __future__ import annotations
import random
import unittest
from typing import Any, Optional, List, Tuple


class SkipNode:
    """A node in the Skip List containing a key, value, and forward pointers."""

    def __init__(self, key: Any, value: Any, level: int) -> None:
        self.key = key
        self.value = value
        self.forward: List[Optional[SkipNode]] = [None] * level


class SkipList:
    """Probabilistic sorted sequence container with expected O(log n) performance."""

    MAX_LEVEL = 16
    P = 0.5

    def __init__(self) -> None:
        self._head = SkipNode(None, None, self.MAX_LEVEL)
        self._level = 1
        self._size = 0
        self._random = random.Random(1337)

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._size == 0

    def _random_level(self) -> int:
        lvl = 1
        while self._random.random() < self.P and lvl < self.MAX_LEVEL:
            lvl += 1
        return lvl

    def find(self, key: Any) -> Optional[Any]:
        curr = self._head
        for i in range(self._level - 1, -1, -1):
            while curr.forward[i] is not None and curr.forward[i].key < key:
                curr = curr.forward[i]
        curr = curr.forward[0]
        if curr is not None and curr.key == key:
            return curr.value
        return None

    def insert(self, key: Any, value: Any) -> bool:
        """Inserts key-value pair. Returns True if inserted, False if updated."""
        update = [None] * self.MAX_LEVEL
        curr = self._head

        for i in range(self._level - 1, -1, -1):
            while curr.forward[i] is not None and curr.forward[i].key < key:
                curr = curr.forward[i]
            update[i] = curr
        curr = curr.forward[0]

        if curr is not None and curr.key == key:
            curr.value = value
            return False

        new_lvl = self._random_level()
        if new_lvl > self._level:
            for i in range(self._level, new_lvl):
                update[i] = self._head
            self._level = new_lvl

        new_node = SkipNode(key, value, new_lvl)
        for i in range(new_lvl):
            new_node.forward[i] = update[i].forward[i]
            update[i].forward[i] = new_node

        self._size += 1
        return True

    def erase(self, key: Any) -> bool:
        """Removes key from skip list. Returns True if found and erased."""
        update = [None] * self.MAX_LEVEL
        curr = self._head

        for i in range(self._level - 1, -1, -1):
            while curr.forward[i] is not None and curr.forward[i].key < key:
                curr = curr.forward[i]
            update[i] = curr
        curr = curr.forward[0]

        if curr is None or curr.key != key:
            return False

        for i in range(self._level):
            if update[i].forward[i] is not curr:
                break
            update[i].forward[i] = curr.forward[i]

        while self._level > 1 and self._head.forward[self._level - 1] is None:
            self._level -= 1

        self._size -= 1
        return True

    def to_list(self) -> List[Tuple[Any, Any]]:
        res = []
        curr = self._head.forward[0]
        while curr is not None:
            res.append((curr.key, curr.value))
            curr = curr.forward[0]
        return res


class TestSkipList(unittest.TestCase):
    def test_basic_crud(self):
        sl = SkipList()
        self.assertTrue(sl.is_empty())
        self.assertEqual(len(sl), 0)

        self.assertTrue(sl.insert(10, "ten"))
        self.assertTrue(sl.insert(20, "twenty"))
        self.assertTrue(sl.insert(5, "five"))
        self.assertTrue(sl.insert(15, "fifteen"))
        self.assertEqual(len(sl), 4)

        self.assertEqual(sl.find(10), "ten")
        self.assertEqual(sl.find(20), "twenty")
        self.assertEqual(sl.find(5), "five")
        self.assertEqual(sl.find(15), "fifteen")
        self.assertIsNone(sl.find(999))

        # Update
        self.assertFalse(sl.insert(10, "TEN_NEW"))
        self.assertEqual(sl.find(10), "TEN_NEW")
        self.assertEqual(len(sl), 4)

        # Sorted traversal check
        self.assertEqual(sl.to_list(), [(5, "five"), (10, "TEN_NEW"), (15, "fifteen"), (20, "twenty")])

        # Delete
        self.assertTrue(sl.erase(15))
        self.assertEqual(len(sl), 3)
        self.assertIsNone(sl.find(15))
        self.assertFalse(sl.erase(999))


if __name__ == "__main__":
    unittest.main()
