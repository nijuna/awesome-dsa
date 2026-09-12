"""
Reference Implementation: Tree Dynamic Programming
Demonstrates:
1. Subtree Aggregation: Subtree sizes, heights, and depth sums via bottom-up post-order DFS.
2. Maximum Weight Independent Set (MWIS): Include/exclude DP O(V) and node set reconstruction.
3. Tree Diameter & Centers: Downward path combining O(V) and diameter witness path reconstruction.
4. All-Node Rerooting DP: Two-pass DFS computing sum of distances to all other nodes in O(V) time.

Language: Python 3
"""

import unittest
from typing import List, Tuple


# ============================================================================
# 1. Subtree Aggregations (Size and Height)
# ============================================================================

def compute_subtree_size_height(graph: List[List[int]], root: int = 0) -> Tuple[List[int], List[int]]:
    """
    Computes subtree size and subtree height for every node via post-order DFS.
    Time: O(V), Space: O(V).
    """
    n = len(graph)
    if n == 0:
        return [], []

    subtree_size = [0] * n
    height = [0] * n

    def dfs(u: int, parent: int):
        subtree_size[u] = 1
        height[u] = 0

        for v in graph[u]:
            if v == parent:
                continue
            dfs(v, u)
            subtree_size[u] += subtree_size[v]
            height[u] = max(height[u], height[v] + 1)

    dfs(root, -1)
    return subtree_size, height


# ============================================================================
# 2. Maximum Weight Independent Set (MWIS) on Trees
# ============================================================================

def maximum_weight_independent_set(graph: List[List[int]], weight: List[int], root: int = 0) -> int:
    """
    Computes MWIS cost: O(V) Time, O(V) Space.
    dp[u][1] = max weight in subtree Tu if u is included.
    dp[u][0] = max weight in subtree Tu if u is excluded.
    """
    n = len(graph)
    if n == 0:
        return 0

    dp = [[0, 0] for _ in range(n)]

    def dfs(u: int, parent: int):
        dp[u][1] = weight[u]
        dp[u][0] = 0

        for v in graph[u]:
            if v == parent:
                continue
            dfs(v, u)
            dp[u][1] += dp[v][0]
            dp[u][0] += max(dp[v][0], dp[v][1])

    dfs(root, -1)
    return max(dp[root][0], dp[root][1])


def maximum_weight_independent_set_reconstruct(
    graph: List[List[int]], weight: List[int], root: int = 0
) -> Tuple[int, List[int]]:
    """
    Computes MWIS and reconstructs the optimal set of node indices.
    Time: O(V) Time, O(V) Space.
    """
    n = len(graph)
    if n == 0:
        return 0, []

    dp = [[0, 0] for _ in range(n)]

    def dfs(u: int, parent: int):
        dp[u][1] = weight[u]
        dp[u][0] = 0

        for v in graph[u]:
            if v == parent:
                continue
            dfs(v, u)
            dp[u][1] += dp[v][0]
            dp[u][0] += max(dp[v][0], dp[v][1])

    dfs(root, -1)

    chosen = []

    def build(u: int, parent: int, parent_taken: bool):
        take_u = False
        if not parent_taken and dp[u][1] >= dp[u][0]:
            take_u = True
            chosen.append(u)

        for v in graph[u]:
            if v == parent:
                continue
            build(v, u, take_u)

    build(root, -1, False)
    chosen.sort()
    return max(dp[root][0], dp[root][1]), chosen


# ============================================================================
# 3. Tree Diameter and Centers
# ============================================================================

def tree_diameter(graph: List[List[int]], root: int = 0) -> int:
    """
    Computes tree diameter in number of edges using downward path DP.
    Time: O(V) Time, O(V) Space.
    """
    n = len(graph)
    if n <= 1:
        return 0

    diameter = 0

    def dfs(u: int, parent: int) -> int:
        nonlocal diameter
        best1 = 0
        best2 = 0

        for v in graph[u]:
            if v == parent:
                continue
            child_down = dfs(v, u) + 1

            if child_down > best1:
                best2 = best1
                best1 = child_down
            elif child_down > best2:
                best2 = child_down

        diameter = max(diameter, best1 + best2)
        return best1

    dfs(root, -1)
    return diameter


def tree_diameter_path(graph: List[List[int]], root: int = 0) -> List[int]:
    """
    Reconstructs a full diameter path (ordered list of nodes from endpoint to endpoint).
    Time: O(V) Time, O(V) Space.
    """
    n = len(graph)
    if n == 0:
        return []
    if n == 1:
        return [0]

    best_diameter = 0
    best_inflection = root
    branch1_child = -1
    branch2_child = -1

    longest_down = [0] * n
    best_child = [-1] * n

    def dfs(u: int, parent: int):
        nonlocal best_diameter, best_inflection, branch1_child, branch2_child
        b1, b2 = 0, 0
        c1, c2 = -1, -1

        for v in graph[u]:
            if v == parent:
                continue
            dfs(v, u)
            d = longest_down[v] + 1
            if d > b1:
                b2, c2 = b1, c1
                b1, c1 = d, v
            elif d > b2:
                b2, c2 = d, v

        longest_down[u] = b1
        best_child[u] = c1

        if b1 + b2 > best_diameter:
            best_diameter = b1 + b2
            best_inflection = u
            branch1_child = c1
            branch2_child = c2

    dfs(root, -1)

    path1 = []
    curr = branch1_child
    while curr != -1:
        path1.append(curr)
        curr = best_child[curr]
    path1.reverse()

    path2 = []
    curr = branch2_child
    while curr != -1:
        path2.append(curr)
        curr = best_child[curr]

    return path1 + [best_inflection] + path2


