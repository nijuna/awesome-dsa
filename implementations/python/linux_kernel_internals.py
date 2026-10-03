"""
Linux Kernel Internals Reference Implementation
================================================
Demonstrates four core architectural pillars of the Linux Kernel:
1. Intrusive Circular Doubly-Linked Lists (list_head & container_of)
2. CFS Scheduler Augmented Red-Black Tree (rb_node & rb_root_cached with O(1) pick-next)
3. Read-Copy-Update (RCU) with atomic publication and quiescent-state synchronization
4. VFS Directory Cache (dcache) with hash indexing and LRU eviction chains
"""

from dataclasses import dataclass
from typing import Optional, Any, Dict, List, Tuple
import threading
import time
import unittest


# ============================================================================
# 1. INTRUSIVE CIRCULAR DOUBLY-LINKED LIST (struct list_head & container_of)
# ============================================================================

class ListHead:
    """Intrusive list anchor mimicking Linux kernel's struct list_head."""
    def __init__(self, owner: Any = None):
        self.next: ListHead = self
        self.prev: ListHead = self
        self.owner: Any = owner  # Emulates container_of(ptr, type, member)

    def is_empty(self) -> bool:
        return self.next is self


def init_list_head(head: ListHead) -> None:
    head.next = head
    head.prev = head


def list_add(new_node: ListHead, head: ListHead) -> None:
    """Insert after head (LIFO stack-like)."""
    next_node = head.next
    next_node.prev = new_node
    new_node.next = next_node
    new_node.prev = head
    head.next = new_node


def list_add_tail(new_node: ListHead, head: ListHead) -> None:
    """Insert before head (FIFO queue-like)."""
    prev_node = head.prev
    prev_node.next = new_node
    new_node.prev = prev_node
    new_node.next = head
    head.prev = new_node


def list_del(entry: ListHead) -> None:
    """Unlink entry from list."""
    entry.prev.next = entry.next
    entry.next.prev = entry.prev
    entry.next = None  # Poison
    entry.prev = None


def list_del_init(entry: ListHead) -> None:
    """Unlink entry and re-initialize as empty circular list."""
    entry.prev.next = entry.next
    entry.next.prev = entry.prev
    entry.next = entry
    entry.prev = entry


class KernelTask:
    """Simulates a task_struct embedded in multiple kernel lists simultaneously."""
    def __init__(self, pid: int, name: str, vruntime: int = 0):
        self.pid = pid
        self.name = name
        self.vruntime = vruntime
        self.run_list = ListHead(owner=self)     # Active runqueue
        self.all_tasks = ListHead(owner=self)    # Global process table


# ============================================================================
# 2. CFS SCHEDULER AUGMENTED RB-TREE (rb_root_cached)
# ============================================================================

RED = 0
BLACK = 1


class RbNode:
    def __init__(self, key: int, payload: Any):
        self.key = key
        self.payload = payload
        self.color = RED
        self.left: Optional[RbNode] = None
        self.right: Optional[RbNode] = None
        self.parent: Optional[RbNode] = None


