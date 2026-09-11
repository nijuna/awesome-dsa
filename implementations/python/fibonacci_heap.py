"""
Reference Implementation: Fibonacci Heap
Demonstrates lazy consolidation, cascading cuts, circular doubly linked lists,
O(1) amortized insert, meld, and decrease_key, and O(log n) amortized extract-min.

Language: Python 3
"""

import math
from typing import Any, Optional, List, Callable
import unittest


class FibonacciNode:
    def __init__(self, val: Any):
        self.val: Any = val
        self.degree: int = 0
        self.mark: bool = False
        self.parent: Optional["FibonacciNode"] = None
        self.child: Optional["FibonacciNode"] = None
        self.left: "FibonacciNode" = self
        self.right: "FibonacciNode" = self


class FibonacciHeap:
    def __init__(self, key: Optional[Callable[[Any], Any]] = None):
        self._min: Optional[FibonacciNode] = None
        self._size: int = 0
        self._key: Callable[[Any], Any] = key if key is not None else (lambda x: x)

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self._min is None

    def _list_insert_after(self, target: FibonacciNode, node: FibonacciNode) -> None:
        node.right = target.right
        node.left = target
        target.right.left = node
        target.right = node

    def _list_remove(self, node: FibonacciNode) -> None:
        node.left.right = node.right
        node.right.left = node.left

    def _list_concat(self, a: Optional[FibonacciNode], b: Optional[FibonacciNode]) -> Optional[FibonacciNode]:
        if not a:
            return b
        if not b:
            return a

        a_next = a.right
        b_prev = b.left

        a.right = b
        b.left = a
        a_next.left = b_prev
        b_prev.right = a_next

        return b if self._key(b.val) < self._key(a.val) else a

    def _link(self, y: FibonacciNode, x: FibonacciNode) -> None:
        self._list_remove(y)
        y.parent = x
        y.mark = False

        if x.child is None:
            x.child = y
            y.left = y
            y.right = y
        else:
            self._list_insert_after(x.child, y)
        x.degree += 1

    def _consolidate(self) -> None:
        if self._min is None:
            return

        roots: List[FibonacciNode] = []
        curr = self._min
        while True:
            roots.append(curr)
            curr = curr.right
            if curr == self._min:
                break

        max_d = int(math.log2(self._size + 1) * 2.0) + 8
        table: List[Optional[FibonacciNode]] = [None] * max_d

        for w in roots:
            x = w
            d = x.degree
            while d < len(table) and table[d] is not None:
                y = table[d]
                if self._key(y.val) < self._key(x.val):
                    x, y = y, x
                self._link(y, x)
                table[d] = None
                d += 1
                if d >= len(table):
                    table.extend([None] * 8)
            table[d] = x

        self._min = None
        for y in table:
            if y is not None:
                y.left = y
                y.right = y
                y.parent = None
                if self._min is None:
                    self._min = y
                else:
                    self._list_insert_after(self._min, y)
                    if self._key(y.val) < self._key(self._min.val):
                        self._min = y

    def _cut(self, x: FibonacciNode, y: FibonacciNode) -> None:
        if x.right == x:
            y.child = None
        else:
            y.child = x.right
            self._list_remove(x)
        y.degree -= 1

        x.left = x
        x.right = x
        x.parent = None
        x.mark = False
        assert self._min is not None
        self._list_insert_after(self._min, x)

    def _cascading_cut(self, y: FibonacciNode) -> None:
        z = y.parent
        if z is not None:
            if not y.mark:
                y.mark = True
            else:
                self._cut(y, z)
                self._cascading_cut(z)

    def top(self) -> Any:
        if self._min is None:
            raise IndexError("FibonacciHeap.top(): heap is empty")
        return self._min.val

    def push(self, val: Any) -> FibonacciNode:
        node = FibonacciNode(val)
        if self._min is None:
            self._min = node
        else:
            self._list_insert_after(self._min, node)
            if self._key(node.val) < self._key(self._min.val):
                self._min = node
        self._size += 1
        return node

    def pop(self) -> Any:
        if self._min is None:
            raise IndexError("FibonacciHeap.pop(): heap is empty")

        z = self._min
        top_val = z.val

        if z.child is not None:
            children: List[FibonacciNode] = []
            c = z.child
            while True:
                children.append(c)
                c = c.right
                if c == z.child:
                    break

            for child in children:
                child.parent = None
                self._list_insert_after(self._min, child)
            z.child = None

        if z.right == z:
            self._min = None
        else:
            self._min = z.right
            self._list_remove(z)
            self._consolidate()

        self._size -= 1
        return top_val

    def meld(self, other: "FibonacciHeap") -> None:
        if self is other or other.is_empty():
            return

        if self._min is None:
            self._min = other._min
        else:
            self._min = self._list_concat(self._min, other._min)

        self._size += other._size
        other._min = None
        other._size = 0

    def decrease_key(self, x: FibonacciNode, new_val: Any) -> None:
        if self._key(x.val) < self._key(new_val):
            raise ValueError("FibonacciHeap.decrease_key(): new key is not higher priority")

        x.val = new_val
        y = x.parent

        if y is not None and self._key(x.val) < self._key(y.val):
            self._cut(x, y)
            self._cascading_cut(y)

        assert self._min is not None
        if self._key(x.val) < self._key(self._min.val):
            self._min = x


