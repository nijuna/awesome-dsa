/**
 * @file r_trees.cpp
 * @brief R-Tree Spatial Index: Minimum Bounding Rectangles (MBR), Guttman's Quadratic Split,
 *        and Spatial Overlap Queries.
 *
 * Implements Arthur's Two-Layer API:
 *   - Layer 1: Rect MBR algebra (area, enclosing, intersection, expansion penalty),
 *              Guttman's Quadratic Split algorithm.
 *   - Layer 2: Safe, high-level RTreeEngine supporting dynamic insertion, spatial overlap search,
 *              MBR recalculation, and differential verification against O(N) oracle.
 *
 * Compilation:
 *   g++ -std=c++17 -O3 -Wall -Wextra -Werror r_trees.cpp -o r_trees
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cassert>
#include <memory>
#include <limits>
#include <random>

namespace rtree {

// ============================================================================
// Layer 1: Geometric Rect Primitives & MBR Algebra
// ============================================================================

struct Rect {
    int64_t min_x{0};
    int64_t max_x{0};
    int64_t min_y{0};
    int64_t max_y{0};

    constexpr Rect() = default;
    constexpr Rect(int64_t x1, int64_t x2, int64_t y1, int64_t y2)
        : min_x(std::min(x1, x2)), max_x(std::max(x1, x2)),
          min_y(std::min(y1, y2)), max_y(std::max(y1, y2)) {}

    __int128_t area() const {
        __int128_t w = static_cast<__int128_t>(max_x) - static_cast<__int128_t>(min_x);
        __int128_t h = static_cast<__int128_t>(max_y) - static_cast<__int128_t>(min_y);
        return w * h;
    }

    bool intersects(Rect const& o) const {
        return !(max_x < o.min_x || min_x > o.max_x || max_y < o.min_y || min_y > o.max_y);
    }

    bool contains(Rect const& o) const {
        return min_x <= o.min_x && max_x >= o.max_x && min_y <= o.min_y && max_y >= o.max_y;
    }

    Rect enclosing(Rect const& o) const {
        return Rect(std::min(min_x, o.min_x), std::max(max_x, o.max_x),
                    std::min(min_y, o.min_y), std::max(max_y, o.max_y));
    }

    constexpr bool operator==(Rect const& o) const {
        return min_x == o.min_x && max_x == o.max_x && min_y == o.min_y && max_y == o.max_y;
    }
};

struct Node;

struct Entry {
    Rect box;
    int id{-1};                  // Valid for leaf entries
    std::unique_ptr<Node> child; // Valid for internal entries

    Entry() = default;
    Entry(Rect b, int item_id) : box(b), id(item_id), child(nullptr) {}
    Entry(Rect b, std::unique_ptr<Node> ch) : box(b), id(-1), child(std::move(ch)) {}
};

constexpr size_t MAX_ENTRIES = 4;
constexpr size_t MIN_ENTRIES = 2;

struct Node {
    bool is_leaf{true};
    std::vector<Entry> entries;

    explicit Node(bool leaf) : is_leaf(leaf) {
        entries.reserve(MAX_ENTRIES + 1);
    }

    Rect computeMBR() const {
        assert(!entries.empty());
        Rect mbr = entries[0].box;
        for (size_t i = 1; i < entries.size(); ++i) {
            mbr = mbr.enclosing(entries[i].box);
        }
        return mbr;
    }
};

// ============================================================================
// Layer 2: High-Level R-Tree Engine
// ============================================================================

class RTree {
private:
    std::unique_ptr<Node> root_{nullptr};
    size_t size_{0};

    static std::pair<std::unique_ptr<Node>, std::unique_ptr<Node>> quadraticSplit(
        std::vector<Entry>& all_entries, bool is_leaf) {
        size_t n = all_entries.size();
        assert(n > MAX_ENTRIES);

        // Step 1: PickSeeds
        size_t seed1 = 0, seed2 = 1;
        __int128_t max_waste = -1;

        for (size_t i = 0; i < n; ++i) {
            for (size_t j = i + 1; j < n; ++j) {
                Rect enc = all_entries[i].box.enclosing(all_entries[j].box);
                __int128_t waste = enc.area() - all_entries[i].box.area() - all_entries[j].box.area();
                if (waste > max_waste) {
                    max_waste = waste;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }

        auto node1 = std::make_unique<Node>(is_leaf);
        auto node2 = std::make_unique<Node>(is_leaf);

        Rect mbr1 = all_entries[seed1].box;
        Rect mbr2 = all_entries[seed2].box;

        node1->entries.push_back(std::move(all_entries[seed1]));
        node2->entries.push_back(std::move(all_entries[seed2]));

        std::vector<bool> assigned(n, false);
        assigned[seed1] = true;
        assigned[seed2] = true;
        size_t remaining = n - 2;

        // Step 2: DistributeRemaining
        while (remaining > 0) {
            if (node1->entries.size() + remaining == MIN_ENTRIES) {
                for (size_t i = 0; i < n; ++i) {
                    if (!assigned[i]) {
                        mbr1 = mbr1.enclosing(all_entries[i].box);
                        node1->entries.push_back(std::move(all_entries[i]));
                        assigned[i] = true;
                    }
                }
                break;
            }
            if (node2->entries.size() + remaining == MIN_ENTRIES) {
                for (size_t i = 0; i < n; ++i) {
                    if (!assigned[i]) {
                        mbr2 = mbr2.enclosing(all_entries[i].box);
                        node2->entries.push_back(std::move(all_entries[i]));
                        assigned[i] = true;
                    }
                }
                break;
            }

            // Find entry with maximum difference in area expansion
            size_t best_entry = 0;
            __int128_t max_diff = -1;
            __int128_t best_d1 = 0, best_d2 = 0;

            for (size_t i = 0; i < n; ++i) {
                if (assigned[i]) continue;
                __int128_t d1 = mbr1.enclosing(all_entries[i].box).area() - mbr1.area();
                __int128_t d2 = mbr2.enclosing(all_entries[i].box).area() - mbr2.area();
                __int128_t diff = (d1 > d2) ? (d1 - d2) : (d2 - d1);

                if (diff > max_diff) {
                    max_diff = diff;
                    best_entry = i;
                    best_d1 = d1;
                    best_d2 = d2;
                }
            }

            // Assign best_entry to the group requiring less enlargement
            if (best_d1 < best_d2) {
                mbr1 = mbr1.enclosing(all_entries[best_entry].box);
                node1->entries.push_back(std::move(all_entries[best_entry]));
            } else if (best_d2 < best_d1) {
                mbr2 = mbr2.enclosing(all_entries[best_entry].box);
                node2->entries.push_back(std::move(all_entries[best_entry]));
            } else {
                // Tie: assign to group with smaller area
                if (mbr1.area() <= mbr2.area()) {
                    mbr1 = mbr1.enclosing(all_entries[best_entry].box);
                    node1->entries.push_back(std::move(all_entries[best_entry]));
                } else {
                    mbr2 = mbr2.enclosing(all_entries[best_entry].box);
                    node2->entries.push_back(std::move(all_entries[best_entry]));
                }
            }

            assigned[best_entry] = true;
            --remaining;
        }

        return {std::move(node1), std::move(node2)};
    }

    // Recursive insert; returns a new sibling entry if current node splits, or nullptr
    std::unique_ptr<Node> insertRec(Node* node, Entry new_entry) {
        if (node->is_leaf) {
            node->entries.push_back(std::move(new_entry));
            if (node->entries.size() > MAX_ENTRIES) {
                auto split_res = quadraticSplit(node->entries, true);
                *node = std::move(*split_res.first);
                return std::move(split_res.second);
            }
            return nullptr;
        }

        // ChooseLeaf: pick child entry whose MBR needs least enlargement
        size_t best_idx = 0;
        __int128_t min_enlargement = -1;
        __int128_t min_area = -1;

        for (size_t i = 0; i < node->entries.size(); ++i) {
            Rect enc = node->entries[i].box.enclosing(new_entry.box);
            __int128_t enlargement = enc.area() - node->entries[i].box.area();
            __int128_t current_area = node->entries[i].box.area();

            if (min_enlargement < 0 || enlargement < min_enlargement ||
                (enlargement == min_enlargement && current_area < min_area)) {
                min_enlargement = enlargement;
                min_area = current_area;
                best_idx = i;
            }
        }

        auto split_child = insertRec(node->entries[best_idx].child.get(), std::move(new_entry));
        node->entries[best_idx].box = node->entries[best_idx].child->computeMBR();

        if (split_child) {
            Rect sibling_mbr = split_child->computeMBR();
            Entry sibling_entry(sibling_mbr, std::move(split_child));
            node->entries.push_back(std::move(sibling_entry));

            if (node->entries.size() > MAX_ENTRIES) {
                auto split_res = quadraticSplit(node->entries, false);
                *node = std::move(*split_res.first);
                return std::move(split_res.second);
            }
        }

        return nullptr;
    }

    void searchRec(Node const* node, Rect const& query_box, std::vector<int>& results) const {
        if (!node) return;

        for (auto const& entry : node->entries) {
            if (entry.box.intersects(query_box)) {
                if (node->is_leaf) {
                    results.push_back(entry.id);
                } else {
                    searchRec(entry.child.get(), query_box, results);
                }
            }
        }
    }

public:
    RTree() : root_(std::make_unique<Node>(true)), size_(0) {}

    size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    /**
     * @brief Inserts a spatial object with its bounding box.
     */
    void insert(int id, Rect const& box) {
        Entry new_entry(box, id);
        auto split_root = insertRec(root_.get(), std::move(new_entry));

        if (split_root) {
            // Root has split; create new root with height + 1
            Rect mbr1 = root_->computeMBR();
            Rect mbr2 = split_root->computeMBR();
            auto new_root = std::make_unique<Node>(false);
            new_root->entries.emplace_back(mbr1, std::move(root_));
            new_root->entries.emplace_back(mbr2, std::move(split_root));
            root_ = std::move(new_root);
        }

        ++size_;
    }

    /**
     * @brief Spatial overlap search: finds all object IDs whose MBR intersects query_box.
     */
    std::vector<int> search(Rect const& query_box) const {
        std::vector<int> results;
        searchRec(root_.get(), query_box, results);
        std::sort(results.begin(), results.end());
        return results;
    }
};

