/**
 * @file work_stealing_deques.cpp
 * @brief Publication-grade C++17 Reference Implementation of the Chase-Lev Work-Stealing Deque.
 *
 * Implements:
 * 1. Chase-Lev Lock-Free Work-Stealing Deque (David Chase & Yossi Lev, 2005):
 *    - Circular buffer with power-of-two capacity and atomic top/bottom indices.
 *    - Asymmetric concurrency: Owner operates at bottom (LIFO), thieves steal at top (FIFO).
 *    - Sequential consistency memory fences resolving the single-element race window.
 *    - Dynamic array resizing preserving lock-free steals.
 * 2. Realistic Work-Stealing Task Scheduler:
 *    - Thread pool where each worker executes its own deque (depth-first execution).
 *    - Randomized victim selection for work stealing under task exhaustion.
 * 3. Comprehensive Layered Verification Harness:
 *    - Layer 1: Owner LIFO sequential semantic oracle.
 *    - Layer 2: Single-element race resolution (owner vs. thieves).
 *    - Layer 3: Parallel divide-and-conquer computation (Parallel Tree Sum) via work stealing.
 *    - Layer 4: High-contention concurrent stress test with full item reconciliation.
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
#include <memory>
#include <random>
#include <functional>
#include <chrono>
#include <mutex>
#include <algorithm>

namespace dsa {

// ============================================================================
// 1. Chase-Lev Work-Stealing Deque
// ============================================================================

template <typename T>
class ChaseLevDeque {
private:
    struct CircularArray {
        size_t capacity;
        size_t mask;
        std::atomic<T>* buffer;

        explicit CircularArray(size_t cap)
            : capacity(cap), mask(cap - 1), buffer(new std::atomic<T>[cap]) {}

        ~CircularArray() {
            delete[] buffer;
        }

        void put(int64_t index, T item) {
            buffer[index & mask].store(item, std::memory_order_relaxed);
        }

        T get(int64_t index) {
            return buffer[index & mask].load(std::memory_order_relaxed);
        }

        CircularArray* grow(int64_t top, int64_t bottom) {
            auto* new_arr = new CircularArray(capacity * 2);
            for (int64_t i = top; i < bottom; ++i) {
                new_arr->put(i, get(i));
            }
            return new_arr;
        }
    };

    alignas(64) std::atomic<int64_t> top_{0};
    alignas(64) std::atomic<int64_t> bottom_{0};
    alignas(64) std::atomic<CircularArray*> array_{nullptr};
    std::vector<std::unique_ptr<CircularArray>> old_arrays_;
    std::mutex resize_mutex_;

public:
    explicit ChaseLevDeque(size_t initial_capacity = 1024) {
        // Capacity must be power of 2
        size_t cap = 1;
        while (cap < initial_capacity) cap <<= 1;
        array_.store(new CircularArray(cap), std::memory_order_relaxed);
    }

    ~ChaseLevDeque() {
        delete array_.load(std::memory_order_relaxed);
    }

    ChaseLevDeque(const ChaseLevDeque&) = delete;
    ChaseLevDeque& operator=(const ChaseLevDeque&) = delete;

    /**
     * @brief Push item to the bottom of the deque (Owner thread only).
     * Time: O(1) amortized.
     */
    void push(T item) {
        int64_t b = bottom_.load(std::memory_order_relaxed);
        int64_t t = top_.load(std::memory_order_acquire);
        CircularArray* a = array_.load(std::memory_order_relaxed);

        if (b - t >= static_cast<int64_t>(a->capacity)) {
            // Deque is full; resize array
            std::lock_guard<std::mutex> lock(resize_mutex_);
            CircularArray* new_arr = a->grow(t, b);
            old_arrays_.emplace_back(a);
            array_.store(new_arr, std::memory_order_release);
            a = new_arr;
        }

        a->put(b, item);
        std::atomic_thread_fence(std::memory_order_release);
        bottom_.store(b + 1, std::memory_order_relaxed);
    }

    /**
     * @brief Pop item from the bottom of the deque (Owner thread only).
     * Time: O(1).
     * Resolves race condition with concurrent thieves on the single remaining item.
     */
    std::optional<T> pop() {
        int64_t b = bottom_.load(std::memory_order_relaxed) - 1;
        CircularArray* a = array_.load(std::memory_order_relaxed);
        bottom_.store(b, std::memory_order_relaxed);

        // Crucial sequential consistency fence: ensures bottom_ store is globally
        // visible before top_ is loaded, preventing double-consumption race with thieves.
        std::atomic_thread_fence(std::memory_order_seq_cst);
        int64_t t = top_.load(std::memory_order_relaxed);

        if (t <= b) {
            // Non-empty deque
            T item = a->get(b);
            if (t == b) {
                // Exactly ONE item remaining: race against concurrent thieves!
                int64_t expected = t;
                if (!top_.compare_exchange_strong(
                        expected, t + 1,
                        std::memory_order_seq_cst,
                        std::memory_order_relaxed)) {
                    // A thief won the race and stole the last item!
                    bottom_.store(t + 1, std::memory_order_relaxed);
                    return std::nullopt;
                }
                bottom_.store(t + 1, std::memory_order_relaxed);
                return item;
            }
            // More than one item: owner pops without contention
            return item;
        } else {
            // Deque was empty
            bottom_.store(t, std::memory_order_relaxed);
            return std::nullopt;
        }
    }

    /**
     * @brief Steal item from the top of the deque (Concurrent thief threads).
     * Time: O(1) uncontended.
     */
    std::optional<T> steal() {
        int64_t t = top_.load(std::memory_order_acquire);
        std::atomic_thread_fence(std::memory_order_seq_cst);
        int64_t b = bottom_.load(std::memory_order_acquire);

        if (t < b) {
            // Deque appears non-empty
            CircularArray* a = array_.load(std::memory_order_consume);
            T item = a->get(t);

            // Attempt CAS on top_ to claim item
            int64_t expected = t;
            if (top_.compare_exchange_weak(
                    expected, t + 1,
                    std::memory_order_seq_cst,
                    std::memory_order_relaxed)) {
                return item; // Successfully stolen!
            }
        }
        return std::nullopt;
    }

    int64_t size() const {
        int64_t b = bottom_.load(std::memory_order_relaxed);
        int64_t t = top_.load(std::memory_order_relaxed);
        return std::max<int64_t>(0, b - t);
    }

    bool empty() const {
        return size() == 0;
    }
};

