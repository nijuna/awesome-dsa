"""
Reference Implementation: Standard Bloom Filter with Kirsch-Mitzenmacher Double Hashing
Demonstrates optimal bit-array sizing, optimal hash count calculation,
zero false negatives guarantee, and empirical false positive measurement.
"""

from __future__ import annotations
import math
import unittest
from typing import List, Tuple


class BloomFilter:
    """Space-efficient probabilistic data structure for set membership queries."""

    def __init__(self, expected_items: int, target_fp_prob: float = 0.01) -> None:
        if expected_items <= 0:
            expected_items = 1
        if not (0.0 < target_fp_prob < 1.0):
            target_fp_prob = 0.01

        self.expected_items = expected_items
        self.target_fp_prob = target_fp_prob

        # Optimal bit count: m = - (n * ln(p)) / (ln(2)^2)
        ln2 = math.log(2.0)
        m = - (expected_items * math.log(target_fp_prob)) / (ln2 * ln2)
        self.num_bits = max(64, int(math.ceil(m)))

        # Optimal hash count: k = (m / n) * ln(2)
        k = (self.num_bits / expected_items) * ln2
        self.num_hashes = max(1, int(round(k)))

        # Bit storage backed by integer words
        self.words: List[int] = [0] * ((self.num_bits + 63) // 64)
        self.items_added = 0

    @staticmethod
    def _hash_pair(key: str) -> Tuple[int, int]:
        """Produces two 32-bit hashes for Kirsch-Mitzenmacher double hashing."""
        h = 0x84222325CBF849C3
        for char in key:
            h = (h ^ ord(char)) * 0x100000001B3
            h &= 0xFFFFFFFFFFFFFFFF

        # SplitMix64
        h ^= (h >> 30)
        h = (h * 0xBF58476D1CE4E5B9) & 0xFFFFFFFFFFFFFFFF
        h ^= (h >> 27)
        h = (h * 0x94D049BB133111EB) & 0xFFFFFFFFFFFFFFFF
        h ^= (h >> 31)

        h1 = (h >> 32) & 0xFFFFFFFF
        h2 = h & 0xFFFFFFFF
        return h1, h2

    def add(self, key: str) -> None:
        """Inserts key into Bloom filter."""
        h1, h2 = self._hash_pair(key)
        for i in range(self.num_hashes):
            bit_idx = (h1 + i * h2) % self.num_bits
            self.words[bit_idx // 64] |= (1 << (bit_idx % 64))
        self.items_added += 1

    def __contains__(self, key: str) -> bool:
        """Returns False if key is definitely absent, True if probably present."""
        h1, h2 = self._hash_pair(key)
        for i in range(self.num_hashes):
            bit_idx = (h1 + i * h2) % self.num_bits
            if not (self.words[bit_idx // 64] & (1 << (bit_idx % 64))):
                return False
        return True


class TestBloomFilter(unittest.TestCase):
    def test_zero_false_negatives(self):
        n = 1000
        bf = BloomFilter(n, 0.01)
        keys = [f"item_{i}" for i in range(n)]

        for k in keys:
            bf.add(k)

        # Zero False Negatives Invariant
        for k in keys:
            self.assertIn(k, bf)

    def test_empirical_false_positive_rate(self):
        n = 2000
        target_p = 0.02
        bf = BloomFilter(n, target_p)

        for i in range(n):
            bf.add(f"member_{i}")

        # Query disjoint keys
        test_queries = 5000
        false_positives = sum(1 for i in range(test_queries) if f"disjoint_{i}" in bf)
        empirical_p = false_positives / test_queries

        # Empirical rate should be close to target 2%
        self.assertLess(empirical_p, 0.05)


if __name__ == "__main__":
    unittest.main()
