/**
 * Reference Implementation: Dynamic Array (Custom Vector)
 * Demonstrates contiguous memory management, geometric growth factor (2.0x),
 * amortized O(1) appending, Rule of 5 RAII, and cache-friendly layout.
 *
 * Language: C++17
 */

#include <iostream>
#include <cstddef>
#include <cassert>
#include <stdexcept>
#include <utility>
#include <algorithm>

template <typename T>
class DynamicArray {
private:
    T* data_;
    std::size_t size_;
    std::size_t capacity_;

    void reallocate(std::size_t new_capacity) {
        if (new_capacity < size_) {
            new_capacity = size_;
        }
        T* new_data = new_capacity > 0 ? new T[new_capacity] : nullptr;
        for (std::size_t i = 0; i < size_; ++i) {
            new_data[i] = std::move(data_[i]);
        }
        delete[] data_;
        data_ = new_data;
        capacity_ = new_capacity;
    }

public:
    // Default constructor
    DynamicArray() : data_(nullptr), size_(0), capacity_(0) {}

    // Constructor with initial capacity
    explicit DynamicArray(std::size_t initial_capacity)
        : data_(initial_capacity > 0 ? new T[initial_capacity] : nullptr),
          size_(0),
          capacity_(initial_capacity) {}

    // Destructor
    ~DynamicArray() {
        delete[] data_;
    }

    // Copy Constructor (Deep copy)
    DynamicArray(const DynamicArray& other)
        : data_(other.capacity_ > 0 ? new T[other.capacity_] : nullptr),
          size_(other.size_),
          capacity_(other.capacity_) {
        for (std::size_t i = 0; i < size_; ++i) {
            data_[i] = other.data_[i];
        }
    }

    // Copy Assignment Operator
    DynamicArray& operator=(const DynamicArray& other) {
        if (this != &other) {
            T* new_data = other.capacity_ > 0 ? new T[other.capacity_] : nullptr;
            for (std::size_t i = 0; i < other.size_; ++i) {
                new_data[i] = other.data_[i];
            }
            delete[] data_;
            data_ = new_data;
            size_ = other.size_;
            capacity_ = other.capacity_;
        }
        return *this;
    }

    // Move Constructor
    DynamicArray(DynamicArray&& other) noexcept
        : data_(other.data_), size_(other.size_), capacity_(other.capacity_) {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // Move Assignment Operator
    DynamicArray& operator=(DynamicArray&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    [[nodiscard]] std::size_t size() const noexcept { return size_; }
    [[nodiscard]] std::size_t capacity() const noexcept { return capacity_; }
    [[nodiscard]] bool empty() const noexcept { return size_ == 0; }

    void reserve(std::size_t new_capacity) {
        if (new_capacity > capacity_) {
            reallocate(new_capacity);
        }
    }

    void shrink_to_fit() {
        if (capacity_ > size_) {
            reallocate(size_);
        }
    }

    void push_back(const T& val) {
        if (size_ >= capacity_) {
            std::size_t next_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
            reallocate(next_cap);
        }
        data_[size_++] = val;
    }

    void push_back(T&& val) {
        if (size_ >= capacity_) {
            std::size_t next_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
            reallocate(next_cap);
        }
        data_[size_++] = std::move(val);
    }

    void pop_back() {
        if (size_ == 0) {
            throw std::out_of_range("pop_back on empty DynamicArray");
        }
        --size_;
    }

    T& operator[](std::size_t idx) noexcept {
        return data_[idx];
    }

    const T& operator[](std::size_t idx) const noexcept {
        return data_[idx];
    }

    T& at(std::size_t idx) {
        if (idx >= size_) {
            throw std::out_of_range("Index out of range in DynamicArray::at");
        }
        return data_[idx];
    }

    const T& at(std::size_t idx) const {
        if (idx >= size_) {
            throw std::out_of_range("Index out of range in DynamicArray::at");
        }
        return data_[idx];
    }

    void insert(std::size_t idx, const T& val) {
        if (idx > size_) {
            throw std::out_of_range("Index out of range in DynamicArray::insert");
        }
        if (size_ >= capacity_) {
            std::size_t next_cap = (capacity_ == 0) ? 1 : capacity_ * 2;
            reallocate(next_cap);
        }
        for (std::size_t i = size_; i > idx; --i) {
            data_[i] = std::move(data_[i - 1]);
        }
        data_[idx] = val;
        ++size_;
    }

    void erase(std::size_t idx) {
        if (idx >= size_) {
            throw std::out_of_range("Index out of range in DynamicArray::erase");
        }
        for (std::size_t i = idx; i + 1 < size_; ++i) {
            data_[i] = std::move(data_[i + 1]);
        }
        --size_;
    }

    void clear() noexcept {
        size_ = 0;
    }
};

void run_tests() {
    DynamicArray<int> arr;
    assert(arr.empty());
    assert(arr.size() == 0);
    assert(arr.capacity() == 0);

    for (int i = 0; i < 10; ++i) {
        arr.push_back(i * 10);
    }
    assert(arr.size() == 10);
    assert(arr.capacity() >= 10);

    for (std::size_t i = 0; i < 10; ++i) {
        assert(arr[i] == static_cast<int>(i * 10));
        assert(arr.at(i) == static_cast<int>(i * 10));
    }

    // Bounds checking
    try {
        arr.at(10);
        assert(false);
    } catch (const std::out_of_range&) {
        // Expected
    }

    // Insert at index 3
    arr.insert(3, 999);
    assert(arr.size() == 11);
    assert(arr[3] == 999);
    assert(arr[4] == 30);

    // Erase at index 3
    arr.erase(3);
    assert(arr.size() == 10);
    assert(arr[3] == 30);

    // Pop back
    arr.pop_back();
    assert(arr.size() == 9);
    assert(arr[8] == 80);

    // Copy semantics
    DynamicArray<int> copy = arr;
    assert(copy.size() == arr.size());
    copy[0] = 777;
    assert(arr[0] == 0); // Deep copy verification

    // Move semantics
    DynamicArray<int> moved = std::move(copy);
    assert(moved.size() == 9);
    assert(moved[0] == 777);

    // Reserve & shrink_to_fit
    arr.reserve(50);
    assert(arr.capacity() >= 50);
    arr.shrink_to_fit();
    assert(arr.capacity() == arr.size());

    std::cout << "All DynamicArray C++ tests passed successfully!\n";
}

int main() {
    run_tests();
    return 0;
}
