/**
 * @file closest_pair_of_points.cpp
 * @brief Divide-and-Conquer Closest Pair of Points in O(N log N) Time.
 *
 * Implements Arthur's Two-Layer API:
 *   - Layer 1: Exact integer squared distance primitives, strip filtering,
 *              and geometric packing comparisons.
 *   - Layer 2: Safe, high-level ClosestPairEngine with divide-and-conquer,
 *              y-merge recursion, input validation, and differential testing against O(N^2) oracle.
 *
 * Compilation:
 *   g++ -std=c++17 -O3 -Wall -Wextra -Werror closest_pair_of_points.cpp -o closest_pair
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <limits>
#include <random>

namespace closest {

// ============================================================================
// Layer 1: Geometric Primitives & Distance Calculations
// ============================================================================

struct Point {
    int id{-1};
    int64_t x{};
    int64_t y{};

    Point() = default;
    constexpr Point(int pt_id, int64_t x_val, int64_t y_val) : id(pt_id), x(x_val), y(y_val) {}

    constexpr bool operator==(Point const& o) const {
        return id == o.id && x == o.x && y == o.y;
    }

    constexpr bool operator!=(Point const& o) const {
        return !(*this == o);
    }
};

inline __int128_t dist_sq_exact(Point const& a, Point const& b) {
    __int128_t dx = static_cast<__int128_t>(a.x) - static_cast<__int128_t>(b.x);
    __int128_t dy = static_cast<__int128_t>(a.y) - static_cast<__int128_t>(b.y);
    return dx * dx + dy * dy;
}

struct ClosestPairResult {
    Point p1;
    Point p2;
    __int128_t distance_squared{std::numeric_limits<int64_t>::max()};
    double distance{0.0};

    ClosestPairResult() = default;
    ClosestPairResult(Point a, Point b, __int128_t d_sq)
        : p1(a), p2(b), distance_squared(d_sq), distance(std::sqrt(static_cast<double>(d_sq))) {
        if (p2.id < p1.id) {
            std::swap(p1, p2);
        }
    }
};

// ============================================================================
// Layer 2: High-Level Closest Pair Engine
// ============================================================================

class ClosestPairEngine {
private:
    static ClosestPairResult closestPairRec(std::vector<Point>& pts_by_x,
                                           std::vector<Point>& pts_by_y,
                                           size_t left, size_t right) {
        size_t n = right - left;
        if (n <= 3) {
            ClosestPairResult best;
            best.distance_squared = static_cast<__int128_t>(~0ULL >> 1); // Large positive
            for (size_t i = left; i < right; ++i) {
                for (size_t j = i + 1; j < right; ++j) {
                    __int128_t d = dist_sq_exact(pts_by_x[i], pts_by_x[j]);
                    if (d < best.distance_squared) {
                        best = ClosestPairResult(pts_by_x[i], pts_by_x[j], d);
                    }
                }
            }
            // Sort this subrange by y for merge property
            std::sort(pts_by_x.begin() + left, pts_by_x.begin() + right,
                      [](Point const& a, Point const& b) { return a.y < b.y; });
            return best;
        }

        size_t mid = left + n / 2;
        int64_t mid_x = pts_by_x[mid].x;

        ClosestPairResult delta_l = closestPairRec(pts_by_x, pts_by_y, left, mid);
        ClosestPairResult delta_r = closestPairRec(pts_by_x, pts_by_y, mid, right);

        ClosestPairResult best = (delta_l.distance_squared < delta_r.distance_squared) ? delta_l : delta_r;
        __int128_t delta_sq = best.distance_squared;

        // Merge two sorted halves [left, mid) and [mid, right) by y in O(N)
        std::inplace_merge(pts_by_x.begin() + left, pts_by_x.begin() + mid, pts_by_x.begin() + right,
                           [](Point const& a, Point const& b) { return a.y < b.y; });

        // Build strip of points whose distance to mid_x is strictly less than delta
        pts_by_y.clear();
        for (size_t i = left; i < right; ++i) {
            __int128_t dx = static_cast<__int128_t>(pts_by_x[i].x) - static_cast<__int128_t>(mid_x);
            if (dx * dx < delta_sq) {
                pts_by_y.push_back(pts_by_x[i]);
            }
        }

        // Compare each point in strip to succeeding points (at most 7 points)
        size_t strip_size = pts_by_y.size();
        for (size_t i = 0; i < strip_size; ++i) {
            for (size_t j = i + 1; j < strip_size; ++j) {
                __int128_t dy = static_cast<__int128_t>(pts_by_y[j].y) - static_cast<__int128_t>(pts_by_y[i].y);
                if (dy * dy >= delta_sq) {
                    break; // Since pts_by_y is sorted by y, no future point can be closer
                }
                __int128_t d = dist_sq_exact(pts_by_y[i], pts_by_y[j]);
                if (d < best.distance_squared) {
                    best = ClosestPairResult(pts_by_y[i], pts_by_y[j], d);
                    delta_sq = best.distance_squared;
                }
            }
        }

        return best;
    }

public:
    /**
     * @brief Computes closest pair of points in O(N log N) using Divide and Conquer.
     */
    static ClosestPairResult findClosestPair(std::vector<Point> points) {
        if (points.size() < 2) {
            throw std::invalid_argument("Closest pair requires at least 2 points.");
        }

        // Pre-sort points primarily by x, secondarily by y
        std::sort(points.begin(), points.end(), [](Point const& a, Point const& b) {
            if (a.x != b.x) return a.x < b.x;
            return a.y < b.y;
        });

        std::vector<Point> scratch_y;
        scratch_y.reserve(points.size());
        return closestPairRec(points, scratch_y, 0, points.size());
    }

    /**
     * @brief Naive O(N^2) brute-force oracle for differential testing.
     */
    static ClosestPairResult naiveClosestPair(std::vector<Point> const& points) {
        if (points.size() < 2) {
            throw std::invalid_argument("Requires at least 2 points.");
        }
        ClosestPairResult best;
        best.distance_squared = static_cast<__int128_t>(~0ULL >> 1);
        size_t n = points.size();
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                __int128_t d = dist_sq_exact(points[i], points[j]);
                if (d < best.distance_squared) {
                    best = ClosestPairResult(points[i], points[j], d);
                }
            }
        }
        return best;
    }
};

} // namespace closest