def tree_centers(graph: List[List[int]]) -> List[int]:
    """
    Finds the center node(s) of the tree (1 or 2 nodes minimizing max distance to all others).
    Center lies at the middle of the diameter path.
    """
    n = len(graph)
    if n == 0:
        return []
    if n == 1:
        return [0]

    path = tree_diameter_path(graph, 0)
    length = len(path)

    if length % 2 == 1:
        return [path[length // 2]]
    else:
        centers = [path[length // 2 - 1], path[length // 2]]
        centers.sort()
        return centers


# ============================================================================
# 4. All-Node Rerooting DP (Sum of Distances to All Nodes)
# ============================================================================

def sum_of_distances_all_nodes(graph: List[List[int]], root: int = 0) -> List[int]:
    """
    Computes sum of distances from every node u to all other nodes in O(V) time.
    Pass 1: Bottom-up post-order DFS computing subtree size and subtree downward distances.
    Pass 2: Top-down pre-order DFS computing all-node distance sums:
            ans[v] = ans[u] - size[v] + (n - size[v]).
    """
    n = len(graph)
    if n == 0:
        return []
    if n == 1:
        return [0]

    size = [0] * n
    down = [0] * n
    ans = [0] * n

    def dfs1(u: int, parent: int):
        size[u] = 1
        down[u] = 0

        for v in graph[u]:
            if v == parent:
                continue
            dfs1(v, u)
            size[u] += size[v]
            down[u] += down[v] + size[v]

    def dfs2(u: int, parent: int):
        for v in graph[u]:
            if v == parent:
                continue
            ans[v] = ans[u] - size[v] + (n - size[v])
            dfs2(v, u)

    dfs1(root, -1)
    ans[root] = down[root]
    dfs2(root, -1)

    return ans


# ============================================================================
# Unit Tests
# ============================================================================

class TestTreeDP(unittest.TestCase):
    def test_star_graph(self):
        star = [[1, 2, 3], [0], [0], [0]]

        sizes, heights = compute_subtree_size_height(star, 0)
        self.assertEqual(sizes[0], 4)
        self.assertEqual(sizes[1], 1)
        self.assertEqual(heights[0], 1)

        weights1 = [10, 5, 5, 5]
        self.assertEqual(maximum_weight_independent_set(star, weights1, 0), 15)
        w1, ch1 = maximum_weight_independent_set_reconstruct(star, weights1, 0)
        self.assertEqual(w1, 15)
        self.assertEqual(ch1, [1, 2, 3])

        weights2 = [20, 5, 5, 5]
        w2, ch2 = maximum_weight_independent_set_reconstruct(star, weights2, 0)
        self.assertEqual(w2, 20)
        self.assertEqual(ch2, [0])

        self.assertEqual(tree_diameter(star, 0), 2)
        path = tree_diameter_path(star, 0)
        self.assertEqual(len(path), 3)
        self.assertEqual(path[1], 0)

        centers = tree_centers(star)
        self.assertEqual(centers, [0])

        dist_sums = sum_of_distances_all_nodes(star, 0)
        self.assertEqual(dist_sums, [3, 5, 5, 5])

    def test_line_graph(self):
        n = 5
        line = [[] for _ in range(n)]
        for i in range(n - 1):
            line[i].append(i + 1)
            line[i + 1].append(i)

        self.assertEqual(tree_diameter(line, 0), 4)
        path = tree_diameter_path(line, 0)
        self.assertEqual(len(path), 5)

        centers = tree_centers(line)
        self.assertEqual(centers, [2])

        dist_sums = sum_of_distances_all_nodes(line, 0)
        self.assertEqual(dist_sums[2], 6)
        self.assertEqual(dist_sums[0], 10)
        self.assertEqual(dist_sums[4], 10)

    def test_even_line_graph(self):
        line4 = [[1], [0, 2], [1, 3], [2]]
        self.assertEqual(tree_diameter(line4, 0), 3)
        centers = tree_centers(line4)
        self.assertEqual(centers, [1, 2])

    def test_single_node(self):
        single = [[]]
        sizes, heights = compute_subtree_size_height(single, 0)
        self.assertEqual(sizes, [1])
        self.assertEqual(heights, [0])

        self.assertEqual(maximum_weight_independent_set(single, [42], 0), 42)
        w, ch = maximum_weight_independent_set_reconstruct(single, [42], 0)
        self.assertEqual(w, 42)
        self.assertEqual(ch, [0])

        self.assertEqual(tree_diameter(single, 0), 0)
        self.assertEqual(tree_centers(single), [0])
        self.assertEqual(sum_of_distances_all_nodes(single, 0), [0])


if __name__ == '__main__':
    unittest.main()
