"""
Reference Implementation: Doubly Linked List with O(1) Splice
Demonstrates sentinel nodes, pointer manipulation, and O(1) splicing.
"""

from __future__ import annotations
import unittest
from typing import Any, Optional, Iterator


class Node:
    """A doubly linked list node holding data and bidirectional links."""

    def __init__(self, data: Any = None) -> None:
        self.data: Any = data
        self.prev: Optional[Node] = None
        self.next: Optional[Node] = None


class DoublyLinkedList:
    """Doubly Linked List with dummy sentinels and O(1) splice."""

    def __init__(self) -> None:
        self._head_sentinel = Node()
        self._tail_sentinel = Node()
        self._head_sentinel.next = self._tail_sentinel
        self._tail_sentinel.prev = self._head_sentinel
        self._size = 0

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._size == 0

    def push_front(self, value: Any) -> Node:
        return self.insert_before(self._head_sentinel.next, value)

    def push_back(self, value: Any) -> Node:
        return self.insert_before(self._tail_sentinel, value)

    def insert_before(self, pos: Node, value: Any) -> Node:
        new_node = Node(value)
        new_node.prev = pos.prev
        new_node.next = pos
        assert pos.prev is not None
        pos.prev.next = new_node
        pos.prev = new_node
        self._size += 1
        return new_node

    def pop_front(self) -> Any:
        if self.is_empty():
            raise IndexError("pop_front from empty list")
        assert self._head_sentinel.next is not None
        val = self._head_sentinel.next.data
        self.erase(self._head_sentinel.next)
        return val

    def pop_back(self) -> Any:
        if self.is_empty():
            raise IndexError("pop_back from empty list")
        assert self._tail_sentinel.prev is not None
        val = self._tail_sentinel.prev.data
        self.erase(self._tail_sentinel.prev)
        return val

    def erase(self, node: Node) -> None:
        if node is self._head_sentinel or node is self._tail_sentinel:
            return
        assert node.prev is not None and node.next is not None
        node.prev.next = node.next
        node.next.prev = node.prev
        self._size -= 1

    def splice(self, pos: Node, other: DoublyLinkedList, node: Node) -> None:
        """Transfers 'node' from 'other' into 'self' right before 'pos' in O(1)."""
        if node is other._head_sentinel or node is other._tail_sentinel:
            return
        # Detach from other
        assert node.prev is not None and node.next is not None
        node.prev.next = node.next
        node.next.prev = node.prev
        other._size -= 1

        # Attach into self before pos
        assert pos.prev is not None
        node.prev = pos.prev
        node.next = pos
        pos.prev.next = node
        pos.prev = node
        self._size += 1

    def to_list(self) -> list[Any]:
        res = []
        curr = self._head_sentinel.next
        while curr is not self._tail_sentinel:
            assert curr is not None
            res.append(curr.data)
            curr = curr.next
        return res

    def __iter__(self) -> Iterator[Any]:
        curr = self._head_sentinel.next
        while curr is not self._tail_sentinel:
            assert curr is not None
            yield curr.data
            curr = curr.next


class TestDoublyLinkedList(unittest.TestCase):
    def test_basic_operations(self):
        dll = DoublyLinkedList()
        self.assertTrue(dll.is_empty())
        self.assertEqual(len(dll), 0)

        n1 = dll.push_back(10)
        n2 = dll.push_back(20)
        n3 = dll.push_back(30)
        self.assertEqual(len(dll), 3)
        self.assertEqual(dll.to_list(), [10, 20, 30])

        n0 = dll.push_front(5)
        self.assertEqual(dll.to_list(), [5, 10, 20, 30])
        self.assertEqual(len(dll), 4)

        dll.erase(n2)
        self.assertEqual(dll.to_list(), [5, 10, 30])

        self.assertEqual(dll.pop_front(), 5)
        self.assertEqual(dll.pop_back(), 30)
        self.assertEqual(dll.to_list(), [10])

    def test_splice(self):
        l1 = DoublyLinkedList()
        n1 = l1.push_back(1)
        n2 = l1.push_back(3)

        l2 = DoublyLinkedList()
        m1 = l2.push_back(2)
        m2 = l2.push_back(99)

        # Splice m1 from l2 before n2 in l1
        l1.splice(n2, l2, m1)
        self.assertEqual(l1.to_list(), [1, 2, 3])
        self.assertEqual(l2.to_list(), [99])
        self.assertEqual(len(l1), 3)
        self.assertEqual(len(l2), 1)


if __name__ == "__main__":
    unittest.main()
