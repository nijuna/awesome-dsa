/**
 * Reference Implementation: Power-of-Two Ring Buffer (Circular Queue)
 * Demonstrates bitwise index masking (index & (capacity - 1)),
 * monotonic sequence tracking, zero modulo overhead, and cache-friendly layout.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <optional>
#include <type_traits>

template <typename T>
class RingBuffer {
private:
    std::vector<T> buffer_;
    std::size_t capacity_;
    std::size_t mask_;
    std::size_t head_; // Read position (monotonic)
    std::size_t tail_; // Write position (monotonic)

    static bool is_power_of_two(std::size_t n) noexcept {
        return n > 0 && (n & (n - 1)) == 0;
    }

    static std::size_t next_power_of_two(std::size_t n) noexcept {
        if (n <= 1) return 1;
        --n;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n |= n >> 32;
        return n + 1;
    }

public:
    explicit RingBuffer(std::size_t requested_capacity)
        : head_(0), tail_(0) {
        if (requested_capacity == 0) {
            requested_capacity = 1;
        }
        capacity_ = is_power_of_two(requested_capacity) 
                        ? requested_capacity 
                        : next_power_of_two(requested_capacity);
        mask_ = capacity_ - 1;
        buffer_.resize(capacity_);
    }

    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] std::size_t size() const noexcept { return tail_ - head_; }
    [[nodiscard]] bool empty() const noexcept { return head_ == tail_; }
    [[nodiscard]] bool full() const noexcept { return size() == capacity_; }

    /**
     * Rejection policy: Pushes element if capacity is available.
     * Returns false if full.
     */
    bool push(const T& item) {
        if (full()) {
            return false;
        }
        buffer_[tail_ & mask_] = item;
        ++tail_;
        return true;
    }

    bool push(T&& item) {
        if (full()) {
            return false;
        }
        buffer_[tail_ & mask_] = std::move(item);
        ++tail_;
        return true;
    }

    /**
     * Overwrite policy: Always writes element, overwriting oldest entry if full.
     */
    void push_overwrite(const T& item) {
        if (full()) {
            ++head_; // Drop oldest element
        }
        buffer_[tail_ & mask_] = item;
        ++tail_;
    }

    /**
     * Pops the oldest element from the buffer.
     * Returns std::nullopt if empty.
     */
    std::optional<T> pop() {
        if (empty()) {
            return std::nullopt;
        }
        T val = std::move(buffer_[head_ & mask_]);
        ++head_;
        return val;
    }

    /**
     * Inspects the oldest element without removing it.
     */
    std::optional<T> peek() const {
        if (empty()) {
            return std::nullopt;
        }
        return buffer_[head_ & mask_];
    }

    void clear() noexcept {
        head_ = 0;
        tail_ = 0;
    }
};

void run_tests() {
    // Requested capacity 6 rounds up to power-of-two 8
    RingBuffer<int> rb(6);
    assert(rb.capacity() == 8);
    assert(rb.empty());
    assert(rb.size() == 0);

    // Push 8 items
    for (int i = 1; i <= 8; ++i) {
        bool ok = rb.push(i * 10);
        assert(ok);
    }
    assert(rb.full());
    assert(rb.size() == 8);

    // 9th push must be rejected under standard policy
    assert(!rb.push(90));

    // Peek oldest
    assert(rb.peek().value() == 10);

    // Pop 3 items (10, 20, 30)
    assert(rb.pop().value() == 10);
    assert(rb.pop().value() == 20);
    assert(rb.pop().value() == 30);
    assert(rb.size() == 5);
    assert(!rb.full());

    // Push 3 new items with wraparound (90, 100, 110)
    assert(rb.push(90));
    assert(rb.push(100));
    assert(rb.push(110));
    assert(rb.full());

    // Test overwrite policy
    rb.push_overwrite(999);
    assert(rb.full());
    assert(rb.size() == 8);
    // Oldest element was 40, now dropped, so peek must be 50
    assert(rb.peek().value() == 50);

    // Drain entire buffer
    std::vector<int> drained;
    while (!rb.empty()) {
        drained.push_back(rb.pop().value());
    }
    assert(drained.size() == 8);
    assert(drained.front() == 50);
    assert(drained.back() == 999);
    assert(rb.empty());

    std::cout << "[PASS] All RingBuffer C++ unit tests passed.\n";
}

int main() {
    run_tests();
    return 0;
}
