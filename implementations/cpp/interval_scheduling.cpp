/**
 * Reference Implementation: Interval Scheduling & Partitioning Patterns
 * Demonstrates:
 * 1. Unweighted Interval Scheduling (Earliest Finish Time Greedy) - O(n log n)
 * 2. Schedule Reconstruction for Unweighted Scheduling - O(n log n)
 * 3. Meeting Rooms II / Interval Partitioning (Min-Heap) - O(n log n)
 * 4. Meeting Rooms II / Interval Partitioning (Sweep-Line Event Delta) - O(n log n)
 * 5. Weighted Interval Scheduling (DP + Binary Search) - O(n log n)
 * 6. Optimal Subset Reconstruction for Weighted Scheduling - O(n log n)
 * 7. Merge Overlapping Intervals - O(n log n)
 * 8. Insert Interval into Sorted Disjoint Intervals - O(n)
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <cassert>
#include <tuple>

namespace interval_patterns {

// Standard half-open interval [start, end)
struct Interval {
    int start;
    int end;

    bool operator==(const Interval& other) const {
        return start == other.start && end == other.end;
    }
};

// Weighted interval with 64-bit weight value
struct WeightedInterval {
    int start;
    int end;
    long long weight;

    bool operator==(const WeightedInterval& other) const {
        return start == other.start && end == other.end && weight == other.weight;
    }
};

// ============================================================================
// 1. Unweighted Interval Scheduling (Max Disjoint Intervals)
// ============================================================================

/**
 * Computes the maximum number of mutually non-overlapping intervals.
 * Greedy choice: Earliest Finish Time.
 * In half-open [s, e), interval b can start exactly when a finishes (b.start >= a.end).
 * Time Complexity: O(n log n) due to sorting. Space: O(1) auxiliary.
 */
int max_non_overlapping_intervals(std::vector<Interval> intervals) {
    if (intervals.empty()) return 0;

    std::sort(intervals.begin(), intervals.end(),
              [](const Interval& a, const Interval& b) {
                  if (a.end != b.end) return a.end < b.end;
                  return a.start < b.start;
              });

    int count = 0;
    int last_end = -2000000000;

    for (const auto& in : intervals) {
        if (in.start >= last_end) {
            ++count;
            last_end = in.end;
        }
    }

    return count;
}

/**
 * Reconstructs the actual optimal subset of mutually disjoint intervals.
 * Time Complexity: O(n log n). Space: O(n) for schedule list.
 */
std::vector<Interval> reconstruct_non_overlapping_schedule(std::vector<Interval> intervals) {
    if (intervals.empty()) return {};

    std::sort(intervals.begin(), intervals.end(),
              [](const Interval& a, const Interval& b) {
                  if (a.end != b.end) return a.end < b.end;
                  return a.start < b.start;
              });

    std::vector<Interval> chosen;
    int last_end = -2000000000;

    for (const auto& in : intervals) {
        if (in.start >= last_end) {
            chosen.push_back(in);
            last_end = in.end;
        }
    }

    return chosen;
}

// ============================================================================
// 2. Meeting Rooms II / Interval Partitioning
// ============================================================================

/**
 * Finds the minimum number of resources (meeting rooms) needed to schedule all intervals.
 * Approach A: Min-Heap tracking active room end-times.
 * Sorts by start time. Frees all rooms where end <= current.start.
 * Time Complexity: O(n log n). Space: O(n).
 */
int min_meeting_rooms_heap(std::vector<Interval> intervals) {
    if (intervals.empty()) return 0;

    std::sort(intervals.begin(), intervals.end(),
              [](const Interval& a, const Interval& b) {
                  if (a.start != b.start) return a.start < b.start;
                  return a.end < b.end;
              });

    std::priority_queue<int, std::vector<int>, std::greater<int>> pq;
    int peak_rooms = 0;

    for (const auto& in : intervals) {
        while (!pq.empty() && pq.top() <= in.start) {
            pq.pop();
        }
        pq.push(in.end);
        peak_rooms = std::max(peak_rooms, static_cast<int>(pq.size()));
    }

    return peak_rooms;
}

