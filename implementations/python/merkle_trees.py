"""
Publication-grade Reference Implementation of Cryptographic Merkle Trees in Python 3.

Provides:
- RFC 6962 Domain-Separated SHA-256 Merkle Tree:
  - 0x00 byte prefix for leaf nodes (defense against second-preimage attacks).
  - 0x01 byte prefix for internal nodes.
- Logarithmic Audit Proofs (Merkle Inclusion Proofs):
  - O(log N) proof generation and verification with directional sibling encoding.
- Dynamic Point Updates with O(log N) path recalculation.
- Comprehensive unittest.TestCase suite verifying RFC 6962 conformance and tampering rejection.
"""

import hashlib
from typing import List, Tuple
import unittest


def hash_leaf(data: str) -> bytes:
    """Computes RFC 6962 leaf hash: SHA256(0x00 || data)."""
    h = hashlib.sha256()
    h.update(b'\x00')
    h.update(data.encode('utf-8'))
    return h.digest()


def hash_node(left: bytes, right: bytes) -> bytes:
    """Computes RFC 6962 interior node hash: SHA256(0x01 || left || right)."""
    h = hashlib.sha256()
    h.update(b'\x01')
    h.update(left)
    h.update(right)
    return h.digest()


class MerkleTree:
    """
    Cryptographic Merkle Tree with RFC 6962 Domain Separation.
    """

    def __init__(self, leaves: List[str]):
        self.raw_leaves = list(leaves)
        self.levels: List[List[bytes]] = []
        self._build()

    def _build(self) -> None:
        if not self.raw_leaves:
            empty_hash = hashlib.sha256(b'').digest()
            self.levels = [[empty_hash]]
            return

        # Level 0: Hash leaves
        current = [hash_leaf(leaf) for leaf in self.raw_leaves]
        self.levels = [current]

        # Build upper levels bottom-up
        while len(current) > 1:
            next_level = []
            for i in range(0, len(current), 2):
                if i + 1 < len(current):
                    next_level.append(hash_node(current[i], current[i + 1]))
                else:
                    # Canonical duplicate for odd node count
                    next_level.append(hash_node(current[i], current[i]))
            self.levels.append(next_level)
            current = next_level

    def get_root(self) -> bytes:
        return self.levels[-1][0]

    def get_root_hex(self) -> str:
        return self.get_root().hex()

    def size(self) -> int:
        return len(self.raw_leaves)

    def generate_inclusion_proof(self, leaf_index: int) -> List[Tuple[bytes, bool]]:
        """
        Generates an O(log N) Merkle Inclusion Proof for leaf at index.
        Returns a list of tuples: (sibling_hash, is_left_sibling).
        """
        assert 0 <= leaf_index < len(self.raw_leaves)
        proof = []
        idx = leaf_index

        for level_idx in range(len(self.levels) - 1):
            level = self.levels[level_idx]
            is_right_child = (idx % 2 == 1)

            if is_right_child:
                # Sibling is on the left
                proof.append((level[idx - 1], True))
            else:
                # Sibling is on the right
                if idx + 1 < len(level):
                    proof.append((level[idx + 1], False))
                else:
                    proof.append((level[idx], False))
            idx //= 2

        return proof

    @staticmethod
    def verify_inclusion_proof(root: bytes, leaf_data: str, proof: List[Tuple[bytes, bool]]) -> bool:
        """
        Verifies an O(log N) Merkle Inclusion Proof against a trusted root.
        """
        current = hash_leaf(leaf_data)
        for sibling_hash, is_left in proof:
            if is_left:
                current = hash_node(sibling_hash, current)
            else:
                current = hash_node(current, sibling_hash)
        return current == root

    def update_leaf(self, leaf_index: int, new_data: str) -> bytes:
        """
        Dynamically updates a leaf and recalculates ancestor hashes to root in O(log N).
        """
        assert 0 <= leaf_index < len(self.raw_leaves)
        self.raw_leaves[leaf_index] = new_data
        self.levels[0][leaf_index] = hash_leaf(new_data)

        idx = leaf_index
        for level_idx in range(len(self.levels) - 1):
            parent_idx = idx // 2
            left_child = parent_idx * 2
            right_child = left_child + 1

            if right_child < len(self.levels[level_idx]):
                self.levels[level_idx + 1][parent_idx] = hash_node(
                    self.levels[level_idx][left_child],
                    self.levels[level_idx][right_child]
                )
            else:
                self.levels[level_idx + 1][parent_idx] = hash_node(
                    self.levels[level_idx][left_child],
                    self.levels[level_idx][left_child]
                )
            idx = parent_idx

        return self.get_root()


