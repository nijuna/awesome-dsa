/**
 * @file kd_trees.cpp
 * @brief 2D / k-Dimensional Space Partitioning Tree (KD-Tree) for Range Search
 *        and K-Nearest Neighbor (k-NN) Queries.
 *
 * Implements Arthur's Two-Layer API:
 *   - Layer 1: Median-splitting spatial partitioning nodes, hyperplane distance pruning,
 *              bounding-box intersection checks.
 *   - Layer 2: High-level KDTreeEngine supporting balanced tree construction O(N log N),
 *              orthogonal range search O(sqrt(N) + K), and k-NN queries with differential testing.
 *
 * Compilation:
 *   g++ -std=c++17 -O3 -Wall -Wextra -Werror kd_trees.cpp -o kd_trees
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <queue>
#include <limits>
#include <memory>
#include <random>

namespace kd {

// ============================================================================
// Layer 1: Geometric Primitives & KD-Node
// ============================================================================

struct Point2D {
    int id{-1};
    int64_t x{};
    int64_t y{};

    Point2D() = default;
    constexpr Point2D(int pt_id, int64_t x_val, int64_t y_val) : id(pt_id), x(x_val), y(y_val) {}

    int64_t get(int axis) const {
        return (axis == 0) ? x : y;
    }

    constexpr bool operator==(Point2D const& o) const {
        return id == o.id && x == o.x && y == o.y;
    }

    constexpr bool operator!=(Point2D const& o) const {
        return !(*this == o);
    }
};

inline __int128_t dist_sq_exact(Point2D const& a, Point2D const& b) {
    __int128_t dx = static_cast<__int128_t>(a.x) - static_cast<__int128_t>(b.x);
    __int128_t dy = static_cast<__int128_t>(a.y) - static_cast<__int128_t>(b.y);
    return dx * dx + dy * dy;
}

struct BoundingBox2D {
    int64_t min_x{std::numeric_limits<int64_t>::min()};
    int64_t max_x{std::numeric_limits<int64_t>::max()};
    int64_t min_y{std::numeric_limits<int64_t>::min()};
    int64_t max_y{std::numeric_limits<int64_t>::max()};

    bool contains(Point2D const& p) const {
        return p.x >= min_x && p.x <= max_x && p.y >= min_y && p.y <= max_y;
    }

    bool intersects(BoundingBox2D const& o) const {
        return !(max_x < o.min_x || min_x > o.max_x || max_y < o.min_y || min_y > o.max_y);
    }
};

struct KDNode {
    Point2D point;
    int axis{0}; // 0 = split along x, 1 = split along y
    std::unique_ptr<KDNode> left;
    std::unique_ptr<KDNode> right;

    explicit KDNode(Point2D pt, int ax) : point(pt), axis(ax), left(nullptr), right(nullptr) {}
};

// ============================================================================
// Layer 2: High-Level KD-Tree Engine
// ============================================================================

class KDTree {
private:
    std::unique_ptr<KDNode> root_{nullptr};
    size_t size_{0};

    static std::unique_ptr<KDNode> buildRec(std::vector<Point2D>& pts, size_t left, size_t right, int depth) {
        if (left >= right) return nullptr;

        int axis = depth % 2;
        size_t mid = left + (right - left) / 2;

        // Partition using median element in O(N)
        std::nth_element(pts.begin() + left, pts.begin() + mid, pts.begin() + right,
                         [axis](Point2D const& a, Point2D const& b) {
                             if (a.get(axis) != b.get(axis)) {
                                 return a.get(axis) < b.get(axis);
                             }
                             return a.id < b.id;
                         });

        auto node = std::make_unique<KDNode>(pts[mid], axis);
        node->left = buildRec(pts, left, mid, depth + 1);
        node->right = buildRec(pts, mid + 1, right, depth + 1);
        return node;
    }

    static void rangeSearchRec(KDNode const* node, BoundingBox2D const& query_box,
                               BoundingBox2D current_box, std::vector<Point2D>& results) {
        if (!node) return;
        if (!current_box.intersects(query_box)) return;

        if (query_box.contains(node->point)) {
            results.push_back(node->point);
        }

        // Subdivide current bounding box
        BoundingBox2D left_box = current_box;
        BoundingBox2D right_box = current_box;
        if (node->axis == 0) {
            left_box.max_x = node->point.x;
            right_box.min_x = node->point.x;
        } else {
            left_box.max_y = node->point.y;
            right_box.min_y = node->point.y;
        }

        rangeSearchRec(node->left.get(), query_box, left_box, results);
        rangeSearchRec(node->right.get(), query_box, right_box, results);
    }

    struct NNEntry {
        __int128_t dist_sq;
        Point2D point;

        bool operator<(NNEntry const& o) const {
            return dist_sq < o.dist_sq; // Max-heap: largest distance on top
        }
    };

    static void knnRec(KDNode const* node, Point2D const& target, size_t k,
                       std::priority_queue<NNEntry>& pq) {
        if (!node) return;

        __int128_t d_sq = dist_sq_exact(node->point, target);
        if (pq.size() < k) {
            pq.push({d_sq, node->point});
        } else if (d_sq < pq.top().dist_sq) {
            pq.pop();
            pq.push({d_sq, node->point});
        }

        int axis = node->axis;
        int64_t diff = target.get(axis) - node->point.get(axis);
        __int128_t axis_dist_sq = static_cast<__int128_t>(diff) * static_cast<__int128_t>(diff);

        KDNode const* first = (diff <= 0) ? node->left.get() : node->right.get();
        KDNode const* second = (diff <= 0) ? node->right.get() : node->left.get();

        // Always explore closer subtree first
        knnRec(first, target, k, pq);

        // Pruning check: only explore second subtree if distance to plane is less than worst in heap
        if (pq.size() < k || axis_dist_sq < pq.top().dist_sq) {
            knnRec(second, target, k, pq);
        }
    }

public:
    KDTree() = default;

    explicit KDTree(std::vector<Point2D> points) {
        build(std::move(points));
    }

    void build(std::vector<Point2D> points) {
        size_ = points.size();
        root_ = buildRec(points, 0, points.size(), 0);
    }

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    /**
     * @brief Orthogonal Range Search: finds all points inside query bounding box.
     */
    std::vector<Point2D> rangeSearch(BoundingBox2D const& query_box) const {
        std::vector<Point2D> results;
        BoundingBox2D universe;
        rangeSearchRec(root_.get(), query_box, universe, results);
        std::sort(results.begin(), results.end(), [](Point2D const& a, Point2D const& b) {
            return a.id < b.id;
        });
        return results;
    }

    /**
     * @brief K-Nearest Neighbor (k-NN) search.
     * @return List of k closest points ordered from nearest to farthest.
     */
    std::vector<Point2D> kNearestNeighbors(Point2D const& target, size_t k) const {
        if (size_ == 0 || k == 0) return {};
        k = std::min(k, size_);

        std::priority_queue<NNEntry> pq;
        knnRec(root_.get(), target, k, pq);

        std::vector<Point2D> res(pq.size());
        for (int i = static_cast<int>(pq.size()) - 1; i >= 0; --i) {
            res[i] = pq.top().point;
            pq.pop();
        }
        return res;
    }

    /**
     * @brief 1-Nearest Neighbor search.
     */
    Point2D nearestNeighbor(Point2D const& target) const {
        auto res = kNearestNeighbors(target, 1);
        if (res.empty()) throw std::runtime_error("KDTree is empty");
        return res[0];
    }
};

