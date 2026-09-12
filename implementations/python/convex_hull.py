"""Convex Hull Algorithms: Andrew's Monotone Chain O(N log N) and
Jarvis March (Gift Wrapping) O(N * H) Differential Oracle.

Implements Arthur's Two-Layer API:
  - Layer 1: Low-level exact geometric orientation predicates and stack hull maintenance.
  - Layer 2: High-level ConvexHullEngine supporting strict/weak hulls, canonicalization,
             and differential verification against Jarvis March.
"""

from __future__ import annotations
import random
import unittest
from typing import NamedTuple, List, Sequence


class Point(NamedTuple):
    x: int
    y: int

    def __lt__(self, other: Point) -> bool:
        if self.x != other.x:
            return self.x < other.x
        return self.y < other.y


def cross_product_exact(a: Point, b: Point, c: Point) -> int:
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)


def orientation(a: Point, b: Point, c: Point) -> int:
    cp = cross_product_exact(a, b, c)
    if cp > 0:
        return 1  # CCW (left turn)
    if cp < 0:
        return -1  # CW (right turn)
    return 0  # Collinear


def dist_sq(a: Point, b: Point) -> int:
    return (a.x - b.x) ** 2 + (a.y - b.y) ** 2


class ConvexHullEngine:
    @staticmethod
    def monotone_chain(points: Sequence[Point], include_collinear: bool = False) -> List[Point]:
        """Computes convex hull via Andrew's Monotone Chain in O(N log N).

        Returns vertices in counter-clockwise order.
        """
        pts = sorted(list(set(points)))
        n = len(pts)
        if n <= 2:
            return pts

        hull: List[Point] = []

        # Lower hull (left to right)
        for p in pts:
            while len(hull) >= 2:
                o = orientation(hull[-2], hull[-1], p)
                if include_collinear:
                    if o < 0:
                        hull.pop()
                    else:
                        break
                else:
                    if o <= 0:
                        hull.pop()
                    else:
                        break
            hull.append(p)

        # Upper hull (right to left)
        lower_len = len(hull)
        for p in reversed(pts[:-1]):
            while len(hull) > lower_len:
                o = orientation(hull[-2], hull[-1], p)
                if include_collinear:
                    if o < 0:
                        hull.pop()
                    else:
                        break
                else:
                    if o <= 0:
                        hull.pop()
                    else:
                        break
            hull.append(p)

        hull.pop()  # Remove duplicate of first point
        return hull

    @staticmethod
    def jarvis_march(points: Sequence[Point]) -> List[Point]:
        """Jarvis March (Gift Wrapping) algorithm in O(N * H) time.

        Serves as an independent differential verification oracle.
        """
        pts = sorted(list(set(points)))
        n = len(pts)
        if n <= 2:
            return pts

        # Start at leftmost point
        start_idx = 0
        for i in range(1, n):
            if pts[i].x < pts[start_idx].x or (pts[i].x == pts[start_idx].x and pts[i].y < pts[start_idx].y):
                start_idx = i

        hull: List[Point] = []
        current = start_idx

        while True:
            hull.append(pts[current])
            next_pt = (current + 1) % n

            for i in range(n):
                if i == current:
                    continue
                o = orientation(pts[current], pts[next_pt], pts[i])
                if o == -1:  # i is to the right of current -> next_pt (CCW wrap)
                    next_pt = i
                elif o == 0:  # Collinear: choose farther point
                    if dist_sq(pts[current], pts[i]) > dist_sq(pts[current], pts[next_pt]):
                        next_pt = i

            current = next_pt
            if current == start_idx:
                break

        return hull

    @staticmethod
    def canonicalize(hull: Sequence[Point]) -> List[Point]:
        """Rotates convex hull vertices so that the lexicographically smallest point is at index 0."""
        if not hull:
            return []
        h = list(hull)
        min_idx = min(range(len(h)), key=lambda i: (h[i].x, h[i].y))
        return h[min_idx:] + h[:min_idx]


# ============================================================================
# Unit & Differential Tests
# ============================================================================

class TestConvexHull(unittest.TestCase):
    def test_basic_hulls(self):
        # Empty and single point
        self.assertEqual(ConvexHullEngine.monotone_chain([]), [])
        self.assertEqual(ConvexHullEngine.monotone_chain([Point(5, 5)]), [Point(5, 5)])

        # Square with interior points
        pts = [
            Point(0, 0), Point(4, 0), Point(4, 4), Point(0, 4),
            Point(1, 1), Point(2, 2), Point(3, 1), Point(1, 3)
        ]
        hull = ConvexHullEngine.monotone_chain(pts)
        canon = ConvexHullEngine.canonicalize(hull)
        expected = [Point(0, 0), Point(4, 0), Point(4, 4), Point(0, 4)]
        self.assertEqual(canon, expected)

    def test_collinear_edges(self):
        coll = [
            Point(0, 0), Point(2, 0), Point(4, 0),
            Point(4, 2), Point(4, 4), Point(0, 4)
        ]
        strict = ConvexHullEngine.monotone_chain(coll, False)
        weak = ConvexHullEngine.monotone_chain(coll, True)
        self.assertEqual(len(strict), 4)
        self.assertEqual(len(weak), 6)

    def test_differential_monotone_chain_vs_jarvis_march(self):
        rng = random.Random(42)
        for _ in range(50):
            n = 40
            pts = [Point(rng.randint(-500, 500), rng.randint(-500, 500)) for _ in range(n)]
            mc = ConvexHullEngine.canonicalize(ConvexHullEngine.monotone_chain(pts, False))
            jm = ConvexHullEngine.canonicalize(ConvexHullEngine.jarvis_march(pts))
            self.assertEqual(len(mc), len(jm))
            self.assertEqual(mc, jm)


if __name__ == "__main__":
    unittest.main()
