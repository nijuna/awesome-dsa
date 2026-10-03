"""
High-Performance Caching Architectures Reference Implementation
===============================================================
Demonstrates the modern generation of caching algorithms that solve
the fundamental scan-resistance and frequency-staleness failures of naive LRU/LFU:
1. 2Q (Two Queues - Johnson & Shasha 1994)
2. ARC (Adaptive Replacement Cache - Megiddo & Modha 2003, IBM)
3. W-TinyLFU (Window TinyLFU - Caffeine Cache Architecture)
"""

from collections import OrderedDict
from typing import Optional, Any, Tuple
import math
import random
import unittest


# ============================================================================
# 1. BASELINE: STANDARD LRU CACHE
# ============================================================================

class LRUCache:
    def __init__(self, capacity: int):
        self.capacity = capacity
        self.items: OrderedDict = OrderedDict()

    def get(self, key: Any) -> Optional[Any]:
        if key not in self.items:
            return None
        self.items.move_to_end(key, last=True)
        return self.items[key]

    def put(self, key: Any, val: Any) -> Optional[Tuple[Any, Any]]:
        evicted = None
        if key in self.items:
            self.items.move_to_end(key, last=True)
            self.items[key] = val
            return None
        if len(self.items) >= self.capacity:
            evicted = self.items.popitem(last=False)
        self.items[key] = val
        return evicted

    def pop_tail(self) -> Optional[Tuple[Any, Any]]:
        if not self.items:
            return None
        return self.items.popitem(last=False)

    def peek_tail(self) -> Optional[Any]:
        if not self.items:
            return None
        return next(iter(self.items))

    def __len__(self) -> int:
        return len(self.items)


# ============================================================================
# 2. 2Q CACHE (Johnson & Shasha 1994)
# ============================================================================

