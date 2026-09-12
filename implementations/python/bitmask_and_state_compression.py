"""
Reference Implementation: Bitmask and State Compression Dynamic Programming
Demonstrates:
1. Held-Karp Travelling Salesperson Problem (TSP): O(n^2 2^n) time, O(n 2^n) space with tour reconstruction.
2. Assignment / Bipartite Matching DP: O(n 2^n) time, O(2^n) space with assignment permutation reconstruction.
3. Submask Iteration: Enumerating all submasks in O(3^n) for subset partitioning / bin packing.
4. Sum Over Subsets (SOS DP): Multidimensional prefix sums on the hypercube in O(n 2^n) time.

Language: Python 3
"""

import unittest
from typing import List, Tuple


# ============================================================================
# 1. Held-Karp Travelling Salesperson Problem (TSP)
# ============================================================================

def tsp_held_karp(dist: List[List[int]]) -> int:
    """
    Held-Karp TSP: Computes minimum cost Hamiltonian cycle starting and ending at node 0.
    Time: O(n^2 2^n), Space: O(n 2^n).
    """
    n = len(dist)
    if n <= 1:
        return 0

    full = 1 << n
    inf = float('inf')

    dp = [[inf] * n for _ in range(full)]
    dp[1][0] = 0

    for mask in range(1, full):
        for u in range(n):
            if not (mask & (1 << u)):
                continue
            if dp[mask][u] == inf:
                continue

            for v in range(n):
                if mask & (1 << v):
                    continue
                next_mask = mask | (1 << v)
                cand = dp[mask][u] + dist[u][v]
                if cand < dp[next_mask][v]:
                    dp[next_mask][v] = cand

    all_mask = full - 1
    ans = inf
    for u in range(1, n):
        if dp[all_mask][u] != inf:
            ans = min(ans, dp[all_mask][u] + dist[u][0])

    return int(ans)


def tsp_held_karp_reconstruct(dist: List[List[int]]) -> Tuple[int, List[int]]:
    """
    Held-Karp TSP with full tour reconstruction.
    Returns (min_cost, tour_sequence).
    """
    n = len(dist)
    if n <= 0:
        return 0, []
    if n == 1:
        return 0, [0, 0]

    full = 1 << n
    inf = float('inf')

    dp = [[inf] * n for _ in range(full)]
    parent = [[-1] * n for _ in range(full)]
    dp[1][0] = 0

    for mask in range(1, full):
        for u in range(n):
            if not (mask & (1 << u)):
                continue
            if dp[mask][u] == inf:
                continue

            for v in range(n):
                if mask & (1 << v):
                    continue
                next_mask = mask | (1 << v)
                cand = dp[mask][u] + dist[u][v]
                if cand < dp[next_mask][v]:
                    dp[next_mask][v] = cand
                    parent[next_mask][v] = u

    all_mask = full - 1
    best_cost = inf
    last = -1

    for u in range(1, n):
        if dp[all_mask][u] != inf:
            cand = dp[all_mask][u] + dist[u][0]
            if cand < best_cost:
                best_cost = cand
                last = u

    route = []
    mask = all_mask
    cur = last

    while cur != -1:
        route.append(cur)
        prev = parent[mask][cur]
        mask ^= (1 << cur)
        cur = prev

    route.reverse()
    route.append(0)
    return int(best_cost), route


# ============================================================================
# 2. Assignment / Perfect Matching DP
# ============================================================================

def assignment_dp(cost: List[List[int]]) -> int:
    """
    Assignment Problem: Finds minimum cost to assign n workers to n tasks.
    worker k = popcount(mask) is assigned to task j not in mask.
    Time: O(n 2^n), Space: O(2^n).
    """
    n = len(cost)
    if n == 0:
        return 0

    full = 1 << n
    inf = float('inf')

    dp = [inf] * full
    dp[0] = 0

    for mask in range(full):
        worker = mask.bit_count()
        if worker >= n:
            continue

        for task in range(n):
            if mask & (1 << task):
                continue
            next_mask = mask | (1 << task)
            cand = dp[mask] + cost[worker][task]
            if cand < dp[next_mask]:
                dp[next_mask] = cand

    return int(dp[full - 1])


