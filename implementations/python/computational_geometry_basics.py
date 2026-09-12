"""Computational Geometry Fundamentals: 2D Points, Vectors, Exact Orientation,
Segment Intersection, Polygon Area, and Point-in-Polygon Tests.

Implements Arthur's Two-Layer API:
  - Layer 1: Low-level mathematical predicates using exact Python integer arithmetic.
  - Layer 2: Safe, high-level GeometryEngine with bounding box filtering, polygon checks,
             and distance measurements.
"""

from __future__ import annotations
import math
import unittest
from enum import Enum
from typing import NamedTuple, List, Optional, Tuple


class Orientation(Enum):
    COLLINEAR = 0
    COUNTER_CLOCKWISE = 1  # Left turn
    CLOCKWISE = -1         # Right turn


class PointLocation(Enum):
    OUTSIDE = 0
    ON_BOUNDARY = 1
    INSIDE = 2


class Point2D(NamedTuple):
    x: int | float
    y: int | float

    def __add__(self, other: Point2D) -> Point2D:
        return Point2D(self.x + other.x, self.y + other.y)

    def __sub__(self, other: Point2D) -> Point2D:
        return Point2D(self.x - other.x, self.y - other.y)

    def __mul__(self, scalar: int | float) -> Point2D:
        return Point2D(self.x * scalar, self.y * scalar)

    def dot(self, other: Point2D) -> int | float:
        return self.x * other.x + self.y * other.y

    def cross(self, other: Point2D) -> int | float:
        return self.x * other.y - self.y * other.x


# ============================================================================
# Layer 1: Low-Level Primitives & Exact Predicates
# ============================================================================

def cross_product_exact(a: Point2D, b: Point2D, c: Point2D) -> int | float:
    """Computes cross product of vectors (b - a) and (c - a)."""
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)


def orientation(a: Point2D, b: Point2D, c: Point2D) -> Orientation:
    """Returns orientation of triplet (a, b, c): CCW, CW, or COLLINEAR."""
    cp = cross_product_exact(a, b, c)
    if cp > 0:
        return Orientation.COUNTER_CLOCKWISE
    if cp < 0:
        return Orientation.CLOCKWISE
    return Orientation.COLLINEAR


def on_segment_collinear(p: Point2D, q: Point2D, r: Point2D) -> bool:
    """Checks if point q lies on segment pr, assuming p, q, r are collinear."""
    return (min(p.x, r.x) <= q.x <= max(p.x, r.x) and
            min(p.y, r.y) <= q.y <= max(p.y, r.y))


def segments_intersect(p1: Point2D, q1: Point2D, p2: Point2D, q2: Point2D) -> bool:
    """Determines whether line segments p1q1 and p2q2 intersect."""
    o1 = orientation(p1, q1, p2)
    o2 = orientation(p1, q1, q2)
    o3 = orientation(p2, q2, p1)
    o4 = orientation(p2, q2, q1)

    # General crossing case
    if o1 != o2 and o3 != o4:
        return True

    # Collinear overlapping cases
    if o1 == Orientation.COLLINEAR and on_segment_collinear(p1, p2, q1):
        return True
    if o2 == Orientation.COLLINEAR and on_segment_collinear(p1, q2, q1):
        return True
    if o3 == Orientation.COLLINEAR and on_segment_collinear(p2, p1, q2):
        return True
    if o4 == Orientation.COLLINEAR and on_segment_collinear(p2, q1, q2):
        return True

    return False


def polygon_area_2x(vertices: List[Point2D]) -> int | float:
    """Computes twice the signed area of a simple polygon via Shoelace formula."""
    n = len(vertices)
    if n < 3:
        return 0
    area2 = 0
    for i in range(n):
        j = (i + 1) % n
        area2 += vertices[i].x * vertices[j].y - vertices[j].x * vertices[i].y
    return area2