// ============================================================================
// 2. Work-Stealing Task Scheduler Simulation
// ============================================================================

class WorkStealingScheduler {
public:
    using Task = std::function<void()>;
    static inline thread_local size_t current_worker_id_{0};

private:
    size_t num_workers_;
    std::vector<std::unique_ptr<ChaseLevDeque<Task*>>> deques_;
    std::vector<std::thread> workers_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> completed_tasks_{0};
    std::atomic<size_t> total_steals_{0};

public:
    explicit WorkStealingScheduler(size_t num_workers) : num_workers_(num_workers) {
        for (size_t i = 0; i < num_workers_; ++i) {
            deques_.push_back(std::make_unique<ChaseLevDeque<Task*>>(2048));
        }
    }

    ~WorkStealingScheduler() {
        stop();
    }

    void submit_initial(size_t worker_id, Task* t) {
        assert(worker_id < num_workers_);
        deques_[worker_id]->push(t);
    }

    void spawn(Task* t) {
        // Enforce SPMC invariant: worker thread always pushes to its OWN deque!
        deques_[current_worker_id_]->push(t);
    }

    void start() {
        for (size_t i = 0; i < num_workers_; ++i) {
            workers_.emplace_back([this, i]() {
                current_worker_id_ = i;
                worker_loop(i);
            });
        }
    }

    void stop() {
        if (!stop_.load()) {
            stop_.store(true, std::memory_order_release);
            for (auto& w : workers_) {
                if (w.joinable()) w.join();
            }
        }
    }