// ============================================================================
// Verification & Differential Test Suite
// ============================================================================

void run_unit_tests() {
    using namespace closest;

    // Test 1: Simple 3-point triangle
    std::vector<Point> pts1 = {
        Point(0, 0, 0),
        Point(1, 10, 0),
        Point(2, 0, 1) // Closest pair: 0 and 2 (dist = 1)
    };
    auto res1 = ClosestPairEngine::findClosestPair(pts1);
    assert(res1.distance_squared == 1);
    assert((res1.p1.id == 0 && res1.p2.id == 2) || (res1.p1.id == 2 && res1.p2.id == 0));

    // Test 2: Identical duplicate points (dist = 0)
    std::vector<Point> pts2 = {
        Point(0, 5, 5),
        Point(1, 100, 100),
        Point(2, 5, 5)
    };
    auto res2 = ClosestPairEngine::findClosestPair(pts2);
    assert(res2.distance_squared == 0);
    assert(res2.distance == 0.0);

    // Test 3: Horizontal line points
    std::vector<Point> pts3 = {
        Point(0, 10, 5),
        Point(1, 25, 5),
        Point(2, 12, 5), // dist(0, 2) = 2
        Point(3, 40, 5)
    };
    auto res3 = ClosestPairEngine::findClosestPair(pts3);
    assert(res3.distance_squared == 4);
    assert(std::abs(res3.distance - 2.0) < 1e-9);
}

void run_differential_stress_tests() {
    using namespace closest;
    std::mt19937_64 rng(1337);
    std::uniform_int_distribution<int64_t> dist(-10000, 10000);

    for (int trial = 0; trial < 100; ++trial) {
        size_t n = 40;
        std::vector<Point> pts;
        pts.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            pts.emplace_back(static_cast<int>(i), dist(rng), dist(rng));
        }

        auto fast_res = ClosestPairEngine::findClosestPair(pts);
        auto naive_res = ClosestPairEngine::naiveClosestPair(pts);

        // Distance squared must match exactly
        assert(fast_res.distance_squared == naive_res.distance_squared);
        assert(std::abs(fast_res.distance - naive_res.distance) < 1e-9);
    }
}

int main() {
    std::cout << "[Verification] Running Closest Pair of Points test suite...\n";
    run_unit_tests();
    run_differential_stress_tests();
    std::cout << "[Verification] All 100 differential trials matched 100% with naive oracle!\n";
    return 0;
}