def point_in_polygon(pt: Point2D, poly: List[Point2D]) -> PointLocation:
    """Tests if pt lies INSIDE, OUTSIDE, or ON_BOUNDARY of poly via Ray-Casting."""
    n = len(poly)
    if n < 3:
        return PointLocation.OUTSIDE

    # Boundary check
    for i in range(n):
        j = (i + 1) % n
        if orientation(poly[i], poly[j], pt) == Orientation.COLLINEAR:
            if on_segment_collinear(poly[i], pt, poly[j]):
                return PointLocation.ON_BOUNDARY

    # Ray casting along horizontal line (y = pt.y, x >= pt.x)
    inside = False
    for i in range(n):
        j = (i + 1) % n
        p = poly[i]
        q = poly[j]

        # Order by y-coordinate
        if p.y > q.y:
            p, q = q, p

        # Half-open vertical interval test: p.y <= pt.y < q.y
        if p.y <= pt.y < q.y:
            # Check if pt is strictly to the left of directed edge p -> q
            if orientation(p, q, pt) == Orientation.COUNTER_CLOCKWISE:
                inside = not inside

    return PointLocation.INSIDE if inside else PointLocation.OUTSIDE


# ============================================================================
# Layer 2: Safe High-Level Geometry Engine
# ============================================================================

class GeometryEngine:
    @staticmethod
    def do_segments_intersect(p1: Point2D, q1: Point2D, p2: Point2D, q2: Point2D) -> bool:
        """High-level segment intersection with bounding box fast rejection."""
        if (max(p1.x, q1.x) < min(p2.x, q2.x) or
            max(p2.x, q2.x) < min(p1.x, q1.x) or
            max(p1.y, q1.y) < min(p2.y, q2.y) or
            max(p2.y, q2.y) < min(p1.y, q1.y)):
            return False
        return segments_intersect(p1, q1, p2, q2)

    @staticmethod
    def polygon_area(vertices: List[Point2D]) -> float:
        """Returns absolute area of simple polygon."""
        return abs(polygon_area_2x(vertices)) / 2.0

    @staticmethod
    def locate_point(pt: Point2D, polygon: List[Point2D]) -> PointLocation:
        """Returns location of point relative to polygon."""
        return point_in_polygon(pt, polygon)

    @staticmethod
    def point_to_segment_distance(pt: Point2D, a: Point2D, b: Point2D) -> float:
        """Calculates Euclidean distance from point pt to segment ab."""
        ab = b - a
        ap = pt - a
        ab_len_sq = ab.x * ab.x + ab.y * ab.y
        if ab_len_sq < 1e-15:
            return math.hypot(ap.x, ap.y)

        t = max(0.0, min(1.0, (ap.x * ab.x + ap.y * ab.y) / ab_len_sq))
        closest = Point2D(a.x + t * ab.x, a.y + t * ab.y)
        return math.hypot(pt.x - closest.x, pt.y - closest.y)

    @staticmethod
    def line_segment_intersection_point(p1: Point2D, q1: Point2D, p2: Point2D, q2: Point2D) -> Optional[Point2D]:
        """Calculates floating-point intersection coordinate of two segments if intersecting."""
        d1 = q1 - p1
        d2 = q2 - p2
        denom = d1.x * d2.y - d1.y * d2.x
        if abs(denom) < 1e-15:
            return None

        dp = p2 - p1
        t = (dp.x * d2.y - dp.y * d2.x) / denom
        u = (dp.x * d1.y - dp.y * d1.x) / denom

        if 0.0 <= t <= 1.0 and 0.0 <= u <= 1.0:
            return Point2D(p1.x + t * d1.x, p1.y + t * d1.y)
        return None


# ============================================================================
# Unit & Differential Tests
# ============================================================================

