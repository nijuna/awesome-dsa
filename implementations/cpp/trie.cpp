/**
 * Reference Implementation: Prefix Trie with Autocomplete & Longest Prefix Match
 * Demonstrates character-by-character prefix descent, O(L) exact search,
 * starts_with queries, longest prefix matching (LPM for routing),
 * autocomplete enumeration, and pruning deletion.
 *
 * Language: C++17
 */

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <cassert>
#include <algorithm>

class Trie {
public:
    struct Node {
        bool is_terminal = false;
        std::unordered_map<char, std::unique_ptr<Node>> children;
    };

private:
    std::unique_ptr<Node> root_;
    std::size_t size_;

    bool erase_helper(Node* curr, const std::string& word, std::size_t depth, bool& erased) {
        if (!curr) return false;

        if (depth == word.size()) {
            if (!curr->is_terminal) {
                erased = false;
                return false;
            }
            curr->is_terminal = false;
            erased = true;
            // Can delete node if it has no children
            return curr->children.empty();
        }

        char ch = word[depth];
        auto it = curr->children.find(ch);
        if (it == curr->children.end()) {
            erased = false;
            return false;
        }

        bool should_delete_child = erase_helper(it->second.get(), word, depth + 1, erased);
        if (should_delete_child) {
            curr->children.erase(it);
        }

        // Return true if this node itself is non-terminal and has no children left
        return !curr->is_terminal && curr->children.empty();
    }

    void collect_words(const Node* curr, std::string& current_prefix, std::vector<std::string>& out) const {
        if (!curr) return;
        if (curr->is_terminal) {
            out.push_back(current_prefix);
        }
        for (const auto& [ch, child] : curr->children) {
            current_prefix.push_back(ch);
            collect_words(child.get(), current_prefix, out);
            current_prefix.pop_back();
        }
    }

public:
    Trie() : root_(std::make_unique<Node>()), size_(0) {}

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    void insert(const std::string& word) {
        Node* curr = root_.get();
        for (char ch : word) {
            auto& child = curr->children[ch];
            if (!child) {
                child = std::make_unique<Node>();
            }
            curr = child.get();
        }
        if (!curr->is_terminal) {
            curr->is_terminal = true;
            ++size_;
        }
    }

    [[nodiscard]] bool search(const std::string& word) const {
        const Node* curr = root_.get();
        for (char ch : word) {
            auto it = curr->children.find(ch);
            if (it == curr->children.end()) {
                return false;
            }
            curr = it->second.get();
        }
        return curr->is_terminal;
    }

    [[nodiscard]] bool starts_with(const std::string& prefix) const {
        const Node* curr = root_.get();
        for (char ch : prefix) {
            auto it = curr->children.find(ch);
            if (it == curr->children.end()) {
                return false;
            }
            curr = it->second.get();
        }
        return true;
    }

    // Longest Prefix Match (LPM): Finds the longest stored key that is a prefix of query
    [[nodiscard]] std::string longest_prefix_match(const std::string& query) const {
        const Node* curr = root_.get();
        std::size_t longest_len = 0;
        std::size_t current_len = 0;

        if (curr->is_terminal) {
            longest_len = 0;
        }

        for (char ch : query) {
            auto it = curr->children.find(ch);
            if (it == curr->children.end()) {
                break;
            }
            curr = it->second.get();
            ++current_len;
            if (curr->is_terminal) {
                longest_len = current_len;
            }
        }

        return query.substr(0, longest_len);
    }

    bool erase(const std::string& word) {
        bool erased = false;
        erase_helper(root_.get(), word, 0, erased);
        if (erased) {
            --size_;
        }
        return erased;
    }

    // Autocomplete: Returns all stored words beginning with prefix
    [[nodiscard]] std::vector<std::string> words_with_prefix(const std::string& prefix) const {
        const Node* curr = root_.get();
        for (char ch : prefix) {
            auto it = curr->children.find(ch);
            if (it == curr->children.end()) {
                return {};
            }
            curr = it->second.get();
        }

        std::vector<std::string> results;
        std::string current = prefix;
        collect_words(curr, current, results);
        std::sort(results.begin(), results.end());
        return results;
    }
};

int main() {
    Trie trie;
    assert(trie.empty());
    assert(trie.size() == 0);

    // 1. Insert words
    trie.insert("to");
    trie.insert("tea");
    trie.insert("ten");
    trie.insert("ted");
    trie.insert("in");
    trie.insert("inn");

    assert(trie.size() == 6);
    assert(!trie.empty());

    // 2. Exact search
    assert(trie.search("to"));
    assert(trie.search("tea"));
    assert(trie.search("ten"));
    assert(trie.search("ted"));
    assert(trie.search("in"));
    assert(trie.search("inn"));

    assert(!trie.search("t"));     // Prefix exists, but not terminal
    assert(!trie.search("te"));    // Prefix exists, but not terminal
    assert(!trie.search("toast")); // Absent
    assert(!trie.search("inner")); // Absent

    // 3. Prefix search (starts_with)
    assert(trie.starts_with("t"));
    assert(trie.starts_with("te"));
    assert(trie.starts_with("tea"));
    assert(trie.starts_with("in"));
    assert(!trie.starts_with("ta"));
    assert(!trie.starts_with("out"));

    // 4. Autocomplete
    std::vector<std::string> te_words = trie.words_with_prefix("te");
    std::vector<std::string> expected_te = {"tea", "ted", "ten"};
    assert(te_words == expected_te);

    std::vector<std::string> in_words = trie.words_with_prefix("in");
    std::vector<std::string> expected_in = {"in", "inn"};
    assert(in_words == expected_in);

    assert(trie.words_with_prefix("xyz").empty());

    // 5. Longest Prefix Matching (CIDR / Routing style)
    trie.insert("192.168.");
    trie.insert("192.168.1.");
    trie.insert("192.168.1.0/24");

    assert(trie.longest_prefix_match("192.168.1.100") == "192.168.1.");
    assert(trie.longest_prefix_match("192.168.2.50") == "192.168.");
    assert(trie.longest_prefix_match("10.0.0.1") == "");

    // 6. Deletion and node pruning
    assert(trie.erase("inn"));
    assert(!trie.search("inn"));
    assert(trie.search("in")); // Sibling/parent prefix preserved

    assert(trie.erase("in"));
    assert(!trie.search("in"));
    assert(!trie.starts_with("in")); // Completely pruned

    assert(!trie.erase("nonexistent"));

    std::cout << "[PASS] All Trie C++ unit tests passed.\n";
    return 0;
}
