"""R-Tree Spatial Index: Minimum Bounding Rectangles (MBR), Guttman's Quadratic Split,
and Spatial Overlap Queries.

Implements Arthur's Two-Layer API:
  - Layer 1: Rect MBR algebra (area, enclosing, intersection, expansion penalties)
             and Guttman's Quadratic Split algorithm.
  - Layer 2: Safe RTreeEngine supporting dynamic spatial insertions, overlap queries,
             and differential testing against an O(N) naive oracle.
"""

from __future__ import annotations
import random
import unittest
from typing import NamedTuple, List, Optional, Tuple


class Rect(NamedTuple):
    min_x: int
    max_x: int
    min_y: int
    max_y: int

    @classmethod
    def from_coords(cls, x1: int, x2: int, y1: int, y2: int) -> Rect:
        return cls(min(x1, x2), max(x1, x2), min(y1, y2), max(y1, y2))

    def area(self) -> int:
        return (self.max_x - self.min_x) * (self.max_y - self.min_y)

    def intersects(self, o: Rect) -> bool:
        return not (self.max_x < o.min_x or self.min_x > o.max_x or
                    self.max_y < o.min_y or self.min_y > o.max_y)

    def contains(self, o: Rect) -> bool:
        return (self.min_x <= o.min_x and self.max_x >= o.max_x and
                self.min_y <= o.min_y and self.max_y >= o.max_y)

    def enclosing(self, o: Rect) -> Rect:
        return Rect(min(self.min_x, o.min_x), max(self.max_x, o.max_x),
                    min(self.min_y, o.min_y), max(self.max_y, o.max_y))


class Entry:
    def __init__(self, box: Rect, item_id: int = -1, child: Optional[Node] = None):
        self.box = box
        self.id = item_id
        self.child = child

    def __repr__(self) -> str:
        return f"Entry(box={self.box}, id={self.id}, is_leaf={self.child is None})"


MAX_ENTRIES = 4
MIN_ENTRIES = 2


class Node:
    def __init__(self, is_leaf: bool):
        self.is_leaf = is_leaf
        self.entries: List[Entry] = []

    def compute_mbr(self) -> Rect:
        assert self.entries, "Cannot compute MBR of empty node"
        mbr = self.entries[0].box
        for e in self.entries[1:]:
            mbr = mbr.enclosing(e.box)
        return mbr


