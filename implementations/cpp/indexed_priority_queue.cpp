/**
 * Reference Implementation: Indexed Priority Queue (Indexed Min-Heap)
 * Demonstrates:
 * 1. Parallel indexing array model (pq, qp, keys)
 * 2. O(1) membership check and key retrieval
 * 3. O(log n) insert, pop-min, decrease-key, increase-key, change-key, and erase
 * 4. Self-verifying heap and inverse-map invariants
 * 5. Dijkstra's Single-Source Shortest Path using Indexed PQ
 * 6. Prim's Minimum Spanning Tree using Indexed PQ
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <stdexcept>
#include <utility>
#include <algorithm>
#include <cassert>
#include <limits>

namespace indexed_pq {

/**
 * An Indexed Min-Priority Queue for items identified by integers in [0, max_n - 1].
 * Uses 1-based heap positions for arithmetic elegance:
 *   parent(k) = k / 2, left_child(k) = 2 * k, right_child(k) = 2 * k + 1.
 */
template <typename Key>
class IndexedMinPQ {
public:
    explicit IndexedMinPQ(int max_n)
        : n_(0), pq_(max_n + 1, -1), qp_(max_n, -1), keys_(max_n) {
        if (max_n < 0) {
            throw std::invalid_argument("Capacity must be non-negative");
        }
    }

    bool empty() const { return n_ == 0; }
    int size() const { return n_; }
    int capacity() const { return static_cast<int>(qp_.size()); }

    bool contains(int i) const {
        validate_index(i);
        return qp_[i] != -1;
    }

    const Key& key_of(int i) const {
        validate_index(i);
        if (!contains(i)) {
            throw std::invalid_argument("Index is not in the priority queue");
        }
        return keys_[i];
    }

    void insert(int i, const Key& key) {
        validate_index(i);
        if (contains(i)) {
            throw std::invalid_argument("Index is already in the priority queue");
        }

        ++n_;
        qp_[i] = n_;
        pq_[n_] = i;
        keys_[i] = key;
        swim(n_);
    }

    int min_index() const {
        if (n_ == 0) {
            throw std::underflow_error("Priority queue underflow");
        }
        return pq_[1];
    }

    const Key& min_key() const {
        if (n_ == 0) {
            throw std::underflow_error("Priority queue underflow");
        }
        return keys_[pq_[1]];
    }

    int pop_min_index() {
        if (n_ == 0) {
            throw std::underflow_error("Priority queue underflow");
        }

        int min_item = pq_[1];
        exch(1, n_);
        --n_;
        sink(1);

        qp_[min_item] = -1;
        pq_[n_ + 1] = -1;
        return min_item;
    }

    void decrease_key(int i, const Key& key) {
        validate_index(i);
        if (!contains(i)) {
            throw std::invalid_argument("Index is not in the priority queue");
        }
        if (!(key < keys_[i])) {
            throw std::invalid_argument("New key is not strictly smaller than current key");
        }

        keys_[i] = key;
        swim(qp_[i]);
    }

    void increase_key(int i, const Key& key) {
        validate_index(i);
        if (!contains(i)) {
            throw std::invalid_argument("Index is not in the priority queue");
        }
        if (!(keys_[i] < key)) {
            throw std::invalid_argument("New key is not strictly larger than current key");
        }

        keys_[i] = key;
        sink(qp_[i]);
    }

    void change_key(int i, const Key& key) {
        validate_index(i);
        if (!contains(i)) {
            throw std::invalid_argument("Index is not in the priority queue");
        }

        Key old_key = keys_[i];
        keys_[i] = key;

        if (key < old_key) {
            swim(qp_[i]);
        } else if (old_key < key) {
            sink(qp_[i]);
        }
    }

    void erase(int i) {
        validate_index(i);
        if (!contains(i)) {
            throw std::invalid_argument("Index is not in the priority queue");
        }

        int pos = qp_[i];
        exch(pos, n_);
        --n_;

        if (pos <= n_) {
            swim(pos);
            sink(pos);
        }

        qp_[i] = -1;
        pq_[n_ + 1] = -1;
    }