/**
 * Finds the minimum number of meeting rooms using a Sweep-Line algorithm.
 * Approach B: Event points (+1 at start, -1 at end).
 * Tie-breaking: In half-open [s, e), an end at t frees a room before a start at t claims one.
 * Thus delta -1 must be processed before delta +1 at the same coordinate.
 * Time Complexity: O(n log n). Space: O(n).
 */
int min_meeting_rooms_sweepline(const std::vector<Interval>& intervals) {
    if (intervals.empty()) return 0;

    // pair<time, delta>: delta = -1 for end, +1 for start
    std::vector<std::pair<int, int>> events;
    events.reserve(intervals.size() * 2);

    for (const auto& in : intervals) {
        events.push_back({in.start, +1});
        events.push_back({in.end, -1});
    }

    std::sort(events.begin(), events.end(),
              [](const auto& a, const auto& b) {
                  if (a.first != b.first) return a.first < b.first;
                  return a.second < b.second; // -1 precedes +1
              });

    int active = 0;
    int peak = 0;
    for (const auto& [time, delta] : events) {
        active += delta;
        peak = std::max(peak, active);
    }

    return peak;
}

// ============================================================================
// 3. Weighted Interval Scheduling (DP + Binary Search)
// ============================================================================

/**
 * Solves Weighted Interval Scheduling using dynamic programming with binary search.
 * dp[i] = max(dp[i - 1], weight[i] + dp[p(i)])
 * where p(i) is the largest index j < i such that interval j is compatible (end_j <= start_i).
 * Time Complexity: O(n log n). Space: O(n).
 */
long long weighted_interval_scheduling(std::vector<WeightedInterval> intervals) {
    if (intervals.empty()) return 0;

    std::sort(intervals.begin(), intervals.end(),
              [](const WeightedInterval& a, const WeightedInterval& b) {
                  if (a.end != b.end) return a.end < b.end;
                  return a.start < b.start;
              });

    int n = static_cast<int>(intervals.size());
    std::vector<int> ends(n + 1, 0);
    std::vector<int> starts(n + 1, 0);
    std::vector<long long> weights(n + 1, 0);

    for (int i = 1; i <= n; ++i) {
        starts[i] = intervals[i - 1].start;
        ends[i] = intervals[i - 1].end;
        weights[i] = intervals[i - 1].weight;
    }

    std::vector<int> p(n + 1, 0);
    for (int i = 1; i <= n; ++i) {
        int s = starts[i];
        // Find largest j < i with ends[j] <= s
        p[i] = static_cast<int>(
            std::upper_bound(ends.begin(), ends.begin() + i, s) - ends.begin()
        ) - 1;
    }

    std::vector<long long> dp(n + 1, 0);
    for (int i = 1; i <= n; ++i) {
        dp[i] = std::max(dp[i - 1], weights[i] + dp[p[i]]);
    }

    return dp[n];
}

/**
 * Computes max weight and reconstructs the optimal set of intervals.
 * Time Complexity: O(n log n). Space: O(n).
 */
