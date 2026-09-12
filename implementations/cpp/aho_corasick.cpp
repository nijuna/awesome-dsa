/**
 * Reference Implementation: Aho-Corasick Multi-Pattern String Matching
 * Demonstrates:
 * 1. Trie construction for a dictionary of patterns - O(sum |P_i|)
 * 2. Breadth-First Search (BFS) failure-link (suffix link) construction
 * 3. Full Deterministic Finite Automaton (DFA) transition matrix computation
 * 4. Output list propagation for nested and overlapping pattern occurrences
 * 5. Dictionary links (compressed suffix links)
 * 6. Linear-time text scanning without text backtracking - O(n + matches)
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <string>
#include <queue>
#include <array>
#include <algorithm>
#include <cassert>
#include <random>

namespace aho_corasick {

struct Match {
    int start;       // 0-based starting index in text
    int pattern_id;  // 0-based pattern identifier
    std::string word; // matching pattern string

    bool operator==(const Match& other) const {
        return start == other.start && pattern_id == other.pattern_id && word == other.word;
    }
};

class AhoCorasick {
public:
    static constexpr int SIGMA = 26;
    static constexpr char BASE_CHAR = 'a';

    AhoCorasick() {
        trie_.push_back(Node()); // root node at index 0
    }

    /**
     * Adds a pattern to the dictionary trie.
     * Returns the assigned pattern ID.
     * Time Complexity: O(|pattern|).
     */
    int add_pattern(const std::string& pattern) {
        if (pattern.empty()) {
            return -1;
        }

        int u = 0;
        for (char ch : pattern) {
            int c = char_to_index(ch);
            if (trie_[u].next[c] == -1) {
                trie_[u].next[c] = static_cast<int>(trie_.size());
                trie_.push_back(Node());
            }
            u = trie_[u].next[c];
        }

        int id = static_cast<int>(patterns_.size());
        patterns_.push_back(pattern);
        trie_[u].out.push_back(id);
        return id;
    }

    /**
     * Computes failure links and builds the full DFA transition table via BFS.
     * Time Complexity: O(sum |P_i| * SIGMA).
     */
    void build() {
        std::queue<int> q;

        // Base cases: root children have fail = 0
        for (int c = 0; c < SIGMA; ++c) {
            int v = trie_[0].next[c];
            if (v != -1) {
                trie_[v].fail = 0;
                trie_[v].dict_link = 0;
                q.push(v);
            } else {
                trie_[0].next[c] = 0; // root transitions back to root
            }
        }

        // BFS level by level
        while (!q.empty()) {
            int u = q.front();
            q.pop();

            int f = trie_[u].fail;

            // Maintain dictionary links (closest terminal node in failure chain)
            if (!trie_[f].out.empty()) {
                trie_[u].dict_link = f;
            } else {
                trie_[u].dict_link = trie_[f].dict_link;
            }

            // Propagate outputs from failure link for direct reporting
            for (int id : trie_[f].out) {
                trie_[u].out.push_back(id);
            }

            for (int c = 0; c < SIGMA; ++c) {
                int v = trie_[u].next[c];
                if (v != -1) {
                    trie_[v].fail = trie_[f].next[c];
                    q.push(v);
                } else {
                    // Full DFA transition: missing edges jump to failure transitions
                    trie_[u].next[c] = trie_[f].next[c];
                }
            }
        }
    }

    /**
     * Finds all occurrences of dictionary patterns in text.
     * Time Complexity: O(|text| + matches). Auxiliary Space: O(matches).
     */
    std::vector<Match> search(const std::string& text) const {
        std::vector<Match> matches;
        int state = 0;

        for (int i = 0; i < static_cast<int>(text.size()); ++i) {
            int c = char_to_index(text[i]);
            state = trie_[state].next[c];

            // Report all patterns ending at current position i
            for (int id : trie_[state].out) {
                int start = i - static_cast<int>(patterns_[id].size()) + 1;
                matches.push_back(Match{start, id, patterns_[id]});
            }
        }

        return matches;
    }

    /**
     * Checks whether the text contains at least one dictionary pattern.
     * Early terminates upon first discovery.
     * Time Complexity: O(|text|).
     */
    bool contains_any(const std::string& text) const {
        int state = 0;
        for (char ch : text) {
            int c = char_to_index(ch);
            state = trie_[state].next[c];
            if (!trie_[state].out.empty()) {
                return true;
            }
        }
        return false;
    }

    /**
     * Returns total pattern occurrences in text.
     * Time Complexity: O(|text|).
     */
    int count_matches(const std::string& text) const {
        int total = 0;
        int state = 0;
        for (char ch : text) {
            int c = char_to_index(ch);
            state = trie_[state].next[c];
            total += static_cast<int>(trie_[state].out.size());
        }
        return total;
    }

    const std::vector<std::string>& patterns() const {
        return patterns_;
    }

private:
    struct Node {
        std::array<int, SIGMA> next;
        int fail;
        int dict_link;
        std::vector<int> out;

        Node() : fail(0), dict_link(0) {
            next.fill(-1);
        }
    };

    std::vector<Node> trie_;
    std::vector<std::string> patterns_;

    static int char_to_index(char c) {
        if (c >= 'a' && c <= 'z') return c - 'a';
        if (c >= 'A' && c <= 'Z') return c - 'A';
        return 0; // fallback for non-alphabetic
    }
};

} // namespace aho_corasick

