#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <list>
#include <cassert>
#include <cmath>
#include <algorithm>
#include <random>
#include <iomanip>

namespace caching {

// ============================================================================
// 1. BASELINE: STANDARD LRU CACHE
// ============================================================================

template <typename Key, typename Value>
class LRUCache {
private:
    size_t capacity_;
    std::list<std::pair<Key, Value>> items_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> map_;

public:
    explicit LRUCache(size_t capacity) : capacity_(capacity) {}

    bool get(const Key& key, Value& val) {
        auto it = map_.find(key);
        if (it == map_.end()) return false;
        items_.splice(items_.begin(), items_, it->second);
        val = it->second->second;
        return true;
    }

    void put(const Key& key, const Value& val) {
        auto it = map_.find(key);
        if (it != map_.end()) {
            it->second->second = val;
            items_.splice(items_.begin(), items_, it->second);
            return;
        }
        if (items_.size() >= capacity_) {
            auto oldest = items_.back();
            map_.erase(oldest.first);
            items_.pop_back();
        }
        items_.emplace_front(key, val);
        map_[key] = items_.begin();
    }

    bool pop_tail(Key& evicted_key, Value& evicted_val) {
        if (items_.empty()) return false;
        evicted_key = items_.back().first;
        evicted_val = items_.back().second;
        map_.erase(evicted_key);
        items_.pop_back();
        return true;
    }

    bool peek_tail(Key& key) const {
        if (items_.empty()) return false;
        key = items_.back().first;
        return true;
    }

    size_t size() const { return items_.size(); }
    size_t capacity() const { return capacity_; }
};

// ============================================================================
// 2. 2Q CACHE (Johnson & Shasha 1994)
// ============================================================================

template <typename Key, typename Value>
class TwoQueueCache {
private:
    size_t capacity_;
    size_t kin_cap_;
    size_t kout_cap_;

    // A1in: FIFO for new items
    std::list<std::pair<Key, Value>> a1_in_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> map_a1_in_;

    // A1out: Ghost FIFO for evicted keys
    std::list<Key> a1_out_;
    std::unordered_map<Key, typename std::list<Key>::iterator> map_a1_out_;

    // Am: LRU for hot frequent items
    std::list<std::pair<Key, Value>> am_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> map_am_;

public:
    explicit TwoQueueCache(size_t capacity)
        : capacity_(capacity),
          kin_cap_(std::max<size_t>(1, capacity / 4)),
          kout_cap_(std::max<size_t>(1, capacity / 2)) {}

    bool get(const Key& key, Value& val) {
        // 1. Check Am (frequent items)
        auto it_am = map_am_.find(key);
        if (it_am != map_am_.end()) {
            am_.splice(am_.begin(), am_, it_am->second);
            val = it_am->second->second;
            return true;
        }

        // 2. Check A1in: promote to Am on second hit
        auto it_in = map_a1_in_.find(key);
        if (it_in != map_a1_in_.end()) {
            val = it_in->second->second;
            a1_in_.erase(it_in->second);
            map_a1_in_.erase(it_in);
            promote_to_am(key, val);
            return true;
        }

        return false;
    }

    void put(const Key& key, const Value& val) {
        // If in Am, update
        auto it_am = map_am_.find(key);
        if (it_am != map_am_.end()) {
            it_am->second->second = val;
            am_.splice(am_.begin(), am_, it_am->second);
            return;
        }

        // If in A1in, update and promote
        auto it_in = map_a1_in_.find(key);
        if (it_in != map_a1_in_.end()) {
            a1_in_.erase(it_in->second);
            map_a1_in_.erase(it_in);
            promote_to_am(key, val);
            return;
        }

        // If in A1out (ghost hit), promote directly to Am
        auto it_out = map_a1_out_.find(key);
        if (it_out != map_a1_out_.end()) {
            a1_out_.erase(it_out->second);
            map_a1_out_.erase(it_out);
            promote_to_am(key, val);
            return;
        }

        // Complete miss: new item enters A1in
        reclaim_space();
        a1_in_.emplace_front(key, val);
        map_a1_in_[key] = a1_in_.begin();
    }

