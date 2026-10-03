#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <sstream>
#include <iomanip>

namespace distributed {

// ============================================================================
// 1. CONSISTENT HASH RING WITH VIRTUAL NODES (Dynamo-Style)
// ============================================================================

class ConsistentHashRing {
public:
    using Token = uint64_t;

private:
    size_t vnodes_per_node_;
    // Sorted token ring: token -> physical node id
    std::map<Token, std::string> ring_;
    std::unordered_set<std::string> physical_nodes_;

    // 64-bit FNV-1a Hash
    static Token hash_fn(const std::string& key) {
        Token hash = 14695981039346656037ULL;
        for (char c : key) {
            hash ^= static_cast<Token>(c);
            hash *= 1099511628211ULL;
        }
        return hash;
    }

public:
    explicit ConsistentHashRing(size_t vnodes_per_node = 100)
        : vnodes_per_node_(vnodes_per_node) {}

    void add_node(const std::string& node_id) {
        if (physical_nodes_.count(node_id)) return;
        physical_nodes_.insert(node_id);

        for (size_t v = 0; v < vnodes_per_node_; ++v) {
            std::string vnode_key = node_id + "#vnode#" + std::to_string(v);
            Token token = hash_fn(vnode_key);
            ring_[token] = node_id;
        }
    }

    void remove_node(const std::string& node_id) {
        if (!physical_nodes_.count(node_id)) return;
        physical_nodes_.erase(node_id);

        for (size_t v = 0; v < vnodes_per_node_; ++v) {
            std::string vnode_key = node_id + "#vnode#" + std::to_string(v);
            Token token = hash_fn(vnode_key);
            ring_.erase(token);
        }
    }

    // Get primary physical node for a given key
    std::string get_node(const std::string& key) const {
        if (ring_.empty()) return "";
        Token token = hash_fn(key);

        // Binary search for first token >= key token
        auto it = ring_.lower_bound(token);
        if (it == ring_.end()) {
            it = ring_.begin(); // Wrap around ring
        }
        return it->second;
    }

    // Get N distinct physical nodes for quorum replication (Dynamo preference list)
    std::vector<std::string> get_preference_list(const std::string& key, size_t replication_factor) const {
        std::vector<std::string> preference_list;
        if (ring_.empty()) return preference_list;

        Token token = hash_fn(key);
        auto it = ring_.lower_bound(token);
        if (it == ring_.end()) it = ring_.begin();

        std::unordered_set<std::string> visited;
        auto start_it = it;

        do {
            if (!visited.count(it->second)) {
                visited.insert(it->second);
                preference_list.push_back(it->second);
                if (preference_list.size() == replication_factor) {
                    break;
                }
            }
            ++it;
            if (it == ring_.end()) it = ring_.begin();
        } while (it != start_it && preference_list.size() < physical_nodes_.size());

        return preference_list;
    }

    size_t node_count() const { return physical_nodes_.size(); }
    size_t ring_size() const { return ring_.size(); }
};

// ============================================================================
// 2. VECTOR CLOCK & CAUSALITY TRACKING (Fidge & Mattern)
// ============================================================================

enum class CausalityOrder {
    BEFORE,    // A < B (A causally precedes B)
    AFTER,     // A > B (B causally precedes A)
    EQUAL,     // A == B
    CONCURRENT // A || B (Conflict / Divergence)
};

class VectorClock {
private:
    std::unordered_map<std::string, uint64_t> clock_;

public:
    VectorClock() = default;

    void tick(const std::string& node_id) {
        clock_[node_id]++;
    }

    uint64_t get(const std::string& node_id) const {
        auto it = clock_.find(node_id);
        return it != clock_.end() ? it->second : 0;
    }

    void merge(const VectorClock& other) {
        for (const auto& kv : other.clock_) {
            clock_[kv.first] = std::max(clock_[kv.first], kv.second);
        }
    }

    CausalityOrder compare(const VectorClock& other) const {
        bool this_greater = false;
        bool other_greater = false;

        // Collect all distinct node keys
        std::unordered_set<std::string> all_nodes;
        for (const auto& kv : clock_) all_nodes.insert(kv.first);
        for (const auto& kv : other.clock_) all_nodes.insert(kv.first);

        for (const auto& node : all_nodes) {
            uint64_t v1 = get(node);
            uint64_t v2 = other.get(node);

            if (v1 > v2) this_greater = true;
            if (v2 > v1) other_greater = true;
        }

        if (this_greater && other_greater) return CausalityOrder::CONCURRENT;
        if (this_greater) return CausalityOrder::AFTER;
        if (other_greater) return CausalityOrder::BEFORE;
        return CausalityOrder::EQUAL;
    }

