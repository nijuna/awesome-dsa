"""
RAM Model vs Real Machines Simulation and Verification.

Implements cache line spatial locality simulator, contiguous vs scattered access metrics,
and unit tests validating memory hierarchy assumptions.
"""

import unittest
from typing import List, Optional


class ListNode:
    def __init__(self, val: int):
        self.val = val
        self.next: Optional["ListNode"] = None


def sum_contiguous(arr: List[int]) -> int:
    """Simulates linear contiguous memory traversal."""
    total = 0
    for x in arr:
        total += x
    return total


def sum_linked_list(head: Optional[ListNode]) -> int:
    """Simulates pointer chasing traversal."""
    total = 0
    curr = head
    while curr:
        total += curr.val
        curr = curr.next
    return total


class CacheLineSimulator:
    """
    Simulates hardware cache line fetches (default 64-byte lines).
    Tracks cache hits, cache misses, and hit ratio.
    """

    def __init__(self, line_size_bytes: int = 64):
        self.line_size_bytes = line_size_bytes
        self.current_cached_tag: Optional[int] = None
        self.hits = 0
        self.misses = 0

    def access(self, byte_address: int) -> None:
        tag = byte_address // self.line_size_bytes
        if tag == self.current_cached_tag:
            self.hits += 1
        else:
            self.misses += 1
            self.current_cached_tag = tag

    @property
    def hit_ratio(self) -> float:
        total = self.hits + self.misses
        return self.hits / total if total > 0 else 0.0


class TestRAMModelVsRealMachines(unittest.TestCase):
    def test_sum_equivalence(self):
        arr = list(range(1, 1001))
        head = ListNode(arr[0])
        curr = head
        for val in arr[1:]:
            curr.next = ListNode(val)
            curr = curr.next

        self.assertEqual(sum_contiguous(arr), sum_linked_list(head))
        self.assertEqual(sum_contiguous(arr), 1000 * 1001 // 2)

    def test_cache_simulator_contiguous(self):
        # 64 integers of 4 bytes each = 256 bytes = 4 cache lines of 64 bytes
        sim = CacheLineSimulator(line_size_bytes=64)
        for i in range(64):
            sim.access(i * 4)

        self.assertEqual(sim.misses, 4)
        self.assertEqual(sim.hits, 60)
        self.assertEqual(sim.hit_ratio, 60 / 64)

    def test_cache_simulator_strided(self):
        # Strided access jumping by 64 bytes (every access is in a new cache line)
        sim = CacheLineSimulator(line_size_bytes=64)
        for i in range(64):
            sim.access(i * 64)

        self.assertEqual(sim.misses, 64)
        self.assertEqual(sim.hits, 0)
        self.assertEqual(sim.hit_ratio, 0.0)


if __name__ == "__main__":
    unittest.main()
