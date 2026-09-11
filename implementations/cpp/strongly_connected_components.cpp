/**
 * Reference Implementation: Strongly Connected Components (SCC) & Condensation Graphs
 * Demonstrates:
 * 1. Kosaraju's 2-Pass DFS Algorithm (Finishing Order + Transpose Graph).
 * 2. Tarjan's 1-Pass DFS Algorithm (Discovery Times + Low-Link Values + Active Stack).
 * 3. Condensation DAG Construction (Deduplicated Component Meta-Graph).
 * 4. 2-Satisfiability (2-SAT) Solver via Implication Graph and SCC Decomposition.
 *
 * Language: C++17
 */

#include <iostream>
#include <vector>
#include <stack>
#include <algorithm>
#include <set>
#include <functional>
#include <cassert>

// ============================================================================
// 1. Kosaraju's Algorithm (2-Pass DFS via Transpose Graph)
// ============================================================================
std::vector<std::vector<int>> strongly_connected_components_kosaraju(
    const std::vector<std::vector<int>>& graph) {
    int n = static_cast<int>(graph.size());
    std::vector<int> visited(n, 0);
    std::vector<int> order;
    order.reserve(n);

    // Pass 1: Forward DFS to record finish order (postorder)
    std::function<void(int)> dfs1 = [&](int u) {
        visited[u] = 1;
        for (int v : graph[u]) {
            if (!visited[v]) {
                dfs1(v);
            }
        }
        order.push_back(u);
    };

    for (int u = 0; u < n; ++u) {
        if (!visited[u]) {
            dfs1(u);
        }
    }

    // Build transpose graph (reversed edges)
    std::vector<std::vector<int>> transpose(n);
    for (int u = 0; u < n; ++u) {
        for (int v : graph[u]) {
            transpose[v].push_back(u);
        }
    }

    // Pass 2: DFS on transpose in decreasing order of finish times
    std::fill(visited.begin(), visited.end(), 0);
    std::reverse(order.begin(), order.end());

    std::vector<std::vector<int>> components;

    std::function<void(int, std::vector<int>&)> dfs2 = [&](int u, std::vector<int>& comp) {
        visited[u] = 1;
        comp.push_back(u);
        for (int v : transpose[u]) {
            if (!visited[v]) {
                dfs2(v, comp);
            }
        }
    };

    for (int u : order) {
        if (!visited[u]) {
            std::vector<int> comp;
            dfs2(u, comp);
            components.push_back(std::move(comp));
        }
    }

    return components;
}

// ============================================================================
// 2. Tarjan's Algorithm (1-Pass DFS via Discovery Times, Low-Link, & Stack)
// ============================================================================
std::vector<std::vector<int>> strongly_connected_components_tarjan(
    const std::vector<std::vector<int>>& graph) {
    int n = static_cast<int>(graph.size());
    std::vector<int> disc(n, -1);
    std::vector<int> low(n, -1);
    std::vector<int> on_stack(n, 0);
    std::stack<int> st;
    std::vector<std::vector<int>> components;
    int timer = 0;

    std::function<void(int)> dfs = [&](int u) {
        disc[u] = low[u] = timer++;
        st.push(u);
        on_stack[u] = 1;

        for (int v : graph[u]) {
            if (disc[v] == -1) {
                // Tree edge: explore neighbor recursively
                dfs(v);
                low[u] = std::min(low[u], low[v]);
            } else if (on_stack[v]) {
                // Back edge to a vertex currently in the active DFS recursion stack
                low[u] = std::min(low[u], disc[v]);
            }
        }

        // u is the root of an SCC: pop vertices down to u
        if (low[u] == disc[u]) {
            std::vector<int> comp;
            while (true) {
                int v = st.top();
                st.pop();
                on_stack[v] = 0;
                comp.push_back(v);
                if (v == u) {
                    break;
                }
            }
            components.push_back(std::move(comp));
        }
    };

    for (int u = 0; u < n; ++u) {
        if (disc[u] == -1) {
            dfs(u);
        }
    }

    return components;
}

// ============================================================================
// 3. Component ID Mapping & Condensation DAG Construction
// ============================================================================
std::vector<int> make_component_id(
    int n,
    const std::vector<std::vector<int>>& components) {
    std::vector<int> component_id(n, -1);
    for (int cid = 0; cid < static_cast<int>(components.size()); ++cid) {
        for (int u : components[cid]) {
            component_id[u] = cid;
        }
    }
    return component_id;
}