// ============================================================================
// Differential Testing Oracle
// ============================================================================

struct RTreeOracle {
    static std::vector<int> naiveSearch(std::vector<std::pair<int, Rect>> const& items, Rect const& query_box) {
        std::vector<int> results;
        for (auto const& item : items) {
            if (item.second.intersects(query_box)) {
                results.push_back(item.first);
            }
        }
        std::sort(results.begin(), results.end());
        return results;
    }
};

} // namespace rtree

// ============================================================================
// Verification Suite
// ============================================================================

void run_unit_tests() {
    using namespace rtree;

    RTree tree;
    assert(tree.empty());

    // Insert 6 rectangles (triggers root split with MAX_ENTRIES = 4)
    tree.insert(1, Rect(0, 2, 0, 2));
    tree.insert(2, Rect(5, 7, 5, 7));
    tree.insert(3, Rect(1, 3, 1, 3));
    tree.insert(4, Rect(10, 12, 10, 12));
    tree.insert(5, Rect(6, 8, 6, 8));
    tree.insert(6, Rect(0, 1, 0, 1));

    assert(tree.size() == 6);

    // Query overlapping (0,0)-(2,2)
    auto q1 = tree.search(Rect(0, 2, 0, 2));
    // Expect items 1, 3, 6
    std::vector<int> exp1 = {1, 3, 6};
    assert(q1 == exp1);

    // Query overlapping (5,5)-(8,8)
    auto q2 = tree.search(Rect(5, 8, 5, 8));
    std::vector<int> exp2 = {2, 5};
    assert(q2 == exp2);

    // Query non-intersecting
    auto q3 = tree.search(Rect(20, 25, 20, 25));
    assert(q3.empty());
}

