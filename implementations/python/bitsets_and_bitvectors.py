"""
Bitsets and Bitvectors implementation in Python.

Provides:
1. Word-level bit manipulation helper functions.
2. Dynamically sized BitVector backed by 64-bit word chunks.
3. Word-level bitwise operations (&, |, ^, ~).
4. Sieve of Eratosthenes prime generator using BitVector.
5. Comprehensive unit tests.
"""

import unittest
from typing import List, Callable


class WordBits:
    """Utilities for 64-bit integer words."""

    BITS_PER_WORD = 64
    WORD_MASK = (1 << 64) - 1

    @staticmethod
    def test(word: int, bit: int) -> bool:
        return (word & (1 << bit)) != 0

    @staticmethod
    def set(word: int, bit: int) -> int:
        return word | (1 << bit)

    @staticmethod
    def clear(word: int, bit: int) -> int:
        return word & ~(1 << bit)

    @staticmethod
    def flip(word: int, bit: int) -> int:
        return word ^ (1 << bit)

    @staticmethod
    def popcount(word: int) -> int:
        return word.bit_count()

    @staticmethod
    def ctz(word: int) -> int:
        """Count trailing zeros in 64-bit word."""
        if word == 0:
            return 64
        lsb = word & -word
        return lsb.bit_length() - 1

    @staticmethod
    def lowest_set_bit(word: int) -> int:
        return word & -word

    @staticmethod
    def iterate_set_bits(word: int, callback: Callable[[int], None]) -> None:
        while word != 0:
            lsb = word & -word
            bit = lsb.bit_length() - 1
            callback(bit)
            word &= (word - 1)


class BitVector:
    """
    Dynamically sized bitvector backed by 64-bit integer words.
    """

    BITS_PER_WORD = 64
    WORD_MASK = (1 << 64) - 1

    def __init__(self, num_bits: int = 0, init_val: bool = False):
        if num_bits < 0:
            raise ValueError("num_bits must be non-negative")
        self._num_bits = num_bits
        num_words = (num_bits + self.BITS_PER_WORD - 1) // self.BITS_PER_WORD
        init_word = self.WORD_MASK if init_val else 0
        self._words = [init_word] * num_words
        self._mask_trailing_bits()

    def _mask_trailing_bits(self) -> None:
        rem = self._num_bits % self.BITS_PER_WORD
        if rem != 0 and self._words:
            mask = (1 << rem) - 1
            self._words[-1] &= mask

    def __len__(self) -> int:
        return self._num_bits

    @property
    def num_words(self) -> int:
        return len(self._words)

    def _check_index(self, i: int) -> None:
        if i < 0 or i >= self._num_bits:
            raise IndexError(f"BitVector index {i} out of range for size {self._num_bits}")

    def test(self, i: int) -> bool:
        self._check_index(i)
        w_idx = i // self.BITS_PER_WORD
        offset = i % self.BITS_PER_WORD
        return (self._words[w_idx] & (1 << offset)) != 0

    def __getitem__(self, i: int) -> bool:
        return self.test(i)

    def set(self, i: int, val: bool = True) -> None:
        self._check_index(i)
        w_idx = i // self.BITS_PER_WORD
        offset = i % self.BITS_PER_WORD
        if val:
            self._words[w_idx] |= (1 << offset)
        else:
            self._words[w_idx] &= ~(1 << offset)

    def reset(self, i: int) -> None:
        self.set(i, False)

    def flip(self, i: int) -> None:
        self._check_index(i)
        w_idx = i // self.BITS_PER_WORD
        offset = i % self.BITS_PER_WORD
        self._words[w_idx] ^= (1 << offset)

    def clear(self) -> None:
        self._words = [0] * len(self._words)

    def set_all(self) -> None:
        self._words = [self.WORD_MASK] * len(self._words)
        self._mask_trailing_bits()

    def count(self) -> int:
        return sum(w.bit_count() for w in self._words)

    def any(self) -> bool:
        return any(w != 0 for w in self._words)

    def none(self) -> bool:
        return not self.any()

    def all(self) -> bool:
        if self._num_bits == 0:
            return True
        return self.count() == self._num_bits

    def get_set_bits(self) -> List[int]:
        res = []
        for w_idx, word in enumerate(self._words):
            base = w_idx * self.BITS_PER_WORD
            while word != 0:
                lsb = word & -word
                bit = lsb.bit_length() - 1
                res.append(base + bit)
                word &= (word - 1)
        return res

    def __invert__(self) -> 'BitVector':
        res = BitVector(self._num_bits)
        res._words = [w ^ self.WORD_MASK for w in self._words]
        res._mask_trailing_bits()
        return res

    def __and__(self, other: 'BitVector') -> 'BitVector':
        if self._num_bits != other._num_bits:
            raise ValueError("BitVector sizes must match for bitwise AND")
        res = BitVector(self._num_bits)
        res._words = [w1 & w2 for w1, w2 in zip(self._words, other._words)]
        return res

    def __or__(self, other: 'BitVector') -> 'BitVector':
        if self._num_bits != other._num_bits:
            raise ValueError("BitVector sizes must match for bitwise OR")
        res = BitVector(self._num_bits)
        res._words = [w1 | w2 for w1, w2 in zip(self._words, other._words)]
        return res

    def __xor__(self, other: 'BitVector') -> 'BitVector':
        if self._num_bits != other._num_bits:
            raise ValueError("BitVector sizes must match for bitwise XOR")
        res = BitVector(self._num_bits)
        res._words = [w1 ^ w2 for w1, w2 in zip(self._words, other._words)]
        return res

    def __eq__(self, other: object) -> bool:
        if not isinstance(other, BitVector):
            return False
        return self._num_bits == other._num_bits and self._words == other._words

    def to_string(self) -> str:
        return "".join("1" if self.test(i) else "0" for i in range(self._num_bits))


