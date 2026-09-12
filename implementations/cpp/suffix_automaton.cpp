/**
 * @file suffix_automaton.cpp
 * @brief Reference implementation of Suffix Automaton (DAWG) for linear-time string analysis.
 *
 * Implements online construction with state cloning, distinct substring counting,
 * O(|P|) pattern matching, longest common substring, and differential oracle testing.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>

namespace dsa {

class SuffixAutomaton {
public:
    struct State {
        int len = 0;
        int link = -1;
        std::map<char, int> next;
        bool is_clone = false;
    };

private:
    std::vector<State> st_;
    int last_ = 0;

public:
    SuffixAutomaton() {
        st_.emplace_back(); // Root state 0
        st_[0].len = 0;
        st_[0].link = -1;
        last_ = 0;
    }

    explicit SuffixAutomaton(const std::string& str) : SuffixAutomaton() {
        extend_string(str);
    }

    void extend(char c) {
        int cur = static_cast<int>(st_.size());
        st_.emplace_back();
        st_[cur].len = st_[last_].len + 1;

        int p = last_;
        while (p != -1 && st_[p].next.find(c) == st_[p].next.end()) {
            st_[p].next[c] = cur;
            p = st_[p].link;
        }

        if (p == -1) {
            // Case 1: Walk fell off root
            st_[cur].link = 0;
        } else {
            int q = st_[p].next[c];
            if (st_[p].len + 1 == st_[q].len) {
                // Case 2: Transition is continuous
                st_[cur].link = q;
            } else {
                // Case 3: Need state clone
                int clone = static_cast<int>(st_.size());
                st_.emplace_back();
                st_[clone].len = st_[p].len + 1;
                st_[clone].next = st_[q].next;
                st_[clone].link = st_[q].link;
                st_[clone].is_clone = true;

                // Redirect transitions
                while (p != -1 && st_[p].next[c] == q) {
                    st_[p].next[c] = clone;
                    p = st_[p].link;
                }

                st_[q].link = clone;
                st_[cur].link = clone;
            }
        }
        last_ = cur;
    }

    void extend_string(const std::string& str) {
        for (char c : str) {
            extend(c);
        }
    }

    // --- Two-Layer API Standard ---

    /**
     * @brief Layer A: Fast pattern search: O(|P|) time.
     */
    bool contains(const std::string& pattern) const {
        int curr = 0;
        for (char c : pattern) {
            auto it = st_[curr].next.find(c);
            if (it == st_[curr].next.end()) return false;
            curr = it->second;
        }
        return true;
    }

    /**
     * @brief Layer B: Safe transition query adapter.
     * Returns pointer to next State or nullptr if no transition exists.
     */
    const State* try_transition(int state_idx, char c) const {
        if (state_idx < 0 || state_idx >= static_cast<int>(st_.size())) {
            return nullptr;
        }
        auto it = st_[state_idx].next.find(c);
        if (it == st_[state_idx].next.end()) {
            return nullptr;
        }
        return &st_[it->second];
    }

    /**
     * @brief Computes total distinct substrings in O(States) = O(N) time.
     * Formula: sum_{v != 0} (len[v] - len[link[v]]).
     */
    int64_t count_distinct_substrings() const {
        int64_t count = 0;
        for (size_t v = 1; v < st_.size(); ++v) {
            count += st_[v].len - st_[st_[v].link].len;
        }
        return count;
    }

    /**
     * @brief Computes the longest common substring between the indexed string and other.
     * Runs in O(|other|) time!
     */
    std::string longest_common_substring(const std::string& other) const {
        int v = 0;
        int l = 0;
        int best_len = 0;
        int best_pos = 0; // Ending position in other

        for (int i = 0; i < static_cast<int>(other.size()); ++i) {
            char c = other[i];
            while (v != 0 && st_[v].next.find(c) == st_[v].next.end()) {
                v = st_[v].link;
                l = st_[v].len;
            }
            auto it = st_[v].next.find(c);
            if (it != st_[v].next.end()) {
                v = it->second;
                l++;
            }
            if (l > best_len) {
                best_len = l;
                best_pos = i;
            }
        }

        if (best_len == 0) return "";
        return other.substr(best_pos - best_len + 1, best_len);
    }

    /**
     * @brief Validates core structural invariants.
     */
    bool verify_invariants(size_t original_str_len) const {
        // Invariant 1: State bound <= 2N - 1 (for N >= 1)
        if (original_str_len > 0 && st_.size() > 2 * original_str_len) return false;

        // Invariant 2: Suffix link lengths strictly decrease
        for (size_t v = 1; v < st_.size(); ++v) {
            int link_v = st_[v].link;
            if (link_v < 0 || link_v >= static_cast<int>(st_.size())) return false;
            if (st_[v].len <= st_[link_v].len) return false;
        }

        return true;
    }

    size_t state_count() const { return st_.size(); }
};

} // namespace dsa

int main() {
    std::cout << "Running Suffix Automaton verification..." << std::endl;

    // 1. Classical example "aab"
    std::string s1 = "aab";
    dsa::SuffixAutomaton sam1(s1);
    assert(sam1.verify_invariants(s1.size()));

    // Distinct substrings of "aab": "", "a", "aa", "aab", "ab", "b" -> 5 non-empty
    assert(sam1.count_distinct_substrings() == 5);
    assert(sam1.contains("a"));
    assert(sam1.contains("aa"));
    assert(sam1.contains("aab"));
    assert(sam1.contains("ab"));
    assert(sam1.contains("b"));
    assert(!sam1.contains("ba"));
    assert(!sam1.contains("aaa"));

    // 2. Longest Common Substring
    std::string text1 = "algorithm_wizardry";
    std::string text2 = "super_algorithm_engine";
    dsa::SuffixAutomaton sam_lcs(text1);
    std::string lcs = sam_lcs.longest_common_substring(text2);
    assert(lcs == "algorithm_");

    // 3. Differential Testing against O(N^2) Substring Oracle
    std::mt19937 rng(42);
    std::string alphabet = "abc"; // Small alphabet causes many clone splits!

    for (int test = 0; test < 100; ++test) {
        size_t len = std::uniform_int_distribution<size_t>(1, 30)(rng);
        std::string rand_str;
        for (size_t i = 0; i < len; ++i) {
            rand_str += alphabet[rng() % alphabet.size()];
        }

        // Build SAM
        dsa::SuffixAutomaton sam(rand_str);
        assert(sam.verify_invariants(rand_str.size()));

        // Build ground-truth substring set
        std::set<std::string> oracle_subs;
        for (size_t i = 0; i < rand_str.size(); ++i) {
            std::string sub;
            for (size_t j = i; j < rand_str.size(); ++j) {
                sub += rand_str[j];
                oracle_subs.insert(sub);
            }
        }

        // Check distinct substring count equivalence
        assert(static_cast<size_t>(sam.count_distinct_substrings()) == oracle_subs.size());

        // Check that all oracle substrings are accepted
        for (const auto& sub : oracle_subs) {
            assert(sam.contains(sub));
        }

        // Check that a mutated negative string is rejected
        std::string fake = rand_str + "z";
        assert(!sam.contains(fake));
    }

    std::cout << "[PASS] Verified distinct substring count and O(|P|) pattern matching on 'aab'." << std::endl;
    std::cout << "[PASS] Longest common substring verified ('algorithm_')." << std::endl;
    std::cout << "[PASS] 100 randomized differential trials against O(N^2) substring set passed." << std::endl;
    std::cout << "All Suffix Automaton assertions passed successfully!" << std::endl;
    return 0;
}