std::pair<long long, std::vector<WeightedInterval>> weighted_interval_scheduling_reconstruct(
    std::vector<WeightedInterval> intervals) {
    if (intervals.empty()) return {0, {}};

    std::sort(intervals.begin(), intervals.end(),
              [](const WeightedInterval& a, const WeightedInterval& b) {
                  if (a.end != b.end) return a.end < b.end;
                  return a.start < b.start;
              });

    int n = static_cast<int>(intervals.size());
    std::vector<int> ends(n + 1, 0);
    std::vector<int> starts(n + 1, 0);
    std::vector<long long> weights(n + 1, 0);
    std::vector<WeightedInterval> items(n + 1);

    for (int i = 1; i <= n; ++i) {
        starts[i] = intervals[i - 1].start;
        ends[i] = intervals[i - 1].end;
        weights[i] = intervals[i - 1].weight;
        items[i] = intervals[i - 1];
    }

    std::vector<int> p(n + 1, 0);
    for (int i = 1; i <= n; ++i) {
        int s = starts[i];
        p[i] = static_cast<int>(
            std::upper_bound(ends.begin(), ends.begin() + i, s) - ends.begin()
        ) - 1;
    }

    std::vector<long long> dp(n + 1, 0);
    for (int i = 1; i <= n; ++i) {
        dp[i] = std::max(dp[i - 1], weights[i] + dp[p[i]]);
    }

    std::vector<WeightedInterval> chosen;
    int curr = n;
    while (curr > 0) {
        if (dp[curr] == dp[curr - 1]) {
            curr -= 1;
        } else {
            chosen.push_back(items[curr]);
            curr = p[curr];
        }
    }

    std::reverse(chosen.begin(), chosen.end());
    return {dp[n], chosen};
}

// ============================================================================
// 4. Interval Merging & Insertion
// ============================================================================

/**
 * Merges all overlapping intervals into contiguous disjoint intervals.
 * For closed or half-open intervals where s <= prev.end causes overlap.
 * Time Complexity: O(n log n). Space: O(n) for output.
 */
std::vector<Interval> merge_intervals(std::vector<Interval> intervals) {
    if (intervals.empty()) return {};

    std::sort(intervals.begin(), intervals.end(),
              [](const Interval& a, const Interval& b) {
                  if (a.start != b.start) return a.start < b.start;
                  return a.end < b.end;
              });

    std::vector<Interval> merged;
    merged.push_back(intervals[0]);

    for (size_t i = 1; i < intervals.size(); ++i) {
        const auto& in = intervals[i];
        if (in.start <= merged.back().end) {
            merged.back().end = std::max(merged.back().end, in.end);
        } else {
            merged.push_back(in);
        }
    }

    return merged;
}

/**
 * Inserts a new interval into an already sorted list of non-overlapping intervals
 * and merges if necessary.
 * Three phases:
 *   1. Add all intervals ending strictly before new_interval begins.
 *   2. Merge all intervals overlapping with new_interval.
 *   3. Add all remaining intervals starting strictly after new_interval ends.
 * Time Complexity: O(n). Space: O(n) for output.
 */
std::vector<Interval> insert_interval(const std::vector<Interval>& intervals, Interval new_interval) {
    std::vector<Interval> result;
    result.reserve(intervals.size() + 1);

    size_t i = 0;
    size_t n = intervals.size();

    // Phase 1: completely before
    while (i < n && intervals[i].end < new_interval.start) {
        result.push_back(intervals[i]);
        ++i;
    }

    // Phase 2: overlapping
    while (i < n && intervals[i].start <= new_interval.end) {
        new_interval.start = std::min(new_interval.start, intervals[i].start);
        new_interval.end = std::max(new_interval.end, intervals[i].end);
        ++i;
    }
    result.push_back(new_interval);

    // Phase 3: completely after
    while (i < n) {
        result.push_back(intervals[i]);
        ++i;
    }

    return result;
}

} // namespace interval_patterns

// ============================================================================
// Unit Tests & Edge Case Verification
// ============================================================================

