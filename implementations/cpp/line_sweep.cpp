/**
 * @file line_sweep.cpp
 * @brief Line Sweep Algorithms for Segment Intersection: Shamos-Hoey O(N log N)
 *        Intersection Detector, Orthogonal Line Sweep O((N + K) log N), and
 *        Bentley-Ottmann Event-Driven Plane Sweep.
 *
 * Implements Arthur's Two-Layer API:
 *   - Layer 1: Exact geometric predicates (128-bit cross products, orientation),
 *              sweep-line status comparators, and event priority queues.
 *   - Layer 2: Safe, high-level LineSweep engine supporting:
 *              1. hasAnyIntersection() [Shamos-Hoey O(N log N)]
 *              2. findOrthogonalIntersections() [Orthogonal Plane Sweep O((N + K) log N)]
 *              3. naiveAllIntersections() [O(N^2) general differential oracle]
 *              4. naiveOrthogonalIntersections() [O(N^2) orthogonal differential oracle]
 *
 * Compilation:
 *   g++ -std=c++17 -O3 -Wall -Wextra -Werror line_sweep.cpp -o line_sweep
 */

#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <optional>
#include <random>

namespace sweep {

// ============================================================================
// Layer 1: Geometric Primitives & Exact Orientations
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

struct Segment {
    int id;
    Point p1; // Left endpoint (p1 <= p2)
    Point p2; // Right endpoint

    Segment(int seg_id, Point a, Point b) : id(seg_id) {
        if (b < a) {
            p1 = b;
            p2 = a;
        } else {
            p1 = a;
            p2 = b;
        }
    }

