/**
 * @file link_cut_trees.cpp
 * @brief Reference implementation of Link-Cut Trees with Path XOR Aggregates.
 *
 * Implements Sleator & Tarjan's Link-Cut Tree using preferred path decomposition
 * with auxiliary splay trees, lazy path reversal, dynamic link/cut, and differential
 * verification against an independent graph/tree oracle.
 * Verified with C++17 (-std=c++17 -O3 -Wall -Wextra -Werror).
 */

#include <iostream>
#include <vector>
#include <cstdint>
#include <cassert>
#include <algorithm>
#include <random>
#include <queue>

namespace dsa {

/**
 * @brief Link-Cut Tree node stored in a flat 1-indexed vector.
 */
struct LctNode {
    int ch[2] = {0, 0};
    int p = 0;
    int64_t val = 0;
    int64_t agg = 0;
    bool rev = false;
};

/**
 * @brief Link-Cut Tree representing dynamic forests of rooted trees.
 * Supports link, cut, make_root, and path queries in amortized O(log N) time.
 */
class LinkCutTree {
private:
    int n_;
    std::vector<LctNode> tree_;

    bool is_root(int x) const {
        int p = tree_[x].p;
        return tree_[p].ch[0] != x && tree_[p].ch[1] != x;
    }

    void push_up(int x) {
        if (x == 0) return;
        tree_[x].agg = tree_[tree_[x].ch[0]].agg ^ tree_[x].val ^ tree_[tree_[x].ch[1]].agg;
    }

    void push_down(int x) {
        if (x == 0 || !tree_[x].rev) return;
        std::swap(tree_[x].ch[0], tree_[x].ch[1]);
        if (tree_[x].ch[0] != 0) tree_[tree_[x].ch[0]].rev ^= true;
        if (tree_[x].ch[1] != 0) tree_[tree_[x].ch[1]].rev ^= true;
        tree_[x].rev = false;
    }

    void rotate(int x) {
        int y = tree_[x].p;
        int z = tree_[y].p;
        int k = (tree_[y].ch[1] == x);

        if (!is_root(y)) {
            tree_[z].ch[tree_[z].ch[1] == y] = x;
        }
        tree_[x].p = z;

        tree_[y].ch[k] = tree_[x].ch[k ^ 1];
        if (tree_[x].ch[k ^ 1] != 0) {
            tree_[tree_[x].ch[k ^ 1]].p = y;
        }

        tree_[x].ch[k ^ 1] = y;
        tree_[y].p = x;

        push_up(y);
        push_up(x);
    }

    void splay(int x) {
        // Collect all ancestors up to the auxiliary root to push down lazily
        std::vector<int> stk;
        int curr = x;
        stk.push_back(curr);
        while (!is_root(curr)) {
            curr = tree_[curr].p;
            stk.push_back(curr);
        }
        while (!stk.empty()) {
            push_down(stk.back());
            stk.pop_back();
        }

        // Standard splay rotations
        while (!is_root(x)) {
            int y = tree_[x].p;
            int z = tree_[y].p;
            if (!is_root(y)) {
                if ((tree_[y].ch[1] == x) ^ (tree_[z].ch[1] == y)) {
                    rotate(x);
                } else {
                    rotate(y);
                }
            }
            rotate(x);
        }
    }

public:
    explicit LinkCutTree(int n) : n_(n), tree_(n + 1) {
        tree_[0].val = 0;
        tree_[0].agg = 0;
    }

    void set_val(int x, int64_t val) {
        assert(x >= 1 && x <= n_);
        splay(x);
        tree_[x].val = val;
        push_up(x);
    }

    int64_t get_val(int x) {
        assert(x >= 1 && x <= n_);
        splay(x);
        return tree_[x].val;
    }

