/**
 * Reference Implementation: Open Addressing Hash Map (Linear Probing with Tombstones)
 * Demonstrates contiguous memory layout, power-of-two bitwise masking,
 * tombstone slot lifecycle, dynamic rehashing, and clean RAII.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <cstddef>
#include <cstdint>
#include <cassert>
#include <optional>
#include <stdexcept>
#include <string>

template <typename Key, typename Value>
class HashTable {
public:
    enum class State : uint8_t { EMPTY, OCCUPIED, DELETED };

    struct Entry {
        Key key;
        Value val;
        State state;
        Entry() : key{}, val{}, state(State::EMPTY) {}
    };

private:
    std::vector<Entry> table_;
    std::size_t capacity_;
    std::size_t mask_;
    std::size_t size_;
    std::size_t occupied_slots_; // includes DELETED for load factor tracking

    static std::size_t hash_key(const Key& k) noexcept {
        // Standard hash combined with 64-bit mixer
        std::size_t h = std::hash<Key>{}(k);
        uint64_t x = static_cast<uint64_t>(h);
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebULL;
        x ^= x >> 31;
        return static_cast<std::size_t>(x);
    }

    void rehash(std::size_t new_cap) {
        std::vector<Entry> old_table = std::move(table_);
        capacity_ = new_cap;
        mask_ = new_cap - 1;
        table_.assign(capacity_, Entry());
        size_ = 0;
        occupied_slots_ = 0;

        for (auto& entry : old_table) {
            if (entry.state == State::OCCUPIED) {
                insert(entry.key, entry.val);
            }
        }
    }

public:
    explicit HashTable(std::size_t initial_cap = 16)
        : capacity_(initial_cap < 8 ? 8 : initial_cap),
          mask_(capacity_ - 1),
          size_(0),
          occupied_slots_(0) {
        // Enforce power-of-two
        while ((capacity_ & (capacity_ - 1)) != 0) {
            capacity_ = (capacity_ | (capacity_ - 1)) + 1;
        }
        mask_ = capacity_ - 1;
        table_.resize(capacity_);
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    bool insert(const Key& key, const Value& val) {
        // Rehash if total occupied slots (including tombstones) exceed 70%
        if ((occupied_slots_ + 1) * 10 >= capacity_ * 7) {
            rehash(capacity_ * 2);
        }

        std::size_t idx = hash_key(key) & mask_;
        std::size_t first_deleted_idx = capacity_;

        while (table_[idx].state != State::EMPTY) {
            if (table_[idx].state == State::OCCUPIED) {
                if (table_[idx].key == key) {
                    table_[idx].val = val; // Update existing
                    return false;
                }
            } else if (table_[idx].state == State::DELETED && first_deleted_idx == capacity_) {
                first_deleted_idx = idx; // Remember first reusable tombstone slot
            }
            idx = (idx + 1) & mask_;
        }

        // Target slot is first tombstone encountered, or the trailing empty slot
        std::size_t target_slot = (first_deleted_idx != capacity_) ? first_deleted_idx : idx;
        if (table_[target_slot].state != State::DELETED) {
            ++occupied_slots_;
        }
        table_[target_slot].key = key;
        table_[target_slot].val = val;
        table_[target_slot].state = State::OCCUPIED;
        ++size_;
        return true;
    }

    std::optional<Value> find(const Key& key) const noexcept {
        std::size_t idx = hash_key(key) & mask_;
        std::size_t probes = 0;

        while (table_[idx].state != State::EMPTY && probes < capacity_) {
            if (table_[idx].state == State::OCCUPIED && table_[idx].key == key) {
                return table_[idx].val;
            }
            idx = (idx + 1) & mask_;
            ++probes;
        }
        return std::nullopt;
    }

    bool erase(const Key& key) noexcept {
        std::size_t idx = hash_key(key) & mask_;
        std::size_t probes = 0;

        while (table_[idx].state != State::EMPTY && probes < capacity_) {
            if (table_[idx].state == State::OCCUPIED && table_[idx].key == key) {
                table_[idx].state = State::DELETED; // Mark tombstone
                --size_;
                return true;
            }
            idx = (idx + 1) & mask_;
            ++probes;
        }
        return false;
    }

    void clear() noexcept {
        table_.assign(capacity_, Entry());
        size_ = 0;
        occupied_slots_ = 0;
    }
};

void run_tests() {
    HashTable<std::string, int> ht(8);
    assert(ht.empty());
    assert(ht.size() == 0);

    // Insert key-values
    assert(ht.insert("alpha", 100));
    assert(ht.insert("beta", 200));
    assert(ht.insert("gamma", 300));
    assert(ht.size() == 3);

    // Lookup
    assert(ht.find("alpha").value() == 100);
    assert(ht.find("beta").value() == 200);
    assert(ht.find("gamma").value() == 300);
    assert(!ht.find("delta").has_value());

    // Update existing key
    assert(!ht.insert("alpha", 999));
    assert(ht.size() == 3);
    assert(ht.find("alpha").value() == 999);

    // Tombstone deletion test
    assert(ht.erase("beta"));
    assert(ht.size() == 2);
    assert(!ht.find("beta").has_value());
    // gamma must still be found despite beta being a tombstone
    assert(ht.find("gamma").value() == 300);

    // Re-inserting into tombstone
    assert(ht.insert("beta", 400));
    assert(ht.size() == 3);
    assert(ht.find("beta").value() == 400);

    // Dynamic rehashing stress test
    for (int i = 0; i < 100; ++i) {
        ht.insert("key_" + std::to_string(i), i * 10);
    }
    assert(ht.size() >= 100);
    for (int i = 0; i < 100; ++i) {
        assert(ht.find("key_" + std::to_string(i)).value() == i * 10);
    }

    std::cout << "[PASS] All HashTable C++ unit tests passed.\n";
}

int main() {
    run_tests();
    return 0;
}