def assignment_dp_reconstruct(cost: List[List[int]]) -> Tuple[int, List[int]]:
    """
    Reconstructs the optimal task assignment vector, where assignment[i] is the task assigned to worker i.
    """
    n = len(cost)
    if n == 0:
        return 0, []

    full = 1 << n
    inf = float('inf')

    dp = [inf] * full
    parent_mask = [-1] * full
    chosen_task = [-1] * full
    dp[0] = 0

    for mask in range(full):
        worker = mask.bit_count()
        if worker >= n:
            continue

        for task in range(n):
            if mask & (1 << task):
                continue
            next_mask = mask | (1 << task)
            cand = dp[mask] + cost[worker][task]
            if cand < dp[next_mask]:
                dp[next_mask] = cand
                parent_mask[next_mask] = mask
                chosen_task[next_mask] = task

    assignment = [-1] * n
    mask = full - 1
    while mask > 0:
        pm = parent_mask[mask]
        task = chosen_task[mask]
        worker = pm.bit_count()
        assignment[worker] = task
        mask = pm

    return int(dp[full - 1]), assignment


# ============================================================================
# 3. Submask Iteration: Partitioning into Valid Subsets
# ============================================================================

def min_subsets_partition(n: int, is_valid_subset: List[bool]) -> int:
    """
    Computes minimum number of valid subsets needed to partition a set of elements.
    Iterates submasks in O(3^n) total time.
    """
    full = 1 << n
    inf = float('inf')
    dp = [inf] * full
    dp[0] = 0

    for mask in range(1, full):
        sub = mask
        while sub > 0:
            if is_valid_subset[sub] and dp[mask ^ sub] != inf:
                cand = dp[mask ^ sub] + 1
                if cand < dp[mask]:
                    dp[mask] = cand
            sub = (sub - 1) & mask

    return -1 if dp[full - 1] == inf else int(dp[full - 1])


# ============================================================================
# 4. Sum Over Subsets (SOS DP)
# ============================================================================

def sos_dp_sum(f: List[int], n: int) -> List[int]:
    r"""
    SOS DP: Computes F[mask] = sum_{sub \subseteq mask} A[sub] in O(n 2^n) time.
    Multidimensional prefix sums over the n-dimensional Boolean hypercube.
    """
    f = f[:]
    full = 1 << n

    for bit in range(n):
        for mask in range(full):
            if mask & (1 << bit):
                f[mask] += f[mask ^ (1 << bit)]

    return f


# ============================================================================
# Unit Tests
# ============================================================================

class TestBitmaskDP(unittest.TestCase):
    def test_held_karp_tsp(self):
        dist = [
            [0, 10, 15, 20],
            [10, 0, 35, 25],
            [15, 35, 0, 30],
            [20, 25, 30, 0]
        ]

        cost = tsp_held_karp(dist)
        self.assertEqual(cost, 80)

        rec_cost, tour = tsp_held_karp_reconstruct(dist)
        self.assertEqual(rec_cost, 80)
        self.assertEqual(len(tour), 5)
        self.assertEqual(tour[0], 0)
        self.assertEqual(tour[-1], 0)

        check = sum(dist[tour[i]][tour[i + 1]] for i in range(len(tour) - 1))
        self.assertEqual(check, 80)

    def test_assignment_dp(self):
        cost = [
            [9, 2, 7],
            [6, 4, 3],
            [5, 8, 1]
        ]

        min_c = assignment_dp(cost)
        self.assertEqual(min_c, 9)

        rec_c, assign = assignment_dp_reconstruct(cost)
        self.assertEqual(rec_c, 9)
        self.assertEqual(assign, [1, 0, 2])

    def test_submask_partition(self):
        n = 4
        valid = [False] * (1 << n)
        for i in range(n):
            valid[1 << i] = True
        valid[(1 << 0) | (1 << 1)] = True
        valid[(1 << 2) | (1 << 3)] = True

        parts = min_subsets_partition(n, valid)
        self.assertEqual(parts, 2)

    def test_sos_dp(self):
        n = 3
        a = [1] * (1 << n)
        f = sos_dp_sum(a, n)
        for mask in range(1 << n):
            self.assertEqual(f[mask], 1 << mask.bit_count())

        a2 = [1, 2, 4, 8]
        f2 = sos_dp_sum(a2, 2)
        self.assertEqual(f2, [1, 3, 5, 15])

    def test_single_element_edge_case(self):
        self.assertEqual(tsp_held_karp([[0]]), 0)
        self.assertEqual(assignment_dp([[42]]), 42)
        c, a = assignment_dp_reconstruct([[42]])
        self.assertEqual(c, 42)
        self.assertEqual(a, [0])


if __name__ == '__main__':
    unittest.main()
