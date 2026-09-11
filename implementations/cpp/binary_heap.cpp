/**
 * Reference Implementation: Contiguous Array-Backed Binary Heap
 * Demonstrates implicit tree indexing (2i + 1, 2i + 2, (i - 1) / 2),
 * sift-up and sift-down mechanics, Floyd's O(n) linear-time build_heap,
 * and in-place heapsort.
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
class BinaryHeap {
private:
    std::vector<T> data_;
    Compare comp_; // Returns true if a has higher priority than b

    static inline std::size_t parent(std::size_t i) noexcept {
        return (i - 1) / 2;
    }

    static inline std::size_t left_child(std::size_t i) noexcept {
        return 2 * i + 1;
    }

    static inline std::size_t right_child(std::size_t i) noexcept {
        return 2 * i + 2;
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
            std::size_t left = left_child(i);
            std::size_t right = right_child(i);

            if (left < n && comp_(data_[left], data_[best])) {
                best = left;
            }
            if (right < n && comp_(data_[right], data_[best])) {
                best = right;
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
    explicit BinaryHeap(Compare comp = Compare{}) : comp_(comp) {}

    // Floyd's O(n) bottom-up heap construction
    explicit BinaryHeap(std::vector<T> elements, Compare comp = Compare{})
        : data_(std::move(elements)), comp_(comp) {
        if (!data_.empty()) {
            for (std::size_t i = data_.size() / 2; i > 0; --i) {
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
            throw std::underflow_error("BinaryHeap::top(): heap is empty");
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
            throw std::underflow_error("BinaryHeap::pop(): heap is empty");
        }
        T top_val = std::move(data_[0]);
        data_[0] = std::move(data_.back());
        data_.pop_back();
        if (!data_.empty()) {
            sift_down(0, data_.size());
        }
        return top_val;
    }

    // In-place heapsort utility: sorts an arbitrary array in-place in O(n log n)
    static void heapsort(std::vector<T>& arr, Compare comp = Compare{}) {
        if (arr.size() <= 1) return;

        // Step 1: Floyd's bottom-up build-heap O(n)
        for (std::size_t i = arr.size() / 2; i > 0; --i) {
            std::size_t idx = i - 1;
            std::size_t n = arr.size();
            while (true) {
                std::size_t best = idx;
                std::size_t l = 2 * idx + 1;
                std::size_t r = 2 * idx + 2;
                if (l < n && comp(arr[l], arr[best])) best = l;
                if (r < n && comp(arr[r], arr[best])) best = r;
                if (best != idx) {
                    std::swap(arr[idx], arr[best]);
                    idx = best;
                } else {
                    break;
                }
            }
        }

        // Step 2: Repeatedly swap top to end and sift-down
        for (std::size_t end = arr.size() - 1; end > 0; --end) {
            std::swap(arr[0], arr[end]);
            std::size_t idx = 0;
            while (true) {
                std::size_t best = idx;
                std::size_t l = 2 * idx + 1;
                std::size_t r = 2 * idx + 2;
                if (l < end && comp(arr[l], arr[best])) best = l;
                if (r < end && comp(arr[r], arr[best])) best = r;
                if (best != idx) {
                    std::swap(arr[idx], arr[best]);
                    idx = best;
                } else {
                    break;
                }
            }
        }
    }

    [[nodiscard]] bool is_valid_heap() const {
        for (std::size_t i = 0; i < data_.size(); ++i) {
            std::size_t l = left_child(i);
            std::size_t r = right_child(i);
            if (l < data_.size() && comp_(data_[l], data_[i])) return false;
            if (r < data_.size() && comp_(data_[r], data_[i])) return false;
        }
        return true;
    }
};

int main() {
    // 1. Min-Heap verification (default std::less<int>)
    BinaryHeap<int> min_heap;
    assert(min_heap.empty());
    assert(min_heap.size() == 0);

    std::vector<int> inputs = {42, 17, 93, 8, 31, 5, 64, 22, 11, 75};
    for (int x : inputs) {
        min_heap.push(x);
        assert(min_heap.is_valid_heap());
    }

    assert(min_heap.size() == 10);
    assert(min_heap.top() == 5);

    std::vector<int> sorted_min;
    while (!min_heap.empty()) {
        sorted_min.push_back(min_heap.pop());
        assert(min_heap.is_valid_heap());
    }
    assert(std::is_sorted(sorted_min.begin(), sorted_min.end()));

    // 2. Max-Heap verification (std::greater<int>)
    BinaryHeap<int, std::greater<int>> max_heap;
    for (int x : inputs) {
        max_heap.push(x);
        assert(max_heap.is_valid_heap());
    }
    assert(max_heap.top() == 93);

    std::vector<int> sorted_max;
    while (!max_heap.empty()) {
        sorted_max.push_back(max_heap.pop());
    }
    assert(std::is_sorted(sorted_max.rbegin(), sorted_max.rend()));

    // 3. Floyd's O(n) Build-Heap constructor
    std::vector<int> batch = {19, 3, 15, 7, 8, 23, 2, 4, 11, 14, 1, 6};
    BinaryHeap<int> floyd_heap(batch);
    assert(floyd_heap.is_valid_heap());
    assert(floyd_heap.top() == 1);
    assert(floyd_heap.size() == batch.size());

    // 4. In-place Heapsort verification
    std::vector<int> arr = {9, 4, 1, 7, 3, 8, 2, 6, 5, 0};
    // To sort in ascending order using heapsort, use a max-heap comparator
    BinaryHeap<int, std::greater<int>>::heapsort(arr, std::greater<int>{});
    assert(std::is_sorted(arr.begin(), arr.end()));

    std::cout << "[PASS] All BinaryHeap C++ unit tests passed.\n";
    return 0;
}
