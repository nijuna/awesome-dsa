/**
 * Reference Implementation: Monotonic Stack and Queue
 * Demonstrates:
 * 1. Next Greater Element (NGE) and Daily Temperatures (decreasing stack of indices).
 * 2. Stock Span Problem (previous greater element barrier).
 * 3. Largest Rectangle in Histogram (previous and next smaller element boundaries).
 * 4. Trapping Rain Water (horizontal basin accumulation via monotonic stack).
 * 5. Next Greater Element II on Circular Arrays (2n virtual scan).
 * 6. Sliding Window Maximum and Minimum (monotonic deque with expiration).
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <stack>
#include <deque>
#include <algorithm>
#include <cassert>

namespace monotonic_structures {

// ============================================================================
// 1. Next Greater Element and Daily Temperatures
// ============================================================================

/**
 * Computes the index of the next strictly greater element for each position.
 * Returns -1 if no greater element exists to the right.
 * Time: O(n) amortized, Space: O(n).
 */
std::vector<int> next_greater_indices(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    std::vector<int> ans(n, -1);
    std::stack<int> st; // stores indices with decreasing values

    for (int i = 0; i < n; ++i) {
        while (!st.empty() && a[i] > a[st.top()]) {
            ans[st.top()] = i;
            st.pop();
        }
        st.push(i);
    }

    return ans;
}

/**
 * Daily Temperatures: Number of days to wait until a warmer temperature.
 * Returns 0 if no warmer day appears in the future.
 * Time: O(n) amortized, Space: O(n).
 */
std::vector<int> daily_temperatures(const std::vector<int>& temp) {
    int n = static_cast<int>(temp.size());
    std::vector<int> ans(n, 0);
    std::stack<int> st;

    for (int i = 0; i < n; ++i) {
        while (!st.empty() && temp[i] > temp[st.top()]) {
            int j = st.top();
            st.pop();
            ans[j] = i - j;
        }
        st.push(i);
    }

    return ans;
}

// ============================================================================
// 2. Stock Span Problem
// ============================================================================

/**
 * Stock Span: Consecutive days ending at day i with price <= price[i].
 * Time: O(n) amortized, Space: O(n).
 */
std::vector<int> stock_span(const std::vector<int>& price) {
    int n = static_cast<int>(price.size());
    std::vector<int> span(n, 0);
    std::stack<int> st; // indices of strictly greater price barriers

    for (int i = 0; i < n; ++i) {
        while (!st.empty() && price[st.top()] <= price[i]) {
            st.pop();
        }

        int prev_greater = st.empty() ? -1 : st.top();
        span[i] = i - prev_greater;
        st.push(i);
    }

    return span;
}

// ============================================================================
// 3. Largest Rectangle in Histogram
// ============================================================================

/**
 * Largest Rectangle in Histogram:
 * Finds maximum rectangular area using previous and next smaller boundaries.
 * Width = right[i] - left[i] - 1.
 * Time: O(n) amortized, Space: O(n).
 */
long long largest_rectangle_histogram(const std::vector<int>& h) {
    int n = static_cast<int>(h.size());
    if (n == 0) return 0;

    std::vector<int> left(n), right(n);
    std::stack<int> st;

    // Previous smaller element index (strict boundary)
    for (int i = 0; i < n; ++i) {
        while (!st.empty() && h[st.top()] >= h[i]) {
            st.pop();
        }
        left[i] = st.empty() ? -1 : st.top();
        st.push(i);
    }

    while (!st.empty()) st.pop();

    // Next smaller element index (strict boundary)
    for (int i = n - 1; i >= 0; --i) {
        while (!st.empty() && h[st.top()] >= h[i]) {
            st.pop();
        }
        right[i] = st.empty() ? n : st.top();
        st.push(i);
    }

    long long max_area = 0;
    for (int i = 0; i < n; ++i) {
        long long width = right[i] - left[i] - 1;
        max_area = std::max(max_area, 1LL * h[i] * width);
    }

    return max_area;
}

// ============================================================================
// 4. Trapping Rain Water (Monotonic Stack Method)
// ============================================================================

