"""
High-Performance Non-Cryptographic Hash Functions and Universal Hashing.

Implements 64-bit FNV-1a, Murmur3 bit-mixer, SplitMix64, and the Carter-Wegman
2-universal hash family with extensive validation and distribution tests.
"""

import random
import unittest
from typing import Union


def fnv1a_64(data: Union[str, bytes]) -> int:
    """Computes 64-bit FNV-1a hash over strings or byte buffers."""
    if isinstance(data, str):
        data = data.encode("utf-8")
    hash_val = 0xCBF29CE484222325
    FNV_PRIME = 0x100000001B3
    MASK_64 = 0xFFFFFFFFFFFFFFFF
    for byte in data:
        hash_val ^= byte
        hash_val = (hash_val * FNV_PRIME) & MASK_64
    return hash_val


def murmur3_mix32(k: int) -> int:
    """MurmurHash3 32-bit finalizer bit-mixer."""
    MASK_32 = 0xFFFFFFFF
    k &= MASK_32
    k ^= k >> 16
    k = (k * 0x85EBCA6B) & MASK_32
    k ^= k >> 13
    k = (k * 0xC2B2AE35) & MASK_32
    k ^= k >> 16
    return k


def splitmix64(x: int) -> int:
    """SplitMix64 bit-mixer for 64-bit integers."""
    MASK_64 = 0xFFFFFFFFFFFFFFFF
    x = (x + 0x9E3779B97F4A7C15) & MASK_64
    x = ((x ^ (x >> 30)) * 0xBF58476D1CE4E5B9) & MASK_64
    x = ((x ^ (x >> 27)) * 0x94D049BB133111EB) & MASK_64
    return x ^ (x >> 31)


class UniversalHash64:
    """Carter-Wegman 2-Universal Hash Family on Mersenne prime 2^61 - 1."""

    MERSENNE_61 = (1 << 61) - 1

    def __init__(self, seed_a: int, seed_b: int, table_size: int):
        self.a = (seed_a % (self.MERSENNE_61 - 1)) + 1
        self.b = seed_b % self.MERSENNE_61
        self.m = table_size

    def hash(self, x: int) -> int:
        x &= self.MERSENNE_61
        prod = self.a * x + self.b
        mod_p = prod % self.MERSENNE_61
        return mod_p % self.m


class TestHashFunctions(unittest.TestCase):
    def test_fnv1a_determinism(self):
        h1 = fnv1a_64("hello world")
        h2 = fnv1a_64("hello world")
        h3 = fnv1a_64("hello worle")
        self.assertEqual(h1, h2)
        self.assertNotEqual(h1, h3)

    def test_murmur3_avalanche(self):
        random.seed(1337)
        SAMPLES = 5_000
        total_flipped = 0
        for _ in range(SAMPLES):
            x = random.randint(0, 0xFFFFFFFF)
            bit = random.randint(0, 31)
            y = x ^ (1 << bit)
            hx = murmur3_mix32(x)
            hy = murmur3_mix32(y)
            diff = hx ^ hy
            total_flipped += bin(diff).count("1")

        avg_flipped = total_flipped / SAMPLES
        self.assertTrue(15.0 <= avg_flipped <= 17.0)

    def test_splitmix64_distinct(self):
        seen = set()
        for i in range(1000):
            h = splitmix64(i)
            self.assertNotIn(h, seen)
            seen.add(h)

    def test_universal_hash_distribution(self):
        uh = UniversalHash64(123456789, 987654321, 50)
        counts = [0] * 50
        random.seed(42)
        for _ in range(50_000):
            k = random.randint(0, 1 << 60)
            b = uh.hash(k)
            counts[b] += 1

        for c in counts:
            self.assertTrue(800 < c < 1200)


if __name__ == "__main__":
    unittest.main()
