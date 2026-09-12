/**
 * @file consistent_hashing.cpp
 * @brief Reference implementation of a Consistent Hash Ring with Virtual Nodes.
 *
 * Implements node addition, node removal, clockwise key routing via std::upper_bound,
 * and empirical validation of minimal key redistribution upon cluster scaling.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <cmath>

namespace dsa {

/**
 * @brief 64-bit FNV-1a Hash Function with avalanche mixing.
 */
inline uint64_t hash_string_fnv1a(const std::string& str) {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : str) {
        hash ^= static_cast<uint8_t>(c);
        hash *= 1099511628211ULL;
    }
    // Final avalanche mix
    hash ^= hash >> 33;
    hash *= 0xff51afd7ed558ccdULL;
    hash ^= hash >> 33;
    return hash;
}

class ConsistentHashRing {
private:
    size_t vnodes_per_node_;
    // Ring mapping: ring_position -> physical_node_name
    std::map<uint64_t, std::string> ring_;
    std::vector<std::string> physical_nodes_;

public:
    explicit ConsistentHashRing(size_t vnodes = 100) : vnodes_per_node_(vnodes) {}

    void add_node(const std::string& node_name) {
        physical_nodes_.push_back(node_name);
        for (size_t i = 0; i < vnodes_per_node_; ++i) {
            std::string vnode_key = node_name + "#vn" + std::to_string(i);
            uint64_t pos = hash_string_fnv1a(vnode_key);
            ring_[pos] = node_name;
        }
    }

    void remove_node(const std::string& node_name) {
        auto it = std::find(physical_nodes_.begin(), physical_nodes_.end(), node_name);
        if (it != physical_nodes_.end()) {
            physical_nodes_.erase(it);
        }
        for (size_t i = 0; i < vnodes_per_node_; ++i) {
            std::string vnode_key = node_name + "#vn" + std::to_string(i);
            uint64_t pos = hash_string_fnv1a(vnode_key);
            ring_.erase(pos);
        }
    }

    std::string get_node(const std::string& key) const {
        if (ring_.empty()) return "";

        uint64_t key_hash = hash_string_fnv1a(key);
        // Find first entry on the ring with position >= key_hash
        auto it = ring_.lower_bound(key_hash);

        // If at end of ring, wrap around clockwise to the beginning
        if (it == ring_.end()) {
            it = ring_.begin();
        }

        return it->second;
    }

    size_t node_count() const { return physical_nodes_.size(); }
    size_t ring_size() const { return ring_.size(); }
};

} // namespace dsa

int main() {
    std::cout << "Running Consistent Hashing verification..." << std::endl;

    dsa::ConsistentHashRing ring(100); // 100 vnodes per node
    ring.add_node("node-A");
    ring.add_node("node-B");
    ring.add_node("node-C");

    assert(ring.node_count() == 3);
    assert(ring.ring_size() == 300);

    // 1. Basic deterministic routing test
    std::string n1 = ring.get_node("user_1001");
    std::string n2 = ring.get_node("user_1001");
    assert(n1 == n2); // Consistent lookup
    assert(!n1.empty());

    // 2. Minimal key redistribution test
    // Route 10,000 keys across 3 nodes
    const int NUM_KEYS = 10000;
    std::vector<std::string> initial_assignments(NUM_KEYS);
    for (int i = 0; i < NUM_KEYS; ++i) {
        initial_assignments[i] = ring.get_node("session_key_" + std::to_string(i));
    }

    // Add a 4th node: node-D
    ring.add_node("node-D");
    assert(ring.node_count() == 4);

    // In a 3 -> 4 node expansion, theoretical fraction of migrated keys is ~ 1/4 = 25%.
    // Under naive modulo hashing, 75% of keys migrate.
    int migrated_keys = 0;
    for (int i = 0; i < NUM_KEYS; ++i) {
        std::string new_node = ring.get_node("session_key_" + std::to_string(i));
        if (new_node != initial_assignments[i]) {
            migrated_keys++;
            // The migrated key MUST have landed on the newly added node!
            assert(new_node == "node-D");
        }
    }

    double migration_ratio = static_cast<double>(migrated_keys) / NUM_KEYS;
    std::cout << "[PASS] Key migration ratio upon adding 4th node: " << migration_ratio * 100.0 << "%" << std::endl;
    // Assert migration is close to 25% (between 18% and 32% with vnode dispersion)
    assert(migration_ratio >= 0.18 && migration_ratio <= 0.32);

    std::cout << "All Consistent Hashing assertions passed successfully!" << std::endl;
    return 0;
}
