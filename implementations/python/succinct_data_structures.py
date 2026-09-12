"""
Succinct Data Structures (Rank & Select Bitvectors) Implementation in Python 3.

Provides:
- SuccinctBitVector: Bitvector using 512-bit superblocks (8 words of 64 bits), achieving
  N + o(N) bits of space (6.25% auxiliary overhead), supporting:
  - access(i): O(1)
  - rank1(i): O(1)
  - rank0(i): O(1)
  - select1(k): O(log N)
  - select0(k): O(log N)
"""

import random
import unittest
from typing import Sequence, List


class SuccinctBitVector:
    """
    Succinct Bitvector with 512-bit superblocks.
    Space: N + 0.0625 N bits (6.25% overhead).
    """
    SUPERBLOCK_BITS = 512
    WORDS_PER_SUPERBLOCK = SUPERBLOCK_BITS // 64  # 8 words (64 bytes)

    def __init__(self, bits: Sequence[bool]):
        self.n = len(bits)
        self.num_words = (self.n + 63) // 64
        self.num_superblocks = (self.n + self.SUPERBLOCK_BITS - 1) // self.SUPERBLOCK_BITS

        self.data: List[int] = [0] * self.num_words
        self.superblock_ranks: List[int] = [0] * (self.num_superblocks + 1)

        for i, b in enumerate(bits):
            if b:
                self.data[i // 64] |= (1 << (i % 64))

        running = 0
        for sb in range(self.num_superblocks):
            self.superblock_ranks[sb] = running
            start_word = sb * self.WORDS_PER_SUPERBLOCK
            end_word = min(start_word + self.WORDS_PER_SUPERBLOCK, self.num_words)
            for w in range(start_word, end_word):
                running += bin(self.data[w]).count('1')
        self.superblock_ranks[self.num_superblocks] = running

    def __len__(self) -> int:
        return self.n

    def is_empty(self) -> bool:
        return self.n == 0

    def total_ones(self) -> int:
        return self.superblock_ranks[-1] if self.superblock_ranks else 0

    def total_zeros(self) -> int:
        return self.n - self.total_ones()

    def access(self, i: int) -> bool:
        return bool((self.data[i // 64] >> (i % 64)) & 1)

    def __getitem__(self, i: int) -> bool:
        return self.access(i)

    def rank1(self, i: int) -> int:
        """Count 1-bits in prefix [0, i]."""
        if i < 0:
            return 0
        if i >= self.n:
            i = self.n - 1

        sb = i // self.SUPERBLOCK_BITS
        rank = self.superblock_ranks[sb]

        target_word = i // 64
        sb_start_word = sb * self.WORDS_PER_SUPERBLOCK

        for w in range(sb_start_word, target_word):
            rank += bin(self.data[w]).count('1')

        bit_offset = i % 64
        mask = (1 << (bit_offset + 1)) - 1
        rank += bin(self.data[target_word] & mask).count('1')
        return rank

    def rank0(self, i: int) -> int:
        """Count 0-bits in prefix [0, i]."""
        if i < 0:
            return 0
        if i >= self.n:
            i = self.n - 1
        return (i + 1) - self.rank1(i)

    @staticmethod
    def _select_in_word(w: int, r: int) -> int:
        pos = 0
        for step in (32, 16, 8, 4, 2, 1):
            mask = (1 << (pos + step)) - 1
            c = bin(w & mask).count('1')
            if c < r:
                pos += step
        return pos

    def select1(self, k: int) -> int:
        """0-indexed position of the k-th 1-bit (1 <= k <= total_ones)."""
        if k <= 0 or k > self.total_ones():
            return -1

        # Binary search over superblocks
        low, high = 0, self.num_superblocks - 1
        sb = 0
        while low <= high:
            mid = (low + high) // 2
            if self.superblock_ranks[mid + 1] >= k:
                sb = mid
                if mid == 0:
                    break
                high = mid - 1
            else:
                low = mid + 1

        rank_needed = k - self.superblock_ranks[sb]
        start_word = sb * self.WORDS_PER_SUPERBLOCK
        end_word = min(start_word + self.WORDS_PER_SUPERBLOCK, self.num_words)

        for w in range(start_word, end_word):
            c = bin(self.data[w]).count('1')
            if c >= rank_needed:
                bit_idx = self._select_in_word(self.data[w], rank_needed)
                return w * 64 + bit_idx
            rank_needed -= c

        return -1

    def select0(self, k: int) -> int:
        """0-indexed position of the k-th 0-bit (1 <= k <= total_zeros)."""
        if k <= 0 or k > self.total_zeros():
            return -1

        low, high = 0, self.n - 1
        ans = -1
        while low <= high:
            mid = (low + high) // 2
            if self.rank0(mid) >= k:
                ans = mid
                high = mid - 1
            else:
                low = mid + 1
        return ans


class TestSuccinctBitVector(unittest.TestCase):
    def test_basic(self):
        bits = [True, False, True, True, False, False, True, False, True, True]
        sbv = SuccinctBitVector(bits)

        self.assertEqual(len(sbv), 10)
        self.assertEqual(sbv.total_ones(), 6)
        self.assertEqual(sbv.total_zeros(), 4)

        self.assertEqual(sbv.rank1(0), 1)
        self.assertEqual(sbv.rank1(1), 1)
        self.assertEqual(sbv.rank1(2), 2)
        self.assertEqual(sbv.rank1(9), 6)

        self.assertEqual(sbv.rank0(0), 0)
        self.assertEqual(sbv.rank0(1), 1)
        self.assertEqual(sbv.rank0(9), 4)

        self.assertEqual(sbv.select1(1), 0)
        self.assertEqual(sbv.select1(2), 2)
        self.assertEqual(sbv.select1(3), 3)
        self.assertEqual(sbv.select1(6), 9)

        self.assertEqual(sbv.select0(1), 1)
        self.assertEqual(sbv.select0(2), 4)
        self.assertEqual(sbv.select0(4), 7)

    def test_random_differential(self):
        rng = random.Random(42)
        N = 2500
        bits = [rng.random() < 0.35 for _ in range(N)]
        sbv = SuccinctBitVector(bits)

        ones_pos = [i for i, b in enumerate(bits) if b]
        zeros_pos = [i for i, b in enumerate(bits) if not b]

        self.assertEqual(sbv.total_ones(), len(ones_pos))
        self.assertEqual(sbv.total_zeros(), len(zeros_pos))

        r1 = 0
        r0 = 0
        for i in range(N):
            if bits[i]:
                r1 += 1
            else:
                r0 += 1
            self.assertEqual(sbv.access(i), bits[i])
            self.assertEqual(sbv.rank1(i), r1)
            self.assertEqual(sbv.rank0(i), r0)

        for k in range(1, len(ones_pos) + 1):
            self.assertEqual(sbv.select1(k), ones_pos[k - 1])

        for k in range(1, len(zeros_pos) + 1):
            self.assertEqual(sbv.select0(k), zeros_pos[k - 1])


if __name__ == '__main__':
    unittest.main()
