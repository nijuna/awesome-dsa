/**
 * @file merkle_trees.cpp
 * @brief Publication-grade C++17 Reference Implementation of Cryptographic Merkle Trees.
 *
 * Implements:
 * 1. Self-contained FIPS 180-4 compliant SHA-256 Cryptographic Hash Engine:
 *    - Zero external library dependencies (no OpenSSL/libcrypto required).
 * 2. RFC 6962 Domain-Separated Merkle Tree:
 *    - 0x00 byte prefix for leaf nodes (defense against second-preimage attacks).
 *    - 0x01 byte prefix for internal nodes.
 *    - Arbitrary leaf count support with canonical odd-node handling.
 * 3. Logarithmic Audit Proofs (Merkle Inclusion Proofs):
 *    - O(log N) proof generation and verification.
 *    - Explicit directional sibling path encoding (left vs. right).
 * 4. Dynamic Point Updates:
 *    - O(log N) leaf modification and ancestor path re-hashing.
 * 5. Comprehensive Verification Suite:
 *    - Valid proof verification across power-of-two and odd tree sizes.
 *    - Cryptographic tampering resistance: detects flipped bits in data, siblings, and path directions.
 *    - Dynamic update consistency checks.
 *
 * Fully compliant with -std=c++17 -O3 -Wall -Wextra -Werror -pthread.
 */

#include <iostream>
#include <vector>
#include <string>
#include <cstdint>
#include <cassert>
#include <array>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <chrono>

namespace crypto {

// ============================================================================
// 1. FIPS 180-4 SHA-256 Cryptographic Engine
// ============================================================================

class SHA256 {
private:
    static inline uint32_t rotr(uint32_t x, uint32_t n) {
        return (x >> n) | (x << (32 - n));
    }
    static inline uint32_t ch(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (~x & z);
    }
    static inline uint32_t maj(uint32_t x, uint32_t y, uint32_t z) {
        return (x & y) ^ (x & z) ^ (y & z);
    }
    static inline uint32_t sigma0(uint32_t x) {
        return rotr(x, 2) ^ rotr(x, 13) ^ rotr(x, 22);
    }
    static inline uint32_t sigma1(uint32_t x) {
        return rotr(x, 6) ^ rotr(x, 11) ^ rotr(x, 25);
    }
    static inline uint32_t gamma0(uint32_t x) {
        return rotr(x, 7) ^ rotr(x, 18) ^ (x >> 3);
    }
    static inline uint32_t gamma1(uint32_t x) {
        return rotr(x, 17) ^ rotr(x, 19) ^ (x >> 10);
    }

    static constexpr uint32_t K[64] = {
        0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
        0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
        0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
        0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
        0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
        0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
        0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
        0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
    };

public:
    using Digest = std::array<uint8_t, 32>;

    static Digest hash(const uint8_t* data, size_t len) {
        uint32_t H[8] = {
            0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
            0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19
        };

        size_t bit_len = len * 8;
        std::vector<uint8_t> padded(data, data + len);
        padded.push_back(0x80);
        while ((padded.size() % 64) != 56) {
            padded.push_back(0x00);
        }
        for (int i = 7; i >= 0; --i) {
            padded.push_back(static_cast<uint8_t>((bit_len >> (i * 8)) & 0xFF));
        }

        for (size_t chunk = 0; chunk < padded.size(); chunk += 64) {
            uint32_t W[64];
            for (size_t i = 0; i < 16; ++i) {
                W[i] = (static_cast<uint32_t>(padded[chunk + i * 4]) << 24) |
                       (static_cast<uint32_t>(padded[chunk + i * 4 + 1]) << 16) |
                       (static_cast<uint32_t>(padded[chunk + i * 4 + 2]) << 8) |
                       (static_cast<uint32_t>(padded[chunk + i * 4 + 3]));
            }
            for (size_t i = 16; i < 64; ++i) {
                W[i] = gamma1(W[i - 2]) + W[i - 7] + gamma0(W[i - 15]) + W[i - 16];
            }

            uint32_t a = H[0], b = H[1], c = H[2], d = H[3];
            uint32_t e = H[4], f = H[5], g = H[6], h = H[7];

            for (size_t i = 0; i < 64; ++i) {
                uint32_t T1 = h + sigma1(e) + ch(e, f, g) + K[i] + W[i];
                uint32_t T2 = sigma0(a) + maj(a, b, c);
                h = g;
                g = f;
                f = e;
                e = d + T1;
                d = c;
                c = b;
                b = a;
                a = T1 + T2;
            }

            H[0] += a; H[1] += b; H[2] += c; H[3] += d;
            H[4] += e; H[5] += f; H[6] += g; H[7] += h;
        }

        Digest out;
        for (size_t i = 0; i < 8; ++i) {
            out[i * 4] = static_cast<uint8_t>((H[i] >> 24) & 0xFF);
            out[i * 4 + 1] = static_cast<uint8_t>((H[i] >> 16) & 0xFF);
            out[i * 4 + 2] = static_cast<uint8_t>((H[i] >> 8) & 0xFF);
            out[i * 4 + 3] = static_cast<uint8_t>(H[i] & 0xFF);
        }
        return out;
    }

