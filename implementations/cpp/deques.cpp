#include <iostream>
#include <vector>
#include <deque>
#include <stdexcept>
#include <cassert>

namespace dsa {

/**
 * @brief Fixed-capacity circular buffer deque.
 * Supports push/pop at both front and back in O(1) time.
 */
template <typename T>
class CircularDeque {
public:
    explicit CircularDeque(size_t capacity)
        : data_(capacity), front_(0), size_(0) {
        if (capacity == 0) {
            throw std::invalid_argument("Capacity must be positive");
        }
    }

    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == data_.size(); }
    size_t size() const { return size_; }
    size_t capacity() const { return data_.size(); }

    void push_front(const T& value) {
        if (full()) {
            throw std::overflow_error("Deque overflow");
        }
        front_ = (front_ - 1 + data_.size()) % data_.size();
        data_[front_] = value;
        ++size_;
    }

    void push_back(const T& value) {
        if (full()) {
            throw std::overflow_error("Deque overflow");
        }
        size_t idx = (front_ + size_) % data_.size();
        data_[idx] = value;
        ++size_;
    }

    void pop_front() {
        if (empty()) {
            throw std::underflow_error("Deque underflow");
        }
        front_ = (front_ + 1) % data_.size();
        --size_;
    }

    void pop_back() {
        if (empty()) {
            throw std::underflow_error("Deque underflow");
        }
        --size_;
    }

    const T& front() const {
        if (empty()) {
            throw std::underflow_error("Deque underflow");
        }
        return data_[front_];
    }

    const T& back() const {
        if (empty()) {
            throw std::underflow_error("Deque underflow");
        }
        size_t idx = (front_ + size_ - 1) % data_.size();
        return data_[idx];
    }

private:
    std::vector<T> data_;
    size_t front_;
    size_t size_;
};

/**
 * @brief Doubly linked-list deque.
 * Worst-case O(1) push and pop at both ends without resizing.
 */
template <typename T>
class LinkedDeque {
private:
    struct Node {
        T value;
        Node* prev;
        Node* next;
        Node(const T& val) : value(val), prev(nullptr), next(nullptr) {}
    };

public:
    LinkedDeque() : head_(nullptr), tail_(nullptr), size_(0) {}

    ~LinkedDeque() {
        clear();
    }

    LinkedDeque(const LinkedDeque&) = delete;
    LinkedDeque& operator=(const LinkedDeque&) = delete;

    void clear() {
        while (head_) {
            Node* old = head_;
            head_ = head_->next;
            delete old;
        }
        tail_ = nullptr;
        size_ = 0;
    }

    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }

    void push_front(const T& value) {
        Node* node = new Node(value);
        node->next = head_;
        if (head_) {
            head_->prev = node;
        } else {
            tail_ = node;
        }
        head_ = node;
        ++size_;
    }

    void push_back(const T& value) {
        Node* node = new Node(value);
        node->prev = tail_;
        if (tail_) {
            tail_->next = node;
        } else {
            head_ = node;
        }
        tail_ = node;
        ++size_;
    }

    void pop_front() {
        if (!head_) {
            throw std::underflow_error("Deque underflow");
        }
        Node* old = head_;
        head_ = head_->next;
        if (head_) {
            head_->prev = nullptr;
        } else {
            tail_ = nullptr;
        }
        delete old;
        --size_;
    }

    void pop_back() {
        if (!tail_) {
            throw std::underflow_error("Deque underflow");
        }
        Node* old = tail_;
        tail_ = tail_->prev;
        if (tail_) {
            tail_->next = nullptr;
        } else {
            head_ = nullptr;
        }
        delete old;
        --size_;
    }

    const T& front() const {
        if (!head_) {
            throw std::underflow_error("Deque underflow");
        }
        return head_->value;
    }

    const T& back() const {
        if (!tail_) {
            throw std::underflow_error("Deque underflow");
        }
        return tail_->value;
    }

private:
    Node* head_;
    Node* tail_;
    size_t size_;
};

/**
 * @brief Computes maximum value in every sliding window of size k in O(n) time.
 * Uses a monotonic deque to maintain decreasing candidates.
 */
inline std::vector<int> sliding_window_max(const std::vector<int>& nums, int k) {
    std::deque<int> dq; // stores indices
    std::vector<int> result;
    int n = static_cast<int>(nums.size());
    if (n == 0 || k <= 0 || k > n) return result;

    for (int i = 0; i < n; ++i) {
        // Remove indices outside current window
        while (!dq.empty() && dq.front() <= i - k) {
            dq.pop_front();
        }

        // Maintain decreasing order: discard candidates smaller than current
        while (!dq.empty() && nums[dq.back()] <= nums[i]) {
            dq.pop_back();
        }

        dq.push_back(i);

        // Record maximum once window reaches size k
        if (i >= k - 1) {
            result.push_back(nums[dq.front()]);
        }
    }

    return result;
}

} // namespace dsa

// ============================================================================
// Unit Tests
// ============================================================================

void test_circular_deque() {
    dsa::CircularDeque<int> dq(4);
    assert(dq.empty());

    dq.push_back(10);
    dq.push_back(20);
    dq.push_front(5);
    dq.push_front(1);
    assert(dq.full());
    assert(dq.size() == 4);
    assert(dq.front() == 1);
    assert(dq.back() == 20);

    dq.pop_front();
    assert(dq.front() == 5);
    dq.pop_back();
    assert(dq.back() == 10);

    dq.push_back(30);
    assert(dq.back() == 30);
    assert(dq.front() == 5);

    dq.pop_front();
    dq.pop_front();
    dq.pop_front();
    assert(dq.empty());
}

void test_linked_deque() {
    dsa::LinkedDeque<std::string> dq;
    dq.push_front("mid");
    dq.push_front("start");
    dq.push_back("end");
    assert(dq.size() == 3);
    assert(dq.front() == "start");
    assert(dq.back() == "end");

    dq.pop_front();
    assert(dq.front() == "mid");
    dq.pop_back();
    assert(dq.back() == "mid");
    dq.pop_front();
    assert(dq.empty());
}

void test_sliding_window_max() {
    std::vector<int> nums = {1, 3, -1, -3, 5, 3, 6, 7};
    int k = 3;
    auto res = dsa::sliding_window_max(nums, k);
    std::vector<int> expected = {3, 3, 5, 5, 6, 7};
    assert(res == expected);
}

int main() {
    std::cout << "Running Deque C++17 unit tests...\n";
    test_circular_deque();
    test_linked_deque();
    test_sliding_window_max();
    std::cout << "All Deque C++17 unit tests passed successfully!\n";
    return 0;
}