    /**
     * Invariant Verification Helper:
     * 1. 1-to-1 inverse mapping: qp[pq[pos]] == pos for pos in [1, n].
     * 2. Inverse mapping: pq[qp[i]] == i for all active items.
     * 3. qp[i] == -1 for inactive items.
     * 4. Heap order property: keys[pq[parent]] <= keys[pq[child]].
     */
    bool check_invariants() const {
        // Check inverse mapping
        for (int pos = 1; pos <= n_; ++pos) {
            int item = pq_[pos];
            if (item < 0 || item >= static_cast<int>(qp_.size())) return false;
            if (qp_[item] != pos) return false;
        }

        int active_count = 0;
        for (size_t i = 0; i < qp_.size(); ++i) {
            if (qp_[i] != -1) {
                active_count++;
                if (qp_[i] < 1 || qp_[i] > n_) return false;
                if (pq_[qp_[i]] != static_cast<int>(i)) return false;
            }
        }
        if (active_count != n_) return false;

        // Check min-heap property
        for (int pos = 1; pos <= n_ / 2; ++pos) {
            int left = 2 * pos;
            int right = 2 * pos + 1;
            if (left <= n_ && keys_[pq_[pos]] > keys_[pq_[left]]) return false;
            if (right <= n_ && keys_[pq_[pos]] > keys_[pq_[right]]) return false;
        }

        return true;
    }

private:
    int n_;
    std::vector<int> pq_;   // heap position -> item index (1-based)
    std::vector<int> qp_;   // item index -> heap position (-1 if absent)
    std::vector<Key> keys_; // item index -> current priority key

    void validate_index(int i) const {
        if (i < 0 || i >= static_cast<int>(qp_.size())) {
            throw std::out_of_range("Item index out of bounds");
        }
    }

    bool greater_pos(int a, int b) const {
        return keys_[pq_[a]] > keys_[pq_[b]];
    }

    void exch(int a, int b) {
        std::swap(pq_[a], pq_[b]);
        qp_[pq_[a]] = a;
        qp_[pq_[b]] = b;
    }

    void swim(int k) {
        while (k > 1 && greater_pos(k / 2, k)) {
            exch(k, k / 2);
            k /= 2;
        }
    }

    void sink(int k) {
        while (2 * k <= n_) {
            int j = 2 * k;
            if (j < n_ && greater_pos(j, j + 1)) {
                ++j;
            }
            if (!greater_pos(k, j)) {
                break;
            }
            exch(k, j);
            k = j;
        }
    }
};

// ============================================================================
// Application 1: Dijkstra's Shortest Path Algorithm
// ============================================================================

/**
 * Computes shortest path distances from source to all vertices.
 * graph[u] contains pairs of (neighbor v, edge weight w).
 * Time Complexity: O((V + E) log V). Space: O(V).
 */
std::vector<long long> dijkstra_indexed_pq(
    const std::vector<std::vector<std::pair<int, long long>>>& graph,
    int source) {

    int n = static_cast<int>(graph.size());
    const long long INF = std::numeric_limits<long long>::max();
    std::vector<long long> dist(n, INF);

    IndexedMinPQ<long long> pq(n);
    dist[source] = 0;
    pq.insert(source, 0);

    while (!pq.empty()) {
        int u = pq.pop_min_index();
        long long du = dist[u];

        for (const auto& edge : graph[u]) {
            int v = edge.first;
            long long weight = edge.second;
            long long nd = du + weight;

            if (nd < dist[v]) {
                dist[v] = nd;
                if (pq.contains(v)) {
                    pq.decrease_key(v, nd);
                } else {
                    pq.insert(v, nd);
                }
            }
        }
    }

    return dist;
}

// ============================================================================
// Application 2: Prim's Minimum Spanning Tree Algorithm
// ============================================================================

/**
 * Computes the total weight of a Minimum Spanning Tree using Prim's algorithm.
 * Returns {mst_weight, edges} where edges are pairs (u, v).
 * Time Complexity: O(E log V). Space: O(V).
 */