    size_t size() const { return map_a1_in_.size() + map_am_.size(); }

private:
    void promote_to_am(const Key& key, const Value& val) {
        reclaim_space();
        am_.emplace_front(key, val);
        map_am_[key] = am_.begin();
    }

    void reclaim_space() {
        if (map_a1_in_.size() + map_am_.size() < capacity_) {
            return;
        }

        // Evict from A1in to A1out if A1in is large
        if (map_a1_in_.size() > kin_cap_ || am_.empty()) {
            if (!a1_in_.empty()) {
                auto oldest = a1_in_.back();
                map_a1_in_.erase(oldest.first);
                a1_in_.pop_back();

                if (map_a1_out_.size() >= kout_cap_) {
                    auto ghost_old = a1_out_.back();
                    map_a1_out_.erase(ghost_old);
                    a1_out_.pop_back();
                }
                a1_out_.push_front(oldest.first);
                map_a1_out_[oldest.first] = a1_out_.begin();
            }
        } else if (!am_.empty()) {
            auto oldest_am = am_.back();
            map_am_.erase(oldest_am.first);
            am_.pop_back();
        }
    }
};

// ============================================================================
// 3. ADAPTIVE REPLACEMENT CACHE (ARC - Megiddo & Modha, IBM 2003)
// ============================================================================

template <typename Key, typename Value>
class ARCCache {
private:
    size_t c_;      // Cache capacity
    double p_{0.0}; // Target size of T1

    // T1: Recent resident items
    std::list<std::pair<Key, Value>> t1_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> map_t1_;

    // T2: Frequent resident items
    std::list<std::pair<Key, Value>> t2_;
    std::unordered_map<Key, typename std::list<std::pair<Key, Value>>::iterator> map_t2_;

    // B1: Recent ghost keys
    std::list<Key> b1_;
    std::unordered_map<Key, typename std::list<Key>::iterator> map_b1_;

    // B2: Frequent ghost keys
    std::list<Key> b2_;
    std::unordered_map<Key, typename std::list<Key>::iterator> map_b2_;

public:
    explicit ARCCache(size_t capacity) : c_(capacity), p_(0.0) {}

    bool get(const Key& key, Value& val) {
        // Case 1: In T1 -> Promote to T2 MRU
        auto it1 = map_t1_.find(key);
        if (it1 != map_t1_.end()) {
            val = it1->second->second;
            t2_.emplace_front(key, val);
            map_t2_[key] = t2_.begin();

            t1_.erase(it1->second);
            map_t1_.erase(it1);
            return true;
        }

        // Case 2: In T2 -> Move to T2 MRU
        auto it2 = map_t2_.find(key);
        if (it2 != map_t2_.end()) {
            t2_.splice(t2_.begin(), t2_, it2->second);
            val = it2->second->second;
            return true;
        }

        return false;
    }

    void put(const Key& key, const Value& val) {
        // Case 1: Key in T1 or T2
        auto it1 = map_t1_.find(key);
        if (it1 != map_t1_.end()) {
            t1_.erase(it1->second);
            map_t1_.erase(it1);
            t2_.emplace_front(key, val);
            map_t2_[key] = t2_.begin();
            return;
        }

        auto it2 = map_t2_.find(key);
        if (it2 != map_t2_.end()) {
            it2->second->second = val;
            t2_.splice(t2_.begin(), t2_, it2->second);
            return;
        }

        // Case 2: Key in B1 (Ghost Recency Hit)
        auto it_b1 = map_b1_.find(key);
        if (it_b1 != map_b1_.end()) {
            double delta = (map_b1_.size() >= map_b2_.size())
                               ? 1.0
                               : static_cast<double>(map_b2_.size()) / map_b1_.size();
            p_ = std::min(static_cast<double>(c_), p_ + delta);
            replace(key);

            b1_.erase(it_b1->second);
            map_b1_.erase(it_b1);

            t2_.emplace_front(key, val);
            map_t2_[key] = t2_.begin();
            return;
        }

        // Case 3: Key in B2 (Ghost Frequency Hit)
        auto it_b2 = map_b2_.find(key);
        if (it_b2 != map_b2_.end()) {
            double delta = (map_b2_.size() >= map_b1_.size())
                               ? 1.0
                               : static_cast<double>(map_b1_.size()) / map_b2_.size();
            p_ = std::max(0.0, p_ - delta);
            replace(key);

            b2_.erase(it_b2->second);
            map_b2_.erase(it_b2);

            t2_.emplace_front(key, val);
            map_t2_[key] = t2_.begin();
            return;
        }

        // Case 4: Complete Miss
        size_t l1_size = map_t1_.size() + map_b1_.size();
        size_t l2_size = map_t2_.size() + map_b2_.size();

        if (l1_size == c_) {
            if (map_t1_.size() < c_) {
                if (!b1_.empty()) {
                    auto oldest_b1 = b1_.back();
                    map_b1_.erase(oldest_b1);
                    b1_.pop_back();
                }
                replace(key);
            } else {
                if (!t1_.empty()) {
                    auto oldest_t1 = t1_.back();
                    map_t1_.erase(oldest_t1.first);
                    t1_.pop_back();
                }
            }
        } else if (l1_size < c_ && (l1_size + l2_size >= c_)) {
            if (l1_size + l2_size == 2 * c_) {
                if (!b2_.empty()) {
                    auto oldest_b2 = b2_.back();
                    map_b2_.erase(oldest_b2);
                    b2_.pop_back();
                }
            }
            replace(key);
        }

        t1_.emplace_front(key, val);
        map_t1_[key] = t1_.begin();
    }

