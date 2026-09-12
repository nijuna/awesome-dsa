"""Closest Pair of Points via Divide-and-Conquer in O(N log N) Time.

Implements Arthur's Two-Layer API:
  - Layer 1: Exact integer squared distance primitives, strip filtering,
             and geometric packing constant checks.
  - Layer 2: Safe ClosestPairEngine with divide-and-conquer recursion,
             y-coordinate merging, input validation, and differential verification.
"""

from __future__ import annotations
import math
import random
import unittest
from typing import NamedTuple, List, Tuple


class Point(NamedTuple):
    id: int
    x: int
    y: int


def dist_sq_exact(a: Point, b: Point) -> int:
    return (a.x - b.x) ** 2 + (a.y - b.y) ** 2


class ClosestPairResult(NamedTuple):
    p1: Point
    p2: Point
    distance_squared: int
    distance: float


class ClosestPairEngine:
    @staticmethod
    def _closest_pair_rec(pts_by_x: List[Point]) -> Tuple[ClosestPairResult, List[Point]]:
        n = len(pts_by_x)
        if n <= 3:
            best_d_sq = float('inf')
            best_pair = (pts_by_x[0], pts_by_x[1] if n > 1 else pts_by_x[0])
            for i in range(n):
                for j in range(i + 1, n):
                    d = dist_sq_exact(pts_by_x[i], pts_by_x[j])
                    if d < best_d_sq:
                        best_d_sq = d
                        best_pair = (pts_by_x[i], pts_by_x[j])

            sorted_by_y = sorted(pts_by_x, key=lambda p: (p.y, p.x))
            res = ClosestPairResult(
                min(best_pair, key=lambda p: p.id),
                max(best_pair, key=lambda p: p.id),
                int(best_d_sq),
                math.sqrt(best_d_sq) if best_d_sq != float('inf') else 0.0
            )
            return res, sorted_by_y

        mid = n // 2
        mid_x = pts_by_x[mid].x

        left_res, left_y = ClosestPairEngine._closest_pair_rec(pts_by_x[:mid])
        right_res, right_y = ClosestPairEngine._closest_pair_rec(pts_by_x[mid:])

        best = left_res if left_res.distance_squared < right_res.distance_squared else right_res
        delta_sq = best.distance_squared

        # Merge left_y and right_y into single list sorted by y in O(N)
        merged_y: List[Point] = []
        i = j = 0
        while i < len(left_y) and j < len(right_y):
            if left_y[i].y <= right_y[j].y:
                merged_y.append(left_y[i])
                i += 1
            else:
                merged_y.append(right_y[j])
                j += 1
        merged_y.extend(left_y[i:])
        merged_y.extend(right_y[j:])

        # Build vertical strip: points within delta of mid_x
        strip = [p for p in merged_y if (p.x - mid_x) ** 2 < delta_sq]

        # Scan strip: each point checked against at most 7 following points
        strip_len = len(strip)
        for i in range(strip_len):
            for j in range(i + 1, strip_len):
                dy = strip[j].y - strip[i].y
                if dy * dy >= delta_sq:
                    break  # Since strip is sorted by y, no subsequent point can be closer
                d = dist_sq_exact(strip[i], strip[j])
                if d < delta_sq:
                    delta_sq = d
                    p_a, p_b = strip[i], strip[j]
                    if p_b.id < p_a.id:
                        p_a, p_b = p_b, p_a
                    best = ClosestPairResult(p_a, p_b, delta_sq, math.sqrt(delta_sq))

        return best, merged_y

    @classmethod
    def find_closest_pair(cls, points: List[Point]) -> ClosestPairResult:
        """Finds closest pair of points in O(N log N) time."""
        if len(points) < 2:
            raise ValueError("Closest pair requires at least 2 points.")

        pts_by_x = sorted(points, key=lambda p: (p.x, p.y))
        res, _ = cls._closest_pair_rec(pts_by_x)
        return res

    @staticmethod
    def naive_closest_pair(points: List[Point]) -> ClosestPairResult:
        """Naive O(N^2) brute-force oracle."""
        n = len(points)
        if n < 2:
            raise ValueError("Requires at least 2 points.")

        best_d_sq = float('inf')
        best_p1, best_p2 = points[0], points[1]

        for i in range(n):
            for j in range(i + 1, n):
                d = dist_sq_exact(points[i], points[j])
                if d < best_d_sq:
                    best_d_sq = d
                    best_p1, best_p2 = points[i], points[j]

        if best_p2.id < best_p1.id:
            best_p1, best_p2 = best_p2, best_p1

        return ClosestPairResult(best_p1, best_p2, int(best_d_sq), math.sqrt(best_d_sq))


# ============================================================================
# Unit & Differential Tests
# ============================================================================

class TestClosestPair(unittest.TestCase):
    def test_basic_triangle(self):
        pts = [
            Point(0, 0, 0),
            Point(1, 10, 0),
            Point(2, 0, 1)
        ]
        res = ClosestPairEngine.find_closest_pair(pts)
        self.assertEqual(res.distance_squared, 1)
        self.assertEqual({res.p1.id, res.p2.id}, {0, 2})

    def test_identical_points(self):
        pts = [
            Point(0, 5, 5),
            Point(1, 100, 100),
            Point(2, 5, 5)
        ]
        res = ClosestPairEngine.find_closest_pair(pts)
        self.assertEqual(res.distance_squared, 0)
        self.assertEqual(res.distance, 0.0)

    def test_horizontal_line(self):
        pts = [
            Point(0, 10, 5),
            Point(1, 25, 5),
            Point(2, 12, 5),
            Point(3, 40, 5)
        ]
        res = ClosestPairEngine.find_closest_pair(pts)
        self.assertEqual(res.distance_squared, 4)
        self.assertAlmostEqual(res.distance, 2.0)

    def test_differential_random_stress(self):
        rng = random.Random(1337)
        for _ in range(50):
            n = 35
            pts = [Point(i, rng.randint(-5000, 5000), rng.randint(-5000, 5000)) for i in range(n)]
            fast = ClosestPairEngine.find_closest_pair(pts)
            naive = ClosestPairEngine.naive_closest_pair(pts)
            self.assertEqual(fast.distance_squared, naive.distance_squared)
            self.assertAlmostEqual(fast.distance, naive.distance)


if __name__ == "__main__":
    unittest.main()
