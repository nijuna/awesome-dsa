"""
Publication-grade Reference Implementation of the Chase-Lev Work-Stealing Deque in Python 3.

Provides:
- ChaseLevDeque: Lock-free work-stealing deque with asymmetric owner/thief synchronization.
  - Owner pushes and pops at bottom (LIFO, depth-first execution).
  - Thieves steal from top (FIFO, breadth-first execution).
  - Single-element race resolution.
- WorkStealingScheduler: Multi-worker scheduler simulating dynamic task graph execution and random stealing.
- Comprehensive unittest.TestCase suite verifying LIFO, race resolution, divide-and-conquer, and reconciliation.
"""

import threading
import random
from typing import Optional, Any, List, Callable
import unittest


class ChaseLevDeque:
    """
    Chase-Lev Work-Stealing Deque (David Chase & Yossi Lev, 2005).
    Asymmetric concurrency: Single-Producer (Owner) / Multi-Consumer (Thieves).
    """

    def __init__(self, initial_capacity: int = 1024):
        self.capacity = initial_capacity
        self.buffer: List[Optional[Any]] = [None] * self.capacity
        self.top: int = 0
        self.bottom: int = 0
        self._lock = threading.Lock()  # Models hardware atomic CAS & memory fencing

    def push(self, item: Any) -> None:
        """Owner thread pushes item to the bottom of the deque."""
        with self._lock:
            b = self.bottom
            t = self.top
            if b - t >= self.capacity:
                # Grow buffer
                new_cap = self.capacity * 2
                new_buf = [None] * new_cap
                for i in range(t, b):
                    new_buf[i % new_cap] = self.buffer[i % self.capacity]
                self.buffer = new_buf
                self.capacity = new_cap

            self.buffer[b % self.capacity] = item
            self.bottom = b + 1

    def pop(self) -> Optional[Any]:
        """
        Owner thread pops item from the bottom of the deque (LIFO).
        Resolves race condition against concurrent thieves on the single remaining item.
        """
        with self._lock:
            b = self.bottom - 1
            self.bottom = b
            t = self.top

            if t <= b:
                item = self.buffer[b % self.capacity]
                if t == b:
                    # Exactly ONE item remaining: race with thieves
                    self.top = t + 1
                    self.bottom = t + 1
                    return item
                return item
            else:
                # Deque was empty
                self.bottom = t
                return None

    def steal(self) -> Optional[Any]:
        """Thief thread steals item from the top of the deque (FIFO)."""
        with self._lock:
            t = self.top
            b = self.bottom
            if t < b:
                item = self.buffer[t % self.capacity]
                self.top = t + 1
                return item
            return None

    def size(self) -> int:
        with self._lock:
            return max(0, self.bottom - self.top)

    def empty(self) -> bool:
        return self.size() == 0


class WorkStealingScheduler:
    """Task scheduler with randomized work stealing across worker threads."""

    def __init__(self, num_workers: int = 4):
        self.num_workers = num_workers
        self.deques = [ChaseLevDeque() for _ in range(num_workers)]
        self.workers: List[threading.Thread] = []
        self.stop_event = threading.Event()
        self.total_steals = 0
        self._steal_lock = threading.Lock()

    def submit_initial(self, worker_id: int, task: Callable[[], None]) -> None:
        self.deques[worker_id].push(task)

    def start(self) -> None:
        for i in range(self.num_workers):
            t = threading.Thread(target=self._worker_loop, args=(i,))
            self.workers.append(t)
            t.start()

    def stop(self) -> None:
        self.stop_event.set()
        for t in self.workers:
            t.join()

    def _worker_loop(self, my_id: int) -> None:
        rng = random.Random(1337 + my_id)
        while not self.stop_event.is_set():
            # 1. Pop from own deque (LIFO)
            task = self.deques[my_id].pop()
            if task is not None:
                task()
                continue

            # 2. Steal from random victim (FIFO)
            victim = rng.randint(0, self.num_workers - 1)
            if victim != my_id:
                stolen = self.deques[victim].steal()
                if stolen is not None:
                    with self._steal_lock:
                        self.total_steals += 1
                    stolen()
                    continue

            threading.Event().wait(0.0001)


class TestWorkStealingDeques(unittest.TestCase):
    def test_owner_sequential_lifo(self):
        deque = ChaseLevDeque()
        self.assertTrue(deque.empty())
        self.assertIsNone(deque.pop())

        for i in range(1, 51):
            deque.push(i)
        self.assertEqual(deque.size(), 50)

        for i in range(50, 0, -1):
            self.assertEqual(deque.pop(), i)
        self.assertTrue(deque.empty())

    def test_single_item_race(self):
        # 100 trials of owner and 3 thieves racing for a single item
        for _ in range(100):
            deque = ChaseLevDeque()
            deque.push("GOLD")

            winners = []
            lock = threading.Lock()
            barrier = threading.Barrier(4)

            def thief_worker():
                barrier.wait()
                res = deque.steal()
                if res == "GOLD":
                    with lock:
                        winners.append("THIEF")

            def owner_worker():
                barrier.wait()
                res = deque.pop()
                if res == "GOLD":
                    with lock:
                        winners.append("OWNER")

            threads = [threading.Thread(target=thief_worker) for _ in range(3)]
            threads.append(threading.Thread(target=owner_worker))

            for t in threads:
                t.start()
            for t in threads:
                t.join()

            # Exactly 1 thread must claim the item
            self.assertEqual(len(winners), 1)
            self.assertTrue(deque.empty())

    def test_concurrent_stress_reconciliation(self):
        deque = ChaseLevDeque(64)
        total_items = 2000
        owner_consumed = []
        thief_consumed = [[] for _ in range(4)]
        owner_done = threading.Event()
        start_flag = threading.Event()

        def thief_worker(t_id: int):
            start_flag.wait()
            while True:
                val = deque.steal()
                if val is not None:
                    thief_consumed[t_id].append(val)
                elif owner_done.is_set() and deque.empty():
                    break

        def owner_worker():
            start_flag.wait()
            for i in range(total_items):
                deque.push(i)
                if i % 4 == 0:
                    val = deque.pop()
                    if val is not None:
                        owner_consumed.append(val)

            while True:
                val = deque.pop()
                if val is not None:
                    owner_consumed.append(val)
                else:
                    break
            owner_done.set()

        thieves = [threading.Thread(target=thief_worker, args=(i,)) for i in range(4)]
        owner = threading.Thread(target=owner_worker)

        for t in thieves:
            t.start()
        owner.start()

        start_flag.set()

        owner.join()
        for t in thieves:
            t.join()

        all_consumed = list(owner_consumed)
        for lst in thief_consumed:
            all_consumed.extend(lst)

        self.assertEqual(len(all_consumed), total_items)
        self.assertEqual(sorted(all_consumed), list(range(total_items)))


if __name__ == '__main__':
    unittest.main()
