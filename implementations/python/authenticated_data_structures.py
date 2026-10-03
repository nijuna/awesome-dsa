"""
Publication-grade Reference Implementation of Authenticated Data Structures (ADS) in Python 3.

Provides:
- AuthenticatedTreap: Cryptographically authenticated key-value dictionary with subtree digest accumulation.
- Logarithmic Membership Proofs: Proves key exists with value v.
- Logarithmic Non-Membership Proofs: Cryptographically proves key k is absent from dataset.
- Soundness & Tampering Rejection: Verifier detects any forged values, forged absences, or path discrepancies.
- Comprehensive unittest.TestCase suite verifying membership, non-membership, and forgery rejection.
"""

import hashlib
import random
from typing import Optional, List, Tuple
import unittest


def empty_hash() -> bytes:
    return hashlib.sha256(b"ADS_EMPTY_NODE").digest()


def hash_node(key: str, val: str, left_h: bytes, right_h: bytes) -> bytes:
    """
    Computes domain-separated authenticated node hash (prefix 0x02).
    0x02 || len(key)[4B] || key || len(val)[4B] || val || left_h[32B] || right_h[32B]
    """
    h = hashlib.sha256()
    h.update(b'\x02')

    k_bytes = key.encode('utf-8')
    h.update(len(k_bytes).to_bytes(4, byteorder='big'))
    h.update(k_bytes)

    v_bytes = val.encode('utf-8')
    h.update(len(v_bytes).to_bytes(4, byteorder='big'))
    h.update(v_bytes)

    h.update(left_h)
    h.update(right_h)
    return h.digest()


class TreapNode:
    def __init__(self, key: str, val: str, prio: int):
        self.key = key
        self.val = val
        self.priority = prio
        self.left: Optional['TreapNode'] = None
        self.right: Optional['TreapNode'] = None
        self.subtree_hash: bytes = b''
        self.update_hash()

    def update_hash(self) -> None:
        lh = self.left.subtree_hash if self.left else empty_hash()
        rh = self.right.subtree_hash if self.right else empty_hash()
        self.subtree_hash = hash_node(self.key, self.val, lh, rh)


class PathStep:
    def __init__(self, node_key: str, node_val: str, went_right: bool, sibling_hash: bytes):
        self.node_key = node_key
        self.node_val = node_val
        self.went_right = went_right
        self.sibling_hash = sibling_hash


class Proof:
    def __init__(self, exists: bool, key: str, val: str = ""):
        self.exists = exists
        self.key = key
        self.value = val
        self.target_left_hash: bytes = empty_hash()
        self.target_right_hash: bytes = empty_hash()
        self.ancestors: List[PathStep] = []


class AuthenticatedTreap:
    """
    Authenticated Merkle Treap Dictionary.
    """

    def __init__(self, seed: int = 42):
        self.root: Optional[TreapNode] = None
        self.rng = random.Random(seed)

    def get_root(self) -> bytes:
        return self.root.subtree_hash if self.root else empty_hash()

    def get_root_hex(self) -> str:
        return self.get_root().hex()

    def insert(self, key: str, val: str) -> None:
        prio = self.rng.randint(0, 0xFFFFFFFF)
        self.root = self._insert_rec(self.root, key, val, prio)

    def prove_membership(self, key: str) -> Optional[Proof]:
        proof = Proof(exists=True, key=key)
        curr = self.root

        while curr is not None:
            if curr.key == key:
                proof.value = curr.val
                proof.target_left_hash = curr.left.subtree_hash if curr.left else empty_hash()
                proof.target_right_hash = curr.right.subtree_hash if curr.right else empty_hash()
                return proof

            if key < curr.key:
                right_h = curr.right.subtree_hash if curr.right else empty_hash()
                proof.ancestors.append(PathStep(curr.key, curr.val, False, right_h))
                curr = curr.left
            else:
                left_h = curr.left.subtree_hash if curr.left else empty_hash()
                proof.ancestors.append(PathStep(curr.key, curr.val, True, left_h))
                curr = curr.right

        return None

    def prove_non_membership(self, key: str) -> Proof:
        proof = Proof(exists=False, key=key)
        curr = self.root

        while curr is not None:
            if curr.key == key:
                proof.exists = True  # Key exists, cannot prove absence
                return proof

            if key < curr.key:
                right_h = curr.right.subtree_hash if curr.right else empty_hash()
                proof.ancestors.append(PathStep(curr.key, curr.val, False, right_h))
                curr = curr.left
            else:
                left_h = curr.left.subtree_hash if curr.left else empty_hash()
                proof.ancestors.append(PathStep(curr.key, curr.val, True, left_h))
                curr = curr.right

        return proof

    @staticmethod
    def verify_membership(root: bytes, proof: Proof) -> bool:
        if not proof.exists:
            return False

        # Validate search path ordering
        for step in proof.ancestors:
            if step.went_right and not (proof.key > step.node_key):
                return False
            if not step.went_right and not (proof.key < step.node_key):
                return False

        current_h = hash_node(proof.key, proof.value, proof.target_left_hash, proof.target_right_hash)

        for step in reversed(proof.ancestors):
            if step.went_right:
                current_h = hash_node(step.node_key, step.node_val, step.sibling_hash, current_h)
            else:
                current_h = hash_node(step.node_key, step.node_val, current_h, step.sibling_hash)

        return current_h == root

    @staticmethod
    def verify_non_membership(root: bytes, proof: Proof) -> bool:
        if proof.exists:
            return False

        # Validate search path ordering
        for step in proof.ancestors:
            if step.went_right and not (proof.key > step.node_key):
                return False
            if not step.went_right and not (proof.key < step.node_key):
                return False

        current_h = empty_hash()

        for step in reversed(proof.ancestors):
            if step.went_right:
                current_h = hash_node(step.node_key, step.node_val, step.sibling_hash, current_h)
            else:
                current_h = hash_node(step.node_key, step.node_val, current_h, step.sibling_hash)

        return current_h == root

    def _rotate_right(self, y: TreapNode) -> TreapNode:
        x = y.left
        assert x is not None
        y.left = x.right
        x.right = y
        y.update_hash()
        x.update_hash()
        return x

    def _rotate_left(self, x: TreapNode) -> TreapNode:
        y = x.right
        assert y is not None
        x.right = y.left
        y.left = x
        x.update_hash()
        y.update_hash()
        return y

    def _insert_rec(self, node: Optional[TreapNode], key: str, val: str, prio: int) -> TreapNode:
        if node is None:
            return TreapNode(key, val, prio)

        if key < node.key:
            node.left = self._insert_rec(node.left, key, val, prio)
            if node.left.priority > node.priority:
                node = self._rotate_right(node)
        elif key > node.key:
            node.right = self._insert_rec(node.right, key, val, prio)
            if node.right.priority > node.priority:
                node = self._rotate_left(node)
        else:
            node.val = val

        node.update_hash()
        return node