class RbTreeCached:
    """
    Red-Black tree maintaining an O(1) rb_leftmost pointer,
    exactly matching Linux CFS (kernel/sched/fair.c).
    """
    def __init__(self):
        self.root: Optional[RbNode] = None
        self.rb_leftmost: Optional[RbNode] = None
        self.count: int = 0

    def empty(self) -> bool:
        return self.root is None

    def pick_next_task(self) -> Optional[Any]:
        """O(1) retrieval of the task with smallest vruntime."""
        if self.rb_leftmost is None:
            return None
        return self.rb_leftmost.payload

    def _rotate_left(self, n: RbNode) -> None:
        r = n.right
        assert r is not None
        n.right = r.left
        if r.left:
            r.left.parent = n
        r.parent = n.parent
        if n.parent is None:
            self.root = r
        elif n == n.parent.left:
            n.parent.left = r
        else:
            n.parent.right = r
        r.left = n
        n.parent = r

    def _rotate_right(self, n: RbNode) -> None:
        l = n.left
        assert l is not None
        n.left = l.right
        if l.right:
            l.right.parent = n
        l.parent = n.parent
        if n.parent is None:
            self.root = l
        elif n == n.parent.right:
            n.parent.right = l
        else:
            n.parent.left = l
        l.right = n
        n.parent = l

    def _insert_fixup(self, z: RbNode) -> None:
        while z.parent and z.parent.color == RED:
            p = z.parent
            g = p.parent
            assert g is not None
            if p == g.left:
                y = g.right
                if y and y.color == RED:
                    p.color = BLACK
                    y.color = BLACK
                    g.color = RED
                    z = g
                else:
                    if z == p.right:
                        z = p
                        self._rotate_left(z)
                        p = z.parent
                        g = p.parent
                    p.color = BLACK
                    g.color = RED
                    self._rotate_right(g)
            else:
                y = g.left
                if y and y.color == RED:
                    p.color = BLACK
                    y.color = BLACK
                    g.color = RED
                    z = g
                else:
                    if z == p.left:
                        z = p
                        self._rotate_right(z)
                        p = z.parent
                        g = p.parent
                    p.color = BLACK
                    g.color = RED
                    self._rotate_left(g)
        if self.root:
            self.root.color = BLACK

    def insert(self, key: int, payload: Any) -> RbNode:
        z = RbNode(key, payload)
        y = None
        x = self.root
        leftmost = True

        while x is not None:
            y = x
            if key < x.key:
                x = x.left
            else:
                x = x.right
                leftmost = False

        z.parent = y
        if y is None:
            self.root = z
        elif key < y.key:
            y.left = z
        else:
            y.right = z

        if leftmost:
            self.rb_leftmost = z

        self._insert_fixup(z)
        self.count += 1
        return z

    def _tree_min(self, n: RbNode) -> RbNode:
        while n.left:
            n = n.left
        return n

    def _transplant(self, u: RbNode, v: Optional[RbNode]) -> None:
        if u.parent is None:
            self.root = v
        elif u == u.parent.left:
            u.parent.left = v
        else:
            u.parent.right = v
        if v:
            v.parent = u.parent

    def erase(self, z: RbNode) -> None:
        if self.root is None or z is None:
            return

        # Update leftmost pointer if deleting the leftmost node
        if z == self.rb_leftmost:
            if z.right:
                self.rb_leftmost = self._tree_min(z.right)
            else:
                self.rb_leftmost = z.parent

        y = z
        y_orig_color = y.color
        x_parent: Optional[RbNode] = None

        if z.left is None:
            x = z.right
            x_parent = z.parent
            self._transplant(z, z.right)
        elif z.right is None:
            x = z.left
            x_parent = z.parent
            self._transplant(z, z.left)
        else:
            y = self._tree_min(z.right)
            y_orig_color = y.color
            x = y.right
            if y.parent == z:
                x_parent = y
            else:
                x_parent = y.parent
                self._transplant(y, y.right)
                y.right = z.right
                y.right.parent = y
            self._transplant(z, y)
            y.left = z.left
            y.left.parent = y
            y.color = z.color

        if y_orig_color == BLACK:
            self._erase_fixup(x, x_parent)

        self.count -= 1
        if self.count == 0:
            self.root = None
            self.rb_leftmost = None

    def _erase_fixup(self, x: Optional[RbNode], x_parent: Optional[RbNode]) -> None:
        while x != self.root and (x is None or x.color == BLACK):
            if x == (x_parent.left if x_parent else None):
                w = x_parent.right if x_parent else None
                if w and w.color == RED:
                    w.color = BLACK
                    x_parent.color = RED
                    self._rotate_left(x_parent)
                    w = x_parent.right if x_parent else None
                if w is None:
                    x = x_parent
                    x_parent = x.parent if x else None
                    continue
                if (w.left is None or w.left.color == BLACK) and (w.right is None or w.right.color == BLACK):
                    w.color = RED
                    x = x_parent
                    x_parent = x.parent if x else None
                else:
                    if w.right is None or w.right.color == BLACK:
                        if w.left:
                            w.left.color = BLACK
                        w.color = RED
                        self._rotate_right(w)
                        w = x_parent.right if x_parent else None
                    if w and x_parent:
                        w.color = x_parent.color
                        x_parent.color = BLACK
                        if w.right:
                            w.right.color = BLACK
                        self._rotate_left(x_parent)
                    x = self.root
                    break
            else:
                w = x_parent.left if x_parent else None
                if w and w.color == RED:
                    w.color = BLACK
                    x_parent.color = RED
                    self._rotate_right(x_parent)
                    w = x_parent.left if x_parent else None
                if w is None:
                    x = x_parent
                    x_parent = x.parent if x else None
                    continue
                if (w.right is None or w.right.color == BLACK) and (w.left is None or w.left.color == BLACK):
                    w.color = RED
                    x = x_parent
                    x_parent = x.parent if x else None
                else:
                    if w.left is None or w.left.color == BLACK:
                        if w.right:
                            w.right.color = BLACK
                        w.color = RED
                        self._rotate_left(w)
                        w = x_parent.left if x_parent else None
                    if w and x_parent:
                        w.color = x_parent.color
                        x_parent.color = BLACK
                        if w.left:
                            w.left.color = BLACK
                        self._rotate_right(x_parent)
                    x = self.root
                    break
        if x:
            x.color = BLACK


