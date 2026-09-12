"""
Publication-grade Reference Implementation of Concurrent Queues, Stacks, and Safe Memory Reclamation in Python 3.

Provides:
- EpochReclaimer: Epoch-Based Reclamation (EBR) engine modeling 3-epoch retirement queues.
- TreiberStack: Lock-free LIFO stack with CAS synchronization and EBR memory safety.
- MichaelScottQueue: Lock-free FIFO queue with sentinel dummy node, cooperative helping, and EBR.
- Layered verification suite with sequential oracles, multi-threaded stress tests, and lifecycle assertions.
"""

import threading
from typing import Optional, Any, List, Tuple, Callable
import unittest


class EpochReclaimer:
    """
    Epoch-Based Reclamation (EBR) Engine.
    Coordinates safe deallocation across concurrent execution threads.
    """

    def __init__(self):
        self._lock = threading.Lock()
        self.global_epoch: int = 0
        self.thread_epochs: dict[int, int] = {}
        self.active_threads: set[int] = set()
        self.retired: List[Tuple[Any, Callable[[Any], None], int]] = []
        self.total_allocated: int = 0
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

    def retire(self, obj: Any, deleter: Callable[[Any], None]) -> None:
        if obj is None:
            return
        to_free = []
        with self._lock:
            self.retired.append((obj, deleter, self.global_epoch))
            if len(self.retired) >= 32:
                self._advance_and_reclaim_locked(to_free)

        for item, del_fn, _ in to_free:
            del_fn(item)
            with self._lock:
                self.total_reclaimed += 1

    def notify_alloc(self) -> None:
        with self._lock:
            self.total_allocated += 1

    def drain_all(self) -> None:
        to_free = []
        with self._lock:
            to_free = list(self.retired)
            self.retired.clear()
        for item, del_fn, _ in to_free:
            del_fn(item)
            with self._lock:
                self.total_reclaimed += 1

    def _advance_and_reclaim_locked(self, to_free: list) -> None:
        if not self.active_threads:
            min_epoch = self.global_epoch
        else:
            min_epoch = min(self.thread_epochs[tid] for tid in self.active_threads)

        if not self.active_threads or min_epoch == self.global_epoch:
            self.global_epoch += 1

        safe_epoch = (min_epoch - 1) if (self.active_threads and min_epoch > 0) else self.global_epoch
        remaining = []
        for entry in self.retired:
            obj, del_fn, ep = entry
            if ep <= safe_epoch:
                to_free.append(entry)
            else:
                remaining.append(entry)
        self.retired = remaining


class EpochGuard:
    """RAII context manager for Epoch-Based Reclamation."""

    def __init__(self, reclaimer: EpochReclaimer):
        self.reclaimer = reclaimer

    def __enter__(self):
        self.reclaimer.pin()
        return self

    def __exit__(self, exc_type, exc_val, exc_tb):
        self.reclaimer.unpin()


class TreiberStackNode:
    def __init__(self, val: Any):
        self.val = val
        self.next: Optional['TreiberStackNode'] = None


class TreiberStack:
    """
    Treiber Lock-Free LIFO Stack (1986).
    Employs compare-and-swap on head pointer with EBR memory safety.
    """

    def __init__(self, reclaimer: EpochReclaimer):
        self._reclaimer = reclaimer
        self._head: Optional[TreiberStackNode] = None
        self._cas_lock = threading.Lock()

    def _cas_head(self, expected: Optional[TreiberStackNode], desired: Optional[TreiberStackNode]) -> bool:
        with self._cas_lock:
            if self._head is expected:
                self._head = desired
                return True
            return False

    def push(self, val: Any) -> None:
        new_node = TreiberStackNode(val)
        self._reclaimer.notify_alloc()

        while True:
            old_head = self._head
            new_node.next = old_head
            if self._cas_head(old_head, new_node):
                return

    def pop(self) -> Optional[Any]:
        with EpochGuard(self._reclaimer):
            while True:
                old_head = self._head
                if old_head is None:
                    return None
                next_node = old_head.next
                if self._cas_head(old_head, next_node):
                    res = old_head.val
                    self._reclaimer.retire(old_head, lambda x: None)
                    return res

    def empty(self) -> bool:
        return self._head is None


class MSQueueNode:
    def __init__(self, val: Optional[Any] = None):
        self.val = val
        self.next: Optional['MSQueueNode'] = None
        self.next_lock = threading.Lock()

    def cas_next(self, expected: Optional['MSQueueNode'], desired: Optional['MSQueueNode']) -> bool:
        with self.next_lock:
            if self.next is expected:
                self.next = desired
                return True
            return False


class MichaelScottQueue:
    """
    Michael-Scott Lock-Free FIFO Queue (1996).
    Maintains sentinel dummy node, cooperative tail advance, and EBR.
    """

    def __init__(self, reclaimer: EpochReclaimer):
        self._reclaimer = reclaimer
        dummy = MSQueueNode(None)
        self._reclaimer.notify_alloc()
        self._head = dummy
        self._tail = dummy
        self._ptr_lock = threading.Lock()

    def _cas_tail(self, expected: MSQueueNode, desired: MSQueueNode) -> bool:
        with self._ptr_lock:
            if self._tail is expected:
                self._tail = desired
                return True
            return False

    def _cas_head(self, expected: MSQueueNode, desired: MSQueueNode) -> bool:
        with self._ptr_lock:
            if self._head is expected:
                self._head = desired
                return True
            return False

    def enqueue(self, val: Any) -> None:
        new_node = MSQueueNode(val)
        self._reclaimer.notify_alloc()

        with EpochGuard(self._reclaimer):
            while True:
                cur_tail = self._tail
                cur_next = cur_tail.next
                if cur_tail is self._tail:
                    if cur_next is None:
                        if cur_tail.cas_next(None, new_node):
                            self._cas_tail(cur_tail, new_node)
                            return
                    else:
                        self._cas_tail(cur_tail, cur_next)

    def dequeue(self) -> Optional[Any]:
        with EpochGuard(self._reclaimer):
            while True:
                cur_head = self._head
                cur_tail = self._tail
                cur_next = cur_head.next

                if cur_head is self._head:
                    if cur_head is cur_tail:
                        if cur_next is None:
                            return None
                        self._cas_tail(cur_tail, cur_next)
                    else:
                        if cur_next is not None:
                            res = cur_next.val
                            if self._cas_head(cur_head, cur_next):
                                self._reclaimer.retire(cur_head, lambda x: None)
                                return res


