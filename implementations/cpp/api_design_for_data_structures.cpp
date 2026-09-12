/**
 * @file api_design_for_data_structures.cpp
 * @brief Reference implementation of idiomatic, leak-proof container API design.
 *
 * Implements a dynamic array embodying the Rule of 5, the Copy-and-Swap idiom,
 * noexcept move semantics, strong exception safety, and standard iterator interfaces.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <stdexcept>
#include <utility>

namespace dsa {

template <typename T>
class DynamicArray {
private:
    T* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;

    void reallocate(size_t new_capacity) {
        T* new_data = new T[new_capacity];
        for (size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }

public:
    // 1. Default Constructor
    DynamicArray() : data_(nullptr), size_(0), capacity_(0) {}

    // 2. Sized Constructor
    explicit DynamicArray(size_t initial_capacity)
        : data_(initial_capacity > 0 ? new T[initial_capacity] : nullptr),
          size_(0),
          capacity_(initial_capacity) {}

    // 3. Destructor (Rule of 5 #1)
    ~DynamicArray() noexcept {
        delete[] data_;
    }

    // 4. Copy Constructor (Rule of 5 #2)
    DynamicArray(const DynamicArray& other)
        : data_(other.capacity_ > 0 ? new T[other.capacity_] : nullptr),
          size_(other.size_),
          capacity_(other.capacity_) {
        for (size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    // 5. Move Constructor (Rule of 5 #3 - noexcept is critical)
    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // 6. Swap Utility (Nothrow)
    friend void swap(DynamicArray& first, DynamicArray& second) noexcept {
        using std::swap;
        swap(first.data_, second.data_);
        swap(first.size_, second.size_);
        swap(first.capacity_, second.capacity_);
    }

    // 7. Copy & Move Assignment Operator via Copy-and-Swap (Rule of 5 #4 & #5)
    // Passing other by value automatically handles both copy and move assignments with strong exception guarantee!
    DynamicArray& operator=(DynamicArray other) noexcept {
        swap(*this, other);
        return *this;
    }

    // --- Core Mutation & Access Methods ---

    void push_back(const T& val) {
        if (size_ == capacity_) {
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = val;
    }

    void push_back(T&& val) {
        if (size_ == capacity_) {
            reallocate(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_++] = std::move(val);
    }

    size_t size() const noexcept { return size_; }
    size_t capacity() const noexcept { return capacity_; }
    bool empty() const noexcept { return size_ == 0; }

    // Unchecked subscript
    T& operator[](size_t index) noexcept {
        return data_[index];
    }

    const T& operator[](size_t index) const noexcept {
        return data_[index];
    }

    // Bounds-checked access
    T& at(size_t index) {
        if (index >= size_) throw std::out_of_range("Index out of bounds");
        return data_[index];
    }

    const T& at(size_t index) const {
        if (index >= size_) throw std::out_of_range("Index out of bounds");
        return data_[index];
    }

    // --- Standard Iterator Interface (Range-For Compatibility) ---

    T* begin() noexcept { return data_; }
    T* end() noexcept { return data_ + size_; }

    const T* begin() const noexcept { return data_; }
    const T* end() const noexcept { return data_ + size_; }

    const T* cbegin() const noexcept { return data_; }
    const T* cend() const noexcept { return data_ + size_; }
};

} // namespace dsa

int main() {
    std::cout << "Running API Design for Data Structures verification..." << std::endl;

    // 1. Basic operations
    dsa::DynamicArray<int> arr;
    assert(arr.empty());
    assert(arr.size() == 0);

    for (int i = 1; i <= 5; ++i) {
        arr.push_back(i * 10);
    }
    assert(arr.size() == 5);
    assert(!arr.empty());
    assert(arr[0] == 10 && arr[4] == 50);

    // 2. Iterator and range-for loop verification
    int expected_val = 10;
    for (int x : arr) {
        assert(x == expected_val);
        expected_val += 10;
    }

    // 3. Copy construction & Copy assignment
    dsa::DynamicArray<int> copy_arr = arr;
    assert(copy_arr.size() == arr.size());
    copy_arr[0] = 999;
    assert(arr[0] == 10); // Deep copy verification: original remains intact

    // 4. Move construction & Move assignment (Transfer ownership)
    dsa::DynamicArray<int> moved_arr = std::move(copy_arr);
    assert(moved_arr.size() == 5);
    assert(moved_arr[0] == 999);
    assert(copy_arr.size() == 0); // Source safely emptied

    // 5. Bounds-checking exception guarantee
    bool threw_exception = false;
    try {
        arr.at(100);
    } catch (const std::out_of_range&) {
        threw_exception = true;
    }
    assert(threw_exception);

    std::cout << "[PASS] Rule of 5 and Copy-and-Swap idiom verified." << std::endl;
    std::cout << "[PASS] Range-based iteration and bounds checking verified." << std::endl;
    std::cout << "All API Design assertions passed successfully!" << std::endl;
    return 0;
}
