#include <iostream>
#include <vector>
#include <stdexcept>
#include <cassert>

namespace dsa {

/**
 * @brief Dynamic array-backed stack (LIFO).
 * Amortized O(1) push and pop, O(1) top.
 */
template <typename T>
class ArrayStack {
public:
    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }

    void push(const T& value) {
        data_.push_back(value);
    }

    void pop() {
        if (data_.empty()) {
            throw std::underflow_error("Stack underflow");
        }
        data_.pop_back();
    }

    const T& top() const {
        if (data_.empty()) {
            throw std::underflow_error("Stack underflow");
        }
        return data_.back();
    }

private:
    std::vector<T> data_;
};

/**
 * @brief Singly-linked list stack (LIFO).
 * Strict worst-case O(1) push and pop, no reallocation.
 */
template <typename T>
class LinkedStack {
private:
    struct Node {
        T value;
        Node* next;
        Node(const T& val, Node* nxt) : value(val), next(nxt) {}
    };

public:
    LinkedStack() : head_(nullptr), size_(0) {}

    ~LinkedStack() {
        clear();
    }

    LinkedStack(const LinkedStack&) = delete;
    LinkedStack& operator=(const LinkedStack&) = delete;

    void clear() {
        while (head_) {
            Node* old = head_;
            head_ = head_->next;
            delete old;
        }
        size_ = 0;
    }

    bool empty() const { return head_ == nullptr; }
    size_t size() const { return size_; }

    void push(const T& value) {
        head_ = new Node(value, head_);
        ++size_;
    }

    void pop() {
        if (!head_) {
            throw std::underflow_error("Stack underflow");
        }
        Node* old = head_;
        head_ = head_->next;
        delete old;
        --size_;
    }

    const T& top() const {
        if (!head_) {
            throw std::underflow_error("Stack underflow");
        }
        return head_->value;
    }

private:
    Node* head_;
    size_t size_;
};

/**
 * @brief Singly-linked list queue (FIFO).
 * Head for dequeue, tail for enqueue. Strict worst-case O(1).
 */
template <typename T>
class LinkedQueue {
private:
    struct Node {
        T value;
        Node* next;
        Node(const T& val) : value(val), next(nullptr) {}
    };

public:
    LinkedQueue() : head_(nullptr), tail_(nullptr), size_(0) {}

    ~LinkedQueue() {
        clear();
    }

    LinkedQueue(const LinkedQueue&) = delete;
    LinkedQueue& operator=(const LinkedQueue&) = delete;

    void clear() {
        while (head_) {
            Node* old = head_;
            head_ = head_->next;
            delete old;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    bool empty() const { return head_ == nullptr; }
    size_t size() const { return size_; }

    void push(const T& value) {
        Node* node = new Node(value);
        if (tail_) {
            tail_->next = node;
        } else {
            head_ = node;
        }
        tail_ = node;
        ++size_;
    }

    void pop() {
        if (!head_) {
            throw std::underflow_error("Queue underflow");
        }
        Node* old = head_;
        head_ = head_->next;
        if (!head_) {
            tail_ = nullptr;
        }
        delete old;
        --size_;
    }

    const T& front() const {
        if (!head_) {
            throw std::underflow_error("Queue underflow");
        }
        return head_->value;
    }

private:
    Node* head_;
    Node* tail_;
    size_t size_;
};

/**
 * @brief Circular queue (Ring Buffer) with fixed capacity (FIFO).
 * Constant-time O(1) push and pop with zero memory allocations during operations.
 */
template <typename T>
class CircularQueue {
public:
    explicit CircularQueue(size_t capacity)
        : data_(capacity), head_(0), tail_(0), size_(0) {
        if (capacity == 0) {
            throw std::invalid_argument("Capacity must be positive");
        }
    }

    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == data_.size(); }
    size_t size() const { return size_; }
    size_t capacity() const { return data_.size(); }

    void push(const T& value) {
        if (full()) {
            throw std::overflow_error("Queue overflow");
        }
        data_[tail_] = value;
        tail_ = (tail_ + 1) % data_.size();
        ++size_;
    }

    void pop() {
        if (empty()) {
            throw std::underflow_error("Queue underflow");
        }
        head_ = (head_ + 1) % data_.size();
        --size_;
    }

    const T& front() const {
        if (empty()) {
            throw std::underflow_error("Queue underflow");
        }
        return data_[head_];
    }

private:
    std::vector<T> data_;
    size_t head_;
    size_t tail_;
    size_t size_;
};

} // namespace dsa

// ============================================================================
// Unit Tests
// ============================================================================

void test_array_stack() {
    dsa::ArrayStack<int> s;
    assert(s.empty());
    assert(s.size() == 0);

    s.push(10);
    s.push(20);
    s.push(30);
    assert(!s.empty());
    assert(s.size() == 3);
    assert(s.top() == 30);

    s.pop();
    assert(s.top() == 20);
    s.pop();
    assert(s.top() == 10);
    s.pop();
    assert(s.empty());

    bool caught = false;
    try {
        s.pop();
    } catch (const std::underflow_error&) {
        caught = true;
    }
    assert(caught);
}

void test_linked_stack() {
    dsa::LinkedStack<std::string> s;
    s.push("apple");
    s.push("banana");
    s.push("cherry");
    assert(s.size() == 3);
    assert(s.top() == "cherry");

    s.pop();
    assert(s.top() == "banana");
    s.pop();
    assert(s.top() == "apple");
    s.pop();
    assert(s.empty());
}

void test_linked_queue() {
    dsa::LinkedQueue<int> q;
    assert(q.empty());
    q.push(1);
    q.push(2);
    q.push(3);
    assert(q.size() == 3);
    assert(q.front() == 1);

    q.pop();
    assert(q.front() == 2);
    q.pop();
    assert(q.front() == 3);
    q.pop();
    assert(q.empty());

    // Re-push after empty
    q.push(42);
    assert(q.front() == 42);
    assert(q.size() == 1);
}

void test_circular_queue() {
    dsa::CircularQueue<int> q(3);
    assert(q.empty());
    assert(!q.full());

    q.push(100);
    q.push(200);
    q.push(300);
    assert(q.full());
    assert(q.front() == 100);

    // Overflow check
    bool overflow_caught = false;
    try {
        q.push(400);
    } catch (const std::overflow_error&) {
        overflow_caught = true;
    }
    assert(overflow_caught);

    // Pop and wrap around
    q.pop(); // removes 100
    assert(q.front() == 200);
    assert(!q.full());
    q.push(400); // wraps into slot 0
    assert(q.full());
    assert(q.front() == 200);

    q.pop();
    assert(q.front() == 300);
    q.pop();
    assert(q.front() == 400);
    q.pop();
    assert(q.empty());
}

int main() {
    std::cout << "Running Stacks and Queues C++17 unit tests...\n";
    test_array_stack();
    test_linked_stack();
    test_linked_queue();
    test_circular_queue();
    std::cout << "All Stacks and Queues C++17 unit tests passed successfully!\n";
    return 0;
}