class TestComputationalGeometryBasics(unittest.TestCase):
    def test_orientation(self):
        a = Point2D(0, 0)
        b = Point2D(4, 0)
        c = Point2D(2, 2)
        self.assertEqual(orientation(a, b, c), Orientation.COUNTER_CLOCKWISE)
        self.assertEqual(orientation(b, a, c), Orientation.CLOCKWISE)
        self.assertEqual(orientation(a, b, Point2D(8, 0)), Orientation.COLLINEAR)

    def test_massive_integer_precision(self):
        # Stress test: 10^18 coordinate values (Python handles arbitrarily large integers)
        p1 = Point2D(10**18, 10**18)
        p2 = Point2D(2 * 10**18, 2 * 10**18)
        p3 = Point2D(3 * 10**18, 3 * 10**18 + 1)
        self.assertEqual(orientation(p1, p2, p3), Orientation.COUNTER_CLOCKWISE)

    def test_segment_intersection(self):
        eng = GeometryEngine()
        # Proper cross
        self.assertTrue(eng.do_segments_intersect(Point2D(0, 0), Point2D(4, 4), Point2D(0, 4), Point2D(4, 0)))
        # Parallel disjoint
        self.assertFalse(eng.do_segments_intersect(Point2D(0, 0), Point2D(4, 4), Point2D(0, 1), Point2D(4, 5)))
        # Collinear overlapping
        self.assertTrue(eng.do_segments_intersect(Point2D(0, 0), Point2D(4, 4), Point2D(2, 2), Point2D(6, 6)))
        # Collinear disjoint
        self.assertFalse(eng.do_segments_intersect(Point2D(0, 0), Point2D(4, 4), Point2D(5, 5), Point2D(8, 8)))
        # T-junction
        self.assertTrue(eng.do_segments_intersect(Point2D(2, 0), Point2D(2, 2), Point2D(0, 2), Point2D(4, 2)))

    def test_polygon_area(self):
        eng = GeometryEngine()
        square = [Point2D(0, 0), Point2D(4, 0), Point2D(4, 4), Point2D(0, 4)]
        self.assertAlmostEqual(eng.polygon_area(square), 16.0)

        triangle = [Point2D(0, 0), Point2D(5, 0), Point2D(0, 6)]
        self.assertAlmostEqual(eng.polygon_area(triangle), 15.0)

    def test_point_in_polygon(self):
        eng = GeometryEngine()
        # L-shaped polygon:
        # (0,0) -> (4,0) -> (4,2) -> (2,2) -> (2,4) -> (0,4)
        l_poly = [
            Point2D(0, 0), Point2D(4, 0), Point2D(4, 2),
            Point2D(2, 2), Point2D(2, 4), Point2D(0, 4)
        ]

        self.assertEqual(eng.locate_point(Point2D(1, 1), l_poly), PointLocation.INSIDE)
        self.assertEqual(eng.locate_point(Point2D(1, 3), l_poly), PointLocation.INSIDE)
        self.assertEqual(eng.locate_point(Point2D(3, 3), l_poly), PointLocation.OUTSIDE)
        self.assertEqual(eng.locate_point(Point2D(2, 2), l_poly), PointLocation.ON_BOUNDARY)
        self.assertEqual(eng.locate_point(Point2D(0, 2), l_poly), PointLocation.ON_BOUNDARY)

    def test_point_to_segment_distance(self):
        eng = GeometryEngine()
        a = Point2D(0, 0)
        b = Point2D(10, 0)
        p = Point2D(5, 5)
        self.assertAlmostEqual(eng.point_to_segment_distance(p, a, b), 5.0)

        p_outside = Point2D(15, 0)
        self.assertAlmostEqual(eng.point_to_segment_distance(p_outside, a, b), 5.0)

    def test_intersection_point(self):
        eng = GeometryEngine()
        pt = eng.line_segment_intersection_point(Point2D(0, 0), Point2D(4, 4), Point2D(0, 4), Point2D(4, 0))
        self.assertIsNotNone(pt)
        self.assertAlmostEqual(pt.x, 2.0)
        self.assertAlmostEqual(pt.y, 2.0)


if __name__ == "__main__":
    unittest.main()