def sieve_primes(limit: int) -> List[int]:
    """Compute prime numbers up to limit using BitVector visited table."""
    if limit < 2:
        return []
    is_prime = BitVector(limit + 1, True)
    is_prime.reset(0)
    is_prime.reset(1)

    p = 2
    while p * p <= limit:
        if is_prime.test(p):
            for mult in range(p * p, limit + 1, p):
                is_prime.reset(mult)
        p += 1
    return is_prime.get_set_bits()


class TestBitsetsAndBitvectors(unittest.TestCase):
    def test_word_bits(self):
        w = 0
        w = WordBits.set(w, 5)
        w = WordBits.set(w, 63)
        self.assertTrue(WordBits.test(w, 5))
        self.assertTrue(WordBits.test(w, 63))
        self.assertFalse(WordBits.test(w, 0))
        self.assertEqual(WordBits.popcount(w), 2)
        self.assertEqual(WordBits.ctz(w), 5)
        self.assertEqual(WordBits.lowest_set_bit(w), 1 << 5)

        collected = []
        WordBits.iterate_set_bits(w, collected.append)
        self.assertEqual(collected, [5, 63])

        w = WordBits.clear(w, 5)
        self.assertFalse(WordBits.test(w, 5))
        self.assertEqual(WordBits.popcount(w), 1)

        w = WordBits.flip(w, 10)
        self.assertTrue(WordBits.test(w, 10))
        w = WordBits.flip(w, 10)
        self.assertFalse(WordBits.test(w, 10))

    def test_bitvector_sizes_and_init(self):
        b0 = BitVector(0)
        self.assertEqual(len(b0), 0)
        self.assertEqual(b0.num_words, 0)
        self.assertEqual(b0.count(), 0)
        self.assertTrue(b0.all())

        b1 = BitVector(1, True)
        self.assertEqual(len(b1), 1)
        self.assertEqual(b1.count(), 1)
        self.assertTrue(b1.test(0))

        b64 = BitVector(64, True)
        self.assertEqual(len(b64), 64)
        self.assertEqual(b64.num_words, 1)
        self.assertEqual(b64.count(), 64)

        b65 = BitVector(65, False)
        self.assertEqual(len(b65), 65)
        self.assertEqual(b65.num_words, 2)
        self.assertEqual(b65.count(), 0)
        self.assertTrue(b65.none())

    def test_modifications(self):
        bv = BitVector(150, False)
        indices = [0, 63, 64, 100, 149]
        for idx in indices:
            bv.set(idx)

        for idx in range(150):
            self.assertEqual(bv.test(idx), idx in indices)

        self.assertEqual(bv.count(), 5)
        self.assertTrue(bv.any())
        self.assertFalse(bv.none())
        self.assertFalse(bv.all())
        self.assertEqual(bv.get_set_bits(), indices)

        bv.flip(64)
        self.assertFalse(bv.test(64))
        self.assertEqual(bv.count(), 4)

    def test_bitwise_operations(self):
        a = BitVector(70, False)
        b = BitVector(70, False)

        a.set(10)
        a.set(65)
        a.set(69)

        b.set(10)
        b.set(20)
        b.set(65)

        and_res = a & b
        self.assertEqual(and_res.count(), 2)
        self.assertTrue(and_res.test(10) and and_res.test(65))
        self.assertFalse(and_res.test(20) or and_res.test(69))

        or_res = a | b
        self.assertEqual(or_res.count(), 4)

        xor_res = a ^ b
        self.assertEqual(xor_res.count(), 2)
        self.assertTrue(xor_res.test(20) and xor_res.test(69))

        not_a = ~a
        self.assertEqual(len(not_a), 70)
        self.assertEqual(not_a.count(), 70 - 3)
        self.assertFalse(not_a.test(10))
        self.assertTrue(not_a.test(0))

    def test_sieve(self):
        primes = sieve_primes(30)
        self.assertEqual(primes, [2, 3, 5, 7, 11, 13, 17, 19, 23, 29])
        primes100 = sieve_primes(100)
        self.assertEqual(len(primes100), 25)

    def test_exceptions(self):
        bv = BitVector(10)
        with self.assertRaises(IndexError):
            bv.test(10)
        with self.assertRaises(IndexError):
            bv.set(15)
        with self.assertRaises(ValueError):
            _ = bv & BitVector(11)


if __name__ == '__main__':
    unittest.main()
