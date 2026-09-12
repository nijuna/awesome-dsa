/**
 * @file robin_hood_cuckoo_and_hopscotch_hashing.cpp
 * @brief Reference implementations of Robin Hood Hashing and Cuckoo Hashing.
 *
 * Implements:
 * 1. RobinHoodHashMap with PSL displacement swapping, early-exit search, and backward-shift deletion.
 * 2. CuckooHashMap with dual-table eviction chains and deterministic O(1) worst-case lookup.
 * 3. Arthur's Two-Layer API and differential testing against std::unordered_map.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <unordered_map>
#include <random>

namespace dsa {

// ============================================================================
// 1. Robin Hood Hash Map
// ============================================================================

template <typename Key, typename Value>
class RobinHoodHashMap {
private:
    struct Slot {
        Key key{};
        Value val{};
        uint32_t psl = 0;
        bool occupied = false;
    };

    size_t capacity_ = 16;
    size_t size_ = 0;
    std::vector<Slot> table_;
    static constexpr double MAX_LOAD_FACTOR = 0.85;

    size_t hash(const Key& k) const {
        return std::hash<Key>{}(k) & (capacity_ - 1);
    }

    void rehash(size_t new_cap) {
        std::vector<Slot> old_table = std::move(table_);
        capacity_ = new_cap;
        table_.assign(capacity_, Slot{});
        size_ = 0;

        for (const auto& slot : old_table) {
            if (slot.occupied) {
                insert(slot.key, slot.val);
            }
        }
    }

public:
    RobinHoodHashMap() : table_(capacity_) {}

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // --- Layer A: Fast Core API (Preconditioned) ---

    const Value& at(const Key& key) const {
        const Value* v = try_find(key);
        assert(v != nullptr);
        return *v;
    }

    // --- Layer B: Safe Adapter API ---

    const Value* try_find(const Key& key) const {
        if (empty()) return nullptr;
        size_t idx = hash(key);
        uint32_t current_psl = 0;

        while (true) {
            const auto& slot = table_[idx];
            if (!slot.occupied || current_psl > slot.psl) {
                // Early exit: key cannot exist beyond this point
                return nullptr;
            }
            if (slot.key == key) {
                return &slot.val;
            }
            idx = (idx + 1) & (capacity_ - 1);
            current_psl++;
        }
    }

    Value* try_find(const Key& key) {
        if (empty()) return nullptr;
        size_t idx = hash(key);
        uint32_t current_psl = 0;

        while (true) {
            auto& slot = table_[idx];
            if (!slot.occupied || current_psl > slot.psl) {
                return nullptr;
            }
            if (slot.key == key) {
                return &slot.val;
            }
            idx = (idx + 1) & (capacity_ - 1);
            current_psl++;
        }
    }

    void insert(Key key, Value val) {
        if (static_cast<double>(size_ + 1) / capacity_ > MAX_LOAD_FACTOR) {
            rehash(capacity_ * 2);
        }

        size_t idx = hash(key);
        uint32_t current_psl = 0;

        while (true) {
            auto& slot = table_[idx];
            if (!slot.occupied) {
                slot.key = std::move(key);
                slot.val = std::move(val);
                slot.psl = current_psl;
                slot.occupied = true;
                size_++;
                return;
            }

            if (slot.key == key) {
                slot.val = std::move(val);
                return;
            }

            if (current_psl > slot.psl) {
                // Steal from the rich: swap incoming item with current occupant
                std::swap(key, slot.key);
                std::swap(val, slot.val);
                std::swap(current_psl, slot.psl);
            }

            idx = (idx + 1) & (capacity_ - 1);
            current_psl++;
        }
    }

    bool erase(const Key& key) {
        if (empty()) return false;
        size_t idx = hash(key);
        uint32_t current_psl = 0;

        while (true) {
            auto& slot = table_[idx];
            if (!slot.occupied || current_psl > slot.psl) {
                return false;
            }
            if (slot.key == key) {
                // Backward-shift deletion without tombstones
                slot.occupied = false;
                size_--;

                size_t curr = idx;
                size_t next = (curr + 1) & (capacity_ - 1);

                while (table_[next].occupied && table_[next].psl > 0) {
                    table_[curr] = table_[next];
                    table_[curr].psl--;
                    table_[next].occupied = false;
                    curr = next;
                    next = (curr + 1) & (capacity_ - 1);
                }
                return true;
            }
            idx = (idx + 1) & (capacity_ - 1);
            current_psl++;
        }
    }
};

// ============================================================================
// 2. Cuckoo Hash Map
// ============================================================================

template <typename Key, typename Value>
class CuckooHashMap {
private:
    struct Entry {
        Key key{};
        Value val{};
        bool occupied = false;
    };

    size_t capacity_ = 16;
    size_t size_ = 0;
    std::vector<Entry> t1_;
    std::vector<Entry> t2_;
    uint64_t seed1_ = 1337;
    uint64_t seed2_ = 7331;
    static constexpr int MAX_LOOP = 50;

    size_t hash1(const Key& k) const {
        return (std::hash<Key>{}(k) ^ seed1_) & (capacity_ - 1);
    }

    size_t hash2(const Key& k) const {
        return (std::hash<Key>{}(k) ^ seed2_) & (capacity_ - 1);
    }

    void rehash(size_t new_cap) {
        std::vector<Entry> old1 = std::move(t1_);
        std::vector<Entry> old2 = std::move(t2_);
        capacity_ = new_cap;
        t1_.assign(capacity_, Entry{});
        t2_.assign(capacity_, Entry{});
        size_ = 0;
        seed1_ += 0x9e3779b97f4a7c15ULL;
        seed2_ += 0xbf58476d1ce4e5b9ULL;

        for (const auto& e : old1) {
            if (e.occupied) insert(e.key, e.val);
        }
        for (const auto& e : old2) {
            if (e.occupied) insert(e.key, e.val);
        }
    }

public:
    CuckooHashMap() : t1_(capacity_), t2_(capacity_) {}

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // --- Layer A: Fast Core API (Preconditioned) ---

    const Value& at(const Key& key) const {
        const Value* v = try_find(key);
        assert(v != nullptr);
        return *v;
    }

    // --- Layer B: Safe Adapter API ---

    // Guaranteed deterministic O(1) worst-case lookup: inspects exactly 2 slots
    const Value* try_find(const Key& key) const {
        if (empty()) return nullptr;
        size_t i1 = hash1(key);
        if (t1_[i1].occupied && t1_[i1].key == key) {
            return &t1_[i1].val;
        }
        size_t i2 = hash2(key);
        if (t2_[i2].occupied && t2_[i2].key == key) {
            return &t2_[i2].val;
        }
        return nullptr;
    }

    Value* try_find(const Key& key) {
        if (empty()) return nullptr;
        size_t i1 = hash1(key);
        if (t1_[i1].occupied && t1_[i1].key == key) {
            return &t1_[i1].val;
        }
        size_t i2 = hash2(key);
        if (t2_[i2].occupied && t2_[i2].key == key) {
            return &t2_[i2].val;
        }
        return nullptr;
    }

    void insert(Key key, Value val) {
        // 1. If key already present, update value in place
        if (Value* existing = try_find(key)) {
            *existing = std::move(val);
            return;
        }

        if (size_ >= capacity_ * 0.5) {
            rehash(capacity_ * 2);
        }

        // 2. Cuckoo eviction loop
        Key curr_key = std::move(key);
        Value curr_val = std::move(val);

        for (int loop = 0; loop < MAX_LOOP; ++loop) {
            size_t i1 = hash1(curr_key);
            if (!t1_[i1].occupied) {
                t1_[i1].key = std::move(curr_key);
                t1_[i1].val = std::move(curr_val);
                t1_[i1].occupied = true;
                size_++;
                return;
            }

            // Evict occupant from T1 to T2
            std::swap(curr_key, t1_[i1].key);
            std::swap(curr_val, t1_[i1].val);

            size_t i2 = hash2(curr_key);
            if (!t2_[i2].occupied) {
                t2_[i2].key = std::move(curr_key);
                t2_[i2].val = std::move(curr_val);
                t2_[i2].occupied = true;
                size_++;
                return;
            }

            // Evict occupant from T2 to T1 on next iteration
            std::swap(curr_key, t2_[i2].key);
            std::swap(curr_val, t2_[i2].val);
        }

        // Eviction loop exceeded: rehash table with new hash seeds
        rehash(capacity_ * 2);
        insert(std::move(curr_key), std::move(curr_val));
    }

    bool erase(const Key& key) {
        if (empty()) return false;
        size_t i1 = hash1(key);
        if (t1_[i1].occupied && t1_[i1].key == key) {
            t1_[i1].occupied = false;
            size_--;
            return true;
        }
        size_t i2 = hash2(key);
        if (t2_[i2].occupied && t2_[i2].key == key) {
            t2_[i2].occupied = false;
            size_--;
            return true;
        }
        return false;
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Robin Hood and Cuckoo Hashing verification..." << std::endl;

    dsa::RobinHoodHashMap<int, int> rh;
    dsa::CuckooHashMap<int, int> cuckoo;
    std::unordered_map<int, int> oracle;

    std::mt19937 rng(42);
    const int OPS = 1000;

    for (int step = 0; step < OPS; ++step) {
        int op = rng() % 3;
        int key = rng() % 300;

        if (op == 0) {
            // Insert
            int val = key * 13;
            rh.insert(key, val);
            cuckoo.insert(key, val);
            oracle[key] = val;
        } else if (op == 1) {
            // Erase
            bool r_rh = rh.erase(key);
            bool r_ck = cuckoo.erase(key);
            bool r_or = oracle.erase(key) > 0;
            assert(r_rh == r_or);
            assert(r_ck == r_or);
        } else {
            // Find
            const int* v_rh = rh.try_find(key);
            const int* v_ck = cuckoo.try_find(key);
            auto it = oracle.find(key);
            if (it == oracle.end()) {
                assert(v_rh == nullptr);
                assert(v_ck == nullptr);
            } else {
                assert(v_rh != nullptr && *v_rh == it->second);
                assert(v_ck != nullptr && *v_ck == it->second);
            }
        }

        assert(rh.size() == oracle.size());
        assert(cuckoo.size() == oracle.size());
    }

    std::cout << "[PASS] 1000 differential operations on Robin Hood hash map matched std::unordered_map." << std::endl;
    std::cout << "[PASS] 1000 differential operations on Cuckoo hash map matched std::unordered_map." << std::endl;
    std::cout << "All Robin Hood and Cuckoo Hashing assertions passed successfully!" << std::endl;
    return 0;
}
