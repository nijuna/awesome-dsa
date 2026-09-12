/**
 * @file computational_geometry_basics.cpp
 * @brief Computational Geometry Fundamentals: 2D Points, Vectors, Exact Orientation,
 *        Segment Intersection, Polygon Area, and Point-in-Polygon Tests.
 *
 * Designed with Arthur's Two-Layer API:
 *   - Layer 1: Low-level exact geometric primitives (Point2D, exact cross/dot products, CCW predicate).
 *   - Layer 2: Safe, validated high-level GeometryEngine (segment intersection, ray-casting point-in-polygon,
 *              bounding boxes, polygon area).
 *
 * Implements exact integer arithmetic via __int128_t to avoid floating-point drift and overflow.
 *
 * Compilation:
 *   g++ -std=c++17 -O3 -Wall -Wextra -Werror computational_geometry_basics.cpp -o geom_basics
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <optional>
#include <algorithm>
#include <string>
#include <sstream>

namespace geom {

// ============================================================================
// Layer 1: Low-Level Primitives & Exact Predicates
// ============================================================================

template <typename T>
struct Point2D {
    T x{};
    T y{};

    constexpr Point2D() = default;
    constexpr Point2D(T x_val, T y_val) : x(x_val), y(y_val) {}

    constexpr Point2D operator+(Point2D const& other) const {
        return Point2D(x + other.x, y + other.y);
    }

    constexpr Point2D operator-(Point2D const& other) const {
        return Point2D(x - other.x, y - other.y);
    }

    constexpr Point2D operator*(T scalar) const {
        return Point2D(x * scalar, y * scalar);
    }

    constexpr bool operator==(Point2D const& other) const {
        return x == other.x && y == other.y;
    }

    constexpr bool operator!=(Point2D const& other) const {
        return !(*this == other);
    }

    constexpr bool operator<(Point2D const& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
};

using Point64 = Point2D<int64_t>;
using PointF = Point2D<double>;

enum class Orientation {
    COLLINEAR = 0,
    COUNTER_CLOCKWISE = 1, // Left turn
    CLOCKWISE = -1         // Right turn
};

enum class PointLocation {
    OUTSIDE = 0,
    ON_BOUNDARY = 1,
    INSIDE = 2
};

struct Segment64 {
    Point64 p1;
    Point64 q1;
};

// Exact 128-bit cross product of vectors (b - a) and (c - a)
inline __int128_t cross_product_exact(Point64 a, Point64 b, Point64 c) {
    __int128_t x1 = static_cast<__int128_t>(b.x) - static_cast<__int128_t>(a.x);
    __int128_t y1 = static_cast<__int128_t>(b.y) - static_cast<__int128_t>(a.y);
    __int128_t x2 = static_cast<__int128_t>(c.x) - static_cast<__int128_t>(a.x);
    __int128_t y2 = static_cast<__int128_t>(c.y) - static_cast<__int128_t>(a.y);
    return x1 * y2 - y1 * x2;
}

// Exact 128-bit dot product of vectors (b - a) and (c - a)
inline __int128_t dot_product_exact(Point64 a, Point64 b, Point64 c) {
    __int128_t x1 = static_cast<__int128_t>(b.x) - static_cast<__int128_t>(a.x);
    __int128_t y1 = static_cast<__int128_t>(b.y) - static_cast<__int128_t>(a.y);
    __int128_t x2 = static_cast<__int128_t>(c.x) - static_cast<__int128_t>(a.x);
    __int128_t y2 = static_cast<__int128_t>(c.y) - static_cast<__int128_t>(a.y);
    return x1 * x2 + y1 * y2;
}

// Orientation predicate: returns CCW, CW, or COLLINEAR
inline Orientation orientation(Point64 a, Point64 b, Point64 c) {
    __int128_t cp = cross_product_exact(a, b, c);
    if (cp > 0) return Orientation::COUNTER_CLOCKWISE;
    if (cp < 0) return Orientation::CLOCKWISE;
    return Orientation::COLLINEAR;
}

// Checks if point q lies on segment pr (assuming p, q, r are collinear)
inline bool on_segment_collinear(Point64 p, Point64 q, Point64 r) {
    return (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
            q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y));
}

// Tests if two line segments p1q1 and p2q2 intersect
inline bool segments_intersect(Point64 p1, Point64 q1, Point64 p2, Point64 q2) {
    Orientation o1 = orientation(p1, q1, p2);
    Orientation o2 = orientation(p1, q1, q2);
    Orientation o3 = orientation(p2, q2, p1);
    Orientation o4 = orientation(p2, q2, q1);

    // General case: segments straddle each other
    if (o1 != o2 && o3 != o4) {
        return true;
    }

    // Special collinear cases
    if (o1 == Orientation::COLLINEAR && on_segment_collinear(p1, p2, q1)) return true;
    if (o2 == Orientation::COLLINEAR && on_segment_collinear(p1, q2, q1)) return true;
    if (o3 == Orientation::COLLINEAR && on_segment_collinear(p2, p1, q2)) return true;
    if (o4 == Orientation::COLLINEAR && on_segment_collinear(p2, q1, q2)) return true;

    return false;
}

// Doubled signed area of polygon: Shoelace formula
inline __int128_t polygon_area_2x_exact(std::vector<Point64> const& vertices) {
    size_t n = vertices.size();
    if (n < 3) return 0;
    __int128_t area2 = 0;
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        __int128_t xi = vertices[i].x;
        __int128_t yi = vertices[i].y;
        __int128_t xj = vertices[j].x;
        __int128_t yj = vertices[j].y;
        area2 += (xi * yj - xj * yi);
    }
    return area2;
}

// Point-in-polygon test via Ray-Casting with exact boundary semantics
inline PointLocation point_in_polygon_exact(Point64 pt, std::vector<Point64> const& poly) {
    size_t n = poly.size();
    if (n < 3) return PointLocation::OUTSIDE;

    // Check if point lies on any polygon edge
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        if (orientation(poly[i], poly[j], pt) == Orientation::COLLINEAR &&
            on_segment_collinear(poly[i], pt, poly[j])) {
            return PointLocation::ON_BOUNDARY;
        }
    }

    // Ray casting to +x direction (horizontal ray pt.y)
    bool inside = false;
    for (size_t i = 0; i < n; ++i) {
        size_t j = (i + 1) % n;
        Point64 p = poly[i];
        Point64 q = poly[j];

        // Ensure p is lower or equal in y
        if (p.y > q.y) std::swap(p, q);

        // Ray condition: p.y <= pt.y < q.y (half-open vertical interval)
        if (p.y <= pt.y && pt.y < q.y) {
            // Orientation of (p -> q) relative to pt
            // If pt is strictly to the left of directed edge p -> q, ray crosses edge
            Orientation o = orientation(p, q, pt);
            if (o == Orientation::COUNTER_CLOCKWISE) {
                inside = !inside;
            }
        }
    }

    return inside ? PointLocation::INSIDE : PointLocation::OUTSIDE;
}

// ============================================================================
// Layer 2: High-Level Geometric Engine
// ============================================================================

class GeometryEngine {
public:
    static bool doSegmentsIntersect(Point64 p1, Point64 q1, Point64 p2, Point64 q2) {
        // Fast rejection test: Bounding boxes must overlap
        int64_t min_x1 = std::min(p1.x, q1.x), max_x1 = std::max(p1.x, q1.x);
        int64_t min_y1 = std::min(p1.y, q1.y), max_y1 = std::max(p1.y, q1.y);
        int64_t min_x2 = std::min(p2.x, q2.x), max_x2 = std::max(p2.x, q2.x);
        int64_t min_y2 = std::min(p2.y, q2.y), max_y2 = std::max(p2.y, q2.y);

        if (max_x1 < min_x2 || max_x2 < min_x1 || max_y1 < min_y2 || max_y2 < min_y1) {
            return false;
        }

        return segments_intersect(p1, q1, p2, q2);
    }

    static double computePolygonArea(std::vector<Point64> const& polygon) {
        __int128_t area2 = polygon_area_2x_exact(polygon);
        if (area2 < 0) area2 = -area2;
        return static_cast<double>(area2) / 2.0;
    }

    static PointLocation testPointInPolygon(Point64 pt, std::vector<Point64> const& polygon) {
        return point_in_polygon_exact(pt, polygon);
    }

    static double pointToSegmentDistance(PointF p, PointF a, PointF b) {
        double ab_x = b.x - a.x;
        double ab_y = b.y - a.y;
        double ap_x = p.x - a.x;
        double ap_y = p.y - a.y;

        double ab_len_sq = ab_x * ab_x + ab_y * ab_y;
        if (ab_len_sq < 1e-12) {
            return std::hypot(ap_x, ap_y);
        }

        // Projection factor t = (ap . ab) / |ab|^2 clamped to [0, 1]
        double t = (ap_x * ab_x + ap_y * ab_y) / ab_len_sq;
        t = std::max(0.0, std::min(1.0, t));

        double closest_x = a.x + t * ab_x;
        double closest_y = a.y + t * ab_y;
        return std::hypot(p.x - closest_x, p.y - closest_y);
    }

    static std::optional<PointF> lineSegmentIntersectionPoint(PointF p1, PointF q1, PointF p2, PointF q2) {
        double d1_x = q1.x - p1.x;
        double d1_y = q1.y - p1.y;
        double d2_x = q2.x - p2.x;
        double d2_y = q2.y - p2.y;

        double denom = d1_x * d2_y - d1_y * d2_x;
        if (std::abs(denom) < 1e-12) {
            return std::nullopt; // Parallel or collinear
        }

        double dp_x = p2.x - p1.x;
        double dp_y = p2.y - p1.y;

        double t = (dp_x * d2_y - dp_y * d2_x) / denom;
        double u = (dp_x * d1_y - dp_y * d1_x) / denom;

        if (t >= 0.0 && t <= 1.0 && u >= 0.0 && u <= 1.0) {
            return PointF(p1.x + t * d1_x, p1.y + t * d1_y);
        }
        return std::nullopt;
    }
};

} // namespace geom

// ============================================================================
// Verification & Differential Test Suite
// ============================================================================

void run_orientation_tests() {
    using namespace geom;
    Point64 a(0, 0), b(4, 0), c(2, 2);
    assert(orientation(a, b, c) == Orientation::COUNTER_CLOCKWISE);
    assert(orientation(b, a, c) == Orientation::CLOCKWISE);
    assert(orientation(a, b, Point64(8, 0)) == Orientation::COLLINEAR);

    // Coordinate magnitude stress test (no overflow with 64-bit coordinates)
    Point64 p1(1'000'000'000LL, 1'000'000'000LL);
    Point64 p2(2'000'000'000LL, 2'000'000'000LL);
    Point64 p3(3'000'000'000LL, 3'000'000'000LL + 1);
    assert(orientation(p1, p2, p3) == Orientation::COUNTER_CLOCKWISE);
}

void run_segment_intersection_tests() {
    using namespace geom;
    // Proper crossing
    Point64 p1(0, 0), q1(4, 4);
    Point64 p2(0, 4), q2(4, 0);
    assert(GeometryEngine::doSegmentsIntersect(p1, q1, p2, q2) == true);

    // Parallel disjoint
    Point64 p3(0, 1), q3(4, 5);
    assert(GeometryEngine::doSegmentsIntersect(p1, q1, p3, q3) == false);

    // Collinear overlapping
    Point64 p4(2, 2), q4(6, 6);
    assert(GeometryEngine::doSegmentsIntersect(p1, q1, p4, q4) == true);

    // Collinear disjoint
    Point64 p5(5, 5), q5(8, 8);
    assert(GeometryEngine::doSegmentsIntersect(p1, q1, p5, q5) == false);

    // T-junction (endpoint touching interior of segment)
    Point64 p6(2, 0), q6(2, 2);
    Point64 seg_a(0, 2), seg_b(4, 2);
    assert(GeometryEngine::doSegmentsIntersect(p6, q6, seg_a, seg_b) == true);
}

void run_polygon_area_tests() {
    using namespace geom;
    // Standard 4x4 square -> Area = 16.0
    std::vector<Point64> square = {{0, 0}, {4, 0}, {4, 4}, {0, 4}};
    assert(std::abs(GeometryEngine::computePolygonArea(square) - 16.0) < 1e-9);

    // Triangle: (0,0), (5,0), (0,6) -> Area = 15.0
    std::vector<Point64> triangle = {{0, 0}, {5, 0}, {0, 6}};
    assert(std::abs(GeometryEngine::computePolygonArea(triangle) - 15.0) < 1e-9);
}

void run_point_in_polygon_tests() {
    using namespace geom;
    // Non-convex L-shaped polygon
    // (0,0) -> (4,0) -> (4,2) -> (2,2) -> (2,4) -> (0,4)
    std::vector<Point64> l_poly = {
        {0, 0}, {4, 0}, {4, 2}, {2, 2}, {2, 4}, {0, 4}
    };

    // Strictly inside
    assert(GeometryEngine::testPointInPolygon({1, 1}, l_poly) == PointLocation::INSIDE);
    assert(GeometryEngine::testPointInPolygon({1, 3}, l_poly) == PointLocation::INSIDE);

    // Cutout area (outside the reflex notch)
    assert(GeometryEngine::testPointInPolygon({3, 3}, l_poly) == PointLocation::OUTSIDE);

    // On vertices and boundary edges
    assert(GeometryEngine::testPointInPolygon({2, 2}, l_poly) == PointLocation::ON_BOUNDARY);
    assert(GeometryEngine::testPointInPolygon({0, 2}, l_poly) == PointLocation::ON_BOUNDARY);
    assert(GeometryEngine::testPointInPolygon({3, 0}, l_poly) == PointLocation::ON_BOUNDARY);
}

void run_floating_point_distance_tests() {
    using namespace geom;
    PointF a(0, 0), b(10, 0);
    PointF p(5, 5);
    assert(std::abs(GeometryEngine::pointToSegmentDistance(p, a, b) - 5.0) < 1e-9);

    PointF p_outside(15, 0);
    assert(std::abs(GeometryEngine::pointToSegmentDistance(p_outside, a, b) - 5.0) < 1e-9);

    // Intersection point
    auto pt = GeometryEngine::lineSegmentIntersectionPoint({0, 0}, {4, 4}, {0, 4}, {4, 0});
    assert(pt.has_value());
    assert(std::abs(pt->x - 2.0) < 1e-9 && std::abs(pt->y - 2.0) < 1e-9);
}

int main() {
    std::cout << "[Verification] Running Computational Geometry Basics test suite...\n";
    run_orientation_tests();
    run_segment_intersection_tests();
    run_polygon_area_tests();
    run_point_in_polygon_tests();
    run_floating_point_distance_tests();
    std::cout << "[Verification] All 5 test suites passed successfully!\n";
    return 0;
}