# ============================================================================
# 3. READ-COPY-UPDATE (RCU) ENGINE
# ============================================================================

@dataclass
class RouteEntry:
    ip: str
    gateway: str
    metric: int


class RcuEngine:
    """Models Linux RCU: lockless reader scaling with quiescent state synchronization."""
    def __init__(self, initial_route: RouteEntry):
        self._route: RouteEntry = initial_route
        self._global_epoch: int = 0
        self._reader_epochs: Dict[int, int] = {}
        self._lock = threading.Lock()

    def rcu_read_lock(self, reader_id: int) -> None:
        with self._lock:
            # Active in current epoch
            self._reader_epochs[reader_id] = self._global_epoch | 1

    def rcu_dereference(self) -> RouteEntry:
        return self._route

    def rcu_read_unlock(self, reader_id: int) -> None:
        with self._lock:
            self._reader_epochs[reader_id] = 0

    def update_route(self, ip: str, gateway: str, metric: int) -> None:
        # Read-Copy: Create new object
        new_entry = RouteEntry(ip, gateway, metric)

        with self._lock:
            # Atomic pointer publication (rcu_assign_pointer)
            self._route = new_entry
            target_epoch = self._global_epoch + 2
            self._global_epoch = target_epoch

        # synchronize_rcu(): Wait until all readers that started before this update quiesce
        self._synchronize_rcu(target_epoch)

    def _synchronize_rcu(self, target_epoch: int) -> None:
        while True:
            with self._lock:
                active = [
                    epoch for epoch in self._reader_epochs.values()
                    if epoch != 0 and epoch < target_epoch
                ]
                if not active:
                    break
            time.sleep(0.001)


# ============================================================================
# 4. VFS DIRECTORY CACHE (dcache)
# ============================================================================

class DEntry:
    def __init__(self, parent_ino: int, name: str, ino: int):
        self.parent_ino = parent_ino
        self.name = name
        self.ino = ino
        self.lru_node = ListHead(owner=self)
        self.hash_node = ListHead(owner=self)


class DCache:
    """Linux VFS Directory Cache with hash buckets and LRU shrinker."""
    def __init__(self, capacity: int, bucket_count: int = 16):
        self.capacity = capacity
        self.bucket_count = bucket_count
        self.buckets: List[ListHead] = [ListHead() for _ in range(bucket_count)]
        self.lru_head = ListHead()
        self.active_count = 0

    def _hash(self, parent_ino: int, name: str) -> int:
        return (hash((parent_ino, name))) % self.bucket_count

    def lookup(self, parent_ino: int, name: str) -> Optional[DEntry]:
        b = self._hash(parent_ino, name)
        curr = self.buckets[b].next
        while curr is not self.buckets[b]:
            entry: DEntry = curr.owner
            if entry.parent_ino == parent_ino and entry.name == name:
                # Refresh LRU position (move to tail)
                list_del_init(entry.lru_node)
                list_add_tail(entry.lru_node, self.lru_head)
                return entry
            curr = curr.next
        return None

    def add(self, parent_ino: int, name: str, ino: int) -> DEntry:
        existing = self.lookup(parent_ino, name)
        if existing:
            return existing

        if self.active_count >= self.capacity:
            self._evict_lru()

        dentry = DEntry(parent_ino, name, ino)
        b = self._hash(parent_ino, name)
        list_add_tail(dentry.hash_node, self.buckets[b])
        list_add_tail(dentry.lru_node, self.lru_head)
        self.active_count += 1
        return dentry

    def _evict_lru(self) -> None:
        if self.lru_head.is_empty():
            return
        oldest_head = self.lru_head.next
        dentry: DEntry = oldest_head.owner
        list_del(dentry.lru_node)
        list_del(dentry.hash_node)
        self.active_count -= 1


