"""
Multi-Way External Merge Sort Simulation in Python.

Implements Phase 1 run generation and Phase 2 heapq-based multi-way merge
with unit tests comparing against standard sort.
"""

import heapq
import random
import unittest
from typing import List


def external_merge_sort(data: List[int], memory_capacity: int) -> List[int]:
    n = len(data)
    if n <= memory_capacity:
        return sorted(data)

    # Phase 1: Run Formation
    runs: List[List[int]] = []
    for i in range(0, n, memory_capacity):
        chunk = sorted(data[i : i + memory_capacity])
        runs.append(chunk)

    # Phase 2: K-way merge
    # Heap stores: (value, run_index, element_index)
    min_heap = []
    for r in range(len(runs)):
        if runs[r]:
            heapq.heappush(min_heap, (runs[r][0], r, 0))

    output: List[int] = []
    while min_heap:
        val, r_idx, elem_idx = heapq.heappop(min_heap)
        output.append(val)

        next_elem_idx = elem_idx + 1
        if next_elem_idx < len(runs[r_idx]):
            heapq.heappush(min_heap, (runs[r_idx][next_elem_idx], r_idx, next_elem_idx))

    return output


class TestExternalSorting(unittest.TestCase):
    def test_external_merge_sort_equivalence(self):
        rng = random.Random(42)
        dataset = [rng.randint(-10000, 10000) for _ in range(2500)]
        m = 100  # 25 runs

        sorted_res = external_merge_sort(dataset, m)
        self.assertEqual(sorted_res, sorted(dataset))


if __name__ == "__main__":
    unittest.main()
