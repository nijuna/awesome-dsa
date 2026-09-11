/**
 * Benchmark Harness: std::priority_queue (2-ary) vs. 4-ary Heap vs. 8-ary Heap vs. Pairing Heap
 *
 * Empirical evaluation of:
 * 1. Bulk Insertion Throughput (1,000,000 keys)
 * 2. Bulk Extract-Min Latency (1,000,000 keys)
 * 3. Steady-State Event Queue / Scheduler (1,000,000 push/pop cycles at size 500k)
 * 4. Priority Queue Melding / Union (1,000 independent heaps of size 1,000)
 * 5. Dijkstra / Priority Decrease Workload (100,000 nodes, 1,000,000 edge relaxations)
 * 6. Physical Memory Footprint (RSS and approximate bytes/element)
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <queue>
#include <chrono>
#include <iomanip>
#include <cstddef>
#include <cstdint>
#include <random>
#include <algorithm>
#include <fstream>
#include <unistd.h>
#include <malloc.h>
#include <cassert>

// Measure Resident Set Size (RSS) in bytes from Linux /proc/self/statm
static std::size_t get_current_rss_bytes() {
    std::ifstream statm("/proc/self/statm");
    if (!statm.is_open()) return 0;
    std::size_t size_pages = 0, resident_pages = 0;
    statm >> size_pages >> resident_pages;
    long page_size = sysconf(_SC_PAGESIZE);
    return resident_pages * static_cast<std::size_t>(page_size);
}

// Fast xorshift64 PRNG for deterministic random workloads
class FastRng {
private:
    uint64_t state_;

public:
    explicit FastRng(uint64_t seed = 88172645463325252ULL) : state_(seed) {}

    inline uint64_t next() noexcept {
        state_ ^= state_ << 13;
        state_ ^= state_ >> 7;
        state_ ^= state_ << 17;
        return state_;
    }

    inline uint64_t next_range(uint64_t max_exclusive) noexcept {
        return next() % max_exclusive;
    }
};

// ============================================================================
// 1. Benchmark d-ary Heap (Branching Factor D)
// ============================================================================
template <typename T, std::size_t D = 4>
class BenchmarkDAryHeap {
    static_assert(D >= 2, "Branching factor D must be at least 2");

private:
    std::vector<T> data_;

    void sift_up(std::size_t i) {
        T val = std::move(data_[i]);
        while (i > 0) {
            std::size_t p = (i - 1) / D;
            if (val < data_[p]) {
                data_[i] = std::move(data_[p]);
                i = p;
            } else {
                break;
            }
        }
        data_[i] = std::move(val);
    }

    void sift_down(std::size_t i) {
        std::size_t n = data_.size();
        T val = std::move(data_[i]);
        while (true) {
            std::size_t first = D * i + 1;
            if (first >= n) break;

            std::size_t best = first;
            std::size_t last = std::min(first + D, n);
            for (std::size_t c = first + 1; c < last; ++c) {
                if (data_[c] < data_[best]) {
                    best = c;
                }
            }

            if (data_[best] < val) {
                data_[i] = std::move(data_[best]);
                i = best;
            } else {
                break;
            }
        }
        data_[i] = std::move(val);
    }

public:
    BenchmarkDAryHeap() = default;

    void reserve(std::size_t cap) { data_.reserve(cap); }

    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }

    const T& top() const noexcept { return data_[0]; }

    void push(const T& val) {
        data_.push_back(val);
        sift_up(data_.size() - 1);
    }

    T pop() {
        T top_val = std::move(data_[0]);
        if (data_.size() == 1) {
            data_.pop_back();
            return top_val;
        }
        data_[0] = std::move(data_.back());
        data_.pop_back();
        sift_down(0);
        return top_val;
    }

    // Melds another d-ary heap by bulk copying and rebuilding in O(N)
    void meld(BenchmarkDAryHeap& other) {
        data_.insert(data_.end(), other.data_.begin(), other.data_.end());
        other.data_.clear();
        if (data_.size() > 1) {
            for (std::size_t i = (data_.size() - 2) / D + 1; i > 0; --i) {
                sift_down(i - 1);
            }
        }
    }
};

// ============================================================================
// 2. Benchmark Pairing Heap (Self-adjusting multiway tree)
// ============================================================================
template <typename T>
class BenchmarkPairingHeap {
public:
    struct Node {
        T val;
        Node* child;
        Node* sibling;

        explicit Node(T v) : val(v), child(nullptr), sibling(nullptr) {}
    };

private:
    Node* root_ = nullptr;
    std::size_t size_ = 0;
    std::vector<Node*> pass_buf_; // Reusable buffer to eliminate heap allocation during two-pass merge

    static Node* meld_nodes(Node* a, Node* b) noexcept {
        if (!a) return b;
        if (!b) return a;
        if (b->val < a->val) {
            std::swap(a, b);
        }
        b->sibling = a->child;
        a->child = b;
        return a;
    }

    Node* two_pass_merge(Node* first) {
        if (!first) return nullptr;
        if (!first->sibling) return first;

        pass_buf_.clear();
        Node* curr = first;
        while (curr) {
            Node* a = curr;
            Node* b = curr->sibling;
            if (b) {
                Node* next = b->sibling;
                a->sibling = nullptr;
                b->sibling = nullptr;
                pass_buf_.push_back(meld_nodes(a, b));
                curr = next;
            } else {
                a->sibling = nullptr;
                pass_buf_.push_back(a);
                curr = nullptr;
            }
        }

        Node* result = pass_buf_.back();
        for (int i = static_cast<int>(pass_buf_.size()) - 2; i >= 0; --i) {
            result = meld_nodes(pass_buf_[i], result);
        }
        return result;
    }

    static void destroy_tree(Node* n) noexcept {
        std::vector<Node*> stack;
        if (n) stack.push_back(n);
        while (!stack.empty()) {
            Node* curr = stack.back();
            stack.pop_back();
            if (curr->sibling) stack.push_back(curr->sibling);
            if (curr->child) stack.push_back(curr->child);
            delete curr;
        }
    }

public:
    BenchmarkPairingHeap() = default;

    ~BenchmarkPairingHeap() {
        destroy_tree(root_);
    }

    BenchmarkPairingHeap(const BenchmarkPairingHeap&) = delete;
    BenchmarkPairingHeap& operator=(const BenchmarkPairingHeap&) = delete;

    BenchmarkPairingHeap(BenchmarkPairingHeap&& other) noexcept
        : root_(other.root_), size_(other.size_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    BenchmarkPairingHeap& operator=(BenchmarkPairingHeap&& other) noexcept {
        if (this != &other) {
            destroy_tree(root_);
            root_ = other.root_;
            size_ = other.size_;
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    const T& top() const noexcept { return root_->val; }

    void push(const T& val) {
        Node* n = new Node(val);
        root_ = meld_nodes(root_, n);
        ++size_;
    }

    T pop() {
        T top_val = root_->val;
        Node* old_root = root_;
        Node* children = root_->child;
        delete old_root;
        root_ = two_pass_merge(children);
        --size_;
        return top_val;
    }

    // Melds another pairing heap in O(1) worst-case time
    void meld(BenchmarkPairingHeap& other) noexcept {
        if (this == &other || other.empty()) return;
        root_ = meld_nodes(root_, other.root_);
        size_ += other.size_;
        other.root_ = nullptr;
        other.size_ = 0;
    }
};

// ============================================================================
// 3. Structures for Decrease-Key / Dijkstra-Style Simulation
// ============================================================================

// (A) Indexed 4-ary Heap with position mapping
template <typename T, std::size_t D = 4>
class IndexedDAryHeap {
private:
    struct Entry {
        T priority;
        uint32_t id;
    };

    std::vector<Entry> data_;
    std::vector<int32_t> pos_; // pos_[id] = index in data_, -1 if not in heap

    void swap_entries(std::size_t i, std::size_t j) noexcept {
        std::swap(data_[i], data_[j]);
        pos_[data_[i].id] = static_cast<int32_t>(i);
        pos_[data_[j].id] = static_cast<int32_t>(j);
    }

    void sift_up(std::size_t i) {
        while (i > 0) {
            std::size_t p = (i - 1) / D;
            if (data_[i].priority < data_[p].priority) {
                swap_entries(i, p);
                i = p;
            } else {
                break;
            }
        }
    }

    void sift_down(std::size_t i) {
        std::size_t n = data_.size();
        while (true) {
            std::size_t first = D * i + 1;
            if (first >= n) break;

            std::size_t best = first;
            std::size_t last = std::min(first + D, n);
            for (std::size_t c = first + 1; c < last; ++c) {
                if (data_[c].priority < data_[best].priority) {
                    best = c;
                }
            }

            if (data_[best].priority < data_[i].priority) {
                swap_entries(i, best);
                i = best;
            } else {
                break;
            }
        }
    }

public:
    explicit IndexedDAryHeap(std::size_t max_ids) : pos_(max_ids, -1) {
        data_.reserve(max_ids);
    }

    [[nodiscard]] std::size_t size() const noexcept { return data_.size(); }
    [[nodiscard]] bool empty() const noexcept { return data_.empty(); }

    void push(uint32_t id, const T& priority) {
        std::size_t idx = data_.size();
        data_.push_back({priority, id});
        pos_[id] = static_cast<int32_t>(idx);
        sift_up(idx);
    }

    void decrease_key(uint32_t id, const T& new_priority) {
        int32_t idx = pos_[id];
        assert(idx >= 0 && "ID not present in heap");
        if (new_priority < data_[idx].priority) {
            data_[idx].priority = new_priority;
            sift_up(static_cast<std::size_t>(idx));
        }
    }

    std::pair<T, uint32_t> pop() {
        Entry top_val = data_[0];
        pos_[top_val.id] = -1;
        if (data_.size() == 1) {
            data_.pop_back();
            return {top_val.priority, top_val.id};
        }
        data_[0] = data_.back();
        pos_[data_[0].id] = 0;
        data_.pop_back();
        sift_down(0);
        return {top_val.priority, top_val.id};
    }
};

// (B) Handle-Based Pairing Heap with O(1) Decrease-Key Cuts
template <typename T>
class HandlePairingHeap {
public:
    struct Node {
        T priority;
        uint32_t id;
        Node* child = nullptr;
        Node* sibling = nullptr;
        Node* prev = nullptr; // Points to parent if first child, or left sibling

        explicit Node(T p, uint32_t i) : priority(p), id(i) {}
    };

private:
    Node* root_ = nullptr;
    std::size_t size_ = 0;
    std::vector<Node*> pass_buf_;

    static Node* meld_nodes(Node* a, Node* b) noexcept {
        if (!a) return b;
        if (!b) return a;
        if (b->priority < a->priority) {
            std::swap(a, b);
        }
        b->sibling = a->child;
        if (a->child) {
            a->child->prev = b;
        }
        b->prev = a;
        a->child = b;
        return a;
    }

    Node* two_pass_merge(Node* first) {
        if (!first) return nullptr;
        if (!first->sibling) {
            first->prev = nullptr;
            return first;
        }

        pass_buf_.clear();
        Node* curr = first;
        while (curr) {
            Node* a = curr;
            Node* b = curr->sibling;
            if (b) {
                Node* next = b->sibling;
                a->sibling = nullptr;
                a->prev = nullptr;
                b->sibling = nullptr;
                b->prev = nullptr;
                pass_buf_.push_back(meld_nodes(a, b));
                curr = next;
            } else {
                a->sibling = nullptr;
                a->prev = nullptr;
                pass_buf_.push_back(a);
                curr = nullptr;
            }
        }

        Node* result = pass_buf_.back();
        for (int i = static_cast<int>(pass_buf_.size()) - 2; i >= 0; --i) {
            result = meld_nodes(pass_buf_[i], result);
        }
        if (result) result->prev = nullptr;
        return result;
    }

public:
    HandlePairingHeap() = default;

    ~HandlePairingHeap() {
        std::vector<Node*> stack;
        if (root_) stack.push_back(root_);
        while (!stack.empty()) {
            Node* curr = stack.back();
            stack.pop_back();
            if (curr->sibling) stack.push_back(curr->sibling);
            if (curr->child) stack.push_back(curr->child);
            delete curr;
        }
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    Node* push(uint32_t id, const T& priority) {
        Node* n = new Node(priority, id);
        root_ = meld_nodes(root_, n);
        ++size_;
        return n;
    }

    void decrease_key(Node* node, const T& new_priority) {
        if (new_priority >= node->priority) return;
        node->priority = new_priority;
        if (node == root_) return;

        // Sever 'node' from parent or left sibling
        if (node->prev->child == node) {
            node->prev->child = node->sibling;
        } else {
            node->prev->sibling = node->sibling;
        }
        if (node->sibling) {
            node->sibling->prev = node->prev;
        }

        node->sibling = nullptr;
        node->prev = nullptr;
        root_ = meld_nodes(root_, node);
    }

    std::pair<T, uint32_t> pop() {
        T top_val = root_->priority;
        uint32_t top_id = root_->id;
        Node* old_root = root_;
        Node* children = root_->child;
        delete old_root;
        root_ = two_pass_merge(children);
        --size_;
        return {top_val, top_id};
    }
};

// ============================================================================
// Main Benchmark Suite
// ============================================================================
int main() {
    std::cout << "========================================================================================\n";
    std::cout << " HEAP FAMILY EMPIRICAL BENCHMARK (Linux Physical Hardware)\n";
    std::cout << " Contiguous Binary vs. 4-ary vs. 8-ary vs. Pointer-Based Pairing Heap\n";
    std::cout << "========================================================================================\n\n";

    constexpr std::size_t N = 1000000;      // 1 Million elements
    constexpr std::size_t STEADY_CYCLES = 1000000;
    constexpr std::size_t STEADY_INIT = 500000;
    constexpr std::size_t MELD_HEAPS = 1000;
    constexpr std::size_t MELD_SIZE = 1000;
    constexpr std::size_t DIJKSTRA_V = 100000;
    constexpr std::size_t DIJKSTRA_RELAX = 1000000;

    std::cout << ">> Generating " << N << " pseudo-random 64-bit keys...\n";
    FastRng rng(42);
    std::vector<uint64_t> test_keys(N);
    for (std::size_t i = 0; i < N; ++i) {
        test_keys[i] = rng.next();
    }

    auto to_ms = [](auto d) { return std::chrono::duration<double, std::milli>(d).count(); };

    // ------------------------------------------------------------------------
    // WORKLOAD 1 & 2: Bulk Insert & Extract-Min (N = 1,000,000)
    // ------------------------------------------------------------------------
    std::cout << "\n>> [Phase 1] Bulk Insert and Extract-Min (N = " << N << ")...\n";

    // 1A. std::priority_queue (Binary)
    malloc_trim(0);
    std::size_t rss_std_before = get_current_rss_bytes();
    auto t0 = std::chrono::high_resolution_clock::now();
    std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>> pq_std;
    for (std::size_t i = 0; i < N; ++i) {
        pq_std.push(test_keys[i]);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    std::size_t rss_std_after = get_current_rss_bytes();
    std::size_t rss_std = (rss_std_after > rss_std_before) ? (rss_std_after - rss_std_before) : 0;

    volatile uint64_t checksum_std = 0;
    auto t2 = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < N; ++i) {
        checksum_std += pq_std.top();
        pq_std.pop();
    }
    auto t3 = std::chrono::high_resolution_clock::now();

    double std_ins_ms = to_ms(t1 - t0);
    double std_pop_ms = to_ms(t3 - t2);

    // 1B. 4-ary Heap
    malloc_trim(0);
    std::size_t rss_d4_before = get_current_rss_bytes();
    t0 = std::chrono::high_resolution_clock::now();
    BenchmarkDAryHeap<uint64_t, 4> pq_d4;
    pq_d4.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        pq_d4.push(test_keys[i]);
    }
    t1 = std::chrono::high_resolution_clock::now();
    std::size_t rss_d4_after = get_current_rss_bytes();
    std::size_t rss_d4 = (rss_d4_after > rss_d4_before) ? (rss_d4_after - rss_d4_before) : 0;

    volatile uint64_t checksum_d4 = 0;
    t2 = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < N; ++i) {
        checksum_d4 += pq_d4.pop();
    }
    t3 = std::chrono::high_resolution_clock::now();

    double d4_ins_ms = to_ms(t1 - t0);
    double d4_pop_ms = to_ms(t3 - t2);

    // 1C. 8-ary Heap
    malloc_trim(0);
    std::size_t rss_d8_before = get_current_rss_bytes();
    t0 = std::chrono::high_resolution_clock::now();
    BenchmarkDAryHeap<uint64_t, 8> pq_d8;
    pq_d8.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        pq_d8.push(test_keys[i]);
    }
    t1 = std::chrono::high_resolution_clock::now();
    std::size_t rss_d8_after = get_current_rss_bytes();
    std::size_t rss_d8 = (rss_d8_after > rss_d8_before) ? (rss_d8_after - rss_d8_before) : 0;

    volatile uint64_t checksum_d8 = 0;
    t2 = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < N; ++i) {
        checksum_d8 += pq_d8.pop();
    }
    t3 = std::chrono::high_resolution_clock::now();

    double d8_ins_ms = to_ms(t1 - t0);
    double d8_pop_ms = to_ms(t3 - t2);

    // 1D. Pairing Heap
    malloc_trim(0);
    std::size_t rss_pair_before = get_current_rss_bytes();
    t0 = std::chrono::high_resolution_clock::now();
    BenchmarkPairingHeap<uint64_t> pq_pair;
    for (std::size_t i = 0; i < N; ++i) {
        pq_pair.push(test_keys[i]);
    }
    t1 = std::chrono::high_resolution_clock::now();
    std::size_t rss_pair_after = get_current_rss_bytes();
    std::size_t rss_pair = (rss_pair_after > rss_pair_before) ? (rss_pair_after - rss_pair_before) : 0;

    volatile uint64_t checksum_pair = 0;
    t2 = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < N; ++i) {
        checksum_pair += pq_pair.pop();
    }
    t3 = std::chrono::high_resolution_clock::now();

    double pair_ins_ms = to_ms(t1 - t0);
    double pair_pop_ms = to_ms(t3 - t2);

    assert(checksum_std == checksum_d4 && checksum_std == checksum_d8 && checksum_std == checksum_pair);

    // ------------------------------------------------------------------------
    // WORKLOAD 3: Steady-State Event Queue (1M Push/Pop cycles at 500k elements)
    // ------------------------------------------------------------------------
    std::cout << ">> [Phase 2] Steady-State Scheduler Simulation (1M cycles, capacity 500k)...\n";
    FastRng steady_rng(12345);
    std::vector<uint64_t> cycle_keys(STEADY_CYCLES);
    for (std::size_t i = 0; i < STEADY_CYCLES; ++i) {
        cycle_keys[i] = steady_rng.next();
    }

    // 3A. std::priority_queue
    std::priority_queue<uint64_t, std::vector<uint64_t>, std::greater<uint64_t>> sq_std;
    for (std::size_t i = 0; i < STEADY_INIT; ++i) sq_std.push(test_keys[i]);
    t0 = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < STEADY_CYCLES; ++i) {
        sq_std.push(cycle_keys[i]);
        volatile uint64_t v = sq_std.top();
        (void)v;
        sq_std.pop();
    }
    t1 = std::chrono::high_resolution_clock::now();
    double std_steady_ms = to_ms(t1 - t0);

    // 3B. 4-ary Heap
    BenchmarkDAryHeap<uint64_t, 4> sq_d4;
    sq_d4.reserve(STEADY_INIT + 2);
    for (std::size_t i = 0; i < STEADY_INIT; ++i) sq_d4.push(test_keys[i]);
    t0 = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < STEADY_CYCLES; ++i) {
        sq_d4.push(cycle_keys[i]);
        volatile uint64_t v = sq_d4.pop();
        (void)v;
    }
    t1 = std::chrono::high_resolution_clock::now();
    double d4_steady_ms = to_ms(t1 - t0);

    // 3C. 8-ary Heap
    BenchmarkDAryHeap<uint64_t, 8> sq_d8;
    sq_d8.reserve(STEADY_INIT + 2);
    for (std::size_t i = 0; i < STEADY_INIT; ++i) sq_d8.push(test_keys[i]);
    t0 = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < STEADY_CYCLES; ++i) {
        sq_d8.push(cycle_keys[i]);
        volatile uint64_t v = sq_d8.pop();
        (void)v;
    }
    t1 = std::chrono::high_resolution_clock::now();
    double d8_steady_ms = to_ms(t1 - t0);

    // 3D. Pairing Heap
    BenchmarkPairingHeap<uint64_t> sq_pair;
    for (std::size_t i = 0; i < STEADY_INIT; ++i) sq_pair.push(test_keys[i]);
    t0 = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < STEADY_CYCLES; ++i) {
        sq_pair.push(cycle_keys[i]);
        volatile uint64_t v = sq_pair.pop();
        (void)v;
    }
    t1 = std::chrono::high_resolution_clock::now();
    double pair_steady_ms = to_ms(t1 - t0);

    // ------------------------------------------------------------------------
    // WORKLOAD 4: Meld / Union (1,000 heaps of size 1,000)
    // ------------------------------------------------------------------------
    std::cout << ">> [Phase 3] Heap Meld / Union (1,000 heaps of 1,000 elements)...\n";
    // 4A. 4-ary Heap Meld
    std::vector<BenchmarkDAryHeap<uint64_t, 4>> d4_heaps(MELD_HEAPS);
    for (std::size_t h = 0; h < MELD_HEAPS; ++h) {
        d4_heaps[h].reserve(MELD_SIZE);
        for (std::size_t i = 0; i < MELD_SIZE; ++i) {
            d4_heaps[h].push(test_keys[(h * MELD_SIZE + i) % N]);
        }
    }
    t0 = std::chrono::high_resolution_clock::now();
    for (std::size_t h = 1; h < MELD_HEAPS; ++h) {
        d4_heaps[0].meld(d4_heaps[h]);
    }
    t1 = std::chrono::high_resolution_clock::now();
    double d4_meld_ms = to_ms(t1 - t0);

    // 4B. Pairing Heap Meld
    std::vector<BenchmarkPairingHeap<uint64_t>> pair_heaps(MELD_HEAPS);
    for (std::size_t h = 0; h < MELD_HEAPS; ++h) {
        for (std::size_t i = 0; i < MELD_SIZE; ++i) {
            pair_heaps[h].push(test_keys[(h * MELD_SIZE + i) % N]);
        }
    }
    t0 = std::chrono::high_resolution_clock::now();
    for (std::size_t h = 1; h < MELD_HEAPS; ++h) {
        pair_heaps[0].meld(pair_heaps[h]);
    }
    t1 = std::chrono::high_resolution_clock::now();
    double pair_meld_ms = to_ms(t1 - t0);

    // ------------------------------------------------------------------------
    // WORKLOAD 5: Dijkstra / Decrease-Key Simulation
    // 100,000 vertices, 1,000,000 edge relaxations
    // ------------------------------------------------------------------------
    std::cout << ">> [Phase 4] Dijkstra-Style Priority Decrease (100k nodes, 1M relaxations)...\n";
    FastRng d_rng(999);
    std::vector<std::pair<uint32_t, uint64_t>> relaxations(DIJKSTRA_RELAX);
    for (std::size_t i = 0; i < DIJKSTRA_RELAX; ++i) {
        uint32_t id = static_cast<uint32_t>(d_rng.next_range(DIJKSTRA_V));
        uint64_t new_dist = d_rng.next_range(50000000ULL);
        relaxations[i] = {id, new_dist};
    }

    // 5A. std::priority_queue (Lazy Deletion idiom)
    malloc_trim(0);
    std::size_t rss_dijk_std_before = get_current_rss_bytes();
    t0 = std::chrono::high_resolution_clock::now();
    std::vector<uint64_t> dist_std(DIJKSTRA_V, UINT64_MAX);
    std::priority_queue<std::pair<uint64_t, uint32_t>,
                        std::vector<std::pair<uint64_t, uint32_t>>,
                        std::greater<std::pair<uint64_t, uint32_t>>> lazy_pq;
    for (uint32_t v = 0; v < DIJKSTRA_V; ++v) {
        dist_std[v] = 100000000ULL + v;
        lazy_pq.push({dist_std[v], v});
    }
    for (std::size_t i = 0; i < DIJKSTRA_RELAX; ++i) {
        uint32_t v = relaxations[i].first;
        uint64_t d = relaxations[i].second;
        if (d < dist_std[v]) {
            dist_std[v] = d;
            lazy_pq.push({d, v}); // Push duplicate entry
        }
    }
    volatile uint64_t visited_std = 0;
    std::vector<bool> finalized_std(DIJKSTRA_V, false);
    while (!lazy_pq.empty()) {
        auto [d, v] = lazy_pq.top();
        lazy_pq.pop();
        if (finalized_std[v]) continue; // Discard stale entry
        finalized_std[v] = true;
        visited_std += d;
    }
    t1 = std::chrono::high_resolution_clock::now();
    std::size_t rss_dijk_std_after = get_current_rss_bytes();
    std::size_t rss_dijk_std = (rss_dijk_std_after > rss_dijk_std_before) ? (rss_dijk_std_after - rss_dijk_std_before) : 0;
    double dijk_std_ms = to_ms(t1 - t0);

    // 5B. Indexed 4-ary Heap (Explicit Sift-Up Decrease-Key)
    malloc_trim(0);
    std::size_t rss_dijk_d4_before = get_current_rss_bytes();
    t0 = std::chrono::high_resolution_clock::now();
    std::vector<uint64_t> dist_d4(DIJKSTRA_V, UINT64_MAX);
    IndexedDAryHeap<uint64_t, 4> indexed_d4(DIJKSTRA_V);
    for (uint32_t v = 0; v < DIJKSTRA_V; ++v) {
        dist_d4[v] = 100000000ULL + v;
        indexed_d4.push(v, dist_d4[v]);
    }
    for (std::size_t i = 0; i < DIJKSTRA_RELAX; ++i) {
        uint32_t v = relaxations[i].first;
        uint64_t d = relaxations[i].second;
        if (d < dist_d4[v]) {
            dist_d4[v] = d;
            indexed_d4.decrease_key(v, d);
        }
    }
    volatile uint64_t visited_d4 = 0;
    while (!indexed_d4.empty()) {
        auto [d, v] = indexed_d4.pop();
        visited_d4 += d;
        (void)v;
    }
    t1 = std::chrono::high_resolution_clock::now();
    std::size_t rss_dijk_d4_after = get_current_rss_bytes();
    std::size_t rss_dijk_d4 = (rss_dijk_d4_after > rss_dijk_d4_before) ? (rss_dijk_d4_after - rss_dijk_d4_before) : 0;
    double dijk_d4_ms = to_ms(t1 - t0);

    // 5C. Handle-Based Pairing Heap (O(1) Cut-and-Meld Decrease-Key)
    malloc_trim(0);
    std::size_t rss_dijk_pair_before = get_current_rss_bytes();
    t0 = std::chrono::high_resolution_clock::now();
    std::vector<uint64_t> dist_pair(DIJKSTRA_V, UINT64_MAX);
    HandlePairingHeap<uint64_t> handle_pair;
    std::vector<HandlePairingHeap<uint64_t>::Node*> handles(DIJKSTRA_V);
    for (uint32_t v = 0; v < DIJKSTRA_V; ++v) {
        dist_pair[v] = 100000000ULL + v;
        handles[v] = handle_pair.push(v, dist_pair[v]);
    }
    for (std::size_t i = 0; i < DIJKSTRA_RELAX; ++i) {
        uint32_t v = relaxations[i].first;
        uint64_t d = relaxations[i].second;
        if (d < dist_pair[v]) {
            dist_pair[v] = d;
            handle_pair.decrease_key(handles[v], d);
        }
    }
    volatile uint64_t visited_pair = 0;
    while (!handle_pair.empty()) {
        auto [d, v] = handle_pair.pop();
        visited_pair += d;
        (void)v;
    }
    t1 = std::chrono::high_resolution_clock::now();
    std::size_t rss_dijk_pair_after = get_current_rss_bytes();
    std::size_t rss_dijk_pair = (rss_dijk_pair_after > rss_dijk_pair_before) ? (rss_dijk_pair_after - rss_dijk_pair_before) : 0;
    double dijk_pair_ms = to_ms(t1 - t0);

    assert(visited_std == visited_d4 && visited_std == visited_pair);

    // ------------------------------------------------------------------------
    // OUTPUT RESULTS TABLE
    // ------------------------------------------------------------------------
    std::cout << "\n========================================================================================\n";
    std::cout << " EMPIRICAL BENCHMARK RESULTS SUMMARY\n";
    std::cout << "========================================================================================\n\n";

    std::cout << std::fixed << std::setprecision(2);

    std::cout << "### Part 1: Bulk Priority Queue Operations (N = 1,000,000 64-bit Keys)\n\n";
    std::cout << "| Metric | std::priority_queue (2-ary) | 4-ary Heap | 8-ary Heap | Pairing Heap (Pointer) |\n";
    std::cout << "| :--- | :---: | :---: | :---: | :---: |\n";
    std::cout << "| **Memory Layout** | Contiguous Array | Contiguous Array | Contiguous Array | Multi-way Node Tree |\n";
    std::cout << "| **Random Bulk Insert** | " << std_ins_ms << " ms (" << (N / (std_ins_ms / 1000.0) / 1e6) << " M/s) | "
              << d4_ins_ms << " ms (" << (N / (d4_ins_ms / 1000.0) / 1e6) << " M/s) | "
              << d8_ins_ms << " ms (" << (N / (d8_ins_ms / 1000.0) / 1e6) << " M/s) | "
              << pair_ins_ms << " ms (" << (N / (pair_ins_ms / 1000.0) / 1e6) << " M/s) |\n";
    std::cout << "| **Extract-Min (Drain)** | " << std_pop_ms << " ms (" << (std_pop_ms * 1e6 / N) << " ns/op) | "
              << d4_pop_ms << " ms (" << (d4_pop_ms * 1e6 / N) << " ns/op) | "
              << d8_pop_ms << " ms (" << (d8_pop_ms * 1e6 / N) << " ns/op) | "
              << pair_pop_ms << " ms (" << (pair_pop_ms * 1e6 / N) << " ns/op) |\n";
    std::cout << "| **Steady-State 1M Cycles** | " << std_steady_ms << " ms (" << (std_steady_ms * 1e6 / STEADY_CYCLES) << " ns/cyc) | "
              << d4_steady_ms << " ms (" << (d4_steady_ms * 1e6 / STEADY_CYCLES) << " ns/cyc) | "
              << d8_steady_ms << " ms (" << (d8_steady_ms * 1e6 / STEADY_CYCLES) << " ns/cyc) | "
              << pair_steady_ms << " ms (" << (pair_steady_ms * 1e6 / STEADY_CYCLES) << " ns/cyc) |\n";
    std::cout << "| **Allocated RAM (RSS)** | ~" << (rss_std / (1024 * 1024)) << " MB | ~"
              << (rss_d4 / (1024 * 1024)) << " MB | ~"
              << (rss_d8 / (1024 * 1024)) << " MB | ~"
              << (rss_pair / (1024 * 1024)) << " MB |\n";
    std::cout << "| **Bytes per Element** | ~" << (rss_std > 0 ? (rss_std / N) : 8) << " B | ~"
              << (rss_d4 > 0 ? (rss_d4 / N) : 8) << " B | ~"
              << (rss_d8 > 0 ? (rss_d8 / N) : 8) << " B | ~"
              << (rss_pair > 0 ? (rss_pair / N) : 24) << " B |\n\n";

    std::cout << "### Part 2: Priority Queue Melding / Union (1,000 Heaps of 1,000 Elements = 1,000,000 total)\n\n";
    std::cout << "| Metric | 4-ary Heap (Array Copy + Rebuild) | Pairing Heap (O(1) Pointer Swaps) |\n";
    std::cout << "| :--- | :---: | :---: |\n";
    std::cout << "| **Total Meld Time** | " << d4_meld_ms << " ms | " << pair_meld_ms << " ms |\n";
    std::cout << "| **Meld Speedup** | Baseline (1.0x) | **" << (d4_meld_ms / std::max(pair_meld_ms, 0.001)) << "x faster** |\n\n";

    std::cout << "### Part 3: Graph Dijkstra / Priority Decrease (100,000 Nodes, 1,000,000 Relaxations)\n\n";
    std::cout << "| Metric | std::priority_queue (Lazy Deletion) | Indexed 4-ary Heap (Sift-Up) | Handle Pairing Heap (Cut-and-Meld) |\n";
    std::cout << "| :--- | :---: | :---: | :---: |\n";
    std::cout << "| **Total SSSP Execution** | " << dijk_std_ms << " ms | " << dijk_d4_ms << " ms | " << dijk_pair_ms << " ms |\n";
    std::cout << "| **Peak RSS Memory** | ~" << (rss_dijk_std / (1024 * 1024)) << " MB | ~"
              << (rss_dijk_d4 / (1024 * 1024)) << " MB | ~"
              << (rss_dijk_pair / (1024 * 1024)) << " MB |\n";
    std::cout << "| **Throughput** | " << (DIJKSTRA_RELAX / (dijk_std_ms / 1000.0) / 1e6) << " M relax/s | "
              << (DIJKSTRA_RELAX / (dijk_d4_ms / 1000.0) / 1e6) << " M relax/s | "
              << (DIJKSTRA_RELAX / (dijk_pair_ms / 1000.0) / 1e6) << " M relax/s |\n";

    std::cout << "\n========================================================================================\n";
    std::cout << " BENCHMARK COMPLETE\n";
    std::cout << "========================================================================================\n";

    return 0;
}
