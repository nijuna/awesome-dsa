/**
 * Benchmark Harness: Separate Chaining (std::unordered_map) vs. Open Addressing (Linear Probing)
 * Evaluates memory consumption, insertion throughput, and random lookup latency under real entropy.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <iomanip>
#include <cstddef>
#include <cstdint>
#include <random>

class LinearProbingMap {
private:
    struct Entry {
        uint64_t key;
        uint64_t val;
        bool occupied;
        Entry() : key(0), val(0), occupied(false) {}
    };

    std::vector<Entry> table_;
    std::size_t capacity_;
    std::size_t mask_;
    std::size_t size_;

    static std::size_t hash_fn(uint64_t x) noexcept {
        // Fast SplitMix64 finalizer
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebULL;
        x ^= x >> 31;
        return static_cast<std::size_t>(x);
    }

public:
    explicit LinearProbingMap(std::size_t initial_cap = 16)
        : capacity_(initial_cap), mask_(initial_cap - 1), size_(0) {
        table_.resize(capacity_);
    }

    void reserve(std::size_t num_elements) {
        std::size_t target_cap = 16;
        while (target_cap * 0.65 < num_elements) {
            target_cap *= 2;
        }
        if (target_cap > capacity_) {
            rehash(target_cap);
        }
    }

    void rehash(std::size_t new_cap) {
        std::vector<Entry> old_table = std::move(table_);
        capacity_ = new_cap;
        mask_ = new_cap - 1;
        table_.assign(capacity_, Entry());
        size_ = 0;

        for (const auto& entry : old_table) {
            if (entry.occupied) {
                insert(entry.key, entry.val);
            }
        }
    }

    void insert(uint64_t key, uint64_t val) {
        if (size_ * 100 >= capacity_ * 65) { // 65% load factor threshold
            rehash(capacity_ * 2);
        }
        std::size_t idx = hash_fn(key) & mask_;
        while (table_[idx].occupied) {
            if (table_[idx].key == key) {
                table_[idx].val = val;
                return;
            }
            idx = (idx + 1) & mask_;
        }
        table_[idx].key = key;
        table_[idx].val = val;
        table_[idx].occupied = true;
        ++size_;
    }

    bool find(uint64_t key, uint64_t& out_val) const noexcept {
        std::size_t idx = hash_fn(key) & mask_;
        while (table_[idx].occupied) {
            if (table_[idx].key == key) {
                out_val = table_[idx].val;
                return true;
            }
            idx = (idx + 1) & mask_;
        }
        return false;
    }

    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] std::size_t size() const noexcept { return size_; }
};

int main() {
    const std::size_t N = 3000000; // 3 Million keys with random entropy
    std::cout << "===================================================================\n";
    std::cout << " BENCHMARK: Separate Chaining vs. Open Addressing (N = " << N << ")\n";
    std::cout << " Random 64-bit Keys (Measuring Cache Misses & Memory Overhead)\n";
    std::cout << "===================================================================\n\n";

    // Generate keys
    std::vector<uint64_t> keys(N);
    std::mt19937_64 rng(1337);
    for (std::size_t i = 0; i < N; ++i) {
        keys[i] = rng();
    }

    // 1. Benchmark Separate Chaining (std::unordered_map)
    auto start_chain_insert = std::chrono::high_resolution_clock::now();
    std::unordered_map<uint64_t, uint64_t> chain_map;
    chain_map.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        chain_map[keys[i]] = keys[i] ^ 0xDEADBEEF;
    }
    auto end_chain_insert = std::chrono::high_resolution_clock::now();

    volatile uint64_t chain_sum = 0;
    auto start_chain_find = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < N; ++i) {
        auto it = chain_map.find(keys[i]);
        if (it != chain_map.end()) {
            chain_sum += it->second;
        }
    }
    auto end_chain_find = std::chrono::high_resolution_clock::now();

    // 2. Benchmark Open Addressing (LinearProbingMap)
    auto start_oa_insert = std::chrono::high_resolution_clock::now();
    LinearProbingMap oa_map;
    oa_map.reserve(N);
    for (std::size_t i = 0; i < N; ++i) {
        oa_map.insert(keys[i], keys[i] ^ 0xDEADBEEF);
    }
    auto end_oa_insert = std::chrono::high_resolution_clock::now();

    volatile uint64_t oa_sum = 0;
    auto start_oa_find = std::chrono::high_resolution_clock::now();
    for (std::size_t i = 0; i < N; ++i) {
        uint64_t val = 0;
        if (oa_map.find(keys[i], val)) {
            oa_sum += val;
        }
    }
    auto end_oa_find = std::chrono::high_resolution_clock::now();

    // Verification
    if (chain_sum != oa_sum) {
        std::cerr << "Error: Checksum mismatch!\n";
        return 1;
    }

    // Durations
    double chain_insert_ms = std::chrono::duration<double, std::milli>(end_chain_insert - start_chain_insert).count();
    double chain_find_ms = std::chrono::duration<double, std::milli>(end_chain_find - start_chain_find).count();

    double oa_insert_ms = std::chrono::duration<double, std::milli>(end_oa_insert - start_oa_insert).count();
    double oa_find_ms = std::chrono::duration<double, std::milli>(end_oa_find - start_oa_find).count();

    // Memory footprint:
    // std::unordered_map: each node is ~32 bytes + bucket pointers
    std::size_t chain_bytes = N * 32 + (chain_map.bucket_count() * sizeof(void*));
    std::size_t oa_bytes = oa_map.capacity() * sizeof(uint64_t) * 3; // key + val + occupied (aligned)

    std::cout << std::fixed << std::setprecision(2);
    std::cout << "| Metric | Separate Chaining (std::unordered_map) | Open Addressing (Linear Probing) | Open Addressing Advantage |\n";
    std::cout << "| :--- | :--- | :--- | :---: |\n";
    std::cout << "| Memory Footprint (Estimated) | " << (chain_bytes / (1024 * 1024)) << " MB | " 
              << (oa_bytes / (1024 * 1024)) << " MB | **" << (static_cast<double>(chain_bytes) / oa_bytes) << "x less RAM** |\n";
    std::cout << "| Insertion Time (3M keys) | " << chain_insert_ms << " ms | " << oa_insert_ms << " ms | **"
              << (chain_insert_ms / oa_insert_ms) << "x faster insertion** |\n";
    std::cout << "| Lookup Time (3M hits) | " << chain_find_ms << " ms | " << oa_find_ms << " ms | **"
              << (chain_find_ms / oa_find_ms) << "x faster (contiguous cache lines)** |\n";

    return 0;
}
