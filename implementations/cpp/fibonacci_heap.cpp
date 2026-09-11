/**
 * Reference Implementation: Fibonacci Heap
 * Demonstrates lazy consolidation, cascading cuts, circular doubly linked lists,
 * O(1) amortized insert, meld, and decrease-key, and O(log n) amortized extract-min.
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
#include <cmath>

template <typename T, typename Compare = std::less<T>>
class FibonacciHeap {
public:
    struct Node {
        T val;
        int degree = 0;
        bool mark = false;
        Node* parent = nullptr;
        Node* child = nullptr;
        Node* left = this;
        Node* right = this;

        explicit Node(T v) : val(std::move(v)) {}
    };

private:
    Node* min_ = nullptr;
    std::size_t size_ = 0;
    Compare comp_; // Returns true if a has higher priority than b

    // Splice a single node into a circular doubly linked list (next to target)
    static void list_insert_after(Node* target, Node* node) noexcept {
        node->right = target->right;
        node->left = target;
        target->right->left = node;
        target->right = node;
    }

    // Remove a node from its circular doubly linked list
    static void list_remove(Node* node) noexcept {
        node->left->right = node->right;
        node->right->left = node->left;
    }

    // Concatenate two circular doubly linked lists, returns pointer to minimum
    static Node* list_concat(Node* a, Node* b, Compare comp) noexcept {
        if (!a) return b;
        if (!b) return a;

        Node* a_next = a->right;
        Node* b_prev = b->left;

        a->right = b;
        b->left = a;
        a_next->left = b_prev;
        b_prev->right = a_next;

        return comp(b->val, a->val) ? b : a;
    }

    // Link tree y as a child of tree x (x becomes parent of y)
    void link(Node* y, Node* x) noexcept {
        list_remove(y);
        y->parent = x;
        y->mark = false;

        if (!x->child) {
            x->child = y;
            y->left = y;
            y->right = y;
        } else {
            list_insert_after(x->child, y);
        }
        ++x->degree;
    }

    // Consolidate trees in the root list by degree
    void consolidate() {
        if (!min_) return;

        // Collect current roots before modifying pointers
        std::vector<Node*> roots;
        Node* curr = min_;
        do {
            roots.push_back(curr);
            curr = curr->right;
        } while (curr != min_);

        std::size_t max_d = static_cast<std::size_t>(std::log2(size_ + 1) * 2.0) + 8;
        std::vector<Node*> table(max_d, nullptr);

        for (Node* w : roots) {
            Node* x = w;
            std::size_t d = static_cast<std::size_t>(x->degree);

            while (d < table.size() && table[d] != nullptr) {
                Node* y = table[d];
                if (comp_(y->val, x->val)) {
                    std::swap(x, y);
                }
                link(y, x);
                table[d] = nullptr;
                ++d;
                if (d >= table.size()) {
                    table.resize(d + 8, nullptr);
                }
            }
            table[d] = x;
        }

        // Rebuild root list from degree table
        min_ = nullptr;
        for (Node* y : table) {
            if (y) {
                y->left = y;
                y->right = y;
                y->parent = nullptr;
                if (!min_) {
                    min_ = y;
                } else {
                    list_insert_after(min_, y);
                    if (comp_(y->val, min_->val)) {
                        min_ = y;
                    }
                }
            }
        }
    }

    // Sever node x from parent y and move x to the root list
    void cut(Node* x, Node* y) noexcept {
        if (x->right == x) {
            y->child = nullptr;
        } else {
            y->child = x->right;
            list_remove(x);
        }
        --y->degree;

        x->left = x;
        x->right = x;
        x->parent = nullptr;
        x->mark = false;
        list_insert_after(min_, x);
    }

    // Cascading cuts upward to maintain logarithmic degree bounds
    void cascading_cut(Node* y) noexcept {
        Node* z = y->parent;
        if (z != nullptr) {
            if (!y->mark) {
                y->mark = true;
            } else {
                cut(y, z);
                cascading_cut(z);
            }
        }
    }

    static void destroy_node(Node* n) noexcept {
        if (!n) return;
        std::vector<Node*> stack;
        stack.push_back(n);

        while (!stack.empty()) {
            Node* curr = stack.back();
            stack.pop_back();

            if (curr->child) {
                Node* c = curr->child;
                Node* start = c;
                do {
                    Node* next = c->right;
                    stack.push_back(c);
                    c = next;
                } while (c != start);
                curr->child = nullptr;
            }
            delete curr;
        }
    }

    void destroy_all() noexcept {
        if (!min_) return;
        std::vector<Node*> roots;
        Node* curr = min_;
        do {
            roots.push_back(curr);
            curr = curr->right;
        } while (curr != min_);

        for (Node* r : roots) {
            destroy_node(r);
        }
        min_ = nullptr;
        size_ = 0;
    }

public:
    explicit FibonacciHeap(Compare comp = Compare{})
        : min_(nullptr), size_(0), comp_(comp) {}

    ~FibonacciHeap() {
        destroy_all();
    }

    FibonacciHeap(const FibonacciHeap&) = delete;
    FibonacciHeap& operator=(const FibonacciHeap&) = delete;

    FibonacciHeap(FibonacciHeap&& other) noexcept
        : min_(other.min_), size_(other.size_), comp_(other.comp_) {
        other.min_ = nullptr;
        other.size_ = 0;
    }

    FibonacciHeap& operator=(FibonacciHeap&& other) noexcept {
        if (this != &other) {
            destroy_all();
            min_ = other.min_;
            size_ = other.size_;
            comp_ = other.comp_;
            other.min_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    const T& top() const {
        if (!min_) {
            throw std::underflow_error("FibonacciHeap::top(): heap is empty");
        }
        return min_->val;
    }

    Node* push(const T& val) {
        Node* node = new Node(val);
        if (!min_) {
            min_ = node;
        } else {
            list_insert_after(min_, node);
            if (comp_(node->val, min_->val)) {
                min_ = node;
            }
        }
        ++size_;
        return node;
    }

    T pop() {
        if (!min_) {
            throw std::underflow_error("FibonacciHeap::pop(): heap is empty");
        }

        Node* z = min_;
        T top_val = std::move(z->val);

        // Promote all children of min_ to the root list
        if (z->child) {
            std::vector<Node*> children;
            Node* c = z->child;
            do {
                children.push_back(c);
                c = c->right;
            } while (c != z->child);

            for (Node* child : children) {
                child->parent = nullptr;
                list_insert_after(min_, child);
            }
            z->child = nullptr;
        }

        // Remove min_ from root list
        if (z->right == z) {
            min_ = nullptr;
        } else {
            min_ = z->right;
            list_remove(z);
            consolidate();
        }

        delete z;
        --size_;
        return top_val;
    }

    // Melds another FibonacciHeap into this one in O(1) amortized and actual time
    void meld(FibonacciHeap& other) noexcept {
        if (this == &other || other.empty()) return;

        if (!min_) {
            min_ = other.min_;
        } else {
            min_ = list_concat(min_, other.min_, comp_);
        }
        size_ += other.size_;

        other.min_ = nullptr;
        other.size_ = 0;
    }

    // Decreases the key of an existing node in O(1) amortized time
    void decrease_key(Node* x, const T& new_val) {
        if (comp_(x->val, new_val)) {
            throw std::invalid_argument("FibonacciHeap::decrease_key(): new key is not higher priority");
        }
        x->val = new_val;
        Node* y = x->parent;

        if (y != nullptr && comp_(x->val, y->val)) {
            cut(x, y);
            cascading_cut(y);
        }

        if (comp_(x->val, min_->val)) {
            min_ = x;
        }
    }
};

int main() {
    FibonacciHeap<int> fib;
    assert(fib.empty());
    assert(fib.size() == 0);

    // 1. Basic push and top
    std::vector<int> vals = {42, 17, 93, 8, 31, 5, 64, 22, 11, 75};
    for (int v : vals) {
        fib.push(v);
    }
    assert(fib.size() == 10);
    assert(fib.top() == 5);

    // 2. Sorted extraction
    std::vector<int> sorted_out;
    while (!fib.empty()) {
        sorted_out.push_back(fib.pop());
    }
    assert(fib.empty());
    assert((sorted_out == std::vector<int>{5, 8, 11, 17, 22, 31, 42, 64, 75, 93}));

    // 3. Decrease-Key with cascading cuts test
    FibonacciHeap<int> fh;
    auto* n1 = fh.push(100);
    auto* n2 = fh.push(50);
    auto* n3 = fh.push(80);
    auto* n4 = fh.push(30);
    auto* n5 = fh.push(60);
    (void)n2;
    (void)n4;

    // Force consolidation
    int top = fh.pop(); // pops 30
    assert(top == 30);

    // Decrease key
    fh.decrease_key(n1, 10); // 100 -> 10, becomes new min
    assert(fh.top() == 10);

    fh.decrease_key(n3, 5);  // 80 -> 5, becomes new min
    assert(fh.top() == 5);

    fh.decrease_key(n5, 1);  // 60 -> 1, becomes new min
    assert(fh.top() == 1);

    std::vector<int> out2;
    while (!fh.empty()) {
        out2.push_back(fh.pop());
    }
    assert((out2 == std::vector<int>{1, 5, 10, 50}));

    // 4. Meld test
    FibonacciHeap<int> h1;
    h1.push(20);
    h1.push(40);

    FibonacciHeap<int> h2;
    h2.push(10);
    h2.push(30);

    h1.meld(h2);
    assert(h1.size() == 4);
    assert(h2.empty());
    assert(h1.top() == 10);

    std::vector<int> out_meld;
    while (!h1.empty()) {
        out_meld.push_back(h1.pop());
    }
    assert((out_meld == std::vector<int>{10, 20, 30, 40}));

    // 5. Duplicate elements
    FibonacciHeap<int> h_dups;
    h_dups.push(5);
    h_dups.push(5);
    h_dups.push(5);
    assert(h_dups.pop() == 5);
    assert(h_dups.pop() == 5);
    assert(h_dups.pop() == 5);
    assert(h_dups.empty());

    // 6. Underflow verification
    try {
        h_dups.pop();
        assert(false);
    } catch (const std::underflow_error&) {
        // Expected
    }

    std::cout << "[PASS] All FibonacciHeap C++ unit tests passed.\n";
    return 0;
}
