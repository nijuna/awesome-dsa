#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>
#include <cassert>
#include <atomic>
#include <thread>
#include <chrono>
#include <unordered_map>
#include <functional>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace linux_kernel {

// ============================================================================
// 1. INTRUSIVE CIRCULAR DOUBLY-LINKED LIST (struct list_head)
// ============================================================================

struct list_head {
    list_head* next{nullptr};
    list_head* prev{nullptr};
};

// Canonical macro and constexpr helper for container_of
#define container_of(ptr, type, member) \
    (reinterpret_cast<type*>(reinterpret_cast<char*>(const_cast<list_head*>(ptr)) - offsetof(type, member)))

inline void INIT_LIST_HEAD(list_head* list) {
    list->next = list;
    list->prev = list;
}

inline bool list_empty(const list_head* head) {
    return head->next == head;
}

inline void __list_add(list_head* _new, list_head* prev, list_head* next) {
    next->prev = _new;
    _new->next = next;
    _new->prev = prev;
    prev->next = _new;
}

inline void list_add(list_head* _new, list_head* head) {
    __list_add(_new, head, head->next);
}

inline void list_add_tail(list_head* _new, list_head* head) {
    __list_add(_new, head->prev, head);
}

inline void __list_del(list_head* prev, list_head* next) {
    next->prev = prev;
    prev->next = next;
}

inline void list_del(list_head* entry) {
    __list_del(entry->prev, entry->next);
    // Poison pointers in debug kernel style
    entry->next = nullptr;
    entry->prev = nullptr;
}

