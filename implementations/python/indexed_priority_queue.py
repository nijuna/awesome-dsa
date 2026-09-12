"""
Reference Implementation: Indexed Priority Queue (Indexed Min-Heap)
Demonstrates:
1. Parallel indexing array model (pq, qp, keys)
2. O(1) membership check and key retrieval
3. O(log n) insert, pop-min, decrease-key, increase-key, change-key, and erase
4. Self-verifying heap and inverse-map invariants
5. Dijkstra's Single-Source Shortest Path using Indexed PQ
6. Prim's Minimum Spanning Tree using Indexed PQ

Language: Python 3
"""

from typing import List, Tuple, Any, Optional
import unittest


class IndexedMinPQ:
    """
    Indexed Min-Priority Queue for integer keys in range [0, max_n - 1].
    Uses 1-based heap positions for arithmetic convenience:
      parent(k) = k // 2, left_child(k) = 2 * k, right_child(k) = 2 * k + 1.
    """

    def __init__(self, max_n: int) -> None:
        if max_n < 0:
            raise ValueError("max_n must be non-negative")
        self.max_n = max_n
        self.n = 0
        # 1-based heap: position -> item index
        self.pq: List[int] = [-1] * (max_n + 1)
        # item index -> heap position (-1 if absent)
        self.qp: List[int] = [-1] * max_n
        # item index -> current priority key
        self.keys: List[Optional[Any]] = [None] * max_n

    def _validate_index(self, i: int) -> None:
        if i < 0 or i >= self.max_n:
            raise IndexError(f"Index {i} out of bounds for capacity {self.max_n}")

    def contains(self, i: int) -> bool:
        self._validate_index(i)
        return self.qp[i] != -1

    def empty(self) -> bool:
        return self.n == 0

    def size(self) -> int:
        return self.n

    def key_of(self, i: int) -> Any:
        self._validate_index(i)
        if not self.contains(i):
            raise KeyError(f"Index {i} is not in priority queue")
        return self.keys[i]

    def _greater_pos(self, a: int, b: int) -> bool:
        # returns True if key at heap position a > key at heap position b
        return self.keys[self.pq[a]] > self.keys[self.pq[b]]

    def _exch(self, a: int, b: int) -> None:
        self.pq[a], self.pq[b] = self.pq[b], self.pq[a]
        self.qp[self.pq[a]] = a
        self.qp[self.pq[b]] = b

    def _swim(self, k: int) -> None:
        while k > 1 and self._greater_pos(k // 2, k):
            self._exch(k, k // 2)
            k //= 2

    def _sink(self, k: int) -> None:
        while 2 * k <= self.n:
            j = 2 * k
            if j < self.n and self._greater_pos(j, j + 1):
                j += 1
            if not self._greater_pos(k, j):
                break
            self._exch(k, j)
            k = j

    def insert(self, i: int, key: Any) -> None:
        self._validate_index(i)
        if self.contains(i):
            raise ValueError(f"Index {i} already present in priority queue")

        self.n += 1
        self.qp[i] = self.n
        self.pq[self.n] = i
        self.keys[i] = key
        self._swim(self.n)

    def min_index(self) -> int:
        if self.n == 0:
            raise IndexError("Priority queue underflow")
        return self.pq[1]

    def min_key(self) -> Any:
        if self.n == 0:
            raise IndexError("Priority queue underflow")
        return self.keys[self.pq[1]]

    def pop_min_index(self) -> int:
        if self.n == 0:
            raise IndexError("Priority queue underflow")

        min_item = self.pq[1]
        self._exch(1, self.n)
        self.n -= 1
        self._sink(1)

        self.qp[min_item] = -1
        self.keys[min_item] = None
        self.pq[self.n + 1] = -1
        return min_item

    def decrease_key(self, i: int, key: Any) -> None:
        self._validate_index(i)
        if not self.contains(i):
            raise ValueError(f"Index {i} not present in priority queue")
        if not (key < self.keys[i]):
            raise ValueError(f"New key {key} is not strictly smaller than current key {self.keys[i]}")

        self.keys[i] = key
        self._swim(self.qp[i])

    def increase_key(self, i: int, key: Any) -> None:
        self._validate_index(i)
        if not self.contains(i):
            raise ValueError(f"Index {i} not present in priority queue")
        if not (self.keys[i] < key):
            raise ValueError(f"New key {key} is not strictly larger than current key {self.keys[i]}")

        self.keys[i] = key
        self._sink(self.qp[i])

    def change_key(self, i: int, key: Any) -> None:
        self._validate_index(i)
        if not self.contains(i):
            raise ValueError(f"Index {i} not present in priority queue")

        old_key = self.keys[i]
        self.keys[i] = key

        if key < old_key:
            self._swim(self.qp[i])
        elif old_key < key:
            self._sink(self.qp[i])

    def erase(self, i: int) -> None:
        self._validate_index(i)
        if not self.contains(i):
            raise ValueError(f"Index {i} not present in priority queue")

        pos = self.qp[i]
        self._exch(pos, self.n)
        self.n -= 1

        if pos <= self.n:
            self._swim(pos)
            self._sink(pos)

        self.qp[i] = -1
        self.keys[i] = None
        self.pq[self.n + 1] = -1

    def check_invariants(self) -> bool:
        """Verifies internal 1-to-1 mapping and heap ordering invariants."""
        # 1. Check inverse map on active positions
        for pos in range(1, self.n + 1):
            item = self.pq[pos]
            if item < 0 or item >= self.max_n:
                return False
            if self.qp[item] != pos:
                return False

        active = 0
        for i in range(self.max_n):
            if self.qp[i] != -1:
                active += 1
                if self.qp[i] < 1 or self.qp[i] > self.n:
                    return False
                if self.pq[self.qp[i]] != i:
                    return False
        if active != self.n:
            return False

        # 2. Check min-heap property
        for pos in range(1, self.n // 2 + 1):
            left = 2 * pos
            right = 2 * pos + 1
            if left <= self.n and self.keys[self.pq[pos]] > self.keys[self.pq[left]]:
                return False
            if right <= self.n and self.keys[self.pq[pos]] > self.keys[self.pq[right]]:
                return False

        return True


# ============================================================================
# Application 1: Dijkstra's Algorithm
# ============================================================================

def dijkstra_indexed_pq(graph: List[List[Tuple[int, int]]], source: int) -> List[int]:
    """
    Computes shortest paths from source to all vertices using an IndexedMinPQ.
    graph[u] = [(v, weight), ...]
    Time Complexity: O((V + E) log V). Space: O(V).
    """
    n = len(graph)
    INF = 10**18
    dist = [INF] * n
    dist[source] = 0

    pq = IndexedMinPQ(n)
    pq.insert(source, 0)

    while not pq.empty():
        u = pq.pop_min_index()
        du = dist[u]

        for v, w in graph[u]:
            nd = du + w
            if nd < dist[v]:
                dist[v] = nd
                if pq.contains(v):
                    pq.decrease_key(v, nd)
                else:
                    pq.insert(v, nd)

    return dist


# ============================================================================
# Application 2: Prim's Minimum Spanning Tree
# ============================================================================

def prim_mst_indexed_pq(
    graph: List[List[Tuple[int, int]]], source: int = 0
) -> Tuple[int, List[Tuple[int, int]]]:
    """
    Computes MST total weight and edge list using Prim's algorithm with IndexedMinPQ.
    Time Complexity: O(E log V). Space: O(V).
    """
    n = len(graph)
    if n == 0:
        return 0, []

    INF = 10**18
    min_edge = [INF] * n
    parent = [-1] * n
    in_mst = [False] * n

    pq = IndexedMinPQ(n)
    min_edge[source] = 0
    pq.insert(source, 0)

    total_weight = 0
    mst_edges: List[Tuple[int, int]] = []

    while not pq.empty():
        u = pq.pop_min_index()
        in_mst[u] = True
        total_weight += min_edge[u]

        if parent[u] != -1:
            mst_edges.append((parent[u], u))

        for v, w in graph[u]:
            if not in_mst[v] and w < min_edge[v]:
                min_edge[v] = w
                parent[v] = u
                if pq.contains(v):
                    pq.decrease_key(v, w)
                else:
                    pq.insert(v, w)

    return total_weight, mst_edges


# ============================================================================
# Unit Tests
# ============================================================================

class TestIndexedPriorityQueue(unittest.TestCase):

    def test_arthur_mini_example(self):
        # 0 -> 8, 1 -> 3, 2 -> 5
        pq = IndexedMinPQ(5)
        self.assertTrue(pq.empty())
        pq.insert(0, 8)
        pq.insert(1, 3)
        pq.insert(2, 5)
        self.assertTrue(pq.check_invariants())

        self.assertEqual(pq.size(), 3)
        self.assertEqual(pq.min_index(), 1)
        self.assertEqual(pq.min_key(), 3)

        # Decrease key of 0 to 2
        pq.decrease_key(0, 2)
        self.assertTrue(pq.check_invariants())
        self.assertEqual(pq.min_index(), 0)
        self.assertEqual(pq.min_key(), 2)

        # Pop root
        self.assertEqual(pq.pop_min_index(), 0)
        self.assertTrue(pq.check_invariants())
        self.assertFalse(pq.contains(0))
        self.assertEqual(pq.min_index(), 1)
        self.assertEqual(pq.min_key(), 3)

        # Increase key of 1 to 10
        pq.increase_key(1, 10)
        self.assertTrue(pq.check_invariants())
        self.assertEqual(pq.min_index(), 2)
        self.assertEqual(pq.min_key(), 5)

        # Change key of 1 back to 1
        pq.change_key(1, 1)
        self.assertTrue(pq.check_invariants())
        self.assertEqual(pq.min_index(), 1)
        self.assertEqual(pq.min_key(), 1)

        # Erase item 2
        pq.erase(2)
        self.assertTrue(pq.check_invariants())
        self.assertFalse(pq.contains(2))
        self.assertEqual(pq.size(), 1)

        self.assertEqual(pq.pop_min_index(), 1)
        self.assertTrue(pq.check_invariants())
        self.assertTrue(pq.empty())

    def test_exceptions_and_boundaries(self):
        pq = IndexedMinPQ(3)
        with self.assertRaises(IndexError):
            pq.insert(3, 10)
        with self.assertRaises(IndexError):
            pq.insert(-1, 10)
        with self.assertRaises(IndexError):
            pq.pop_min_index()

        pq.insert(0, 100)
        with self.assertRaises(ValueError):
            pq.insert(0, 50)
        with self.assertRaises(ValueError):
            pq.decrease_key(0, 150)
        with self.assertRaises(ValueError):
            pq.increase_key(0, 50)

    def test_stress_invariants(self):
        n = 50
        pq = IndexedMinPQ(n)
        for i in range(n):
            pq.insert(i, (i * 37 + 17) % 100)
            self.assertTrue(pq.check_invariants())

        for i in range(0, n, 2):
            pq.change_key(i, i * 2)
            self.assertTrue(pq.check_invariants())

        for i in range(1, n, 4):
            pq.erase(i)
            self.assertTrue(pq.check_invariants())

        last_key = -1
        while not pq.empty():
            cur_key = pq.min_key()
            _ = pq.pop_min_index()
            self.assertGreaterEqual(cur_key, last_key)
            self.assertTrue(pq.check_invariants())
            last_key = cur_key

    def test_dijkstra(self):
        graph = [
            [(1, 4), (2, 2)],
            [(3, 5)],
            [(1, 1)],
            [],
        ]
        dist = dijkstra_indexed_pq(graph, 0)
        self.assertEqual(dist, [0, 3, 2, 8])

    def test_prim_mst(self):
        graph = [
            [(1, 1), (2, 4)],
            [(0, 1), (2, 2), (3, 6)],
            [(0, 4), (1, 2), (3, 3)],
            [(1, 6), (2, 3)],
        ]
        total_weight, edges = prim_mst_indexed_pq(graph, 0)
        self.assertEqual(total_weight, 6)
        self.assertEqual(len(edges), 3)


if __name__ == "__main__":
    unittest.main()