class TwoQueueCache:
    """
    Two Queue Cache with A1in (FIFO), A1out (ghost history), and Am (frequent LRU).
    Provides scan resistance in O(1) time.
    """
    def __init__(self, capacity: int):
        self.capacity = capacity
        self.kin_cap = max(1, capacity // 4)
        self.kout_cap = max(1, capacity // 2)

        self.a1_in: OrderedDict = OrderedDict()  # Resident FIFO
        self.a1_out: OrderedDict = OrderedDict() # Ghost keys FIFO
        self.am: OrderedDict = OrderedDict()     # Resident LRU

    def get(self, key: Any) -> Optional[Any]:
        # 1. Check Am
        if key in self.am:
            self.am.move_to_end(key, last=True)
            return self.am[key]

        # 2. Check A1in: promote to Am on hit
        if key in self.a1_in:
            val = self.a1_in.pop(key)
            self._promote_to_am(key, val)
            return val

        return None

    def put(self, key: Any, val: Any) -> None:
        if key in self.am:
            self.am.move_to_end(key, last=True)
            self.am[key] = val
            return

        if key in self.a1_in:
            self.a1_in.pop(key)
            self._promote_to_am(key, val)
            return

        if key in self.a1_out:
            self.a1_out.pop(key)
            self._promote_to_am(key, val)
            return

        # Miss: enters A1in
        self._reclaim_space()
        self.a1_in[key] = val

    def _promote_to_am(self, key: Any, val: Any) -> None:
        self._reclaim_space()
        self.am[key] = val

    def _reclaim_space(self) -> None:
        if len(self.a1_in) + len(self.am) < self.capacity:
            return

        if len(self.a1_in) > self.kin_cap or not self.am:
            if self.a1_in:
                oldest_k, _ = self.a1_in.popitem(last=False)
                if len(self.a1_out) >= self.kout_cap:
                    self.a1_out.popitem(last=False)
                self.a1_out[oldest_k] = True
        elif self.am:
            self.am.popitem(last=False)

    def __len__(self) -> int:
        return len(self.a1_in) + len(self.am)


# ============================================================================
# 3. ADAPTIVE REPLACEMENT CACHE (ARC - Megiddo & Modha 2003, IBM)
# ============================================================================

class ARCCache:
    """
    Self-tuning Adaptive Replacement Cache tracking recency (T1, B1)
    and frequency (T2, B2) with dynamic target parameter p in [0, c].
    """
    def __init__(self, capacity: int):
        self.c = capacity
        self.p: float = 0.0

        self.t1: OrderedDict = OrderedDict()  # Recent resident
        self.t2: OrderedDict = OrderedDict()  # Frequent resident
        self.b1: OrderedDict = OrderedDict()  # Recent ghost
        self.b2: OrderedDict = OrderedDict()  # Frequent ghost

    def get(self, key: Any) -> Optional[Any]:
        # Case 1: In T1 -> promote to T2 MRU
        if key in self.t1:
            val = self.t1.pop(key)
            self.t2[key] = val
            return val

        # Case 2: In T2 -> move to MRU
        if key in self.t2:
            self.t2.move_to_end(key, last=True)
            return self.t2[key]

        return None

    def put(self, key: Any, val: Any) -> None:
        # Case 1: In T1 or T2
        if key in self.t1:
            self.t1.pop(key)
            self.t2[key] = val
            return

        if key in self.t2:
            self.t2.move_to_end(key, last=True)
            self.t2[key] = val
            return

        # Case 2: In B1 (Ghost recency hit)
        if key in self.b1:
            delta = 1.0 if len(self.b1) >= len(self.b2) else float(len(self.b2)) / len(self.b1)
            self.p = min(float(self.c), self.p + delta)
            self._replace(key)
            self.b1.pop(key)
            self.t2[key] = val
            return

        # Case 3: In B2 (Ghost frequency hit)
        if key in self.b2:
            delta = 1.0 if len(self.b2) >= len(self.b1) else float(len(self.b1)) / len(self.b2)
            self.p = max(0.0, self.p - delta)
            self._replace(key)
            self.b2.pop(key)
            self.t2[key] = val
            return

        # Case 4: Complete miss
        l1_size = len(self.t1) + len(self.b1)
        l2_size = len(self.t2) + len(self.b2)

        if l1_size == self.c:
            if len(self.t1) < self.c:
                if self.b1:
                    self.b1.popitem(last=False)
                self._replace(key)
            else:
                if self.t1:
                    self.t1.popitem(last=False)
        elif l1_size < self.c and (l1_size + l2_size >= self.c):
            if l1_size + l2_size == 2 * self.c:
                if self.b2:
                    self.b2.popitem(last=False)
            self._replace(key)

        self.t1[key] = val

    def _replace(self, key: Any) -> None:
        if self.t1 and ((len(self.t1) > int(self.p)) or (key in self.b2 and len(self.t1) == int(self.p))):
            k, _ = self.t1.popitem(last=False)
            self.b1[k] = True
        elif self.t2:
            k, _ = self.t2.popitem(last=False)
            self.b2[k] = True

    def __len__(self) -> int:
        return len(self.t1) + len(self.t2)


# ============================================================================
# 4. W-TINYLFU (Window TinyLFU - Caffeine Cache Architecture)
# ============================================================================

class CountMin4Bit:
    """4-bit Count-Min Sketch frequency estimator with periodic halving reset."""
    def __init__(self, capacity: int, depth: int = 4):
        self.depth = depth
        self.width = max(64, capacity * 4)
        self.table = [[0] * self.width for _ in range(depth)]
        self.additions = 0
        self.reset_threshold = capacity * 10

    def increment(self, key: Any) -> None:
        kh = hash(key)
        for d in range(self.depth):
            idx = (kh ^ (0x9e3779b97f4a7c15 * (d + 1))) % self.width
            if self.table[d][idx] < 15:
                self.table[d][idx] += 1
        self.additions += 1
        if self.additions >= self.reset_threshold:
            self.reset()

    def estimate(self, key: Any) -> int:
        kh = hash(key)
        min_v = 15
        for d in range(self.depth):
            idx = (kh ^ (0x9e3779b97f4a7c15 * (d + 1))) % self.width
            min_v = min(min_v, self.table[d][idx])
        return min_v

    def reset(self) -> None:
        for d in range(self.depth):
            for i in range(self.width):
                self.table[d][i] >>= 1
        self.additions = 0


class WTinyLFUCache:
    """
    Window TinyLFU cache:
    - Window Cache (LRU, ~5% of capacity) absorbs recency bursts.
    - Main Cache (Segmented LRU):
      * Protected Cache (~80% of main) holds hot working set.
      * Probation Cache (~20% of main) stages candidates with frequency admission filtering.
    """
    def __init__(self, capacity: int):
        self.capacity = capacity
        self.window_cap = max(1, capacity // 20)
        self.main_cap = capacity - self.window_cap

        prob_cap = max(1, (self.main_cap * 2) // 10)
        prot_cap = max(1, self.main_cap - prob_cap)

        self.filter = CountMin4Bit(capacity)
        self.window_cache = LRUCache(self.window_cap)
        self.probation_cache = LRUCache(prob_cap)
        self.protected_cache = LRUCache(prot_cap)

    def get(self, key: Any) -> Optional[Any]:
        self.filter.increment(key)

        v = self.window_cache.get(key)
        if v is not None:
            return v

        v = self.protected_cache.get(key)
        if v is not None:
            return v

        v = self.probation_cache.get(key)
        if v is not None:
            # Hit in probation: promote to protected
            if len(self.protected_cache) >= self.protected_cache.capacity:
                demoted = self.protected_cache.pop_tail()
                if demoted:
                    self.probation_cache.put(demoted[0], demoted[1])
            self.protected_cache.put(key, v)
            return v

        return None

    def put(self, key: Any, val: Any) -> None:
        self.filter.increment(key)

        if self.window_cache.get(key) is not None:
            self.window_cache.put(key, val)
            return
        if self.protected_cache.get(key) is not None:
            self.protected_cache.put(key, val)
            return
        if self.probation_cache.get(key) is not None:
            self.probation_cache.put(key, val)
            return

        if len(self.window_cache) >= self.window_cache.capacity:
            victim = self.window_cache.pop_tail()
            if victim:
                self._admit_to_main(victim[0], victim[1])

        self.window_cache.put(key, val)

    def _admit_to_main(self, cand_k: Any, cand_v: Any) -> None:
        if len(self.probation_cache) < self.probation_cache.capacity:
            self.probation_cache.put(cand_k, cand_v)
            return

        vict_k = self.probation_cache.peek_tail()
        if vict_k is not None:
            cand_freq = self.filter.estimate(cand_k)
            vict_freq = self.filter.estimate(vict_k)

            if cand_freq > vict_freq:
                self.probation_cache.pop_tail()
                self.probation_cache.put(cand_k, cand_v)

    def __len__(self) -> int:
        return len(self.window_cache) + len(self.probation_cache) + len(self.protected_cache)


# ============================================================================
# UNIT TESTS & BENCHMARK VERIFICATION
# ============================================================================

def read_through(cache: Any, key: Any) -> bool:
    v = cache.get(key)
    if v is not None:
        return True
    cache.put(key, key)
    return False


class TestHighPerformanceCaching(unittest.TestCase):
    def test_arc_adaptation_invariant(self):
        """Verify dynamic tuning of target recency parameter p."""
        arc = ARCCache(capacity=4)
        for i in range(1, 5):
            arc.put(i, f"val-{i}")
        self.assertEqual(len(arc), 4)

        # Access 2 to promote to T2
        self.assertEqual(arc.get(2), "val-2")

        # Put 5, evicts 1 to B1
        arc.put(5, "val-5")

        # Access 1 (ghost hit in B1): p must adapt upward
        arc.put(1, "val-1-again")
        self.assertGreater(arc.p, 0.0)

    def test_2q_and_wtiny_invariants(self):
        """Verify basic get/put, eviction, and structural integrity."""
        tq = TwoQueueCache(capacity=4)
        tq.put(10, 100)
        tq.put(20, 200)
        self.assertEqual(tq.get(10), 100)

        wtiny = WTinyLFUCache(capacity=10)
        for i in range(15):
            wtiny.put(i, i * 10)
        self.assertLessEqual(len(wtiny), 10)

    def test_scan_resistance_benchmark(self):
        """Verify ARC, 2Q, and W-TinyLFU outperform Naive LRU under sequential scan."""
        CACHE_SIZE = 40
        HOT_KEYS = 30
        SCAN_LENGTH = 200

        lru = LRUCache(CACHE_SIZE)
        tq = TwoQueueCache(CACHE_SIZE)
        arc = ARCCache(CACHE_SIZE)
        wtiny = WTinyLFUCache(CACHE_SIZE)

        # 1. Warm-up working set
        for _ in range(5):
            for k in range(HOT_KEYS):
                read_through(lru, k)
                read_through(tq, k)
                read_through(arc, k)
                read_through(wtiny, k)

        lru_hits = tq_hits = arc_hits = wtiny_hits = 0
        total_lookups = 0

        # 2. Interleave hot lookups with large sequential scan
        rng = random.Random(42)
        for step in range(1500):
            if rng.random() < 0.85:
                k = rng.randint(0, HOT_KEYS - 1)
                total_lookups += 1
                if read_through(lru, k): lru_hits += 1
                if read_through(tq, k): tq_hits += 1
                if read_through(arc, k): arc_hits += 1
                if read_through(wtiny, k): wtiny_hits += 1
            else:
                cold_k = 10000 + (step % SCAN_LENGTH)
                read_through(lru, cold_k)
                read_through(tq, cold_k)
                read_through(arc, cold_k)
                read_through(wtiny, cold_k)

        lru_hr = lru_hits / total_lookups * 100.0
        tq_hr = tq_hits / total_lookups * 100.0
        arc_hr = arc_hits / total_lookups * 100.0
        wtiny_hr = wtiny_hits / total_lookups * 100.0

        # Assert superior scan resistance
        self.assertGreater(arc_hr, lru_hr)
        self.assertGreater(tq_hr, lru_hr)
        self.assertGreater(wtiny_hr, lru_hr)


if __name__ == "__main__":
    unittest.main()
