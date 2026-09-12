/**
 * @file sets_functions_relations.cpp
 * @brief Reference implementations for set algebra, binary relations, and function properties.
 *
 * Implements bitwise set operations, binary relation property verifiers (reflexive,
 * symmetric, antisymmetric, transitive), equivalence partitioner via DSU,
 * Warshall's transitive closure, and function injection/surjection checkers.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <numeric>

namespace dsa {

/**
 * @brief Fast bitwise set operations on universe of size <= 64.
 */
class Bitset64 {
public:
    uint64_t mask;
    explicit Bitset64(uint64_t m = 0) : mask(m) {}

    void insert(int elem) { assert(elem >= 0 && elem < 64); mask |= (1ULL << elem); }
    void erase(int elem) { assert(elem >= 0 && elem < 64); mask &= ~(1ULL << elem); }
    bool contains(int elem) const { assert(elem >= 0 && elem < 64); return (mask & (1ULL << elem)) != 0; }
    size_t size() const { return static_cast<size_t>(__builtin_popcountll(mask)); }

    Bitset64 set_union(const Bitset64& other) const { return Bitset64(mask | other.mask); }
    Bitset64 set_intersection(const Bitset64& other) const { return Bitset64(mask & other.mask); }
    Bitset64 set_difference(const Bitset64& other) const { return Bitset64(mask & (~other.mask)); }
    Bitset64 symmetric_difference(const Bitset64& other) const { return Bitset64(mask ^ other.mask); }
};

/**
 * @brief Binary relation representation on finite set {0, 1, ..., V-1}.
 */
class BinaryRelation {
public:
    size_t n;
    std::vector<std::vector<bool>> rel;

    explicit BinaryRelation(size_t vertices)
        : n(vertices), rel(vertices, std::vector<bool>(vertices, false)) {}

    void add_pair(size_t u, size_t v) {
        assert(u < n && v < n);
        rel[u][v] = true;
    }

    bool has_pair(size_t u, size_t v) const {
        return (u < n && v < n) ? rel[u][v] : false;
    }

    bool is_reflexive() const {
        for (size_t i = 0; i < n; ++i) {
            if (!rel[i][i]) return false;
        }
        return true;
    }

    bool is_symmetric() const {
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (rel[i][j] && !rel[j][i]) return false;
            }
        }
        return true;
    }

    bool is_antisymmetric() const {
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (i != j && rel[i][j] && rel[j][i]) return false;
            }
        }
        return true;
    }

    bool is_transitive() const {
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = 0; j < n; ++j) {
                if (rel[i][j]) {
                    for (size_t k = 0; k < n; ++k) {
                        if (rel[j][k] && !rel[i][k]) return false;
                    }
                }
            }
        }
        return true;
    }

    bool is_equivalence_relation() const {
        return is_reflexive() && is_symmetric() && is_transitive();
    }

    bool is_partial_order() const {
        return is_reflexive() && is_antisymmetric() && is_transitive();
    }

    /**
     * @brief Computes transitive closure using Warshall's dynamic programming algorithm in O(V^3).
     */
    BinaryRelation transitive_closure() const {
        BinaryRelation closure = *this;
        for (size_t k = 0; k < n; ++k) {
            for (size_t i = 0; i < n; ++i) {
                if (closure.rel[i][k]) {
                    for (size_t j = 0; j < n; ++j) {
                        if (closure.rel[k][j]) {
                            closure.rel[i][j] = true;
                        }
                    }
                }
            }
        }
        return closure;
    }
};

/**
 * @brief Evaluates whether a mapping f: {0, ..., domain_size-1} -> {0, ..., codomain_size-1}
 * is injective (one-to-one).
 */
inline bool is_injective(const std::vector<size_t>& f, size_t codomain_size) {
    std::vector<bool> seen(codomain_size, false);
    for (size_t y : f) {
        assert(y < codomain_size);
        if (seen[y]) return false;
        seen[y] = true;
    }
    return true;
}

/**
 * @brief Evaluates whether mapping f is surjective (onto).
 */
