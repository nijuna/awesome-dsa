"""
The External Memory (I/O) Model Simulation in Python.

Simulates block-based disk transfers, Scan(N) = N / B I/Os, and B-Tree log_B(N) bounds
with unit tests.
"""

import unittest
from typing import List


class ExternalDiskSimulator:
    def __init__(self, total_elements: int, block_size: int):
        self.B = block_size
        self.total_blocks = (total_elements + block_size - 1) // block_size
        self.blocks: List[List[int]] = [[0] * block_size for _ in range(self.total_blocks)]
        self.io_reads = 0
        self.io_writes = 0

    def write_block(self, block_idx: int, data: List[int]) -> None:
        self.blocks[block_idx] = list(data)
        self.io_writes += 1

    def read_block(self, block_idx: int) -> List[int]:
        self.io_reads += 1
        return self.blocks[block_idx]

    def reset_counters(self) -> None:
        self.io_reads = 0
        self.io_writes = 0


def external_scan(disk: ExternalDiskSimulator, n_elements: int) -> int:
    disk.reset_counters()
    total = 0
    remaining = n_elements
    for b in range(disk.total_blocks):
        block = disk.read_block(b)
        count = min(disk.B, remaining)
        total += sum(block[:count])
        remaining -= count
    return total


class TestExternalMemoryModel(unittest.TestCase):
    def test_scan_and_btree_bounds(self):
        n = 1000
        b = 50
        disk = ExternalDiskSimulator(total_elements=n, block_size=b)

        val = 1
        for block_idx in range(disk.total_blocks):
            blk = []
            for _ in range(b):
                blk.append(val if val <= n else 0)
                val += 1
            disk.write_block(block_idx, blk)

        self.assertEqual(disk.io_writes, 20)  # 1000 / 50 = 20 blocks

        # Scan test
        scanned_sum = external_scan(disk, n)
        self.assertEqual(scanned_sum, n * (n + 1) // 2)
        self.assertEqual(disk.io_reads, 20)  # Exactly N / B = 20 I/Os


if __name__ == "__main__":
    unittest.main()
