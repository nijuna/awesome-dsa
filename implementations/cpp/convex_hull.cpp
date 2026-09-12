/**
 * @file convex_hull.cpp
 * @brief Convex Hull Algorithms: Andrew's Monotone Chain O(N log N) and
 *        Jarvis March (Gift Wrapping) O(N * H) Differential Oracle.
 *
 * Implements Arthur's Two-Layer API:
 *   - Layer 1: Low-level exact 128-bit orientation predicates and stack hull maintenance.
 *   - Layer 2: Safe, high-level ConvexHullEngine supporting strict/weak hulls,
 *              canonicalization, degenerate set handling, and differential verification.
 *
 * Compilation:
 *   g++ -std=c++17 -O3 -Wall -Wextra -Werror convex_hull.cpp -o convex_hull
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <random>

namespace chull {

// ============================================================================
// Layer 1: Exact Arithmetic Primitives
// ============================================================================

struct Point {
    int64_t x{};
    int64_t y{};

    Point() = default;
    constexpr Point(int64_t x_val, int64_t y_val) : x(x_val), y(y_val) {}

    constexpr bool operator==(Point const& o) const {
        return x == o.x && y == o.y;
    }

    constexpr bool operator!=(Point const& o) const {
        return !(*this == o);
    }

    constexpr bool operator<(Point const& o) const {
        if (x != o.x) return x < o.x;
        return y < o.y;
    }
};

inline __int128_t cross_product_exact(Point a, Point b, Point c) {
    __int128_t x1 = static_cast<__int128_t>(b.x) - static_cast<__int128_t>(a.x);
    __int128_t y1 = static_cast<__int128_t>(b.y) - static_cast<__int128_t>(a.y);
    __int128_t x2 = static_cast<__int128_t>(c.x) - static_cast<__int128_t>(a.x);
    __int128_t y2 = static_cast<__int128_t>(c.y) - static_cast<__int128_t>(a.y);
    return x1 * y2 - y1 * x2;
}

inline int orientation(Point a, Point b, Point c) {
    __int128_t cp = cross_product_exact(a, b, c);
    if (cp > 0) return 1;  // CCW (left turn)
    if (cp < 0) return -1; // CW (right turn)
    return 0;              // Collinear
}

inline int64_t dist_sq(Point a, Point b) {
    int64_t dx = a.x - b.x;
    int64_t dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// ============================================================================
// Layer 2: High-Level Convex Hull Engine
// ============================================================================

class ConvexHullEngine {
public:
    /**
     * @brief Computes Convex Hull via Andrew's Monotone Chain Algorithm in O(N log N).
     * @param points Input 2D point cloud.
     * @param include_collinear If true, retains collinear boundary points; otherwise strict vertices only.
     * @return Vertices of convex hull in counter-clockwise order.
     */
    static std::vector<Point> monotoneChain(std::vector<Point> points, bool include_collinear = false) {
        size_t n = points.size();
        if (n <= 1) return points;

        std::sort(points.begin(), points.end());
        points.erase(std::unique(points.begin(), points.end()), points.end());
        n = points.size();
        if (n <= 2) return points;

        std::vector<Point> hull;
        hull.reserve(2 * n);

        // Lower hull (left to right)
        for (size_t i = 0; i < n; ++i) {
            while (hull.size() >= 2) {
                int o = orientation(hull[hull.size() - 2], hull.back(), points[i]);
                if (include_collinear) {
                    if (o < 0) hull.pop_back();
                    else break;
                } else {
                    if (o <= 0) hull.pop_back();
                    else break;
                }
            }
            hull.push_back(points[i]);
        }

        // Upper hull (right to left)
        size_t lower_size = hull.size();
        for (int i = static_cast<int>(n) - 2; i >= 0; --i) {
            while (hull.size() > lower_size) {
                int o = orientation(hull[hull.size() - 2], hull.back(), points[i]);
                if (include_collinear) {
                    if (o < 0) hull.pop_back();
                    else break;
                } else {
                    if (o <= 0) hull.pop_back();
                    else break;
                }
            }
            hull.push_back(points[i]);
        }

        hull.pop_back(); // Remove duplicate of first point
        return hull;
    }

    /**
     * @brief Jarvis March (Gift Wrapping) Algorithm in O(N * H) time.
     * Generates counter-clockwise convex hull vertices.
     * Serves as an independent differential verification oracle.
     */
    static std::vector<Point> jarvisMarch(std::vector<Point> points) {
        size_t n = points.size();
        if (n <= 1) return points;

        std::sort(points.begin(), points.end());
        points.erase(std::unique(points.begin(), points.end()), points.end());
        n = points.size();
        if (n <= 2) return points;

        // Start at leftmost point (minimum x, then minimum y)
        size_t start_idx = 0;
        for (size_t i = 1; i < n; ++i) {
            if (points[i].x < points[start_idx].x ||
                (points[i].x == points[start_idx].x && points[i].y < points[start_idx].y)) {
                start_idx = i;
            }
        }

        std::vector<Point> hull;
        size_t current = start_idx;

        while (true) {
            hull.push_back(points[current]);
            size_t next_pt = (current + 1) % n;

            for (size_t i = 0; i < n; ++i) {
                if (i == current) continue;
                int o = orientation(points[current], points[next_pt], points[i]);
                if (o == -1) { // i is to the right of current -> next_pt (CCW wrap)
                    next_pt = i;
                } else if (o == 0) { // Collinear: pick farther point
                    if (dist_sq(points[current], points[i]) > dist_sq(points[current], points[next_pt])) {
                        next_pt = i;
                    }
                }
            }

            current = next_pt;
            if (current == start_idx) break;
        }

        return hull;
    }

    /**
     * @brief Normalizes a convex hull polygon representation:
     * Rotates vertices so that the lexicographically smallest point is at index 0.
     */
    static std::vector<Point> canonicalize(std::vector<Point> hull) {
        if (hull.empty()) return hull;
        auto min_it = std::min_element(hull.begin(), hull.end());
        std::rotate(hull.begin(), min_it, hull.end());
        return hull;
    }
};

} // namespace chull

