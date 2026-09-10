/**
 * Reference Implementation: 1-to-Max-Level Skip List
 * Demonstrates randomized geometric height generation (p = 0.5),
 * multi-level pointer skipping, expected O(log n) search/insert/delete,
 * and Redis-style sorted set mechanics.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <random>
#include <cstddef>
#include <cassert>
#include <optional>
#include <limits>

template <typename Key, typename Value>
class SkipList {
public:
    static constexpr std::size_t MAX_LEVEL = 16;
    static constexpr float P = 0.5f;

    struct Node {
        Key key;
        Value val;
        std::vector<Node*> forward;

        Node(const Key& k, const Value& v, std::size_t level)
            : key(k), val(v), forward(level, nullptr) {}
    };

private:
    Node* head_;
    std::size_t current_level_;
    std::size_t size_;
    std::mt19937 rng_;
    std::uniform_real_distribution<float> dist_;

    std::size_t random_level() {
        std::size_t lvl = 1;
        while (dist_(rng_) < P && lvl < MAX_LEVEL) {
            ++lvl;
        }
        return lvl;
    }

public:
    SkipList()
        : current_level_(1), size_(0), rng_(1337), dist_(0.0f, 1.0f) {
        // Sentinel head node holding dummy values
        head_ = new Node(Key{}, Value{}, MAX_LEVEL);
    }

    ~SkipList() {
        Node* curr = head_;
        while (curr != nullptr) {
            Node* next = curr->forward[0];
            delete curr;
            curr = next;
        }
    }

    // Non-copyable for simplicity, but movable
    SkipList(const SkipList&) = delete;
    SkipList& operator=(const SkipList&) = delete;

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }
    [[nodiscard]] std::size_t current_level() const noexcept { return current_level_; }

    std::optional<Value> find(const Key& key) const {
        Node* curr = head_;
        for (int i = static_cast<int>(current_level_) - 1; i >= 0; --i) {
            while (curr->forward[i] != nullptr && curr->forward[i]->key < key) {
                curr = curr->forward[i];
            }
        }
        curr = curr->forward[0];
        if (curr != nullptr && curr->key == key) {
            return curr->val;
        }
        return std::nullopt;
    }

    bool insert(const Key& key, const Value& val) {
        std::vector<Node*> update(MAX_LEVEL, nullptr);
        Node* curr = head_;

        for (int i = static_cast<int>(current_level_) - 1; i >= 0; --i) {
            while (curr->forward[i] != nullptr && curr->forward[i]->key < key) {
                curr = curr->forward[i];
            }
            update[i] = curr;
        }
        curr = curr->forward[0];

        // Key already exists: update value
        if (curr != nullptr && curr->key == key) {
            curr->val = val;
            return false;
        }

        std::size_t new_level = random_level();
        if (new_level > current_level_) {
            for (std::size_t i = current_level_; i < new_level; ++i) {
                update[i] = head_;
            }
            current_level_ = new_level;
        }

        Node* new_node = new Node(key, val, new_level);
        for (std::size_t i = 0; i < new_level; ++i) {
            new_node->forward[i] = update[i]->forward[i];
            update[i]->forward[i] = new_node;
        }
        ++size_;
        return true;
    }

    bool erase(const Key& key) {
        std::vector<Node*> update(MAX_LEVEL, nullptr);
        Node* curr = head_;

        for (int i = static_cast<int>(current_level_) - 1; i >= 0; --i) {
            while (curr->forward[i] != nullptr && curr->forward[i]->key < key) {
                curr = curr->forward[i];
            }
            update[i] = curr;
        }
        curr = curr->forward[0];

        if (curr == nullptr || curr->key != key) {
            return false; // Key not found
        }

        for (std::size_t i = 0; i < current_level_; ++i) {
            if (update[i]->forward[i] != curr) break;
            update[i]->forward[i] = curr->forward[i];
        }
        delete curr;

        // Lower current_level if top levels are empty
        while (current_level_ > 1 && head_->forward[current_level_ - 1] == nullptr) {
            --current_level_;
        }
        --size_;
        return true;
    }

    std::vector<std::pair<Key, Value>> to_vector() const {
        std::vector<std::pair<Key, Value>> res;
        Node* curr = head_->forward[0];
        while (curr != nullptr) {
            res.emplace_back(curr->key, curr->val);
            curr = curr->forward[0];
        }
        return res;
    }
};

void run_tests() {
    SkipList<int, std::string> sl;
    assert(sl.empty());
    assert(sl.size() == 0);

    // Insert keys
    assert(sl.insert(10, "ten"));
    assert(sl.insert(20, "twenty"));
    assert(sl.insert(5, "five"));
    assert(sl.insert(15, "fifteen"));
    assert(sl.size() == 4);

    // Find queries
    assert(sl.find(10).value() == "ten");
    assert(sl.find(20).value() == "twenty");
    assert(sl.find(5).value() == "five");
    assert(sl.find(15).value() == "fifteen");
    assert(!sl.find(100).has_value());

    // Update existing key
    assert(!sl.insert(10, "TEN_UPDATED"));
    assert(sl.size() == 4);
    assert(sl.find(10).value() == "TEN_UPDATED");

    // Ordered traversal check (forward[0] must be strictly sorted)
    auto vec = sl.to_vector();
    assert(vec.size() == 4);
    assert(vec[0].first == 5);
    assert(vec[1].first == 10);
    assert(vec[2].first == 15);
    assert(vec[3].first == 20);

    // Deletion
    assert(sl.erase(15));
    assert(sl.size() == 3);
    assert(!sl.find(15).has_value());
    assert(!sl.erase(999)); // Absent key

    // Stress test with 1000 items
    for (int i = 100; i < 1100; ++i) {
        sl.insert(i, "val_" + std::to_string(i));
    }
    assert(sl.size() == 1003);

    for (int i = 100; i < 1100; ++i) {
        assert(sl.find(i).value() == "val_" + std::to_string(i));
    }

    std::cout << "[PASS] All SkipList C++ unit tests passed.\n";
}

int main() {
    run_tests();
    return 0;
}
