"""Line Sweep Algorithms for Segment Intersections.

Implements Arthur's Two-Layer API:
  - Layer 1: Exact geometric predicates (orientation, intersection testing),
             sweep-line status tracking, and event structures.
  - Layer 2: High-level LineSweepEngine supporting:
             1. has_any_intersection() [Shamos-Hoey O(N log N)]
             2. find_orthogonal_intersections() [Orthogonal Plane Sweep O((N + K) log N)]
             3. naive_all_intersections() [O(N^2) differential oracle]
             4. naive_orthogonal_intersections() [O(N^2) differential oracle]
"""

from __future__ import annotations
import bisect
import random
import unittest
from typing import NamedTuple, List, Tuple, Optional


class Point(NamedTuple):
    x: int
    y: int

    def __lt__(self, other: Point) -> bool:
        if self.x != other.x:
            return self.x < other.x
        return self.y < other.y


class Segment:
    def __init__(self, seg_id: int, a: Point, b: Point):
        self.id = seg_id
        if b < a:
            self.p1 = b
            self.p2 = a
        else:
            self.p1 = a
            self.p2 = b

    def eval_y(self, sweep_x: float) -> float:
        if self.p1.x == self.p2.x:
            return float(self.p1.y)
        t = (sweep_x - self.p1.x) / (self.p2.x - self.p1.x)
        return float(self.p1.y) + t * float(self.p2.y - self.p1.y)

    def __repr__(self) -> str:
        return f"Segment({self.id}, {self.p1}, {self.p2})"


# ============================================================================
# Layer 1: Geometric Primitives
# ============================================================================

def cross_product_exact(a: Point, b: Point, c: Point) -> int:
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)


def orientation(a: Point, b: Point, c: Point) -> int:
    cp = cross_product_exact(a, b, c)
    if cp > 0:
        return 1  # CCW
    if cp < 0:
        return -1  # CW
    return 0  # Collinear


def on_segment_collinear(p: Point, q: Point, r: Point) -> bool:
    return (min(p.x, r.x) <= q.x <= max(p.x, r.x) and
            min(p.y, r.y) <= q.y <= max(p.y, r.y))


def segments_intersect(s1: Segment, s2: Segment) -> bool:
    o1 = orientation(s1.p1, s1.p2, s2.p1)
    o2 = orientation(s1.p1, s1.p2, s2.p2)
    o3 = orientation(s2.p1, s2.p2, s1.p1)
    o4 = orientation(s2.p1, s2.p2, s1.p2)

    if o1 != o2 and o3 != o4:
        return True

    if o1 == 0 and on_segment_collinear(s1.p1, s2.p1, s1.p2):
        return True
    if o2 == 0 and on_segment_collinear(s1.p1, s2.p2, s1.p2):
        return True
    if o3 == 0 and on_segment_collinear(s2.p1, s1.p1, s2.p2):
        return True
    if o4 == 0 and on_segment_collinear(s2.p1, s1.p2, s2.p2):
        return True

    return False


# ============================================================================
# Layer 2: High-Level Line Sweep Engine
# ============================================================================

class LineSweepEngine:
    @staticmethod
    def has_any_intersection(segments: List[Segment]) -> bool:
        """Shamos-Hoey O(N log N) algorithm for detecting any intersection."""
        events: List[Tuple[float, int, Segment]] = []
        for s in segments:
            events.append((float(s.p1.x), 0, s))  # START
            events.append((float(s.p2.x), 1, s))  # END

        # Sort events by x; for ties, START before END
        events.sort(key=lambda ev: (ev[0], ev[1]))

        # Sweep-line status: list of active segments kept sorted by eval_y(curr_x)
        status: List[Segment] = []

        for curr_x, ev_type, seg in events:
            if ev_type == 0:  # START
                # Find insertion position in status
                y_val = seg.eval_y(curr_x)
                keys = [s.eval_y(curr_x) for s in status]
                idx = bisect.bisect_left(keys, y_val)
                status.insert(idx, seg)

                # Check intersection with predecessor
                if idx > 0:
                    if segments_intersect(status[idx - 1], seg):
                        return True
                # Check intersection with successor
                if idx + 1 < len(status):
                    if segments_intersect(status[idx + 1], seg):
                        return True
            else:  # END
                # Find and remove segment from status
                if seg in status:
                    idx = status.index(seg)
                    prev_seg = status[idx - 1] if idx > 0 else None
                    next_seg = status[idx + 1] if idx + 1 < len(status) else None

                    if prev_seg and next_seg:
                        if segments_intersect(prev_seg, next_seg):
                            return True
                    status.pop(idx)

        return False

    @staticmethod
    def find_orthogonal_intersections(segments: List[Segment]) -> List[Tuple[int, int]]:
        """Orthogonal plane sweep in O((N + K) log N) for horizontal and vertical segments."""
        events: List[Tuple[int, int, Segment]] = []
        # Types: 0 = H_START, 1 = V_QUERY, 2 = H_END
        for s in segments:
            if s.p1.x == s.p2.x:  # Vertical
                events.append((s.p1.x, 1, s))
            elif s.p1.y == s.p2.y:  # Horizontal
                events.append((s.p1.x, 0, s))
                events.append((s.p2.x, 2, s))

        # Sort: at same x, H_START before V_QUERY before H_END
        events.sort(key=lambda ev: (ev[0], ev[1]))

        # Active horizontal segments: sorted list of (y, id)
        active_y: List[Tuple[int, int]] = []
        results: List[Tuple[int, int]] = []

        for _, ev_type, seg in events:
            if ev_type == 0:  # H_START
                bisect.insort(active_y, (seg.p1.y, seg.id))
            elif ev_type == 2:  # H_END
                target = (seg.p1.y, seg.id)
                idx = bisect.bisect_left(active_y, target)
                if idx < len(active_y) and active_y[idx] == target:
                    active_y.pop(idx)
            else:  # V_QUERY
                y_low = min(seg.p1.y, seg.p2.y)
                y_high = max(seg.p1.y, seg.p2.y)
                left_idx = bisect.bisect_left(active_y, (y_low, -1))
                right_idx = bisect.bisect_right(active_y, (y_high, float('inf')))

                for i in range(left_idx, right_idx):
                    h_id = active_y[i][1]
                    v_id = seg.id
                    results.append((min(h_id, v_id), max(h_id, v_id)))

        results = sorted(list(set(results)))
        return results

    @staticmethod
    def naive_all_intersections(segments: List[Segment]) -> List[Tuple[int, int]]:
        """Naive O(N^2) general intersection oracle."""
        res: List[Tuple[int, int]] = []
        n = len(segments)
        for i in range(n):
            for j in range(i + 1, n):
                if segments_intersect(segments[i], segments[j]):
                    id1, id2 = segments[i].id, segments[j].id
                    res.append((min(id1, id2), max(id1, id2)))
        return sorted(list(set(res)))

    @staticmethod
    def naive_orthogonal_intersections(segments: List[Segment]) -> List[Tuple[int, int]]:
        """Naive O(N^2) orthogonal oracle (only tests horizontal vs vertical)."""
        res: List[Tuple[int, int]] = []
        n = len(segments)
        for i in range(n):
            i_vert = (segments[i].p1.x == segments[i].p2.x)
            for j in range(i + 1, n):
                j_vert = (segments[j].p1.x == segments[j].p2.x)
                if i_vert != j_vert:
                    if segments_intersect(segments[i], segments[j]):
                        id1, id2 = segments[i].id, segments[j].id
                        res.append((min(id1, id2), max(id1, id2)))
        return sorted(list(set(res)))


