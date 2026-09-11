"""
Reference Implementation: Contiguous Array-Backed Binary Heap
Demonstrates implicit tree indexing (2i + 1, 2i + 2, (i - 1) // 2),
sift-up and sift-down mechanics, Floyd's O(n) linear-time build_heap,
and in-place heapsort.

Language: Python 3
"""

from typing import Any, List, Callable, Optional
import unittest


class BinaryHeap:
    def __init__(self, elements: Optional[List[Any]] = None, key: Optional[Callable[[Any], Any]] = None):
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

    @staticmethod
    def _parent(i: int) -> int:
        return (i - 1) // 2

    @staticmethod
    def _left(i: int) -> int:
        return 2 * i + 1

    @staticmethod
    def _right(i: int) -> int:
        return 2 * i + 2

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
            left = self._left(i)
            right = self._right(i)

            if left < n and self._key(self._data[left]) < self._key(self._data[best]):
                best = left
            if right < n and self._key(self._data[right]) < self._key(self._data[best]):
                best = right

            if best != i:
                self._data[i], self._data[best] = self._data[best], self._data[i]
                i = best
            else:
                break

    def _build_heap(self) -> None:
        # Floyd's bottom-up linear time O(n) heap construction
        n = len(self._data)
        for i in range(n // 2 - 1, -1, -1):
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
        for i in range(len(self._data)):
            l = self._left(i)
            r = self._right(i)
            if l < len(self._data) and self._key(self._data[l]) < self._key(self._data[i]):
                return False
            if r < len(self._data) and self._key(self._data[r]) < self._key(self._data[i]):
                return False
        return True

    @staticmethod
    def heapsort(arr: List[Any]) -> None:
        """In-place heapsort in O(n log n)."""
        n = len(arr)
        # 1. Build max-heap using Floyd's algorithm
        def _max_sift_down(i: int, limit: int):
            while True:
                best = i
                l = 2 * i + 1
                r = 2 * i + 2
                if l < limit and arr[l] > arr[best]:
                    best = l
                if r < limit and arr[r] > arr[best]:
                    best = r
                if best != i:
                    arr[i], arr[best] = arr[best], arr[i]
                    i = best
                else:
                    break

        for i in range(n // 2 - 1, -1, -1):
            _max_sift_down(i, n)

        # 2. Extract elements to end of array
        for end in range(n - 1, 0, -1):
            arr[0], arr[end] = arr[end], arr[0]
            _max_sift_down(0, end)


class TestBinaryHeap(unittest.TestCase):
    def test_min_heap(self):
        heap = BinaryHeap()
        self.assertTrue(heap.is_empty())
        self.assertEqual(len(heap), 0)

        inputs = [42, 17, 93, 8, 31, 5, 64, 22, 11, 75]
        for x in inputs:
            heap.push(x)
            self.assertTrue(heap.is_valid_heap())

        self.assertEqual(len(heap), 10)
        self.assertEqual(heap.peek(), 5)

        sorted_out = []
        while not heap.is_empty():
            sorted_out.append(heap.pop())
            self.assertTrue(heap.is_valid_heap())

        self.assertEqual(sorted_out, sorted(inputs))

    def test_floyd_build_heap(self):
        batch = [19, 3, 15, 7, 8, 23, 2, 4, 11, 14, 1, 6]
        heap = BinaryHeap(batch)
        self.assertTrue(heap.is_valid_heap())
        self.assertEqual(heap.peek(), 1)
        self.assertEqual(len(heap), len(batch))

    def test_heapsort(self):
        arr = [9, 4, 1, 7, 3, 8, 2, 6, 5, 0]
        BinaryHeap.heapsort(arr)
        self.assertEqual(arr, list(range(10)))


if __name__ == "__main__":
    unittest.main()