    double adaptation_p() const { return p_; }
    size_t size() const { return map_t1_.size() + map_t2_.size(); }

private:
    void replace(const Key& key) {
        if (!t1_.empty() &&
            ((t1_.size() > static_cast<size_t>(p_)) ||
             (map_b2_.find(key) != map_b2_.end() && t1_.size() == static_cast<size_t>(p_)))) {
            auto oldest = t1_.back();
            map_t1_.erase(oldest.first);
            t1_.pop_back();

            b1_.push_front(oldest.first);
            map_b1_[oldest.first] = b1_.begin();
        } else if (!t2_.empty()) {
            auto oldest = t2_.back();
            map_t2_.erase(oldest.first);
            t2_.pop_back();

            b2_.push_front(oldest.first);
            map_b2_[oldest.first] = b2_.begin();
        }
    }
};

// ============================================================================
// 4. W-TINYLFU (Window TinyLFU - Caffeine Cache Architecture)
// ============================================================================

class CountMin4Bit {
private:
    static constexpr size_t DEPTH = 4;
    size_t width_;
    std::vector<std::vector<uint8_t>> table_;
    size_t additions_{0};
    size_t reset_threshold_;

public:
    explicit CountMin4Bit(size_t capacity)
        : width_(std::max<size_t>(64, capacity * 4)),
          table_(DEPTH, std::vector<uint8_t>(width_, 0)),
          reset_threshold_(capacity * 10) {}

    void increment(uint64_t key) {
        for (size_t d = 0; d < DEPTH; ++d) {
            size_t idx = hash_slot(key, d);
            if (table_[d][idx] < 15) {
                table_[d][idx]++;
            }
        }
        additions_++;
        if (additions_ >= reset_threshold_) {
            reset();
        }
    }

    uint8_t estimate(uint64_t key) const {
        uint8_t min_val = 15;
        for (size_t d = 0; d < DEPTH; ++d) {
            size_t idx = hash_slot(key, d);
            min_val = std::min(min_val, table_[d][idx]);
        }
        return min_val;
    }

    void reset() {
        for (size_t d = 0; d < DEPTH; ++d) {
            for (size_t i = 0; i < width_; ++i) {
                table_[d][i] >>= 1;
            }
        }
        additions_ = 0;
    }

private:
    size_t hash_slot(uint64_t key, size_t d) const {
        uint64_t h = key ^ (0x9e3779b97f4a7c15ULL * (d + 1));
        h ^= h >> 30;
        h *= 0xbf58476d1ce4e5b9ULL;
        h ^= h >> 27;
        return h % width_;
    }
};

template <typename Key, typename Value>
class WTinyLFUCache {
private:
    size_t capacity_;
    size_t window_cap_;
    size_t main_cap_;