// ============================================================================
// Differential Testing Oracles
// ============================================================================

struct KDOracles {
    static std::vector<Point2D> naiveRangeSearch(std::vector<Point2D> const& pts, BoundingBox2D const& box) {
        std::vector<Point2D> res;
        for (auto const& p : pts) {
            if (box.contains(p)) res.push_back(p);
        }
        std::sort(res.begin(), res.end(), [](Point2D const& a, Point2D const& b) {
            return a.id < b.id;
        });
        return res;
    }

    static std::vector<Point2D> naiveKNN(std::vector<Point2D> const& pts, Point2D const& target, size_t k) {
        if (pts.empty() || k == 0) return {};
        k = std::min(k, pts.size());

        std::vector<std::pair<__int128_t, Point2D>> ranked;
        ranked.reserve(pts.size());
        for (auto const& p : pts) {
            ranked.emplace_back(dist_sq_exact(p, target), p);
        }

        std::sort(ranked.begin(), ranked.end(), [](auto const& a, auto const& b) {
            if (a.first != b.first) return a.first < b.first;
            return a.second.id < b.second.id;
        });

        std::vector<Point2D> res;
        for (size_t i = 0; i < k; ++i) {
            res.push_back(ranked[i].second);
        }
        return res;
    }
};

} // namespace kd

