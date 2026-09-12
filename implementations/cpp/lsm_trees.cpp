/**
 * @file lsm_trees.cpp
 * @brief High-performance C++17 reference implementation of a Log-Structured Merge-Tree (LSM-Tree).
 *
 * Implements modern Leveled Compaction (RocksDB / LevelDB architecture):
 * 1. MemTable: In-memory ordered buffer (std::map) for fast sequential and random writes.
 * 2. Write-Ahead Log (WAL): Simulated append-only log providing crash durability.
 * 3. Immutable SSTables (Sorted String Tables):
 *    - Sorted key-value pairs.
 *    - Sparse block index for fast binary search without loading entire file.
 *    - Bloom filter with double-hashing for O(1) negative lookup pruning.
 *    - Explicit tombstones for deletion semantics.
 * 4. Leveled Compaction Engine:
 *    - Level 0: Holds flushed SSTables with potentially overlapping key ranges.
 *    - Levels 1+: Strictly non-overlapping key ranges per level.
 *    - K-way merge compaction deduplicating keys and purging expired tombstones at bottom level.
 * 5. Lookup Priority Order:
 *    MemTable -> L0 (newest to oldest) -> L1+ (binary search key range routing).
 *
 * Fully compliant with -std=c++17 -O3 -Wall -Wextra -Werror.
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <optional>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <memory>
#include <random>

namespace dsa {

/**
 * @brief Bloom Filter for fast negative lookups.
 */
class BloomFilter {
private:
    std::vector<uint64_t> bits_;
    size_t num_bits_{0};
    size_t num_hashes_{0};

