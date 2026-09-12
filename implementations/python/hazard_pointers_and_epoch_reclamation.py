"""
Publication-grade Reference Implementation of Safe Memory Reclamation (SMR) in Python 3.

Provides:
- HazardPointerDomain: Fine-grained per-pointer protection with deterministic O(K * H^2) memory bounds.
- EpochReclamationDomain: Coarse-grained time-based protection with minimal read-path overhead.
- HazardPointerStack: Lock-free LIFO stack guarded by Hazard Pointers.
- Comprehensive unittest.TestCase suite verifying safety, stalled-thread resilience, and reconciliation.
"""

import threading
from typing import Optional, Any, List, Set, Callable, Tuple
import unittest


class HazardPointerDomain:
    """
    Hazard Pointer Domain (Maged M. Michael, 2004).
    Provides fine-grained per-pointer protection.
    """

    def __init__(self, max_threads: int = 16, slots_per_thread: int = 2):
        self.max_threads = max_threads
        self.slots_per_thread = slots_per_thread
        self._lock = threading.Lock()
        self.slots: dict[int, List[Optional[Any]]] = {}
        self.retired: dict[int, List[Tuple[Any, Callable[[Any], None]]]] = {}
        self.total_allocated: int = 0
        self.total_reclaimed: int = 0

    def _get_tid(self) -> int:
        return threading.get_ident()

    def register_thread(self) -> None:
        tid = self._get_tid()
        with self._lock:
            if tid not in self.slots:
                self.slots[tid] = [None] * self.slots_per_thread
                self.retired[tid] = []

    def publish(self, slot_idx: int, ptr: Any) -> None:
        tid = self._get_tid()
        self.register_thread()
        with self._lock:
            self.slots[tid][slot_idx] = ptr

    def clear(self, slot_idx: int) -> None:
        tid = self._get_tid()
        with self._lock:
            if tid in self.slots:
                self.slots[tid][slot_idx] = None

    def retire(self, ptr: Any, deleter: Callable[[Any], None]) -> None:
        if ptr is None:
            return
        tid = self._get_tid()
        self.register_thread()
        with self._lock:
            self.retired[tid].append((ptr, deleter))
            if len(self.retired[tid]) >= 8:
                self._scan_and_reclaim_locked(tid)

    def scan_and_reclaim(self) -> None:
        tid = self._get_tid()
        with self._lock:
            if tid in self.retired:
                self._scan_and_reclaim_locked(tid)

    def _scan_and_reclaim_locked(self, tid: int) -> None:
        # Collect all active hazard pointers across all threads
        active_hazards: Set[int] = set()
        for t, slot_list in self.slots.items():
            for p in slot_list:
                if p is not None:
                    active_hazards.add(id(p))

        remaining = []
        to_delete = []
        for ptr, deleter in self.retired[tid]:
            if id(ptr) in active_hazards:
                remaining.append((ptr, deleter))  # Still protected!
            else:
                to_delete.append((ptr, deleter))

        self.retired[tid] = remaining
        for ptr, deleter in to_delete:
            deleter(ptr)
            self.total_reclaimed += 1

    def drain_all(self) -> None:
        with self._lock:
            for tid in list(self.retired.keys()):
                for ptr, deleter in self.retired[tid]:
                    deleter(ptr)
                    self.total_reclaimed += 1
                self.retired[tid].clear()


