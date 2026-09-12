"""2D / k-Dimensional Tree (KD-Tree) for Range Search and K-Nearest Neighbor Queries.

Implements Arthur's Two-Layer API:
  - Layer 1: Median-splitting spatial partitioning, axis-alternating KDNode,
             and hyperplane pruning logic.
  - Layer 2: Safe KDTreeEngine with balanced construction O(N log N),
             orthogonal range search, and k-NN search with differential testing.
"""

from __future__ import annotations
import heapq
import random
import unittest
from typing import NamedTuple, List, Optional, Tuple


class Point2D(NamedTuple):
    id: int
    x: int
    y: int

    def get(self, axis: int) -> int:
        return self.x if axis == 0 else self.y


def dist_sq_exact(a: Point2D, b: Point2D) -> int:
    return (a.x - b.x) ** 2 + (a.y - b.y) ** 2


class BoundingBox2D(NamedTuple):
    min_x: int
    max_x: int
    min_y: int
    max_y: int

    def contains(self, p: Point2D) -> bool:
        return self.min_x <= p.x <= self.max_x and self.min_y <= p.y <= self.max_y

    def intersects(self, o: BoundingBox2D) -> bool:
        return not (self.max_x < o.min_x or self.min_x > o.max_x or
                    self.max_y < o.min_y or self.min_y > o.max_y)


class KDNode:
    def __init__(self, point: Point2D, axis: int):
        self.point = point
        self.axis = axis
        self.left: Optional[KDNode] = None
        self.right: Optional[KDNode] = None


class KDTree:
    def __init__(self, points: Optional[List[Point2D]] = None):
        self.root: Optional[KDNode] = None
        self._size = 0
        if points:
            self.build(points)

    def build(self, points: List[Point2D]) -> None:
        pts = list(points)
        self._size = len(pts)

        def _build_rec(sub_pts: List[Point2D], depth: int) -> Optional[KDNode]:
            if not sub_pts:
                return None
            axis = depth % 2
            sub_pts.sort(key=lambda p: (p.get(axis), p.id))
            mid = len(sub_pts) // 2

            node = KDNode(sub_pts[mid], axis)
            node.left = _build_rec(sub_pts[:mid], depth + 1)
            node.right = _build_rec(sub_pts[mid + 1:], depth + 1)
            return node

        self.root = _build_rec(pts, 0)

    def __len__(self) -> int:
        return self._size

    def range_search(self, query_box: BoundingBox2D) -> List[Point2D]:
        """Orthogonal Range Search: returns all points in query_box."""
        results: List[Point2D] = []

        def _search_rec(node: Optional[KDNode], curr_box: BoundingBox2D) -> None:
            if not node:
                return
            if not curr_box.intersects(query_box):
                return
            if query_box.contains(node.point):
                results.append(node.point)

            if node.axis == 0:
                left_box = BoundingBox2D(curr_box.min_x, node.point.x, curr_box.min_y, curr_box.max_y)
                right_box = BoundingBox2D(node.point.x, curr_box.max_x, curr_box.min_y, curr_box.max_y)
            else:
                left_box = BoundingBox2D(curr_box.min_x, curr_box.max_x, curr_box.min_y, node.point.y)
                right_box = BoundingBox2D(curr_box.min_x, curr_box.max_x, node.point.y, curr_box.max_y)

            _search_rec(node.left, left_box)
            _search_rec(node.right, right_box)

        inf = float('inf')
        universe = BoundingBox2D(int(-1e18), int(1e18), int(-1e18), int(1e18))
        _search_rec(self.root, universe)
        return sorted(results, key=lambda p: p.id)

    def k_nearest_neighbors(self, target: Point2D, k: int) -> List[Point2D]:
        """K-Nearest Neighbors (k-NN) search using max-heap branch and bound."""
        if not self.root or k <= 0:
            return []
        k = min(k, self._size)

        # Max-heap storing (-dist_sq, point.id, point)
        heap: List[Tuple[int, int, Point2D]] = []

        def _knn_rec(node: Optional[KDNode]) -> None:
            if not node:
                return

            d_sq = dist_sq_exact(node.point, target)
            entry = (-d_sq, node.point.id, node.point)

            if len(heap) < k:
                heapq.heappush(heap, entry)
            elif d_sq < -heap[0][0]:
                heapq.heappop(heap)
                heapq.heappush(heap, entry)

            axis = node.axis
            diff = target.get(axis) - node.point.get(axis)
            axis_dist_sq = diff * diff

            first = node.left if diff <= 0 else node.right
            second = node.right if diff <= 0 else node.left

            # Search closer subtree first
            _knn_rec(first)

            # Prune farther subtree if distance to hyperplane exceeds worst candidate
            if len(heap) < k or axis_dist_sq < -heap[0][0]:
                _knn_rec(second)

        _knn_rec(self.root)

        # Extract sorted from nearest to farthest
        sorted_entries = sorted(heap, key=lambda item: (-item[0], item[1]))
        return [item[2] for item in sorted_entries]

    def nearest_neighbor(self, target: Point2D) -> Point2D:
        """1-Nearest Neighbor search."""
        res = self.k_nearest_neighbors(target, 1)
        if not res:
            raise ValueError("KDTree is empty")
        return res[0]