class TestConcurrentStructures(unittest.TestCase):
    def test_sequential_treiber_stack(self):
        reclaimer = EpochReclaimer()
        stack = TreiberStack(reclaimer)
        self.assertTrue(stack.empty())
        self.assertIsNone(stack.pop())

        for i in range(1, 51):
            stack.push(i)

        for i in range(50, 0, -1):
            self.assertEqual(stack.pop(), i)

        self.assertTrue(stack.empty())
        self.assertIsNone(stack.pop())
        reclaimer.drain_all()

    def test_sequential_ms_queue(self):
        reclaimer = EpochReclaimer()
        queue = MichaelScottQueue(reclaimer)
        self.assertIsNone(queue.dequeue())

        for i in range(1, 51):
            queue.enqueue(i)

        for i in range(1, 51):
            self.assertEqual(queue.dequeue(), i)

        self.assertIsNone(queue.dequeue())
        reclaimer.drain_all()

    def test_concurrent_treiber_stack(self):
        reclaimer = EpochReclaimer()
        stack = TreiberStack(reclaimer)
        num_producers = 4
        num_consumers = 4
        items_per_producer = 500
        total_items = num_producers * items_per_producer

        barrier = threading.Barrier(num_producers + num_consumers)
        producers_done = threading.Event()
        done_count = [0]
        done_lock = threading.Lock()

        consumed_by_thread = [[] for _ in range(num_consumers)]

        def producer_fn(p_id: int):
            barrier.wait()
            for i in range(items_per_producer):
                val = (p_id << 32) | i
                stack.push(val)
            with done_lock:
                done_count[0] += 1
                if done_count[0] == num_producers:
                    producers_done.set()

        def consumer_fn(c_id: int):
            barrier.wait()
            while True:
                val = stack.pop()
                if val is not None:
                    consumed_by_thread[c_id].append(val)
                elif producers_done.is_set():
                    val = stack.pop()
                    if val is not None:
                        consumed_by_thread[c_id].append(val)
                    else:
                        break

        threads = []
        for p in range(num_producers):
            t = threading.Thread(target=producer_fn, args=(p,))
            threads.append(t)
            t.start()

        for c in range(num_consumers):
            t = threading.Thread(target=consumer_fn, args=(c,))
            threads.append(t)
            t.start()

        for t in threads:
            t.join()

        all_consumed = []
        for lst in consumed_by_thread:
            all_consumed.extend(lst)

        self.assertEqual(len(all_consumed), total_items)
        all_consumed.sort()

        expected = []
        for p in range(num_producers):
            for i in range(items_per_producer):
                expected.append((p << 32) | i)
        expected.sort()

        self.assertEqual(all_consumed, expected)
        reclaimer.drain_all()

    def test_concurrent_ms_queue_mpsc_fifo(self):
        reclaimer = EpochReclaimer()
        queue = MichaelScottQueue(reclaimer)
        num_producers = 4
        items_per_producer = 500
        total_items = num_producers * items_per_producer

        barrier = threading.Barrier(num_producers + 1)
        producers_done = threading.Event()
        done_count = [0]
        done_lock = threading.Lock()

        consumed = []

        def producer_fn(p_id: int):
            barrier.wait()
            for i in range(items_per_producer):
                val = (p_id << 32) | i
                queue.enqueue(val)
            with done_lock:
                done_count[0] += 1
                if done_count[0] == num_producers:
                    producers_done.set()

        def single_consumer():
            barrier.wait()
            while True:
                val = queue.dequeue()
                if val is not None:
                    consumed.append(val)
                elif producers_done.is_set():
                    val = queue.dequeue()
                    if val is not None:
                        consumed.append(val)
                    else:
                        break

        threads = [threading.Thread(target=producer_fn, args=(p,)) for p in range(num_producers)]
        consumer_thread = threading.Thread(target=single_consumer)
        threads.append(consumer_thread)

        for t in threads:
            t.start()
        for t in threads:
            t.join()

        self.assertEqual(len(consumed), total_items)

        # Verify strict FIFO preservation per producer
        per_producer = [[] for _ in range(num_producers)]
        for val in consumed:
            p_id = val >> 32
            per_producer[p_id].append(val)

        for p in range(num_producers):
            self.assertEqual(len(per_producer[p]), items_per_producer)
            for i in range(items_per_producer):
                expected_val = (p << 32) | i
                self.assertEqual(per_producer[p][i], expected_val)

        reclaimer.drain_all()

    def test_smr_lifecycle(self):
        reclaimer = EpochReclaimer()
        stack = TreiberStack(reclaimer)
        for i in range(200):
            stack.push(i)
        for _ in range(200):
            stack.pop()
        reclaimer.drain_all()

        self.assertEqual(reclaimer.total_allocated, 200)
        self.assertEqual(reclaimer.total_reclaimed, 200)


if __name__ == '__main__':
    unittest.main()
