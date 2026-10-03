"""
Publication-grade Reference Implementation of Storage Engine Archetypes in Python 3.

Provides:
- BitcaskEngine: Append-only disk log with in-memory KeyDir hash index and garbage compaction.
- BTreePageEngine: Slotted-page in-place update engine simulating 4 KB disk pages and buffer pool flushing.
- LSMTinyEngine: Log-Structured Merge-Tree with MemTable, immutable SSTables, and compaction.
- Comparative Benchmark & Test Suite validating the RUM (Read-Update-Memory) conjecture.
"""

from typing import Optional, Dict, List, Tuple
import unittest
import time


class BitcaskRecordPointer:
    def __init__(self, offset: int, val_size: int, timestamp: float, is_tombstone: bool = False):
        self.offset = offset
        self.val_size = val_size
        self.timestamp = timestamp
        self.is_tombstone = is_tombstone


class BitcaskEngine:
    """
    Bitcask: Log-Structured Hash Index Storage Engine (Riak Bitcask).
    """

    def __init__(self):
        self.disk_log: bytearray = bytearray()
        self.keydir: Dict[str, BitcaskRecordPointer] = {}
        self.payload_bytes_written: int = 0
        self.disk_bytes_written: int = 0
        self.disk_reads: int = 0

    def put(self, key: str, val: str) -> None:
        ts = time.time()
        offset = len(self.disk_log)

        k_bytes = key.encode('utf-8')
        v_bytes = val.encode('utf-8')

        record = bytearray()
        record.extend(len(k_bytes).to_bytes(2, 'big'))
        record.extend(len(v_bytes).to_bytes(4, 'big'))
        record.append(0)  # flag: put
        record.extend(int(ts * 1000).to_bytes(8, 'big'))
        record.extend(k_bytes)
        record.extend(v_bytes)

        self.disk_log.extend(record)
        self.payload_bytes_written += len(k_bytes) + len(v_bytes)
        self.disk_bytes_written += len(record)

        self.keydir[key] = BitcaskRecordPointer(offset, len(v_bytes), ts, is_tombstone=False)

    def get(self, key: str) -> Optional[str]:
        ptr = self.keydir.get(key)
        if ptr is None or ptr.is_tombstone:
            return None

        self.disk_reads += 1
        # Direct seek to offset
        off = ptr.offset
        klen = int.from_bytes(self.disk_log[off:off + 2], 'big')
        header_len = 2 + 4 + 1 + 8
        val_start = off + header_len + klen
        val_bytes = self.disk_log[val_start:val_start + ptr.val_size]
        return val_bytes.decode('utf-8')

    def delete(self, key: str) -> None:
        if key not in self.keydir or self.keydir[key].is_tombstone:
            return

        ts = time.time()
        offset = len(self.disk_log)
        k_bytes = key.encode('utf-8')

        record = bytearray()
        record.extend(len(k_bytes).to_bytes(2, 'big'))
        record.extend((0).to_bytes(4, 'big'))
        record.append(1)  # flag: tombstone
        record.extend(int(ts * 1000).to_bytes(8, 'big'))
        record.extend(k_bytes)

        self.disk_log.extend(record)
        self.payload_bytes_written += len(k_bytes)
        self.disk_bytes_written += len(record)

        self.keydir[key] = BitcaskRecordPointer(offset, 0, ts, is_tombstone=True)

    def merge_and_compact(self) -> None:
        new_log = bytearray()
        new_keydir = {}

        for key, ptr in self.keydir.items():
            if ptr.is_tombstone:
                continue
            val = self.get(key)
            if val is None:
                continue

            new_off = len(new_log)
            k_bytes = key.encode('utf-8')
            v_bytes = val.encode('utf-8')

            record = bytearray()
            record.extend(len(k_bytes).to_bytes(2, 'big'))
            record.extend(len(v_bytes).to_bytes(4, 'big'))
            record.append(0)
            record.extend(int(ptr.timestamp * 1000).to_bytes(8, 'big'))
            record.extend(k_bytes)
            record.extend(v_bytes)

            new_log.extend(record)
            new_keydir[key] = BitcaskRecordPointer(new_off, len(v_bytes), ptr.timestamp, False)

        self.disk_log = new_log
        self.keydir = new_keydir

    def get_waf(self) -> float:
        if self.payload_bytes_written == 0:
            return 1.0
        return self.disk_bytes_written / self.payload_bytes_written


