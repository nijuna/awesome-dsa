"""
Pythonic Container API Design and Protocols.

Implements a custom DynamicArray container satisfying Python's Sequence
and Collection protocols (__len__, __getitem__, __setitem__, __iter__, __contains__, __repr__)
with complete unit tests.
"""

import unittest
from typing import Any, Iterator, List


class DynamicArray:
    """A pythonic dynamic array implementing standard collection protocols."""

    def __init__(self, initial_capacity: int = 4):
        self._capacity = max(1, initial_capacity)
        self._size = 0
        self._data: List[Any] = [None] * self._capacity

    def _resize(self, new_cap: int) -> None:
        new_data = [None] * new_cap
        for i in range(self._size):
            new_data[i] = self._data[i]
        self._data = new_data
        self._capacity = new_cap

    def append(self, item: Any) -> None:
        if self._size == self._capacity:
            self._resize(self._capacity * 2)
        self._data[self._size] = item
        self._size += 1

    def pop(self) -> Any:
        if self._size == 0:
            raise IndexError("pop from empty array")
        self._size -= 1
        val = self._data[self._size]
        self._data[self._size] = None  # Prevent memory retention
        return val

    def __len__(self) -> int:
        return self._size

    def __getitem__(self, index: int) -> Any:
        if isinstance(index, slice):
            start, stop, step = index.indices(self._size)
            result = DynamicArray(len(range(start, stop, step)))
            for i in range(start, stop, step):
                result.append(self._data[i])
            return result

        if index < 0:
            index += self._size
        if index < 0 or index >= self._size:
            raise IndexError("Index out of range")
        return self._data[index]

    def __setitem__(self, index: int, value: Any) -> None:
        if index < 0:
            index += self._size
        if index < 0 or index >= self._size:
            raise IndexError("Index out of range")
        self._data[index] = value

    def __iter__(self) -> Iterator[Any]:
        for i in range(self._size):
            yield self._data[i]

    def __contains__(self, item: Any) -> bool:
        for i in range(self._size):
            if self._data[i] == item:
                return True
        return False

    def __repr__(self) -> str:
        items_str = ", ".join(repr(self._data[i]) for i in range(self._size))
        return f"DynamicArray([{items_str}])"


class TestAPIDesign(unittest.TestCase):
    def test_container_protocols(self):
        arr = DynamicArray()
        self.assertEqual(len(arr), 0)

        for i in range(5):
            arr.append(i * 10)

        self.assertEqual(len(arr), 5)
        self.assertEqual(arr[0], 0)
        self.assertEqual(arr[4], 40)
        self.assertEqual(arr[-1], 40)

        # Iteration
        self.assertEqual(list(arr), [0, 10, 20, 30, 40])

        # Contains
        self.assertIn(20, arr)
        self.assertNotIn(999, arr)

        # Mutation
        arr[0] = 77
        self.assertEqual(arr[0], 77)

        # Slicing
        slice_res = arr[1:4]
        self.assertEqual(list(slice_res), [10, 20, 30])

        # Bounds error
        with self.assertRaises(IndexError):
            _ = arr[100]


if __name__ == "__main__":
    unittest.main()
