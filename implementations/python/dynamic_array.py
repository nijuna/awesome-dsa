"""
Reference Implementation: Dynamic Array (Custom Resizable Array)
Demonstrates contiguous memory management via ctypes, geometric growth factor (2.0x),
amortized O(1) appending, bounds checking, and unittests.
"""

from __future__ import annotations
import ctypes
import unittest
from typing import Any, Iterator


class DynamicArray:
    """A dynamic resizable array backed by raw contiguous memory."""

    def __init__(self, initial_capacity: int = 1) -> None:
        if initial_capacity < 0:
            raise ValueError("Initial capacity must be non-negative")
        self._size: int = 0
        self._capacity: int = max(1, initial_capacity)
        self._array = self._make_array(self._capacity)

    def _make_array(self, capacity: int) -> ctypes.Array[Any]:
        """Allocates raw contiguous memory block of py_object pointers."""
        return (capacity * ctypes.py_object)()

    def __len__(self) -> int:
        return self._size

    @property
    def capacity(self) -> int:
        return self._capacity

    def is_empty(self) -> bool:
        return self._size == 0

    def __getitem__(self, index: int) -> Any:
        if not 0 <= index < self._size:
            raise IndexError(f"Index {index} out of bounds (size {self._size})")
        return self._array[index]

    def __setitem__(self, index: int, value: Any) -> None:
        if not 0 <= index < self._size:
            raise IndexError(f"Index {index} out of bounds (size {self._size})")
        self._array[index] = value

    def _resize(self, new_capacity: int) -> None:
        """Reallocates underlying memory and copies existing elements."""
        new_capacity = max(new_capacity, self._size)
        new_array = self._make_array(new_capacity)
        for i in range(self._size):
            new_array[i] = self._array[i]
        self._array = new_array
        self._capacity = new_capacity

    def append(self, value: Any) -> None:
        """Amortized O(1) append using geometric doubling."""
        if self._size == self._capacity:
            self._resize(2 * self._capacity)
        self._array[self._size] = value
        self._size += 1

    def pop(self) -> Any:
        """O(1) pop from the end of the array."""
        if self._size == 0:
            raise IndexError("pop from empty DynamicArray")
        val = self._array[self._size - 1]
        self._array[self._size - 1] = None  # dereference for garbage collection
        self._size -= 1
        return val

    def insert(self, index: int, value: Any) -> None:
        """O(N) insert at specified index."""
        if not 0 <= index <= self._size:
            raise IndexError(f"Index {index} out of bounds (size {self._size})")
        if self._size == self._capacity:
            self._resize(2 * self._capacity)
        for i in range(self._size, index, -1):
            self._array[i] = self._array[i - 1]
        self._array[index] = value
        self._size += 1

    def delete_at(self, index: int) -> Any:
        """O(N) deletion at specified index."""
        if not 0 <= index < self._size:
            raise IndexError(f"Index {index} out of bounds (size {self._size})")
        removed_val = self._array[index]
        for i in range(index, self._size - 1):
            self._array[i] = self._array[i + 1]
        self._array[self._size - 1] = None
        self._size -= 1
        return removed_val

    def shrink_to_fit(self) -> None:
        """Shrinks allocated capacity down to current size."""
        if self._capacity > self._size:
            self._resize(max(1, self._size))

    def __iter__(self) -> Iterator[Any]:
        for i in range(self._size):
            yield self._array[i]

    def to_list(self) -> list[Any]:
        return [self._array[i] for i in range(self._size)]


class TestDynamicArray(unittest.TestCase):
    def test_basic_append_and_access(self):
        arr = DynamicArray()
        self.assertEqual(len(arr), 0)
        self.assertTrue(arr.is_empty())

        for i in range(10):
            arr.append(i * 5)

        self.assertEqual(len(arr), 10)
        self.assertFalse(arr.is_empty())
        self.assertGreaterEqual(arr.capacity, 10)

        for i in range(10):
            self.assertEqual(arr[i], i * 5)

    def test_out_of_bounds(self):
        arr = DynamicArray()
        arr.append(42)
        with self.assertRaises(IndexError):
            _ = arr[1]
        with self.assertRaises(IndexError):
            _ = arr[-1]

    def test_insert_and_delete(self):
        arr = DynamicArray()
        for x in [10, 20, 30, 40]:
            arr.append(x)

        arr.insert(2, 999)  # [10, 20, 999, 30, 40]
        self.assertEqual(len(arr), 5)
        self.assertEqual(arr[2], 999)
        self.assertEqual(arr[3], 30)

        val = arr.delete_at(2)
        self.assertEqual(val, 999)
        self.assertEqual(len(arr), 4)
        self.assertEqual(arr.to_list(), [10, 20, 30, 40])

    def test_pop(self):
        arr = DynamicArray()
        arr.append(100)
        arr.append(200)
        self.assertEqual(arr.pop(), 200)
        self.assertEqual(arr.pop(), 100)
        self.assertEqual(len(arr), 0)
        with self.assertRaises(IndexError):
            arr.pop()

    def test_shrink_to_fit(self):
        arr = DynamicArray()
        for i in range(20):
            arr.append(i)
        prev_cap = arr.capacity
        self.assertGreater(prev_cap, 20)
        arr.shrink_to_fit()
        self.assertEqual(arr.capacity, 20)


if __name__ == "__main__":
    unittest.main()