/**
 * Trapping Rain Water:
 * Accumulates water volume by discovering horizontal basins bounded by taller walls.
 * Time: O(n) amortized, Space: O(n).
 */
long long trap_rain_water_stack(const std::vector<int>& h) {
    int n = static_cast<int>(h.size());
    long long water = 0;
    std::stack<int> st; // stores indices with decreasing heights

    for (int i = 0; i < n; ++i) {
        while (!st.empty() && h[i] > h[st.top()]) {
            int mid = st.top();
            st.pop();

            if (st.empty()) break; // No left wall boundary

            int left = st.top();
            int width = i - left - 1;
            int bounded_height = std::min(h[left], h[i]) - h[mid];
            water += 1LL * width * bounded_height;
        }
        st.push(i);
    }

    return water;
}

// ============================================================================
// 5. Circular Array Next Greater Element (NGE II)
// ============================================================================

/**
 * Next Greater Element on a Circular Array:
 * Scans 2n virtual positions; only pushes unresolved indices from the first pass.
 * Returns values of next greater elements (-1 if none exists).
 * Time: O(n) amortized, Space: O(n).
 */
std::vector<int> next_greater_circular(const std::vector<int>& a) {
    int n = static_cast<int>(a.size());
    if (n == 0) return {};

    std::vector<int> ans(n, -1);
    std::stack<int> st;

    for (int i = 0; i < 2 * n; ++i) {
        int idx = i % n;

        while (!st.empty() && a[idx] > a[st.top()]) {
            ans[st.top()] = a[idx];
            st.pop();
        }

        if (i < n) {
            st.push(idx);
        }
    }

    return ans;
}

// ============================================================================
// 6. Sliding Window Maximum and Minimum (Monotonic Deque)
// ============================================================================

/**
 * Sliding Window Maximum:
 * Uses a monotonic deque maintaining indices of decreasing values.
 * Front is always the maximum of the current window of size k.
 * Time: O(n) amortized, Space: O(k).
 */
std::vector<int> sliding_window_maximum(const std::vector<int>& a, int k) {
    int n = static_cast<int>(a.size());
    if (n == 0 || k <= 0) return {};
    k = std::min(k, n);

    std::deque<int> dq;
    std::vector<int> ans;
    ans.reserve(n - k + 1);

    for (int i = 0; i < n; ++i) {
        // Step 1: Remove expired indices from front
        while (!dq.empty() && dq.front() <= i - k) {
            dq.pop_front();
        }

        // Step 2: Remove dominated elements from back
        while (!dq.empty() && a[dq.back()] <= a[i]) {
            dq.pop_back();
        }

        // Step 3: Push current index
        dq.push_back(i);

        // Step 4: Record answer once first window is formed
        if (i >= k - 1) {
            ans.push_back(a[dq.front()]);
        }
    }

    return ans;
}

/**
 * Sliding Window Minimum:
 * Uses a monotonic deque maintaining indices of increasing values.
 * Front is always the minimum of the current window of size k.
 * Time: O(n) amortized, Space: O(k).
 */
std::vector<int> sliding_window_minimum(const std::vector<int>& a, int k) {
    int n = static_cast<int>(a.size());
    if (n == 0 || k <= 0) return {};
    k = std::min(k, n);

    std::deque<int> dq;
    std::vector<int> ans;
    ans.reserve(n - k + 1);

    for (int i = 0; i < n; ++i) {
        // Step 1: Remove expired indices from front
        while (!dq.empty() && dq.front() <= i - k) {
            dq.pop_front();
        }

        // Step 2: Remove dominated elements from back
        while (!dq.empty() && a[dq.back()] >= a[i]) {
            dq.pop_back();
        }

        // Step 3: Push current index
        dq.push_back(i);

        // Step 4: Record answer once first window is formed
        if (i >= k - 1) {
            ans.push_back(a[dq.front()]);
        }
    }

    return ans;
}

} // namespace monotonic_structures

// ============================================================================
// Unit Tests
// ============================================================================

