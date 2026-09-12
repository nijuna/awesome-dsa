"""
HyperLogLog Cardinality Estimation Algorithm in Python.

Implements 64-bit hashing, harmonic mean estimator, small-range LinearCounting
correction, and sketch merging with unit tests.
"""

import hashlib
import math
import unittest
from typing import List


def hash64(val: str) -> int:
    return int(hashlib.md5(val.encode("utf-8")).hexdigest()[:16], 16)


class HyperLogLog:
    def __init__(self, precision: int = 10):
        assert 4 <= precision <= 16
        self.p = precision
        self.m = 1 << precision
        self.registers = [0] * self.m

    def _get_alpha(self) -> float:
        if self.m == 16:
            return 0.673
        elif self.m == 32:
            return 0.697
        elif self.m == 64:
            return 0.709
        return 0.7213 / (1.0 + 1.079 / self.m)

    def _count_leading_zeros(self, w: int, max_bits: int) -> int:
        if w == 0:
            return max_bits
        binary_str = bin(w)[2:].zfill(max_bits)
        zeros = 0
        for char in binary_str:
            if char == "0":
                zeros += 1
            else:
                break
        return min(zeros + 1, max_bits)

    def add(self, item: str) -> None:
        x = hash64(item)
        idx = x >> (64 - self.p)
        w = x & ((1 << (64 - self.p)) - 1)
        lz = self._count_leading_zeros(w, 64 - self.p)
        if lz > self.registers[idx]:
            self.registers[idx] = lz

    def estimate(self) -> float:
        raw_sum = sum(2.0 ** (-r) for r in self.registers)
        alpha = self._get_alpha()
        raw_est = alpha * (self.m**2) / raw_sum

        empty_registers = self.registers.count(0)
        # Small range correction
        if raw_est <= 2.5 * self.m and empty_registers > 0:
            return self.m * math.log(self.m / empty_registers)

        return raw_est

    def merge(self, other: "HyperLogLog") -> None:
        assert self.p == other.p
        for i in range(self.m):
            self.registers[i] = max(self.registers[i], other.registers[i])


class TestHyperLogLog(unittest.TestCase):
    def test_cardinality_and_merge(self):
        hll1 = HyperLogLog(precision=10)
        n = 5000
        for i in range(n):
            hll1.add(f"user_id_{i}")

        est1 = hll1.estimate()
        error1 = abs(est1 - n) / n
        self.assertLess(error1, 0.10)

        # Merge test
        hll2 = HyperLogLog(precision=10)
        for i in range(n, n + 3000):
            hll2.add(f"user_id_{i}")

        hll1.merge(hll2)
        merged_est = hll1.estimate()
        error_merged = abs(merged_est - 8000) / 8000
        self.assertLess(error_merged, 0.10)


if __name__ == "__main__":
    unittest.main()
