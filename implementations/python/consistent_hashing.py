"""
Consistent Hashing with Virtual Nodes in Python.

Implements consistent hash ring with virtual nodes, bisect-based routing,
and minimal key redistribution verification upon cluster scaling.
"""

import bisect
import hashlib
import unittest
from typing import Dict, List, Optional


def hash_key(key: str) -> int:
    return int(hashlib.md5(key.encode("utf-8")).hexdigest(), 16)


class ConsistentHashRing:
    def __init__(self, vnodes_per_node: int = 50):
        self.vnodes_per_node = vnodes_per_node
        self.ring: List[int] = []
        self.ring_map: Dict[int, str] = {}
        self.nodes: List[str] = []

    def add_node(self, node: str) -> None:
        self.nodes.append(node)
        for i in range(self.vnodes_per_node):
            vkey = f"{node}#vn{i}"
            h = hash_key(vkey)
            self.ring_map[h] = node
            bisect.insort(self.ring, h)

    def remove_node(self, node: str) -> None:
        if node in self.nodes:
            self.nodes.remove(node)
        for i in range(self.vnodes_per_node):
            vkey = f"{node}#vn{i}"
            h = hash_key(vkey)
            if h in self.ring_map:
                del self.ring_map[h]
                idx = bisect.bisect_left(self.ring, h)
                if idx < len(self.ring) and self.ring[idx] == h:
                    self.ring.pop(idx)

    def get_node(self, key: str) -> Optional[str]:
        if not self.ring:
            return None
        h = hash_key(key)
        idx = bisect.bisect_right(self.ring, h)
        if idx == len(self.ring):
            idx = 0  # Wrap around
        return self.ring_map[self.ring[idx]]


class TestConsistentHashing(unittest.TestCase):
    def test_routing_and_migration(self):
        ring = ConsistentHashRing(vnodes_per_node=64)
        ring.add_node("server-A")
        ring.add_node("server-B")
        ring.add_node("server-C")

        # Routing consistency
        self.assertEqual(ring.get_node("user_1001"), ring.get_node("user_1001"))

        # Measure migration upon adding 4th server
        n_keys = 2000
        initial = [ring.get_node(f"key_{i}") for i in range(n_keys)]

        ring.add_node("server-D")
        migrated = 0
        for i in range(n_keys):
            new_node = ring.get_node(f"key_{i}")
            if new_node != initial[i]:
                migrated += 1
                self.assertEqual(new_node, "server-D")

        migration_ratio = migrated / n_keys
        # Theoretical is ~25% (allow 18% to 32%)
        self.assertGreaterEqual(migration_ratio, 0.18)
        self.assertLessEqual(migration_ratio, 0.32)


if __name__ == "__main__":
    unittest.main()
