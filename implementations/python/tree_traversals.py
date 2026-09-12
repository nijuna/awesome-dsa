"""
Tree Basics and Traversals implementation in Python.

Provides:
1. Binary Tree (TreeNode) & N-ary Tree (NaryNode) representations.
2. Recursive Depth-First Traversals (Pre-order, In-order, Post-order).
3. Iterative Depth-First Traversals (Pre-order, In-order, Post-order with two stacks).
4. Breadth-First / Level-Order Traversals (flat and grouped by level).
5. N-ary Tree traversals.
6. Tree metrics: height and size.
7. Comprehensive unit tests.
"""

import unittest
from collections import deque
from typing import Optional, List, Any


class TreeNode:
    """Binary Tree Node."""
    def __init__(self, val: int = 0, left: Optional['TreeNode'] = None, right: Optional['TreeNode'] = None):
        self.val = val
        self.left = left
        self.right = right


class NaryNode:
    """N-ary Tree Node."""
    def __init__(self, val: int = 0, children: Optional[List['NaryNode']] = None):
        self.val = val
        self.children = children if children is not None else []


# ==========================================
# Binary Tree Recursive Traversals
# ==========================================

def preorder_recursive(root: Optional[TreeNode]) -> List[int]:
    out: List[int] = []

    def dfs(node: Optional[TreeNode]) -> None:
        if not node:
            return
        out.append(node.val)
        dfs(node.left)
        dfs(node.right)

    dfs(root)
    return out


def inorder_recursive(root: Optional[TreeNode]) -> List[int]:
    out: List[int] = []

    def dfs(node: Optional[TreeNode]) -> None:
        if not node:
            return
        dfs(node.left)
        out.append(node.val)
        dfs(node.right)

    dfs(root)
    return out


def postorder_recursive(root: Optional[TreeNode]) -> List[int]:
    out: List[int] = []

    def dfs(node: Optional[TreeNode]) -> None:
        if not node:
            return
        dfs(node.left)
        dfs(node.right)
        out.append(node.val)

    dfs(root)
    return out


# ==========================================
# Binary Tree Iterative Traversals
# ==========================================

def preorder_iterative(root: Optional[TreeNode]) -> List[int]:
    if not root:
        return []
    out: List[int] = []
    stack = [root]
    while stack:
        curr = stack.pop()
        out.append(curr.val)
        if curr.right:
            stack.append(curr.right)
        if curr.left:
            stack.append(curr.left)
    return out


def inorder_iterative(root: Optional[TreeNode]) -> List[int]:
    out: List[int] = []
    stack = []
    curr = root
    while curr is not None or stack:
        while curr is not None:
            stack.append(curr)
            curr = curr.left
        curr = stack.pop()
        out.append(curr.val)
        curr = curr.right
    return out


def postorder_iterative(root: Optional[TreeNode]) -> List[int]:
    if not root:
        return []
    st1 = [root]
    st2 = []
    while st1:
        curr = st1.pop()
        st2.append(curr)
        if curr.left:
            st1.append(curr.left)
        if curr.right:
            st1.append(curr.right)
    return [node.val for node in reversed(st2)]


# ==========================================
# Level-Order (BFS) Traversals
# ==========================================

def level_order(root: Optional[TreeNode]) -> List[int]:
    if not root:
        return []
    out: List[int] = []
    queue = deque([root])
    while queue:
        curr = queue.popleft()
        out.append(curr.val)
        if curr.left:
            queue.append(curr.left)
        if curr.right:
            queue.append(curr.right)
    return out


def level_order_by_levels(root: Optional[TreeNode]) -> List[List[int]]:
    if not root:
        return []
    levels: List[List[int]] = []
    queue = deque([root])
    while queue:
        level_len = len(queue)
        curr_level = []
        for _ in range(level_len):
            node = queue.popleft()
            curr_level.append(node.val)
            if node.left:
                queue.append(node.left)
            if node.right:
                queue.append(node.right)
        levels.append(curr_level)
    return levels


# ==========================================
# Tree Metrics
# ==========================================

def tree_height(root: Optional[TreeNode]) -> int:
    """Height of tree: empty = -1, single node = 0."""
    if not root:
        return -1
    return 1 + max(tree_height(root.left), tree_height(root.right))


def tree_size(root: Optional[TreeNode]) -> int:
    """Total number of nodes in tree."""
    if not root:
        return 0
    return 1 + tree_size(root.left) + tree_size(root.right)


# ==========================================
# N-ary Tree Traversals
# ==========================================