int main() {
    using namespace monotonic_structures;

    // Test 1: Next Greater Indices and Daily Temperatures
    // a = [2, 1, 2, 4, 3] -> NGE indices: [3, 2, 3, -1, -1]
    {
        std::vector<int> a = {2, 1, 2, 4, 3};
        auto nge = next_greater_indices(a);
        std::vector<int> expected_nge = {3, 2, 3, -1, -1};
        assert(nge == expected_nge);

        // Daily temperatures: [73, 74, 75, 71, 69, 72, 76, 73] -> [1, 1, 4, 2, 1, 1, 0, 0]
        std::vector<int> temp = {73, 74, 75, 71, 69, 72, 76, 73};
        auto dt = daily_temperatures(temp);
        std::vector<int> expected_dt = {1, 1, 4, 2, 1, 1, 0, 0};
        assert(dt == expected_dt);
    }

    // Test 2: Stock Span Problem
    // prices = [100, 80, 60, 70, 60, 75, 85] -> span: [1, 1, 1, 2, 1, 4, 6]
    {
        std::vector<int> prices = {100, 80, 60, 70, 60, 75, 85};
        auto span = stock_span(prices);
        std::vector<int> expected_span = {1, 1, 1, 2, 1, 4, 6};
        assert(span == expected_span);
    }

    // Test 3: Largest Rectangle in Histogram
    // heights = [2, 1, 5, 6, 2, 3] -> max area = 10 (bars [5, 6] of width 2, height 5)
    {
        std::vector<int> h = {2, 1, 5, 6, 2, 3};
        assert(largest_rectangle_histogram(h) == 10);

        // All equal heights [2, 2, 2, 2] -> area = 8
        std::vector<int> eq = {2, 2, 2, 2};
        assert(largest_rectangle_histogram(eq) == 8);

        // Monotonically increasing [1, 2, 3, 4] -> max area = 6 (h=2, w=3 or h=3, w=2)
        std::vector<int> inc = {1, 2, 3, 4};
        assert(largest_rectangle_histogram(inc) == 6);

        // Empty histogram
        assert(largest_rectangle_histogram({}) == 0);
    }

    // Test 4: Trapping Rain Water (Stack Method)
    // h = [0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1] -> water = 6
    {
        std::vector<int> h = {0, 1, 0, 2, 1, 0, 1, 3, 2, 1, 2, 1};
        assert(trap_rain_water_stack(h) == 6);

        // Flat or decreasing -> 0
        assert(trap_rain_water_stack({3, 2, 1}) == 0);
        assert(trap_rain_water_stack({1, 2, 3}) == 0);
        assert(trap_rain_water_stack({}) == 0);
    }

    // Test 5: Circular Next Greater Element (NGE II)
    // a = [1, 2, 1] -> [2, -1, 2]
    {
        std::vector<int> a = {1, 2, 1};
        auto circ = next_greater_circular(a);
        std::vector<int> expected_circ = {2, -1, 2};
        assert(circ == expected_circ);

        std::vector<int> a2 = {1, 2, 3, 4, 3};
        auto circ2 = next_greater_circular(a2);
        std::vector<int> expected_circ2 = {2, 3, 4, -1, 4};
        assert(circ2 == expected_circ2);
    }

    // Test 6: Sliding Window Maximum and Minimum
    // a = [1, 3, -1, -3, 5, 3, 6, 7], k = 3
    // max: [3, 3, 5, 5, 6, 7]
    // min: [-1, -3, -3, -3, 3, 3]
    {
        std::vector<int> a = {1, 3, -1, -3, 5, 3, 6, 7};
        int k = 3;

        auto win_max = sliding_window_maximum(a, k);
        std::vector<int> expected_max = {3, 3, 5, 5, 6, 7};
        assert(win_max == expected_max);

        auto win_min = sliding_window_minimum(a, k);
        std::vector<int> expected_min = {-1, -3, -3, -3, 3, 3};
        assert(win_min == expected_min);

        // k = 1
        assert(sliding_window_maximum(a, 1) == a);
        assert(sliding_window_minimum(a, 1) == a);

        // Empty array
        assert(sliding_window_maximum({}, 3).empty());
        assert(sliding_window_minimum({}, 3).empty());
    }

    std::cout << "[PASS] All Monotonic Stack and Queue C++ unit tests passed." << std::endl;
    return 0;
}
