"""
Double-Ended Queues (Deques)

This module provides reference implementations of:
- CircularDeque: Fixed-capacity circular buffer deque
- LinkedDeque: Doubly-linked list deque
- sliding_window_max: Monotonic deque algorithm for window maxima
"""

from typing import Any, List, Optional
from collections import deque
import unittest


class CircularDeque:
    """Fixed-capacity circular array deque."""
    def __init__(self, capacity: int):
        if capacity <= 0:
            raise ValueError("Capacity must be positive")
        self._data = [None] * capacity
        self._front = 0
        self._size = 0
        self._capacity = capacity

    def empty(self) -> bool:
        return self._size == 0

    def full(self) -> bool:
        return self._size == self._capacity

    def size(self) -> int:
        return self._size

    def push_front(self, value: Any) -> None:
        if self.full():
            raise OverflowError("Deque overflow")
        self._front = (self._front - 1 + self._capacity) % self._capacity
        self._data[self._front] = value
        self._size += 1

    def push_back(self, value: Any) -> None:
        if self.full():
            raise OverflowError("Deque overflow")
        idx = (self._front + self._size) % self._capacity
        self._data[idx] = value
        self._size += 1

    def pop_front(self) -> Any:
        if self.empty():
            raise IndexError("Deque underflow")
        val = self._data[self._front]
        self._data[self._front] = None
        self._front = (self._front + 1) % self._capacity
        self._size -= 1
        return val

    def pop_back(self) -> Any:
        if self.empty():
            raise IndexError("Deque underflow")
        idx = (self._front + self._size - 1) % self._capacity
        val = self._data[idx]
        self._data[idx] = None
        self._size -= 1
        return val

    def front(self) -> Any:
        if self.empty():
            raise IndexError("Deque underflow")
        return self._data[self._front]

    def back(self) -> Any:
        if self.empty():
            raise IndexError("Deque underflow")
        idx = (self._front + self._size - 1) % self._capacity
        return self._data[idx]


class LinkedDeque:
    """Doubly linked-list deque."""
    class _Node:
        def __init__(self, value: Any):
            self.value = value
            self.prev: Optional['LinkedDeque._Node'] = None
            self.next: Optional['LinkedDeque._Node'] = None

    def __init__(self):
        self._head: Optional[LinkedDeque._Node] = None
        self._tail: Optional[LinkedDeque._Node] = None
        self._size = 0

    def empty(self) -> bool:
        return self._size == 0

    def size(self) -> int:
        return self._size

    def push_front(self, value: Any) -> None:
        node = self._Node(value)
        node.next = self._head
        if self._head:
            self._head.prev = node
        else:
            self._tail = node
        self._head = node
        self._size += 1

    def push_back(self, value: Any) -> None:
        node = self._Node(value)
        node.prev = self._tail
        if self._tail:
            self._tail.next = node
        else:
            self._head = node
        self._tail = node
        self._size += 1

    def pop_front(self) -> Any:
        if self.empty():
            raise IndexError("Deque underflow")
        val = self._head.value
        self._head = self._head.next
        if self._head:
            self._head.prev = None
        else:
            self._tail = None
        self._size -= 1
        return val

    def pop_back(self) -> Any:
        if self.empty():
            raise IndexError("Deque underflow")
        val = self._tail.value
        self._tail = self._tail.prev
        if self._tail:
            self._tail.next = None
        else:
            self._head = None
        self._size -= 1
        return val

    def front(self) -> Any:
        if self.empty():
            raise IndexError("Deque underflow")
        return self._head.value

    def back(self) -> Any:
        if self.empty():
            raise IndexError("Deque underflow")
        return self._tail.value


def sliding_window_max(nums: List[int], k: int) -> List[int]:
    """Computes sliding window maximum using a monotonic deque in O(n) time."""
    dq = deque()
    result = []
    n = len(nums)
    if n == 0 or k <= 0 or k > n:
        return result

    for i, x in enumerate(nums):
        while dq and dq[0] <= i - k:
            dq.popleft()

        while dq and nums[dq[-1]] <= x:
            dq.pop()

        dq.append(i)

        if i >= k - 1:
            result.append(nums[dq[0]])

    return result


# ============================================================================
# Unit Tests
# ============================================================================

class TestDeques(unittest.TestCase):
    def test_circular_deque(self):
        dq = CircularDeque(4)
        self.assertTrue(dq.empty())

        dq.push_back(10)
        dq.push_back(20)
        dq.push_front(5)
        dq.push_front(1)
        self.assertTrue(dq.full())
        self.assertEqual(dq.front(), 1)
        self.assertEqual(dq.back(), 20)

        self.assertEqual(dq.pop_front(), 1)
        self.assertEqual(dq.pop_back(), 20)
        self.assertEqual(dq.front(), 5)
        self.assertEqual(dq.back(), 10)

    def test_linked_deque(self):
        dq = LinkedDeque()
        dq.push_front("mid")
        dq.push_front("start")
        dq.push_back("end")
        self.assertEqual(dq.size(), 3)
        self.assertEqual(dq.front(), "start")
        self.assertEqual(dq.back(), "end")

        self.assertEqual(dq.pop_front(), "start")
        self.assertEqual(dq.pop_back(), "end")
        self.assertEqual(dq.pop_front(), "mid")
        self.assertTrue(dq.empty())

    def test_sliding_window_max(self):
        nums = [1, 3, -1, -3, 5, 3, 6, 7]
        k = 3
        res = sliding_window_max(nums, k)
        self.assertEqual(res, [3, 3, 5, 5, 6, 7])


if __name__ == "__main__":
    unittest.main()
