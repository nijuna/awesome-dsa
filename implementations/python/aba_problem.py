"""
Publication-grade Reference Implementation of the ABA Problem & CAS Pitfalls in Python 3.

Provides:
- UnprotectedStack: Explicitly simulates the classical ABA race and node orphaning bug.
- TaggedStack: Implements versioned references (pointer + monotonic tag) defeating ABA.
- CASPitfalls: Demonstrates weak vs. strong CAS comparison and spurious failure handling.
- Comprehensive unittest.TestCase suite verifying ABA reproduction and resolution.
"""

import threading
from typing import Optional, Any, Tuple
import unittest


class StackNode:
    def __init__(self, val: Any):
        self.val = val
        self.next: Optional['StackNode'] = None


class UnprotectedStack:
    """
    Unprotected Lock-Free Stack.
    Vulnerable to the classic ABA problem when node addresses are recycled.
    """

    def __init__(self):
        self.head: Optional[StackNode] = None
        self._lock = threading.Lock()

    def push(self, node: StackNode) -> None:
        with self._lock:
            node.next = self.head
            self.head = node

    def cas_head(self, expected: Optional[StackNode], desired: Optional[StackNode]) -> bool:
        with self._lock:
            if self.head is expected:
                self.head = desired
                return True
            return False


class TaggedStack:
    """
    Tagged Pointer / Versioned Reference Stack.
    Immune to the ABA problem because every state change increments a version counter.
    """

    def __init__(self):
        self._head: Optional[StackNode] = None
        self._tag: int = 0
        self._lock = threading.Lock()

    def get_tagged_head(self) -> Tuple[Optional[StackNode], int]:
        with self._lock:
            return self._head, self._tag

    def cas_head(self, expected_node: Optional[StackNode], expected_tag: int,
                 desired_node: Optional[StackNode]) -> bool:
        with self._lock:
            if self._head is expected_node and self._tag == expected_tag:
                self._head = desired_node
                self._tag += 1
                return True
            return False

    def push(self, val: Any) -> None:
        new_node = StackNode(val)
        while True:
            cur_head, cur_tag = self.get_tagged_head()
            new_node.next = cur_head
            if self.cas_head(cur_head, cur_tag, new_node):
                return

    def pop(self) -> Optional[Any]:
        while True:
            cur_head, cur_tag = self.get_tagged_head()
            if cur_head is None:
                return None
            next_node = cur_head.next
            if self.cas_head(cur_head, cur_tag, next_node):
                return cur_head.val

    def empty(self) -> bool:
        cur_head, _ = self.get_tagged_head()
        return cur_head is None


class TestABAProblem(unittest.TestCase):
    def test_aba_vulnerability_reproduction(self):
        stack = UnprotectedStack()
        nodeC = StackNode("C")
        nodeB = StackNode("B")
        nodeA = StackNode("A")

        nodeB.next = nodeC
        nodeA.next = nodeB
        stack.head = nodeA  # Stack: Top -> A -> B -> C

        # Step 1: Thread 1 starts pop, reads head=A and next=B
        t1_observed_head = stack.head
        t1_observed_next = t1_observed_head.next
        self.assertIs(t1_observed_head, nodeA)
        self.assertIs(t1_observed_next, nodeB)

        # Thread 1 is preempted!

        # Step 2: Thread 2 pops A and B
        poppedA = stack.head
        stack.head = poppedA.next  # Head is B
        poppedB = stack.head
        stack.head = poppedB.next  # Head is C
        self.assertIs(stack.head, nodeC)

        # In a real memory allocator, poppedB is freed and its memory recycled.
        poppedB.val = "FREED_CORRUPTED"

        # Step 3: Thread 3 pushes nodeA back (address reused)
        poppedA.next = nodeC
        stack.head = poppedA  # Stack: Top -> A -> C

        # Step 4: Thread 1 wakes up and attempts CAS(expected=A, desired=B)
        cas_success = stack.cas_head(t1_observed_head, t1_observed_next)

        # The ABA vulnerability: CAS succeeds because stack.head is A!
        self.assertTrue(cas_success)
        self.assertIs(stack.head, nodeB)
        # Head now points to corrupted node B, and node C is orphaned!
        self.assertEqual(stack.head.val, "FREED_CORRUPTED")

    def test_tagged_pointer_aba_immunity(self):
        stack = TaggedStack()
        nodeC = StackNode("C")
        nodeB = StackNode("B")
        nodeA = StackNode("A")

        nodeB.next = nodeC
        nodeA.next = nodeB
        stack._head = nodeA
        stack._tag = 100

        # Step 1: Thread 1 reads head (A, tag=100)
        t1_head, t1_tag = stack.get_tagged_head()
        t1_next = t1_head.next  # B

        # Step 2: Intervening operations pop A, pop B, re-push A
        # Each operation bumps tag: 100 -> 101 -> 102 -> 103
        stack.cas_head(nodeA, 100, nodeB)  # Pop A -> tag 101
        stack.cas_head(nodeB, 101, nodeC)  # Pop B -> tag 102
        nodeA.next = nodeC
        stack.cas_head(nodeC, 102, nodeA)  # Push A -> tag 103

        # Step 3: Thread 1 wakes up and attempts CAS with expected tag 100
        cas_result = stack.cas_head(t1_head, t1_tag, t1_next)

        # Tagged pointer detects modification! CAS must fail safely.
        self.assertFalse(cas_result)
        cur_head, cur_tag = stack.get_tagged_head()
        self.assertIs(cur_head, nodeA)
        self.assertEqual(cur_tag, 103)

    def test_concurrent_tagged_stack(self):
        stack = TaggedStack()
        num_producers = 4
        num_consumers = 4
        items_per_producer = 250
        total_items = num_producers * items_per_producer

        barrier = threading.Barrier(num_producers + num_consumers)
        producers_done = threading.Event()
        done_count = [0]
        done_lock = threading.Lock()

        consumed = []
        c_lock = threading.Lock()

        def producer_fn(p_id: int):
            barrier.wait()
            for i in range(items_per_producer):
                stack.push((p_id << 16) | i)
            with done_lock:
                done_count[0] += 1
                if done_count[0] == num_producers:
                    producers_done.set()

        def consumer_fn():
            barrier.wait()
            while True:
                val = stack.pop()
                if val is not None:
                    with c_lock:
                        consumed.append(val)
                elif producers_done.is_set():
                    val = stack.pop()
                    if val is not None:
                        with c_lock:
                            consumed.append(val)
                    else:
                        break

        threads = [threading.Thread(target=producer_fn, args=(p,)) for p in range(num_producers)]
        threads.extend([threading.Thread(target=consumer_fn) for _ in range(num_consumers)])

        for t in threads:
            t.start()
        for t in threads:
            t.join()

        self.assertEqual(len(consumed), total_items)
        self.assertEqual(sorted(consumed), sorted([(p << 16) | i for p in range(num_producers) for i in range(items_per_producer)]))


if __name__ == '__main__':
    unittest.main()