# ============================================================================
# Differential Oracles
# ============================================================================

def naive_range_search(points: List[Point2D], box: BoundingBox2D) -> List[Point2D]:
    res = [p for p in points if box.contains(p)]
    return sorted(res, key=lambda p: p.id)


def naive_knn(points: List[Point2D], target: Point2D, k: int) -> List[Point2D]:
    ranked = sorted(points, key=lambda p: (dist_sq_exact(p, target), p.id))
    return ranked[:k]


# ============================================================================
# Unit & Differential Tests
# ============================================================================

class TestKDTree(unittest.TestCase):
    def test_basic_queries(self):
        pts = [
            Point2D(0, 2, 3),
            Point2D(1, 5, 4),
            Point2D(2, 9, 6),
            Point2D(3, 4, 7),
            Point2D(4, 8, 1),
            Point2D(5, 7, 2)
        ]
        tree = KDTree(pts)
        self.assertEqual(len(tree), 6)

        # Range search
        box = BoundingBox2D(2, 6, 2, 5)
        range_res = tree.range_search(box)
        naive_range = naive_range_search(pts, box)
        self.assertEqual(range_res, naive_range)

        # 1-NN query
        query = Point2D(99, 9, 2)
        nn = tree.nearest_neighbor(query)
        self.assertIn(nn.id, [4, 5])

        # 3-NN query
        knn = tree.k_nearest_neighbors(query, 3)
        naive_k = naive_knn(pts, query, 3)
        self.assertEqual([dist_sq_exact(p, query) for p in knn],
                         [dist_sq_exact(p, query) for p in naive_k])

    def test_differential_random(self):
        rng = random.Random(42)
        for _ in range(50):
            n = 50
            pts = [Point2D(i, rng.randint(-300, 300), rng.randint(-300, 300)) for i in range(n)]
            tree = KDTree(pts)

            # Range search test
            x1, x2 = rng.randint(-300, 300), rng.randint(-300, 300)
            y1, y2 = rng.randint(-300, 300), rng.randint(-300, 300)
            box = BoundingBox2D(min(x1, x2), max(x1, x2), min(y1, y2), max(y1, y2))
            self.assertEqual(tree.range_search(box), naive_range_search(pts, box))

            # K-NN test
            query = Point2D(999, rng.randint(-300, 300), rng.randint(-300, 300))
            k = 5
            tree_knn = tree.k_nearest_neighbors(query, k)
            naive_k = naive_knn(pts, query, k)
            self.assertEqual([dist_sq_exact(p, query) for p in tree_knn],
                             [dist_sq_exact(p, query) for p in naive_k])


if __name__ == "__main__":
    unittest.main()
