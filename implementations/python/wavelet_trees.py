"""
Wavelet Tree Implementation in Python 3.

Provides:
- BitVector: Succinct bitvector supporting O(1) rank queries using 64-bit blocks.
- WaveletTree: Succinct sequence representation over an alphabet Sigma, supporting:
  - access(i): O(log |Sigma|)
  - rank(i, c): O(log |Sigma|)
  - range_count(L, R, c): O(log |Sigma|)
  - quantile(L, R, k): O(log |Sigma|)
  - range_frequency(L, R, low, high): O(log |Sigma|)
"""

import random
import unittest
from typing import List, Sequence, Optional


class BitVector:
    """
    BitVector supporting O(1) rank queries via 64-bit packed words and prefix directory.
    """
    def __init__(self, bits: Sequence[bool]):
        self.n = len(bits)
        num_words = (self.n + 63) // 64
        self.words = [0] * num_words
        self.prefix_popcounts = [0] * (num_words + 1)

        for i, b in enumerate(bits):
            if b:
                self.words[i // 64] |= (1 << (i % 64))

        running = 0
        for i in range(num_words):
            self.prefix_popcounts[i] = running
            running += bin(self.words[i]).count('1')
        self.prefix_popcounts[num_words] = running

    def __len__(self) -> int:
        return self.n

    def get(self, i: int) -> bool:
        return bool((self.words[i // 64] >> (i % 64)) & 1)

    def rank1(self, i: int) -> int:
        """Count 1-bits in prefix [0, i]."""
        if i < 0:
            return 0
        if i >= self.n:
            i = self.n - 1
        word_idx = i // 64
        bit_idx = i % 64
        count = self.prefix_popcounts[word_idx]
        mask = (1 << (bit_idx + 1)) - 1
        count += bin(self.words[word_idx] & mask).count('1')
        return count

    def rank0(self, i: int) -> int:
        """Count 0-bits in prefix [0, i]."""
        if i < 0:
            return 0
        if i >= self.n:
            i = self.n - 1
        return (i + 1) - self.rank1(i)


class WaveletTree:
    """
    Wavelet Tree over integer sequence with alphabet range [low, high].
    """
    def __init__(self, arr: Sequence[int], low: int, high: int):
        self.low = low
        self.high = high
        self.left: Optional['WaveletTree'] = None
        self.right: Optional['WaveletTree'] = None

        if not arr or low >= high:
            self.bv = BitVector([])
            return

        mid = low + (high - low) // 2
        bits = [x > mid for x in arr]
        self.bv = BitVector(bits)

        left_arr = [x for x in arr if x <= mid]
        right_arr = [x for x in arr if x > mid]

        self.left = WaveletTree(left_arr, low, mid)
        self.right = WaveletTree(right_arr, mid + 1, high)

    def access(self, i: int) -> int:
        """Retrieve element at index i (0-indexed)."""
        if self.low == self.high:
            return self.low
        bit = self.bv.get(i)
        if not bit:
            assert self.left is not None
            return self.left.access(self.bv.rank0(i) - 1)
        else:
            assert self.right is not None
            return self.right.access(self.bv.rank1(i) - 1)

    def rank(self, i: int, c: int) -> int:
        """Count occurrences of symbol c in prefix [0, i]."""
        if i < 0 or c < self.low or c > self.high:
            return 0
        if self.low == self.high:
            return i + 1

        mid = self.low + (self.high - self.low) // 2
        if c <= mid:
            assert self.left is not None
            return self.left.rank(self.bv.rank0(i) - 1, c)
        else:
            assert self.right is not None
            return self.right.rank(self.bv.rank1(i) - 1, c)

    def range_count(self, L: int, R: int, c: int) -> int:
        """Count occurrences of symbol c in range [L, R]."""
        if L > R:
            return 0
        return self.rank(R, c) - self.rank(L - 1, c)

    def quantile(self, L: int, R: int, k: int) -> int:
        """Find k-th smallest element in range [L, R] (1-indexed)."""
        if self.low == self.high:
            return self.low

        left_L = self.bv.rank0(L - 1)
        left_R = self.bv.rank0(R) - 1
        zeros_in_range = max(0, left_R - left_L + 1)

        if k <= zeros_in_range:
            assert self.left is not None
            return self.left.quantile(left_L, left_R, k)
        else:
            assert self.right is not None
            right_L = self.bv.rank1(L - 1)
            right_R = self.bv.rank1(R) - 1
            return self.right.quantile(right_L, right_R, k - zeros_in_range)

    def range_frequency(self, L: int, R: int, val_low: int, val_high: int) -> int:
        """Count number of elements in range [L, R] whose values lie in [val_low, val_high]."""
        if L > R or val_low > self.high or val_high < self.low:
            return 0
        if val_low <= self.low and self.high <= val_high:
            return R - L + 1

        left_L = self.bv.rank0(L - 1)
        left_R = self.bv.rank0(R) - 1

        right_L = self.bv.rank1(L - 1)
        right_R = self.bv.rank1(R) - 1

        res = 0
        if left_R >= left_L and self.left:
            res += self.left.range_frequency(left_L, left_R, val_low, val_high)
        if right_R >= right_L and self.right:
            res += self.right.range_frequency(right_L, right_R, val_low, val_high)
        return res


class TestWaveletTree(unittest.TestCase):
    def test_basic_queries(self):
        arr = [5, 2, 8, 3, 9, 2, 7, 1, 6, 4, 3, 8]
        wt = WaveletTree(arr, 1, 9)

        for i, val in enumerate(arr):
            self.assertEqual(wt.access(i), val)

        self.assertEqual(wt.range_count(0, 11, 2), 2)
        self.assertEqual(wt.range_count(0, 11, 8), 2)
        self.assertEqual(wt.range_count(0, 11, 10), 0)

        # {5, 2, 8, 3, 9} -> sorted: {2, 3, 5, 8, 9}
        self.assertEqual(wt.quantile(0, 4, 1), 2)
        self.assertEqual(wt.quantile(0, 4, 2), 3)
        self.assertEqual(wt.quantile(0, 4, 3), 5)
        self.assertEqual(wt.quantile(0, 4, 5), 9)

        self.assertEqual(wt.range_frequency(0, 4, 2, 5), 3)

    def test_random_differential(self):
        rng = random.Random(42)
        N = 300
        arr = [rng.randint(0, 50) for _ in range(N)]
        wt = WaveletTree(arr, 0, 50)

        for _ in range(1500):
            l = rng.randint(0, N - 1)
            r = rng.randint(0, N - 1)
            if l > r:
                l, r = r, l

            sub = arr[l:r + 1]

            # Quantile
            k = rng.randint(1, len(sub))
            sorted_sub = sorted(sub)
            self.assertEqual(wt.quantile(l, r, k), sorted_sub[k - 1])

            # Range count
            c = rng.randint(0, 50)
            self.assertEqual(wt.range_count(l, r, c), sub.count(c))

            # Range frequency
            v1 = rng.randint(0, 50)
            v2 = rng.randint(0, 50)
            if v1 > v2:
                v1, v2 = v2, v1
            expected_freq = sum(1 for x in sub if v1 <= x <= v2)
            self.assertEqual(wt.range_frequency(l, r, v1, v2), expected_freq)


if __name__ == '__main__':
    unittest.main()