    std::string to_string() const {
        std::ostringstream oss;
        oss << "{";
        bool first = true;
        for (const auto& kv : clock_) {
            if (!first) oss << ", ";
            oss << kv.first << ":" << kv.second;
            first = false;
        }
        oss << "}";
        return oss.str();
    }
};

// ============================================================================
// 3. RAFT REPLICATED LOG & STATE MACHINE (Ongaro & Ousterhout)
// ============================================================================

struct LogEntry {
    uint64_t term;
    uint64_t index;
    std::string command;
};

class RaftServer {
public:
    std::string id;
    uint64_t current_term{0};
    std::string voted_for{""};

    // Replicated Log: 1-indexed (index 0 is dummy)
    std::vector<LogEntry> log;

    // Volatile state on all servers
    uint64_t commit_index{0};
    uint64_t last_applied{0};

    // State machine output
    std::vector<std::string> applied_commands;

    explicit RaftServer(std::string server_id) : id(std::move(server_id)) {
        // Dummy entry at index 0
        log.push_back({0, 0, ""});
    }

    uint64_t last_log_index() const {
        return log.empty() ? 0 : log.back().index;
    }

    uint64_t last_log_term() const {
        return log.empty() ? 0 : log.back().term;
    }

    // AppendEntries RPC Receiver implementation
    bool append_entries(uint64_t term,
                        const std::string& leader_id,
                        uint64_t prev_log_index,
                        uint64_t prev_log_term,
                        const std::vector<LogEntry>& entries,
                        uint64_t leader_commit) {
        // 1. Reply false if term < currentTerm
        if (term < current_term) {
            return false;
        }

        if (term > current_term) {
            current_term = term;
            voted_for = "";
        }

        // 2. Reply false if log doesn't contain an entry at prevLogIndex matching prevLogTerm
        if (prev_log_index > last_log_index()) {
            return false;
        }
        if (prev_log_index > 0 && log[prev_log_index].term != prev_log_term) {
            return false;
        }

        // 3. If an existing entry conflicts with a new one, delete existing entry and all that follow it
        size_t insert_idx = prev_log_index + 1;
        for (const auto& entry : entries) {
            if (insert_idx <= last_log_index()) {
                if (log[insert_idx].term != entry.term) {
                    log.erase(log.begin() + insert_idx, log.end());
                    log.push_back(entry);
                }
            } else {
                log.push_back(entry);
            }
            insert_idx++;
        }

        // 4. Update commitIndex
        if (leader_commit > commit_index) {
            commit_index = std::min(leader_commit, last_log_index());
            apply_to_state_machine();
        }

        (void)leader_id;
        return true;
    }

    void apply_to_state_machine() {
        while (last_applied < commit_index) {
            last_applied++;
            applied_commands.push_back(log[last_applied].command);
        }
    }
};

class RaftClusterSimulation {
public:
    std::vector<RaftServer> nodes;
    size_t leader_idx{0};

    explicit RaftClusterSimulation(size_t node_count = 3) {
        for (size_t i = 0; i < node_count; ++i) {
            nodes.emplace_back("node_" + std::to_string(i));
        }
    }

    void elect_leader(size_t idx, uint64_t term) {
        leader_idx = idx;
        for (auto& n : nodes) {
            n.current_term = term;
            n.voted_for = nodes[idx].id;
        }
    }

    // Client proposes command to current leader
    bool client_request(const std::string& cmd) {
        RaftServer& leader = nodes[leader_idx];
        uint64_t entry_index = leader.last_log_index() + 1;
        LogEntry new_entry{leader.current_term, entry_index, cmd};
        leader.log.push_back(new_entry);

        // Replicate to followers
        size_t match_count = 1; // Leader matches its own log
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (i == leader_idx) continue;

            uint64_t prev_idx = new_entry.index - 1;
            uint64_t prev_term = leader.log[prev_idx].term;
            std::vector<LogEntry> to_send = {new_entry};

            bool success = nodes[i].append_entries(leader.current_term,
                                                  leader.id,
                                                  prev_idx,
                                                  prev_term,
                                                  to_send,
                                                  leader.commit_index);
            if (success) {
                match_count++;
            }
        }

        // Quorum consensus: majority commit
        if (match_count > nodes.size() / 2) {
            leader.commit_index = entry_index;
            leader.apply_to_state_machine();

            // Notify followers of updated commitIndex
            for (size_t i = 0; i < nodes.size(); ++i) {
                if (i == leader_idx) continue;
                nodes[i].commit_index = std::min(leader.commit_index, nodes[i].last_log_index());
                nodes[i].apply_to_state_machine();
            }
            return true;
        }
        return false;
    }
};

} // namespace distributed

// ============================================================================
// VERIFICATION & EMPIRICAL EXPERIMENTS
// ============================================================================

