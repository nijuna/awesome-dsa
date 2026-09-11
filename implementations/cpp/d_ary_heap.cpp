/**
 * Reference Implementation: Contiguous Array-Backed d-ary Heap
 * Demonstrates arbitrary branching factor D (e.g., D = 4, D = 8),
 * generalized 0-based index arithmetic, sift-up (O(log_D n)),
 * sift-down with D-child scan (O(D log_D n)), and Floyd-style O(n) build_heap.
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

template <typename T, std::size_t D = 4, typename Compare = std::less<T>>
class DAryHeap {
    static_assert(D >= 2, "d-ary heap branching factor D must be at least 2");

private:
    std::vector<T> data_;
    Compare comp_; // Returns true if a has higher priority than b

    static inline std::size_t parent(std::size_t i) noexcept {
        return (i - 1) / D;
    }

    static inline std::size_t first_child(std::size_t i) noexcept {
        return D * i + 1;
    }

    void sift_up(std::size_t i) {
        while (i > 0) {
            std::size_t p = parent(i);
            if (comp_(data_[i], data_[p])) {
                std::swap(data_[i], data_[p]);
                i = p;
            } else {
                break;
            }
        }
    }

    void sift_down(std::size_t i, std::size_t n) {
        while (true) {
            std::size_t best = i;
            std::size_t first = first_child(i);

            if (first >= n) {
                break; // No children
            }

            std::size_t last = std::min(first + D, n);
            for (std::size_t c = first; c < last; ++c) {
                if (comp_(data_[c], data_[best])) {
                    best = c;
                }
            }

            if (best != i) {
                std::swap(data_[i], data_[best]);
                i = best;
            } else {
                break;
            }
        }
    }

public:
    explicit DAryHeap(Compare comp = Compare{}) : comp_(comp) {}

    // Floyd-style O(n) bottom-up heap construction
    explicit DAryHeap(std::vector<T> elements, Compare comp = Compare{})
        : data_(std::move(elements)), comp_(comp) {
        if (data_.size() > 1) {
            for (std::size_t i = (data_.size() - 2) / D + 1; i > 0; --i) {
                sift_down(i - 1, data_.size());
            }
        }
    }

    [[nodiscard]] std::size_t size() const noexcept {
        return data_.size();
    }

    [[nodiscard]] bool empty() const noexcept {
        return data_.empty();
    }

    const T& top() const {
        if (data_.empty()) {
            throw std::underflow_error("DAryHeap::top(): heap is empty");
        }
        return data_[0];
    }

    void push(const T& val) {
        data_.push_back(val);
        sift_up(data_.size() - 1);
    }

    void push(T&& val) {
        data_.push_back(std::move(val));
        sift_up(data_.size() - 1);
    }

    T pop() {
        if (data_.empty()) {
            throw std::underflow_error("DAryHeap::pop(): heap is empty");
        }
        T top_val = std::move(data_[0]);
        data_[0] = std::move(data_.back());
        data_.pop_back();
        if (!data_.empty()) {
            sift_down(0, data_.size());
        }
        return top_val;
    }

    [[nodiscard]] bool is_valid_heap() const {
        for (std::size_t i = 0; i < data_.size(); ++i) {
            std::size_t first = first_child(i);
            std::size_t last = std::min(first + D, data_.size());
            for (std::size_t c = first; c < last; ++c) {
                if (comp_(data_[c], data_[i])) {
                    return false;
                }
            }
        }
        return true;
    }
};

int main() {
    // 1. 4-ary Min-Heap verification
    DAryHeap<int, 4> heap4;
    assert(heap4.empty());
    assert(heap4.size() == 0);

    std::vector<int> inputs = {55, 12, 89, 4, 32, 1, 67, 23, 19, 78, 3, 99, 45, 6};
    for (int x : inputs) {
        heap4.push(x);
        assert(heap4.is_valid_heap());
    }

    assert(heap4.size() == inputs.size());
    assert(heap4.top() == 1);

    std::vector<int> sorted4;
    while (!heap4.empty()) {
        sorted4.push_back(heap4.pop());
        assert(heap4.is_valid_heap());
    }
    assert(std::is_sorted(sorted4.begin(), sorted4.end()));

    // 2. 8-ary Max-Heap verification
    DAryHeap<int, 8, std::greater<int>> heap8;
    for (int x : inputs) {
        heap8.push(x);
        assert(heap8.is_valid_heap());
    }
    assert(heap8.top() == 99);

    std::vector<int> sorted8;
    while (!heap8.empty()) {
        sorted8.push_back(heap8.pop());
        assert(heap8.is_valid_heap());
    }
    assert(std::is_sorted(sorted8.rbegin(), sorted8.rend()));

    // 3. Floyd-style O(n) construction for D = 4
    DAryHeap<int, 4> floyd_heap4(inputs);
    assert(floyd_heap4.is_valid_heap());
    assert(floyd_heap4.top() == 1);

    // 4. Floyd-style O(n) construction for D = 3
    DAryHeap<int, 3> floyd_heap3(inputs);
    assert(floyd_heap3.is_valid_heap());
    assert(floyd_heap3.top() == 1);

    std::cout << "[PASS] All d-ary Heap C++ unit tests passed.\n";
    return 0;
}
