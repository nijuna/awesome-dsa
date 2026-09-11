/**
 * Reference Implementation: Divide and Conquer Paradigm Mechanics
 * Demonstrates:
 * 1. Merge Sort (Recursive Half-Splitting & Stable Buffer Merging, O(N log N)).
 * 2. Inversion Counting via Augmented Merge Sort (O(N log N)).
 * 3. Binary Search (Degenerate Divide-and-Conquer, O(log N)).
 * 4. Maximum Subarray via Divide-and-Conquer (O(N log N)) vs. Kadane's Algorithm (O(N)).
 * 5. QuickSelect (Single-Branch Selection for k-th Smallest Element, O(N) Average).
 * 6. Karatsuba Fast Integer Multiplication (Recurrence T(N) = 3 T(N/2) + O(N) => O(N^1.585)).
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <stdexcept>
#include <limits>
#include <cstdint>
#include <cassert>

// ============================================================================
// 1. Merge Sort
// ============================================================================
void merge_sort(std::vector<int>& a) {
    std::vector<int> temp(a.size());

    auto solve = [&](auto&& self, int left, int right) -> void {
        if (right - left <= 1) {
            return;
        }

        int mid = left + (right - left) / 2;
        self(self, left, mid);
        self(self, mid, right);

        int i = left, j = mid, k = left;
        while (i < mid && j < right) {
            if (a[i] <= a[j]) {
                temp[k++] = a[i++];
            } else {
                temp[k++] = a[j++];
            }
        }
        while (i < mid) temp[k++] = a[i++];
        while (j < right) temp[k++] = a[j++];

        for (int p = left; p < right; ++p) {
            a[p] = temp[p];
        }
    };

    solve(solve, 0, static_cast<int>(a.size()));
}

// ============================================================================
// 2. Inversion Counting via Augmented Merge Sort
// ============================================================================
long long count_inversions(std::vector<int>& a) {
    std::vector<int> temp(a.size());

    auto solve = [&](auto&& self, int left, int right) -> long long {
        if (right - left <= 1) {
            return 0;
        }

        int mid = left + (right - left) / 2;
        long long inv = self(self, left, mid) + self(self, mid, right);

        int i = left, j = mid, k = left;
        while (i < mid && j < right) {
            if (a[i] <= a[j]) {
                temp[k++] = a[i++];
            } else {
                temp[k++] = a[j++];
                inv += (mid - i); // All remaining elements in left half form inversions
            }
        }

        while (i < mid) temp[k++] = a[i++];
        while (j < right) temp[k++] = a[j++];

        for (int p = left; p < right; ++p) {
            a[p] = temp[p];
        }

        return inv;
    };

    return solve(solve, 0, static_cast<int>(a.size()));
}

// ============================================================================
// 3. Binary Search (Safe Midpoint Calculation)
// ============================================================================
int binary_search_index(const std::vector<int>& a, int target) {
    int low = 0, high = static_cast<int>(a.size()) - 1;

    while (low <= high) {
        int mid = low + (high - low) / 2; // Prevents 32-bit signed integer overflow
        if (a[mid] == target) {
            return mid;
        } else if (a[mid] < target) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    return -1;
}

// ============================================================================
// 4. Maximum Subarray: Divide-and-Conquer vs. Kadane's Algorithm
// ============================================================================
long long maximum_subarray_dc(const std::vector<int>& a) {
    if (a.empty()) return 0;

    auto solve = [&](auto&& self, int left, int right) -> long long {
        if (right - left == 1) {
            return a[left];
        }

        int mid = left + (right - left) / 2;
        long long left_best = self(self, left, mid);
        long long right_best = self(self, mid, right);

        long long best_left_suffix = std::numeric_limits<long long>::min();
        long long sum = 0;
        for (int i = mid - 1; i >= left; --i) {
            sum += a[i];
            best_left_suffix = std::max(best_left_suffix, sum);
        }

        long long best_right_prefix = std::numeric_limits<long long>::min();
        sum = 0;
        for (int i = mid; i < right; ++i) {
            sum += a[i];
            best_right_prefix = std::max(best_right_prefix, sum);
        }

        long long cross = best_left_suffix + best_right_prefix;
        return std::max({left_best, right_best, cross});
    };

    return solve(solve, 0, static_cast<int>(a.size()));
}

long long maximum_subarray_kadane(const std::vector<int>& a) {
    if (a.empty()) return 0;
    long long max_so_far = a[0];
    long long curr_max = a[0];

    for (size_t i = 1; i < a.size(); ++i) {
        curr_max = std::max<long long>(a[i], curr_max + a[i]);
        max_so_far = std::max(max_so_far, curr_max);
    }
    return max_so_far;
}

// ============================================================================
// 5. QuickSelect (k-th Smallest Element, 0-Indexed)
// ============================================================================
int quickselect(std::vector<int> a, int k) {
    if (k < 0 || k >= static_cast<int>(a.size())) {
        throw std::out_of_range("k out of range");
    }

    int left = 0, right = static_cast<int>(a.size()) - 1;

    while (true) {
        int pivot = a[right];
        int i = left;

        for (int j = left; j < right; ++j) {
            if (a[j] <= pivot) {
                std::swap(a[i], a[j]);
                ++i;
            }
        }
        std::swap(a[i], a[right]);

        if (i == k) {
            return a[i];
        } else if (k < i) {
            right = i - 1;
        } else {
            left = i + 1;
        }
    }
}

// ============================================================================
// 6. Karatsuba Fast Integer Multiplication
// ============================================================================
uint64_t karatsuba_u32(uint32_t x, uint32_t y) {
    // Base case: below 16 bits, standard multiplication is faster
    if (x < (1U << 16) && y < (1U << 16)) {
        return static_cast<uint64_t>(x) * y;
    }

    uint32_t x_h = x >> 16;
    uint32_t x_l = x & 0xFFFF;
    uint32_t y_h = y >> 16;
    uint32_t y_l = y & 0xFFFF;

    uint64_t z2 = karatsuba_u32(x_h, y_h);
    uint64_t z0 = karatsuba_u32(x_l, y_l);
    uint64_t z1 = static_cast<uint64_t>(x_h + x_l) * (y_h + y_l) - z2 - z0;

    return (z2 << 32) + (z1 << 16) + z0;
}

// Decimal string addition & multiplication for arbitrarily large integers
static std::string add_str(const std::string& num1, const std::string& num2) {
    std::string res = "";
    int i = static_cast<int>(num1.size()) - 1;
    int j = static_cast<int>(num2.size()) - 1;
    int carry = 0;

    while (i >= 0 || j >= 0 || carry) {
        int sum = carry;
        if (i >= 0) sum += num1[i--] - '0';
        if (j >= 0) sum += num2[j--] - '0';
        carry = sum / 10;
        res.push_back(static_cast<char>((sum % 10) + '0'));
    }
    std::reverse(res.begin(), res.end());
    return res;
}

static std::string sub_str(const std::string& num1, const std::string& num2) {
    // Assumes num1 >= num2
    std::string res = "";
    int i = static_cast<int>(num1.size()) - 1;
    int j = static_cast<int>(num2.size()) - 1;
    int borrow = 0;

    while (i >= 0) {
        int diff = (num1[i] - '0') - borrow - (j >= 0 ? (num2[j] - '0') : 0);
        if (diff < 0) {
            diff += 10;
            borrow = 1;
        } else {
            borrow = 0;
        }
        res.push_back(static_cast<char>(diff + '0'));
        --i;
        --j;
    }
    while (res.size() > 1 && res.back() == '0') {
        res.pop_back();
    }
    std::reverse(res.begin(), res.end());
    return res;
}

std::string karatsuba_str(std::string x, std::string y) {
    if (x.size() < 4 || y.size() < 4) {
        long long prod = std::stoll(x) * std::stoll(y);
        return std::to_string(prod);
    }

    size_t n = std::max(x.size(), y.size());
    if (n % 2 != 0) ++n;

    while (x.size() < n) x.insert(x.begin(), '0');
    while (y.size() < n) y.insert(y.begin(), '0');

    size_t m = n / 2;
    std::string x_h = x.substr(0, m);
    std::string x_l = x.substr(m);
    std::string y_h = y.substr(0, m);
    std::string y_l = y.substr(m);

    std::string z2 = karatsuba_str(x_h, y_h);
    std::string z0 = karatsuba_str(x_l, y_l);
    std::string z1_raw = karatsuba_str(add_str(x_h, x_l), add_str(y_h, y_l));
    std::string z1 = sub_str(sub_str(z1_raw, z2), z0);

    // Combine: z2 * 10^(2m) + z1 * 10^m + z0
    std::string p2 = z2 + std::string(2 * (n - m), '0');
    std::string p1 = z1 + std::string(n - m, '0');
    std::string res = add_str(add_str(p2, p1), z0);

    // Strip leading zeros
    size_t non_zero = res.find_first_not_of('0');
    if (non_zero == std::string::npos) return "0";
    return res.substr(non_zero);
}

// ============================================================================
// Unit Tests & Invariant Verification
// ============================================================================
int main() {
    // ------------------------------------------------------------------------
    // Test 1: Merge Sort Correctness
    // ------------------------------------------------------------------------
    {
        std::vector<int> a = {38, 27, 43, 3, 9, 82, 10};
        std::vector<int> expected = {3, 9, 10, 27, 38, 43, 82};
        merge_sort(a);
        assert(a == expected);
    }

    // ------------------------------------------------------------------------
    // Test 2: Inversion Counting Verification
    // ------------------------------------------------------------------------
    {
        // Array {2, 4, 1, 3, 5} has 3 inversions: (2,1), (4,1), (4,3)
        std::vector<int> a = {2, 4, 1, 3, 5};
        long long inv = count_inversions(a);
        assert(inv == 3);

        // Reverse sorted: {5, 4, 3, 2, 1} => 5 * 4 / 2 = 10 inversions
        std::vector<int> rev = {5, 4, 3, 2, 1};
        assert(count_inversions(rev) == 10);
    }

    // ------------------------------------------------------------------------
    // Test 3: Binary Search Verification
    // ------------------------------------------------------------------------
    {
        std::vector<int> a = {-10, -3, 0, 5, 9, 12, 18, 42};
        assert(binary_search_index(a, 9) == 4);
        assert(binary_search_index(a, -10) == 0);
        assert(binary_search_index(a, 42) == 7);
        assert(binary_search_index(a, 100) == -1);
        assert(binary_search_index(a, -20) == -1);
    }

    // ------------------------------------------------------------------------
    // Test 4: Maximum Subarray Divide-and-Conquer vs. Kadane
    // ------------------------------------------------------------------------
    {
        // Classical Kadane array: {-2, 1, -3, 4, -1, 2, 1, -5, 4} => max is {4, -1, 2, 1} = 6
        std::vector<int> a = {-2, 1, -3, 4, -1, 2, 1, -5, 4};
        long long dc_res = maximum_subarray_dc(a);
        long long kadane_res = maximum_subarray_kadane(a);
        assert(dc_res == 6);
        assert(dc_res == kadane_res);

        // All negative array: {-5, -2, -8, -1, -4} => max is {-1}
        std::vector<int> neg = {-5, -2, -8, -1, -4};
        assert(maximum_subarray_dc(neg) == -1);
        assert(maximum_subarray_dc(neg) == maximum_subarray_kadane(neg));
    }

    // ------------------------------------------------------------------------
    // Test 5: QuickSelect Rank Selection
    // ------------------------------------------------------------------------
    {
        std::vector<int> a = {7, 10, 4, 3, 20, 15};
        // Sorted: {3, 4, 7, 10, 15, 20}
        assert(quickselect(a, 0) == 3);  // 1st smallest
        assert(quickselect(a, 2) == 7);  // 3rd smallest
        assert(quickselect(a, 5) == 20); // maximum
    }

    // ------------------------------------------------------------------------
    // Test 6: Karatsuba Multiplication
    // ------------------------------------------------------------------------
    {
        uint32_t x = 123456;
        uint32_t y = 654321;
        uint64_t expected = static_cast<uint64_t>(x) * y;
        uint64_t res_u32 = karatsuba_u32(x, y);
        assert(res_u32 == expected);

        std::string s_x = "123456789";
        std::string s_y = "987654321";
        std::string res_str = karatsuba_str(s_x, s_y);
        assert(res_str == "121932631112635269");
    }

    std::cout << "[PASS] All Divide and Conquer C++ unit tests passed.\n";
    return 0;
}