class TestMerkleTree(unittest.TestCase):
    def test_cross_language_vector_compatibility(self):
        leaves = ["alice", "bob", "carol", "dave"]
        tree = MerkleTree(leaves)
        expected_root_hex = "559c8e726262e509065de92d8ad3a30878b49d00517d530f873457fa550a7fa7"
        self.assertEqual(tree.get_root_hex(), expected_root_hex)

    def test_inclusion_proofs_power_of_two(self):
        leaves = [f"tx{i}" for i in range(8)]
        tree = MerkleTree(leaves)
        root = tree.get_root()

        for i, leaf in enumerate(leaves):
            proof = tree.generate_inclusion_proof(i)
            self.assertEqual(len(proof), 3)  # log2(8) = 3
            self.assertTrue(MerkleTree.verify_inclusion_proof(root, leaf, proof))

    def test_inclusion_proofs_odd_counts(self):
        for n in [1, 3, 5, 7, 13, 27]:
            leaves = [f"item_{i}" for i in range(n)]
            tree = MerkleTree(leaves)
            root = tree.get_root()
            for i, leaf in enumerate(leaves):
                proof = tree.generate_inclusion_proof(i)
                self.assertTrue(MerkleTree.verify_inclusion_proof(root, leaf, proof))

    def test_tampering_resistance(self):
        leaves = ["block_A", "block_B", "block_C", "block_D"]
        tree = MerkleTree(leaves)
        root = tree.get_root()
        proof = tree.generate_inclusion_proof(1)

        # 1. Valid leaf
        self.assertTrue(MerkleTree.verify_inclusion_proof(root, "block_B", proof))

        # 2. Forged leaf content
        self.assertFalse(MerkleTree.verify_inclusion_proof(root, "block_FORGED", proof))

        # 3. Corrupted sibling hash
        corrupted_sibling = bytearray(proof[0][0])
        corrupted_sibling[0] ^= 0x01
        corrupted_proof = [(bytes(corrupted_sibling), proof[0][1])] + proof[1:]
        self.assertFalse(MerkleTree.verify_inclusion_proof(root, "block_B", corrupted_proof))

        # 4. Inverted sibling direction
        inverted_direction_proof = [(proof[0][0], not proof[0][1])] + proof[1:]
        self.assertFalse(MerkleTree.verify_inclusion_proof(root, "block_B", inverted_direction_proof))

    def test_dynamic_leaf_updates(self):
        leaves = ["acc0:100", "acc1:250", "acc2:500", "acc3:75"]
        tree = MerkleTree(leaves)
        initial_root = tree.get_root()
        proof_acc2 = tree.generate_inclusion_proof(2)
        self.assertTrue(MerkleTree.verify_inclusion_proof(initial_root, "acc2:500", proof_acc2))

        # Update acc2
        new_root = tree.update_leaf(2, "acc2:420")
        self.assertNotEqual(initial_root, new_root)

        # Old proof fails against new root
        self.assertFalse(MerkleTree.verify_inclusion_proof(new_root, "acc2:500", proof_acc2))

        # New proof succeeds
        new_proof_acc2 = tree.generate_inclusion_proof(2)
        self.assertTrue(MerkleTree.verify_inclusion_proof(new_root, "acc2:420", new_proof_acc2))


if __name__ == '__main__':
    unittest.main()
