/**
 * Reference Implementation: Disjoint Set Union (Union-Find)
 * Demonstrates:
 * 1. Standard DSU with path compression and union by size (near-constant amortized O(alpha(n))).
 * 2. Rollback DSU with an undo stack for offline dynamic connectivity (O(log n) find, O(1) rollback).
 * 3. Parity / Potential DSU for bipartite graph consistency and 2-coloring constraints.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <numeric>
#include <cassert>
#include <utility>
#include <algorithm>

// ============================================================================
// 1. Standard Disjoint Set Union (Path Compression + Union by Size)
// ============================================================================
class DisjointSetUnion {
private:
    std::vector<int> parent_;
    std::vector<int> size_;
    std::size_t num_components_;

public:
    explicit DisjointSetUnion(std::size_t n)
        : parent_(n), size_(n, 1), num_components_(n) {
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    // Path compression flattens tree on query: amortized O(alpha(n))
    int find(int x) {
        if (parent_[x] != x) {
            parent_[x] = find(parent_[x]);
        }
        return parent_[x];
    }

    // Union by size attaches smaller tree under larger: amortized O(alpha(n))
    bool unite(int a, int b) {
        int root_a = find(a);
        int root_b = find(b);

        if (root_a == root_b) {
            return false; // Already in the same equivalence class
        }

        if (size_[root_a] < size_[root_b]) {
            std::swap(root_a, root_b);
        }

        parent_[root_b] = root_a;
        size_[root_a] += size_[root_b];
        --num_components_;
        return true;
    }

    [[nodiscard]] bool same(int a, int b) {
        return find(a) == find(b);
    }

    [[nodiscard]] int size_of(int x) {
        return size_[find(x)];
    }

    [[nodiscard]] std::size_t num_components() const noexcept {
        return num_components_;
    }
};

// ============================================================================
// 2. Rollback DSU (Union by Size WITHOUT Path Compression + Undo Stack)
// ============================================================================
class RollbackDSU {
private:
    struct Operation {
        int child;
        int parent;
        int old_parent_size;
    };

    std::vector<int> parent_;
    std::vector<int> size_;
    std::vector<Operation> history_;
    std::size_t num_components_;

public:
    explicit RollbackDSU(std::size_t n)
        : parent_(n), size_(n, 1), num_components_(n) {
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    // Must NOT use path compression so historical state can be cleanly unwound
    int find(int x) const {
        while (parent_[x] != x) {
            x = parent_[x];
        }
        return x;
    }

    bool unite(int a, int b) {
        int root_a = find(a);
        int root_b = find(b);

        if (root_a == root_b) {
            return false;
        }

        if (size_[root_a] < size_[root_b]) {
            std::swap(root_a, root_b);
        }

        history_.push_back({root_b, root_a, size_[root_a]});
        parent_[root_b] = root_a;
        size_[root_a] += size_[root_b];
        --num_components_;
        return true;
    }

    [[nodiscard]] bool same(int a, int b) const {
        return find(a) == find(b);
    }

    [[nodiscard]] std::size_t checkpoint() const noexcept {
        return history_.size();
    }

    void rollback_to(std::size_t cp) {
        while (history_.size() > cp) {
            Operation op = history_.back();
            history_.pop_back();

            parent_[op.child] = op.child;
            size_[op.parent] = op.old_parent_size;
            ++num_components_;
        }
    }

    void rollback_one() {
        if (!history_.empty()) {
            rollback_to(history_.size() - 1);
        }
    }

    [[nodiscard]] std::size_t num_components() const noexcept {
        return num_components_;
    }
};

// ============================================================================
// 3. Parity / Potential DSU (Tracking Bipartiteness / 2-Coloring)
// ============================================================================
class ParityDSU {
private:
    std::vector<int> parent_;
    std::vector<int> parity_; // parity_[x] = relative color to parent (0: same, 1: opposite)
    std::vector<int> size_;
    bool is_bipartite_ = true;

public:
    explicit ParityDSU(std::size_t n)
        : parent_(n), parity_(n, 0), size_(n, 1) {
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    // Path compression accumulates XOR parity along the path to root
    int find(int x) {
        if (parent_[x] != x) {
            int original_parent = parent_[x];
            parent_[x] = find(parent_[x]);
            parity_[x] ^= parity_[original_parent];
        }
        return parent_[x];
    }

    // Assert that 'a' and 'b' must have relation 'rel' (0: same color, 1: opposite color)
    // Returns false if this constraint contradicts prior constraints (odd cycle detected)
    bool add_relation(int a, int b, int rel) {
        int root_a = find(a);
        int root_b = find(b);

        if (root_a == root_b) {
            if ((parity_[a] ^ parity_[b]) != rel) {
                is_bipartite_ = false;
                return false; // Contradiction!
            }
            return true;
        }

        if (size_[root_a] < size_[root_b]) {
            std::swap(root_a, root_b);
            std::swap(a, b);
        }

        // Compute needed parity between root_b and root_a
        parent_[root_b] = root_a;
        parity_[root_b] = parity_[a] ^ parity_[b] ^ rel;
        size_[root_a] += size_[root_b];
        return true;
    }

    [[nodiscard]] bool is_bipartite() const noexcept {
        return is_bipartite_;
    }

    [[nodiscard]] int get_parity(int x) {
        find(x);
        return parity_[x];
    }
};

// ============================================================================
// Unit Tests
// ============================================================================
int main() {
    // -------------------------------------------------------------
    // 1. Standard DSU Tests
    // -------------------------------------------------------------
    DisjointSetUnion dsu(10);
    assert(dsu.num_components() == 10);
    for (int i = 0; i < 10; ++i) {
        assert(dsu.size_of(i) == 1);
        assert(dsu.find(i) == i);
    }

    assert(dsu.unite(1, 2));
    assert(dsu.unite(2, 3));
    assert(dsu.same(1, 3));
    assert(!dsu.same(1, 4));
    assert(dsu.size_of(1) == 3);
    assert(dsu.num_components() == 8);

    assert(!dsu.unite(1, 3)); // Redundant edge (cycle detection)
    assert(dsu.num_components() == 8);

    assert(dsu.unite(4, 5));
    assert(dsu.unite(3, 4));
    assert(dsu.same(1, 5));
    assert(dsu.size_of(5) == 5);
    assert(dsu.num_components() == 6);

    // -------------------------------------------------------------
    // 2. Rollback DSU Tests
    // -------------------------------------------------------------
    RollbackDSU rdsu(6);
    assert(rdsu.num_components() == 6);

    rdsu.unite(0, 1);
    rdsu.unite(1, 2);
    assert(rdsu.same(0, 2));
    assert(rdsu.num_components() == 4);

    std::size_t cp = rdsu.checkpoint();

    rdsu.unite(3, 4);
    rdsu.unite(4, 5);
    rdsu.unite(2, 5);
    assert(rdsu.same(0, 5));
    assert(rdsu.num_components() == 1);

    // Rollback to checkpoint
    rdsu.rollback_to(cp);
    assert(rdsu.same(0, 2));
    assert(!rdsu.same(0, 5));
    assert(!rdsu.same(3, 4));
    assert(rdsu.num_components() == 4);

    // Rollback all
    rdsu.rollback_to(0);
    assert(!rdsu.same(0, 1));
    assert(rdsu.num_components() == 6);

    // -------------------------------------------------------------
    // 3. Parity / Bipartite DSU Tests
    // -------------------------------------------------------------
    ParityDSU pdsu(5);
    // Build a bipartite cycle: 0 - 1 - 2 - 3 - 0 (4 vertices, even cycle)
    assert(pdsu.add_relation(0, 1, 1)); // 0 and 1 opposite
    assert(pdsu.add_relation(1, 2, 1)); // 1 and 2 opposite
    assert(pdsu.add_relation(2, 3, 1)); // 2 and 3 opposite
    assert(pdsu.add_relation(3, 0, 1)); // 3 and 0 opposite -> valid even cycle
    assert(pdsu.is_bipartite());

    // Introduce an odd cycle: 0 and 2 must have opposite color (contradiction with 0-1-2)
    assert(!pdsu.add_relation(0, 2, 1)); // Should fail!
    assert(!pdsu.is_bipartite());

    std::cout << "[PASS] All DisjointSetUnion C++ unit tests passed.\n";
    return 0;
}