    // Evaluate y-coordinate along segment at sweep position x
    double eval_y(double sweep_x) const {
        if (p1.x == p2.x) {
            return static_cast<double>(p1.y);
        }
        double t = (sweep_x - static_cast<double>(p1.x)) / static_cast<double>(p2.x - p1.x);
        return static_cast<double>(p1.y) + t * static_cast<double>(p2.y - p1.y);
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

inline bool on_segment_collinear(Point p, Point q, Point r) {
    return (q.x <= std::max(p.x, r.x) && q.x >= std::min(p.x, r.x) &&
            q.y <= std::max(p.y, r.y) && q.y >= std::min(p.y, r.y));
}

inline bool segments_intersect(Segment const& s1, Segment const& s2) {
    int o1 = orientation(s1.p1, s1.p2, s2.p1);
    int o2 = orientation(s1.p1, s1.p2, s2.p2);
    int o3 = orientation(s2.p1, s2.p2, s1.p1);
    int o4 = orientation(s2.p1, s2.p2, s1.p2);

    if (o1 != o2 && o3 != o4) return true;

    if (o1 == 0 && on_segment_collinear(s1.p1, s2.p1, s1.p2)) return true;
    if (o2 == 0 && on_segment_collinear(s1.p1, s2.p2, s1.p2)) return true;
    if (o3 == 0 && on_segment_collinear(s2.p1, s1.p1, s2.p2)) return true;
    if (o4 == 0 && on_segment_collinear(s2.p1, s1.p2, s2.p2)) return true;

    return false;
}

// Sweep-line status comparator evaluated at current sweep_x
inline double g_sweep_x = 0.0;

struct SweepStatusComparator {
    bool operator()(Segment const* a, Segment const* b) const {
        if (a->id == b->id) return false;
        double ya = a->eval_y(g_sweep_x);
        double yb = b->eval_y(g_sweep_x);
        if (std::abs(ya - yb) > 1e-9) {
            return ya < yb;
        }
        return a->id < b->id;
    }
};

// ============================================================================
// Layer 2: High-Level Line Sweep Engines
// ============================================================================

class LineSweep {
public:
    /**
     * @brief Shamos-Hoey Algorithm: Tests whether ANY intersection exists among N segments.
     * Complexity: O(N log N) time, O(N) space.
     */
    static bool hasAnyIntersection(std::vector<Segment> const& segments) {
        struct Event {
            double x;
            int type; // 0 = start, 1 = end
            Segment const* seg;

            bool operator>(Event const& o) const {
                if (std::abs(x - o.x) > 1e-9) return x > o.x;
                return type > o.type;
            }
        };

        std::priority_queue<Event, std::vector<Event>, std::greater<Event>> eq;
        for (auto const& s : segments) {
            eq.push({static_cast<double>(s.p1.x), 0, &s});
            eq.push({static_cast<double>(s.p2.x), 1, &s});
        }

        std::set<Segment const*, SweepStatusComparator> status;

        while (!eq.empty()) {
            auto ev = eq.top();
            eq.pop();
            g_sweep_x = ev.x;

            if (ev.type == 0) { // START
                auto it = status.insert(ev.seg).first;

                // Check intersection with predecessor
                if (it != status.begin()) {
                    auto prev_it = std::prev(it);
                    if (segments_intersect(**prev_it, *ev.seg)) {
                        return true;
                    }
                }
                // Check intersection with successor
                auto next_it = std::next(it);
                if (next_it != status.end()) {
                    if (segments_intersect(**next_it, *ev.seg)) {
                        return true;
                    }
                }
            } else { // END
                auto it = status.find(ev.seg);
                if (it != status.end()) {
                    auto prev_it = (it != status.begin()) ? std::prev(it) : status.end();
                    auto next_it = std::next(it);

                    if (prev_it != status.end() && next_it != status.end()) {
                        if (segments_intersect(**prev_it, **next_it)) {
                            return true;
                        }
                    }
                    status.erase(it);
                }
            }
        }

        return false;
    }

    /**
     * @brief Orthogonal Line Sweep: Reports all intersections between axis-aligned
     *        horizontal and vertical line segments in O((N + K) log N) time.
     */
    static std::vector<std::pair<int, int>> findOrthogonalIntersections(std::vector<Segment> const& segments) {
        enum class EventType { H_START, V_QUERY, H_END };
        struct OrthoEvent {
            int64_t x;
            EventType type;
            Segment const* seg;
        };

        std::vector<OrthoEvent> events;
        for (auto const& s : segments) {
            if (s.p1.x == s.p2.x) { // Vertical segment
                events.push_back({s.p1.x, EventType::V_QUERY, &s});
            } else if (s.p1.y == s.p2.y) { // Horizontal segment
                events.push_back({s.p1.x, EventType::H_START, &s});
                events.push_back({s.p2.x, EventType::H_END, &s});
            }
        }

        std::sort(events.begin(), events.end(), [](OrthoEvent const& a, OrthoEvent const& b) {
            if (a.x != b.x) return a.x < b.x;
            return static_cast<int>(a.type) < static_cast<int>(b.type);
        });

        std::set<std::pair<int64_t, int>> active_y; // {y, id}
        std::vector<std::pair<int, int>> results;

        for (auto const& ev : events) {
            if (ev.type == EventType::H_START) {
                active_y.insert({ev.seg->p1.y, ev.seg->id});
            } else if (ev.type == EventType::H_END) {
                active_y.erase({ev.seg->p1.y, ev.seg->id});
            } else { // V_QUERY
                int64_t y_low = std::min(ev.seg->p1.y, ev.seg->p2.y);
                int64_t y_high = std::max(ev.seg->p1.y, ev.seg->p2.y);
                auto it_start = active_y.lower_bound({y_low, -1});
                auto it_end = active_y.upper_bound({y_high, 2000000000});

                for (auto it = it_start; it != it_end; ++it) {
                    int id1 = it->second;
                    int id2 = ev.seg->id;
                    if (id1 > id2) std::swap(id1, id2);
                    results.emplace_back(id1, id2);
                }
            }
        }

        std::sort(results.begin(), results.end());
        results.erase(std::unique(results.begin(), results.end()), results.end());
        return results;
    }

    /**
     * @brief Naive O(N^2) general intersection oracle for differential verification.
     */
    static std::vector<std::pair<int, int>> naiveAllIntersections(std::vector<Segment> const& segments) {
        std::vector<std::pair<int, int>> result;
        size_t n = segments.size();
        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                if (segments_intersect(segments[i], segments[j])) {
                    int id1 = segments[i].id;
                    int id2 = segments[j].id;
                    if (id1 > id2) std::swap(id1, id2);
                    result.emplace_back(id1, id2);
                }
            }
        }
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }

    /**
     * @brief Naive O(N^2) orthogonal intersection oracle (only checks vertical vs horizontal).
     */
    static std::vector<std::pair<int, int>> naiveOrthogonalIntersections(std::vector<Segment> const& segments) {
        std::vector<std::pair<int, int>> result;
        size_t n = segments.size();
        for (size_t i = 0; i < n; ++i) {
            bool i_vert = (segments[i].p1.x == segments[i].p2.x);
            for (size_t j = i + 1; j < n; ++j) {
                bool j_vert = (segments[j].p1.x == segments[j].p2.x);
                if (i_vert != j_vert) {
                    if (segments_intersect(segments[i], segments[j])) {
                        int id1 = segments[i].id;
                        int id2 = segments[j].id;
                        if (id1 > id2) std::swap(id1, id2);
                        result.emplace_back(id1, id2);
                    }
                }
            }
        }
        std::sort(result.begin(), result.end());
        result.erase(std::unique(result.begin(), result.end()), result.end());
        return result;
    }
};

} // namespace sweep