void run_differential_stress_tests() {
    using namespace rtree;
    std::mt19937_64 rng(42);
    std::uniform_int_distribution<int64_t> coord_dist(-200, 200);
    std::uniform_int_distribution<int64_t> size_dist(1, 25);

    for (int trial = 0; trial < 50; ++trial) {
        RTree tree;
        std::vector<std::pair<int, Rect>> items;
        size_t n = 60;

        for (size_t i = 0; i < n; ++i) {
            int64_t x = coord_dist(rng);
            int64_t y = coord_dist(rng);
            int64_t w = size_dist(rng);
            int64_t h = size_dist(rng);
            Rect box(x, x + w, y, y + h);
            items.emplace_back(static_cast<int>(i), box);
            tree.insert(static_cast<int>(i), box);
        }

        // Run 10 random range queries per trial
        for (int q = 0; q < 10; ++q) {
            int64_t qx = coord_dist(rng);
            int64_t qy = coord_dist(rng);
            int64_t qw = size_dist(rng) * 2;
            int64_t qh = size_dist(rng) * 2;
            Rect q_box(qx, qx + qw, qy, qy + qh);

            auto tree_res = tree.search(q_box);
            auto naive_res = RTreeOracle::naiveSearch(items, q_box);
            assert(tree_res == naive_res);
        }
    }
}

int main() {
    std::cout << "[Verification] Running R-Tree test suite...\n";
    run_unit_tests();
    run_differential_stress_tests();
    std::cout << "[Verification] All R-Tree differential trials matched 100% with naive oracle!\n";
    return 0;
}