std::pair<long long, std::vector<std::pair<int, int>>> prim_mst_indexed_pq(
    const std::vector<std::vector<std::pair<int, long long>>>& graph,
    int source = 0) {

    int n = static_cast<int>(graph.size());
    if (n == 0) return {0, {}};

    const long long INF = std::numeric_limits<long long>::max();
    std::vector<long long> min_edge(n, INF);
    std::vector<int> parent(n, -1);
    std::vector<bool> in_mst(n, false);

    IndexedMinPQ<long long> pq(n);
    min_edge[source] = 0;
    pq.insert(source, 0);

    long long total_weight = 0;
    std::vector<std::pair<int, int>> mst_edges;

    while (!pq.empty()) {
        int u = pq.pop_min_index();
        in_mst[u] = true;
        total_weight += min_edge[u];

        if (parent[u] != -1) {
            mst_edges.push_back({parent[u], u});
        }

        for (const auto& edge : graph[u]) {
            int v = edge.first;
            long long weight = edge.second;

            if (!in_mst[v] && weight < min_edge[v]) {
                min_edge[v] = weight;
                parent[v] = u;
                if (pq.contains(v)) {
                    pq.decrease_key(v, weight);
                } else {
                    pq.insert(v, weight);
                }
            }
        }
    }

    return {total_weight, mst_edges};
}

} // namespace indexed_pq

// ============================================================================
// Unit Tests & Invariant Verification
// ============================================================================

