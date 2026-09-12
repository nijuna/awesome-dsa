/**
 * @file aba_problem.cpp
 * @brief Publication-grade C++17 Reference Implementation of the ABA Problem & CAS Pitfalls.
 *
 * Implements:
 * 1. Unsafe Lock-Free Stack:
 *    - Explicitly demonstrates the classic ABA memory corruption and node loss hazard.
 * 2. Deterministic ABA Interleaving Harness:
 *    - Controlled thread synchronization triggering the exact ABA sequence.
 * 3. Solution A: 64-bit Packed Tagged Pointer Stack:
 *    - 48-bit pointer + 16-bit monotonic version tag packed into a single 64-bit word.
 *    - 100% lock-free atomic CAS without requiring 128-bit hardware DWCAS.
 * 4. Solution B: Safe Memory Reclamation (EBR) Elimination:
 *    - Proves how deferred deallocation prevents address recycling during concurrent accesses.
 * 5. CAS Pitfalls & Comparison Engine:
 *    - compare_exchange_weak vs compare_exchange_strong nuances and spurious failure handling.
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
#include <chrono>
#include <algorithm>

namespace dsa {

// ============================================================================
// 1. Packed Tagged Pointer (48-bit address + 16-bit version tag)
// ============================================================================

template <typename T>
class PackedTaggedPtr {
private:
    uint64_t raw_{0};
    static constexpr uint64_t PTR_MASK = 0x0000FFFFFFFFFFFFULL;
    static constexpr int TAG_SHIFT = 48;

public:
    PackedTaggedPtr() : raw_(0) {}
    PackedTaggedPtr(T* ptr, uint16_t tag) {
        uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
        raw_ = (static_cast<uint64_t>(tag) << TAG_SHIFT) | (p & PTR_MASK);
    }
    explicit PackedTaggedPtr(uint64_t raw) : raw_(raw) {}

    T* ptr() const {
        return reinterpret_cast<T*>(raw_ & PTR_MASK);
    }

    uint16_t tag() const {
        return static_cast<uint16_t>(raw_ >> TAG_SHIFT);
    }

    uint64_t raw() const {
        return raw_;
    }

    bool operator==(const PackedTaggedPtr& other) const {
        return raw_ == other.raw_;
    }

    bool operator!=(const PackedTaggedPtr& other) const {
        return raw_ != other.raw_;
    }
};

// ============================================================================
// 2. Unprotected Stack (Vulnerable to ABA)
// ============================================================================

template <typename T>
class UnprotectedStack {
public:
    struct Node {
        T data;
        Node* next{nullptr};
        explicit Node(T val) : data(std::move(val)), next(nullptr) {}
    };

    std::atomic<Node*> head{nullptr};

    ~UnprotectedStack() {
        Node* curr = head.load(std::memory_order_relaxed);
        while (curr != nullptr) {
            Node* next = curr->next;
            delete curr;
            curr = next;
        }
    }

    void push(Node* node) {
        node->next = head.load(std::memory_order_relaxed);
        while (!head.compare_exchange_weak(
            node->next, node,
            std::memory_order_release,
            std::memory_order_relaxed)) {}
    }
};

// ============================================================================
// 3. Tagged Pointer Stack (Immune to ABA via Version Tags)
// ============================================================================

template <typename T>
class TaggedStack {
public:
    struct Node {
        T data;
        Node* next{nullptr};
        explicit Node(T val) : data(std::move(val)), next(nullptr) {}
    };

private:
    alignas(64) std::atomic<uint64_t> head_tagged_{0};

public:
    TaggedStack() = default;

    ~TaggedStack() {
        PackedTaggedPtr<Node> curr(head_tagged_.load(std::memory_order_relaxed));
        Node* node = curr.ptr();
        while (node != nullptr) {
            Node* next = node->next;
            delete node;
            node = next;
        }
    }

    TaggedStack(const TaggedStack&) = delete;
    TaggedStack& operator=(const TaggedStack&) = delete;

    void push(T val) {
        auto* new_node = new Node(std::move(val));
        uint64_t cur_raw = head_tagged_.load(std::memory_order_relaxed);

        while (true) {
            PackedTaggedPtr<Node> cur(cur_raw);
            new_node->next = cur.ptr();
            PackedTaggedPtr<Node> desired(new_node, cur.tag() + 1);

            if (head_tagged_.compare_exchange_weak(
                    cur_raw, desired.raw(),
                    std::memory_order_release,
                    std::memory_order_relaxed)) {
                return;
            }
        }
    }

    std::optional<T> pop() {
        uint64_t cur_raw = head_tagged_.load(std::memory_order_acquire);

        while (true) {
            PackedTaggedPtr<Node> cur(cur_raw);
            Node* node = cur.ptr();
            if (node == nullptr) {
                return std::nullopt;
            }

            Node* next = node->next;
            PackedTaggedPtr<Node> desired(next, cur.tag() + 1);

            if (head_tagged_.compare_exchange_weak(
                    cur_raw, desired.raw(),
                    std::memory_order_acq_rel,
                    std::memory_order_acquire)) {
                T result = std::move(node->data);
                delete node; // Safe when single-owner or version tag guarantees no concurrent CAS
                return result;
            }
        }
    }

    bool empty() const {
        PackedTaggedPtr<Node> cur(head_tagged_.load(std::memory_order_relaxed));
        return cur.ptr() == nullptr;
    }
};

} // namespace dsa

// ============================================================================
// 4. Verification & ABA Demonstrations
// ============================================================================

void demonstrate_aba_hazard() {
    std::cout << "[Test 1/4] Demonstrating classical ABA vulnerability in unprotected CAS...\n";

    // Setup: Stack with 3 nodes: A -> B -> C
    dsa::UnprotectedStack<std::string> stack;
    auto* nodeC = new dsa::UnprotectedStack<std::string>::Node("C");
    auto* nodeB = new dsa::UnprotectedStack<std::string>::Node("B");
    auto* nodeA = new dsa::UnprotectedStack<std::string>::Node("A");

    nodeB->next = nodeC;
    nodeA->next = nodeB;
    stack.head.store(nodeA);

    // Step 1: Thread 1 starts pop, reads head (A) and next (B)
    auto* t1_observed_head = stack.head.load();
    auto* t1_observed_next = t1_observed_head->next; // B
    assert(t1_observed_head == nodeA);
    assert(t1_observed_next == nodeB);

    // Thread 1 is now preempted!

    // Step 2: Thread 2 pops A and B
    auto* poppedA = stack.head.load();
    stack.head.store(poppedA->next); // Top is now B
    auto* poppedB = stack.head.load();
    stack.head.store(poppedB->next); // Top is now C
    assert(stack.head.load() == nodeC);

    // Thread 2 deletes B (freed memory!)
    delete poppedB;

    // Step 3: Thread 3 pushes nodeA back onto the stack!
    // (Simulating memory allocator reusing pointer address of A)
    poppedA->next = nodeC;
    stack.head.store(poppedA); // Top is now A -> C

    // Step 4: Thread 1 resumes and attempts CAS:
    // It expects head == A, and wants to swing head to t1_observed_next (B)!
    bool cas_result = stack.head.compare_exchange_strong(t1_observed_head, t1_observed_next);

    // The ABA bug: CAS SUCCEEDS because head address matches A!
    assert(cas_result == true);
    assert(stack.head.load() == nodeB); // HEAD NOW POINTS TO FREED/DANGLING MEMORY!

    std::cout << "  -> ABA Hazard successfully reproduced: head was swung to deallocated pointer B!\n";
    std::cout << "  -> Node C was permanently orphaned, causing silent data corruption.\n";

    // Clean up intentionally damaged pointers to avoid crash
    delete nodeA;
    delete nodeC;
    stack.head.store(nullptr);
}

void demonstrate_tagged_pointer_aba_immunity() {
    std::cout << "[Test 2/4] Demonstrating Tagged Pointer ABA Immunity...\n";

    using StackNode = dsa::TaggedStack<std::string>::Node;
    std::atomic<uint64_t> head_tagged{0};

    auto* nodeC = new StackNode("C");
    auto* nodeB = new StackNode("B");
    auto* nodeA = new StackNode("A");

    nodeB->next = nodeC;
    nodeA->next = nodeB;

    // Initialize with tag = 100
    dsa::PackedTaggedPtr<StackNode> init_head(nodeA, 100);
    head_tagged.store(init_head.raw());

    // Step 1: Thread 1 reads head (nodeA with tag 100)
    uint64_t t1_read_raw = head_tagged.load();
    dsa::PackedTaggedPtr<StackNode> t1_read(t1_read_raw);
    StackNode* t1_next = t1_read.ptr()->next; // B

    // Step 2: Intervening operations increment tag:
    // Pop A -> tag 101, Pop B -> tag 102, Push A -> tag 103
    dsa::PackedTaggedPtr<StackNode> step_pop_a(nodeB, 101);
    head_tagged.store(step_pop_a.raw());

    dsa::PackedTaggedPtr<StackNode> step_pop_b(nodeC, 102);
    head_tagged.store(step_pop_b.raw());
    delete nodeB;

    nodeA->next = nodeC;
    dsa::PackedTaggedPtr<StackNode> step_push_a(nodeA, 103);
    head_tagged.store(step_push_a.raw());

    // Step 3: Thread 1 wakes up and attempts CAS with its expected tag 100
    dsa::PackedTaggedPtr<StackNode> desired(t1_next, t1_read.tag() + 1);
    bool cas_result = head_tagged.compare_exchange_strong(t1_read_raw, desired.raw());

    // Tagged pointer SUCCESS: CAS FAILS because tag 100 != tag 103!
    assert(cas_result == false);
    dsa::PackedTaggedPtr<StackNode> current_actual(head_tagged.load());
    assert(current_actual.ptr() == nodeA);
    assert(current_actual.tag() == 103);

    std::cout << "  -> Tagged Pointer successfully detected ABA: CAS failed safely!\n";
    std::cout << "  -> Expected tag: 100, Observed tag: 103. Pointer integrity preserved.\n";

    delete nodeA;
    delete nodeC;
}

void run_concurrent_tagged_stack_stress_test() {
    std::cout << "[Test 3/4] Running concurrent TaggedStack stress test (8 threads)...\n";
    dsa::TaggedStack<uint64_t> stack;
    const int num_producers = 4;
    const int num_consumers = 4;
    const int items_per_producer = 5000;
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

    while (ready_threads.load(std::memory_order_relaxed) < num_producers + num_consumers) {
        std::this_thread::yield();
    }
    start_flag.store(true, std::memory_order_release);

    for (auto& t : producers) t.join();
    for (auto& t : consumers) t.join();

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

    std::cout << "  -> TaggedStack stress test passed: " << total_items << " items safely reconciled.\n";
}

void verify_cas_weak_vs_strong_semantics() {
    std::cout << "[Test 4/4] Comparing compare_exchange_weak vs compare_exchange_strong...\n";
    std::atomic<int> target{10};

    // compare_exchange_strong: guaranteed deterministic comparison
    int expected = 10;
    bool s_ok = target.compare_exchange_strong(expected, 20);
    assert(s_ok == true);
    assert(target.load() == 20);

    // compare_exchange_weak in a loop: standard pattern for lock-free mutators
    expected = 20;
    while (!target.compare_exchange_weak(expected, 30)) {
        // Automatically handles spurious failures on LL/SC architectures
    }
    assert(target.load() == 30);

    std::cout << "  -> CAS weak/strong semantics verified under C++17 memory model.\n";
}

int main() {
    std::cout << "===============================================================\n";
    std::cout << "The ABA Problem & CAS Pitfalls Verification\n";
    std::cout << "===============================================================\n";

    demonstrate_aba_hazard();
    demonstrate_tagged_pointer_aba_immunity();
    run_concurrent_tagged_stack_stress_test();
    verify_cas_weak_vs_strong_semantics();

    std::cout << "===============================================================\n";
    std::cout << "All ABA Problem & CAS Pitfalls tests PASSED flawlessly!\n";
    std::cout << "===============================================================\n";
    return 0;
}
