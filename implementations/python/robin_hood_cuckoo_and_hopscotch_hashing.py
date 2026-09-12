"""
Reference implementations of Robin Hood Hashing and Cuckoo Hashing.

Implements:
1. RobinHoodHashMap with PSL displacement swapping, early-exit search, and backward-shift deletion.
2. CuckooHashMap with dual-table eviction chains and deterministic O(1) worst-case lookup.
3. Differential testing against Python dict.
"""

import random
import unittest
from typing import Optional, Any, List


class RobinHoodHashMap:
    """Robin Hood Hash Map with PSL displacement swapping and backward-shift deletion."""

    class Slot:
        __slots__ = ('key', 'val', 'psl', 'occupied')

        def __init__(self):
            self.key = None
            self.val = None
            self.psl = 0
            self.occupied = False

    def __init__(self, initial_capacity: int = 16):
        self.capacity = initial_capacity
        self.table = [self.Slot() for _ in range(self.capacity)]
        self._size = 0
        self.max_load_factor = 0.85

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._size == 0

    def _hash(self, key: Any) -> int:
        return hash(key) & (self.capacity - 1)

    def _rehash(self, new_cap: int) -> None:
        old_table = self.table
        self.capacity = new_cap
        self.table = [self.Slot() for _ in range(self.capacity)]
        self._size = 0

        for slot in old_table:
            if slot.occupied:
                self.insert(slot.key, slot.val)

    def find(self, key: Any) -> Optional[Any]:
        if self.is_empty():
            return None
        idx = self._hash(key)
        current_psl = 0

        while True:
            slot = self.table[idx]
            if not slot.occupied or current_psl > slot.psl:
                # Early exit: key cannot exist beyond this point
                return None
            if slot.key == key:
                return slot.val
            idx = (idx + 1) & (self.capacity - 1)
            current_psl += 1

    def contains(self, key: Any) -> bool:
        return self.find(key) is not None

    def insert(self, key: Any, val: Any) -> None:
        if (self._size + 1) / self.capacity > self.max_load_factor:
            self._rehash(self.capacity * 2)

        idx = self._hash(key)
        current_psl = 0

        while True:
            slot = self.table[idx]
            if not slot.occupied:
                slot.key = key
                slot.val = val
                slot.psl = current_psl
                slot.occupied = True
                self._size += 1
                return

            if slot.key == key:
                slot.val = val
                return

            if current_psl > slot.psl:
                # Swap incoming key with current occupant
                key, slot.key = slot.key, key
                val, slot.val = slot.val, val
                current_psl, slot.psl = slot.psl, current_psl

            idx = (idx + 1) & (self.capacity - 1)
            current_psl += 1

    def erase(self, key: Any) -> bool:
        if self.is_empty():
            return False
        idx = self._hash(key)
        current_psl = 0

        while True:
            slot = self.table[idx]
            if not slot.occupied or current_psl > slot.psl:
                return False
            if slot.key == key:
                # Backward-shift deletion without tombstones
                slot.occupied = False
                self._size -= 1

                curr = idx
                next_idx = (curr + 1) & (self.capacity - 1)

                while self.table[next_idx].occupied and self.table[next_idx].psl > 0:
                    self.table[curr].key = self.table[next_idx].key
                    self.table[curr].val = self.table[next_idx].val
                    self.table[curr].psl = self.table[next_idx].psl - 1
                    self.table[curr].occupied = True

                    self.table[next_idx].occupied = False
                    curr = next_idx
                    next_idx = (curr + 1) & (self.capacity - 1)

                return True

            idx = (idx + 1) & (self.capacity - 1)
            current_psl += 1


