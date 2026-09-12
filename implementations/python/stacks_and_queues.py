"""
Stacks and Queues

This module provides reference implementations of:
- ArrayStack: LIFO dynamic-array stack
- LinkedStack: LIFO singly-linked list stack
- LinkedQueue: FIFO singly-linked list queue
- CircularQueue: FIFO fixed-capacity ring buffer queue
"""

from typing import Any, Optional
import unittest


class ArrayStack:
    """Dynamic array-backed stack (LIFO)."""
    def __init__(self):
        self._data = []

    def empty(self) -> bool:
        return len(self._data) == 0

    def size(self) -> int:
        return len(self._data)

    def push(self, value: Any) -> None:
        self._data.append(value)

    def pop(self) -> Any:
        if not self._data:
            raise IndexError("Stack underflow")
        return self._data.pop()

    def top(self) -> Any:
        if not self._data:
            raise IndexError("Stack underflow")
        return self._data[-1]


class LinkedStack:
    """Singly-linked list stack (LIFO)."""
    class _Node:
        def __init__(self, value: Any, next_node: Optional['LinkedStack._Node']):
            self.value = value
            self.next = next_node

    def __init__(self):
        self._head: Optional[LinkedStack._Node] = None
        self._size = 0

    def empty(self) -> bool:
        return self._head is None

    def size(self) -> int:
        return self._size

    def push(self, value: Any) -> None:
        self._head = self._Node(value, self._head)
        self._size += 1

    def pop(self) -> Any:
        if self._head is None:
            raise IndexError("Stack underflow")
        val = self._head.value
        self._head = self._head.next
        self._size -= 1
        return val

    def top(self) -> Any:
        if self._head is None:
            raise IndexError("Stack underflow")
        return self._head.value


class LinkedQueue:
    """Singly-linked list queue (FIFO)."""
    class _Node:
        def __init__(self, value: Any):
            self.value = value
            self.next: Optional['LinkedQueue._Node'] = None

    def __init__(self):
        self._head: Optional[LinkedQueue._Node] = None
        self._tail: Optional[LinkedQueue._Node] = None
        self._size = 0

    def empty(self) -> bool:
        return self._head is None

    def size(self) -> int:
        return self._size

    def push(self, value: Any) -> None:
        node = self._Node(value)
        if self._tail:
            self._tail.next = node
        else:
            self._head = node
        self._tail = node
        self._size += 1

    def pop(self) -> Any:
        if self._head is None:
            raise IndexError("Queue underflow")
        val = self._head.value
        self._head = self._head.next
        if self._head is None:
            self._tail = None
        self._size -= 1
        return val

    def front(self) -> Any:
        if self._head is None:
            raise IndexError("Queue underflow")
        return self._head.value


class CircularQueue:
    """Fixed-capacity ring buffer queue (FIFO)."""
    def __init__(self, capacity: int):
        if capacity <= 0:
            raise ValueError("Capacity must be positive")
        self._data = [None] * capacity
        self._head = 0
        self._tail = 0
        self._size = 0
        self._capacity = capacity

    def empty(self) -> bool:
        return self._size == 0

    def full(self) -> bool:
        return self._size == self._capacity

    def size(self) -> int:
        return self._size

    def push(self, value: Any) -> None:
        if self.full():
            raise OverflowError("Queue overflow")
        self._data[self._tail] = value
        self._tail = (self._tail + 1) % self._capacity
        self._size += 1

    def pop(self) -> Any:
        if self.empty():
            raise IndexError("Queue underflow")
        val = self._data[self._head]
        self._data[self._head] = None
        self._head = (self._head + 1) % self._capacity
        self._size -= 1
        return val

    def front(self) -> Any:
        if self.empty():
            raise IndexError("Queue underflow")
        return self._data[self._head]


# ============================================================================
# Unit Tests
# ============================================================================

class TestStacksAndQueues(unittest.TestCase):
    def test_array_stack(self):
        s = ArrayStack()
        self.assertTrue(s.empty())
        s.push(10)
        s.push(20)
        self.assertEqual(s.size(), 2)
        self.assertEqual(s.top(), 20)
        self.assertEqual(s.pop(), 20)
        self.assertEqual(s.pop(), 10)
        self.assertTrue(s.empty())
        with self.assertRaises(IndexError):
            s.pop()

    def test_linked_stack(self):
        s = LinkedStack()
        s.push("a")
        s.push("b")
        self.assertEqual(s.top(), "b")
        self.assertEqual(s.pop(), "b")
        self.assertEqual(s.pop(), "a")
        self.assertTrue(s.empty())

    def test_linked_queue(self):
        q = LinkedQueue()
        self.assertTrue(q.empty())
        q.push(1)
        q.push(2)
        self.assertEqual(q.front(), 1)
        self.assertEqual(q.pop(), 1)
        self.assertEqual(q.front(), 2)
        self.assertEqual(q.pop(), 2)
        self.assertTrue(q.empty())

    def test_circular_queue(self):
        q = CircularQueue(3)
        self.assertTrue(q.empty())
        q.push(100)
        q.push(200)
        q.push(300)
        self.assertTrue(q.full())
        with self.assertRaises(OverflowError):
            q.push(400)

        self.assertEqual(q.pop(), 100)
        q.push(400)
        self.assertTrue(q.full())
        self.assertEqual(q.pop(), 200)
        self.assertEqual(q.pop(), 300)
        self.assertEqual(q.pop(), 400)
        self.assertTrue(q.empty())


if __name__ == "__main__":
    unittest.main()
