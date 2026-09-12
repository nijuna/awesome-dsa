"""
Greedy Exchange Arguments Verification in Python.

Implements Interval Scheduling (Earliest Finish Time) and Minimizing Lateness (EDD)
with brute-force differential testing proving greedy optimality via the exchange property.
"""

import itertools
import unittest
from typing import List, NamedTuple


class Interval(NamedTuple):
    start: int
    finish: int
    id: int


def interval_scheduling_greedy(intervals: List[Interval]) -> List[Interval]:
    sorted_ivs = sorted(intervals, key=lambda x: (x.finish, x.start))
    selected: List[Interval] = []
    last_finish = -1

    for iv in sorted_ivs:
        if iv.start >= last_finish:
            selected.append(iv)
            last_finish = iv.finish

    return selected


def interval_scheduling_brute_force(intervals: List[Interval]) -> int:
    n = len(intervals)
    max_len = 0

    for r in range(1, n + 1):
        for subset in itertools.combinations(intervals, r):
            sorted_sub = sorted(subset, key=lambda x: x.start)
            compatible = all(
                sorted_sub[i].start >= sorted_sub[i - 1].finish
                for i in range(1, len(sorted_sub))
            )
            if compatible:
                max_len = max(max_len, len(sorted_sub))

    return max_len


class Job(NamedTuple):
    duration: int
    deadline: int
    id: int


def minimize_lateness_edd(jobs: List[Job]) -> int:
    sorted_jobs = sorted(jobs, key=lambda x: x.deadline)
    curr_time = 0
    max_l = 0
    for j in sorted_jobs:
        curr_time += j.duration
        lateness = max(0, curr_time - j.deadline)
        max_l = max(max_l, lateness)
    return max_l


def minimize_lateness_brute_force(jobs: List[Job]) -> int:
    min_l = float("inf")
    for p in itertools.permutations(jobs):
        curr_time = 0
        max_l = 0
        for j in p:
            curr_time += j.duration
            l = max(0, curr_time - j.deadline)
            max_l = max(max_l, l)
        min_l = min(min_l, max_l)
    return int(min_l)


class TestExchangeArguments(unittest.TestCase):
    def test_interval_scheduling_optimality(self):
        intervals = [
            Interval(1, 4, 1), Interval(3, 5, 2), Interval(0, 6, 3),
            Interval(5, 7, 4), Interval(3, 9, 5), Interval(5, 9, 6),
            Interval(6, 10, 7), Interval(8, 11, 8), Interval(8, 12, 9),
            Interval(2, 14, 10), Interval(12, 16, 11)
        ]
        greedy = interval_scheduling_greedy(intervals)
        brute_cardinality = interval_scheduling_brute_force(intervals)
        self.assertEqual(len(greedy), brute_cardinality)
        self.assertEqual(len(greedy), 4)

    def test_minimizing_lateness_optimality(self):
        jobs = [
            Job(3, 6, 1),
            Job(2, 8, 2),
            Job(1, 9, 3),
            Job(4, 9, 4),
            Job(3, 14, 5),
            Job(2, 15, 6)
        ]
        edd = minimize_lateness_edd(jobs)
        brute = minimize_lateness_brute_force(jobs)
        self.assertEqual(edd, brute)


if __name__ == "__main__":
    unittest.main()