    static uint64_t fnv1a(const std::string& key, uint64_t seed) noexcept {
        uint64_t hash = 14695981039346656037ULL ^ seed;
        for (char c : key) {
            hash ^= static_cast<uint8_t>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

public:
    BloomFilter() = default;

    BloomFilter(size_t expected_keys, double fpp = 0.01) {
        if (expected_keys == 0) expected_keys = 1;
        double m = -static_cast<double>(expected_keys) * std::log(fpp) / (std::log(2.0) * std::log(2.0));
        num_bits_ = std::max(size_t(64), static_cast<size_t>(std::ceil(m)));
        num_hashes_ = std::max(size_t(1), static_cast<size_t>(std::round((num_bits_ / static_cast<double>(expected_keys)) * std::log(2.0))));

        bits_.assign((num_bits_ + 63) / 64, 0);
    }

    void add(const std::string& key) {
        if (num_bits_ == 0) return;
        uint64_t h1 = fnv1a(key, 0);
        uint64_t h2 = fnv1a(key, 0x9e3779b97f4a7c15ULL);
        for (size_t i = 0; i < num_hashes_; ++i) {
            uint64_t combined = h1 + i * h2;
            size_t bit_idx = combined % num_bits_;
            bits_[bit_idx / 64] |= (1ULL << (bit_idx % 64));
        }
    }

    [[nodiscard]] bool may_contain(const std::string& key) const noexcept {
        if (num_bits_ == 0) return true;
        uint64_t h1 = fnv1a(key, 0);
        uint64_t h2 = fnv1a(key, 0x9e3779b97f4a7c15ULL);
        for (size_t i = 0; i < num_hashes_; ++i) {
            uint64_t combined = h1 + i * h2;
            size_t bit_idx = combined % num_bits_;
            if (!((bits_[bit_idx / 64] >> (bit_idx % 64)) & 1ULL)) {
                return false;
            }
        }
        return true;
    }
};

/**
 * @brief Key-Value entry supporting deletion tombstones.
 */
struct Entry {
    std::string key;
    std::string value;
    bool is_tombstone{false};

    bool operator<(const Entry& other) const noexcept {
        return key < other.key;
    }
};

/**
 * @brief Immutable SSTable (Sorted String Table) component.
 */
class SSTable {
public:
    struct IndexEntry {
        std::string key;
        size_t offset;
    };

private:
    std::vector<Entry> entries_;
    std::vector<IndexEntry> sparse_index_;
    BloomFilter bloom_;
    std::string min_key_;
    std::string max_key_;
    static constexpr size_t INDEX_INTERVAL = 4; // Sample every 4 entries

public:
    explicit SSTable(std::vector<Entry> entries) : entries_(std::move(entries)) {
        assert(!entries_.empty());
        std::sort(entries_.begin(), entries_.end());

        min_key_ = entries_.front().key;
        max_key_ = entries_.back().key;

        bloom_ = BloomFilter(entries_.size(), 0.01);
        for (size_t i = 0; i < entries_.size(); ++i) {
            bloom_.add(entries_[i].key);
            if (i % INDEX_INTERVAL == 0) {
                sparse_index_.push_back({entries_[i].key, i});
            }
        }
    }

    [[nodiscard]] const std::string& min_key() const noexcept { return min_key_; }
    [[nodiscard]] const std::string& max_key() const noexcept { return max_key_; }
    [[nodiscard]] const std::vector<Entry>& entries() const noexcept { return entries_; }
    [[nodiscard]] size_t size() const noexcept { return entries_.size(); }

    [[nodiscard]] bool overlaps_with(const std::string& low, const std::string& high) const noexcept {
        return !(max_key_ < low || min_key_ > high);
    }

    [[nodiscard]] std::optional<Entry> get(const std::string& key) const {
        if (key < min_key_ || key > max_key_) return std::nullopt;
        if (!bloom_.may_contain(key)) return std::nullopt;

        if (sparse_index_.empty()) return std::nullopt;

        size_t low = 0, high = sparse_index_.size() - 1, block_idx = 0;
        while (low <= high) {
            size_t mid = low + (high - low) / 2;
            if (sparse_index_[mid].key <= key) {
                block_idx = mid;
                low = mid + 1;
            } else {
                if (mid == 0) break;
                high = mid - 1;
            }
        }

        size_t start_offset = sparse_index_[block_idx].offset;
        size_t end_offset = (block_idx + 1 < sparse_index_.size()) ? sparse_index_[block_idx + 1].offset : entries_.size();

        for (size_t i = start_offset; i < end_offset; ++i) {
            if (entries_[i].key == key) {
                return entries_[i];
            }
            if (entries_[i].key > key) break;
        }

        return std::nullopt;
    }
};

/**
 * @brief LSM-Tree implementing Leveled Compaction.
 */
class LSMTree {
public:
    static constexpr size_t MEMTABLE_THRESHOLD = 8;
    static constexpr size_t L0_COMPACTION_THRESHOLD = 4;
    static constexpr size_t MAX_LEVELS = 4;

private:
    std::map<std::string, Entry> memtable_;
    std::vector<std::shared_ptr<SSTable>> levels_[MAX_LEVELS];
    std::vector<std::string> wal_log_;

    void flush_memtable() {
        if (memtable_.empty()) return;

        std::vector<Entry> entries;
        entries.reserve(memtable_.size());
        for (auto& kv : memtable_) {
            entries.push_back(kv.second);
        }

        auto sstable = std::make_shared<SSTable>(std::move(entries));
        levels_[0].insert(levels_[0].begin(), sstable);
        memtable_.clear();
        wal_log_.clear();

        if (levels_[0].size() >= L0_COMPACTION_THRESHOLD) {
            compact_level(0);
        }
    }

    void compact_level(size_t level) {
        if (level >= MAX_LEVELS - 1) return;

        std::vector<std::shared_ptr<SSTable>> tables_to_compact;
        std::string low_key, high_key;

        if (level == 0) {
            tables_to_compact = levels_[0];
            low_key = tables_to_compact.front()->min_key();
            high_key = tables_to_compact.front()->max_key();
            for (auto& t : tables_to_compact) {
                low_key = std::min(low_key, t->min_key());
                high_key = std::max(high_key, t->max_key());
            }
        } else {
            if (levels_[level].empty()) return;
            auto pick = levels_[level].front();
            tables_to_compact.push_back(pick);
            low_key = pick->min_key();
            high_key = pick->max_key();
        }

        size_t next_level = level + 1;
        std::vector<std::shared_ptr<SSTable>> next_overlapping;
        std::vector<std::shared_ptr<SSTable>> next_retained;

        for (auto& t : levels_[next_level]) {
            if (t->overlaps_with(low_key, high_key)) {
                next_overlapping.push_back(t);
            } else {
                next_retained.push_back(t);
            }
        }

        std::map<std::string, Entry> merged_entries;

        // Older level first
        for (auto& t : next_overlapping) {
            for (const auto& e : t->entries()) {
                merged_entries[e.key] = e;
            }
        }

        // Newer level overrides
        for (auto it = tables_to_compact.rbegin(); it != tables_to_compact.rend(); ++it) {
            for (const auto& e : (*it)->entries()) {
                merged_entries[e.key] = e;
            }
        }

        bool is_bottom_level = (next_level == MAX_LEVELS - 1);
        std::vector<Entry> final_entries;
        final_entries.reserve(merged_entries.size());
        for (auto& kv : merged_entries) {
            if (is_bottom_level && kv.second.is_tombstone) {
                continue; // Discard expired tombstone at bottom level
            }
            final_entries.push_back(kv.second);
        }

        if (level == 0) {
            levels_[0].clear();
        } else {
            levels_[level].erase(levels_[level].begin());
        }

        std::vector<std::shared_ptr<SSTable>> new_next_tables;
        if (!final_entries.empty()) {
            size_t chunk_size = MEMTABLE_THRESHOLD * 2;
            for (size_t i = 0; i < final_entries.size(); i += chunk_size) {
                size_t end = std::min(i + chunk_size, final_entries.size());
                std::vector<Entry> chunk(final_entries.begin() + i, final_entries.begin() + end);
                new_next_tables.push_back(std::make_shared<SSTable>(std::move(chunk)));
            }
        }

        next_retained.insert(next_retained.end(), new_next_tables.begin(), new_next_tables.end());
        std::sort(next_retained.begin(), next_retained.end(), [](const auto& a, const auto& b) {
            return a->min_key() < b->min_key();
        });

        levels_[next_level] = std::move(next_retained);

        size_t capacity = L0_COMPACTION_THRESHOLD * (1 << next_level);
        if (levels_[next_level].size() > capacity && next_level < MAX_LEVELS - 1) {
            compact_level(next_level);
        }
    }

public:
    LSMTree() = default;

    void put(const std::string& key, const std::string& value) {
        wal_log_.push_back("PUT:" + key + "=" + value);
        memtable_[key] = {key, value, false};
        if (memtable_.size() >= MEMTABLE_THRESHOLD) {
            flush_memtable();
        }
    }

    void remove(const std::string& key) {
        wal_log_.push_back("DEL:" + key);
        memtable_[key] = {key, "", true};
        if (memtable_.size() >= MEMTABLE_THRESHOLD) {
            flush_memtable();
        }
    }

    [[nodiscard]] std::optional<std::string> get(const std::string& key) {
        // Priority 1: MemTable
        auto it = memtable_.find(key);
        if (it != memtable_.end()) {
            if (it->second.is_tombstone) return std::nullopt;
            return it->second.value;
        }

        // Priority 2: Level 0 (ordered newest to oldest)
        for (auto& sstable : levels_[0]) {
            auto res = sstable->get(key);
            if (res.has_value()) {
                if (res->is_tombstone) return std::nullopt;
                return res->value;
            }
        }

        // Priority 3: Levels 1+ (disjoint key ranges per level)
        for (size_t lvl = 1; lvl < MAX_LEVELS; ++lvl) {
            if (levels_[lvl].empty()) continue;

            size_t low = 0, high = levels_[lvl].size() - 1;
            while (low <= high) {
                size_t mid = low + (high - low) / 2;
                const auto& table = levels_[lvl][mid];
                if (key >= table->min_key() && key <= table->max_key()) {
                    auto res = table->get(key);
                    if (res.has_value()) {
                        if (res->is_tombstone) return std::nullopt;
                        return res->value;
                    }
                    break;
                } else if (key < table->min_key()) {
                    if (mid == 0) break;
                    high = mid - 1;
                } else {
                    low = mid + 1;
                }
            }
        }

        return std::nullopt;
    }

    void flush() {
        flush_memtable();
    }
};

} // namespace dsa

int main() {
    using namespace dsa;
    std::cout << "Testing LSM-Tree with Leveled Compaction against std::map oracle...\n";

    LSMTree lsm;
    std::map<std::string, std::string> oracle;

    std::mt19937 rng(1337);
    std::vector<std::string> keys;
    for (int i = 0; i < 250; ++i) {
        keys.push_back("usr_" + std::to_string(i));
    }

    for (int step = 0; step < 5000; ++step) {
        int op = rng() % 3;
        const std::string& k = keys[rng() % keys.size()];

        if (op == 0) { // Put
            std::string v = "payload_" + std::to_string(step);
            lsm.put(k, v);
            oracle[k] = v;
        } else if (op == 1) { // Remove
            lsm.remove(k);
            oracle.erase(k);
        } else { // Get
            auto expected = oracle.count(k) ? std::make_optional(oracle[k]) : std::nullopt;
            auto actual = lsm.get(k);
            assert(actual == expected);
        }
    }

    lsm.flush();
    for (const auto& k : keys) {
        auto expected = oracle.count(k) ? std::make_optional(oracle[k]) : std::nullopt;
        auto actual = lsm.get(k);
        assert(actual == expected);
    }

    std::cout << "All 5,000 LSM-Tree operations with Leveled Compaction passed with 100% precision!\n";
    return 0;
}