std::vector<std::vector<int>> build_condensation_dag(
    const std::vector<std::vector<int>>& graph,
    const std::vector<int>& component_id,
    int component_count) {
    std::vector<std::set<int>> dag_set(component_count);

    for (int u = 0; u < static_cast<int>(graph.size()); ++u) {
        for (int v : graph[u]) {
            int cu = component_id[u];
            int cv = component_id[v];
            if (cu != cv) {
                dag_set[cu].insert(cv);
            }
        }
    }

    std::vector<std::vector<int>> dag(component_count);
    for (int c = 0; c < component_count; ++c) {
        dag[c] = std::vector<int>(dag_set[c].begin(), dag_set[c].end());
    }

    return dag;
}

// ============================================================================
// 4. 2-Satisfiability (2-SAT) Solver
// ============================================================================
class TwoSatSolver {
public:
    int num_vars;
    std::vector<std::vector<int>> adj;

    explicit TwoSatSolver(int vars) : num_vars(vars), adj(2 * vars) {}

    // Adds clause: (u == val_u) OR (v == val_v)
    void add_clause(int u, bool val_u, int v, bool val_v) {
        int lit_u = 2 * u + (val_u ? 0 : 1);
        int lit_v = 2 * v + (val_v ? 0 : 1);
        int neg_u = lit_u ^ 1;
        int neg_v = lit_v ^ 1;
        // ~lit_u => lit_v, ~lit_v => lit_u
        adj[neg_u].push_back(lit_v);
        adj[neg_v].push_back(lit_u);
    }

    // Solves 2-SAT instance. Returns true if satisfiable, and fills assignment.
    bool solve(std::vector<bool>& assignment) {
        assignment.assign(num_vars, false);
        std::vector<std::vector<int>> sccs = strongly_connected_components_tarjan(adj);
        std::vector<int> comp_id = make_component_id(2 * num_vars, sccs);

        for (int i = 0; i < num_vars; ++i) {
            if (comp_id[2 * i] == comp_id[2 * i + 1]) {
                return false; // Contradiction: x and ~x belong to the same SCC
            }
            // Tarjan produces components in reverse topological order.
            // If comp_id[2*i] < comp_id[2*i+1], literal x is finished earlier in reverse topological order,
            // which corresponds to x being topologically later (or reachable from ~x), so assign true.
            assignment[i] = (comp_id[2 * i] < comp_id[2 * i + 1]);
        }
        return true;
    }
};

// ============================================================================
// Unit Tests & Verification
// ============================================================================
static bool are_same_partition(int n,
                               const std::vector<std::vector<int>>& a,
                               const std::vector<std::vector<int>>& b) {
    std::vector<int> id_a = make_component_id(n, a);
    std::vector<int> id_b = make_component_id(n, b);

    // Two vertices u, v must be in the same component in a iff they are in the same component in b
    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if ((id_a[u] == id_a[v]) != (id_b[u] == id_b[v])) {
                return false;
            }
        }
    }
    return true;
}