# ============================================================================
# Unit & Differential Tests
# ============================================================================

class TestLineSweep(unittest.TestCase):
    def test_basic_intersections(self):
        segs = [
            Segment(0, Point(0, 0), Point(4, 4)),
            Segment(1, Point(0, 4), Point(4, 0))
        ]
        self.assertTrue(LineSweepEngine.has_any_intersection(segs))
        self.assertEqual(LineSweepEngine.naive_all_intersections(segs), [(0, 1)])

    def test_parallel_disjoint(self):
        segs = [
            Segment(0, Point(0, 0), Point(4, 0)),
            Segment(1, Point(0, 2), Point(4, 2)),
            Segment(2, Point(0, 4), Point(4, 4))
        ]
        self.assertFalse(LineSweepEngine.has_any_intersection(segs))
        self.assertEqual(LineSweepEngine.naive_all_intersections(segs), [])

    def test_orthogonal_sweep_grid(self):
        segs = [
            Segment(0, Point(1, 0), Point(1, 4)),  # Vertical
            Segment(1, Point(3, 0), Point(3, 4)),  # Vertical
            Segment(2, Point(0, 1), Point(4, 1)),  # Horizontal
            Segment(3, Point(0, 3), Point(4, 3))   # Horizontal
        ]
        self.assertTrue(LineSweepEngine.has_any_intersection(segs))
        ortho_res = LineSweepEngine.find_orthogonal_intersections(segs)
        naive_res = LineSweepEngine.naive_orthogonal_intersections(segs)
        self.assertEqual(len(ortho_res), 4)
        self.assertEqual(ortho_res, naive_res)

    def test_differential_shamos_hoey(self):
        rng = random.Random(1337)
        for _ in range(30):
            n = 15
            segs = []
            for i in range(n):
                p1 = Point(rng.randint(-100, 100), rng.randint(-100, 100))
                p2 = Point(rng.randint(-100, 100), rng.randint(-100, 100))
                if p1.x == p2.x:
                    p2 = Point(p2.x + 1, p2.y)
                segs.append(Segment(i, p1, p2))

            naive = LineSweepEngine.naive_all_intersections(segs)
            shamos = LineSweepEngine.has_any_intersection(segs)
            self.assertEqual(bool(naive), shamos)

    def test_differential_orthogonal_sweep(self):
        rng = random.Random(42)
        for _ in range(40):
            n = 25
            segs = []
            for i in range(n):
                if i % 2 == 0:
                    # Horizontal
                    y = rng.randint(-100, 100)
                    x1 = rng.randint(-100, 100)
                    x2 = rng.randint(-100, 100)
                    if x1 == x2:
                        x2 += 5
                    segs.append(Segment(i, Point(x1, y), Point(x2, y)))
                else:
                    # Vertical
                    x = rng.randint(-100, 100)
                    y1 = rng.randint(-100, 100)
                    y2 = rng.randint(-100, 100)
                    if y1 == y2:
                        y2 += 5
                    segs.append(Segment(i, Point(x, y1), Point(x, y2)))

            sweep_res = LineSweepEngine.find_orthogonal_intersections(segs)
            naive_res = LineSweepEngine.naive_orthogonal_intersections(segs)
            self.assertEqual(sweep_res, naive_res)


if __name__ == "__main__":
    unittest.main()
