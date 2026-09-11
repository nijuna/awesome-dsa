"""
Reference Implementation: Greedy Algorithms & Paradigm Mechanics
Demonstrates:
1. Interval Scheduling (Earliest Finish Time Heuristic, O(N log N)).
2. Fractional Knapsack (Value-to-Weight Density Greedy Choice, O(N log N)).
3. Huffman Coding (Prefix-Free Tree Construction via Min-Heap, O(N log N)).
4. Coin Change Analysis (Greedy Optimality on Canonical Systems vs. Failure Counterexamples).

Language: Python 3
"""

import heapq
from typing import Dict, List, Tuple
import unittest


def interval_scheduling(intervals: List[Tuple[int, int]]) -> List[Tuple[int, int]]:
    """Selects the maximum number of mutually non-overlapping intervals.

    Greedy Choice: Earliest Finish Time.
    Time Complexity: O(N log N)
    Space Complexity: O(N)
    """
    sorted_intervals = sorted(intervals, key=lambda x: (x[1], x[0]))
    chosen = []
    current_finish = -1

    for start, finish in sorted_intervals:
        if start >= current_finish:
            chosen.append((start, finish))
            current_finish = finish

    return chosen


def fractional_knapsack(
    capacity: float,
    items: List[Tuple[int, float, float]],  # (item_id, value, weight)
) -> Tuple[float, List[Tuple[int, float]]]:
    """Maximizes total value in a knapsack where items can be fractionally divided.

    Greedy Choice: Highest Value-to-Weight Ratio.
    Time Complexity: O(N log N)
    Space Complexity: O(N)
    """
    sorted_items = sorted(items, key=lambda x: x[1] / x[2], reverse=True)
    total_value = 0.0
    remaining = capacity
    taken = []

    for item_id, value, weight in sorted_items:
        if remaining <= 0.0:
            break
        if weight <= remaining:
            total_value += value
            remaining -= weight
            taken.append((item_id, 1.0))
        else:
            fraction = remaining / weight
            total_value += value * fraction
            taken.append((item_id, fraction))
            remaining = 0.0

    return total_value, taken


class _HuffmanNode:
    def __init__(
        self,
        freq: int,
        symbol: str = "",
        left: "_HuffmanNode" = None,
        right: "_HuffmanNode" = None,
    ):
        self.freq = freq
        self.symbol = symbol
        self.left = left
        self.right = right

    def __lt__(self, other: "_HuffmanNode") -> bool:
        return self.freq < other.freq


def huffman_encoding(frequencies: Dict[str, int]) -> Dict[str, str]:
    """Generates an optimal prefix-free binary encoding tree.

    Greedy Choice: Merge two least frequent nodes.
    Time Complexity: O(N log N)
    Space Complexity: O(N)
    """
    if not frequencies:
        return {}

    pq = [_HuffmanNode(freq, symbol=symbol) for symbol, freq in frequencies.items()]
    heapq.heapify(pq)

    if len(pq) == 1:
        return {pq[0].symbol: "0"}

    while len(pq) > 1:
        left = heapq.heappop(pq)
        right = heapq.heappop(pq)
        parent = _HuffmanNode(left.freq + right.freq, left=left, right=right)
        heapq.heappush(pq, parent)

    root = pq[0]
    codes = {}

    def _traverse(node: _HuffmanNode, prefix: str):
        if not node.left and not node.right:
            codes[node.symbol] = prefix if prefix else "0"
            return
        if node.left:
            _traverse(node.left, prefix + "0")
        if node.right:
            _traverse(node.right, prefix + "1")

    _traverse(root, "")
    return codes


def greedy_coin_change(amount: int, denominations: List[int]) -> int:
    """Computes coin count using greedy heuristic (assumes denominations sorted descending)."""
    count = 0
    remaining = amount
    for coin in denominations:
        if remaining <= 0:
            break
        take = remaining // coin
        count += take
        remaining -= take * coin
    return count if remaining == 0 else -1


class TestGreedyAlgorithms(unittest.TestCase):
    def test_interval_scheduling(self):
        intervals = [
            (1, 4),
            (3, 5),
            (0, 6),
            (5, 7),
            (3, 9),
            (5, 9),
            (6, 10),
            (8, 11),
            (8, 12),
            (2, 14),
            (12, 16),
        ]
        chosen = interval_scheduling(intervals)
        self.assertEqual(len(chosen), 4)
        self.assertEqual(chosen, [(1, 4), (5, 7), (8, 11), (12, 16)])

        # Mutual disjointness
        for i in range(1, len(chosen)):
            self.assertGreaterEqual(chosen[i][0], chosen[i - 1][1])

    def test_fractional_knapsack(self):
        items = [
            (1, 60.0, 10.0),  # density = 6.0
            (2, 100.0, 20.0),  # density = 5.0
            (3, 120.0, 30.0),  # density = 4.0
        ]
        capacity = 50.0
        val, taken = fractional_knapsack(capacity, items)
        self.assertAlmostEqual(val, 240.0)
        self.assertEqual(len(taken), 3)
        self.assertEqual(taken[0], (1, 1.0))
        self.assertEqual(taken[1], (2, 1.0))
        self.assertEqual(taken[2][0], 3)
        self.assertAlmostEqual(taken[2][1], 2.0 / 3.0)

    def test_huffman_encoding(self):
        freqs = {"A": 5, "B": 9, "C": 12, "D": 13, "E": 16, "F": 45}
        codes = huffman_encoding(freqs)
        self.assertEqual(len(codes), 6)

        # Prefix-free check
        for s1, c1 in codes.items():
            for s2, c2 in codes.items():
                if s1 != s2:
                    self.assertFalse(c1.startswith(c2))
                    self.assertFalse(c2.startswith(c1))

        # Highest frequency must receive strictly shorter or equal code than lowest
        self.assertLess(len(codes["F"]), len(codes["A"]))

    def test_coin_change_canonical_vs_failure(self):
        us_coins = [25, 10, 5, 1]
        self.assertEqual(greedy_coin_change(63, us_coins), 6)

        # Counterexample where greedy is suboptimal: {4, 3, 1} for amount 6
        # Greedy chooses 4, 1, 1 (3 coins), optimal is 3, 3 (2 coins)
        non_canonical = [4, 3, 1]
        self.assertEqual(greedy_coin_change(6, non_canonical), 3)


if __name__ == "__main__":
    unittest.main()