    size_t get_completed_tasks() const { return completed_tasks_.load(); }
    size_t get_total_steals() const { return total_steals_.load(); }

private:
    void worker_loop(size_t my_id) {
        std::mt19937_64 rng(1337 + my_id);
        std::uniform_int_distribution<size_t> victim_dist(0, num_workers_ - 1);

        while (!stop_.load(std::memory_order_relaxed)) {
            // 1. Try popping from own deque (LIFO - Depth First)
            auto task_opt = deques_[my_id]->pop();
            if (task_opt.has_value()) {
                Task* t = *task_opt;
                (*t)();
                delete t;
                completed_tasks_.fetch_add(1, std::memory_order_relaxed);
                continue;
            }

            // 2. Own deque empty; attempt to steal from random victim (FIFO - Breadth First)
            size_t victim = victim_dist(rng);
            if (victim != my_id) {
                auto stolen = deques_[victim]->steal();
                if (stolen.has_value()) {
                    total_steals_.fetch_add(1, std::memory_order_relaxed);
                    Task* t = *stolen;
                    (*t)();
                    delete t;
                    completed_tasks_.fetch_add(1, std::memory_order_relaxed);
                    continue;
                }
            }

            std::this_thread::yield();
        }
    }
};

} // namespace dsa

// ============================================================================
// 3. Verification & Stress Test Harness
// ============================================================================

void test_owner_sequential_lifo() {
    std::cout << "[Test 1/4] Validating owner sequential LIFO semantics...\n";
    dsa::ChaseLevDeque<int> deque;

    assert(deque.empty());
    assert(!deque.pop().has_value());

    for (int i = 1; i <= 100; ++i) {
        deque.push(i);
    }
    assert(deque.size() == 100);

    // Owner pop must return strictly LIFO: 100 down to 1
    for (int i = 100; i >= 1; --i) {
        auto val = deque.pop();
        assert(val.has_value() && *val == i);
    }
    assert(deque.empty());
    assert(!deque.pop().has_value());

    std::cout << "  -> Owner sequential LIFO verified successfully.\n";
}

