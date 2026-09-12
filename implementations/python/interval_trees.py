"""
Reference implementation of Augmented Interval Trees with Subtree Max Augmentation.

Implements balanced Interval Tree (augmented Treap) for dynamic interval storage,
point stabbing queries, single-overlap search pruning, and all-overlaps reporting.
Verified against a linear naive oracle.
"""

import random
import unittest
from typing import Optional, List, Tuple


class Interval:
    __slots__ = ('low', 'high')

    def __init__(self, low: int, high: int):
        assert low <= high, f"Invalid interval: low ({low}) must be <= high ({high})"
        self.low = low
        self.high = high

    def overlaps(self, other: 'Interval') -> bool:
        return self.low <= other.high and other.low <= self.high

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, Interval):
            return False
        return self.low == other.low and self.high == other.high

    def __lt__(self, other: 'Interval') -> bool:
        if self.low != other.low:
            return self.low < other.low
        return self.high < other.high

    def __repr__(self) -> str:
        return f"[{self.low}, {self.high}]"


class IntervalNode:
    __slots__ = ('intv', 'max_high', 'priority', 'left', 'right')

    def __init__(self, intv: Interval, priority: int):
        self.intv = intv
        self.max_high = intv.high
        self.priority = priority
        self.left: Optional['IntervalNode'] = None
        self.right: Optional['IntervalNode'] = None


class IntervalTree:
    """Augmented Balanced Interval Tree (Treap-based) with O(log N) operations."""

    def __init__(self):
        self.root: Optional[IntervalNode] = None
        self._size = 0
        self._rng = random.Random(42)

    def __len__(self) -> int:
        return self._size

    def is_empty(self) -> bool:
        return self.root is None

    @staticmethod
    def _update_node(n: Optional[IntervalNode]) -> None:
        if not n:
            return
        m = n.intv.high
        if n.left:
            m = max(m, n.left.max_high)
        if n.right:
            m = max(m, n.right.max_high)
        n.max_high = m

    def _rotate_right(self, y: IntervalNode) -> IntervalNode:
        x = y.left
        assert x is not None
        y.left = x.right
        x.right = y
        self._update_node(y)
        self._update_node(x)
        return x

    def _rotate_left(self, x: IntervalNode) -> IntervalNode:
        y = x.right
        assert y is not None
        x.right = y.left
        y.left = x
        self._update_node(x)
        self._update_node(y)
        return y

    def insert(self, low: int, high: int) -> None:
        intv = Interval(low, high)
        prio = self._rng.randint(0, 1 << 30)
        inserted = False

        def _insert(t: Optional[IntervalNode]) -> IntervalNode:
            nonlocal inserted
            if not t:
                inserted = True
                return IntervalNode(intv, prio)
            if intv == t.intv:
                return t
            if intv < t.intv:
                t.left = _insert(t.left)
                if t.left.priority > t.priority:
                    t = self._rotate_right(t)
            else:
                t.right = _insert(t.right)
                if t.right.priority > t.priority:
                    t = self._rotate_left(t)
            self._update_node(t)
            return t

        self.root = _insert(self.root)
        if inserted:
            self._size += 1

    def erase(self, low: int, high: int) -> bool:
        intv = Interval(low, high)
        erased = False

        def _erase(t: Optional[IntervalNode]) -> Optional[IntervalNode]:
            nonlocal erased
            if not t:
                return None
            if intv < t.intv:
                t.left = _erase(t.left)
            elif t.intv < intv:
                t.right = _erase(t.right)
            else:
                erased = True
                if not t.left and not t.right:
                    return None
                elif not t.left:
                    return t.right
                elif not t.right:
                    return t.left
                else:
                    if t.left.priority > t.right.priority:
                        t = self._rotate_right(t)
                        t.right = _erase(t.right)
                    else:
                        t = self._rotate_left(t)
                        t.left = _erase(t.left)
            self._update_node(t)
            return t

        self.root = _erase(self.root)
        if erased:
            self._size -= 1
        return erased

    def find_overlap(self, query: Interval) -> Optional[Interval]:
        """Find any interval overlapping with query using O(log N) pruning."""
        curr = self.root
        while curr:
            if curr.intv.overlaps(query):
                return curr.intv
            if curr.left and curr.left.max_high >= query.low:
                curr = curr.left
            else:
                curr = curr.right
        return None

    def find_all_overlaps(self, query: Interval) -> List[Interval]:
        """Find all intervals overlapping with query in O(k + log N) time."""
        out: List[Interval] = []

        def _search(t: Optional[IntervalNode]):
            if not t:
                return
            if t.left and t.left.max_high >= query.low:
                _search(t.left)
            if t.intv.overlaps(query):
                out.append(t.intv)
            if t.right and t.intv.low <= query.high:
                _search(t.right)

        _search(self.root)
        return out

    def stabbing_query(self, point: int) -> List[Interval]:
        """Find all intervals containing point."""
        return self.find_all_overlaps(Interval(point, point))

    def verify_invariants(self) -> bool:
        if not self.root:
            return self._size == 0

        def check(n: Optional[IntervalNode], min_i: Optional[Interval], max_i: Optional[Interval]) -> bool:
            if not n:
                return True
            if min_i and not (min_i < n.intv):
                return False
            if max_i and not (n.intv < max_i):
                return False

            expected_max = n.intv.high
            if n.left:
                expected_max = max(expected_max, n.left.max_high)
            if n.right:
                expected_max = max(expected_max, n.right.max_high)
            if n.max_high != expected_max:
                return False

            return check(n.left, min_i, n.intv) and check(n.right, n.intv, max_i)

        return check(self.root, None, None)