class RTree:
    def __init__(self):
        self.root = Node(is_leaf=True)
        self._size = 0

    def __len__(self) -> int:
        return self._size

    @staticmethod
    def _quadratic_split(all_entries: List[Entry], is_leaf: bool) -> Tuple[Node, Node]:
        n = len(all_entries)
        assert n > MAX_ENTRIES

        # Step 1: PickSeeds (find pair maximizing wasted area)
        seed1, seed2 = 0, 1
        max_waste = -1

        for i in range(n):
            for j in range(i + 1, n):
                enc = all_entries[i].box.enclosing(all_entries[j].box)
                waste = enc.area() - all_entries[i].box.area() - all_entries[j].box.area()
                if waste > max_waste:
                    max_waste = waste
                    seed1, seed2 = i, j

        node1 = Node(is_leaf)
        node2 = Node(is_leaf)

        mbr1 = all_entries[seed1].box
        mbr2 = all_entries[seed2].box

        node1.entries.append(all_entries[seed1])
        node2.entries.append(all_entries[seed2])

        assigned = [False] * n
        assigned[seed1] = True
        assigned[seed2] = True
        remaining = n - 2

        # Step 2: DistributeRemaining
        while remaining > 0:
            if len(node1.entries) + remaining == MIN_ENTRIES:
                for i in range(n):
                    if not assigned[i]:
                        mbr1 = mbr1.enclosing(all_entries[i].box)
                        node1.entries.append(all_entries[i])
                        assigned[i] = True
                break

            if len(node2.entries) + remaining == MIN_ENTRIES:
                for i in range(n):
                    if not assigned[i]:
                        mbr2 = mbr2.enclosing(all_entries[i].box)
                        node2.entries.append(all_entries[i])
                        assigned[i] = True
                break

            # Pick entry with maximum difference in area expansion
            best_entry = -1
            max_diff = -1
            best_d1 = best_d2 = 0

            for i in range(n):
                if assigned[i]:
                    continue
                d1 = mbr1.enclosing(all_entries[i].box).area() - mbr1.area()
                d2 = mbr2.enclosing(all_entries[i].box).area() - mbr2.area()
                diff = abs(d1 - d2)

                if diff > max_diff:
                    max_diff = diff
                    best_entry = i
                    best_d1 = d1
                    best_d2 = d2

            # Assign best_entry to group requiring smaller enlargement
            if best_d1 < best_d2:
                mbr1 = mbr1.enclosing(all_entries[best_entry].box)
                node1.entries.append(all_entries[best_entry])
            elif best_d2 < best_d1:
                mbr2 = mbr2.enclosing(all_entries[best_entry].box)
                node2.entries.append(all_entries[best_entry])
            else:
                # Tie: assign to group with smaller area
                if mbr1.area() <= mbr2.area():
                    mbr1 = mbr1.enclosing(all_entries[best_entry].box)
                    node1.entries.append(all_entries[best_entry])
                else:
                    mbr2 = mbr2.enclosing(all_entries[best_entry].box)
                    node2.entries.append(all_entries[best_entry])

            assigned[best_entry] = True
            remaining -= 1

        return node1, node2

    def _insert_rec(self, node: Node, new_entry: Entry) -> Optional[Node]:
        if node.is_leaf:
            node.entries.append(new_entry)
            if len(node.entries) > MAX_ENTRIES:
                n1, n2 = self._quadratic_split(node.entries, is_leaf=True)
                node.is_leaf = n1.is_leaf
                node.entries = n1.entries
                return n2
            return None

        # ChooseLeaf: pick child whose MBR requires minimum area enlargement
        best_idx = 0
        min_enlargement = float('inf')
        min_area = float('inf')

        for i, entry in enumerate(node.entries):
            enc = entry.box.enclosing(new_entry.box)
            enlargement = enc.area() - entry.box.area()
            current_area = entry.box.area()

            if (enlargement < min_enlargement or
                    (enlargement == min_enlargement and current_area < min_area)):
                min_enlargement = enlargement
                min_area = current_area
                best_idx = i

        split_child = self._insert_rec(node.entries[best_idx].child, new_entry)
        node.entries[best_idx].box = node.entries[best_idx].child.compute_mbr()

        if split_child:
            sibling_entry = Entry(split_child.compute_mbr(), -1, split_child)
            node.entries.append(sibling_entry)

            if len(node.entries) > MAX_ENTRIES:
                n1, n2 = self._quadratic_split(node.entries, is_leaf=False)
                node.is_leaf = n1.is_leaf
                node.entries = n1.entries
                return n2

        return None

    def insert(self, item_id: int, box: Rect) -> None:
        """Inserts an item with bounding rectangle into R-Tree."""
        new_entry = Entry(box, item_id)
        split_root = self._insert_rec(self.root, new_entry)

        if split_root:
            new_root = Node(is_leaf=False)
            new_root.entries.append(Entry(self.root.compute_mbr(), -1, self.root))
            new_root.entries.append(Entry(split_root.compute_mbr(), -1, split_root))
            self.root = new_root

        self._size += 1

    def search(self, query_box: Rect) -> List[int]:
        """Spatial overlap search: returns list of item IDs intersecting query_box."""
        results: List[int] = []

        def _search_rec(node: Optional[Node]) -> None:
            if not node:
                return
            for entry in node.entries:
                if entry.box.intersects(query_box):
                    if node.is_leaf:
                        results.append(entry.id)
                    else:
                        _search_rec(entry.child)

        _search_rec(self.root)
        return sorted(results)


# ============================================================================
# Differential Oracle
# ============================================================================

def naive_search(items: List[Tuple[int, Rect]], query_box: Rect) -> List[int]:
    return sorted([item_id for item_id, box in items if box.intersects(query_box)])


# ============================================================================
# Unit & Differential Tests
# ============================================================================

class TestRTree(unittest.TestCase):
    def test_basic_tree(self):
        tree = RTree()
        self.assertEqual(len(tree), 0)

        tree.insert(1, Rect.from_coords(0, 2, 0, 2))
        tree.insert(2, Rect.from_coords(5, 7, 5, 7))
        tree.insert(3, Rect.from_coords(1, 3, 1, 3))
        tree.insert(4, Rect.from_coords(10, 12, 10, 12))
        tree.insert(5, Rect.from_coords(6, 8, 6, 8))
        tree.insert(6, Rect.from_coords(0, 1, 0, 1))

        self.assertEqual(len(tree), 6)

        # Overlapping query
        q1 = tree.search(Rect.from_coords(0, 2, 0, 2))
        self.assertEqual(q1, [1, 3, 6])

        # Overlapping query 2
        q2 = tree.search(Rect.from_coords(5, 8, 5, 8))
        self.assertEqual(q2, [2, 5])

        # Disjoint query
        q3 = tree.search(Rect.from_coords(20, 25, 20, 25))
        self.assertEqual(q3, [])

    def test_differential_random(self):
        rng = random.Random(42)
        for _ in range(30):
            tree = RTree()
            items: List[Tuple[int, Rect]] = []
            n = 50

            for i in range(n):
                x = rng.randint(-150, 150)
                y = rng.randint(-150, 150)
                w = rng.randint(1, 20)
                h = rng.randint(1, 20)
                box = Rect.from_coords(x, x + w, y, y + h)
                items.append((i, box))
                tree.insert(i, box)

            for _ in range(10):
                qx = rng.randint(-150, 150)
                qy = rng.randint(-150, 150)
                qw = rng.randint(5, 40)
                qh = rng.randint(5, 40)
                q_box = Rect.from_coords(qx, qx + qw, qy, qy + qh)

                tree_res = tree.search(q_box)
                naive_res = naive_search(items, q_box)
                self.assertEqual(tree_res, naive_res)


if __name__ == "__main__":
    unittest.main()
