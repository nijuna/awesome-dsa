#include <cassert>
#include <cstdint>
#include <iostream>
#include <vector>
#include <stdexcept>
#include <algorithm>

namespace dsa {

/**
 * @brief Dynamic array instrumented with amortized cost and potential tracking.
 * Potential function: Phi = 2 * size - capacity (when size >= capacity / 2).
 */
class TrackedDynamicArray {
private:
    int* data_;
    size_t size_;
    size_t capacity_;
    uint64_t total_copies_;
    uint64_t total_operations_;

public:
    TrackedDynamicArray()
        : data_(nullptr), size_(0), capacity_(0), total_copies_(0), total_operations_(0) {}

    ~TrackedDynamicArray() {
        delete[] data_;
    }

    // Disallow copies for simple tracking
    TrackedDynamicArray(const TrackedDynamicArray&) = delete;
    TrackedDynamicArray& operator=(const TrackedDynamicArray&) = delete;

    size_t size() const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }
    uint64_t total_copies() const noexcept { return total_copies_; }
    uint64_t total_operations() const noexcept { return total_operations_; }

    // Potential function: Phi = 2 * size - capacity
    int64_t potential() const noexcept {
        if (capacity_ == 0) return 0;
        return static_cast<int64_t>(2 * size_) - static_cast<int64_t>(capacity_);
    }

    void push_back(int val) {
        total_operations_++;
        if (size_ == capacity_) {
            size_t new_capacity = (capacity_ == 0) ? 1 : capacity_ * 2;
            int* new_data = new int[new_capacity];
            for (size_t i = 0; i < size_; ++i) {
                new_data[i] = data_[i];
                total_copies_++;
            }
            delete[] data_;
            data_ = new_data;
            capacity_ = new_capacity;
        }
        data_[size_++] = val;
    }

    int operator[](size_t idx) const {
        if (idx >= size_) throw std::out_of_range("Index out of range");
        return data_[idx];
    }
};

/**
 * @brief Multipop stack instrumented to verify aggregate and potential bounds.
 * Potential function: Phi = size.
 */
class TrackedMultipopStack {
private:
    std::vector<int> data_;
    uint64_t total_pushes_;
    uint64_t total_pops_;

public:
    TrackedMultipopStack() : total_pushes_(0), total_pops_(0) {}

    size_t size() const noexcept { return data_.size(); }
    uint64_t total_pushes() const noexcept { return total_pushes_; }
    uint64_t total_pops() const noexcept { return total_pops_; }

    int64_t potential() const noexcept {
        return static_cast<int64_t>(data_.size());
    }

    void push(int val) {
        data_.push_back(val);
        total_pushes_++;
    }

    int pop() {
        if (data_.empty()) throw std::underflow_error("Stack underflow");
        int val = data_.back();
        data_.pop_back();
        total_pops_++;
        return val;
    }

    void multipop(size_t k) {
        size_t count = std::min(k, data_.size());
        for (size_t i = 0; i < count; ++i) {
            data_.pop_back();
            total_pops_++;
        }
    }
};

/**
 * @brief Binary counter increment simulator.
 * Demonstrates aggregate bit flips bounded by 2 * N.
 */
class TrackedBinaryCounter {
private:
    std::vector<bool> bits_;
    uint64_t total_flips_;

public:
    TrackedBinaryCounter() : total_flips_(0) {}

    uint64_t total_flips() const noexcept { return total_flips_; }
    size_t count_ones() const noexcept {
        size_t ones = 0;
        for (bool b : bits_) if (b) ones++;
        return ones;
    }

    void increment() {
        size_t i = 0;
        while (i < bits_.size() && bits_[i]) {
            bits_[i] = false;
            total_flips_++;
            i++;
        }
        if (i < bits_.size()) {
            bits_[i] = true;
            total_flips_++;
        } else {
            bits_.push_back(true);
            total_flips_++;
        }
    }
};

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Amortized Analysis C++17 Verification..." << std::endl;

    // 1. Verify Dynamic Array Amortized O(1) Expansion
    {
        TrackedDynamicArray arr;
        const size_t N = 100000;
        for (size_t i = 0; i < N; ++i) {
            int64_t phi_before = arr.potential();
            uint64_t copies_before = arr.total_copies();
            arr.push_back(static_cast<int>(i));
            uint64_t copies_cost = arr.total_copies() - copies_before;
            int64_t actual_cost = 1 + static_cast<int64_t>(copies_cost);
            int64_t delta_phi = arr.potential() - phi_before;
            int64_t amortized_cost = actual_cost + delta_phi;

            // Theoretical proof: amortized cost of push_back is at most 3
            assert(amortized_cost <= 3);
        }

        // Aggregate analysis check: total copies must strictly be < 2 * N
        assert(arr.total_copies() < 2 * N);
        assert(arr.size() == N);
    }

    // 2. Verify Multipop Stack Amortized O(1)
    {
        TrackedMultipopStack st;
        for (int i = 0; i < 500; ++i) st.push(i);
        st.multipop(200);
        for (int i = 0; i < 300; ++i) st.push(i);
        st.multipop(1000); // Pops everything remaining

        assert(st.size() == 0);
        // Aggregate property: total pops can never exceed total pushes
        assert(st.total_pops() <= st.total_pushes());
    }

    // 3. Verify Binary Counter Amortized O(1) Bit Flips
    {
        TrackedBinaryCounter counter;
        const size_t N = 65536;
        for (size_t i = 0; i < N; ++i) {
            counter.increment();
        }

        // Aggregate property: total flips strictly < 2 * N
        assert(counter.total_flips() < 2 * N);
    }

    std::cout << "[PASSED] Amortized Analysis C++17 All Tests Passed!" << std::endl;
    return 0;
}