def nary_preorder(root: Optional[NaryNode]) -> List[int]:
    out: List[int] = []

    def dfs(node: Optional[NaryNode]) -> None:
        if not node:
            return
        out.append(node.val)
        for ch in node.children:
            dfs(ch)

    dfs(root)
    return out


def nary_postorder(root: Optional[NaryNode]) -> List[int]:
    out: List[int] = []

    def dfs(node: Optional[NaryNode]) -> None:
        if not node:
            return
        for ch in node.children:
            dfs(ch)
        out.append(node.val)

    dfs(root)
    return out


def nary_level_order(root: Optional[NaryNode]) -> List[int]:
    if not root:
        return []
    out: List[int] = []
    queue = deque([root])
    while queue:
        node = queue.popleft()
        out.append(node.val)
        for ch in node.children:
            if ch:
                queue.append(ch)
    return out


class TestTreeTraversals(unittest.TestCase):
    def test_empty_tree(self):
        empty = None
        self.assertEqual(preorder_recursive(empty), [])
        self.assertEqual(inorder_recursive(empty), [])
        self.assertEqual(postorder_recursive(empty), [])
        self.assertEqual(preorder_iterative(empty), [])
        self.assertEqual(inorder_iterative(empty), [])
        self.assertEqual(postorder_iterative(empty), [])
        self.assertEqual(level_order(empty), [])
        self.assertEqual(level_order_by_levels(empty), [])
        self.assertEqual(tree_height(empty), -1)
        self.assertEqual(tree_size(empty), 0)

    def test_single_node(self):
        root = TreeNode(42)
        self.assertEqual(preorder_recursive(root), [42])
        self.assertEqual(preorder_iterative(root), [42])
        self.assertEqual(inorder_recursive(root), [42])
        self.assertEqual(inorder_iterative(root), [42])
        self.assertEqual(postorder_recursive(root), [42])
        self.assertEqual(postorder_iterative(root), [42])
        self.assertEqual(level_order(root), [42])
        self.assertEqual(tree_height(root), 0)
        self.assertEqual(tree_size(root), 1)

    def test_balanced_binary_tree(self):
        #         1
        #       /           #      2     3
        #     / \   /         #    4   5 6   7
        root = TreeNode(1,
            TreeNode(2, TreeNode(4), TreeNode(5)),
            TreeNode(3, TreeNode(6), TreeNode(7))
        )
        self.assertEqual(tree_height(root), 2)
        self.assertEqual(tree_size(root), 7)

        expected_pre = [1, 2, 4, 5, 3, 6, 7]
        self.assertEqual(preorder_recursive(root), expected_pre)
        self.assertEqual(preorder_iterative(root), expected_pre)

        expected_in = [4, 2, 5, 1, 6, 3, 7]
        self.assertEqual(inorder_recursive(root), expected_in)
        self.assertEqual(inorder_iterative(root), expected_in)

        expected_post = [4, 5, 2, 6, 7, 3, 1]
        self.assertEqual(postorder_recursive(root), expected_post)
        self.assertEqual(postorder_iterative(root), expected_post)

        expected_bfs = [1, 2, 3, 4, 5, 6, 7]
        self.assertEqual(level_order(root), expected_bfs)
        self.assertEqual(level_order_by_levels(root), [[1], [2, 3], [4, 5, 6, 7]])

    def test_skewed_tree(self):
        # 3 -> 2 -> 1 (left-skewed)
        root = TreeNode(3, TreeNode(2, TreeNode(1)))
        self.assertEqual(tree_height(root), 2)
        self.assertEqual(tree_size(root), 3)
        self.assertEqual(inorder_recursive(root), [1, 2, 3])
        self.assertEqual(inorder_iterative(root), [1, 2, 3])
        self.assertEqual(preorder_iterative(root), [3, 2, 1])
        self.assertEqual(postorder_iterative(root), [1, 2, 3])

    def test_nary_tree(self):
        #         1
        #      /  |          #     2   3   4
        #    /         #   5   6
        root = NaryNode(1, [
            NaryNode(2, [NaryNode(5), NaryNode(6)]),
            NaryNode(3),
            NaryNode(4)
        ])
        self.assertEqual(nary_preorder(root), [1, 2, 5, 6, 3, 4])
        self.assertEqual(nary_postorder(root), [5, 6, 2, 3, 4, 1])
        self.assertEqual(nary_level_order(root), [1, 2, 3, 4, 5, 6])


if __name__ == '__main__':
    unittest.main()