class CuckooHashMap:
    """Cuckoo Hash Map with dual tables and deterministic O(1) worst-case lookup."""

    class Entry:
        __slots__ = ('key', 'val', 'occupied')

        def __init__(self):
            self.key = None
            self.val = None
            self.occupied = False

    def __init__(self, initial_capacity: int = 16):
        self.capacity = initial_capacity
        self.t1 = [self.Entry() for _ in range(self.capacity)]
        self.t2 = [self.Entry() for _ in range(self.capacity)]
        self._size = 0
        self.seed1 = 1337
        self.seed2 = 7331
        self.max_loop = 50

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._size == 0

    def _hash1(self, key: Any) -> int:
        return (hash(key) ^ self.seed1) & (self.capacity - 1)

    def _hash2(self, key: Any) -> int:
        return (hash(key) ^ self.seed2) & (self.capacity - 1)

    def _rehash(self, new_cap: int) -> None:
        old1 = self.t1
        old2 = self.t2
        self.capacity = new_cap
        self.t1 = [self.Entry() for _ in range(self.capacity)]
        self.t2 = [self.Entry() for _ in range(self.capacity)]
        self._size = 0
        self.seed1 = (self.seed1 + 0x9e3779b9) & 0xFFFFFFFF
        self.seed2 = (self.seed2 + 0xbf58476d) & 0xFFFFFFFF

        for e in old1:
            if e.occupied:
                self.insert(e.key, e.val)
        for e in old2:
            if e.occupied:
                self.insert(e.key, e.val)

    def find(self, key: Any) -> Optional[Any]:
        if self.is_empty():
            return None
        i1 = self._hash1(key)
        if self.t1[i1].occupied and self.t1[i1].key == key:
            return self.t1[i1].val
        i2 = self._hash2(key)
        if self.t2[i2].occupied and self.t2[i2].key == key:
            return self.t2[i2].val
        return None

    def contains(self, key: Any) -> bool:
        return self.find(key) is not None

    def insert(self, key: Any, val: Any) -> None:
        # Check if key already exists in either table
        i1 = self._hash1(key)
        if self.t1[i1].occupied and self.t1[i1].key == key:
            self.t1[i1].val = val
            return
        i2 = self._hash2(key)
        if self.t2[i2].occupied and self.t2[i2].key == key:
            self.t2[i2].val = val
            return

        if self._size >= self.capacity * 0.5:
            self._rehash(self.capacity * 2)

        curr_key = key
        curr_val = val

        for _ in range(self.max_loop):
            pos1 = self._hash1(curr_key)
            if not self.t1[pos1].occupied:
                self.t1[pos1].key = curr_key
                self.t1[pos1].val = curr_val
                self.t1[pos1].occupied = True
                self._size += 1
                return

            # Evict occupant from T1 to T2
            curr_key, self.t1[pos1].key = self.t1[pos1].key, curr_key
            curr_val, self.t1[pos1].val = self.t1[pos1].val, curr_val

            pos2 = self._hash2(curr_key)
            if not self.t2[pos2].occupied:
                self.t2[pos2].key = curr_key
                self.t2[pos2].val = curr_val
                self.t2[pos2].occupied = True
                self._size += 1
                return

            # Evict occupant from T2 to T1 on next loop
            curr_key, self.t2[pos2].key = self.t2[pos2].key, curr_key
            curr_val, self.t2[pos2].val = self.t2[pos2].val, curr_val

        # Eviction loop limit exceeded: rehash with new seeds
        self._rehash(self.capacity * 2)
        self.insert(curr_key, curr_val)

    def erase(self, key: Any) -> bool:
        if self.is_empty():
            return False
        i1 = self._hash1(key)
        if self.t1[i1].occupied and self.t1[i1].key == key:
            self.t1[i1].occupied = False
            self._size -= 1
            return True
        i2 = self._hash2(key)
        if self.t2[i2].occupied and self.t2[i2].key == key:
            self.t2[i2].occupied = False
            self._size -= 1
            return True
        return False


class TestHashingImplementations(unittest.TestCase):
    def test_basic_operations(self):
        rh = RobinHoodHashMap()
        ck = CuckooHashMap()

        for k in range(10):
            rh.insert(k, k * 10)
            ck.insert(k, k * 10)

        self.assertEqual(len(rh), 10)
        self.assertEqual(len(ck), 10)

        for k in range(10):
            self.assertEqual(rh.find(k), k * 10)
            self.assertEqual(ck.find(k), k * 10)

        self.assertTrue(rh.erase(5))
        self.assertTrue(ck.erase(5))
        self.assertFalse(rh.contains(5))
        self.assertFalse(ck.contains(5))
        self.assertEqual(len(rh), 9)
        self.assertEqual(len(ck), 9)

    def test_differential_fuzzing(self):
        rh = RobinHoodHashMap()
        ck = CuckooHashMap()
        oracle = {}

        rng = random.Random(42)
        for _ in range(1000):
            op = rng.randint(0, 2)
            key = rng.randint(0, 300)

            if op == 0:
                val = key * 13
                rh.insert(key, val)
                ck.insert(key, val)
                oracle[key] = val
            elif op == 1:
                r_rh = rh.erase(key)
                r_ck = ck.erase(key)
                r_or = key in oracle
                if r_or:
                    del oracle[key]
                self.assertEqual(r_rh, r_or)
                self.assertEqual(r_ck, r_or)
            else:
                v_rh = rh.find(key)
                v_ck = ck.find(key)
                v_or = oracle.get(key)
                self.assertEqual(v_rh, v_or)
                self.assertEqual(v_ck, v_or)

            self.assertEqual(len(rh), len(oracle))
            self.assertEqual(len(ck), len(oracle))


if __name__ == '__main__':
    unittest.main()
