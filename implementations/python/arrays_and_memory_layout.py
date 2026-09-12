"""
Arrays and Memory Layout demonstration in Python.

Provides:
1. Flat 1D array Row-Major 2D Matrix wrapper.
2. Flat 1D array Column-Major 2D Matrix wrapper.
3. Stride calculations and linear index translations.
4. Comprehensive unit tests.
"""

import unittest
from typing import List, Any


class RowMajorMatrix:
    """Contiguous 2D matrix stored in row-major order: index = r * cols + c."""

    def __init__(self, rows: int, cols: int, init_val: Any = 0):
        if rows < 0 or cols < 0:
            raise ValueError("Matrix dimensions must be non-negative")
        self.rows = rows
        self.cols = cols
        self.data: List[Any] = [init_val] * (rows * cols)

    def linear_index(self, r: int, c: int) -> int:
        if not (0 <= r < self.rows and 0 <= c < self.cols):
            raise IndexError("Matrix indices out of bounds")
        return r * self.cols + c

    def get(self, r: int, c: int) -> Any:
        return self.data[self.linear_index(r, c)]

    def set(self, r: int, c: int, val: Any) -> None:
        self.data[self.linear_index(r, c)] = val

    def __getitem__(self, idx: tuple) -> Any:
        r, c = idx
        return self.get(r, c)

    def __setitem__(self, idx: tuple, val: Any) -> None:
        r, c = idx
        self.set(r, c, val)


class ColumnMajorMatrix:
    """Contiguous 2D matrix stored in column-major order: index = c * rows + r."""

    def __init__(self, rows: int, cols: int, init_val: Any = 0):
        if rows < 0 or cols < 0:
            raise ValueError("Matrix dimensions must be non-negative")
        self.rows = rows
        self.cols = cols
        self.data: List[Any] = [init_val] * (rows * cols)

    def linear_index(self, r: int, c: int) -> int:
        if not (0 <= r < self.rows and 0 <= c < self.cols):
            raise IndexError("Matrix indices out of bounds")
        return c * self.rows + r

    def get(self, r: int, c: int) -> Any:
        return self.data[self.linear_index(r, c)]

    def set(self, r: int, c: int, val: Any) -> None:
        self.data[self.linear_index(r, c)] = val

    def __getitem__(self, idx: tuple) -> Any:
        r, c = idx
        return self.get(r, c)

    def __setitem__(self, idx: tuple, val: Any) -> None:
        r, c = idx
        self.set(r, c, val)


class TestArraysAndMemoryLayout(unittest.TestCase):
    def test_row_major(self):
        R, C = 3, 4
        m = RowMajorMatrix(R, C)
        counter = 1
        for i in range(R):
            for j in range(C):
                m[i, j] = counter
                counter += 1

        self.assertEqual(m.data, list(range(1, 13)))
        self.assertEqual(m[0, 0], 1)
        self.assertEqual(m[0, 3], 4)
        self.assertEqual(m[1, 0], 5)
        self.assertEqual(m[2, 3], 12)

    def test_column_major(self):
        R, C = 3, 4
        m = ColumnMajorMatrix(R, C)
        for i in range(R):
            for j in range(C):
                m[i, j] = 10 * i + j

        # First column elements: (0,0)->0, (1,0)->10, (2,0)->20
        self.assertEqual(m.data[0], 0)
        self.assertEqual(m.data[1], 10)
        self.assertEqual(m.data[2], 20)
        self.assertEqual(m.data[3], 1)
        self.assertEqual(m.data[4], 11)
        self.assertEqual(m.data[5], 21)

    def test_bounds(self):
        m = RowMajorMatrix(2, 2)
        with self.assertRaises(IndexError):
            _ = m[2, 0]
        with self.assertRaises(IndexError):
            _ = m[0, 2]


if __name__ == '__main__':
    unittest.main()