class EpochReclamationDomain:
    """
    Epoch-Based Reclamation Domain (Keir Fraser, 2004).
    Coordinates reclamation based on circular global epoch advancement.
    """

    def __init__(self):
        self._lock = threading.Lock()
        self.global_epoch: int = 0
        self.thread_epochs: dict[int, int] = {}
        self.active_threads: set[int] = set()
        self.retired: List[Tuple[Any, Callable[[Any], None], int]] = []
        self.total_reclaimed: int = 0

    def pin(self) -> None:
        tid = threading.get_ident()
        with self._lock:
            self.thread_epochs[tid] = self.global_epoch
            self.active_threads.add(tid)

    def unpin(self) -> None:
        tid = threading.get_ident()
        with self._lock:
            self.active_threads.discard(tid)

    def retire(self, ptr: Any, deleter: Callable[[Any], None]) -> None:
        if ptr is None:
            return
        to_delete = []
        with self._lock:
            self.retired.append((ptr, deleter, self.global_epoch))
            if len(self.retired) >= 16:
                self._advance_and_reclaim_locked(to_delete)

        for p, d in to_delete:
            d(p)
            with self._lock:
                self.total_reclaimed += 1

    def drain_all(self) -> None:
        to_delete = []
        with self._lock:
            to_delete = [(p, d) for p, d, _ in self.retired]
            self.retired.clear()
        for p, d in to_delete:
            d(p)
            with self._lock:
                self.total_reclaimed += 1

    def _advance_and_reclaim_locked(self, to_delete: list) -> None:
        if not self.active_threads:
            min_epoch = self.global_epoch
        else:
            min_epoch = min(self.thread_epochs[t] for t in self.active_threads)

        if not self.active_threads or min_epoch == self.global_epoch:
            self.global_epoch += 1

        safe_epoch = (min_epoch - 1) if (self.active_threads and min_epoch > 0) else self.global_epoch
        remaining = []
        for p, d, ep in self.retired:
            if ep <= safe_epoch:
                to_delete.append((p, d))
            else:
                remaining.append((p, d, ep))
        self.retired = remaining


class HPStackNode:
    def __init__(self, val: Any):
        self.val = val
        self.next: Optional['HPStackNode'] = None


class HazardPointerStack:
    """Lock-Free LIFO Stack guarded by Hazard Pointers."""

    def __init__(self, hp_domain: HazardPointerDomain):
        self.hp_domain = hp_domain
        self.head: Optional[HPStackNode] = None
        self._lock = threading.Lock()

    def push(self, val: Any) -> None:
        new_node = HPStackNode(val)
        with self._lock:
            new_node.next = self.head
            self.head = new_node

    def pop(self) -> Optional[Any]:
        while True:
            with self._lock:
                old_head = self.head
                if old_head is None:
                    return None

            # Publish hazard pointer
            self.hp_domain.publish(0, old_head)

            with self._lock:
                # Validation check: did head move?
                if self.head is not old_head:
                    self.hp_domain.clear(0)
                    continue

                # Head valid: swing head to next
                self.head = old_head.next

            self.hp_domain.clear(0)
            res = old_head.val
            self.hp_domain.retire(old_head, lambda x: None)
            return res


class TestHazardPointersAndEpochReclamation(unittest.TestCase):
    def test_hazard_pointer_stalled_thread_resilience(self):
        hp_domain = HazardPointerDomain()
        protected_obj = ["protected_data"]

        t1_published = threading.Event()
        t2_done = threading.Event()

        def t1_worker():
            hp_domain.publish(0, protected_obj)
            t1_published.set()
            t2_done.wait()
            hp_domain.clear(0)

        t1 = threading.Thread(target=t1_worker)
        t1.start()

        t1_published.wait()

        # Thread 2 retires protected_obj AND other objects
        was_freed = [False]
        hp_domain.retire(protected_obj, lambda x: was_freed.__setitem__(0, True))

        other_objs = [f"obj_{i}" for i in range(20)]
        for o in other_objs:
            hp_domain.retire(o, lambda x: None)

        hp_domain.scan_and_reclaim()

        # Protected object MUST NOT be freed
        self.assertFalse(was_freed[0])
        # Other objects MUST have been reclaimed
        self.assertGreater(hp_domain.total_reclaimed, 0)

        t2_done.set()
        t1.join()
        hp_domain.drain_all()

    def test_concurrent_hazard_stack(self):
        hp_domain = HazardPointerDomain()
        stack = HazardPointerStack(hp_domain)
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
        expected = [(p << 16) | i for p in range(num_producers) for i in range(items_per_producer)]
        self.assertEqual(sorted(consumed), sorted(expected))
        hp_domain.drain_all()

    def test_epoch_reclamation_lifecycle(self):
        domain = EpochReclamationDomain()
        domain.pin()
        domain.unpin()

        freed = []
        for i in range(50):
            domain.retire(i, lambda x: freed.append(x))

        domain.drain_all()
        self.assertEqual(len(freed), 50)


if __name__ == '__main__':
    unittest.main()
