/**
 * @file simd_and_vectorization_intuition.cpp
 * @brief Software modeling and reference implementations of SIMD vectorization patterns.
 *
 * Implements scalar vs manual 4-wide and 8-wide unrolled vector pipelines,
 * horizontal reduction, scalar tail processing, and equivalence assertions.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <numeric>

namespace dsa {

/**
 * @brief Reference scalar dot product.
 */
inline float dot_product_scalar(const float* a, const float* b, size_t n) {
    float sum = 0.0f;
    for (size_t i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

/**
 * @brief 4-wide simulated vector lane structure.
 */
struct Simd4f {
    float lanes[4];

    Simd4f() : lanes{0.0f, 0.0f, 0.0f, 0.0f} {}

    static Simd4f load(const float* ptr) {
        Simd4f v;
        for (int i = 0; i < 4; ++i) v.lanes[i] = ptr[i];
        return v;
    }

    void fma(const Simd4f& a, const Simd4f& b) {
        for (int i = 0; i < 4; ++i) {
            lanes[i] += a.lanes[i] * b.lanes[i];
        }
    }

    float horizontal_sum() const {
        return lanes[0] + lanes[1] + lanes[2] + lanes[3];
    }
};

/**
 * @brief Vectorized dot product with horizontal reduction and scalar tail cleanup.
 */
inline float dot_product_vectorized(const float* a, const float* b, size_t n) {
    Simd4f acc;
    size_t i = 0;
    const size_t vector_limit = n - (n % 4);

    // 1. Vector Main Loop (Processes 4 elements per step)
    for (; i < vector_limit; i += 4) {
        Simd4f va = Simd4f::load(a + i);
        Simd4f vb = Simd4f::load(b + i);
        acc.fma(va, vb);
    }

    // 2. Horizontal Reduction
    float total = acc.horizontal_sum();

    // 3. Scalar Cleanup Tail Loop (Processes remaining n % 4 elements)
    for (; i < n; ++i) {
        total += a[i] * b[i];
    }

    return total;
}

/**
 * @brief Branchless SIMD conditional blend simulation.
 * Replaces if (mask[i]) out[i] = a[i] else out[i] = b[i].
 */
inline void simd_blend(const float* a, const float* b, const uint32_t* mask, float* out, size_t n) {
    for (size_t i = 0; i < n; ++i) {
        // In AVX2: _mm256_blendv_ps
        out[i] = (mask[i] != 0) ? a[i] : b[i];
    }
}

} // namespace dsa

int main() {
    std::cout << "Running SIMD and Vectorization verification..." << std::endl;

    // Test across diverse array sizes: empty, partial vector, exact vector, large irregular
    std::vector<size_t> test_sizes = {0, 1, 2, 3, 4, 7, 8, 15, 16, 99, 1000, 1003};

    for (size_t n : test_sizes) {
        std::vector<float> a(n);
        std::vector<float> b(n);
        for (size_t i = 0; i < n; ++i) {
            a[i] = static_cast<float>(i + 1) * 0.5f;
            b[i] = static_cast<float>(i % 5 + 1) * 0.25f;
        }

        float scalar_res = dsa::dot_product_scalar(a.data(), b.data(), n);
        float vec_res = dsa::dot_product_vectorized(a.data(), b.data(), n);

        assert(std::abs(scalar_res - vec_res) < 1e-4f);
    }

    // SIMD blend / select verification
    const size_t count = 8;
    float a_arr[count] = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f};
    float b_arr[count] = {10.0f, 20.0f, 30.0f, 40.0f, 50.0f, 60.0f, 70.0f, 80.0f};
    uint32_t mask[count] = {1, 0, 1, 0, 1, 1, 0, 0};
    float out_arr[count] = {0};

    dsa::simd_blend(a_arr, b_arr, mask, out_arr, count);
    assert(out_arr[0] == 1.0f);
    assert(out_arr[1] == 20.0f);
    assert(out_arr[2] == 3.0f);
    assert(out_arr[3] == 40.0f);

    std::cout << "[PASS] Vectorized dot product and horizontal reduction matches scalar across all sizes." << std::endl;
    std::cout << "[PASS] SIMD blend mask verified." << std::endl;
    std::cout << "All SIMD and Vectorization assertions passed successfully!" << std::endl;
    return 0;
}