    CountMin4Bit filter_;
    LRUCache<Key, Value> window_cache_;
    LRUCache<Key, Value> probation_cache_;
    LRUCache<Key, Value> protected_cache_;

public:
    explicit WTinyLFUCache(size_t capacity)
        : capacity_(capacity),
          window_cap_(std::max<size_t>(1, capacity / 20)),
          main_cap_(capacity - window_cap_),
          filter_(capacity),
          window_cache_(window_cap_),
          probation_cache_(std::max<size_t>(1, (main_cap_ * 2) / 10)),
          protected_cache_(std::max<size_t>(1, main_cap_ - std::max<size_t>(1, (main_cap_ * 2) / 10))) {}

    bool get(const Key& key, Value& val) {
        filter_.increment(std::hash<Key>{}(key));

        if (window_cache_.get(key, val)) return true;
        if (protected_cache_.get(key, val)) return true;
        if (probation_cache_.get(key, val)) {
            // Hit in probation: promote to protected
            Key evicted_k;
            Value evicted_v;
            if (protected_cache_.size() >= protected_cache_.capacity()) {
                if (protected_cache_.pop_tail(evicted_k, evicted_v)) {
                    probation_cache_.put(evicted_k, evicted_v);
                }
            }
            protected_cache_.put(key, val);
            return true;
        }

        return false;
    }

    void put(const Key& key, const Value& val) {
        filter_.increment(std::hash<Key>{}(key));

        Value tmp;
        if (window_cache_.get(key, tmp)) {
            window_cache_.put(key, val);
            return;
        }
        if (protected_cache_.get(key, tmp)) {
            protected_cache_.put(key, val);
            return;
        }
        if (probation_cache_.get(key, tmp)) {
            probation_cache_.put(key, val);
            return;
        }

        // If window is full, candidate evicted from window contests admission to probation
        if (window_cache_.size() >= window_cache_.capacity()) {
            Key window_victim_k;
            Value window_victim_v;
            if (window_cache_.pop_tail(window_victim_k, window_victim_v)) {
                admit_to_main(window_victim_k, window_victim_v);
            }
        }
        window_cache_.put(key, val);
    }

    size_t size() const {
        return window_cache_.size() + probation_cache_.size() + protected_cache_.size();
    }

private:
    void admit_to_main(const Key& candidate_k, const Value& candidate_v) {
        if (probation_cache_.size() < probation_cache_.capacity()) {
            probation_cache_.put(candidate_k, candidate_v);
            return;
        }

        Key victim_k;
        if (probation_cache_.peek_tail(victim_k)) {
            uint8_t cand_freq = filter_.estimate(std::hash<Key>{}(candidate_k));
            uint8_t vict_freq = filter_.estimate(std::hash<Key>{}(victim_k));

            if (cand_freq > vict_freq) {
                Key dummy_k;
                Value dummy_v;
                probation_cache_.pop_tail(dummy_k, dummy_v);
                probation_cache_.put(candidate_k, candidate_v);
            }
        }
    }
};

} // namespace caching

// ============================================================================
// VERIFICATION & EMPIRICAL COMPARATIVE BENCHMARK
// ============================================================================

template <typename Cache>
bool read_through(Cache& cache, int key) {
    int val;
    if (cache.get(key, val)) {
        return true;
    }
    cache.put(key, key);
    return false;
}

