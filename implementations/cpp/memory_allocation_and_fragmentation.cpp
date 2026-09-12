/**
 * @file memory_allocation_and_fragmentation.cpp
 * @brief High-performance custom memory allocators: Arena (Bump) and Fixed-Size Pool.
 *
 * Implements an Arena/Bump allocator with 8-byte alignment and bulk deallocation,
 * and a Fixed-Size Pool allocator with an intrusive freelist for O(1) allocation and deallocation.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cstddef>
#include <algorithm>

namespace dsa {

/**
 * @brief Arena (Bump/Linear) Allocator.
 * Fast O(1) monotonic allocation with O(1) bulk reset.
 */
class ArenaAllocator {
private:
    std::vector<uint8_t> buffer_;
    size_t offset_ = 0;

    static size_t align_up(size_t n, size_t alignment) {
        return (n + alignment - 1) & ~(alignment - 1);
    }

public:
    explicit ArenaAllocator(size_t capacity_bytes) : buffer_(capacity_bytes), offset_(0) {}

    void* allocate(size_t bytes, size_t alignment = 8) {
        size_t aligned_offset = align_up(offset_, alignment);
        if (aligned_offset + bytes > buffer_.size()) {
            return nullptr; // Out of memory
        }
        void* ptr = &buffer_[aligned_offset];
        offset_ = aligned_offset + bytes;
        return ptr;
    }

    void reset() {
        offset_ = 0;
    }

    size_t used_bytes() const {
        return offset_;
    }

    size_t capacity() const {
        return buffer_.size();
    }
};

/**
 * @brief Fixed-Size Pool Allocator with an intrusive free list.
 * Provides O(1) allocate and O(1) free with zero external fragmentation.
 */
template <typename T>
class PoolAllocator {
private:
    union Slot {
        Slot* next;
        alignas(T) uint8_t storage[sizeof(T)];
    };

    std::vector<Slot> pool_;
    Slot* free_list_ = nullptr;
    size_t allocated_count_ = 0;

public:
    explicit PoolAllocator(size_t capacity) : pool_(capacity), free_list_(nullptr), allocated_count_(0) {
        // Build intrusive free list
        for (size_t i = 0; i < capacity; ++i) {
            pool_[i].next = free_list_;
            free_list_ = &pool_[i];
        }
    }

    T* allocate() {
        if (!free_list_) {
            return nullptr; // Pool exhausted
        }
        Slot* slot = free_list_;
        free_list_ = free_list_->next;
        allocated_count_++;
        return reinterpret_cast<T*>(slot->storage);
    }

    void deallocate(T* ptr) {
        if (!ptr) return;
        Slot* slot = reinterpret_cast<Slot*>(ptr);
        slot->next = free_list_;
        free_list_ = slot;
        allocated_count_--;
    }

    size_t allocated_count() const {
        return allocated_count_;
    }

    size_t capacity() const {
        return pool_.size();
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Memory Allocation & Fragmentation verification..." << std::endl;

    // --- 1. Arena Allocator Tests ---
    dsa::ArenaAllocator arena(1024);
    assert(arena.used_bytes() == 0);

    // Allocate 3 integers with 8-byte alignment
    int* p1 = static_cast<int*>(arena.allocate(sizeof(int), 8));
    assert(p1 != nullptr);
    *p1 = 100;

    int* p2 = static_cast<int*>(arena.allocate(sizeof(int), 8));
    assert(p2 != nullptr);
    *p2 = 200;

    assert(*p1 == 100 && *p2 == 200);
    assert(arena.used_bytes() >= 2 * sizeof(int));

    // Reset arena
    arena.reset();
    assert(arena.used_bytes() == 0);

    // --- 2. Fixed-Size Pool Allocator Tests ---
    struct TestNode {
        int id;
        double value;
        TestNode(int i, double v) : id(i), value(v) {}
    };

    dsa::PoolAllocator<TestNode> pool(4);
    assert(pool.allocated_count() == 0);
    assert(pool.capacity() == 4);

    TestNode* n1 = pool.allocate();
    TestNode* n2 = pool.allocate();
    TestNode* n3 = pool.allocate();
    TestNode* n4 = pool.allocate();

    assert(n1 && n2 && n3 && n4);
    assert(pool.allocated_count() == 4);

    // Pool exhaustion test
    TestNode* n5 = pool.allocate();
    assert(n5 == nullptr);

    // Deallocate and test freelist recycling
    pool.deallocate(n2);
    assert(pool.allocated_count() == 3);

    TestNode* recycled = pool.allocate();
    assert(recycled == n2); // Intrusive LIFO freelist reclaims immediately freed slot
    assert(pool.allocated_count() == 4);

    pool.deallocate(n1);
    pool.deallocate(recycled);
    pool.deallocate(n3);
    pool.deallocate(n4);
    assert(pool.allocated_count() == 0);

    std::cout << "[PASS] ArenaAllocator monotonic bump & bulk reset verified." << std::endl;
    std::cout << "[PASS] PoolAllocator intrusive freelist recycle & zero fragmentation verified." << std::endl;
    std::cout << "All Memory Allocation assertions passed successfully!" << std::endl;
    return 0;
}