// ============================================================================
// Unit Tests & Invariant Verification
// ============================================================================

int main() {
    using namespace aho_corasick;

    std::cout << "Running Aho-Corasick C++17 unit tests...\n";

    // ------------------------------------------------------------------------
    // Test 1: Arthur's Classic Worked Example
    // Patterns: {"he", "she", "his", "hers"}
    // Text: "ushers"
    // "ushers" contains:
    // - "she" ending at index 3 (start 1)
    // - "he"  ending at index 3 (start 2)
    // - "hers" ending at index 5 (start 2)
    // ------------------------------------------------------------------------
    {
        AhoCorasick ac;
        int id_he   = ac.add_pattern("he");
        int id_she  = ac.add_pattern("she");
        int id_his  = ac.add_pattern("his");
        int id_hers = ac.add_pattern("hers");

        assert(id_he == 0 && id_she == 1 && id_his == 2 && id_hers == 3);
        ac.build();

        std::string text = "ushers";
        auto matches = ac.search(text);

        assert(matches.size() == 3);
        // Sort matches by start index, then pattern_id for deterministic comparison
        std::sort(matches.begin(), matches.end(), [](const Match& a, const Match& b) {
            if (a.start != b.start) return a.start < b.start;
            return a.pattern_id < b.pattern_id;
        });

        assert(matches[0].word == "she" && matches[0].start == 1);
        assert(matches[1].word == "he" && matches[1].start == 2);
        assert(matches[2].word == "hers" && matches[2].start == 2);

        assert(ac.count_matches(text) == 3);
        assert(ac.contains_any(text) == true);
    }

    // ------------------------------------------------------------------------
    // Test 2: Nested & Heavily Overlapping Patterns
    // Patterns: {"a", "aa", "aaa"}
    // Text: "aaaa"
    // Expected occurrences:
    // - "a" at 0, 1, 2, 3 (4 matches)
    // - "aa" at 0, 1, 2   (3 matches)
    // - "aaa" at 0, 1     (2 matches)
    // Total = 9 matches
    // ------------------------------------------------------------------------
    {
        AhoCorasick ac;
        ac.add_pattern("a");
        ac.add_pattern("aa");
        ac.add_pattern("aaa");
        ac.build();

        std::string text = "aaaa";
        auto matches = ac.search(text);
        assert(matches.size() == 9);
        assert(ac.count_matches(text) == 9);

        int count_a = 0, count_aa = 0, count_aaa = 0;
        for (const auto& m : matches) {
            if (m.word == "a") count_a++;
            if (m.word == "aa") count_aa++;
            if (m.word == "aaa") count_aaa++;
        }
        assert(count_a == 4);
        assert(count_aa == 3);
        assert(count_aaa == 2);
    }

    // ------------------------------------------------------------------------
    // Test 3: No Matches & Boundary Edge Cases
    // ------------------------------------------------------------------------
    {
        AhoCorasick ac;
        ac.add_pattern("xyz");
        ac.add_pattern("hello");
        ac.build();

        assert(ac.search("abcdef").empty());
        assert(ac.count_matches("abcdef") == 0);
        assert(ac.contains_any("abcdef") == false);

        // Empty text
        assert(ac.search("").empty());
        assert(ac.count_matches("") == 0);
        assert(ac.contains_any("") == false);

        // Single character text & pattern
        AhoCorasick ac_single;
        ac_single.add_pattern("z");
        ac_single.build();
        assert(ac_single.search("z").size() == 1);
        assert(ac_single.search("a").empty());
    }

    // ------------------------------------------------------------------------
    // Test 4: Random Cross-Validation Against Naive String Search
    // ------------------------------------------------------------------------
    {
        std::mt19937 rng(1337);
        std::uniform_int_distribution<int> char_dist(0, 3); // small alphabet 'a'-'d'

        std::vector<std::string> dict;
        for (int p = 0; p < 10; ++p) {
            int len = 2 + (p % 4);
            std::string pat;
            for (int i = 0; i < len; ++i) pat.push_back(static_cast<char>('a' + char_dist(rng)));
            dict.push_back(pat);
        }

        AhoCorasick ac;
        for (const auto& pat : dict) {
            ac.add_pattern(pat);
        }
        ac.build();

        for (int iter = 0; iter < 50; ++iter) {
            int text_len = 100;
            std::string text;
            for (int i = 0; i < text_len; ++i) text.push_back(static_cast<char>('a' + char_dist(rng)));

            auto ac_matches = ac.search(text);

            // Brute-force reference
            std::vector<Match> brute_matches;
            for (int pid = 0; pid < static_cast<int>(dict.size()); ++pid) {
                const auto& pat = dict[pid];
                size_t pos = text.find(pat, 0);
                while (pos != std::string::npos) {
                    brute_matches.push_back(Match{static_cast<int>(pos), pid, pat});
                    pos = text.find(pat, pos + 1);
                }
            }

            auto sort_fn = [](const Match& a, const Match& b) {
                if (a.start != b.start) return a.start < b.start;
                return a.pattern_id < b.pattern_id;
            };

            std::sort(ac_matches.begin(), ac_matches.end(), sort_fn);
            std::sort(brute_matches.begin(), brute_matches.end(), sort_fn);

            assert(ac_matches.size() == brute_matches.size());
            assert(ac_matches == brute_matches);
        }
    }

    std::cout << "All Aho-Corasick C++17 unit tests passed successfully!\n";
    return 0;
}
