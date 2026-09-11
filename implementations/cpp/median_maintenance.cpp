/**
 * Reference Implementation: Median Maintenance & Sliding Window Median
 * Demonstrates:
 * 1. Dual-heap running median over an unbounded stream (O(log n) insert, O(1) query).
 * 2. Sliding-window median with hash-map lazy deletion (O(log k) amortized per step).
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <functional>
#include <cassert>
#include <stdexcept>
#include <algorithm>
#include <cmath>

// ============================================================================
// 1. Dual-Heap Running Median (Unbounded Insert-Only Stream)
// ============================================================================
template <typename T>
class MedianMaintenance {
private:
    std::priority_queue<T> low_;                                       // Max-heap: lower half
    std::priority_queue<T, std::vector<T>, std::greater<T>> high_;     // Min-heap: upper half

    void balance() {
        if (low_.size() > high_.size() + 1) {
            high_.push(low_.top());
            low_.pop();
        } else if (high_.size() > low_.size()) {
            low_.push(high_.top());
            high_.pop();
        }
    }

public:
    MedianMaintenance() = default;

    [[nodiscard]] std::size_t size() const noexcept {
        return low_.size() + high_.size();
    }

    [[nodiscard]] bool empty() const noexcept {
        return low_.empty() && high_.empty();
    }

    void add(const T& x) {
        if (low_.empty() || x <= low_.top()) {
            low_.push(x);
        } else {
            high_.push(x);
        }
        balance();
    }

    T lower_median() const {
        if (empty()) {
            throw std::underflow_error("MedianMaintenance::lower_median(): stream is empty");
        }
        return low_.top();
    }

    T upper_median() const {
        if (empty()) {
            throw std::underflow_error("MedianMaintenance::upper_median(): stream is empty");
        }
        if (low_.size() > high_.size()) {
            return low_.top();
        }
        return high_.top();
    }

    double find_median() const {
        if (empty()) {
            throw std::underflow_error("MedianMaintenance::find_median(): stream is empty");
        }
        if (low_.size() > high_.size()) {
            return static_cast<double>(low_.top());
        }
        return (static_cast<double>(low_.top()) + static_cast<double>(high_.top())) / 2.0;
    }
};

// ============================================================================
// 2. Sliding Window Median (Fixed Window Size K with Lazy Deletion)
// ============================================================================
class SlidingWindowMedian {
private:
    std::priority_queue<int64_t> low_;                                                // Max-heap
    std::priority_queue<int64_t, std::vector<int64_t>, std::greater<int64_t>> high_;  // Min-heap
    std::unordered_map<int64_t, int> delayed_;                                        // Stale counts
    std::size_t low_valid_ = 0;
    std::size_t high_valid_ = 0;

    void prune(std::priority_queue<int64_t>& heap) {
        while (!heap.empty()) {
            int64_t x = heap.top();
            auto it = delayed_.find(x);
            if (it != delayed_.end() && it->second > 0) {
                if (--it->second == 0) {
                    delayed_.erase(it);
                }
                heap.pop();
            } else {
                break;
            }
        }
    }

    void prune(std::priority_queue<int64_t, std::vector<int64_t>, std::greater<int64_t>>& heap) {
        while (!heap.empty()) {
            int64_t x = heap.top();
            auto it = delayed_.find(x);
            if (it != delayed_.end() && it->second > 0) {
                if (--it->second == 0) {
                    delayed_.erase(it);
                }
                heap.pop();
            } else {
                break;
            }
        }
    }

    void balance() {
        if (low_valid_ > high_valid_ + 1) {
            high_.push(low_.top());
            low_.pop();
            --low_valid_;
            ++high_valid_;
            prune(low_);
        } else if (high_valid_ > low_valid_) {
            low_.push(high_.top());
            high_.pop();
            --high_valid_;
            ++low_valid_;
            prune(high_);
        }
    }

public:
    SlidingWindowMedian() = default;

    void add(int64_t x) {
        if (low_.empty() || x <= low_.top()) {
            low_.push(x);
            ++low_valid_;
        } else {
            high_.push(x);
            ++high_valid_;
        }
        balance();
    }

    void remove(int64_t x) {
        ++delayed_[x];
        if (!low_.empty() && x <= low_.top()) {
            --low_valid_;
            if (x == low_.top()) {
                prune(low_);
            }
        } else {
            --high_valid_;
            if (!high_.empty() && x == high_.top()) {
                prune(high_);
            }
        }
        balance();
    }

    double find_median() {
        prune(low_);
        prune(high_);
        if (low_valid_ > high_valid_) {
            return static_cast<double>(low_.top());
        }
        return (static_cast<double>(low_.top()) + static_cast<double>(high_.top())) / 2.0;
    }
};

// ============================================================================
// Unit Tests
// ============================================================================
int main() {
    // Test 1: Unbounded Running Median
    MedianMaintenance<int> mm;
    assert(mm.empty());
    assert(mm.size() == 0);

    // Stream: 5, 2, 10, 4, 8
    mm.add(5);
    assert(mm.find_median() == 5.0);
    assert(mm.lower_median() == 5);

    mm.add(2);
    assert(mm.find_median() == 3.5);
    assert(mm.lower_median() == 2);
    assert(mm.upper_median() == 5);

    mm.add(10);
    assert(mm.find_median() == 5.0);

    mm.add(4);
    assert(mm.find_median() == 4.5);
    assert(mm.lower_median() == 4);
    assert(mm.upper_median() == 5);

    mm.add(8);
    assert(mm.find_median() == 5.0);

    // Test 2: Stress test against sorting baseline
    std::vector<int> stream = {15, -3, 42, 7, 0, -100, 88, 23, 14, 5, 9, 11, 2};
    MedianMaintenance<int> mm_stress;
    std::vector<int> buffer;

    for (int val : stream) {
        mm_stress.add(val);
        buffer.push_back(val);
        std::sort(buffer.begin(), buffer.end());

        double expected = 0.0;
        std::size_t n = buffer.size();
        if (n % 2 == 1) {
            expected = static_cast<double>(buffer[n / 2]);
        } else {
            expected = (static_cast<double>(buffer[n / 2 - 1]) + static_cast<double>(buffer[n / 2])) / 2.0;
        }

        assert(std::abs(mm_stress.find_median() - expected) < 1e-9);
    }

    // Test 3: Sliding Window Median
    std::vector<int64_t> nums = {1, 3, -1, -3, 5, 3, 6, 7};
    std::size_t k = 3;
    SlidingWindowMedian swm;
    std::vector<double> results;

    for (std::size_t i = 0; i < nums.size(); ++i) {
        swm.add(nums[i]);
        if (i >= k) {
            swm.remove(nums[i - k]);
        }
        if (i >= k - 1) {
            results.push_back(swm.find_median());
        }
    }

    // Expected for [1, 3, -1, -3, 5, 3, 6, 7] with k=3:
    // [1, 3, -1] -> sorted [-1, 1, 3] -> median 1.0
    // [3, -1, -3] -> sorted [-3, -1, 3] -> median -1.0
    // [-1, -3, 5] -> sorted [-3, -1, 5] -> median -1.0
    // [-3, 5, 3] -> sorted [-3, 3, 5] -> median 3.0
    // [5, 3, 6] -> sorted [3, 5, 6] -> median 5.0
    // [3, 6, 7] -> sorted [3, 6, 7] -> median 6.0
    std::vector<double> expected_sw = {1.0, -1.0, -1.0, 3.0, 5.0, 6.0};
    assert(results.size() == expected_sw.size());
    for (std::size_t i = 0; i < results.size(); ++i) {
        assert(std::abs(results[i] - expected_sw[i]) < 1e-9);
    }

    // Test 4: Underflow check
    MedianMaintenance<int> empty_mm;
    try {
        empty_mm.find_median();
        assert(false);
    } catch (const std::underflow_error&) {
        // Expected
    }

    std::cout << "[PASS] All MedianMaintenance & SlidingWindowMedian C++ unit tests passed.\n";
    return 0;
}