void test_single_item_race() {
    std::cout << "[Test 2/4] Testing single-item race condition (Owner vs. 3 Thieves)...\n";

    // Run 500 trials of the race condition
    for (int trial = 0; trial < 500; ++trial) {
        dsa::ChaseLevDeque<int> deque;
        deque.push(42); // Exactly one item

        std::atomic<int> winners{0};
        std::atomic<bool> start_flag{false};

        // 3 Thief threads
        std::vector<std::thread> thieves;
        for (int t = 0; t < 3; ++t) {
            thieves.emplace_back([&]() {
                while (!start_flag.load(std::memory_order_acquire)) {
                    std::this_thread::yield();
                }
                auto res = deque.steal();
                if (res.has_value() && *res == 42) {
                    winners.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        // Owner thread pops concurrently
        start_flag.store(true, std::memory_order_release);
        auto owner_res = deque.pop();
        if (owner_res.has_value() && *owner_res == 42) {
            winners.fetch_add(1, std::memory_order_relaxed);
        }

        for (auto& th : thieves) th.join();

        // Exactly ONE thread must win the single item (Linearizability guarantee)
        assert(winners.load() == 1);
        assert(deque.empty());
    }

    std::cout << "  -> Single-item race verified: Exactly 1 winner across 500 concurrent trials.\n";
}

void test_work_stealing_scheduler() {
    std::cout << "[Test 3/4] Running divide-and-conquer computation via WorkStealingScheduler...\n";

    const size_t NUM_WORKERS = 4;
    dsa::WorkStealingScheduler scheduler(NUM_WORKERS);

    std::atomic<int64_t> global_sum{0};

    // Parallel recursive tree sum task generator
    std::function<void(int64_t, int64_t)> spawn_range;
    spawn_range = [&](int64_t left, int64_t right) {
        if (right - left <= 100) {
            int64_t local_sum = 0;
            for (int64_t i = left; i < right; ++i) local_sum += i;
            global_sum.fetch_add(local_sum, std::memory_order_relaxed);
            return;
        }

        int64_t mid = left + (right - left) / 2;

        // Push right half to current worker's own deque (SPMC safe)
        auto* right_task = new dsa::WorkStealingScheduler::Task([=]() {
            spawn_range(mid, right);
        });
        scheduler.spawn(right_task);

        // Execute left half locally
        spawn_range(left, mid);
    };

    const int64_t N = 10000;
    auto* root_task = new dsa::WorkStealingScheduler::Task([&]() {
        spawn_range(0, N);
    });
    scheduler.submit_initial(0, root_task);
    scheduler.start();

    // Wait for computation to complete
    int64_t expected_sum = (N * (N - 1)) / 2;
    while (global_sum.load(std::memory_order_relaxed) != expected_sum) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
    }

    scheduler.stop();

    assert(global_sum.load() == expected_sum);
    std::cout << "  -> Scheduler verified: Global sum " << expected_sum
              << " computed with " << scheduler.get_total_steals() << " work steals across workers.\n";
}

void test_concurrent_stress_reconciliation() {
    std::cout << "[Test 4/4] Running concurrent ChaseLevDeque stress test (1 Owner, 4 Thieves)...\n";
    dsa::ChaseLevDeque<uint64_t> deque(2048);

    const int TOTAL_ITEMS = 30000;
    std::atomic<bool> start_flag{false};
    std::atomic<bool> owner_done{false};

    std::vector<uint64_t> owner_consumed;
    std::vector<std::vector<uint64_t>> thief_consumed(4);

    // 4 Thief threads
    std::vector<std::thread> thieves;
    for (int t = 0; t < 4; ++t) {
        thieves.emplace_back([&, t]() {
            while (!start_flag.load(std::memory_order_acquire)) {
                std::this_thread::yield();
            }

            while (true) {
                auto val = deque.steal();
                if (val.has_value()) {
                    thief_consumed[t].push_back(*val);
                } else if (owner_done.load(std::memory_order_acquire) && deque.empty()) {
                    break;
                } else {
                    std::this_thread::yield();
                }
            }
        });
    }

    // Owner thread: pushes 30,000 items, periodically popping
    std::thread owner([&]() {
        while (!start_flag.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }

        for (uint64_t i = 0; i < TOTAL_ITEMS; ++i) {
            deque.push(i);
            if (i % 4 == 0) {
                auto val = deque.pop();
                if (val.has_value()) {
                    owner_consumed.push_back(*val);
                }
            }
        }

        // Drain remainder
        while (true) {
            auto val = deque.pop();
            if (val.has_value()) {
                owner_consumed.push_back(*val);
            } else {
                break;
            }
        }
        owner_done.store(true, std::memory_order_release);
    });

    start_flag.store(true, std::memory_order_release);

    owner.join();
    for (auto& th : thieves) th.join();

    // Reconcile all consumed items
    std::vector<uint64_t> all_consumed = owner_consumed;
    for (const auto& list : thief_consumed) {
        all_consumed.insert(all_consumed.end(), list.begin(), list.end());
    }

    assert(static_cast<int>(all_consumed.size()) == TOTAL_ITEMS);
    std::sort(all_consumed.begin(), all_consumed.end());

    for (int i = 0; i < TOTAL_ITEMS; ++i) {
        assert(all_consumed[i] == static_cast<uint64_t>(i));
    }

    std::cout << "  -> Chase-Lev stress test passed: " << TOTAL_ITEMS << " items reconciled.\n";
    std::cout << "     Owner popped: " << owner_consumed.size() << ", Thieves stole: "
              << (TOTAL_ITEMS - owner_consumed.size()) << " items.\n";
}

int main() {
    std::cout << "===============================================================\n";
    std::cout << "Chase-Lev Work-Stealing Deque & Scheduler Verification\n";
    std::cout << "===============================================================\n";

    test_owner_sequential_lifo();
    test_single_item_race();
    test_work_stealing_scheduler();
    test_concurrent_stress_reconciliation();

    std::cout << "===============================================================\n";
    std::cout << "All Work-Stealing Deque tests PASSED flawlessly!\n";
    std::cout << "===============================================================\n";
    return 0;
}
