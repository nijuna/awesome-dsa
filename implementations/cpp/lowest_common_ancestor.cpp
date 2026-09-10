/**
 * Reference Implementation: Lowest Common Ancestor (LCA) via Binary Lifting
 * Production-ready modern C++17 implementation with unit tests.
 */

#include <iostream>
#include <vector>
#include <cmath>
#include <cassert>
#include <algorithm>

class BinaryLiftingLCA {
private:
    int n_;
    int log_;
    int root_;
    std::vector<int> depth_;
    std::vector<std::vector<int>> up_;

    void build_iterative(int root, const std::vector<std::vector<int>>& adj) {
        // Explicit stack to prevent call-stack overflows on skewed trees
        struct Frame {
            int u;
            int p;
            int d;
            bool visited;
        };

        std::vector<Frame> stack;
        stack.push_back({root, root, 0, false});

        while (!stack.empty()) {
            Frame frame = stack.back();
            stack.pop_back();

            if (!frame.visited) {
                depth_[frame.u] = frame.d;
                up_[frame.u][0] = frame.p;
                for (int i = 1; i < log_; ++i) {
                    up_[frame.u][i] = up_[up_[frame.u][i - 1]][i - 1];
                }

                stack.push_back({frame.u, frame.p, frame.d, true});
                for (int v : adj[frame.u]) {
                    if (v != frame.p) {
                        stack.push_back({v, frame.u, frame.d + 1, false});
                    }
                }
            }
        }
    }

public:
    BinaryLiftingLCA(int n, int root, const std::vector<std::vector<int>>& adj)
        : n_(n),
          log_(static_cast<int>(std::ceil(std::log2(std::max(n, 2)))) + 1),
          root_(root),
          depth_(n + 1, 0),
          up_(n + 1, std::vector<int>(log_, 0)) {
        build_iterative(root, adj);
    }

    int query(int u, int v) const {
        if (depth_[u] < depth_[v]) {
            std::swap(u, v);
        }

        // Lift u to the same depth as v
        int diff = depth_[u] - depth_[v];
        for (int i = 0; i < log_; ++i) {
            if ((diff >> i) & 1) {
                u = up_[u][i];
            }
        }

        if (u == v) return u;

        // Jump together in powers of 2
        for (int i = log_ - 1; i >= 0; --i) {
            if (up_[u][i] != up_[v][i]) {
                u = up_[u][i];
                v = up_[v][i];
            }
        }

        return up_[u][0];
    }

    int distance(int u, int v) const {
        int lca_node = query(u, v);
        return depth_[u] + depth_[v] - 2 * depth_[lca_node];
    }
};

void run_tests() {
    /* Tree topology:
           1
          / \
         2   3
        / \   \
       4   5   6
    */
    int n = 6;
    std::vector<std::vector<int>> adj(n + 1);
    std::vector<std::pair<int, int>> edges = {{1, 2}, {1, 3}, {2, 4}, {2, 5}, {3, 6}};
    for (const auto& [u, v] : edges) {
        adj[u].push_back(v);
        adj[v].push_back(u);
    }

    BinaryLiftingLCA lca(n, 1, adj);

    assert(lca.query(4, 5) == 2);
    assert(lca.query(4, 6) == 1);
    assert(lca.query(2, 4) == 2);
    assert(lca.query(1, 6) == 1);
    assert(lca.query(4, 4) == 4);
    assert(lca.distance(4, 5) == 2);
    assert(lca.distance(4, 6) == 4);

    std::cout << "[PASS] All Binary Lifting LCA C++ unit tests passed.\n";
}

int main() {
    run_tests();
    return 0;
}