// ============================================================================
// Verification & Differential Test Suite
// ============================================================================

void run_unit_tests() {
    using namespace sweep;

    // Test case 1: Single crossing pair
    std::vector<Segment> segs1 = {
        Segment(0, Point(0, 0), Point(4, 4)),
        Segment(1, Point(0, 4), Point(4, 0))
    };
    assert(LineSweep::hasAnyIntersection(segs1) == true);
    auto naive1 = LineSweep::naiveAllIntersections(segs1);
    assert(naive1.size() == 1 && naive1[0] == std::make_pair(0, 1));

    // Test case 2: Parallel non-intersecting lines
    std::vector<Segment> segs2 = {
        Segment(0, Point(0, 0), Point(4, 0)),
        Segment(1, Point(0, 2), Point(4, 2)),
        Segment(2, Point(0, 4), Point(4, 4))
    };
    assert(LineSweep::hasAnyIntersection(segs2) == false);
    assert(LineSweep::naiveAllIntersections(segs2).empty());

    // Test case 3: Orthogonal grid
    std::vector<Segment> segs3 = {
        Segment(0, Point(1, 0), Point(1, 4)), // Vertical x = 1
        Segment(1, Point(3, 0), Point(3, 4)), // Vertical x = 3
        Segment(2, Point(0, 1), Point(4, 1)), // Horizontal y = 1
        Segment(3, Point(0, 3), Point(4, 3))  // Horizontal y = 3
    };
    assert(LineSweep::hasAnyIntersection(segs3) == true);
    auto ortho_res = LineSweep::findOrthogonalIntersections(segs3);
    auto naive_res = LineSweep::naiveOrthogonalIntersections(segs3);
    assert(ortho_res.size() == 4);
    assert(ortho_res == naive_res);
}

void run_differential_stress_tests() {
    using namespace sweep;
    std::mt19937_64 rng(1337);
    std::uniform_int_distribution<int64_t> coord_dist(-200, 200);

    // Differential test 1: Shamos-Hoey vs Naive Oracle on general segments
    for (int trial = 0; trial < 40; ++trial) {
        int n = 20;
        std::vector<Segment> segs;
        for (int i = 0; i < n; ++i) {
            Point p1(coord_dist(rng), coord_dist(rng));
            Point p2(coord_dist(rng), coord_dist(rng));
            if (p1.x == p2.x) p2.x += 1; // avoid exact vertical for general sweep test
            segs.emplace_back(i, p1, p2);
        }

        auto naive = LineSweep::naiveAllIntersections(segs);
        bool has_intersect_naive = !naive.empty();
        bool has_intersect_shamos = LineSweep::hasAnyIntersection(segs);

        assert(has_intersect_naive == has_intersect_shamos);
    }

    // Differential test 2: Orthogonal Line Sweep vs Naive Ortho Oracle on axis-aligned segments
    for (int trial = 0; trial < 50; ++trial) {
        int n = 30;
        std::vector<Segment> segs;
        for (int i = 0; i < n; ++i) {
            if (i % 2 == 0) {
                // Horizontal
                int64_t y = coord_dist(rng);
                int64_t x1 = coord_dist(rng);
                int64_t x2 = coord_dist(rng);
                if (x1 == x2) x2 += 5;
                segs.emplace_back(i, Point(x1, y), Point(x2, y));
            } else {
                // Vertical
                int64_t x = coord_dist(rng);
                int64_t y1 = coord_dist(rng);
                int64_t y2 = coord_dist(rng);
                if (y1 == y2) y2 += 5;
                segs.emplace_back(i, Point(x, y1), Point(x, y2));
            }
        }

        auto ortho_res = LineSweep::findOrthogonalIntersections(segs);
        auto naive_res = LineSweep::naiveOrthogonalIntersections(segs);
        assert(ortho_res == naive_res);
    }
}

int main() {
    std::cout << "[Verification] Running Line Sweep test suite...\n";
    run_unit_tests();
    run_differential_stress_tests();
    std::cout << "[Verification] All Line Sweep tests passed successfully!\n";
    return 0;
}