int main() {
    using namespace interval_patterns;

    std::cout << "Running Interval Scheduling C++17 unit tests...\n";

    // ------------------------------------------------------------------------
    // Test 1: Unweighted Interval Scheduling - Arthur's Counterexample
    // Input: [1, 10), [2, 3), [4, 5), [6, 7), [8, 9)
    // Earliest start greedily picks [1, 10) -> count = 1.
    // Earliest finish greedily picks 4 intervals -> count = 4.
    // ------------------------------------------------------------------------
    {
        std::vector<Interval> intervals = {{1, 10}, {2, 3}, {4, 5}, {6, 7}, {8, 9}};
        assert(max_non_overlapping_intervals(intervals) == 4);

        auto chosen = reconstruct_non_overlapping_schedule(intervals);
        assert(chosen.size() == 4);
        assert(chosen[0] == (Interval{2, 3}));
        assert(chosen[1] == (Interval{4, 5}));
        assert(chosen[2] == (Interval{6, 7}));
        assert(chosen[3] == (Interval{8, 9}));
    }

    // ------------------------------------------------------------------------
    // Test 2: Unweighted Scheduling - Empty & Single Element
    // ------------------------------------------------------------------------
    {
        assert(max_non_overlapping_intervals({}) == 0);
        assert(reconstruct_non_overlapping_schedule({}).empty());

        std::vector<Interval> single = {{5, 10}};
        assert(max_non_overlapping_intervals(single) == 1);
        auto chosen = reconstruct_non_overlapping_schedule(single);
        assert(chosen.size() == 1 && chosen[0] == (Interval{5, 10}));
    }

    // ------------------------------------------------------------------------
    // Test 3: Unweighted Scheduling - Touching Endpoints in [s, e)
    // [1, 3) and [3, 5) do not overlap
    // ------------------------------------------------------------------------
    {
        std::vector<Interval> intervals = {{1, 3}, {3, 5}, {5, 7}};
        assert(max_non_overlapping_intervals(intervals) == 3);
        auto chosen = reconstruct_non_overlapping_schedule(intervals);
        assert(chosen.size() == 3);
    }

    // ------------------------------------------------------------------------
    // Test 4: Meeting Rooms II - Equivalence of Heap and Sweep-Line
    // ------------------------------------------------------------------------
    {
        std::vector<Interval> intervals = {{0, 30}, {5, 10}, {15, 20}};
        // [0, 30] overlaps with both, but [5, 10] and [15, 20] do not overlap
        // Peak overlap = 2 rooms
        assert(min_meeting_rooms_heap(intervals) == 2);
        assert(min_meeting_rooms_sweepline(intervals) == 2);

        // Touching intervals [1, 5) and [5, 10):
        // In half-open convention, room frees at 5 before next room starts at 5 -> 1 room needed
        std::vector<Interval> touching = {{1, 5}, {5, 10}, {10, 15}};
        assert(min_meeting_rooms_heap(touching) == 1);
        assert(min_meeting_rooms_sweepline(touching) == 1);

        // All overlapping simultaneously
        std::vector<Interval> all_overlap = {{1, 10}, {2, 9}, {3, 8}, {4, 7}};
        assert(min_meeting_rooms_heap(all_overlap) == 4);
        assert(min_meeting_rooms_sweepline(all_overlap) == 4);

        // Empty input
        assert(min_meeting_rooms_heap({}) == 0);
        assert(min_meeting_rooms_sweepline({}) == 0);
    }

    // ------------------------------------------------------------------------
    // Test 5: Weighted Interval Scheduling - Arthur's Counterexample
    // [1, 2, w=2], [2, 3, w=2], [1, 3, w=10]
    // Earliest finish greedy picks [1, 2] and [2, 3] -> total weight = 4.
    // DP picks [1, 3] -> total weight = 10.
    // ------------------------------------------------------------------------
    {
        std::vector<WeightedInterval> intervals = {
            {1, 2, 2},
            {2, 3, 2},
            {1, 3, 10}
        };
        assert(weighted_interval_scheduling(intervals) == 10);
        auto [max_wt, chosen] = weighted_interval_scheduling_reconstruct(intervals);
        assert(max_wt == 10);
        assert(chosen.size() == 1);
        assert(chosen[0] == (WeightedInterval{1, 3, 10}));
    }

    // ------------------------------------------------------------------------
    // Test 6: Weighted Scheduling - Complex Schedule & Large Weights
    // ------------------------------------------------------------------------
    {
        std::vector<WeightedInterval> intervals = {
            {1, 4, 1000000000LL},
            {3, 5, 2000000000LL},
            {0, 6, 2500000000LL},
            {4, 7, 2000000000LL},
            {3, 8, 1000000000LL},
            {5, 9, 3000000000LL},
            {6, 10, 2000000000LL},
            {8, 11, 4000000000LL}
        };
        // Best subset: {1, 4, 10^9}, {4, 7, 2*10^9}, {8, 11, 4*10^9} => 7 * 10^9
        // Alternatively: {3, 5, 2*10^9}, {5, 9, 3*10^9} => 5*10^9
        // Or: {1, 4, 10^9}, {5, 9, 3*10^9} => 4*10^9
        // What about {0, 6, 2.5*10^9}, {8, 11, 4*10^9} => 6.5 * 10^9
        // Best is indeed 1-4 (1G) + 4-7 (2G) + 8-11 (4G) = 7G.
        long long ans = weighted_interval_scheduling(intervals);
        assert(ans == 7000000000LL);

        auto [wt, chosen] = weighted_interval_scheduling_reconstruct(intervals);
        assert(wt == 7000000000LL);
        assert(chosen.size() == 3);
        assert(chosen[0] == (WeightedInterval{1, 4, 1000000000LL}));
        assert(chosen[1] == (WeightedInterval{4, 7, 2000000000LL}));
        assert(chosen[2] == (WeightedInterval{8, 11, 4000000000LL}));
    }

    // ------------------------------------------------------------------------
    // Test 7: Merge Intervals
    // ------------------------------------------------------------------------
    {
        // Standard merge
        std::vector<Interval> intervals = {{1, 3}, {2, 6}, {8, 10}, {15, 18}};
        auto merged = merge_intervals(intervals);
        assert(merged.size() == 3);
        assert(merged[0] == (Interval{1, 6}));
        assert(merged[1] == (Interval{8, 10}));
        assert(merged[2] == (Interval{15, 18}));

        // Touching intervals [1, 4] and [4, 5] merge to [1, 5]
        std::vector<Interval> touching = {{1, 4}, {4, 5}};
        auto merged_touching = merge_intervals(touching);
        assert(merged_touching.size() == 1);
        assert(merged_touching[0] == (Interval{1, 5}));

        // Fully contained
        std::vector<Interval> contained = {{1, 10}, {2, 3}, {4, 8}};
        auto merged_contained = merge_intervals(contained);
        assert(merged_contained.size() == 1);
        assert(merged_contained[0] == (Interval{1, 10}));

        // Empty
        assert(merge_intervals({}).empty());
    }

    // ------------------------------------------------------------------------
    // Test 8: Insert Interval
    // ------------------------------------------------------------------------
    {
        std::vector<Interval> intervals = {{1, 3}, {6, 9}};
        auto res1 = insert_interval(intervals, {2, 5});
        assert(res1.size() == 2);
        assert(res1[0] == (Interval{1, 5}));
        assert(res1[1] == (Interval{6, 9}));

        std::vector<Interval> intervals2 = {{1, 2}, {3, 5}, {6, 7}, {8, 10}, {12, 16}};
        auto res2 = insert_interval(intervals2, {4, 8});
        assert(res2.size() == 3);
        assert(res2[0] == (Interval{1, 2}));
        assert(res2[1] == (Interval{3, 10}));
        assert(res2[2] == (Interval{12, 16}));

        // Insert before all
        auto res3 = insert_interval(intervals, {0, 0});
        assert(res3.size() == 3);
        assert(res3[0] == (Interval{0, 0}));

        // Insert after all
        auto res4 = insert_interval(intervals, {11, 12});
        assert(res4.size() == 3);
        assert(res4[2] == (Interval{11, 12}));

        // Insert into empty list
        auto res5 = insert_interval({}, {5, 7});
        assert(res5.size() == 1);
        assert(res5[0] == (Interval{5, 7}));
    }

    std::cout << "All Interval Scheduling C++17 unit tests passed successfully!\n";
    return 0;
}
