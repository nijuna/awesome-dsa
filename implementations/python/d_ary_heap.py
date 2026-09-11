"""
Reference Implementation: Contiguous Array-Backed d-ary Heap
Demonstrates arbitrary branching factor d (e.g., d = 4, d = 8),
generalized 0-based index arithmetic, sift-up (O(log_d n)),
sift-down with d-child scan (O(d log_d n)), and Floyd-style O(n) build_heap.

Language: Python 3
"""

from typing import Any, List, Callable, Optional
import unittest


class DAryHeap:
    def __init__(self, d: int = 4, elements: Optional[List[Any]] = None, key: Optional[Callable[[Any], Any]] = None):
        if d < 2:
            raise ValueError("Branching factor d must be at least 2")
        self._d = d
        self._key = key if key is not None else (lambda x: x)
        if elements is not None:
            self._data = list(elements)
            self._build_heap()
        else:
            self._data = []

    def __len__(self) -> int:
        return len(self._data)

    def is_empty(self) -> bool:
        return len(self._data) == 0

    def _parent(self, i: int) -> int:
        return (i - 1) // self._d

    def _first_child(self, i: int) -> int:
        return self._d * i + 1

    def _sift_up(self, i: int) -> None:
        while i > 0:
            p = self._parent(i)
            if self._key(self._data[i]) < self._key(self._data[p]):
                self._data[i], self._data[p] = self._data[p], self._data[i]
                i = p
            else:
                break

    def _sift_down(self, i: int, n: int) -> None:
        while True:
            best = i
            first = self._first_child(i)
            if first >= n:
                break

            last = min(first + self._d, n)
            for c in range(first, last):
                if self._key(self._data[c]) < self._key(self._data[best]):
                    best = c

            if best != i:
                self._data[i], self._data[best] = self._data[best], self._data[i]
                i = best
            else:
                break

    def _build_heap(self) -> None:
        n = len(self._data)
        if n > 1:
            for i in range((n - 2) // self._d, -1, -1):
                self._sift_down(i, n)

    def push(self, val: Any) -> None:
        self._data.append(val)
        self._sift_up(len(self._data) - 1)

    def pop(self) -> Any:
        if not self._data:
            raise IndexError("pop from an empty heap")
        top_val = self._data[0]
        last = self._data.pop()
        if self._data:
            self._data[0] = last
            self._sift_down(0, len(self._data))
        return top_val

    def peek(self) -> Any:
        if not self._data:
            raise IndexError("peek from an empty heap")
        return self._data[0]

    def is_valid_heap(self) -> bool:
        n = len(self._data)
        for i in range(n):
            first = self._first_child(i)
            last = min(first + self._d, n)
            for c in range(first, last):
                if self._key(self._data[c]) < self._key(self._data[i]):
                    return False
        return True


class TestDAryHeap(unittest.TestCase):
    def test_4ary_min_heap(self):
        heap = DAryHeap(d=4)
        self.assertTrue(heap.is_empty())
        self.assertEqual(len(heap), 0)

        inputs = [55, 12, 89, 4, 32, 1, 67, 23, 19, 78, 3, 99, 45, 6]
        for x in inputs:
            heap.push(x)
            self.assertTrue(heap.is_valid_heap())

        self.assertEqual(len(heap), len(inputs))
        self.assertEqual(heap.peek(), 1)

        sorted_out = []
        while not heap.is_empty():
            sorted_out.append(heap.pop())
            self.assertTrue(heap.is_valid_heap())

        self.assertEqual(sorted_out, sorted(inputs))

    def test_8ary_build_heap(self):
        inputs = [55, 12, 89, 4, 32, 1, 67, 23, 19, 78, 3, 99, 45, 6]
        heap8 = DAryHeap(d=8, elements=inputs)
        self.assertTrue(heap8.is_valid_heap())
        self.assertEqual(heap8.peek(), 1)

    def test_3ary_heap(self):
        inputs = [10, 9, 8, 7, 6, 5, 4, 3, 2, 1]
        heap3 = DAryHeap(d=3, elements=inputs)
        self.assertTrue(heap3.is_valid_heap())
        self.assertEqual(heap3.peek(), 1)


if __name__ == "__main__":
    unittest.main()
