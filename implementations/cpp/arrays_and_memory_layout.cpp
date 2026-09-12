#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <vector>
#include <numeric>
#include <atomic>

namespace dsa {

/**
 * @brief Contiguous 2D Matrix stored in row-major order within a flat 1D buffer.
 * Index formula: index(row, col) = row * cols + col.
 */
template <typename T>
class RowMajorMatrix {
private:
    size_t rows_;
    size_t cols_;
    std::vector<T> data_;

public:
    RowMajorMatrix(size_t rows, size_t cols, T init_val = T())
        : rows_(rows), cols_(cols), data_(rows * cols, init_val) {}

    size_t rows() const noexcept { return rows_; }
    size_t cols() const noexcept { return cols_; }
    size_t size() const noexcept { return data_.size(); }

    inline size_t linear_index(size_t r, size_t c) const noexcept {
        return r * cols_ + c;
    }

    T& at(size_t r, size_t c) {
        return data_[linear_index(r, c)];
    }

    const T& at(size_t r, size_t c) const {
        return data_[linear_index(r, c)];
    }

    const T* raw_data() const noexcept {
        return data_.data();
    }
};

/**
 * @brief Contiguous 2D Matrix stored in column-major order within a flat 1D buffer.
 * Index formula: index(row, col) = col * rows + row.
 */
template <typename T>
class ColumnMajorMatrix {
private:
    size_t rows_;
    size_t cols_;
    std::vector<T> data_;

public:
    ColumnMajorMatrix(size_t rows, size_t cols, T init_val = T())
        : rows_(rows), cols_(cols), data_(rows * cols, init_val) {}

    size_t rows() const noexcept { return rows_; }
    size_t cols() const noexcept { return cols_; }
    size_t size() const noexcept { return data_.size(); }

    inline size_t linear_index(size_t r, size_t c) const noexcept {
        return c * rows_ + r;
    }

    T& at(size_t r, size_t c) {
        return data_[linear_index(r, c)];
    }

    const T& at(size_t r, size_t c) const {
        return data_[linear_index(r, c)];
    }

    const T* raw_data() const noexcept {
        return data_.data();
    }
};

/**
 * @brief Demonstration structs for memory alignment and compiler padding.
 */
struct UnpackedStruct {
    char a;       // 1 byte
    // 7 bytes padding
    double b;     // 8 bytes
    char c;       // 1 byte
    // 3 bytes padding
    int d;        // 4 bytes
    // 4 bytes trailing padding
}; // Total size: 24 bytes

struct PackedStruct {
    double b;     // 8 bytes (largest alignment first)
    int d;        // 4 bytes
    char a;       // 1 byte
    char c;       // 1 byte
    // 2 bytes trailing padding
}; // Total size: 16 bytes

/**
 * @brief Cache-line aligned counter to eliminate false sharing.
 * Aligns on typical 64-byte hardware cache lines.
 */
struct alignas(64) PaddedAtomicCounter {
    std::atomic<int64_t> value{0};
};

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Arrays and Memory Layout C++17 Verification..." << std::endl;

    // 1. Verify Contiguous Memory Formula
    {
        std::vector<int> arr = {10, 20, 30, 40, 50};
        const uintptr_t base = reinterpret_cast<uintptr_t>(&arr[0]);
        for (size_t i = 0; i < arr.size(); ++i) {
            uintptr_t expected_addr = base + i * sizeof(int);
            uintptr_t actual_addr = reinterpret_cast<uintptr_t>(&arr[i]);
            assert(actual_addr == expected_addr);
        }
    }

    // 2. Row-Major Matrix Verification
    {
        size_t R = 3, C = 4;
        RowMajorMatrix<int> rm(R, C);
        int counter = 1;
        for (size_t i = 0; i < R; ++i) {
            for (size_t j = 0; j < C; ++j) {
                rm.at(i, j) = counter++;
            }
        }

        // Verify underlying memory is strictly sequential row by row: 1, 2, 3, ..., 12
        const int* raw = rm.raw_data();
        for (int k = 0; k < 12; ++k) {
            assert(raw[k] == k + 1);
        }
    }

    // 3. Column-Major Matrix Verification
    {
        size_t R = 3, C = 4;
        ColumnMajorMatrix<int> cm(R, C);
        // Fill such that cm.at(i, j) = 10 * i + j
        for (size_t i = 0; i < R; ++i) {
            for (size_t j = 0; j < C; ++j) {
                cm.at(i, j) = static_cast<int>(10 * i + j);
            }
        }

        // Under column-major: column 0 elements sit first: (0,0), (1,0), (2,0)
        const int* raw = cm.raw_data();
        assert(raw[0] == 0);   // (0, 0)
        assert(raw[1] == 10);  // (1, 0)
        assert(raw[2] == 20);  // (2, 0)
        assert(raw[3] == 1);   // (0, 1)
        assert(raw[4] == 11);  // (1, 1)
        assert(raw[5] == 21);  // (2, 1)
    }

    // 4. Memory Padding and Alignment Verification
    {
        assert(sizeof(UnpackedStruct) == 24);
        assert(sizeof(PackedStruct) == 16);
        assert(sizeof(PackedStruct) < sizeof(UnpackedStruct));
        assert(alignof(UnpackedStruct) == 8);
        assert(alignof(PackedStruct) == 8);
    }

    // 5. Cache-Line Alignment Verification
    {
        assert(alignof(PaddedAtomicCounter) == 64);
        assert(sizeof(PaddedAtomicCounter) == 64);

        PaddedAtomicCounter counters[2];
        uintptr_t addr0 = reinterpret_cast<uintptr_t>(&counters[0]);
        uintptr_t addr1 = reinterpret_cast<uintptr_t>(&counters[1]);
        assert(addr0 % 64 == 0);
        assert(addr1 % 64 == 0);
        assert(addr1 - addr0 == 64); // Separate cache lines, 0% false sharing
    }

    std::cout << "[PASSED] Arrays and Memory Layout C++17 All Tests Passed!" << std::endl;
    return 0;
}