    static Digest hash(const std::string& str) {
        return hash(reinterpret_cast<const uint8_t*>(str.data()), str.size());
    }

    static std::string to_hex(const Digest& digest) {
        std::ostringstream oss;
        for (uint8_t byte : digest) {
            oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte);
        }
        return oss.str();
    }
};

// ============================================================================
// 2. Merkle Tree & Inclusion Proof Architecture
// ============================================================================

struct MerkleProofStep {
    SHA256::Digest sibling_hash;
    bool is_left; // True if sibling is on the left; False if on the right
};

using MerkleProof = std::vector<MerkleProofStep>;

class MerkleTree {
private:
    std::vector<std::string> raw_leaves_;
    std::vector<std::vector<SHA256::Digest>> levels_;

    // RFC 6962 Domain Separation Prefixes
    static constexpr uint8_t LEAF_PREFIX = 0x00;
    static constexpr uint8_t NODE_PREFIX = 0x01;

public:
    MerkleTree() = default;

    explicit MerkleTree(const std::vector<std::string>& leaves) {
        build(leaves);
    }

    /**
     * @brief Computes leaf hash with RFC 6962 domain separation prefix: H(0x00 || data).
     */
    static SHA256::Digest hash_leaf(const std::string& data) {
        std::vector<uint8_t> payload;
        payload.reserve(1 + data.size());
        payload.push_back(LEAF_PREFIX);
        payload.insert(payload.end(), data.begin(), data.end());
        return SHA256::hash(payload.data(), payload.size());
    }

    /**
     * @brief Computes parent hash with RFC 6962 prefix: H(0x01 || left || right).
     */
    static SHA256::Digest hash_node(const SHA256::Digest& left, const SHA256::Digest& right) {
        std::vector<uint8_t> payload;
        payload.reserve(1 + 32 + 32);
        payload.push_back(NODE_PREFIX);
        payload.insert(payload.end(), left.begin(), left.end());
        payload.insert(payload.end(), right.begin(), right.end());
        return SHA256::hash(payload.data(), payload.size());
    }

    void build(const std::vector<std::string>& leaves) {
        raw_leaves_ = leaves;
        levels_.clear();

        if (leaves.empty()) {
            levels_.push_back({SHA256::hash("")});
            return;
        }

        // Level 0: Hash all leaves
        std::vector<SHA256::Digest> current_level;
        current_level.reserve(leaves.size());
        for (const auto& leaf : leaves) {
            current_level.push_back(hash_leaf(leaf));
        }
        levels_.push_back(current_level);

        // Build upper levels bottom-up
        while (current_level.size() > 1) {
            std::vector<SHA256::Digest> next_level;
            next_level.reserve((current_level.size() + 1) / 2);

            for (size_t i = 0; i < current_level.size(); i += 2) {
                if (i + 1 < current_level.size()) {
                    next_level.push_back(hash_node(current_level[i], current_level[i + 1]));
                } else {
                    // Canonical carry-over for odd count
                    next_level.push_back(hash_node(current_level[i], current_level[i]));
                }
            }
            levels_.push_back(next_level);
            current_level = std::move(next_level);
        }
    }

    SHA256::Digest get_root() const {
        assert(!levels_.empty() && !levels_.back().empty());
        return levels_.back()[0];
    }

