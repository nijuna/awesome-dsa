/**
 * @file binomial_heaps.cpp
 * @brief Reference implementation of a Binomial Heap supporting O(log n) worst-case merge.
 *
 * Implements insert, find_min, extract_min, decrease_key, and heap merge
 * using Jean Vuillemin's binomial tree structure.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <random>

namespace dsa {

template <typename T>
class BinomialHeap {
public:
    struct Node {
        T key;
        size_t degree;
        Node* parent;
        Node* child;
        Node* sibling;

        explicit Node(const T& k)
            : key(k), degree(0), parent(nullptr), child(nullptr), sibling(nullptr) {}
    };

private:
    Node* head = nullptr;
    size_t count = 0;

    static Node* link_trees(Node* y, Node* z) {
        // Link y as child of z (assuming z->key <= y->key)
        assert(z->key <= y->key);
        y->parent = z;
        y->sibling = z->child;
        z->child = y;
        z->degree++;
        return z;
    }

    static Node* merge_root_lists(Node* h1, Node* h2) {
        if (!h1) return h2;
        if (!h2) return h1;

        Node* new_head = nullptr;
        Node** tail = &new_head;

        while (h1 && h2) {
            if (h1->degree <= h2->degree) {
                *tail = h1;
                h1 = h1->sibling;
            } else {
                *tail = h2;
                h2 = h2->sibling;
            }
            tail = &((*tail)->sibling);
        }
        *tail = (h1 ? h1 : h2);
        return new_head;
    }

    static Node* union_heaps_internal(Node* h1, Node* h2) {
        Node* merged = merge_root_lists(h1, h2);
        if (!merged) return nullptr;

        Node* prev = nullptr;
        Node* curr = merged;
        Node* next = curr->sibling;

        while (next) {
            // Case 1 & 2: degrees differ or 3 consecutive trees with matching degree
            if ((curr->degree != next->degree) ||
                (next->sibling && next->sibling->degree == curr->degree)) {
                prev = curr;
                curr = next;
            } else {
                // Case 3 & 4: curr and next have matching degree
                if (curr->key <= next->key) {
                    curr->sibling = next->sibling;
                    link_trees(next, curr);
                } else {
                    if (!prev) {
                        merged = next;
                    } else {
                        prev->sibling = next;
                    }
                    link_trees(curr, next);
                    curr = next;
                }
            }
            next = curr->sibling;
        }
        return merged;
    }

    static void destroy_tree(Node* n) {
        while (n) {
            Node* sibling = n->sibling;
            if (n->child) destroy_tree(n->child);
            delete n;
            n = sibling;
        }
    }

public:
    BinomialHeap() = default;

    ~BinomialHeap() {
        destroy_tree(head);
    }

    // Move semantics
    BinomialHeap(BinomialHeap&& other) noexcept : head(other.head), count(other.count) {
        other.head = nullptr;
        other.count = 0;
    }

    BinomialHeap& operator=(BinomialHeap&& other) noexcept {
        if (this != &other) {
            destroy_tree(head);
            head = other.head;
            count = other.count;
            other.head = nullptr;
            other.count = 0;
        }
        return *this;
    }

    // Disable copy for pointer safety
    BinomialHeap(const BinomialHeap&) = delete;
    BinomialHeap& operator=(const BinomialHeap&) = delete;

    bool empty() const {
        return head == nullptr;
    }

    size_t size() const {
        return count;
    }

    Node* insert(const T& key) {
        Node* node = new Node(key);
        head = union_heaps_internal(head, node);
        count++;
        return node;
    }

    const T& find_min() const {
        if (empty()) throw std::runtime_error("Heap is empty");
        const Node* curr = head;
        const Node* min_node = curr;
        while (curr) {
            if (curr->key < min_node->key) {
                min_node = curr;
            }
            curr = curr->sibling;
        }
        return min_node->key;
    }

    T extract_min() {
        if (empty()) throw std::runtime_error("Heap is empty");

        // 1. Find root with minimum key
        Node* min_node = head;
        Node* min_prev = nullptr;
        Node* curr = head;
        Node* prev = nullptr;

        while (curr) {
            if (curr->key < min_node->key) {
                min_node = curr;
                min_prev = prev;
            }
            prev = curr;
            curr = curr->sibling;
        }

        T min_val = min_node->key;

        // 2. Remove min_node from root list
        if (!min_prev) {
            head = min_node->sibling;
        } else {
            min_prev->sibling = min_node->sibling;
        }

        // 3. Reverse the children list of min_node (to sort in increasing degree)
        Node* child = min_node->child;
        Node* rev_children = nullptr;
        while (child) {
            Node* next_child = child->sibling;
            child->parent = nullptr;
            child->sibling = rev_children;
            rev_children = child;
            child = next_child;
        }

        delete min_node;
        count--;

        // 4. Merge reversed children with remaining root list
        head = union_heaps_internal(head, rev_children);
        return min_val;
    }

    void merge(BinomialHeap&& other) {
        head = union_heaps_internal(head, other.head);
        count += other.count;
        other.head = nullptr;
        other.count = 0;
    }

    void decrease_key(Node* node, const T& new_key) {
        if (new_key > node->key) {
            throw std::invalid_argument("New key is greater than current key");
        }
        node->key = new_key;
        Node* curr = node;
        Node* p = curr->parent;
        while (p && curr->key < p->key) {
            std::swap(curr->key, p->key);
            curr = p;
            p = curr->parent;
        }
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Binomial Heap C++17 unit tests..." << std::endl;

    dsa::BinomialHeap<int> bh;

    // Test 1: Empty checks
    assert(bh.empty());
    assert(bh.size() == 0);

    // Test 2: Insertions and find_min
    bh.insert(10);
    assert(bh.find_min() == 10);
    bh.insert(20);
    bh.insert(5);
    bh.insert(15);
    bh.insert(30);

    assert(bh.size() == 5);
    assert(bh.find_min() == 5);

    // Test 3: Extract min
    assert(bh.extract_min() == 5);
    assert(bh.size() == 4);
    assert(bh.find_min() == 10);

    assert(bh.extract_min() == 10);
    assert(bh.extract_min() == 15);
    assert(bh.extract_min() == 20);
    assert(bh.extract_min() == 30);
    assert(bh.empty());

    // Test 4: Merging two heaps
    dsa::BinomialHeap<int> h1;
    h1.insert(8);
    h1.insert(3);
    h1.insert(12);

    dsa::BinomialHeap<int> h2;
    h2.insert(4);
    h2.insert(17);
    h2.insert(1);

    h1.merge(std::move(h2));
    assert(h1.size() == 6);
    assert(h2.empty());
    assert(h1.find_min() == 1);

    std::vector<int> extracted;
    while (!h1.empty()) {
        extracted.push_back(h1.extract_min());
    }
    std::vector<int> expected = {1, 3, 4, 8, 12, 17};
    assert(extracted == expected);

    // Test 5: Decrease key
    {
        dsa::BinomialHeap<int> h3;
        auto* n1 = h3.insert(50);
        h3.insert(20);
        h3.insert(40);
        assert(h3.find_min() == 20);

        h3.decrease_key(n1, 5); // 50 -> 5
        assert(h3.find_min() == 5);
    }

    std::cout << "[PASS] All Binomial Heap C++ unit tests passed." << std::endl;
    return 0;
}