class TestAuthenticatedDataStructures(unittest.TestCase):
    def test_membership_verification(self):
        treap = AuthenticatedTreap()
        data = {"apple": "1.50", "banana": "0.75", "cherry": "3.00", "date": "5.25", "elderberry": "8.10"}
        for k, v in data.items():
            treap.insert(k, v)

        root = treap.get_root()
        self.assertEqual(len(root), 32)

        proof = treap.prove_membership("cherry")
        self.assertIsNotNone(proof)
        self.assertEqual(proof.value, "3.00")
        self.assertTrue(AuthenticatedTreap.verify_membership(root, proof))

    def test_non_membership_verification(self):
        treap = AuthenticatedTreap()
        treap.insert("alpha", "10")
        treap.insert("charlie", "30")
        treap.insert("echo", "50")

        root = treap.get_root()

        # "bravo" is absent
        proof_bravo = treap.prove_non_membership("bravo")
        self.assertFalse(proof_bravo.exists)
        self.assertTrue(AuthenticatedTreap.verify_non_membership(root, proof_bravo))

        # "delta" is absent
        proof_delta = treap.prove_non_membership("delta")
        self.assertFalse(proof_delta.exists)
        self.assertTrue(AuthenticatedTreap.verify_non_membership(root, proof_delta))

    def test_forgery_rejection(self):
        treap = AuthenticatedTreap()
        treap.insert("user_alice", "$1000")
        treap.insert("user_bob", "$500")

        root = treap.get_root()
        valid_proof = treap.prove_membership("user_bob")
        self.assertIsNotNone(valid_proof)
        self.assertTrue(AuthenticatedTreap.verify_membership(root, valid_proof))

        # Forged value
        valid_proof.value = "$99999"
        self.assertFalse(AuthenticatedTreap.verify_membership(root, valid_proof))

        # Forged non-membership for existing user
        proof_existing = treap.prove_non_membership("user_alice")
        self.assertTrue(proof_existing.exists)
        proof_existing.exists = False  # try to forge absence
        self.assertFalse(AuthenticatedTreap.verify_non_membership(root, proof_existing))

    def test_scaling_and_randomized(self):
        treap = AuthenticatedTreap()
        for i in range(100):
            treap.insert(f"k_{i * 2:04d}", f"v_{i * 2}")

        root = treap.get_root()

        for i in range(20):
            # Even keys exist
            p = treap.prove_membership(f"k_{i * 2:04d}")
            self.assertIsNotNone(p)
            self.assertTrue(AuthenticatedTreap.verify_membership(root, p))

            # Odd keys absent
            p_absent = treap.prove_non_membership(f"k_{i * 2 + 1:04d}")
            self.assertFalse(p_absent.exists)
            self.assertTrue(AuthenticatedTreap.verify_non_membership(root, p_absent))


if __name__ == '__main__':
    unittest.main()
