"""
Log-Structured Merge-Tree (LSM-Tree) Implementation in Python 3.

Provides:
- BloomFilter: Probabilistic filter using double-hashing (FNV-1a based).
- SSTable: Sorted String Table with sparse block index and Bloom filter.
- LSMTree: Complete implementation with MemTable, Write-Ahead Log (WAL),
  deletion tombstones, and RocksDB-style Leveled Compaction.
"""

import bisect
import math
import random
import unittest
from typing import Optional, List, Dict, NamedTuple


class Entry(NamedTuple):
    key: str
    value: str
    is_tombstone: bool = False


class BloomFilter:
    """
    Bloom Filter supporting fast negative lookup checks.
    """
    def __init__(self, expected_keys: int, fpp: float = 0.01):
        if expected_keys <= 0:
            expected_keys = 1
        m = -expected_keys * math.log(fpp) / (math.log(2) ** 2)
        self.num_bits = max(64, int(math.ceil(m)))
        self.num_hashes = max(1, int(round((self.num_bits / expected_keys) * math.log(2))))
        self.bits = [0] * ((self.num_bits + 63) // 64)

    def _hash(self, key: str, seed: int) -> int:
        h = 14695981039346656037 ^ seed
        for c in key.encode('utf-8'):
            h ^= c
            h = (h * 1099511628211) & 0xFFFFFFFFFFFFFFFF
        return h

    def add(self, key: str):
        h1 = self._hash(key, 0)
        h2 = self._hash(key, 0x9e3779b97f4a7c15)
        for i in range(self.num_hashes):
            idx = (h1 + i * h2) % self.num_bits
            self.bits[idx // 64] |= (1 << (idx % 64))

    def may_contain(self, key: str) -> bool:
        h1 = self._hash(key, 0)
        h2 = self._hash(key, 0x9e3779b97f4a7c15)
        for i in range(self.num_hashes):
            idx = (h1 + i * h2) % self.num_bits
            if not ((self.bits[idx // 64] >> (idx % 64)) & 1):
                return False
        return True


class SSTable:
    """
    Immutable Sorted String Table component with sparse block indexing.
    """
    INDEX_INTERVAL = 4

    def __init__(self, entries: List[Entry]):
        assert len(entries) > 0
        self.entries = sorted(entries, key=lambda e: e.key)
        self.min_key = self.entries[0].key
        self.max_key = self.entries[-1].key

        self.bloom = BloomFilter(len(self.entries), 0.01)
        self.sparse_index: List[tuple] = []  # (key, offset)

        for i, e in enumerate(self.entries):
            self.bloom.add(e.key)
            if i % self.INDEX_INTERVAL == 0:
                self.sparse_index.append((e.key, i))

    def overlaps_with(self, low: str, high: str) -> bool:
        return not (self.max_key < low or self.min_key > high)

    def get(self, key: str) -> Optional[Entry]:
        if key < self.min_key or key > self.max_key:
            return None
        if not self.bloom.may_contain(key):
            return None

        keys = [item[0] for item in self.sparse_index]
        idx = bisect.bisect_right(keys, key) - 1
        if idx < 0:
            idx = 0

        start = self.sparse_index[idx][1]
        end = self.sparse_index[idx + 1][1] if idx + 1 < len(self.sparse_index) else len(self.entries)

        for i in range(start, end):
            if self.entries[i].key == key:
                return self.entries[i]
            if self.entries[i].key > key:
                break
        return None


class LSMTree:
    """
    Log-Structured Merge-Tree with Leveled Compaction.
    """
    MEMTABLE_THRESHOLD = 8
    L0_COMPACTION_THRESHOLD = 4
    MAX_LEVELS = 4

    def __init__(self):
        self.memtable: Dict[str, Entry] = {}
        self.levels: List[List[SSTable]] = [[] for _ in range(self.MAX_LEVELS)]
        self.wal: List[str] = []

    def put(self, key: str, value: str):
        self.wal.append(f"PUT:{key}={value}")
        self.memtable[key] = Entry(key, value, False)
        if len(self.memtable) >= self.MEMTABLE_THRESHOLD:
            self.flush()

    def remove(self, key: str):
        self.wal.append(f"DEL:{key}")
        self.memtable[key] = Entry(key, "", True)
        if len(self.memtable) >= self.MEMTABLE_THRESHOLD:
            self.flush()

    def flush(self):
        if not self.memtable:
            return

        entries = [self.memtable[k] for k in sorted(self.memtable.keys())]
        sstable = SSTable(entries)
        self.levels[0].insert(0, sstable)
        self.memtable.clear()
        self.wal.clear()

        if len(self.levels[0]) >= self.L0_COMPACTION_THRESHOLD:
            self._compact_level(0)

    def _compact_level(self, level: int):
        if level >= self.MAX_LEVELS - 1:
            return

        if level == 0:
            tables_to_compact = list(self.levels[0])
            low_key = min(t.min_key for t in tables_to_compact)
            high_key = max(t.max_key for t in tables_to_compact)
        else:
            if not self.levels[level]:
                return
            pick = self.levels[level][0]
            tables_to_compact = [pick]
            low_key = pick.min_key
            high_key = pick.max_key

        next_level = level + 1
        next_overlapping = [t for t in self.levels[next_level] if t.overlaps_with(low_key, high_key)]
        next_retained = [t for t in self.levels[next_level] if not t.overlaps_with(low_key, high_key)]

        merged: Dict[str, Entry] = {}
        for t in next_overlapping:
            for e in t.entries:
                merged[e.key] = e

        for t in reversed(tables_to_compact):
            for e in t.entries:
                merged[e.key] = e

        is_bottom = (next_level == self.MAX_LEVELS - 1)
        final_entries: List[Entry] = []
        for k in sorted(merged.keys()):
            e = merged[k]
            if is_bottom and e.is_tombstone:
                continue
            final_entries.append(e)

        if level == 0:
            self.levels[0].clear()
        else:
            self.levels[level].pop(0)

        new_next_tables: List[SSTable] = []
        if final_entries:
            chunk_size = self.MEMTABLE_THRESHOLD * 2
            for i in range(0, len(final_entries), chunk_size):
                chunk = final_entries[i:i + chunk_size]
                new_next_tables.append(SSTable(chunk))

        next_retained.extend(new_next_tables)
        next_retained.sort(key=lambda t: t.min_key)
        self.levels[next_level] = next_retained

        capacity = self.L0_COMPACTION_THRESHOLD * (2 ** next_level)
        if len(self.levels[next_level]) > capacity and next_level < self.MAX_LEVELS - 1:
            self._compact_level(next_level)

    def get(self, key: str) -> Optional[str]:
        # 1. MemTable
        if key in self.memtable:
            e = self.memtable[key]
            return None if e.is_tombstone else e.value

        # 2. Level 0
        for sstable in self.levels[0]:
            res = sstable.get(key)
            if res is not None:
                return None if res.is_tombstone else res.value

        # 3. Levels 1+
        for lvl in range(1, self.MAX_LEVELS):
            if not self.levels[lvl]:
                continue
            low, high = 0, len(self.levels[lvl]) - 1
            while low <= high:
                mid = (low + high) // 2
                table = self.levels[lvl][mid]
                if table.min_key <= key <= table.max_key:
                    res = table.get(key)
                    if res is not None:
                        return None if res.is_tombstone else res.value
                    break
                elif key < table.min_key:
                    high = mid - 1
                else:
                    low = mid + 1

        return None


class TestLSMTree(unittest.TestCase):
    def test_random_differential(self):
        lsm = LSMTree()
        oracle = {}
        rng = random.Random(42)
        keys = [f"key_{i}" for i in range(100)]

        for step in range(2500):
            op = rng.randint(0, 2)
            k = rng.choice(keys)

            if op == 0:
                v = f"val_{step}"
                lsm.put(k, v)
                oracle[k] = v
            elif op == 1:
                lsm.remove(k)
                oracle.pop(k, None)
            else:
                expected = oracle.get(k)
                actual = lsm.get(k)
                self.assertEqual(actual, expected)

        lsm.flush()
        for k in keys:
            self.assertEqual(lsm.get(k), oracle.get(k))


if __name__ == '__main__':
    unittest.main()
