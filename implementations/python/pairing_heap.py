"""
Reference Implementation: Self-Adjusting Pairing Heap
Demonstrates the two-pass pairing algorithm (Fredman et al., 1986),
O(1) meld, O(1) insert, O(log n) amortized delete_min,
and first-child / next-sibling representation.

Language: Python 3
"""

from typing import Any, Optional, List, Callable
import unittest


class PairingNode:
    def __init__(self, val: Any):
        self.val: Any = val
        self.child: Optional["PairingNode"] = None
        self.sibling: Optional["PairingNode"] = None


class PairingHeap:
    def __init__(self, key: Optional[Callable[[Any], Any]] = None):
        self._root: Optional[PairingNode] = None
        self._size: int = 0
        self._key: Callable[[Any], Any] = key if key is not None else (lambda x: x)

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._root is None

    def _meld_nodes(self, a: Optional[PairingNode], b: Optional[PairingNode]) -> Optional[PairingNode]:
        if not a:
            return b
        if not b:
            return a

        if self._key(b.val) < self._key(a.val):
            a, b = b, a

        # 'b' becomes the leftmost child of 'a'
        b.sibling = a.child
        a.child = b
        return a

    def _two_pass_merge(self, first: Optional[PairingNode]) -> Optional[PairingNode]:
        if not first:
            return None
        if not first.sibling:
            return first

        # Pass 1: Left-to-Right pairing
        pairs = []
        curr = first
        while curr:
            a = curr
            b = curr.sibling
            if b:
                next_node = b.sibling
                a.sibling = None
                b.sibling = None
                pairs.append(self._meld_nodes(a, b))
                curr = next_node
            else:
                a.sibling = None
                pairs.append(a)
                curr = None

        # Pass 2: Right-to-Left accumulation
        result = pairs[-1]
        for i in range(len(pairs) - 2, -1, -1):
            result = self._meld_nodes(pairs[i], result)

        return result

    def peek(self) -> Any:
        if not self._root:
            raise IndexError("peek from empty heap")
        return self._root.val

    def push(self, val: Any) -> None:
        new_node = PairingNode(val)
        self._root = self._meld_nodes(self._root, new_node)
        self._size += 1

    def pop(self) -> Any:
        if not self._root:
            raise IndexError("pop from empty heap")
        top_val = self._root.val
        children = self._root.child
        self._root = self._two_pass_merge(children)
        self._size -= 1
        return top_val

    def meld(self, other: "PairingHeap") -> None:
        if self is other or other.is_empty():
            return
        self._root = self._meld_nodes(self._root, other._root)
        self._size += other._size
        other._root = None
        other._size = 0


class TestPairingHeap(unittest.TestCase):
    def test_basic_operations(self):
        heap = PairingHeap()
        self.assertTrue(heap.is_empty())
        self.assertEqual(len(heap), 0)

        inputs = [42, 17, 93, 8, 31, 5, 64, 22, 11, 75]
        for x in inputs:
            heap.push(x)

        self.assertEqual(len(heap), len(inputs))
        self.assertEqual(heap.peek(), 5)

        sorted_out = []
        while not heap.is_empty():
            sorted_out.append(heap.pop())

        self.assertEqual(sorted_out, sorted(inputs))
        self.assertTrue(heap.is_empty())

    def test_meld(self):
        h1 = PairingHeap()
        for x in [10, 30, 50]:
            h1.push(x)

        h2 = PairingHeap()
        for x in [5, 25, 70]:
            h2.push(x)

        h1.meld(h2)
        self.assertEqual(len(h1), 6)
        self.assertTrue(h2.is_empty())
        self.assertEqual(h1.peek(), 5)

        sorted_out = []
        while not h1.is_empty():
            sorted_out.append(h1.pop())

        self.assertEqual(sorted_out, [5, 10, 25, 30, 50, 70])


if __name__ == "__main__":
    unittest.main()