// ============================================================================
// Verification Suite
// ============================================================================

void run_unit_tests() {
    using namespace chull;

    // Test 1: Empty and single point
    assert(ConvexHullEngine::monotoneChain({}).empty());
    assert(ConvexHullEngine::monotoneChain({{5, 5}}).size() == 1);

    // Test 2: Square with interior points
    std::vector<Point> pts = {
        {0, 0}, {4, 0}, {4, 4}, {0, 4},
        {1, 1}, {2, 2}, {3, 1}, {1, 3}
    };
    auto hull = ConvexHullEngine::monotoneChain(pts);
    assert(hull.size() == 4);
    auto canon_hull = ConvexHullEngine::canonicalize(hull);
    std::vector<Point> expected = {{0, 0}, {4, 0}, {4, 4}, {0, 4}};
    assert(canon_hull == expected);

    // Test 3: Collinear points on boundary
    std::vector<Point> coll = {
        {0, 0}, {2, 0}, {4, 0},
        {4, 2}, {4, 4},
        {0, 4}
    };
    auto strict_hull = ConvexHullEngine::monotoneChain(coll, false);
    assert(strict_hull.size() == 4); // Only (0,0), (4,0), (4,4), (0,4)
    auto weak_hull = ConvexHullEngine::monotoneChain(coll, true);
    assert(weak_hull.size() == 6);  // Includes (2,0) and (4,2)
}

void run_differential_stress_tests() {
    using namespace chull;
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int64_t> dist(-1000, 1000);

    for (int trial = 0; trial < 100; ++trial) {
        size_t n = 50;
        std::vector<Point> pts;
        pts.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            pts.emplace_back(dist(rng), dist(rng));
        }

        auto mc_hull = ConvexHullEngine::canonicalize(ConvexHullEngine::monotoneChain(pts, false));
        auto jm_hull = ConvexHullEngine::canonicalize(ConvexHullEngine::jarvisMarch(pts));

        assert(mc_hull.size() == jm_hull.size());
        assert(mc_hull == jm_hull);
    }
}

int main() {
    std::cout << "[Verification] Running Convex Hull test suite...\n";
    run_unit_tests();
    run_differential_stress_tests();
    std::cout << "[Verification] All 100 differential trials matched 100% with Jarvis March oracle!\n";
    return 0;
}