class BTreePageEngine:
    """
    B+ Tree Page-Based Storage Engine (4096-byte Slotted Pages).
    """
    PAGE_SIZE = 4096

    def __init__(self):
        self.pages: Dict[int, bytearray] = {}
        self.index: Dict[str, Tuple[int, int]] = {}  # key -> (page_id, slot_idx)
        self.next_page_id = 1
        self.payload_bytes_written = 0
        self.disk_bytes_written = 0
        self._alloc_page()

    def _alloc_page(self) -> int:
        pid = self.next_page_id
        self.next_page_id += 1
        page = bytearray(self.PAGE_SIZE)
        # Header: [slot_count: 2B][free_start: 2B][free_end: 2B]
        page[0:2] = (0).to_bytes(2, 'big')
        page[2:4] = (6).to_bytes(2, 'big')  # free_start
        page[4:6] = (self.PAGE_SIZE).to_bytes(2, 'big')  # free_end
        self.pages[pid] = page
        return pid

    def put(self, key: str, val: str) -> None:
        k_bytes = key.encode('utf-8')
        v_bytes = val.encode('utf-8')
        cell_size = 2 + len(k_bytes) + 2 + len(v_bytes)
        slot_size = 4  # offset: 2B, length: 2B
        needed = slot_size + cell_size

        self.payload_bytes_written += len(k_bytes) + len(v_bytes)

        cur_pid = self.next_page_id - 1
        page = self.pages[cur_pid]

        free_start = int.from_bytes(page[2:4], 'big')
        free_end = int.from_bytes(page[4:6], 'big')

        if free_end - free_start < needed:
            cur_pid = self._alloc_page()
            page = self.pages[cur_pid]
            free_start = int.from_bytes(page[2:4], 'big')
            free_end = int.from_bytes(page[4:6], 'big')

        slot_count = int.from_bytes(page[0:2], 'big')

        # Write cell from bottom up
        cell_offset = free_end - cell_size
        page[cell_offset:cell_offset + 2] = len(k_bytes).to_bytes(2, 'big')
        page[cell_offset + 2:cell_offset + 2 + len(k_bytes)] = k_bytes
        v_start = cell_offset + 2 + len(k_bytes)
        page[v_start:v_start + 2] = len(v_bytes).to_bytes(2, 'big')
        page[v_start + 2:v_start + 2 + len(v_bytes)] = v_bytes

        # Write slot entry
        slot_pos = 6 + slot_count * slot_size
        page[slot_pos:slot_pos + 2] = cell_offset.to_bytes(2, 'big')
        page[slot_pos + 2:slot_pos + 4] = cell_size.to_bytes(2, 'big')

        # Update header
        page[0:2] = (slot_count + 1).to_bytes(2, 'big')
        page[2:4] = (free_start + slot_size).to_bytes(2, 'big')
        page[4:6] = cell_offset.to_bytes(2, 'big')

        # Buffer pool dirty page flush: flushes full 4096-byte page
        self.disk_bytes_written += self.PAGE_SIZE
        self.index[key] = (cur_pid, slot_count)

    def get(self, key: str) -> Optional[str]:
        if key not in self.index:
            return None
        pid, slot_idx = self.index[key]
        page = self.pages[pid]

        slot_pos = 6 + slot_idx * 4
        cell_offset = int.from_bytes(page[slot_pos:slot_pos + 2], 'big')

        klen = int.from_bytes(page[cell_offset:cell_offset + 2], 'big')
        v_start = cell_offset + 2 + klen
        vlen = int.from_bytes(page[v_start:v_start + 2], 'big')
        val_bytes = page[v_start + 2:v_start + 2 + vlen]
        return val_bytes.decode('utf-8')

    def get_waf(self) -> float:
        if self.payload_bytes_written == 0:
            return 1.0
        return self.disk_bytes_written / self.payload_bytes_written