int main() {
    // ------------------------------------------------------------------------
    // Test 1: Standard 2-SCC Graph
    // 0 -> 1 -> 2 -> 0; 2 -> 3; 3 -> 4 -> 5 -> 3
    // ------------------------------------------------------------------------
    {
        std::vector<std::vector<int>> g(6);
        g[0] = {1};
        g[1] = {2};
        g[2] = {0, 3};
        g[3] = {4};
        g[4] = {5};
        g[5] = {3};

        auto scc_k = strongly_connected_components_kosaraju(g);
        auto scc_t = strongly_connected_components_tarjan(g);

        assert(scc_k.size() == 2);
        assert(scc_t.size() == 2);
        assert(are_same_partition(6, scc_k, scc_t));

        std::vector<int> comp_id = make_component_id(6, scc_t);
        assert(comp_id[0] == comp_id[1] && comp_id[1] == comp_id[2]);
        assert(comp_id[3] == comp_id[4] && comp_id[4] == comp_id[5]);
        assert(comp_id[0] != comp_id[3]);

        auto dag = build_condensation_dag(g, comp_id, 2);
        assert(dag.size() == 2);
        // The condensation DAG must have exactly one directed edge from {0,1,2} to {3,4,5}
        int c_source = comp_id[0];
        int c_sink = comp_id[3];
        assert(dag[c_source].size() == 1 && dag[c_source][0] == c_sink);
        assert(dag[c_sink].empty());
    }

    // ------------------------------------------------------------------------
    // Test 2: Linear DAG (Each vertex is its own SCC)
    // 0 -> 1 -> 2 -> 3
    // ------------------------------------------------------------------------
    {
        std::vector<std::vector<int>> g(4);
        g[0] = {1};
        g[1] = {2};
        g[2] = {3};

        auto scc_k = strongly_connected_components_kosaraju(g);
        auto scc_t = strongly_connected_components_tarjan(g);

        assert(scc_k.size() == 4);
        assert(scc_t.size() == 4);
        assert(are_same_partition(4, scc_k, scc_t));
    }

    // ------------------------------------------------------------------------
    // Test 3: Complete Directed Cycle (All vertices form 1 single SCC)
    // 0 -> 1 -> 2 -> 3 -> 4 -> 0
    // ------------------------------------------------------------------------
    {
        std::vector<std::vector<int>> g(5);
        g[0] = {1};
        g[1] = {2};
        g[2] = {3};
        g[3] = {4};
        g[4] = {0};

        auto scc_k = strongly_connected_components_kosaraju(g);
        auto scc_t = strongly_connected_components_tarjan(g);

        assert(scc_k.size() == 1);
        assert(scc_t.size() == 1);
        assert(scc_k[0].size() == 5);
        assert(scc_t[0].size() == 5);
        assert(are_same_partition(5, scc_k, scc_t));
    }

    // ------------------------------------------------------------------------
    // Test 4: Disconnected Graph with Multiple Isolated Components
    // 0 -> 1 -> 0, 2 (isolated), 3 -> 4, 4 -> 5, 5 -> 3
    // ------------------------------------------------------------------------
    {
        std::vector<std::vector<int>> g(6);
        g[0] = {1};
        g[1] = {0};
        g[3] = {4};
        g[4] = {5};
        g[5] = {3};

        auto scc_k = strongly_connected_components_kosaraju(g);
        auto scc_t = strongly_connected_components_tarjan(g);

        assert(scc_k.size() == 3);
        assert(scc_t.size() == 3);
        assert(are_same_partition(6, scc_k, scc_t));
    }

    // ------------------------------------------------------------------------
    // Test 5: 2-SAT Satisfiable Instance
    // (x0 or x1) and (~x0 or x1) and (x0 or ~x1)
    // ------------------------------------------------------------------------
    {
        TwoSatSolver solver(2);
        solver.add_clause(0, true, 1, true);   // (x0 or x1)
        solver.add_clause(0, false, 1, true);  // (~x0 or x1)
        solver.add_clause(0, true, 1, false);  // (x0 or ~x1)

        std::vector<bool> assignment;
        bool sat = solver.solve(assignment);
        assert(sat);
        assert(assignment.size() == 2);
        // Verify clauses
        assert(assignment[0] || assignment[1]);
        assert(!assignment[0] || assignment[1]);
        assert(assignment[0] || !assignment[1]);
        assert(assignment[0] == true && assignment[1] == true);
    }

    // ------------------------------------------------------------------------
    // Test 6: 2-SAT Unsatisfiable Contradiction
    // (x0 or x0) and (~x0 or ~x0) => x0 must be both true and false
    // ------------------------------------------------------------------------
    {
        TwoSatSolver solver(1);
        solver.add_clause(0, true, 0, true);
        solver.add_clause(0, false, 0, false);

        std::vector<bool> assignment;
        bool sat = solver.solve(assignment);
        assert(!sat);
    }

    // ------------------------------------------------------------------------
    // Test 7: 2-SAT XOR + XNOR Contradiction
    // (x0 ^ x1) AND (x0 == x1)
    // ------------------------------------------------------------------------
    {
        TwoSatSolver solver(2);
        // XOR: (x0 or x1) and (~x0 or ~x1)
        solver.add_clause(0, true, 1, true);
        solver.add_clause(0, false, 1, false);
        // XNOR: (~x0 or x1) and (x0 or ~x1)
        solver.add_clause(0, false, 1, true);
        solver.add_clause(0, true, 1, false);

        std::vector<bool> assignment;
        bool sat = solver.solve(assignment);
        assert(!sat);
    }

    // ------------------------------------------------------------------------
    // Test 8: 2-SAT 3-Variable Complex System
    // (x0 or x1), (~x1 or x2), (~x2 or ~x0), (x0 or ~x2)
    // ------------------------------------------------------------------------
    {
        TwoSatSolver solver(3);
        solver.add_clause(0, true, 1, true);
        solver.add_clause(1, false, 2, true);
        solver.add_clause(2, false, 0, false);
        solver.add_clause(0, true, 2, false);

        std::vector<bool> assignment;
        bool sat = solver.solve(assignment);
        assert(sat);
        // Verify all 4 clauses
        assert(assignment[0] || assignment[1]);
        assert(!assignment[1] || assignment[2]);
        assert(!assignment[2] || !assignment[0]);
        assert(assignment[0] || !assignment[2]);
    }

    std::cout << "[PASS] All Strongly Connected Components C++ unit tests passed.\n";
    return 0;
}