inline void list_del_init(list_head* entry) {
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

// Example domain struct embedding multiple list_heads simultaneously
struct Task {
    uint32_t pid;
    std::string name;
    uint64_t vruntime; // For CFS scheduler
    list_head run_list; // Member of runnable scheduler queue
    list_head all_tasks; // Member of global process table
};

// ============================================================================
// 2. CFS SCHEDULER AUGMENTED RED-BLACK TREE (rb_node & rb_root_cached)
// ============================================================================

enum class RbColor : uint8_t { RED, BLACK };

struct rb_node {
    rb_node* parent{nullptr};
    rb_node* left{nullptr};
    rb_node* right{nullptr};
    RbColor color{RbColor::RED};
};

struct SchedEntity {
    uint32_t pid;
    uint64_t vruntime;
    std::string comm;
    rb_node rb_sched;
};

#define rb_entry(ptr, type, member) \
    (reinterpret_cast<type*>(reinterpret_cast<char*>(ptr) - offsetof(type, member)))

class RbTreeCached {
private:
    rb_node* root_{nullptr};
    rb_node* rb_leftmost_{nullptr}; // O(1) pick-next-task cache
    size_t size_{0};

    void rotate_left(rb_node* n) {
        rb_node* r = n->right;
        assert(r != nullptr);
        n->right = r->left;
        if (r->left != nullptr) {
            r->left->parent = n;
        }
        r->parent = n->parent;
        if (n->parent == nullptr) {
            root_ = r;
        } else if (n == n->parent->left) {
            n->parent->left = r;
        } else {
            n->parent->right = r;
        }
        r->left = n;
        n->parent = r;
    }

    void rotate_right(rb_node* n) {
        rb_node* l = n->left;
        assert(l != nullptr);
        n->left = l->right;
        if (l->right != nullptr) {
            l->right->parent = n;
        }
        l->parent = n->parent;
        if (n->parent == nullptr) {
            root_ = l;
        } else if (n == n->parent->right) {
            n->parent->right = l;
        } else {
            n->parent->left = l;
        }
        l->right = n;
        n->parent = l;
    }

    void insert_fixup(rb_node* z) {
        while (z->parent != nullptr && z->parent->color == RbColor::RED) {
            rb_node* p = z->parent;
            rb_node* g = p->parent;
            if (p == g->left) {
                rb_node* y = g->right; // Uncle
                if (y != nullptr && y->color == RbColor::RED) {
                    p->color = RbColor::BLACK;
                    y->color = RbColor::BLACK;
                    g->color = RbColor::RED;
                    z = g;
                } else {
                    if (z == p->right) {
                        z = p;
                        rotate_left(z);
                        p = z->parent;
                        g = p->parent;
                    }
                    p->color = RbColor::BLACK;
                    g->color = RbColor::RED;
                    rotate_right(g);
                }
            } else {
                rb_node* y = g->left; // Uncle
                if (y != nullptr && y->color == RbColor::RED) {
                    p->color = RbColor::BLACK;
                    y->color = RbColor::BLACK;
                    g->color = RbColor::RED;
                    z = g;
                } else {
                    if (z == p->left) {
                        z = p;
                        rotate_right(z);
                        p = z->parent;
                        g = p->parent;
                    }
                    p->color = RbColor::BLACK;
                    g->color = RbColor::RED;
                    rotate_left(g);
                }
            }
        }
        root_->color = RbColor::BLACK;
    }

    rb_node* tree_minimum(rb_node* x) const {
        while (x != nullptr && x->left != nullptr) {
            x = x->left;
        }
        return x;
    }

public:
    RbTreeCached() = default;

    size_t size() const { return size_; }
    bool empty() const { return root_ == nullptr; }

    // O(1) pick next task in CFS scheduler
    SchedEntity* pick_next_task() const {
        if (rb_leftmost_ == nullptr) return nullptr;
        return rb_entry(rb_leftmost_, SchedEntity, rb_sched);
    }

    // Insert entity keyed by vruntime
    void insert(SchedEntity* entity) {
        rb_node* z = &entity->rb_sched;
        z->left = nullptr;
        z->right = nullptr;
        z->color = RbColor::RED;

        rb_node* y = nullptr;
        rb_node* x = root_;
        bool leftmost = true;

        while (x != nullptr) {
            y = x;
            SchedEntity* cur = rb_entry(x, SchedEntity, rb_sched);
            if (entity->vruntime < cur->vruntime) {
                x = x->left;
            } else {
                x = x->right;
                leftmost = false; // Went right, so z cannot be the new leftmost
            }
        }

        z->parent = y;
        if (y == nullptr) {
            root_ = z;
        } else {
            SchedEntity* parent_entity = rb_entry(y, SchedEntity, rb_sched);
            if (entity->vruntime < parent_entity->vruntime) {
                y->left = z;
            } else {
                y->right = z;
            }
        }

        if (leftmost) {
            rb_leftmost_ = z;
        }

        insert_fixup(z);
        size_++;
    }

    // Remove entity
    void erase(SchedEntity* entity) {
        rb_node* z = &entity->rb_sched;
        if (z == nullptr || root_ == nullptr) return;

        // If removing the leftmost node, update cached leftmost pointer
        if (z == rb_leftmost_) {
            // Successor of leftmost is either right child's min, or parent if z was left child
            if (z->right != nullptr) {
                rb_leftmost_ = tree_minimum(z->right);
            } else {
                rb_leftmost_ = z->parent;
            }
        }

        // Standard RB deletion
        rb_node* y = z;
        rb_node* x = nullptr;
        rb_node* x_parent = nullptr;
        RbColor y_original_color = y->color;

        if (z->left == nullptr) {
            x = z->right;
            x_parent = z->parent;
            rb_transplant(z, z->right);
        } else if (z->right == nullptr) {
            x = z->left;
            x_parent = z->parent;
            rb_transplant(z, z->left);
        } else {
            y = tree_minimum(z->right);
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z) {
                x_parent = y;
            } else {
                x_parent = y->parent;
                rb_transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            rb_transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }

        if (y_original_color == RbColor::BLACK) {
            erase_fixup(x, x_parent);
        }

        size_--;
        if (size_ == 0) {
            root_ = nullptr;
            rb_leftmost_ = nullptr;
        }
    }

private:
    void rb_transplant(rb_node* u, rb_node* v) {
        if (u->parent == nullptr) {
            root_ = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        if (v != nullptr) {
            v->parent = u->parent;
        }
    }

    void erase_fixup(rb_node* x, rb_node* x_parent) {
        while (x != root_ && (x == nullptr || x->color == RbColor::BLACK)) {
            if (x == (x_parent ? x_parent->left : nullptr)) {
                rb_node* w = x_parent->right;
                if (w != nullptr && w->color == RbColor::RED) {
                    w->color = RbColor::BLACK;
                    x_parent->color = RbColor::RED;
                    rotate_left(x_parent);
                    w = x_parent->right;
                }
                if (w == nullptr) {
                    x = x_parent;
                    x_parent = x ? x->parent : nullptr;
                    continue;
                }
                if ((w->left == nullptr || w->left->color == RbColor::BLACK) &&
                    (w->right == nullptr || w->right->color == RbColor::BLACK)) {
                    w->color = RbColor::RED;
                    x = x_parent;
                    x_parent = x ? x->parent : nullptr;
                } else {
                    if (w->right == nullptr || w->right->color == RbColor::BLACK) {
                        if (w->left != nullptr) w->left->color = RbColor::BLACK;
                        w->color = RbColor::RED;
                        rotate_right(w);
                        w = x_parent->right;
                    }
                    if (w != nullptr) {
                        w->color = x_parent->color;
                        x_parent->color = RbColor::BLACK;
                        if (w->right != nullptr) w->right->color = RbColor::BLACK;
                        rotate_left(x_parent);
                    }
                    x = root_;
                    break;
                }
            } else {
                rb_node* w = x_parent ? x_parent->left : nullptr;
                if (w != nullptr && w->color == RbColor::RED) {
                    w->color = RbColor::BLACK;
                    x_parent->color = RbColor::RED;
                    rotate_right(x_parent);
                    w = x_parent->left;
                }
                if (w == nullptr) {
                    x = x_parent;
                    x_parent = x ? x->parent : nullptr;
                    continue;
                }
                if ((w->right == nullptr || w->right->color == RbColor::BLACK) &&
                    (w->left == nullptr || w->left->color == RbColor::BLACK)) {
                    w->color = RbColor::RED;
                    x = x_parent;
                    x_parent = x ? x->parent : nullptr;
                } else {
                    if (w->left == nullptr || w->left->color == RbColor::BLACK) {
                        if (w->right != nullptr) w->right->color = RbColor::BLACK;
                        w->color = RbColor::RED;
                        rotate_left(w);
                        w = x_parent->left;
                    }
                    if (w != nullptr) {
                        w->color = x_parent->color;
                        x_parent->color = RbColor::BLACK;
                        if (w->left != nullptr) w->left->color = RbColor::BLACK;
                        rotate_right(x_parent);
                    }
                    x = root_;
                    break;
                }
            }
        }
        if (x != nullptr) x->color = RbColor::BLACK;
    }
};

// ============================================================================
// 3. READ-COPY-UPDATE (RCU) MECHANISM
// ============================================================================

struct RoutingEntry {
    std::string ip;
    std::string gateway;
    uint32_t metric;
};

class SimpleRcuEngine {
private:
    std::atomic<RoutingEntry*> published_route_{nullptr};
    std::atomic<uint64_t> global_epoch_{0};
    
    // Per-thread reader active epoch tracking (0 = inactive)
    static constexpr size_t MAX_READERS = 32;
    std::atomic<uint64_t> reader_epochs_[MAX_READERS];

public:
    SimpleRcuEngine(RoutingEntry* initial) {
        published_route_.store(initial, std::memory_order_release);
        for (size_t i = 0; i < MAX_READERS; ++i) {
            reader_epochs_[i].store(0, std::memory_order_relaxed);
        }
    }

    ~SimpleRcuEngine() {
        RoutingEntry* cur = published_route_.load(std::memory_order_relaxed);
        delete cur;
    }

    // Reader side: rcu_read_lock
    void rcu_read_lock(size_t thread_id) {
        assert(thread_id < MAX_READERS);
        uint64_t e = global_epoch_.load(std::memory_order_relaxed);
        // Odd epoch indicates active reader in generation e | 1
        reader_epochs_[thread_id].store(e | 1, std::memory_order_seq_cst);
    }

    // Reader side: rcu_dereference (acquire semantics)
    const RoutingEntry* rcu_dereference() const {
        return published_route_.load(std::memory_order_acquire);
    }

    // Reader side: rcu_read_unlock
    void rcu_read_unlock(size_t thread_id) {
        assert(thread_id < MAX_READERS);
        reader_epochs_[thread_id].store(0, std::memory_order_seq_cst);
    }

    // Writer side: atomic update with synchronize_rcu grace period
    void update_route(const std::string& new_ip, const std::string& new_gw, uint32_t metric) {
        // 1. Read existing and make a copy
        RoutingEntry* new_entry = new RoutingEntry{new_ip, new_gw, metric};

        // 2. Publish new pointer atomically via store-release (rcu_assign_pointer)
        RoutingEntry* old_entry = published_route_.exchange(new_entry, std::memory_order_acq_rel);

        // 3. synchronize_rcu(): Advance epoch and wait for all active readers to quiesce
        synchronize_rcu();

        // 4. Safely reclaim old entry
        delete old_entry;
    }

    void synchronize_rcu() {
        // Advance global epoch
        uint64_t target_epoch = global_epoch_.fetch_add(2, std::memory_order_seq_cst) + 2;

        // Quiescent state check: Wait until every thread has observed this or is inactive
        for (size_t i = 0; i < MAX_READERS; ++i) {
            while (true) {
                uint64_t re = reader_epochs_[i].load(std::memory_order_seq_cst);
                // Inactive (0) or started after our target epoch
                if (re == 0 || re > target_epoch) {
                    break;
                }
                std::this_thread::yield();
            }
        }
    }
};

// ============================================================================
// 4. VFS DIRECTORY CACHE (dcache) & HASH TABLE WITH LRU
// ============================================================================

struct dentry {
    std::string name;
    uint64_t parent_ino;
    uint64_t ino;
    list_head d_lru;   // LRU chain for unused dentries
    list_head d_hash;  // Hash chain in dcache bucket
};

class Dcache {
private:
    static constexpr size_t BUCKET_COUNT = 64;
    list_head hash_buckets_[BUCKET_COUNT];
    list_head lru_head_;
    size_t capacity_;
    size_t active_count_{0};

    size_t hash_func(uint64_t parent_ino, const std::string& name) const {
        size_t h = std::hash<uint64_t>{}(parent_ino);
        size_t h2 = std::hash<std::string>{}(name);
        return (h ^ (h2 << 1)) % BUCKET_COUNT;
    }

public:
    explicit Dcache(size_t capacity) : capacity_(capacity) {
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            INIT_LIST_HEAD(&hash_buckets_[i]);
        }
        INIT_LIST_HEAD(&lru_head_);
    }

    ~Dcache() {
        // Clean up remaining dentries
        for (size_t i = 0; i < BUCKET_COUNT; ++i) {
            list_head* curr = hash_buckets_[i].next;
            while (curr != &hash_buckets_[i]) {
                list_head* next = curr->next;
                dentry* d = container_of(curr, dentry, d_hash);
                delete d;
                curr = next;
            }
        }
    }

    dentry* lookup(uint64_t parent_ino, const std::string& name) {
        size_t b = hash_func(parent_ino, name);
        list_head* head = &hash_buckets_[b];
        list_head* curr = head->next;
        while (curr != head) {
            dentry* d = container_of(curr, dentry, d_hash);
            if (d->parent_ino == parent_ino && d->name == name) {
                // Move to tail of LRU (most recently used)
                list_del_init(&d->d_lru);
                list_add_tail(&d->d_lru, &lru_head_);
                return d;
            }
            curr = curr->next;
        }
        return nullptr;
    }

    void add(uint64_t parent_ino, const std::string& name, uint64_t ino) {
        if (lookup(parent_ino, name) != nullptr) return;

        // Shrink if capacity exceeded
        if (active_count_ >= capacity_) {
            evict_lru();
        }

        dentry* d = new dentry;
        d->name = name;
        d->parent_ino = parent_ino;
        d->ino = ino;
        INIT_LIST_HEAD(&d->d_lru);
        INIT_LIST_HEAD(&d->d_hash);

        // Add to hash bucket
        size_t b = hash_func(parent_ino, name);
        list_add_tail(&d->d_hash, &hash_buckets_[b]);

        // Add to LRU tail
        list_add_tail(&d->d_lru, &lru_head_);
        active_count_++;
    }

    void evict_lru() {
        if (list_empty(&lru_head_)) return;
        // Evict LRU head (least recently used)
        list_head* oldest = lru_head_.next;
        dentry* d = container_of(oldest, dentry, d_lru);
        list_del(&d->d_lru);
        list_del(&d->d_hash);
        delete d;
        active_count_--;
    }

    size_t size() const { return active_count_; }
};

} // namespace linux_kernel

