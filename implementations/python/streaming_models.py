"""
Streaming Models and Algorithms Implementation in Python 3.

Provides:
- MisraGries: Deterministic epsilon-heavy hitters in the Cash Register streaming model.
- ReservoirSampler: Single-pass uniform random sampling of k elements (Algorithm R).
- DGIM: Sliding window frequency estimation for 1-bits with (1 +/- eps) relative error.
"""

import random
import unittest
from collections import deque
from typing import Dict, List, Any


class MisraGries:
    """
    Misra-Gries algorithm for deterministic epsilon-heavy hitters.
    Guarantees retention of all items with frequency > N / k using O(k) space.
    """
    def __init__(self, k: int):
        assert k > 1
        self.k = k
        self.counters: Dict[Any, int] = {}
        self.total = 0

    def process(self, item: Any):
        self.total += 1
        if item in self.counters:
            self.counters[item] += 1
        elif len(self.counters) < self.k - 1:
            self.counters[item] = 1
        else:
            to_remove = []
            for key in list(self.counters.keys()):
                self.counters[key] -= 1
                if self.counters[key] == 0:
                    to_remove.append(key)
            for key in to_remove:
                del self.counters[key]

    def estimate(self, item: Any) -> int:
        return self.counters.get(item, 0)


class ReservoirSampler:
    """
    Reservoir sampling (Algorithm R).
    Uniformly selects k items from an unknown length stream.
    """
    def __init__(self, k: int, seed: int = 42):
        assert k > 0
        self.k = k
        self.reservoir: List[Any] = []
        self.count = 0
        self.rng = random.Random(seed)

    def process(self, item: Any):
        self.count += 1
        if len(self.reservoir) < self.k:
            self.reservoir.append(item)
        else:
            j = self.rng.randint(0, self.count - 1)
            if j < self.k:
                self.reservoir[j] = item

    def sample(self) -> List[Any]:
        return list(self.reservoir)


class DGIM:
    """
    DGIM algorithm for counting 1-bits in a sliding window of width W.
    Guarantees relative error <= eps using O((1 / eps) * log^2 W) bits.
    """
    class Bucket:
        __slots__ = ('timestamp', 'size')

        def __init__(self, timestamp: int, size: int):
            self.timestamp = timestamp
            self.size = size

    def __init__(self, window_size: int, eps: float = 0.5):
        assert window_size > 0
        assert 0 < eps <= 1.0
        self.window_size = window_size
        self.k = max(2, int(1.0 / eps + 0.9999))
        self.current_time = 0
        self.buckets: deque['DGIM.Bucket'] = deque()

    def process(self, bit: bool):
        self.current_time += 1

        # Drop expired buckets (where most recent bit is older than the window)
        while self.buckets and self.buckets[-1].timestamp + self.window_size <= self.current_time:
            self.buckets.pop()

        if not bit:
            return

        # Add bucket of size 1 at front (newest timestamp = current_time)
        self.buckets.appendleft(self.Bucket(self.current_time, 1))

        # Merge check from size 1 upwards
        cur_size = 1
        while True:
            matching = [i for i, b in enumerate(self.buckets) if b.size == cur_size]
            if len(matching) > self.k:
                idx_newer = matching[-2]
                idx_older = matching[-1]
                self.buckets[idx_newer].size *= 2
                del self.buckets[idx_older]
                cur_size *= 2
            else:
                break

    def count_ones(self) -> int:
        if not self.buckets:
            return 0
        total = sum(b.size for b in self.buckets)
        last_size = self.buckets[-1].size
        return total - (last_size // 2)


class TestStreamingModels(unittest.TestCase):
    def test_misra_gries(self):
        mg = MisraGries(4)
        for _ in range(400):
            mg.process("A")
        for _ in range(300):
            mg.process("B")
        for i in range(300):
            mg.process(f"noise_{i}")

        self.assertIn("A", mg.counters)
        self.assertIn("B", mg.counters)
        self.assertGreaterEqual(mg.estimate("A"), 400 - 1000 // 4)
        self.assertGreaterEqual(mg.estimate("B"), 300 - 1000 // 4)

    def test_reservoir_sampler(self):
        rs = ReservoirSampler(10, seed=1337)
        for i in range(1000):
            rs.process(i)
        self.assertEqual(len(rs.sample()), 10)
        self.assertEqual(rs.count, 1000)

    def test_dgim(self):
        W = 100
        dgim = DGIM(W, eps=0.5)
        window = deque()
        rng = random.Random(42)

        for step in range(3000):
            b = (rng.randint(0, 2) == 0)
            dgim.process(b)
            window.append(b)
            if len(window) > W:
                window.popleft()

            if step >= W:
                true_count = sum(window)
                est_count = dgim.count_ones()
                if true_count > 0:
                    rel_err = abs(est_count - true_count) / true_count
                    self.assertLessEqual(rel_err, 0.5001)


if __name__ == '__main__':
    unittest.main()
