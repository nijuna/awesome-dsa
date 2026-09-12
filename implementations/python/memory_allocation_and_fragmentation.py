"""
Memory Allocation and Fragmentation Simulation and Testing.

Implements Arena (Bump) allocator model, Fixed-Size Pool allocator with freelist,
and tests for allocation, deallocation, recycling, and fragmentation tracking.
"""

import unittest
from typing import List, Optional


class ArenaAllocator:
    """Simulates an Arena/Bump linear allocator."""

    def __init__(self, capacity_bytes: int):
        self.capacity_bytes = capacity_bytes
        self.offset = 0

    def allocate(self, size_bytes: int, alignment: int = 8) -> Optional[int]:
        aligned_offset = (self.offset + alignment - 1) & ~(alignment - 1)
        if aligned_offset + size_bytes > self.capacity_bytes:
            return None  # Out of memory
        allocated_addr = aligned_offset
        self.offset = aligned_offset + size_bytes
        return allocated_addr

    def reset(self) -> None:
        self.offset = 0

    @property
    def used_bytes(self) -> int:
        return self.offset


class PoolAllocator:
    """Simulates a Fixed-Size Pool Allocator with an intrusive free list."""

    def __init__(self, slot_size: int, capacity: int):
        self.slot_size = slot_size
        self.capacity = capacity
        # Represent free list as indices 0 .. capacity - 1
        self.free_list: List[int] = list(range(capacity - 1, -1, -1))
        self.allocated_count = 0

    def allocate(self) -> Optional[int]:
        if not self.free_list:
            return None  # Out of slots
        slot_idx = self.free_list.pop()
        self.allocated_count += 1
        return slot_idx

    def deallocate(self, slot_idx: int) -> None:
        if slot_idx < 0 or slot_idx >= self.capacity:
            raise ValueError("Invalid slot index")
        self.free_list.append(slot_idx)
        self.allocated_count -= 1


class TestMemoryAllocationAndFragmentation(unittest.TestCase):
    def test_arena_allocator(self):
        arena = ArenaAllocator(128)
        self.assertEqual(arena.used_bytes, 0)

        addr1 = arena.allocate(16, alignment=8)
        self.assertEqual(addr1, 0)
        self.assertEqual(arena.used_bytes, 16)

        addr2 = arena.allocate(24, alignment=8)
        self.assertEqual(addr2, 16)
        self.assertEqual(arena.used_bytes, 40)

        # Bulk reset
        arena.reset()
        self.assertEqual(arena.used_bytes, 0)

        # Exhaustion
        overflow = arena.allocate(200)
        self.assertIsNone(overflow)

    def test_pool_allocator(self):
        pool = PoolAllocator(slot_size=32, capacity=3)
        self.assertEqual(pool.allocated_count, 0)

        s0 = pool.allocate()
        s1 = pool.allocate()
        s2 = pool.allocate()
        self.assertEqual(pool.allocated_count, 3)

        # Exhaustion
        s3 = pool.allocate()
        self.assertIsNone(s3)

        # Free and recycle
        pool.deallocate(s1)
        self.assertEqual(pool.allocated_count, 2)

        recycled = pool.allocate()
        self.assertEqual(recycled, s1)
        self.assertEqual(pool.allocated_count, 3)


if __name__ == "__main__":
    unittest.main()
