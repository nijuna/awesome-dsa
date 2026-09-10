"""
Reference Implementation: Open Addressing Hash Map (Linear Probing with Tombstones)
Demonstrates contiguous memory layout, tombstone lifecycle, dynamic rehashing,
and full unittests.
"""

from __future__ import annotations
import unittest
from typing import Any, Optional, List


class _Tombstone:
    """Sentinel representation for deleted slots."""
    pass


DELETED = _Tombstone()


class HashTable:
    """Open addressing hash map with linear probing and tombstones."""

    def __init__(self, initial_capacity: int = 8) -> None:
        self._capacity = max(8, initial_capacity)
        # Power-of-two capacity
        while (self._capacity & (self._capacity - 1)) != 0:
            self._capacity = (self._capacity | (self._capacity - 1)) + 1
        self._mask = self._capacity - 1
        self._keys: List[Any] = [None] * self._capacity
        self._values: List[Any] = [None] * self._capacity
        self._size = 0
        self._occupied_slots = 0  # Includes DELETED tombstones

    def __len__(self) -> int:
        return self._size

    @property
    def capacity(self) -> int:
        return self._capacity

    def is_empty(self) -> bool:
        return self._size == 0

    def _hash(self, key: Any) -> int:
        h = hash(key)
        # SplitMix64 integer mixer for Python integers/hashes
        h ^= (h >> 30) & 0xFFFFFFFFFFFFFFFF
        h = (h * 0xBF58476D1CE4E5B9) & 0xFFFFFFFFFFFFFFFF
        h ^= (h >> 27) & 0xFFFFFFFFFFFFFFFF
        h = (h * 0x94D049BB133111EB) & 0xFFFFFFFFFFFFFFFF
        h ^= (h >> 31) & 0xFFFFFFFFFFFFFFFF
        return h & self._mask

    def _rehash(self, new_capacity: int) -> None:
        old_keys = self._keys
        old_values = self._values

        self._capacity = new_capacity
        self._mask = new_capacity - 1
        self._keys = [None] * self._capacity
        self._values = [None] * self._capacity
        self._size = 0
        self._occupied_slots = 0

        for k, v in zip(old_keys, old_values):
            if k is not None and k is not DELETED:
                self.insert(k, v)

    def insert(self, key: Any, value: Any) -> bool:
        """Inserts or updates key-value pair. Returns True if new key was inserted."""
        if (self._occupied_slots + 1) * 10 >= self._capacity * 7:
            self._rehash(self._capacity * 2)

        idx = self._hash(key)
        first_deleted_idx: Optional[int] = None

        while self._keys[idx] is not None:
            if self._keys[idx] is DELETED:
                if first_deleted_idx is None:
                    first_deleted_idx = idx
            elif self._keys[idx] == key:
                self._values[idx] = value
                return False  # Update
            idx = (idx + 1) & self._mask

        target_slot = first_deleted_idx if first_deleted_idx is not None else idx
        if self._keys[target_slot] is not DELETED:
            self._occupied_slots += 1

        self._keys[target_slot] = key
        self._values[target_slot] = value
        self._size += 1
        return True

    def find(self, key: Any) -> Optional[Any]:
        """Finds value associated with key, or returns None."""
        idx = self._hash(key)
        probes = 0

        while self._keys[idx] is not None and probes < self._capacity:
            if self._keys[idx] is not DELETED and self._keys[idx] == key:
                return self._values[idx]
            idx = (idx + 1) & self._mask
            probes += 1

        return None

    def erase(self, key: Any) -> bool:
        """Removes key by placing a tombstone. Returns True if key was found."""
        idx = self._hash(key)
        probes = 0

        while self._keys[idx] is not None and probes < self._capacity:
            if self._keys[idx] is not DELETED and self._keys[idx] == key:
                self._keys[idx] = DELETED
                self._values[idx] = None
                self._size -= 1
                return True
            idx = (idx + 1) & self._mask
            probes += 1

        return False


class TestHashTable(unittest.TestCase):
    def test_basic_crud(self):
        ht = HashTable(8)
        self.assertTrue(ht.is_empty())
        self.assertEqual(len(ht), 0)

        self.assertTrue(ht.insert("apple", 1))
        self.assertTrue(ht.insert("banana", 2))
        self.assertTrue(ht.insert("cherry", 3))
        self.assertEqual(len(ht), 3)

        self.assertEqual(ht.find("apple"), 1)
        self.assertEqual(ht.find("banana"), 2)
        self.assertEqual(ht.find("cherry"), 3)
        self.assertIsNone(ht.find("durian"))

        # Update
        self.assertFalse(ht.insert("apple", 99))
        self.assertEqual(ht.find("apple"), 99)
        self.assertEqual(len(ht), 3)

    def test_tombstone_lifecycle(self):
        ht = HashTable(8)
        ht.insert("k1", 10)
        ht.insert("k2", 20)
        ht.insert("k3", 30)

        self.assertTrue(ht.erase("k2"))
        self.assertEqual(len(ht), 2)
        self.assertIsNone(ht.find("k2"))
        # k3 must still be found despite tombstone
        self.assertEqual(ht.find("k3"), 30)

        # Re-insert into tombstone
        self.assertTrue(ht.insert("k2", 200))
        self.assertEqual(ht.find("k2"), 200)
        self.assertEqual(len(ht), 3)

    def test_rehash_growth(self):
        ht = HashTable(8)
        for i in range(100):
            ht.insert(f"user_{i}", i * 10)

        self.assertEqual(len(ht), 100)
        self.assertGreaterEqual(ht.capacity, 128)
        for i in range(100):
            self.assertEqual(ht.find(f"user_{i}"), i * 10)


if __name__ == "__main__":
    unittest.main()
