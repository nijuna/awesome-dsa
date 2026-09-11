/**
 * Reference Implementation: Graph Traversal Patterns
 * Demonstrates:
 * 1. Standard BFS (layer-by-layer exploration, unweighted shortest path, parent reconstruction).
 * 2. Multi-source BFS (simultaneous wavefront expansion).
 * 3. 0-1 BFS with std::deque (O(V + E) shortest paths on 0/1 weighted graphs).
 * 4. DFS (recursive and iterative explicit stack).
 * 5. Cycle detection for undirected and directed graphs (3-color state tracking).
 * 6. Connected components enumeration.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <queue>
#include <deque>
#include <stack>
#include <cassert>
#include <algorithm>

struct BFSResult {
    std::vector<int> dist;
    std::vector<int> parent;

    [[nodiscard]] std::vector<int> reconstruct_path(int target) const {
        if (dist[target] == -1) return {};
        std::vector<int> path;
        for (int curr = target; curr != -1; curr = parent[curr]) {
            path.push_back(curr);
        }
        std::reverse(path.begin(), path.end());
        return path;
    }
};

// 1. Standard BFS with Parent Tracking
BFSResult bfs(const std::vector<std::vector<int>>& adj, int start) {
    int n = static_cast<int>(adj.size());
    std::vector<int> dist(n, -1);
    std::vector<int> parent(n, -1);
    std::queue<int> q;

    dist[start] = 0;
    q.push(start);

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v : adj[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }

    return {dist, parent};
}

// 2. Multi-Source BFS
std::vector<int> multi_source_bfs(const std::vector<std::vector<int>>& adj, const std::vector<int>& sources) {
    int n = static_cast<int>(adj.size());
    std::vector<int> dist(n, -1);
    std::queue<int> q;

    for (int s : sources) {
        dist[s] = 0;
        q.push(s);
    }

    while (!q.empty()) {
        int u = q.front();
        q.pop();

        for (int v : adj[u]) {
            if (dist[v] == -1) {
                dist[v] = dist[u] + 1;
                q.push(v);
            }
        }
    }

    return dist;
}

// 3. 0-1 BFS using std::deque for O(V + E) Shortest Paths
struct Edge01 {
    int to;
    int weight; // 0 or 1
};

std::vector<int> zero_one_bfs(const std::vector<std::vector<Edge01>>& adj, int start) {
    int n = static_cast<int>(adj.size());
    constexpr int INF = 1e9;
    std::vector<int> dist(n, INF);
    std::deque<int> dq;

    dist[start] = 0;
    dq.push_back(start);

    while (!dq.empty()) {
        int u = dq.front();
        dq.pop_front();

        for (const auto& edge : adj[u]) {
            int v = edge.to;
            int w = edge.weight;

            if (dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                if (w == 0) {
                    dq.push_front(v); // 0-weight edges pushed to front
                } else {
                    dq.push_back(v);  // 1-weight edges pushed to back
                }
            }
        }
    }

    return dist;
}

// 4A. Recursive DFS
void dfs_recursive_helper(const std::vector<std::vector<int>>& adj, int u, std::vector<bool>& visited, std::vector<int>& order) {
    visited[u] = true;
    order.push_back(u);

    for (int v : adj[u]) {
        if (!visited[v]) {
            dfs_recursive_helper(adj, v, visited, order);
        }
    }
}

std::vector<int> dfs_recursive(const std::vector<std::vector<int>>& adj, int start) {
    int n = static_cast<int>(adj.size());
    std::vector<bool> visited(n, false);
    std::vector<int> order;
    dfs_recursive_helper(adj, start, visited, order);
    return order;
}

// 4B. Iterative DFS using explicit stack
std::vector<int> dfs_iterative(const std::vector<std::vector<int>>& adj, int start) {
    int n = static_cast<int>(adj.size());
    std::vector<bool> visited(n, false);
    std::vector<int> order;
    std::stack<int> s;

    s.push(start);

    while (!s.empty()) {
        int u = s.top();
        s.pop();

        if (visited[u]) continue;
        visited[u] = true;
        order.push_back(u);

        // Push neighbors in reverse to preserve left-to-right visitation
        for (auto it = adj[u].rbegin(); it != adj[u].rend(); ++it) {
            if (!visited[*it]) {
                s.push(*it);
            }
        }
    }

    return order;
}

// 5A. Cycle Detection in Undirected Graph
bool has_cycle_undirected_helper(const std::vector<std::vector<int>>& adj, int u, int parent, std::vector<bool>& visited) {
    visited[u] = true;

    for (int v : adj[u]) {
        if (!visited[v]) {
            if (has_cycle_undirected_helper(adj, v, u, visited)) {
                return true;
            }
        } else if (v != parent) {
            // Visited non-parent neighbor indicates cycle
            return true;
        }
    }
    return false;
}

bool has_cycle_undirected(const std::vector<std::vector<int>>& adj) {
    int n = static_cast<int>(adj.size());
    std::vector<bool> visited(n, false);

    for (int i = 0; i < n; ++i) {
        if (!visited[i]) {
            if (has_cycle_undirected_helper(adj, i, -1, visited)) {
                return true;
            }
        }
    }
    return false;
}

// 5B. Cycle Detection in Directed Graph (3-Color State: 0=White, 1=Gray, 2=Black)
bool has_cycle_directed_helper(const std::vector<std::vector<int>>& adj, int u, std::vector<int>& color) {
    color[u] = 1; // Gray: currently exploring

    for (int v : adj[u]) {
        if (color[v] == 1) {
            return true; // Back-edge to ancestor in active recursion stack
        }
        if (color[v] == 0) {
            if (has_cycle_directed_helper(adj, v, color)) {
                return true;
            }
        }
    }

    color[u] = 2; // Black: fully explored
    return false;
}

bool has_cycle_directed(const std::vector<std::vector<int>>& adj) {
    int n = static_cast<int>(adj.size());
    std::vector<int> color(n, 0);

    for (int i = 0; i < n; ++i) {
        if (color[i] == 0) {
            if (has_cycle_directed_helper(adj, i, color)) {
                return true;
            }
        }
    }
    return false;
}

// 6. Connected Components Enumeration
std::vector<std::vector<int>> get_connected_components(const std::vector<std::vector<int>>& adj) {
    int n = static_cast<int>(adj.size());
    std::vector<bool> visited(n, false);
    std::vector<std::vector<int>> components;

    for (int i = 0; i < n; ++i) {
        if (!visited[i]) {
            std::vector<int> comp;
            std::queue<int> q;
            visited[i] = true;
            q.push(i);

            while (!q.empty()) {
                int u = q.front();
                q.pop();
                comp.push_back(u);

                for (int v : adj[u]) {
                    if (!visited[v]) {
                        visited[v] = true;
                        q.push(v);
                    }
                }
            }
            components.push_back(comp);
        }
    }

    return components;
}

// ============================================================================
// Unit Tests
// ============================================================================
int main() {
    // 1. BFS & Path Reconstruction Test
    // 0 - 1 - 3 - 5
    // |   |
    // 2 - 4
    std::vector<std::vector<int>> adj1(6);
    adj1[0] = {1, 2};
    adj1[1] = {0, 3, 4};
    adj1[2] = {0, 4};
    adj1[3] = {1, 5};
    adj1[4] = {1, 2};
    adj1[5] = {3};

    BFSResult res = bfs(adj1, 0);
    assert(res.dist[0] == 0);
    assert(res.dist[1] == 1);
    assert(res.dist[2] == 1);
    assert(res.dist[3] == 2);
    assert(res.dist[4] == 2);
    assert(res.dist[5] == 3);

    std::vector<int> path_to_5 = res.reconstruct_path(5);
    std::vector<int> expected_path = {0, 1, 3, 5};
    assert(path_to_5 == expected_path);

    // 2. Multi-Source BFS Test
    // Sources: {2, 5}
    std::vector<int> ms_dist = multi_source_bfs(adj1, {2, 5});
    assert(ms_dist[2] == 0);
    assert(ms_dist[5] == 0);
    assert(ms_dist[0] == 1); // 0 is 1 step from 2
    assert(ms_dist[3] == 1); // 3 is 1 step from 5

    // 3. 0-1 BFS Test
    std::vector<std::vector<Edge01>> adj01(4);
    adj01[0].push_back({1, 1});
    adj01[0].push_back({2, 0});
    adj01[2].push_back({3, 0});
    adj01[1].push_back({3, 1});

    std::vector<int> dist01 = zero_one_bfs(adj01, 0);
    assert(dist01[0] == 0);
    assert(dist01[2] == 0);
    assert(dist01[3] == 0); // 0 -> 2 -> 3 with total weight 0!
    assert(dist01[1] == 1);

    // 4. DFS Order & Match
    std::vector<int> dfs_rec = dfs_recursive(adj1, 0);
    std::vector<int> dfs_iter = dfs_iterative(adj1, 0);
    assert(dfs_rec.size() == 6);
    assert(dfs_iter.size() == 6);
    assert(dfs_rec == dfs_iter); // Both match with reverse neighbor pushing

    // 5. Cycle Detection Tests
    // 5A. Undirected Cycle: 0 - 1 - 2 - 0
    std::vector<std::vector<int>> cycle_undirected(3);
    cycle_undirected[0] = {1, 2};
    cycle_undirected[1] = {0, 2};
    cycle_undirected[2] = {0, 1};
    assert(has_cycle_undirected(cycle_undirected));

    // Undirected Tree (no cycle): 0 - 1 - 2
    std::vector<std::vector<int>> tree_undirected(3);
    tree_undirected[0] = {1};
    tree_undirected[1] = {0, 2};
    tree_undirected[2] = {1};
    assert(!has_cycle_undirected(tree_undirected));

    // 5B. Directed Cycle: 0 -> 1 -> 2 -> 0
    std::vector<std::vector<int>> cycle_directed(3);
    cycle_directed[0] = {1};
    cycle_directed[1] = {2};
    cycle_directed[2] = {0};
    assert(has_cycle_directed(cycle_directed));

    // Directed DAG (no cycle): 0 -> 1 -> 2
    std::vector<std::vector<int>> dag_directed(3);
    dag_directed[0] = {1};
    dag_directed[1] = {2};
    dag_directed[2] = {};
    assert(!has_cycle_directed(dag_directed));

    // 6. Connected Components
    // Component 1: {0, 1}, Component 2: {2}, Component 3: {3, 4}
    std::vector<std::vector<int>> disconnected(5);
    disconnected[0] = {1};
    disconnected[1] = {0};
    disconnected[2] = {};
    disconnected[3] = {4};
    disconnected[4] = {3};

    auto comps = get_connected_components(disconnected);
    assert(comps.size() == 3);

    std::cout << "[PASS] All Graph Traversals C++ unit tests passed.\n";
    return 0;
}