# ============================================================================
# UNIT TESTS & PROPERTY VERIFICATION
# ============================================================================

class TestLinuxKernelInternals(unittest.TestCase):
    def test_intrusive_lists_and_container_of(self):
        """Verify multiple list head embedment without multiple wrapper nodes."""
        runqueue = ListHead()
        all_procs = ListHead()

        tasks = [
            KernelTask(1, "systemd", 10),
            KernelTask(2, "kthreadd", 5),
            KernelTask(100, "sshd", 20),
        ]

        for t in tasks:
            list_add_tail(t.run_list, runqueue)
            list_add_tail(t.all_tasks, all_procs)

        # Iterate runqueue
        pids = []
        curr = runqueue.next
        while curr is not runqueue:
            t = curr.owner
            pids.append(t.pid)
            curr = curr.next
        self.assertEqual(pids, [1, 2, 100])

        # Delete t[1] from runqueue only
        list_del(tasks[1].run_list)

        remaining_sched = []
        curr = runqueue.next
        while curr is not runqueue:
            remaining_sched.append(curr.owner.pid)
            curr = curr.next
        self.assertEqual(remaining_sched, [1, 100])

        # all_procs still has all 3
        all_pids = []
        curr = all_procs.next
        while curr is not all_procs:
            all_pids.append(curr.owner.pid)
            curr = curr.next
        self.assertEqual(all_pids, [1, 2, 100])

    def test_cfs_rb_tree_cached(self):
        """Verify CFS leftmost O(1) tracking and in-order priority eviction."""
        cfs = RbTreeCached()
        tasks = [
            (50, KernelTask(5, "task5", 50)),
            (20, KernelTask(2, "task2", 20)),
            (80, KernelTask(8, "task8", 80)),
            (10, KernelTask(1, "task1", 10)),
            (35, KernelTask(3, "task3", 35)),
        ]

        nodes = []
        for vruntime, t in tasks:
            node = cfs.insert(vruntime, t)
            nodes.append(node)

        # Smallest vruntime should be task1 (vruntime 10)
        self.assertEqual(cfs.pick_next_task().pid, 1)

        popped = []
        while not cfs.empty():
            nxt = cfs.pick_next_task()
            popped.append(nxt.vruntime)
            # Find node and erase
            node_to_del = cfs.rb_leftmost
            cfs.erase(node_to_del)

        self.assertEqual(popped, [10, 20, 35, 50, 80])

    def test_rcu_concurrent_scaling(self):
        """Verify lockless reader updates and quiescent synchronization."""
        rcu = RcuEngine(RouteEntry("192.168.1.1", "10.0.0.1", 1))
        stop_event = threading.Event()
        read_count = [0]

        def reader_loop(rid: int):
            while not stop_event.is_set():
                rcu.rcu_read_lock(rid)
                route = rcu.rcu_dereference()
                self.assertTrue(len(route.ip) > 0)
                self.assertTrue(len(route.gateway) > 0)
                rcu.rcu_read_unlock(rid)
                read_count[0] += 1

        threads = [threading.Thread(target=reader_loop, args=(i,)) for i in range(4)]
        for t in threads:
            t.start()

        for i in range(15):
            rcu.update_route(f"192.168.1.{i+10}", f"10.0.0.{i+1}", i + 10)

        stop_event.set()
        for t in threads:
            t.join()

        self.assertGreater(read_count[0], 50)

    def test_dcache_lru_and_hash(self):
        """Verify VFS dcache lookup, hash chain, and LRU eviction."""
        dcache = DCache(capacity=3)

        dcache.add(1, "home", 100)
        dcache.add(1, "etc", 101)
        dcache.add(1, "var", 102)

        self.assertEqual(dcache.active_count, 3)
        self.assertIsNotNone(dcache.lookup(1, "home"))

        # Access "home" -> updates LRU. Oldest is now "etc"
        dcache.lookup(1, "home")

        # Adding 4th item "usr" should evict "etc"
        dcache.add(1, "usr", 103)
        self.assertEqual(dcache.active_count, 3)

        self.assertIsNone(dcache.lookup(1, "etc"))      # Evicted
        self.assertIsNotNone(dcache.lookup(1, "home"))  # Kept
        self.assertIsNotNone(dcache.lookup(1, "var"))   # Kept
        self.assertIsNotNone(dcache.lookup(1, "usr"))   # New


if __name__ == "__main__":
    unittest.main()