class TestFibonacciHeap(unittest.TestCase):
    def test_basic_operations(self):
        fh = FibonacciHeap()
        self.assertTrue(fh.is_empty())
        self.assertEqual(len(fh), 0)

        inputs = [42, 17, 93, 8, 31, 5, 64, 22, 11, 75]
        for x in inputs:
            fh.push(x)

        self.assertFalse(fh.is_empty())
        self.assertEqual(len(fh), 10)
        self.assertEqual(fh.top(), 5)

        sorted_out = []
        while not fh.is_empty():
            sorted_out.append(fh.pop())

        self.assertEqual(sorted_out, sorted(inputs))
        self.assertTrue(fh.is_empty())

    def test_decrease_key(self):
        fh = FibonacciHeap()
        n1 = fh.push(100)
        _n2 = fh.push(50)
        n3 = fh.push(80)
        _n4 = fh.push(30)
        n5 = fh.push(60)

        top = fh.pop()
        self.assertEqual(top, 30)

        fh.decrease_key(n1, 10)
        self.assertEqual(fh.top(), 10)

        fh.decrease_key(n3, 5)
        self.assertEqual(fh.top(), 5)

        fh.decrease_key(n5, 1)
        self.assertEqual(fh.top(), 1)

        out = []
        while not fh.is_empty():
            out.append(fh.pop())
        self.assertEqual(out, [1, 5, 10, 50])

    def test_meld(self):
        h1 = FibonacciHeap()
        h1.push(20)
        h1.push(40)

        h2 = FibonacciHeap()
        h2.push(10)
        h2.push(30)

        h1.meld(h2)
        self.assertEqual(len(h1), 4)
        self.assertTrue(h2.is_empty())
        self.assertEqual(h1.top(), 10)

        out = []
        while not h1.is_empty():
            out.append(h1.pop())
        self.assertEqual(out, [10, 20, 30, 40])

    def test_duplicates_and_empty(self):
        fh = FibonacciHeap()
        with self.assertRaises(IndexError):
            fh.pop()
        with self.assertRaises(IndexError):
            fh.top()

        fh.push(7)
        fh.push(7)
        fh.push(7)
        self.assertEqual(len(fh), 3)
        self.assertEqual(fh.pop(), 7)
        self.assertEqual(fh.pop(), 7)
        self.assertEqual(fh.pop(), 7)
        self.assertTrue(fh.is_empty())


if __name__ == "__main__":
    unittest.main()
