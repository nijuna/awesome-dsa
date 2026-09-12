"""
Reference implementation of Branch and Bound for 0/1 Knapsack Optimization.

Implements Best-First Search Branch and Bound with fractional knapsack bounding,
greedy warm starting, and differential verification against an exact DP oracle.
"""

import heapq
import random
import unittest
from typing import List, Tuple


class KnapsackItem:
    __slots__ = ('value', 'weight')

    def __init__(self, value: int, weight: int):
        assert value >= 0 and weight >= 0
        self.value = value
        self.weight = weight


class BranchAndBoundKnapsack:
    """Best-First Search Branch and Bound with Fractional Knapsack Bounding."""

    class _InternalItem:
        __slots__ = ('value', 'weight', 'density')

        def __init__(self, value: int, weight: int):
            self.value = value
            self.weight = weight
            self.density = value / weight if weight > 0 else float('inf')

        def __lt__(self, other: 'BranchAndBoundKnapsack._InternalItem') -> bool:
            return self.density > other.density  # Sort descending by density

    class _SearchNode:
        __slots__ = ('bound', 'level', 'value', 'weight')

        def __init__(self, bound: float, level: int, value: int, weight: int):
            self.bound = bound
            self.level = level
            self.value = value
            self.weight = weight

        def __lt__(self, other: 'BranchAndBoundKnapsack._SearchNode') -> bool:
            # Python's heapq is a min-heap; we invert bound so highest bound comes first
            return self.bound > other.bound

    @staticmethod
    def _compute_bound(level: int, current_val: int, current_weight: int,
                       capacity: int, items: List['_InternalItem']) -> float:
        if current_weight >= capacity:
            return 0.0

        bound = float(current_val)
        rem_cap = capacity - current_weight
        n = len(items)

        for i in range(level, n):
            if items[i].weight <= rem_cap:
                rem_cap -= items[i].weight
                bound += items[i].value
            else:
                bound += rem_cap * items[i].density
                break
        return bound

    @classmethod
    def solve(cls, raw_items: List[KnapsackItem], capacity: int) -> int:
        if capacity <= 0 or not raw_items:
            return 0

        total_value = 0
        items: List[cls._InternalItem] = []
        total_weight = 0

        for it in raw_items:
            if it.weight == 0:
                total_value += it.value
            elif it.weight <= capacity and it.value > 0:
                items.append(cls._InternalItem(it.value, it.weight))
                total_weight += it.weight

        if total_weight <= capacity:
            return total_value + sum(it.value for it in items)

        items.sort()

        # Greedy warm-start incumbent
        incumbent = 0
        greedy_weight = 0
        for it in items:
            if greedy_weight + it.weight <= capacity:
                greedy_weight += it.weight
                incumbent += it.value

        # Best-First Search with Priority Queue
        pq: List[cls._SearchNode] = []
        root_bound = cls._compute_bound(0, 0, 0, capacity, items)
        if root_bound > incumbent:
            heapq.heappush(pq, cls._SearchNode(root_bound, 0, 0, 0))

        n = len(items)
        while pq:
            curr = heapq.heappop(pq)

            if curr.bound <= incumbent:
                continue

            if curr.level == n:
                continue

            next_it = items[curr.level]

            # Branch 1: Include item
            if curr.weight + next_it.weight <= capacity:
                left_val = curr.value + next_it.value
                left_wt = curr.weight + next_it.weight
                left_bound = cls._compute_bound(curr.level + 1, left_val, left_wt, capacity, items)

                if left_val > incumbent:
                    incumbent = left_val
                if left_bound > incumbent:
                    heapq.heappush(pq, cls._SearchNode(left_bound, curr.level + 1, left_val, left_wt))

            # Branch 2: Exclude item
            right_bound = cls._compute_bound(curr.level + 1, curr.value, curr.weight, capacity, items)
            if right_bound > incumbent:
                heapq.heappush(pq, cls._SearchNode(right_bound, curr.level + 1, curr.value, curr.weight))

        return incumbent + total_value


class DynamicProgrammingOracle:
    """Exact Dynamic Programming Oracle for 0/1 Knapsack."""

    @staticmethod
    def solve(items: List[KnapsackItem], capacity: int) -> int:
        if capacity <= 0 or not items:
            return 0

        zero_val = 0
        filtered: List[KnapsackItem] = []
        for it in items:
            if it.weight == 0:
                zero_val += it.value
            elif it.weight <= capacity:
                filtered.append(it)

        dp = [0] * (capacity + 1)
        for it in filtered:
            for w in range(capacity, it.weight - 1, -1):
                dp[w] = max(dp[w], dp[w - it.weight] + it.value)

        return dp[capacity] + zero_val


class TestBranchAndBound(unittest.TestCase):
    def test_textbook_instance(self):
        textbook = [
            KnapsackItem(40, 2),
            KnapsackItem(42, 5),
            KnapsackItem(25, 7),
            KnapsackItem(12, 3),
        ]
        cap = 10
        bnb_res = BranchAndBoundKnapsack.solve(textbook, cap)
        dp_res = DynamicProgrammingOracle.solve(textbook, cap)

        self.assertEqual(bnb_res, 94)
        self.assertEqual(bnb_res, dp_res)

    def test_differential_fuzzing(self):
        rng = random.Random(42)
        for _ in range(50):
            n = 15
            capacity = rng.randint(20, 150)
            items = [KnapsackItem(rng.randint(1, 50), rng.randint(1, 30)) for _ in range(n)]

            bnb_ans = BranchAndBoundKnapsack.solve(items, capacity)
            dp_ans = DynamicProgrammingOracle.solve(items, capacity)
            self.assertEqual(bnb_ans, dp_ans)

    def test_edge_cases(self):
        self.assertEqual(BranchAndBoundKnapsack.solve([], 100), 0)
        items = [KnapsackItem(10, 5)]
        self.assertEqual(BranchAndBoundKnapsack.solve(items, 0), 0)
        # All items fit
        items2 = [KnapsackItem(10, 2), KnapsackItem(20, 3)]
        self.assertEqual(BranchAndBoundKnapsack.solve(items2, 10), 30)


if __name__ == '__main__':
    unittest.main()
