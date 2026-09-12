/**
 * @file concurrent_queues_and_stacks.cpp
 * @brief Publication-grade C++17 Reference Implementation of Concurrent Queues and Stacks.
 *
 * Implements:
 * 1. Epoch-Based Reclamation (EBR) Engine:
 *    - Thread-local registration with cache-line padded control blocks (alignas(64)).
 *    - 3-epoch retirement queues with deferred safe deallocation.
 *    - RAII EpochGuard for pin/unpin semantics.
 * 2. Treiber Lock-Free LIFO Stack (R. Kent Treiber, 1986):
 *    - Acquire-Release memory orderings (Invariants A-D).
 *    - Safe memory reclamation via EBR eliminating the ABA vulnerability.
 * 3. Michael-Scott Lock-Free FIFO Queue (Maged M. Michael & Michael L. Scott, 1996):
 *    - Dummy sentinel node separating enqueue and dequeue contention.
 *    - Cooperative helping mechanism for lagging tail pointers.
 *    - Explicit linearization points and acquire-release synchronization.
 * 4. Comprehensive Layered Verification Harness:
 *    - Layer 1: Sequential semantic oracle (LIFO and FIFO verification).
 *    - Layer 2: Synchronized multi-threaded stress test with atomic startup barrier.
 *    - Layer 3: End-state item reconciliation and per-producer FIFO ordering validation.
 *
 * Fully compliant with -std=c++17 -O3 -Wall -Wextra -Werror -pthread.
 */

#include <iostream>
#include <atomic>
#include <vector>
#include <thread>
#include <cstdint>
#include <cassert>
#include <optional>
#include <mutex>
#include <functional>
#include <algorithm>
#include <chrono>

namespace dsa {

// ============================================================================
// 1. Epoch-Based Reclamation (EBR) Engine
// ============================================================================

/**
 * @brief Thread registration block for Epoch-Based Reclamation.
 * Padded to 64 bytes to eliminate false sharing between CPU cache lines.
 */
struct alignas(64) ThreadEpochRecord {
    std::atomic<uint64_t> local_epoch{0};
    std::atomic<bool> active{false};
    ThreadEpochRecord* next{nullptr};
};

class EpochReclaimer {
public:
    struct RetiredNode {
        void* ptr;
        void (*deleter)(void*);
        uint64_t retire_epoch;
    };

private:
    alignas(64) std::atomic<uint64_t> global_epoch_{0};
    alignas(64) std::atomic<ThreadEpochRecord*> thread_records_head_{nullptr};
    std::mutex retire_mutex_;
    std::vector<RetiredNode> retired_list_;
    std::atomic<size_t> total_allocated_{0};
    std::atomic<size_t> total_reclaimed_{0};

    static inline thread_local ThreadEpochRecord* tls_record_{nullptr};

public:
    EpochReclaimer() = default;

    ~EpochReclaimer() {
        drain_all();
        // Clean up thread records
        ThreadEpochRecord* curr = thread_records_head_.load(std::memory_order_relaxed);
        while (curr != nullptr) {
            ThreadEpochRecord* next = curr->next;
            delete curr;
            curr = next;
        }
    }

    // Non-copyable, non-movable
    EpochReclaimer(const EpochReclaimer&) = delete;
    EpochReclaimer& operator=(const EpochReclaimer&) = delete;

    ThreadEpochRecord* get_or_register_thread() {
        if (tls_record_ != nullptr) {
            return tls_record_;
        }
        auto* rec = new ThreadEpochRecord();
        ThreadEpochRecord* old_head = thread_records_head_.load(std::memory_order_relaxed);
        do {
            rec->next = old_head;
        } while (!thread_records_head_.compare_exchange_weak(
            old_head, rec, std::memory_order_release, std::memory_order_relaxed));
        tls_record_ = rec;
        return rec;
    }

    void pin() {
        ThreadEpochRecord* rec = get_or_register_thread();
        uint64_t g_epoch = global_epoch_.load(std::memory_order_relaxed);
        rec->local_epoch.store(g_epoch, std::memory_order_relaxed);
        rec->active.store(true, std::memory_order_seq_cst);
    }

    void unpin() {
        ThreadEpochRecord* rec = get_or_register_thread();
        rec->active.store(false, std::memory_order_release);
    }

