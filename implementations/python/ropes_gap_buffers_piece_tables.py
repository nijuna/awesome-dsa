"""
Reference implementations of Gap Buffers and Piece Tables for Text Editing.

Implements:
1. GapBuffer: Localized typing with moving gap and dynamic resizing.
2. PieceTable: Dual-buffer (original/add) piece table with piece coalescing.
3. Differential testing against Python string oracle.
"""

import random
import unittest
from typing import List


class GapBuffer:
    """Gap Buffer for localized cursor text editing."""

    def __init__(self, initial_capacity: int = 16):
        self.capacity = initial_capacity
        self.buffer = [''] * self.capacity
        self.gap_start = 0
        self.gap_end = self.capacity

    def __len__(self) -> int:
        return self.capacity - (self.gap_end - self.gap_start)

    def _grow(self, min_needed: int) -> None:
        new_cap = max(self.capacity * 2, self.capacity + min_needed + 16)
        new_buf = [''] * new_cap

        # Copy prefix
        for i in range(self.gap_start):
            new_buf[i] = self.buffer[i]

        # Copy suffix to end
        suffix_len = self.capacity - self.gap_end
        new_gap_end = new_cap - suffix_len
        for i in range(suffix_len):
            new_buf[new_gap_end + i] = self.buffer[self.gap_end + i]

        self.buffer = new_buf
        self.gap_end = new_gap_end
        self.capacity = new_cap

    def move_cursor(self, pos: int) -> None:
        assert 0 <= pos <= len(self)
        if pos < self.gap_start:
            shift = self.gap_start - pos
            while shift > 0:
                self.gap_start -= 1
                self.gap_end -= 1
                self.buffer[self.gap_end] = self.buffer[self.gap_start]
                shift -= 1
        elif pos > self.gap_start:
            shift = pos - self.gap_start
            while shift > 0:
                self.buffer[self.gap_start] = self.buffer[self.gap_end]
                self.gap_start += 1
                self.gap_end += 1
                shift -= 1

    def insert(self, text: str) -> None:
        if self.gap_end - self.gap_start < len(text):
            self._grow(len(text))
        for c in text:
            self.buffer[self.gap_start] = c
            self.gap_start += 1

    def erase_back(self) -> bool:
        if self.gap_start == 0:
            return False
        self.gap_start -= 1
        return True

    def to_string(self) -> str:
        prefix = "".join(self.buffer[:self.gap_start])
        suffix = "".join(self.buffer[self.gap_end:self.capacity])
        return prefix + suffix


class Piece:
    __slots__ = ('buffer_type', 'start', 'length')

    def __init__(self, buffer_type: str, start: int, length: int):
        self.buffer_type = buffer_type  # 'ORIGINAL' or 'ADD'
        self.start = start
        self.length = length


class PieceTable:
    """Piece Table decoupling text into Original/Add buffers and piece sequence."""

    def __init__(self, initial_text: str = ""):
        self.original = initial_text
        self.add = ""
        self.pieces: List[Piece] = []
        if initial_text:
            self.pieces.append(Piece('ORIGINAL', 0, len(initial_text)))

    def __len__(self) -> int:
        return sum(p.length for p in self.pieces)

    def insert(self, pos: int, text: str) -> None:
        if not text:
            return
        assert 0 <= pos <= len(self)

        add_offset = len(self.add)
        self.add += text
        text_len = len(text)

        if not self.pieces:
            self.pieces.append(Piece('ADD', add_offset, text_len))
            return

        # Coalescing optimization for typing at the end
        if (pos == len(self) and self.pieces[-1].buffer_type == 'ADD' and
                self.pieces[-1].start + self.pieces[-1].length == add_offset):
            self.pieces[-1].length += text_len
            return

        curr_pos = 0
        for i, p in enumerate(self.pieces):
            next_pos = curr_pos + p.length

            if pos == curr_pos:
                self.pieces.insert(i, Piece('ADD', add_offset, text_len))
                return
            elif curr_pos < pos < next_pos:
                left_len = pos - curr_pos
                right_len = p.length - left_len

                left_piece = Piece(p.buffer_type, p.start, left_len)
                new_piece = Piece('ADD', add_offset, text_len)
                right_piece = Piece(p.buffer_type, p.start + left_len, right_len)

                self.pieces[i] = left_piece
                self.pieces.insert(i + 1, new_piece)
                self.pieces.insert(i + 2, right_piece)
                return
            curr_pos = next_pos

        self.pieces.append(Piece('ADD', add_offset, text_len))

    def erase(self, pos: int, length: int) -> None:
        if length == 0 or not self.pieces:
            return
        assert pos + length <= len(self)

        curr_pos = 0
        new_pieces: List[Piece] = []

        for p in self.pieces:
            p_start = curr_pos
            p_end = curr_pos + p.length

            if p_end <= pos or p_start >= pos + length:
                new_pieces.append(p)
            else:
                if pos > p_start:
                    new_pieces.append(Piece(p.buffer_type, p.start, pos - p_start))
                if pos + length < p_end:
                    skipped = (pos + length) - p_start
                    new_pieces.append(Piece(p.buffer_type, p.start + skipped, p.length - skipped))
            curr_pos = p_end

        self.pieces = new_pieces

    def to_string(self) -> str:
        res = []
        for p in self.pieces:
            buf = self.original if p.buffer_type == 'ORIGINAL' else self.add
            res.append(buf[p.start:p.start + p.length])
        return "".join(res)


class TestTextStructures(unittest.TestCase):
    def test_basic_gap_buffer(self):
        gb = GapBuffer(8)
        gb.insert("Hello")
        gb.insert(" ")
        gb.insert("World")
        self.assertEqual(gb.to_string(), "Hello World")

        gb.move_cursor(5)
        gb.insert(" Beautiful")
        self.assertEqual(gb.to_string(), "Hello Beautiful World")

        gb.move_cursor(len(gb))
        gb.erase_back()
        self.assertEqual(gb.to_string(), "Hello Beautiful Worl")

    def test_basic_piece_table(self):
        pt = PieceTable("The brown fox")
        pt.insert(4, "quick ")
        self.assertEqual(pt.to_string(), "The quick brown fox")

        pt.insert(len(pt), " jumps")
        self.assertEqual(pt.to_string(), "The quick brown fox jumps")

        pt.erase(4, 6)
        self.assertEqual(pt.to_string(), "The brown fox jumps")

    def test_differential_fuzzing(self):
        oracle = "Initial Document Content for Testing"
        pt = PieceTable(oracle)
        gb = GapBuffer(64)
        gb.insert(oracle)

        rng = random.Random(42)
        for step in range(500):
            op = rng.randint(0, 1)

            if op == 0:
                pos = 0 if not oracle else rng.randint(0, len(oracle))
                text = f" [edit_{step}] "

                oracle = oracle[:pos] + text + oracle[pos:]
                pt.insert(pos, text)

                gb.move_cursor(pos)
                gb.insert(text)
            else:
                if oracle:
                    pos = rng.randint(0, len(oracle) - 1)
                    max_len = min(len(oracle) - pos, 8)
                    length = rng.randint(1, max_len)

                    oracle = oracle[:pos] + oracle[pos + length:]
                    pt.erase(pos, length)

                    gb.move_cursor(pos + length)
                    for _ in range(length):
                        gb.erase_back()

            self.assertEqual(len(pt), len(oracle))
            self.assertEqual(len(gb), len(oracle))
            self.assertEqual(pt.to_string(), oracle)
            self.assertEqual(gb.to_string(), oracle)


if __name__ == '__main__':
    unittest.main()