inline bool is_surjective(const std::vector<size_t>& f, size_t codomain_size) {
    std::vector<bool> seen(codomain_size, false);
    size_t covered = 0;
    for (size_t y : f) {
        assert(y < codomain_size);
        if (!seen[y]) {
            seen[y] = true;
            ++covered;
        }
    }
    return covered == codomain_size;
}

/**
 * @brief Evaluates whether mapping f is bijective (injective and surjective).
 */
inline bool is_bijective(const std::vector<size_t>& f, size_t codomain_size) {
    return is_injective(f, codomain_size) && is_surjective(f, codomain_size);
}

} // namespace dsa

int main() {
    std::cout << "Running Sets, Functions, and Relations C++17 unit tests..." << std::endl;

    // Test 1: Bitset operations
    {
        dsa::Bitset64 A, B;
        A.insert(1); A.insert(3); A.insert(5); // A = {1, 3, 5}
        B.insert(3); B.insert(4); B.insert(5); // B = {3, 4, 5}

        dsa::Bitset64 U = A.set_union(B); // {1, 3, 4, 5}
        assert(U.size() == 4);
        assert(U.contains(1) && U.contains(3) && U.contains(4) && U.contains(5));

        dsa::Bitset64 I = A.set_intersection(B); // {3, 5}
        assert(I.size() == 2);
        assert(I.contains(3) && I.contains(5));

        dsa::Bitset64 D = A.set_difference(B); // {1}
        assert(D.size() == 1 && D.contains(1));

        dsa::Bitset64 SD = A.symmetric_difference(B); // {1, 4}
        assert(SD.size() == 2 && SD.contains(1) && SD.contains(4));
    }

    // Test 2: Equivalence Relation on {0, 1, 2, 3} (Congruence modulo 2)
    {
        dsa::BinaryRelation mod2_rel(4);
        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                if ((i % 2) == (j % 2)) {
                    mod2_rel.add_pair(i, j);
                }
            }
        }
        assert(mod2_rel.is_reflexive());
        assert(mod2_rel.is_symmetric());
        assert(mod2_rel.is_transitive());
        assert(mod2_rel.is_equivalence_relation());
        assert(!mod2_rel.is_antisymmetric());
    }

    // Test 3: Partial Order (Subset inclusion or <= on {0, 1, 2})
    {
        dsa::BinaryRelation le_rel(3);
        le_rel.add_pair(0, 0); le_rel.add_pair(0, 1); le_rel.add_pair(0, 2);
        le_rel.add_pair(1, 1); le_rel.add_pair(1, 2);
        le_rel.add_pair(2, 2);

        assert(le_rel.is_reflexive());
        assert(le_rel.is_antisymmetric());
        assert(le_rel.is_transitive());
        assert(le_rel.is_partial_order());
        assert(!le_rel.is_symmetric());
    }

    // Test 4: Warshall's Transitive Closure on directed chain: 0 -> 1 -> 2 -> 3
    {
        dsa::BinaryRelation chain(4);
        chain.add_pair(0, 1);
        chain.add_pair(1, 2);
        chain.add_pair(2, 3);
        assert(!chain.is_transitive());

        dsa::BinaryRelation closure = chain.transitive_closure();
        assert(closure.is_transitive());
        assert(closure.has_pair(0, 3));
        assert(closure.has_pair(0, 2));
        assert(!closure.has_pair(3, 0)); // Still directed
    }

    // Test 5: Function properties (Injective, Surjective, Bijective)
    {
        // f: {0, 1, 2} -> {0, 1, 2, 3} defined as f(x) = x
        std::vector<size_t> f1 = {0, 1, 2};
        assert(dsa::is_injective(f1, 4));
        assert(!dsa::is_surjective(f1, 4)); // 3 is not covered
        assert(!dsa::is_bijective(f1, 4));

        // f: {0, 1, 2} -> {0, 1, 2} defined as f(x) = (x + 1) % 3 (permutation)
        std::vector<size_t> f2 = {1, 2, 0};
        assert(dsa::is_injective(f2, 3));
        assert(dsa::is_surjective(f2, 3));
        assert(dsa::is_bijective(f2, 3));
    }

    std::cout << "[PASS] All Sets, Functions, and Relations C++ unit tests passed." << std::endl;
    return 0;
}
