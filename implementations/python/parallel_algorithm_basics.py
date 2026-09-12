"""
Parallel Algorithm Basics and the Work-Depth Model Implementation in Python 3.

Provides:
- parallel_reduce: Tree-based reduction model (Work: O(N), Span: O(log N)).
- blelloch_exclusive_scan: Work-efficient prefix scan via upsweep & downsweep (Work: O(N), Span: O(log N)).
- parallel_filter: Stream compaction using prefix scan offsets (Work: O(N), Span: O(log N)).
- parallel_mergesort: Divide-and-conquer parallel sorting model.
"""

import random
import unittest
from typing import List, Callable, TypeVar

T = TypeVar('T')


def parallel_reduce(arr: List[T], identity: T, op: Callable[[T, T], T]) -> T:
    """
    Tree-based parallel reduction model.
    Work: O(N), Span: O(log N).
    """
    if not arr:
        return identity
    current = list(arr)
    while len(current) > 1:
        next_level = []
        for i in range(0, len(current), 2):
            if i + 1 < len(current):
                next_level.append(op(current[i], current[i + 1]))
            else:
                next_level.append(current[i])
        current = next_level
    return current[0]


def blelloch_exclusive_scan(data: List[int]) -> List[int]:
    """
    Blelloch's Work-Efficient Parallel Prefix Scan.
    Work: O(N), Span: O(log N).
    Pass 1: Upsweep (Reduction Tree).
    Pass 2: Downsweep (Prefix Distribution Tree).
    """
    n = len(data)
    if n == 0:
        return []
    if n == 1:
        return [0]

    # Pad to nearest power of 2
    m = 1
    while m < n:
        m <<= 1
    tree = [0] * (2 * m)

    for i in range(n):
        tree[m + i] = data[i]

    # Pass 1: Upsweep (reduce phase)
    d = m // 2
    while d > 0:
        for i in range(d, 2 * d):
            tree[i] = tree[2 * i] + tree[2 * i + 1]
        d //= 2

    # Clear root for exclusive scan
    tree[1] = 0

    # Pass 2: Downsweep phase
    d = 1
    while d < m:
        for i in range(d, 2 * d):
            t = tree[2 * i]
            tree[2 * i] = tree[i]
            tree[2 * i + 1] = tree[i] + t
        d *= 2

    return [tree[m + i] for i in range(n)]


def parallel_filter(data: List[T], pred: Callable[[T], bool]) -> List[T]:
    """
    Parallel Stream Compaction / Filter.
    Work: O(N), Span: O(log N).
    """
    n = len(data)
    if n == 0:
        return []

    flags = [1 if pred(x) else 0 for x in data]
    offsets = blelloch_exclusive_scan(flags)
    total = offsets[-1] + flags[-1]

    out = [None] * total
    for i in range(n):
        if flags[i]:
            out[offsets[i]] = data[i]
    return out


def parallel_mergesort(arr: List[int]) -> List[int]:
    """
    Divide-and-conquer parallel mergesort model.
    Work: O(N log N), Span: O(log^2 N).
    """
    if len(arr) <= 1:
        return list(arr)

    mid = len(arr) // 2
    left = parallel_mergesort(arr[:mid])
    right = parallel_mergesort(arr[mid:])

    # Merge
    res = []
    i, j = 0, 0
    while i < len(left) and j < len(right):
        if left[i] <= right[j]:
            res.append(left[i])
            i += 1
        else:
            res.append(right[j])
            j += 1
    res.extend(left[i:])
    res.extend(right[j:])
    return res


class TestParallelBasics(unittest.TestCase):
    def test_reduction(self):
        arr = list(range(1, 101))
        expected = sum(arr)
        actual = parallel_reduce(arr, 0, lambda a, b: a + b)
        self.assertEqual(actual, expected)

    def test_blelloch_scan(self):
        data = [3, 1, 7, 0, 4, 1, 6, 3]
        expected = [0, 3, 4, 11, 11, 15, 16, 22]
        actual = blelloch_exclusive_scan(data)
        self.assertEqual(actual, expected)

    def test_parallel_filter(self):
        data = list(range(1, 21))
        evens = parallel_filter(data, lambda x: x % 2 == 0)
        self.assertEqual(evens, [x for x in data if x % 2 == 0])

    def test_parallel_mergesort(self):
        rng = random.Random(42)
        arr = [rng.randint(0, 1000) for _ in range(500)]
        expected = sorted(arr)
        actual = parallel_mergesort(arr)
        self.assertEqual(actual, expected)


if __name__ == '__main__':
    unittest.main()