int main() {
    using namespace indexed_pq;

    std::cout << "Running Indexed Priority Queue C++17 unit tests...\n";

    // ------------------------------------------------------------------------
    // Test 1: Basic Operations & Arthur's Worked Mini-Example
    // Items: 0 -> 8, 1 -> 3, 2 -> 5
    // Root is item 1.
    // Decrease key of item 0 from 8 to 2 -> root becomes item 0.
    // ------------------------------------------------------------------------
    {
        IndexedMinPQ<int> pq(5);
        assert(pq.empty());
        assert(pq.size() == 0);

        pq.insert(0, 8);
        assert(pq.check_invariants());
        pq.insert(1, 3);
        assert(pq.check_invariants());
        pq.insert(2, 5);
        assert(pq.check_invariants());

        assert(pq.size() == 3);
        assert(pq.contains(0) && pq.contains(1) && pq.contains(2));
        assert(!pq.contains(3) && !pq.contains(4));

        assert(pq.min_index() == 1);
        assert(pq.min_key() == 3);

        // Decrease key of item 0 from 8 to 2
        pq.decrease_key(0, 2);
        assert(pq.check_invariants());
        assert(pq.min_index() == 0);
        assert(pq.min_key() == 2);
        assert(pq.key_of(0) == 2);

        // Extract min
        assert(pq.pop_min_index() == 0);
        assert(pq.check_invariants());
        assert(!pq.contains(0));
        assert(pq.size() == 2);
        assert(pq.min_index() == 1);
        assert(pq.min_key() == 3);

        // Increase key of item 1 from 3 to 10
        pq.increase_key(1, 10);
        assert(pq.check_invariants());
        assert(pq.min_index() == 2); // item 2 (key 5) is now min
        assert(pq.min_key() == 5);

        // Change key
        pq.change_key(1, 1); // item 1 key becomes 1 (smaller)
        assert(pq.check_invariants());
        assert(pq.min_index() == 1);
        assert(pq.min_key() == 1);

        // Erase arbitrary item
        pq.erase(2);
        assert(pq.check_invariants());
        assert(!pq.contains(2));
        assert(pq.size() == 1);
        assert(pq.min_index() == 1);

        assert(pq.pop_min_index() == 1);
        assert(pq.check_invariants());
        assert(pq.empty());
    }

    // ------------------------------------------------------------------------
    // Test 2: Error Handling & Boundary Exceptions
    // ------------------------------------------------------------------------
    {
        IndexedMinPQ<int> pq(3);

        // Out of bounds
        bool caught = false;
        try { pq.insert(3, 10); } catch (const std::out_of_range&) { caught = true; }
        assert(caught);

        caught = false;
        try { pq.insert(-1, 10); } catch (const std::out_of_range&) { caught = true; }
        assert(caught);

        // Empty pop
        caught = false;
        try { pq.pop_min_index(); } catch (const std::underflow_error&) { caught = true; }
        assert(caught);

        pq.insert(0, 100);
        // Duplicate insert
        caught = false;
        try { pq.insert(0, 50); } catch (const std::invalid_argument&) { caught = true; }
        assert(caught);

        // Invalid decrease key (new key not smaller)
        caught = false;
        try { pq.decrease_key(0, 150); } catch (const std::invalid_argument&) { caught = true; }
        assert(caught);

        // Invalid increase key (new key not larger)
        caught = false;
        try { pq.increase_key(0, 50); } catch (const std::invalid_argument&) { caught = true; }
        assert(caught);
    }

    // ------------------------------------------------------------------------
    // Test 3: Comprehensive Stress & Invariant Test
    // ------------------------------------------------------------------------
    {
        const int N = 50;
        IndexedMinPQ<int> pq(N);

        for (int i = 0; i < N; ++i) {
            pq.insert(i, (i * 37 + 17) % 100);
            assert(pq.check_invariants());
        }

        // Randomly modify keys
        for (int i = 0; i < N; i += 2) {
            pq.change_key(i, i * 2);
            assert(pq.check_invariants());
        }

        // Erase some elements
        for (int i = 1; i < N; i += 4) {
            pq.erase(i);
            assert(pq.check_invariants());
        }

        // Pop everything in ascending key order
        int last_key = -1;
        while (!pq.empty()) {
            int cur_key = pq.min_key();
            int idx = pq.pop_min_index();
            assert(cur_key >= last_key);
            assert(pq.check_invariants());
            last_key = cur_key;
            (void)idx;
        }
        assert(pq.empty());
    }

    // ------------------------------------------------------------------------
    // Test 4: Application - Dijkstra's Shortest Path
    // Triangle: 0 -> 1 (w=4), 0 -> 2 (w=2), 2 -> 1 (w=1), 1 -> 3 (w=5)
    // Shortest paths from 0:
    // dist[0] = 0, dist[1] = 3 (0->2->1), dist[2] = 2, dist[3] = 8 (0->2->1->3)
    // ------------------------------------------------------------------------
    {
        std::vector<std::vector<std::pair<int, long long>>> graph(4);
        graph[0].push_back({1, 4});
        graph[0].push_back({2, 2});
        graph[2].push_back({1, 1});
        graph[1].push_back({3, 5});

        auto dist = dijkstra_indexed_pq(graph, 0);
        assert(dist[0] == 0);
        assert(dist[1] == 3);
        assert(dist[2] == 2);
        assert(dist[3] == 8);
    }

    // ------------------------------------------------------------------------
    // Test 5: Application - Prim's MST
    // Graph:
    // 0-1 (1), 1-2 (2), 0-2 (4), 1-3 (6), 2-3 (3)
    // MST edges: (0, 1) w=1, (1, 2) w=2, (2, 3) w=3 -> total = 6
    // ------------------------------------------------------------------------
    {
        std::vector<std::vector<std::pair<int, long long>>> graph(4);
        auto add_undirected = [&](int u, int v, long long w) {
            graph[u].push_back({v, w});
            graph[v].push_back({u, w});
        };

        add_undirected(0, 1, 1);
        add_undirected(1, 2, 2);
        add_undirected(0, 2, 4);
        add_undirected(1, 3, 6);
        add_undirected(2, 3, 3);

        auto [mst_weight, edges] = prim_mst_indexed_pq(graph, 0);
        assert(mst_weight == 6);
        assert(edges.size() == 3);
    }

    std::cout << "All Indexed Priority Queue C++17 unit tests passed successfully!\n";
    return 0;
}