    std::string get_root_hex() const {
        return SHA256::to_hex(get_root());
    }

    size_t size() const {
        return raw_leaves_.size();
    }

    /**
     * @brief Generates an O(log N) Merkle Inclusion Proof for leaf at index.
     */
    MerkleProof generate_inclusion_proof(size_t leaf_index) const {
        assert(leaf_index < raw_leaves_.size());
        MerkleProof proof;

        size_t idx = leaf_index;
        for (size_t l = 0; l < levels_.size() - 1; ++l) {
            const auto& level = levels_[l];
            bool is_right_child = (idx % 2 == 1);

            if (is_right_child) {
                // Sibling is to the left
                proof.push_back({level[idx - 1], true});
            } else {
                // Sibling is to the right (or duplicate if odd)
                if (idx + 1 < level.size()) {
                    proof.push_back({level[idx + 1], false});
                } else {
                    proof.push_back({level[idx], false});
                }
            }
            idx /= 2;
        }

        return proof;
    }

    /**
     * @brief Verifies an O(log N) Merkle Inclusion Proof against a trusted root.
     * Time: O(log N) hash operations.
     */
    static bool verify_inclusion_proof(
        const SHA256::Digest& root,
        const std::string& leaf_data,
        const MerkleProof& proof) {

        SHA256::Digest current = hash_leaf(leaf_data);

        for (const auto& step : proof) {
            if (step.is_left) {
                current = hash_node(step.sibling_hash, current);
            } else {
                current = hash_node(current, step.sibling_hash);
            }
        }

        return current == root;
    }

    /**
     * @brief Dynamically updates a leaf and recalculates path hashes to root.
     * Time: O(log N).
     */
    SHA256::Digest update_leaf(size_t leaf_index, const std::string& new_data) {
        assert(leaf_index < raw_leaves_.size());
        raw_leaves_[leaf_index] = new_data;

        // Update Level 0
        levels_[0][leaf_index] = hash_leaf(new_data);

        // Recalculate ancestors along the path
        size_t idx = leaf_index;
        for (size_t l = 0; l < levels_.size() - 1; ++l) {
            size_t parent_idx = idx / 2;
            size_t left_child = parent_idx * 2;
            size_t right_child = left_child + 1;

            if (right_child < levels_[l].size()) {
                levels_[l + 1][parent_idx] = hash_node(levels_[l][left_child], levels_[l][right_child]);
            } else {
                levels_[l + 1][parent_idx] = hash_node(levels_[l][left_child], levels_[l][left_child]);
            }
            idx = parent_idx;
        }

        return get_root();
    }
};

} // namespace crypto

// ============================================================================
// 3. Verification & Tampering Resistance Test Suite
// ============================================================================

void test_basic_construction() {
    std::cout << "[Test 1/5] Testing basic Merkle Tree construction & root stability...\n";

    std::vector<std::string> leaves = {"alice", "bob", "carol", "dave"};
    crypto::MerkleTree tree(leaves);

    std::string root_hex = tree.get_root_hex();
    assert(root_hex.size() == 64);

    // Reconstructing with same leaves yields exact identical deterministic root
    crypto::MerkleTree tree2(leaves);
    assert(tree.get_root() == tree2.get_root());

    std::cout << "  -> Deterministic Root: " << root_hex << "\n";
}

void test_inclusion_proofs_power_of_two() {
    std::cout << "[Test 2/5] Testing inclusion proofs on power-of-two leaves (N = 8)...\n";

    std::vector<std::string> leaves = {"tx0", "tx1", "tx2", "tx3", "tx4", "tx5", "tx6", "tx7"};
    crypto::MerkleTree tree(leaves);
    auto root = tree.get_root();

    for (size_t i = 0; i < leaves.size(); ++i) {
        crypto::MerkleProof proof = tree.generate_inclusion_proof(i);
        assert(proof.size() == 3); // log2(8) = 3

        bool valid = crypto::MerkleTree::verify_inclusion_proof(root, leaves[i], proof);
        assert(valid == true);
    }

    std::cout << "  -> All 8 inclusion proofs verified against root in O(log N) steps.\n";
}