    /**
     * @brief Expose the path from represented root to node x into a single preferred path.
     */
    void access(int x) {
        assert(x >= 1 && x <= n_);
        for (int t = 0; x != 0; t = x, x = tree_[x].p) {
            splay(x);
            tree_[x].ch[1] = t;
            push_up(x);
        }
    }

    /**
     * @brief Make node x the root of its represented tree.
     */
    void make_root(int x) {
        assert(x >= 1 && x <= n_);
        access(x);
        splay(x);
        tree_[x].rev ^= true;
    }

    /**
     * @brief Find the root of the represented tree containing node x.
     */
    int find_root(int x) {
        assert(x >= 1 && x <= n_);
        access(x);
        splay(x);
        while (tree_[x].ch[0] != 0) {
            push_down(x);
            x = tree_[x].ch[0];
        }
        splay(x);
        return x;
    }

    // --- Layer A: Fast Core API (Preconditioned) ---

    /**
     * @brief Connect tree of x to node y. Assumes x and y are in different trees.
     */
    void link(int x, int y) {
        assert(x >= 1 && x <= n_ && y >= 1 && y <= n_);
        make_root(x);
        assert(find_root(y) != x); // Precondition: no cycles
        tree_[x].p = y;
    }

    /**
     * @brief Sever the edge between adjacent nodes x and y.
     */
    void cut(int x, int y) {
        assert(x >= 1 && x <= n_ && y >= 1 && y <= n_);
        make_root(x);
        access(y);
        splay(y);
        assert(tree_[y].ch[0] == x && tree_[x].ch[1] == 0); // Precondition: edge exists
        tree_[y].ch[0] = 0;
        tree_[x].p = 0;
        push_up(y);
    }

    /**
     * @brief Compute aggregate (XOR sum) along path between x and y.
     */
    int64_t query_path(int x, int y) {
        assert(x >= 1 && x <= n_ && y >= 1 && y <= n_);
        make_root(x);
        access(y);
        splay(y);
        return tree_[y].agg;
    }

    // --- Layer B: Safe Adapter API ---

    /**
     * @brief Check if x and y belong to the same represented tree.
     */
    bool is_connected(int x, int y) {
        assert(x >= 1 && x <= n_ && y >= 1 && y <= n_);
        if (x == y) return true;
        make_root(x);
        return find_root(y) == x;
    }

    /**
     * @brief Safe link: returns true if link succeeded, false if already connected.
     */
    bool try_link(int x, int y) {
        assert(x >= 1 && x <= n_ && y >= 1 && y <= n_);
        make_root(x);
        if (find_root(y) == x) return false;
        tree_[x].p = y;
        return true;
    }

    /**
     * @brief Safe cut: returns true if cut succeeded, false if edge does not exist.
     */
    bool try_cut(int x, int y) {
        assert(x >= 1 && x <= n_ && y >= 1 && y <= n_);
        make_root(x);
        if (find_root(y) != x) return false;
        access(y);
        splay(y);
        if (tree_[y].ch[0] != x || tree_[x].ch[1] != 0) {
            return false;
        }
        tree_[y].ch[0] = 0;
        tree_[x].p = 0;
        push_up(y);
        return true;
    }
};

/**
 * @brief Naive Tree Oracle for differential verification.
 */
class NaiveTreeOracle {
private:
    int n_;
    std::vector<int64_t> val_;
    std::vector<std::vector<int>> adj_;

    bool dfs_path(int u, int target, int parent, std::vector<int>& path) {
        path.push_back(u);
        if (u == target) return true;
        for (int v : adj_[u]) {
            if (v != parent) {
                if (dfs_path(v, target, u, path)) return true;
            }
        }
        path.pop_back();
        return false;
    }

public:
    explicit NaiveTreeOracle(int n) : n_(n), val_(n + 1, 0), adj_(n + 1) {}

    void set_val(int x, int64_t v) {
        val_[x] = v;
    }

    bool is_connected(int u, int v) {
        std::vector<int> path;
        return dfs_path(u, v, 0, path);
    }

