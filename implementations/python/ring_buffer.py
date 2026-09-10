"""
Reference Implementation: Power-of-Two Ring Buffer (Circular Queue)
Demonstrates bitwise index masking (index & mask), monotonic sequence counters,
rejection vs overwrite policies, and unittests.
"""

from __future__ import annotations
import unittest
from typing import Any, Optional, List


def next_power_of_two(n: int) -> int:
    """Returns the smallest power of two greater than or equal to n."""
    if n <= 1:
        return 1
    n -= 1
    n |= n >> 1
    n |= n >> 2
    n |= n >> 4
    n |= n >> 8
    n |= n >> 16
    n |= n >> 32
    return n + 1


class RingBuffer:
    """Fixed-capacity circular ring buffer with bitwise masking."""

    def __init__(self, requested_capacity: int = 8) -> None:
        if requested_capacity <= 0:
            requested_capacity = 1
        self._capacity = next_power_of_two(requested_capacity)
        self._mask = self._capacity - 1
        self._buffer: List[Any] = [None] * self._capacity
        self._head = 0  # Monotonic read index
        self._tail = 0  # Monotonic write index

    @property
    def capacity(self) -> int:
        return self._capacity

    def __len__(self) -> int:
        return self._tail - self._head

    def is_empty(self) -> bool:
        return self._head == self._tail

    def is_full(self) -> bool:
        return len(self) == self._capacity

    def push(self, item: Any) -> bool:
        """Pushes element into buffer. Returns False if buffer is full."""
        if self.is_full():
            return False
        self._buffer[self._tail & self._mask] = item
        self._tail += 1
        return True

    def push_overwrite(self, item: Any) -> None:
        """Always pushes element, overwriting oldest entry if buffer is full."""
        if self.is_full():
            self._head += 1
        self._buffer[self._tail & self._mask] = item
        self._tail += 1

    def pop(self) -> Optional[Any]:
        """Pops oldest element from buffer. Returns None if empty."""
        if self.is_empty():
            return None
        val = self._buffer[self._head & self._mask]
        self._buffer[self._head & self._mask] = None  # GC dereference
        self._head += 1
        return val

    def peek(self) -> Optional[Any]:
        """Inspects oldest element without removing it."""
        if self.is_empty():
            return None
        return self._buffer[self._head & self._mask]

    def clear(self) -> None:
        self._buffer = [None] * self._capacity
        self._head = 0
        self._tail = 0


class TestRingBuffer(unittest.TestCase):
    def test_power_of_two_rounding(self):
        rb = RingBuffer(6)
        self.assertEqual(rb.capacity, 8)
        self.assertTrue(rb.is_empty())
        self.assertEqual(len(rb), 0)

    def test_push_pop_basic(self):
        rb = RingBuffer(4)
        for i in range(1, 5):
            self.assertTrue(rb.push(i * 10))

        self.assertTrue(rb.is_full())
        self.assertFalse(rb.push(50))  # Full, rejected

        self.assertEqual(rb.peek(), 10)
        self.assertEqual(rb.pop(), 10)
        self.assertEqual(rb.pop(), 20)
        self.assertEqual(len(rb), 2)

        # Wraparound push
        self.assertTrue(rb.push(50))
        self.assertTrue(rb.push(60))
        self.assertTrue(rb.is_full())

        self.assertEqual(rb.pop(), 30)
        self.assertEqual(rb.pop(), 40)
        self.assertEqual(rb.pop(), 50)
        self.assertEqual(rb.pop(), 60)
        self.assertTrue(rb.is_empty())
        self.assertIsNone(rb.pop())

    def test_push_overwrite(self):
        rb = RingBuffer(4)
        for x in [1, 2, 3, 4]:
            rb.push(x)

        self.assertTrue(rb.is_full())
        rb.push_overwrite(99)  # Drops 1
        self.assertEqual(len(rb), 4)
        self.assertEqual(rb.peek(), 2)

        drained = []
        while not rb.is_empty():
            drained.append(rb.pop())
        self.assertEqual(drained, [2, 3, 4, 99])


if __name__ == "__main__":
    unittest.main()