// ============================================================================
// Verification Suite
// ============================================================================

void run_unit_tests() {
    using namespace kd;

    std::vector<Point2D> pts = {
        Point2D(0, 2, 3),
        Point2D(1, 5, 4),
        Point2D(2, 9, 6),
        Point2D(3, 4, 7),
        Point2D(4, 8, 1),
        Point2D(5, 7, 2)
    };

    KDTree tree(pts);
    assert(tree.size() == 6);

    // Range search
    BoundingBox2D box{2, 6, 2, 5};
    auto range_res = tree.rangeSearch(box);
    auto naive_range = KDOracles::naiveRangeSearch(pts, box);
    assert(range_res == naive_range);

    // 1-NN query
    Point2D query(99, 9, 2);
    auto nn = tree.nearestNeighbor(query);
    assert(nn.id == 4 || nn.id == 5); // (8, 1) or (7, 2)

    // 3-NN query
    auto knn = tree.kNearestNeighbors(query, 3);
    auto naive_knn = KDOracles::naiveKNN(pts, query, 3);
    // Compare distances of returned k-NN set
    assert(knn.size() == 3);
    for (size_t i = 0; i < 3; ++i) {
        assert(dist_sq_exact(knn[i], query) == dist_sq_exact(naive_knn[i], query));
    }
}

void run_differential_stress_tests() {
    using namespace kd;
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int64_t> dist(-500, 500);

    for (int trial = 0; trial < 100; ++trial) {
        size_t n = 60;
        std::vector<Point2D> pts;
        pts.reserve(n);
        for (size_t i = 0; i < n; ++i) {
            pts.emplace_back(static_cast<int>(i), dist(rng), dist(rng));
        }

        KDTree tree(pts);

        // 1. Range search verification
        int64_t x1 = dist(rng), x2 = dist(rng);
        int64_t y1 = dist(rng), y2 = dist(rng);
        BoundingBox2D box{std::min(x1, x2), std::max(x1, x2), std::min(y1, y2), std::max(y1, y2)};

        auto tree_range = tree.rangeSearch(box);
        auto naive_range = KDOracles::naiveRangeSearch(pts, box);
        assert(tree_range == naive_range);

        // 2. K-NN search verification
        Point2D query(999, dist(rng), dist(rng));
        size_t k = 5;
        auto tree_knn = tree.kNearestNeighbors(query, k);
        auto naive_knn = KDOracles::naiveKNN(pts, query, k);

        assert(tree_knn.size() == naive_knn.size());
        for (size_t i = 0; i < tree_knn.size(); ++i) {
            assert(dist_sq_exact(tree_knn[i], query) == dist_sq_exact(naive_knn[i], query));
        }
    }
}

int main() {
    std::cout << "[Verification] Running KD-Tree test suite...\n";
    run_unit_tests();
    run_differential_stress_tests();
    std::cout << "[Verification] All 100 differential trials matched 100% with naive oracles!\n";
    return 0;
}
