"""
Amortized Analysis verification and instrumentation in Python.

Provides:
1. TrackedDynamicArray verifying O(1) amortized push and potential method.
2. TrackedMultipopStack verifying aggregate pop work bounded by push count.
3. TrackedBinaryCounter verifying aggregate bit flips bounded by 2N.
4. Comprehensive unit tests.
"""

import unittest
from typing import List


class TrackedDynamicArray:
    """Dynamic array tracking memory copies and potential function Phi = 2 * size - capacity."""

    def __init__(self):
        self._data: List[int] = []
        self._capacity: int = 0
        self._total_copies: int = 0

    @property
    def size(self) -> int:
        return len(self._data)

    @property
    def capacity(self) -> int:
        return self._capacity

    @property
    def total_copies(self) -> int:
        return self._total_copies

    def potential(self) -> int:
        if self._capacity == 0:
            return 0
        return 2 * len(self._data) - self._capacity

    def push_back(self, val: int) -> int:
        """Appends element and returns amortized cost of this push."""
        phi_before = self.potential()
        copies_before = self._total_copies

        if len(self._data) == self._capacity:
            new_capacity = 1 if self._capacity == 0 else self._capacity * 2
            # Simulate copying old elements into new array
            self._total_copies += len(self._data)
            self._capacity = new_capacity

        self._data.append(val)
        actual_cost = 1 + (self._total_copies - copies_before)
        delta_phi = self.potential() - phi_before
        amortized_cost = actual_cost + delta_phi
        return amortized_cost


class TrackedMultipopStack:
    """Stack tracking total pushes and pops to verify aggregate linear bound."""

    def __init__(self):
        self._data: List[int] = []
        self.total_pushes: int = 0
        self.total_pops: int = 0

    def push(self, x: int) -> None:
        self._data.append(x)
        self.total_pushes += 1

    def pop(self) -> int:
        if not self._data:
            raise IndexError("pop from empty stack")
        val = self._data.pop()
        self.total_pops += 1
        return val

    def multipop(self, k: int) -> List[int]:
        cnt = min(k, len(self._data))
        res = []
        for _ in range(cnt):
            res.append(self.pop())
        return res

    def __len__(self) -> int:
        return len(self._data)


class TrackedBinaryCounter:
    """k-bit binary counter tracking total bit flips."""

    def __init__(self):
        self.bits: List[bool] = []
        self.total_flips: int = 0

    def increment(self) -> None:
        i = 0
        while i < len(self.bits) and self.bits[i]:
            self.bits[i] = False
            self.total_flips += 1
            i += 1
        if i < len(self.bits):
            self.bits[i] = True
            self.total_flips += 1
        else:
            self.bits.append(True)
            self.total_flips += 1


class TestAmortizedAnalysis(unittest.TestCase):
    def test_dynamic_array(self):
        arr = TrackedDynamicArray()
        N = 10000
        for i in range(N):
            amortized_cost = arr.push_back(i)
            # Amortized cost upper bound is 3
            self.assertLessEqual(amortized_cost, 3)

        # Aggregate total copies must strictly be < 2 * N
        self.assertLess(arr.total_copies, 2 * N)
        self.assertEqual(arr.size, N)

    def test_multipop_stack(self):
        st = TrackedMultipopStack()
        for i in range(500):
            st.push(i)
        st.multipop(200)
        for i in range(300):
            st.push(i)
        st.multipop(1000)

        self.assertEqual(len(st), 0)
        self.assertLessEqual(st.total_pops, st.total_pushes)

    def test_binary_counter(self):
        counter = TrackedBinaryCounter()
        N = 5000
        for _ in range(N):
            counter.increment()

        # Total bit flips must strictly be < 2 * N
        self.assertLess(counter.total_flips, 2 * N)


if __name__ == '__main__':
    unittest.main()