    bool link(int u, int v) {
        if (is_connected(u, v)) return false;
        adj_[u].push_back(v);
        adj_[v].push_back(u);
        return true;
    }

    bool cut(int u, int v) {
        auto it1 = std::find(adj_[u].begin(), adj_[u].end(), v);
        auto it2 = std::find(adj_[v].begin(), adj_[v].end(), u);
        if (it1 == adj_[u].end() || it2 == adj_[v].end()) return false;
        adj_[u].erase(it1);
        adj_[v].erase(it2);
        return true;
    }

    int64_t query_path(int u, int v) {
        std::vector<int> path;
        bool found = dfs_path(u, v, 0, path);
        assert(found);
        int64_t res = 0;
        for (int node : path) {
            res ^= val_[node];
        }
        return res;
    }
};

} // namespace dsa

int main() {
    std::cout << "Running Link-Cut Tree verification..." << std::endl;

    const int N = 20;
    dsa::LinkCutTree lct(N);
    dsa::NaiveTreeOracle oracle(N);

    for (int i = 1; i <= N; ++i) {
        int64_t val = i * 17 + 3;
        lct.set_val(i, val);
        oracle.set_val(i, val);
    }

    // 1. Basic Path & Connectivity Tests
    assert(lct.try_link(1, 2));
    assert(oracle.link(1, 2));

    assert(lct.try_link(2, 3));
    assert(oracle.link(2, 3));

    assert(lct.try_link(3, 4));
    assert(oracle.link(3, 4));

    // Cycle rejection
    assert(!lct.try_link(1, 4));
    assert(!oracle.link(1, 4));

    assert(lct.is_connected(1, 4));
    assert(oracle.is_connected(1, 4));
    assert(!lct.is_connected(1, 5));
    assert(!oracle.is_connected(1, 5));

    // Path XOR query
    int64_t lct_xor = lct.query_path(1, 4);
    int64_t oracle_xor = oracle.query_path(1, 4);
    assert(lct_xor == oracle_xor);

    // Dynamic cut
    assert(lct.try_cut(2, 3));
    assert(oracle.cut(2, 3));
    assert(!lct.is_connected(1, 4));
    assert(!oracle.is_connected(1, 4));
    assert(lct.is_connected(1, 2));
    assert(oracle.is_connected(1, 2));
    assert(lct.is_connected(3, 4));
    assert(oracle.is_connected(3, 4));

    // 2. Randomized Command-Sequence Differential Fuzzing
    std::mt19937_64 rng(1337);
    for (int step = 0; step < 500; ++step) {
        int op = rng() % 4;
        int u = (rng() % N) + 1;
        int v = (rng() % N) + 1;

        if (op == 0) {
            // Link
            bool r1 = lct.try_link(u, v);
            bool r2 = oracle.link(u, v);
            assert(r1 == r2);
        } else if (op == 1) {
            // Cut
            bool r1 = lct.try_cut(u, v);
            bool r2 = oracle.cut(u, v);
            assert(r1 == r2);
        } else if (op == 2) {
            // Query path
            bool conn1 = lct.is_connected(u, v);
            bool conn2 = oracle.is_connected(u, v);
            assert(conn1 == conn2);
            if (conn1) {
                int64_t q1 = lct.query_path(u, v);
                int64_t q2 = oracle.query_path(u, v);
                assert(q1 == q2);
            }
        } else {
            // Update node value
            int64_t new_val = rng() % 10000;
            lct.set_val(u, new_val);
            oracle.set_val(u, new_val);
        }
    }

    std::cout << "[PASS] Basic path, cycle rejection, and dynamic cut verified." << std::endl;
    std::cout << "[PASS] 500 randomized command-sequence differential fuzzing operations matched oracle." << std::endl;
    std::cout << "All Link-Cut Tree assertions passed successfully!" << std::endl;
    return 0;
}