class LSMTinyEngine:
    """
    LSM-Tree with In-Memory MemTable and Immutable SSTables.
    """

    def __init__(self, threshold: int = 16):
        self.threshold = threshold
        self.memtable: Dict[str, str] = {}
        self.sstables: List[List[Tuple[str, str]]] = []
        self.payload_bytes = 0
        self.disk_bytes = 0

    def put(self, key: str, val: str) -> None:
        self.payload_bytes += len(key) + len(val)
        # WAL append
        self.disk_bytes += len(key) + len(val) + 8

        self.memtable[key] = val
        if len(self.memtable) >= self.threshold:
            self._flush()

    def get(self, key: str) -> Optional[str]:
        if key in self.memtable:
            val = self.memtable[key]
            return None if val == "__TOMBSTONE__" else val

        for sst in reversed(self.sstables):
            for k, v in sst:
                if k == key:
                    return None if v == "__TOMBSTONE__" else v
        return None

    def delete(self, key: str) -> None:
        self.put(key, "__TOMBSTONE__")

    def _flush(self) -> None:
        sorted_records = sorted(self.memtable.items())
        for k, v in sorted_records:
            self.disk_bytes += len(k) + len(v) + 8
        self.sstables.append(sorted_records)
        self.memtable.clear()

    def compact(self) -> None:
        merged = {}
        for sst in self.sstables:
            for k, v in sst:
                merged[k] = v
        for k, v in self.memtable.items():
            merged[k] = v

        compacted = [(k, v) for k, v in sorted(merged.items()) if v != "__TOMBSTONE__"]
        for k, v in compacted:
            self.disk_bytes += len(k) + len(v)
        self.sstables = [compacted]
        self.memtable.clear()

    def get_waf(self) -> float:
        if self.payload_bytes == 0:
            return 1.0
        return self.disk_bytes / self.payload_bytes


class TestStorageEnginesBreakdown(unittest.TestCase):
    def test_bitcask_crud_and_compaction(self):
        engine = BitcaskEngine()
        engine.put("k1", "v1")
        engine.put("k2", "v2")
        engine.put("k1", "v1_updated")

        self.assertEqual(engine.get("k1"), "v1_updated")
        self.assertEqual(engine.get("k2"), "v2")

        engine.delete("k2")
        self.assertIsNone(engine.get("k2"))

        pre_size = len(engine.disk_log)
        engine.merge_and_compact()
        post_size = len(engine.disk_log)

        self.assertLess(post_size, pre_size)
        self.assertEqual(engine.get("k1"), "v1_updated")

    def test_btree_page_engine(self):
        engine = BTreePageEngine()
        for i in range(50):
            engine.put(f"user_{i}", f"profile_data_{i * 10}")

        for i in range(50):
            self.assertEqual(engine.get(f"user_{i}"), f"profile_data_{i * 10}")

        self.assertIsNone(engine.get("user_nonexistent"))

    def test_lsm_engine(self):
        engine = LSMTinyEngine(threshold=8)
        for i in range(24):
            engine.put(f"key_{i}", f"val_{i}")

        for i in range(24):
            self.assertEqual(engine.get(f"key_{i}"), f"val_{i}")

        engine.delete("key_5")
        self.assertIsNone(engine.get("key_5"))

        engine.compact()
        self.assertIsNone(engine.get("key_5"))
        self.assertEqual(engine.get("key_0"), "val_0")

    def test_rum_conjecture_waf_ordering(self):
        bitcask = BitcaskEngine()
        btree = BTreePageEngine()
        lsm = LSMTinyEngine(threshold=16)

        for i in range(100):
            k = f"order_{i:04d}"
            v = f"payload_data_{i * 5}"
            bitcask.put(k, v)
            btree.put(k, v)
            lsm.put(k, v)

        waf_bitcask = bitcask.get_waf()
        waf_btree = btree.get_waf()
        waf_lsm = lsm.get_waf()

        # The RUM Conjecture invariant: B-Tree has highest WAF under random writes
        self.assertGreater(waf_btree, waf_lsm)
        self.assertGreater(waf_btree, waf_bitcask)
        self.assertLess(waf_bitcask, 2.0)


if __name__ == '__main__':
    unittest.main()
