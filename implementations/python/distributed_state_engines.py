"""
Distributed State Engines Reference Implementation
==================================================
Demonstrates core distributed data structures and coordination primitives:
1. Consistent Hashing with Virtual Nodes (Dynamo-Style Ring & Preference Lists)
2. Vector Clocks & Causality / Conflict Detection (Fidge & Mattern)
3. Raft Replicated Log & State Machine Consensus (Ongaro & Ousterhout)
"""

from enum import Enum
from typing import Dict, List, Set, Tuple, Optional
import bisect
import hashlib
import unittest


# ============================================================================
# 1. CONSISTENT HASH RING WITH VIRTUAL NODES (Dynamo-Style)
# ============================================================================

class ConsistentHashRing:
    """
    Consistent Hash Ring mapping keys to nodes with minimal migration.
    Virtual nodes ensure uniform distribution across physical nodes.
    """
    def __init__(self, vnodes_per_node: int = 150):
        self.vnodes_per_node = vnodes_per_node
        self.ring: List[Tuple[int, str]] = []  # Sorted list of (token, node_id)
        self.tokens: List[int] = []            # Parallel token list for bisect
        self.physical_nodes: Set[str] = set()

    @staticmethod
    def _hash(key: str) -> int:
        h = hashlib.sha256(key.encode("utf-8")).digest()
        return int.from_bytes(h[:8], "big")

    def add_node(self, node_id: str) -> None:
        if node_id in self.physical_nodes:
            return
        self.physical_nodes.add(node_id)

        for v in range(self.vnodes_per_node):
            vkey = f"{node_id}#vnode#{v}"
            token = self._hash(vkey)
            self.ring.append((token, node_id))

        self.ring.sort(key=lambda x: x[0])
        self.tokens = [t for t, _ in self.ring]

    def remove_node(self, node_id: str) -> None:
        if node_id not in self.physical_nodes:
            return
        self.physical_nodes.remove(node_id)
        self.ring = [(t, n) for t, n in self.ring if n != node_id]
        self.tokens = [t for t, _ in self.ring]

    def get_node(self, key: str) -> Optional[str]:
        if not self.ring:
            return None
        token = self._hash(key)
        idx = bisect.bisect_left(self.tokens, token)
        if idx == len(self.ring):
            idx = 0
        return self.ring[idx][1]

    def get_preference_list(self, key: str, replication_factor: int) -> List[str]:
        """Dynamo preference list of N distinct physical nodes."""
        if not self.ring:
            return []
        token = self._hash(key)
        idx = bisect.bisect_left(self.tokens, token)
        if idx == len(self.ring):
            idx = 0

        pref: List[str] = []
        visited: Set[str] = set()
        start_idx = idx

        while len(pref) < min(replication_factor, len(self.physical_nodes)):
            node = self.ring[idx][1]
            if node not in visited:
                visited.add(node)
                pref.append(node)
            idx = (idx + 1) % len(self.ring)
            if idx == start_idx:
                break
        return pref


# ============================================================================
# 2. VECTOR CLOCK & CAUSALITY / CONFLICT DETECTION
# ============================================================================

class CausalityOrder(Enum):
    BEFORE = 1      # A < B
    AFTER = 2       # A > B
    EQUAL = 3       # A == B
    CONCURRENT = 4  # A || B (Conflict)


class VectorClock:
    """Vector Clock tracking causal dependencies across distributed actors."""
    def __init__(self, clock_dict: Optional[Dict[str, int]] = None):
        self.clock: Dict[str, int] = dict(clock_dict) if clock_dict else {}

    def tick(self, node_id: str) -> None:
        self.clock[node_id] = self.clock.get(node_id, 0) + 1

    def get(self, node_id: str) -> int:
        return self.clock.get(node_id, 0)

    def merge(self, other: "VectorClock") -> None:
        for k, v in other.clock.items():
            self.clock[k] = max(self.clock.get(k, 0), v)

    def compare(self, other: "VectorClock") -> CausalityOrder:
        all_keys = set(self.clock.keys()) | set(other.clock.keys())
        this_greater = False
        other_greater = False

        for k in all_keys:
            v1 = self.get(k)
            v2 = other.get(k)
            if v1 > v2:
                this_greater = True
            elif v2 > v1:
                other_greater = True

        if this_greater and other_greater:
            return CausalityOrder.CONCURRENT
        if this_greater:
            return CausalityOrder.AFTER
        if other_greater:
            return CausalityOrder.BEFORE
        return CausalityOrder.EQUAL


# ============================================================================
# 3. RAFT REPLICATED LOG & STATE MACHINE (Ongaro & Ousterhout)
# ============================================================================

class LogEntry:
    def __init__(self, term: int, index: int, command: str):
        self.term = term
        self.index = index
        self.command = command