int main() {
    using namespace distributed;

    std::cout << "===============================================================\n";
    std::cout << "Distributed State Engines: Consistent Hashing, Clocks & Raft\n";
    std::cout << "===============================================================\n";

    // ------------------------------------------------------------------------
    // Test 1: Consistent Hash Ring & Key Migration Invariance
    // ------------------------------------------------------------------------
    std::cout << "[Test 1/3] Testing Consistent Hashing & Virtual Node Migration...\n";
    {
        ConsistentHashRing ring(200); // 200 vnodes per physical node
        ring.add_node("node-A");
        ring.add_node("node-B");
        ring.add_node("node-C");
        ring.add_node("node-D");

        const size_t NUM_KEYS = 10000;
        std::vector<std::string> keys;
        keys.reserve(NUM_KEYS);
        for (size_t i = 0; i < NUM_KEYS; ++i) {
            keys.push_back("user_session_token_" + std::to_string(i));
        }

        // Record initial placement across 4 nodes
        std::unordered_map<std::string, std::string> initial_map;
        for (const auto& k : keys) {
            initial_map[k] = ring.get_node(k);
        }

        // Add 5th node
        ring.add_node("node-E");

        // Count migrated keys
        size_t migrated = 0;
        for (const auto& k : keys) {
            std::string new_node = ring.get_node(k);
            if (new_node != initial_map[k]) {
                migrated++;
                // Invariant: migrated keys MUST move to the newly added node-E!
                assert(new_node == "node-E");
            }
        }

        double migration_rate = (double)migrated / NUM_KEYS * 100.0;
        double theoretical_rate = 1.0 / 5.0 * 100.0; // 20%
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "  -> Total keys: " << NUM_KEYS << "\n";
        std::cout << "  -> Migrated keys: " << migrated << " (" << migration_rate << "%)\n";
        std::cout << "  -> Theoretical optimal (1/N): " << theoretical_rate << "%\n" << std::flush;

        // Migration rate should be close to theoretical 20% (+/- 8%)
        assert(std::abs(migration_rate - theoretical_rate) < 8.0);

        // Test Dynamo Preference List (RF=3)
        auto pref = ring.get_preference_list("order_id_4491", 3);
        assert(pref.size() == 3);
        assert(pref[0] != pref[1] && pref[1] != pref[2] && pref[0] != pref[2]);
        std::cout << "  -> Preference List for replication factor 3: ["
                  << pref[0] << ", " << pref[1] << ", " << pref[2] << "]\n";
    }

    // ------------------------------------------------------------------------
    // Test 2: Vector Clocks & Causality / Conflict Detection
    // ------------------------------------------------------------------------
    std::cout << "[Test 2/3] Testing Vector Clocks & Concurrent Conflict Detection...\n";
    {
        VectorClock vc_a, vc_b;

        // Process A executes event
        vc_a.tick("A"); // {A:1}

        // Process A sends message to B
        vc_b.merge(vc_a);
        vc_b.tick("B"); // {A:1, B:1}

        // vc_a should causally precede vc_b
        assert(vc_a.compare(vc_b) == CausalityOrder::BEFORE);
        assert(vc_b.compare(vc_a) == CausalityOrder::AFTER);

        // Concurrent divergence: A and B both execute independent local events
        vc_a.tick("A"); // {A:2}
        vc_b.tick("B"); // {A:1, B:2}

        // Neither causally precedes the other -> Conflict!
        assert(vc_a.compare(vc_b) == CausalityOrder::CONCURRENT);
        assert(vc_b.compare(vc_a) == CausalityOrder::CONCURRENT);

        // Reconcile divergence (e.g., via CRDT or merge)
        VectorClock vc_merged = vc_a;
        vc_merged.merge(vc_b);
        vc_merged.tick("A"); // {A:3, B:2}

        assert(vc_a.compare(vc_merged) == CausalityOrder::BEFORE);
        assert(vc_b.compare(vc_merged) == CausalityOrder::BEFORE);

        std::cout << "  -> Vector Clock causality & concurrency resolution verified.\n";
    }

    // ------------------------------------------------------------------------
    // Test 3: Raft Consensus Replicated Log & State Machine
    // ------------------------------------------------------------------------
    std::cout << "[Test 3/3] Testing Raft Consensus Replicated Log Simulation...\n";
    {
        RaftClusterSimulation cluster(3);
        cluster.elect_leader(0, 1); // Node 0 is leader in Term 1

        // Submit client requests
        assert(cluster.client_request("SET x = 10"));
        assert(cluster.client_request("SET y = 20"));
        assert(cluster.client_request("INCR x"));

        // Verify all 3 nodes committed and applied the exact same sequence
        for (const auto& node : cluster.nodes) {
            assert(node.commit_index == 3);
            assert(node.applied_commands.size() == 3);
            assert(node.applied_commands[0] == "SET x = 10");
            assert(node.applied_commands[1] == "SET y = 20");
            assert(node.applied_commands[2] == "INCR x");
        }

        std::cout << "  -> Raft state machine consistency verified across all 3 nodes.\n";
    }

    std::cout << "===============================================================\n";
    std::cout << "All Distributed State Engine tests PASSED flawlessly!\n";
    std::cout << "===============================================================\n";

    return 0;
}
