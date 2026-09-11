/**
 * Reference Implementation: Self-Adjusting Pairing Heap
 * Demonstrates the two-pass pairing algorithm (Fredman et al., 1986),
 * O(1) meld, O(1) insert, O(log n) amortized delete_min,
 * and first-child / next-sibling representation.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <functional>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <algorithm>

template <typename T, typename Compare = std::less<T>>
class PairingHeap {
public:
    struct Node {
        T val;
        Node* child;
        Node* sibling;

        explicit Node(T v) : val(std::move(v)), child(nullptr), sibling(nullptr) {}
    };

private:
    Node* root_;
    std::size_t size_;
    Compare comp_; // Returns true if a has higher priority than b

    static Node* meld_nodes(Node* a, Node* b, Compare comp) {
        if (!a) return b;
        if (!b) return a;

        // Ensure 'a' has the higher priority
        if (comp(b->val, a->val)) {
            std::swap(a, b);
        }

        // 'b' becomes the leftmost child of 'a'
        b->sibling = a->child;
        a->child = b;
        return a;
    }

    // Canonical Two-Pass Pairing Algorithm (Fredman, Sedgewick, Sleator, Tarjan 1986)
    static Node* two_pass_merge(Node* first, Compare comp) {
        if (!first) return nullptr;
        if (!first->sibling) return first;

        // Pass 1: Left-to-Right pairing
        std::vector<Node*> pairs;
        Node* curr = first;
        while (curr) {
            Node* a = curr;
            Node* b = curr->sibling;
            if (b) {
                Node* next = b->sibling;
                a->sibling = nullptr;
                b->sibling = nullptr;
                pairs.push_back(meld_nodes(a, b, comp));
                curr = next;
            } else {
                a->sibling = nullptr;
                pairs.push_back(a);
                curr = nullptr;
            }
        }

        // Pass 2: Right-to-Left accumulation
        Node* result = pairs.back();
        for (int i = static_cast<int>(pairs.size()) - 2; i >= 0; --i) {
            result = meld_nodes(pairs[i], result, comp);
        }
        return result;
    }

    static void destroy_tree(Node* n) noexcept {
        // Non-recursive destruction to prevent stack overflow on deep trees
        std::vector<Node*> stack;
        if (n) stack.push_back(n);

        while (!stack.empty()) {
            Node* curr = stack.back();
            stack.pop_back();

            if (curr->sibling) stack.push_back(curr->sibling);
            if (curr->child) stack.push_back(curr->child);
            delete curr;
        }
    }

public:
    explicit PairingHeap(Compare comp = Compare{})
        : root_(nullptr), size_(0), comp_(comp) {}

    ~PairingHeap() {
        destroy_tree(root_);
    }

    PairingHeap(const PairingHeap&) = delete;
    PairingHeap& operator=(const PairingHeap&) = delete;

    PairingHeap(PairingHeap&& other) noexcept
        : root_(other.root_), size_(other.size_), comp_(other.comp_) {
        other.root_ = nullptr;
        other.size_ = 0;
    }

    PairingHeap& operator=(PairingHeap&& other) noexcept {
        if (this != &other) {
            destroy_tree(root_);
            root_ = other.root_;
            size_ = other.size_;
            comp_ = other.comp_;
            other.root_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    const T& top() const {
        if (!root_) {
            throw std::underflow_error("PairingHeap::top(): heap is empty");
        }
        return root_->val;
    }

    void push(const T& val) {
        Node* node = new Node(val);
        root_ = meld_nodes(root_, node, comp_);
        ++size_;
    }

    void push(T&& val) {
        Node* node = new Node(std::move(val));
        root_ = meld_nodes(root_, node, comp_);
        ++size_;
    }

    T pop() {
        if (!root_) {
            throw std::underflow_error("PairingHeap::pop(): heap is empty");
        }
        T top_val = std::move(root_->val);
        Node* old_root = root_;
        Node* children = root_->child;

        delete old_root;
        root_ = two_pass_merge(children, comp_);
        --size_;
        return top_val;
    }

    // Melds another PairingHeap into this one in O(1) time
    void meld(PairingHeap& other) {
        if (this == &other || other.empty()) return;

        root_ = meld_nodes(root_, other.root_, comp_);
        size_ += other.size_;

        other.root_ = nullptr;
        other.size_ = 0;
    }
};

int main() {
    PairingHeap<int> min_heap;
    assert(min_heap.empty());
    assert(min_heap.size() == 0);

    // 1. Push elements
    std::vector<int> inputs = {42, 17, 93, 8, 31, 5, 64, 22, 11, 75};
    for (int x : inputs) {
        min_heap.push(x);
    }
    assert(min_heap.size() == inputs.size());
    assert(min_heap.top() == 5);

    // 2. Pop all and verify sorted order
    std::vector<int> sorted;
    while (!min_heap.empty()) {
        sorted.push_back(min_heap.pop());
    }
    assert(sorted == std::vector<int>({5, 8, 11, 17, 22, 31, 42, 64, 75, 93}));
    assert(min_heap.empty());

    // 3. O(1) Meld verification
    PairingHeap<int> h1;
    h1.push(10);
    h1.push(30);
    h1.push(50);

    PairingHeap<int> h2;
    h2.push(5);
    h2.push(25);
    h2.push(70);

    h1.meld(h2);
    assert(h1.size() == 6);
    assert(h2.empty());
    assert(h1.top() == 5);

    std::vector<int> melded_sorted;
    while (!h1.empty()) {
        melded_sorted.push_back(h1.pop());
    }
    assert(melded_sorted == std::vector<int>({5, 10, 25, 30, 50, 70}));

    // 4. Max-Heap comparator verification
    PairingHeap<int, std::greater<int>> max_heap;
    for (int x : inputs) {
        max_heap.push(x);
    }
    assert(max_heap.top() == 93);
    assert(max_heap.pop() == 93);
    assert(max_heap.pop() == 75);

    std::cout << "[PASS] All PairingHeap C++ unit tests passed.\n";
    return 0;
}