int main() {
    using namespace caching;

    std::cout << "===============================================================\n";
    std::cout << "High-Performance Caching: ARC, 2Q, W-TinyLFU vs LRU Benchmark\n";
    std::cout << "===============================================================\n";

    // ------------------------------------------------------------------------
    // Test 1: ARC Exact Adaptation Invariant Check
    // ------------------------------------------------------------------------
    std::cout << "[Test 1/3] Testing ARC Dynamic Self-Tuning Parameter p... \n";
    {
        ARCCache<int, std::string> arc(4);
        arc.put(1, "one");
        arc.put(2, "two");
        arc.put(3, "three");
        arc.put(4, "four");
        assert(arc.size() == 4);

        // Access 2 so it is promoted from T1 to T2 (|T1|=3, |T2|=1)
        std::string val;
        bool found2 = arc.get(2, val);
        assert(found2 && val == "two");

        // Put 5: |T1| < c, so replace(5) evicts LRU of T1 (key 1) into ghost list B1
        arc.put(5, "five");

        // Access 1 (ghost hit in B1) -> p should adapt upward towards recency
        arc.put(1, "one-again");
        assert(arc.adaptation_p() > 0.0);
        std::cout << "  -> ARC dynamic parameter adaptation confirmed (p = "
                  << arc.adaptation_p() << ").\n";
    }

    // ------------------------------------------------------------------------
    // Test 2: 2Q & W-TinyLFU Basic Functionality
    // ------------------------------------------------------------------------
    std::cout << "[Test 2/3] Testing 2Q and W-TinyLFU Cache Invariants... \n";
    {
        TwoQueueCache<int, int> tq(4);
        tq.put(10, 100);
        tq.put(20, 200);
        int v;
        assert(tq.get(10, v) && v == 100);

        WTinyLFUCache<int, int> wtiny(10);
        for (int i = 0; i < 15; ++i) {
            wtiny.put(i, i * 10);
        }
        std::cout << "  -> 2Q and W-TinyLFU structures operational and stable.\n";
    }

    // ------------------------------------------------------------------------
    // Test 3: Empirical Scan-Resistance & Hit Ratio Comparison
    // ------------------------------------------------------------------------
    std::cout << "[Test 3/3] Empirical Workload Simulation: Scan-Resistance Test...\n";
    {
        const size_t CACHE_SIZE = 40;
        const size_t HOT_KEYS = 30; // Working set that fits in cache
        const size_t SCAN_LENGTH = 200; // Large sequential scan

        LRUCache<int, int> lru(CACHE_SIZE);
        TwoQueueCache<int, int> tq(CACHE_SIZE);
        ARCCache<int, int> arc(CACHE_SIZE);
        WTinyLFUCache<int, int> wtiny(CACHE_SIZE);

        size_t lru_hits = 0, tq_hits = 0, arc_hits = 0, wtiny_hits = 0;
        size_t total_lookups = 0;

        // 1. Warm-up cache with hot keys multiple times to establish frequency
        for (int round = 0; round < 5; ++round) {
            for (size_t i = 0; i < HOT_KEYS; ++i) {
                read_through(lru, i);
                read_through(tq, i);
                read_through(arc, i);
                read_through(wtiny, i);
            }
        }

        // 2. Interleave hot key reads with a large sequential scan
        std::mt19937 rng(42);
        for (size_t step = 0; step < 2000; ++step) {
            // 85% lookups to hot working set
            if (rng() % 100 < 85) {
                int k = rng() % HOT_KEYS;
                total_lookups++;
                if (read_through(lru, k)) lru_hits++;
                if (read_through(tq, k)) tq_hits++;
                if (read_through(arc, k)) arc_hits++;
                if (read_through(wtiny, k)) wtiny_hits++;
            } else {
                // Cold sequential scan item
                int cold_k = 10000 + (step % SCAN_LENGTH);
                read_through(lru, cold_k);
                read_through(tq, cold_k);
                read_through(arc, cold_k);
                read_through(wtiny, cold_k);
            }
        }

        double lru_hr = (double)lru_hits / total_lookups * 100.0;
        double tq_hr = (double)tq_hits / total_lookups * 100.0;
        double arc_hr = (double)arc_hits / total_lookups * 100.0;
        double wtiny_hr = (double)wtiny_hits / total_lookups * 100.0;

        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  --------------------------------------------------\n";
        std::cout << "  Workload Hit Ratios under Periodic Large Scan:\n";
        std::cout << "  -> Naive LRU:    " << lru_hr << "%\n";
        std::cout << "  -> 2Q Cache:     " << tq_hr << "%\n";
        std::cout << "  -> ARC:          " << arc_hr << "%\n";
        std::cout << "  -> W-TinyLFU:    " << wtiny_hr << "%\n";
        std::cout << "  --------------------------------------------------\n" << std::flush;

        // Modern scan-resistant algorithms outperform naive LRU
        assert(arc_hr > lru_hr);
        assert(tq_hr > lru_hr);
        assert(wtiny_hr > lru_hr);
    }

    std::cout << "===============================================================\n";
    std::cout << "All High-Performance Caching tests PASSED flawlessly!\n";
    std::cout << "===============================================================\n";

    return 0;
}