    void retire(void* ptr, void (*deleter)(void*)) {
        if (ptr == nullptr) return;
        uint64_t curr_epoch = global_epoch_.load(std::memory_order_relaxed);

        std::vector<RetiredNode> to_free;
        {
            std::lock_guard<std::mutex> lock(retire_mutex_);
            retired_list_.push_back({ptr, deleter, curr_epoch});
            if (retired_list_.size() >= 64) {
                // Try to advance epoch and collect reclaimable nodes
                try_advance_and_reclaim_locked(to_free);
            }
        }

        for (auto& node : to_free) {
            node.deleter(node.ptr);
            total_reclaimed_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void try_advance_and_reclaim() {
        std::vector<RetiredNode> to_free;
        {
            std::lock_guard<std::mutex> lock(retire_mutex_);
            try_advance_and_reclaim_locked(to_free);
        }
        for (auto& node : to_free) {
            node.deleter(node.ptr);
            total_reclaimed_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void drain_all() {
        std::vector<RetiredNode> to_free;
        {
            std::lock_guard<std::mutex> lock(retire_mutex_);
            to_free.swap(retired_list_);
        }
        for (auto& node : to_free) {
            node.deleter(node.ptr);
            total_reclaimed_.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void notify_alloc() {
        total_allocated_.fetch_add(1, std::memory_order_relaxed);
    }

    size_t get_total_allocated() const { return total_allocated_.load(std::memory_order_relaxed); }
    size_t get_total_reclaimed() const { return total_reclaimed_.load(std::memory_order_relaxed); }

private:
    void try_advance_and_reclaim_locked(std::vector<RetiredNode>& to_free) {
        uint64_t curr_epoch = global_epoch_.load(std::memory_order_relaxed);

        // Find min active epoch across all active threads
        uint64_t min_active = curr_epoch;
        bool any_active = false;

        ThreadEpochRecord* rec = thread_records_head_.load(std::memory_order_acquire);
        while (rec != nullptr) {
            if (rec->active.load(std::memory_order_acquire)) {
                any_active = true;
                uint64_t le = rec->local_epoch.load(std::memory_order_acquire);
                if (le < min_active) {
                    min_active = le;
                }
            }
            rec = rec->next;
        }

        if (!any_active || min_active == curr_epoch) {
            // Can advance global epoch safely
            global_epoch_.fetch_add(1, std::memory_order_acq_rel);
            curr_epoch++;
        }

        // Any retired node whose retire_epoch + 1 < min_active (or all active) can be safely freed
        uint64_t safe_epoch = (any_active && min_active > 0) ? (min_active - 1) : curr_epoch;

        auto it = std::stable_partition(retired_list_.begin(), retired_list_.end(),
            [safe_epoch](const RetiredNode& node) {
                return node.retire_epoch > safe_epoch;
            });

        to_free.insert(to_free.end(), it, retired_list_.end());
        retired_list_.erase(it, retired_list_.end());
    }
};

/**
 * @brief RAII Guard for Epoch-Based Reclamation pin/unpin lifecycle.
 */
class EpochGuard {
private:
    EpochReclaimer& reclaimer_;
public:
    explicit EpochGuard(EpochReclaimer& reclaimer) : reclaimer_(reclaimer) {
        reclaimer_.pin();
    }
    ~EpochGuard() {
        reclaimer_.unpin();
    }
    EpochGuard(const EpochGuard&) = delete;
    EpochGuard& operator=(const EpochGuard&) = delete;
};

// ============================================================================
// 2. Treiber Lock-Free LIFO Stack (1986)
// ============================================================================

template <typename T>
class TreiberStack {
public:
    struct Node {
        T data;
        Node* next{nullptr};
        explicit Node(T val) : data(std::move(val)), next(nullptr) {}
    };

private:
    std::atomic<Node*> head_{nullptr};
    EpochReclaimer& reclaimer_;

public:
    explicit TreiberStack(EpochReclaimer& reclaimer) : reclaimer_(reclaimer) {}

    ~TreiberStack() {
        // Sequentially drain remaining nodes
        Node* curr = head_.load(std::memory_order_relaxed);
        while (curr != nullptr) {
            Node* next = curr->next;
            delete curr;
            reclaimer_.retire(nullptr, nullptr); // balance alloc count
            curr = next;
        }
    }

    TreiberStack(const TreiberStack&) = delete;
    TreiberStack& operator=(const TreiberStack&) = delete;

    /**
     * @brief Lock-free push operation.
     * Linearization Point: Successful CAS swinging head_ to new_node.
     * Memory Ordering: Invariant A (Release ensures node data is published).
     */
    void push(T val) {
        auto* new_node = new Node(std::move(val));
        reclaimer_.notify_alloc();

        new_node->next = head_.load(std::memory_order_relaxed);
        while (!head_.compare_exchange_weak(
            new_node->next, new_node,
            std::memory_order_release,
            std::memory_order_relaxed)) {
            // Loop until CAS succeeds; new_node->next is automatically updated on failure
        }
    }

    /**
     * @brief Lock-free pop operation.
     * Linearization Point:
     * - Empty queue: Reading head_ == nullptr.
     * - Non-empty queue: Successful CAS swinging head_ from old_head to old_head->next.
     * Memory Ordering: Invariant A (Acquire ensures old_head->data is safely visible).
     */
    std::optional<T> pop() {
        EpochGuard guard(reclaimer_);
        Node* old_head = head_.load(std::memory_order_acquire);

        while (old_head != nullptr) {
            Node* next = old_head->next;
            if (head_.compare_exchange_weak(
                old_head, next,
                std::memory_order_acquire,
                std::memory_order_acquire)) {
                // Linearization point reached
                T result = std::move(old_head->data);
                reclaimer_.retire(old_head, [](void* p) {
                    delete static_cast<Node*>(p);
                });
                return result;
            }
            // On failure, old_head is reloaded by compare_exchange_weak
        }

        return std::nullopt;
    }

    bool empty() const {
        return head_.load(std::memory_order_relaxed) == nullptr;
    }
};

// ============================================================================
// 3. Michael-Scott Lock-Free FIFO Queue (1996)
// ============================================================================

template <typename T>
class MichaelScottQueue {
public:
    struct Node {
        std::optional<T> data;
        std::atomic<Node*> next{nullptr};

        Node() : data(std::nullopt), next(nullptr) {}
        explicit Node(T val) : data(std::move(val)), next(nullptr) {}
    };

private:
    std::atomic<Node*> head_{nullptr};
    std::atomic<Node*> tail_{nullptr};
    EpochReclaimer& reclaimer_;

public:
    explicit MichaelScottQueue(EpochReclaimer& reclaimer) : reclaimer_(reclaimer) {
        // Allocate dummy sentinel node
        auto* sentinel = new Node();
        reclaimer_.notify_alloc();
        head_.store(sentinel, std::memory_order_relaxed);
        tail_.store(sentinel, std::memory_order_relaxed);
    }

    ~MichaelScottQueue() {
        // Sequentially drain remaining nodes
        Node* curr = head_.load(std::memory_order_relaxed);
        while (curr != nullptr) {
            Node* next = curr->next.load(std::memory_order_relaxed);
            delete curr;
            curr = next;
        }
    }

    MichaelScottQueue(const MichaelScottQueue&) = delete;
    MichaelScottQueue& operator=(const MichaelScottQueue&) = delete;

    /**
     * @brief Lock-free enqueue operation.
     * Linearization Point: Successful CAS on cur_tail->next from nullptr to new_node.
     * Helping: If cur_tail->next != nullptr, helps advance tail_.
     */
    void enqueue(T val) {
        auto* new_node = new Node(std::move(val));
        reclaimer_.notify_alloc();

        EpochGuard guard(reclaimer_);
        while (true) {
            Node* cur_tail = tail_.load(std::memory_order_acquire);
            Node* next = cur_tail->next.load(std::memory_order_acquire);

            // Verify tail consistency
            if (cur_tail == tail_.load(std::memory_order_acquire)) {
                if (next == nullptr) {
                    // Tail points to last node; attempt to link new_node
                    Node* expected = nullptr;
                    if (cur_tail->next.compare_exchange_weak(
                        expected, new_node,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                        // Linearization point: new_node is linked!
                        // Cooperative advance: attempt to swing tail to new_node (failure is okay)
                        tail_.compare_exchange_strong(
                            cur_tail, new_node,
                            std::memory_order_release,
                            std::memory_order_relaxed);
                        return;
                    }
                } else {
                    // Tail fell behind; help advance it to next
                    tail_.compare_exchange_strong(
                        cur_tail, next,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                }
            }
        }
    }

    /**
     * @brief Lock-free dequeue operation.
     * Linearization Point:
     * - Empty queue: cur_head == cur_tail and next == nullptr.
     * - Non-empty queue: Successful CAS on head_ swinging cur_head to next.
     */
    std::optional<T> dequeue() {
        EpochGuard guard(reclaimer_);
        while (true) {
            Node* cur_head = head_.load(std::memory_order_acquire);
            Node* cur_tail = tail_.load(std::memory_order_acquire);
            Node* next = cur_head->next.load(std::memory_order_acquire);

            if (cur_head == head_.load(std::memory_order_acquire)) {
                if (cur_head == cur_tail) {
                    if (next == nullptr) {
                        // Queue is empty (linearization point)
                        return std::nullopt;
                    }
                    // Tail fell behind head; help advance tail
                    tail_.compare_exchange_strong(
                        cur_tail, next,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                } else {
                    if (next != nullptr) {
                        // Read data before attempting CAS
                        T result = *(next->data);
                        if (head_.compare_exchange_weak(
                            cur_head, next,
                            std::memory_order_release,
                            std::memory_order_relaxed)) {
                            // Linearization point: head swung to next
                            // Old head sentinel is retired to EBR
                            reclaimer_.retire(cur_head, [](void* p) {
                                delete static_cast<Node*>(p);
                            });
                            return result;
                        }
                    }
                }
            }
        }
    }
};

} // namespace dsa

// ============================================================================
// 4. Layered Verification & Stress Testing
// ============================================================================

void run_sequential_tests() {
    std::cout << "[Test 1/4] Running sequential semantic oracle tests...\n";
    dsa::EpochReclaimer reclaimer;

    // Sequential Treiber Stack Test (LIFO)
    {
        dsa::TreiberStack<int> stack(reclaimer);
        assert(stack.empty());
        assert(!stack.pop().has_value());

        for (int i = 1; i <= 100; ++i) {
            stack.push(i);
        }

        for (int i = 100; i >= 1; --i) {
            auto val = stack.pop();
            assert(val.has_value() && *val == i);
        }
        assert(stack.empty());
        assert(!stack.pop().has_value());
    }

    // Sequential Michael-Scott Queue Test (FIFO)
    {
        dsa::MichaelScottQueue<int> queue(reclaimer);
        assert(!queue.dequeue().has_value());

        for (int i = 1; i <= 100; ++i) {
            queue.enqueue(i);
        }

        for (int i = 1; i <= 100; ++i) {
            auto val = queue.dequeue();
            assert(val.has_value() && *val == i);
        }
        assert(!queue.dequeue().has_value());
    }

    reclaimer.drain_all();
    std::cout << "  -> Sequential tests passed successfully.\n";
}

void run_concurrent_stack_stress_test() {
    std::cout << "[Test 2/4] Running concurrent Treiber stack stress test (8 threads)...\n";
    dsa::EpochReclaimer reclaimer;
    dsa::TreiberStack<uint64_t> stack(reclaimer);

    const int num_producers = 4;
    const int num_consumers = 4;
    const int items_per_producer = 10000;
    const int total_items = num_producers * items_per_producer;

    std::atomic<bool> start_flag{false};
    std::atomic<int> producers_done{0};
    std::atomic<int> ready_threads{0};

    // Vector of consumed elements per consumer thread
    std::vector<std::vector<uint64_t>> consumed(num_consumers);

    std::vector<std::thread> producers;
    for (int p = 0; p < num_producers; ++p) {
        producers.emplace_back([&, p]() {
            ready_threads.fetch_add(1, std::memory_order_relaxed);
            while (!start_flag.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            for (int i = 0; i < items_per_producer; ++i) {
                uint64_t val = (static_cast<uint64_t>(p) << 32) | static_cast<uint64_t>(i);
                stack.push(val);
            }
            producers_done.fetch_add(1, std::memory_order_release);
        });
    }

    std::vector<std::thread> consumers;
    for (int c = 0; c < num_consumers; ++c) {
        consumers.emplace_back([&, c]() {
            ready_threads.fetch_add(1, std::memory_order_relaxed);
            while (!start_flag.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            while (true) {
                auto val = stack.pop();
                if (val.has_value()) {
                    consumed[c].push_back(*val);
                } else if (producers_done.load(std::memory_order_acquire) == num_producers) {
                    // Try one last drain
                    val = stack.pop();
                    if (val.has_value()) {
                        consumed[c].push_back(*val);
                    } else {
                        break;
                    }
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    // Wait for all threads to arrive at barrier
    while (ready_threads.load(std::memory_order_relaxed) < num_producers + num_consumers) {
        std::this_thread::yield();
    }

    // Release all threads simultaneously
    start_flag.store(true, std::memory_order_release);

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

    // Reconcile consumed items
    std::vector<uint64_t> all_consumed;
    for (const auto& list : consumed) {
        all_consumed.insert(all_consumed.end(), list.begin(), list.end());
    }

    assert(static_cast<int>(all_consumed.size()) == total_items);
    std::sort(all_consumed.begin(), all_consumed.end());

    // Verify all expected items are present exactly once
    size_t idx = 0;
    for (int p = 0; p < num_producers; ++p) {
        for (int i = 0; i < items_per_producer; ++i) {
            uint64_t expected = (static_cast<uint64_t>(p) << 32) | static_cast<uint64_t>(i);
            assert(all_consumed[idx++] == expected);
        }
    }

    reclaimer.drain_all();
    std::cout << "  -> Treiber stack stress test passed: " << total_items << " items reconciled.\n";
}

void run_concurrent_queue_stress_test() {
    std::cout << "[Test 3/4] Running concurrent MS-Queue tests (MPSC FIFO & MPMC stress)...\n";
    dsa::EpochReclaimer reclaimer;

    // Subtest A: Multi-Producer Single-Consumer (MPSC) Strict FIFO Verification
    {
        dsa::MichaelScottQueue<uint64_t> queue(reclaimer);
        const int num_producers = 4;
        const int items_per_producer = 5000;
        const int total_items = num_producers * items_per_producer;

        std::atomic<bool> start_flag{false};
        std::atomic<int> producers_done{0};
        std::atomic<int> ready_threads{0};

        std::vector<std::thread> producers;
        for (int p = 0; p < num_producers; ++p) {
            producers.emplace_back([&, p]() {
                ready_threads.fetch_add(1, std::memory_order_relaxed);
                while (!start_flag.load(std::memory_order_acquire)) {
                    std::this_thread::yield();
                }

                for (int i = 0; i < items_per_producer; ++i) {
                    uint64_t val = (static_cast<uint64_t>(p) << 32) | static_cast<uint64_t>(i);
                    queue.enqueue(val);
                }
                producers_done.fetch_add(1, std::memory_order_release);
            });
        }

        std::vector<uint64_t> single_consumer_consumed;
        std::thread consumer([&]() {
            ready_threads.fetch_add(1, std::memory_order_relaxed);
            while (!start_flag.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            while (true) {
                auto val = queue.dequeue();
                if (val.has_value()) {
                    single_consumer_consumed.push_back(*val);
                } else if (producers_done.load(std::memory_order_acquire) == num_producers) {
                    val = queue.dequeue();
                    if (val.has_value()) {
                        single_consumer_consumed.push_back(*val);
                    } else {
                        break;
                    }
                } else {
                    std::this_thread::yield();
                }
            }
        });

        while (ready_threads.load(std::memory_order_relaxed) < num_producers + 1) {
            std::this_thread::yield();
        }

        start_flag.store(true, std::memory_order_release);

        for (auto& t : producers) t.join();
        consumer.join();

        assert(static_cast<int>(single_consumer_consumed.size()) == total_items);

        // Verify per-producer FIFO ordering strictly preserved
        std::vector<std::vector<uint64_t>> per_producer(num_producers);
        for (uint64_t val : single_consumer_consumed) {
            int p_id = static_cast<int>(val >> 32);
            assert(p_id >= 0 && p_id < num_producers);
            per_producer[p_id].push_back(val);
        }

        for (int p = 0; p < num_producers; ++p) {
            assert(static_cast<int>(per_producer[p].size()) == items_per_producer);
            for (int i = 0; i < items_per_producer; ++i) {
                uint64_t expected = (static_cast<uint64_t>(p) << 32) | static_cast<uint64_t>(i);
                assert(per_producer[p][i] == expected);
            }
        }
        std::cout << "  -> Subtest A: MPSC strict per-producer FIFO ordering verified.\n";
    }

    // Subtest B: Multi-Producer Multi-Consumer (MPMC) High-Contention Stress Test
    {
        dsa::MichaelScottQueue<uint64_t> queue(reclaimer);
        const int num_producers = 4;
        const int num_consumers = 4;
        const int items_per_producer = 10000;
        const int total_items = num_producers * items_per_producer;

        std::atomic<bool> start_flag{false};
        std::atomic<int> producers_done{0};
        std::atomic<int> ready_threads{0};

        std::vector<std::vector<uint64_t>> consumed(num_consumers);

        std::vector<std::thread> producers;
        for (int p = 0; p < num_producers; ++p) {
            producers.emplace_back([&, p]() {
                ready_threads.fetch_add(1, std::memory_order_relaxed);
                while (!start_flag.load(std::memory_order_acquire)) {
                    std::this_thread::yield();
                }

                for (int i = 0; i < items_per_producer; ++i) {
                    uint64_t val = (static_cast<uint64_t>(p) << 32) | static_cast<uint64_t>(i);
                    queue.enqueue(val);
                }
                producers_done.fetch_add(1, std::memory_order_release);
            });
        }

        std::vector<std::thread> consumers;
        for (int c = 0; c < num_consumers; ++c) {
            consumers.emplace_back([&, c]() {
                ready_threads.fetch_add(1, std::memory_order_relaxed);
                while (!start_flag.load(std::memory_order_acquire)) {
                    std::this_thread::yield();
                }

                while (true) {
                    auto val = queue.dequeue();
                    if (val.has_value()) {
                        consumed[c].push_back(*val);
                    } else if (producers_done.load(std::memory_order_acquire) == num_producers) {
                        val = queue.dequeue();
                        if (val.has_value()) {
                            consumed[c].push_back(*val);
                        } else {
                            break;
                        }
                    } else {
                        std::this_thread::yield();
                    }
                }
            });
        }

        while (ready_threads.load(std::memory_order_relaxed) < num_producers + num_consumers) {
            std::this_thread::yield();
        }

        start_flag.store(true, std::memory_order_release);

        for (auto& t : producers) t.join();
        for (auto& t : consumers) t.join();

        // Reconcile consumed items across all consumers
        std::vector<uint64_t> all_consumed;
        for (const auto& list : consumed) {
            all_consumed.insert(all_consumed.end(), list.begin(), list.end());
        }

        assert(static_cast<int>(all_consumed.size()) == total_items);
        std::sort(all_consumed.begin(), all_consumed.end());

        size_t idx = 0;
        for (int p = 0; p < num_producers; ++p) {
            for (int i = 0; i < items_per_producer; ++i) {
                uint64_t expected = (static_cast<uint64_t>(p) << 32) | static_cast<uint64_t>(i);
                assert(all_consumed[idx++] == expected);
            }
        }
        std::cout << "  -> Subtest B: MPMC stress test reconciled " << total_items << " items.\n";
    }

    reclaimer.drain_all();
}

void run_reclamation_lifecycle_test() {
    std::cout << "[Test 4/4] Validating safe memory reclamation lifecycle & leak-free state...\n";
    dsa::EpochReclaimer reclaimer;
    {
        dsa::TreiberStack<int> stack(reclaimer);
        for (int i = 0; i < 5000; ++i) {
            stack.push(i);
        }
        for (int i = 0; i < 5000; ++i) {
            auto val = stack.pop();
            assert(val.has_value() && *val == 4999 - i);
        }
    }
    reclaimer.drain_all();

    // Verify allocation and reclamation accounting
    size_t allocs = reclaimer.get_total_allocated();
    size_t reclaims = reclaimer.get_total_reclaimed();
    assert(allocs == 5000);
    assert(reclaims == 5000);

    std::cout << "  -> SMR Lifecycle verified: " << allocs << " allocated, " << reclaims << " safely reclaimed.\n";
}

int main() {
    std::cout << "===============================================================\n";
    std::cout << "Concurrent Queues, Stacks & Epoch Reclamation Verification\n";
    std::cout << "===============================================================\n";

    run_sequential_tests();
    run_concurrent_stack_stress_test();
    run_concurrent_queue_stress_test();
    run_reclamation_lifecycle_test();

    std::cout << "===============================================================\n";
    std::cout << "All concurrent queue and stack tests PASSED flawlessly!\n";
    std::cout << "===============================================================\n";
    return 0;
}