void test_inclusion_proofs_odd_leaf_counts() {
    std::cout << "[Test 3/5] Testing inclusion proofs on odd leaf counts (N = 7 and N = 13)...\n";

    for (size_t n : {1UL, 3UL, 5UL, 7UL, 13UL, 31UL}) {
        std::vector<std::string> leaves;
        for (size_t i = 0; i < n; ++i) {
            leaves.push_back("record_" + std::to_string(i));
        }

        crypto::MerkleTree tree(leaves);
        auto root = tree.get_root();

        for (size_t i = 0; i < n; ++i) {
            crypto::MerkleProof proof = tree.generate_inclusion_proof(i);
            bool valid = crypto::MerkleTree::verify_inclusion_proof(root, leaves[i], proof);
            assert(valid == true);
        }
    }

    std::cout << "  -> Odd and arbitrary leaf count proofs verified flawlessly.\n";
}

void test_tampering_resistance() {
    std::cout << "[Test 4/5] Testing cryptographic tampering resistance...\n";

    std::vector<std::string> leaves = {"block_A", "block_B", "block_C", "block_D"};
    crypto::MerkleTree tree(leaves);
    auto root = tree.get_root();

    // 1. Valid proof for block_B
    crypto::MerkleProof proof = tree.generate_inclusion_proof(1);
    assert(crypto::MerkleTree::verify_inclusion_proof(root, "block_B", proof) == true);

    // 2. Tampered Leaf Data: Try claiming "block_MALICIOUS" was at index 1
    assert(crypto::MerkleTree::verify_inclusion_proof(root, "block_MALICIOUS", proof) == false);

    // 3. Tampered Proof Hash: Flip a single bit in a sibling hash
    crypto::MerkleProof tampered_proof = proof;
    tampered_proof[0].sibling_hash[0] ^= 0x01;
    assert(crypto::MerkleTree::verify_inclusion_proof(root, "block_B", tampered_proof) == false);

    // 4. Tampered Direction: Invert left/right sibling positioning
    crypto::MerkleProof direction_tampered = proof;
    direction_tampered[0].is_left = !direction_tampered[0].is_left;
    assert(crypto::MerkleTree::verify_inclusion_proof(root, "block_B", direction_tampered) == false);

    std::cout << "  -> Tampering resistance proven: Bit flips and forged leaves strictly rejected.\n";
}

void test_dynamic_point_updates() {
    std::cout << "[Test 5/5] Testing dynamic point updates & path re-hashing...\n";

    std::vector<std::string> leaves = {"acc0:100", "acc1:250", "acc2:500", "acc3:75"};
    crypto::MerkleTree tree(leaves);
    auto initial_root = tree.get_root();

    // Initial proof for acc2
    auto initial_proof = tree.generate_inclusion_proof(2);
    assert(crypto::MerkleTree::verify_inclusion_proof(initial_root, "acc2:500", initial_proof) == true);

    // Dynamic update: acc2 transfers funds -> new state "acc2:420"
    auto new_root = tree.update_leaf(2, "acc2:420");
    assert(new_root != initial_root);

    // Old proof MUST fail against new root!
    assert(crypto::MerkleTree::verify_inclusion_proof(new_root, "acc2:500", initial_proof) == false);

    // New proof for updated leaf succeeds against new root
    auto new_proof = tree.generate_inclusion_proof(2);
    assert(crypto::MerkleTree::verify_inclusion_proof(new_root, "acc2:420", new_proof) == true);

    // Untouched leaf (acc1) retains valid proof against new root
    auto acc1_proof = tree.generate_inclusion_proof(1);
    assert(crypto::MerkleTree::verify_inclusion_proof(new_root, "acc1:250", acc1_proof) == true);

    std::cout << "  -> Dynamic point updates verified in O(log N) time.\n";
}

int main() {
    std::cout << "===============================================================\n";
    std::cout << "Cryptographic Merkle Tree & Audit Proof Verification\n";
    std::cout << "===============================================================\n";

    test_basic_construction();
    test_inclusion_proofs_power_of_two();
    test_inclusion_proofs_odd_leaf_counts();
    test_tampering_resistance();
    test_dynamic_point_updates();

    std::cout << "===============================================================\n";
    std::cout << "All Merkle Tree & Cryptographic Proof tests PASSED flawlessly!\n";
    std::cout << "===============================================================\n";
    return 0;
}