class NaiveIntervalOracle:
    def __init__(self):
        self.intervals: List[Interval] = []

    def insert(self, low: int, high: int) -> None:
        intv = Interval(low, high)
        if intv not in self.intervals:
            self.intervals.append(intv)

    def erase(self, low: int, high: int) -> bool:
        intv = Interval(low, high)
        if intv in self.intervals:
            self.intervals.remove(intv)
            return True
        return False

    def has_overlap(self, q: Interval) -> bool:
        return any(i.overlaps(q) for i in self.intervals)

    def find_all_overlaps(self, q: Interval) -> List[Interval]:
        out = [i for i in self.intervals if i.overlaps(q)]
        out.sort()
        return out

    def __len__(self) -> int:
        return len(self.intervals)


class TestIntervalTree(unittest.TestCase):
    def test_textbook_network(self):
        itree = IntervalTree()
        oracle = NaiveIntervalOracle()

        textbook = [
            (16, 21), (8, 9), (25, 30), (5, 8), (15, 23),
            (17, 19), (26, 26), (0, 3), (6, 10), (19, 20)
        ]
        for l, h in textbook:
            itree.insert(l, h)
            oracle.insert(l, h)

        self.assertEqual(len(itree), 10)
        self.assertTrue(itree.verify_invariants())

        # Overlap with [22, 25] -> should find [15, 23]
        q1 = Interval(22, 25)
        found = itree.find_overlap(q1)
        self.assertIsNotNone(found)
        self.assertTrue(found.overlaps(q1))

        # Stabbing at 7 -> should hit [5, 8], [6, 10]
        stab = itree.stabbing_query(7)
        self.assertEqual(len(stab), 2)

    def test_differential_fuzzing(self):
        itree = IntervalTree()
        oracle = NaiveIntervalOracle()

        rng = random.Random(1337)
        for _ in range(1000):
            op = rng.randint(0, 3)
            l = rng.randint(0, 100)
            h = l + rng.randint(0, 30)

            if op == 0:
                itree.insert(l, h)
                oracle.insert(l, h)
            elif op == 1:
                r1 = itree.erase(l, h)
                r2 = oracle.erase(l, h)
                self.assertEqual(r1, r2)
            elif op == 2:
                q = Interval(l, h)
                res = itree.find_overlap(q)
                has = oracle.has_overlap(q)
                if has:
                    self.assertIsNotNone(res)
                    self.assertTrue(res.overlaps(q))
                else:
                    self.assertIsNone(res)
            else:
                q = Interval(l, h)
                res_all = itree.find_all_overlaps(q)
                res_oracle = oracle.find_all_overlaps(q)
                res_all.sort()
                self.assertEqual(res_all, res_oracle)

            self.assertEqual(len(itree), len(oracle))
            self.assertTrue(itree.verify_invariants())


if __name__ == '__main__':
    unittest.main()
