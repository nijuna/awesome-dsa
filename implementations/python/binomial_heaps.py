"""
Binomial Heap: Mergeable Priority Queue.

Implements Jean Vuillemin's binomial heap supporting insert, find_min,
extract_min, decrease_key, and O(log n) worst-case heap merging.
"""

import unittest
from typing import Optional


class BinomialNode:
    """Node in a binomial tree."""

    def __init__(self, key: int):
        self.key = key
        self.degree = 0
        self.parent: Optional["BinomialNode"] = None
        self.child: Optional["BinomialNode"] = None
        self.sibling: Optional["BinomialNode"] = None


class BinomialHeap:
    """Binomial Heap supporting O(log n) worst-case merge."""

    def __init__(self):
        self.head: Optional[BinomialNode] = None
        self._count = 0

    def empty(self) -> bool:
        return self.head is None

    def size(self) -> int:
        return self._count

    @staticmethod
    def _link(y: BinomialNode, z: BinomialNode) -> BinomialNode:
        """Links tree y as child of tree z (assumes z.key <= y.key)."""
        assert z.key <= y.key
        y.parent = z
        y.sibling = z.child
        z.child = y
        z.degree += 1
        return z

    @staticmethod
    def _merge_roots(
        h1: Optional[BinomialNode], h2: Optional[BinomialNode]
    ) -> Optional[BinomialNode]:
        """Merges two root lists sorted by degree."""
        if not h1:
            return h2
        if not h2:
            return h1

        dummy = BinomialNode(0)
        tail = dummy
        while h1 and h2:
            if h1.degree <= h2.degree:
                tail.sibling = h1
                h1 = h1.sibling
            else:
                tail.sibling = h2
                h2 = h2.sibling
            tail = tail.sibling
        tail.sibling = h1 if h1 else h2
        return dummy.sibling

    @classmethod
    def _union(
        cls, h1: Optional[BinomialNode], h2: Optional[BinomialNode]
    ) -> Optional[BinomialNode]:
        """Unions two binomial heaps into one valid root list."""
        merged = cls._merge_roots(h1, h2)
        if not merged:
            return None

        prev: Optional[BinomialNode] = None
        curr: Optional[BinomialNode] = merged
        next_node: Optional[BinomialNode] = curr.sibling

        while next_node:
            if (curr.degree != next_node.degree) or (
                next_node.sibling and next_node.sibling.degree == curr.degree
            ):
                prev = curr
                curr = next_node
            else:
                if curr.key <= next_node.key:
                    curr.sibling = next_node.sibling
                    cls._link(next_node, curr)
                else:
                    if not prev:
                        merged = next_node
                    else:
                        prev.sibling = next_node
                    cls._link(curr, next_node)
                    curr = next_node
            next_node = curr.sibling
        return merged

    def insert(self, key: int) -> BinomialNode:
        """Inserts key into heap in O(log n) worst-case (O(1) amortized)."""
        node = BinomialNode(key)
        self.head = self._union(self.head, node)
        self._count += 1
        return node

    def find_min(self) -> int:
        """Returns the minimum key in O(log n)."""
        if self.empty():
            raise IndexError("Heap is empty")
        curr = self.head
        min_val = curr.key
        while curr:
            if curr.key < min_val:
                min_val = curr.key
            curr = curr.sibling
        return min_val

    def extract_min(self) -> int:
        """Removes and returns the minimum key in O(log n)."""
        if self.empty():
            raise IndexError("Heap is empty")

        # 1. Locate min root
        min_node = self.head
        min_prev = None
        curr = self.head
        prev = None
        min_val = self.head.key

        while curr:
            if curr.key < min_val:
                min_val = curr.key
                min_node = curr
                min_prev = prev
            prev = curr
            curr = curr.sibling

        # 2. Detach min root
        if not min_prev:
            self.head = min_node.sibling
        else:
            min_prev.sibling = min_node.sibling

        # 3. Reverse children of min_node
        child = min_node.child
        rev_children: Optional[BinomialNode] = None
        while child:
            next_child = child.sibling
            child.parent = None
            child.sibling = rev_children
            rev_children = child
            child = next_child

        self._count -= 1

        # 4. Merge reversed children into remaining heap
        self.head = self._union(self.head, rev_children)
        return min_val

    def merge(self, other: "BinomialHeap") -> None:
        """Merges other heap into self in O(log n) time."""
        self.head = self._union(self.head, other.head)
        self._count += other._count
        other.head = None
        other._count = 0

    def decrease_key(self, node: BinomialNode, new_key: int) -> None:
        """Decreases the key of a node in O(log n) time."""
        if new_key > node.key:
            raise ValueError("New key is greater than current key")
        node.key = new_key
        curr = node
        p = curr.parent
        while p and curr.key < p.key:
            curr.key, p.key = p.key, curr.key
            curr = p
            p = curr.parent


class TestBinomialHeap(unittest.TestCase):
    def test_basic_operations(self):
        bh = BinomialHeap()
        self.assertTrue(bh.empty())
        self.assertEqual(bh.size(), 0)

        for x in [10, 20, 5, 15, 30]:
            bh.insert(x)

        self.assertEqual(bh.size(), 5)
        self.assertEqual(bh.find_min(), 5)

        self.assertEqual(bh.extract_min(), 5)
        self.assertEqual(bh.find_min(), 10)
        self.assertEqual(bh.extract_min(), 10)
        self.assertEqual(bh.extract_min(), 15)
        self.assertEqual(bh.extract_min(), 20)
        self.assertEqual(bh.extract_min(), 30)
        self.assertTrue(bh.empty())

    def test_merge_heaps(self):
        h1 = BinomialHeap()
        for x in [8, 3, 12]:
            h1.insert(x)

        h2 = BinomialHeap()
        for x in [4, 17, 1]:
            h2.insert(x)

        h1.merge(h2)
        self.assertEqual(h1.size(), 6)
        self.assertEqual(h2.size(), 0)
        self.assertEqual(h1.find_min(), 1)

        res = []
        while not h1.empty():
            res.append(h1.extract_min())
        self.assertEqual(res, [1, 3, 4, 8, 12, 17])

    def test_decrease_key(self):
        bh = BinomialHeap()
        n1 = bh.insert(50)
        bh.insert(20)
        bh.insert(40)
        self.assertEqual(bh.find_min(), 20)

        bh.decrease_key(n1, 5)
        self.assertEqual(bh.find_min(), 5)


if __name__ == "__main__":
    unittest.main()
