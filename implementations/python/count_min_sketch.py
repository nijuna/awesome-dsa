"""
Count-Min Sketch Frequency Estimation in Python.

Implements Count-Min Sketch with point queries, conservative updates,
and one-sided error validation with unit tests.
"""

import hashlib
import unittest
from typing import List


def hash_seed(val: str, seed: int) -> int:
    return int(hashlib.md5(f"{seed}:{val}".encode("utf-8")).hexdigest()[:16], 16)


class CountMinSketch:
    def __init__(self, depth: int = 5, width: int = 1000):
        self.depth = depth
        self.width = width
        self.table = [[0] * width for _ in range(depth)]
        self.seeds = [0x9E3779B9 + i * 0x85EBCA6B for i in range(depth)]
        self.total_count = 0

    def update(self, key: str, count: int = 1) -> None:
        self.total_count += count
        for i in range(self.depth):
            col = hash_seed(key, self.seeds[i]) % self.width
            self.table[i][col] += count

    def update_conservative(self, key: str, count: int = 1) -> None:
        self.total_count += count
        curr_min = self.estimate(key)
        for i in range(self.depth):
            col = hash_seed(key, self.seeds[i]) % self.width
            if self.table[i][col] == curr_min:
                self.table[i][col] += count

    def estimate(self, key: str) -> int:
        min_val = float("inf")
        for i in range(self.depth):
            col = hash_seed(key, self.seeds[i]) % self.width
            min_val = min(min_val, self.table[i][col])
        return int(min_val)


class TestCountMinSketch(unittest.TestCase):
    def test_frequency_estimation(self):
        cms = CountMinSketch(depth=5, width=1000)

        # Stream insertions
        for _ in range(300):
            cms.update("alpha")
        for _ in range(100):
            cms.update("beta")

        for i in range(2000):
            cms.update(f"noise_{i}")

        # One-sided error: never underestimate
        self.assertGreaterEqual(cms.estimate("alpha"), 300)
        self.assertGreaterEqual(cms.estimate("beta"), 100)

        # Error bound
        self.assertLessEqual(cms.estimate("alpha"), 330)
        self.assertLess(cms.estimate("unseen"), 15)


if __name__ == "__main__":
    unittest.main()