class RaftServer:
    def __init__(self, server_id: str):
        self.id = server_id
        self.current_term = 0
        self.voted_for: Optional[str] = None
        self.log: List[LogEntry] = [LogEntry(0, 0, "")]  # 1-indexed (dummy at 0)
        self.commit_index = 0
        self.last_applied = 0
        self.applied_commands: List[str] = []

    @property
    def last_log_index(self) -> int:
        return self.log[-1].index

    @property
    def last_log_term(self) -> int:
        return self.log[-1].term

    def append_entries(self,
                       term: int,
                       leader_id: str,
                       prev_log_index: int,
                       prev_log_term: int,
                       entries: List[LogEntry],
                       leader_commit: int) -> bool:
        if term < self.current_term:
            return False

        if term > self.current_term:
            self.current_term = term
            self.voted_for = None

        if prev_log_index > self.last_log_index:
            return False

        if prev_log_index > 0 and self.log[prev_log_index].term != prev_log_term:
            return False

        insert_idx = prev_log_index + 1
        for entry in entries:
            if insert_idx <= self.last_log_index:
                if self.log[insert_idx].term != entry.term:
                    self.log = self.log[:insert_idx]
                    self.log.append(entry)
            else:
                self.log.append(entry)
            insert_idx += 1

        if leader_commit > self.commit_index:
            self.commit_index = min(leader_commit, self.last_log_index)
            self._apply_to_state_machine()

        return True

    def _apply_to_state_machine(self) -> None:
        while self.last_applied < self.commit_index:
            self.last_applied += 1
            self.applied_commands.append(self.log[self.last_applied].command)


class RaftClusterSimulation:
    def __init__(self, node_count: int = 3):
        self.nodes = [RaftServer(f"node_{i}") for i in range(node_count)]
        self.leader_idx = 0

    def elect_leader(self, idx: int, term: int) -> None:
        self.leader_idx = idx
        for n in self.nodes:
            n.current_term = term
            n.voted_for = self.nodes[idx].id

    def client_request(self, command: str) -> bool:
        leader = self.nodes[self.leader_idx]
        new_index = leader.last_log_index + 1
        new_entry = LogEntry(leader.current_term, new_index, command)
        leader.log.append(new_entry)

        matches = 1
        for i, node in enumerate(self.nodes):
            if i == self.leader_idx:
                continue
            prev_idx = new_entry.index - 1
            prev_term = leader.log[prev_idx].term
            success = node.append_entries(leader.current_term,
                                          leader.id,
                                          prev_idx,
                                          prev_term,
                                          [new_entry],
                                          leader.commit_index)
            if success:
                matches += 1

        if matches > len(self.nodes) // 2:
            leader.commit_index = new_index
            leader._apply_to_state_machine()
            for i, node in enumerate(self.nodes):
                if i != self.leader_idx:
                    node.commit_index = min(leader.commit_index, node.last_log_index)
                    node._apply_to_state_machine()
            return True
        return False


# ============================================================================
# UNIT TESTS & PROPERTY VERIFICATION
# ============================================================================

class TestDistributedStateEngines(unittest.TestCase):
    def test_consistent_hashing_and_migration(self):
        """Verify minimal key migration and preference list properties."""
        ring = ConsistentHashRing(vnodes_per_node=150)
        ring.add_node("node-A")
        ring.add_node("node-B")
        ring.add_node("node-C")
        ring.add_node("node-D")

        keys = [f"session_{i}" for i in range(5000)]
        initial_placement = {k: ring.get_node(k) for k in keys}

        # Add 5th node
        ring.add_node("node-E")

        migrated = 0
        for k in keys:
            new_node = ring.get_node(k)
            if new_node != initial_placement[k]:
                migrated += 1
                # Must move to the new node
                self.assertEqual(new_node, "node-E")

        migration_rate = migrated / len(keys) * 100.0
        # Expected around 20% (1/5)
        self.assertAlmostEqual(migration_rate, 20.0, delta=8.0)

        # Preference list
        pref = ring.get_preference_list("order_1234", 3)
        self.assertEqual(len(pref), 3)
        self.assertEqual(len(set(pref)), 3)

    def test_vector_clocks_and_concurrency(self):
        """Verify Lamport causality ordering and concurrent branch detection."""
        va = VectorClock()
        vb = VectorClock()

        va.tick("A")
        vb.merge(va)
        vb.tick("B")

        # va < vb
        self.assertEqual(va.compare(vb), CausalityOrder.BEFORE)
        self.assertEqual(vb.compare(va), CausalityOrder.AFTER)

        # Divergent concurrent edits
        va.tick("A")  # {A:2}
        vb.tick("B")  # {A:1, B:2}

        self.assertEqual(va.compare(vb), CausalityOrder.CONCURRENT)
        self.assertEqual(vb.compare(va), CausalityOrder.CONCURRENT)

        # Merge
        vc = VectorClock(va.clock)
        vc.merge(vb)
        vc.tick("A")

        self.assertEqual(va.compare(vc), CausalityOrder.BEFORE)
        self.assertEqual(vb.compare(vc), CausalityOrder.BEFORE)

    def test_raft_cluster_replicated_log(self):
        """Verify Raft leader log replication and majority commit consistency."""
        cluster = RaftClusterSimulation(3)
        cluster.elect_leader(0, term=1)

        self.assertTrue(cluster.client_request("SET a = 100"))
        self.assertTrue(cluster.client_request("SET b = 200"))
        self.assertTrue(cluster.client_request("ADD a b"))

        for node in cluster.nodes:
            self.assertEqual(node.commit_index, 3)
            self.assertEqual(node.applied_commands, ["SET a = 100", "SET b = 200", "ADD a b"])


if __name__ == "__main__":
    unittest.main()