// ============================================================================
// TEST SUITE & VERIFICATION
// ============================================================================

int main() {
    using namespace linux_kernel;

    std::cout << "===============================================================\n";
    std::cout << "Linux Kernel Internals: Intrusive Structures & RCU Verification\n";
    std::cout << "===============================================================\n";

    // ------------------------------------------------------------------------
    // Test 1: Intrusive Multi-List Membership & container_of
    // ------------------------------------------------------------------------
    std::cout << "[Test 1/4] Testing Intrusive Doubly-Linked Lists (list_head)... \n";
    {
        list_head scheduler_queue;
        list_head all_tasks_table;
        INIT_LIST_HEAD(&scheduler_queue);
        INIT_LIST_HEAD(&all_tasks_table);

        Task t1{1, "init", 100, {}, {}};
        Task t2{2, "kthreadd", 150, {}, {}};
        Task t3{105, "systemd-udevd", 120, {}, {}};

        INIT_LIST_HEAD(&t1.run_list);
        INIT_LIST_HEAD(&t1.all_tasks);
        INIT_LIST_HEAD(&t2.run_list);
        INIT_LIST_HEAD(&t2.all_tasks);
        INIT_LIST_HEAD(&t3.run_list);
        INIT_LIST_HEAD(&t3.all_tasks);

        // Add tasks to both lists simultaneously without any extra wrapper allocation!
        list_add_tail(&t1.run_list, &scheduler_queue);
        list_add_tail(&t2.run_list, &scheduler_queue);
        list_add_tail(&t3.run_list, &scheduler_queue);

        list_add_tail(&t1.all_tasks, &all_tasks_table);
        list_add_tail(&t2.all_tasks, &all_tasks_table);
        list_add_tail(&t3.all_tasks, &all_tasks_table);

        // Traverse scheduler queue using container_of
        std::vector<uint32_t> sched_pids;
        list_head* curr = scheduler_queue.next;
        while (curr != &scheduler_queue) {
            Task* t = container_of(curr, Task, run_list);
            sched_pids.push_back(t->pid);
            curr = curr->next;
        }
        assert((sched_pids == std::vector<uint32_t>{1, 2, 105}));

        // Remove t2 from scheduler queue, but keep in all_tasks
        list_del(&t2.run_list);
        assert(!list_empty(&scheduler_queue));

        sched_pids.clear();
        curr = scheduler_queue.next;
        while (curr != &scheduler_queue) {
            Task* t = container_of(curr, Task, run_list);
            sched_pids.push_back(t->pid);
            curr = curr->next;
        }
        assert((sched_pids == std::vector<uint32_t>{1, 105}));

        // Verify all_tasks still has all 3
        std::vector<uint32_t> all_pids;
        curr = all_tasks_table.next;
        while (curr != &all_tasks_table) {
            Task* t = container_of(curr, Task, all_tasks);
            all_pids.push_back(t->pid);
            curr = curr->next;
        }
        assert((all_pids == std::vector<uint32_t>{1, 2, 105}));
        std::cout << "  -> Multi-list intrusive membership & container_of verified.\n";
    }

    // ------------------------------------------------------------------------
    // Test 2: CFS Scheduler Augmented RB-Tree (rb_leftmost O(1) pick-next)
    // ------------------------------------------------------------------------
    std::cout << "[Test 2/4] Testing CFS Augmented RB-Tree with cached leftmost... \n";
    {
        RbTreeCached cfs_tree;
        SchedEntity s1{101, 500, "worker-1", {}};
        SchedEntity s2{102, 200, "worker-2", {}};
        SchedEntity s3{103, 800, "worker-3", {}};
        SchedEntity s4{104, 150, "worker-4", {}};
        SchedEntity s5{105, 350, "worker-5", {}};

        cfs_tree.insert(&s1);
        assert(cfs_tree.pick_next_task()->pid == 101);

        cfs_tree.insert(&s2);
        assert(cfs_tree.pick_next_task()->pid == 102);

        cfs_tree.insert(&s3);
        assert(cfs_tree.pick_next_task()->pid == 102);

        cfs_tree.insert(&s4);
        assert(cfs_tree.pick_next_task()->pid == 104); // 150 is lowest

        cfs_tree.insert(&s5);
        assert(cfs_tree.pick_next_task()->pid == 104);

        // Pop tasks in vruntime order
        std::vector<uint64_t> vruntimes;
        while (!cfs_tree.empty()) {
            SchedEntity* next = cfs_tree.pick_next_task();
            assert(next != nullptr);
            vruntimes.push_back(next->vruntime);
            cfs_tree.erase(next);
        }
        assert((vruntimes == std::vector<uint64_t>{150, 200, 350, 500, 800}));
        std::cout << "  -> CFS O(1) leftmost retrieval and in-order scheduling verified.\n";
    }

    // ------------------------------------------------------------------------
    // Test 3: RCU Lockless Reader & Grace Period Reclamation
    // ------------------------------------------------------------------------
    std::cout << "[Test 3/4] Testing Read-Copy-Update (RCU) concurrent readers & updates... \n";
    {
        RoutingEntry* init = new RoutingEntry{"192.168.1.1", "10.0.0.1", 10};
        SimpleRcuEngine rcu(init);

        std::atomic<bool> stop_flag{false};
        std::atomic<uint64_t> read_ops{0};

        // Spawn 4 concurrent lockless readers
        std::vector<std::thread> readers;
        for (size_t t = 0; t < 4; ++t) {
            readers.emplace_back([&rcu, &stop_flag, &read_ops, t]() {
                while (!stop_flag.load(std::memory_order_relaxed)) {
                    rcu.rcu_read_lock(t);
                    const RoutingEntry* r = rcu.rcu_dereference();
                    if (r != nullptr) {
                        assert(!r->ip.empty());
                        assert(!r->gateway.empty());
                    }
                    rcu.rcu_read_unlock(t);
                    read_ops.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        // Writer updates routes through RCU
        for (int i = 0; i < 50; ++i) {
            rcu.update_route("192.168.1." + std::to_string(i + 10),
                             "10.0.0." + std::to_string(i + 1),
                             static_cast<uint32_t>(i + 5));
        }

        stop_flag.store(true, std::memory_order_relaxed);
        for (auto& th : readers) {
            th.join();
        }

        std::cout << "  -> RCU readers executed " << read_ops.load()
                  << " lockless lookups during 50 writer updates without crash.\n";
    }

    // ------------------------------------------------------------------------
    // Test 4: VFS Dcache & Inode LRU Eviction
    // ------------------------------------------------------------------------
    std::cout << "[Test 4/4] Testing VFS Dcache Hash Table & LRU Eviction... \n";
    {
        Dcache dcache(3); // Capacity = 3

        dcache.add(1, "etc", 1001);
        dcache.add(1, "usr", 1002);
        dcache.add(1, "bin", 1003);
        assert(dcache.size() == 3);

        // Access "etc" to refresh LRU
        assert(dcache.lookup(1, "etc") != nullptr);

        // Add 4th item "var", should evict "usr" (the oldest unaccessed)
        dcache.add(1, "var", 1004);
        assert(dcache.size() == 3);
        assert(dcache.lookup(1, "usr") == nullptr); // Evicted!
        assert(dcache.lookup(1, "etc") != nullptr); // Kept!
        assert(dcache.lookup(1, "bin") != nullptr); // Kept!
        assert(dcache.lookup(1, "var") != nullptr); // New!

        std::cout << "  -> VFS dcache lookup and LRU eviction verified successfully.\n";
    }

    std::cout << "===============================================================\n";
    std::cout << "All Linux Kernel Internals tests PASSED flawlessly!\n";
    std::cout << "===============================================================\n";

    return 0;
}
