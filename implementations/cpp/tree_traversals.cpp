#include <cassert>
#include <iostream>
#include <queue>
#include <stack>
#include <vector>
#include <algorithm>

namespace dsa {

/**
 * @brief Binary tree node structure.
 */
struct TreeNode {
    int val;
    TreeNode* left;
    TreeNode* right;

    explicit TreeNode(int v, TreeNode* l = nullptr, TreeNode* r = nullptr)
        : val(v), left(l), right(r) {}
};

/**
 * @brief N-ary tree node structure.
 */
struct NaryNode {
    int val;
    std::vector<NaryNode*> children;

    explicit NaryNode(int v) : val(v) {}
    explicit NaryNode(int v, std::vector<NaryNode*> ch) : val(v), children(std::move(ch)) {}
};

inline void free_tree(TreeNode* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    delete root;
}

inline void free_nary_tree(NaryNode* root) {
    if (!root) return;
    for (auto* child : root->children) {
        free_nary_tree(child);
    }
    delete root;
}

// ==========================================
// Binary Tree Recursive Traversals
// ==========================================

inline void preorder_recursive(const TreeNode* root, std::vector<int>& out) {
    if (!root) return;
    out.push_back(root->val);
    preorder_recursive(root->left, out);
    preorder_recursive(root->right, out);
}

inline void inorder_recursive(const TreeNode* root, std::vector<int>& out) {
    if (!root) return;
    inorder_recursive(root->left, out);
    out.push_back(root->val);
    inorder_recursive(root->right, out);
}

inline void postorder_recursive(const TreeNode* root, std::vector<int>& out) {
    if (!root) return;
    postorder_recursive(root->left, out);
    postorder_recursive(root->right, out);
    out.push_back(root->val);
}

// ==========================================
// Binary Tree Iterative Traversals
// ==========================================

inline std::vector<int> preorder_iterative(const TreeNode* root) {
    std::vector<int> out;
    if (!root) return out;
    std::stack<const TreeNode*> st;
    st.push(root);

    while (!st.empty()) {
        const TreeNode* curr = st.top();
        st.pop();
        out.push_back(curr->val);

        // Push right first so left is processed first
        if (curr->right) st.push(curr->right);
        if (curr->left) st.push(curr->left);
    }
    return out;
}

inline std::vector<int> inorder_iterative(const TreeNode* root) {
    std::vector<int> out;
    std::stack<const TreeNode*> st;
    const TreeNode* curr = root;

    while (curr != nullptr || !st.empty()) {
        while (curr != nullptr) {
            st.push(curr);
            curr = curr->left;
        }
        curr = st.top();
        st.pop();
        out.push_back(curr->val);
        curr = curr->right;
    }
    return out;
}

inline std::vector<int> postorder_iterative(const TreeNode* root) {
    std::vector<int> out;
    if (!root) return out;

    std::stack<const TreeNode*> st1;
    std::stack<const TreeNode*> st2;
    st1.push(root);

    while (!st1.empty()) {
        const TreeNode* curr = st1.top();
        st1.pop();
        st2.push(curr);

        if (curr->left) st1.push(curr->left);
        if (curr->right) st1.push(curr->right);
    }

    while (!st2.empty()) {
        out.push_back(st2.top()->val);
        st2.pop();
    }
    return out;
}

// ==========================================
// Level-Order (BFS) Traversals
// ==========================================

inline std::vector<int> level_order(const TreeNode* root) {
    std::vector<int> out;
    if (!root) return out;
    std::queue<const TreeNode*> q;
    q.push(root);

    while (!q.empty()) {
        const TreeNode* curr = q.front();
        q.pop();
        out.push_back(curr->val);

        if (curr->left) q.push(curr->left);
        if (curr->right) q.push(curr->right);
    }
    return out;
}

inline std::vector<std::vector<int>> level_order_by_levels(const TreeNode* root) {
    std::vector<std::vector<int>> levels;
    if (!root) return levels;
    std::queue<const TreeNode*> q;
    q.push(root);

    while (!q.empty()) {
        size_t level_size = q.size();
        std::vector<int> current_level;
        current_level.reserve(level_size);

        for (size_t i = 0; i < level_size; ++i) {
            const TreeNode* curr = q.front();
            q.pop();
            current_level.push_back(curr->val);
            if (curr->left) q.push(curr->left);
            if (curr->right) q.push(curr->right);
        }
        levels.push_back(std::move(current_level));
    }
    return levels;
}

// ==========================================
// Metrics: Height and Size
// ==========================================

inline int tree_height(const TreeNode* root) {
    if (!root) return -1; // Standard: height of empty is -1, leaf is 0
    return 1 + std::max(tree_height(root->left), tree_height(root->right));
}

inline int tree_size(const TreeNode* root) {
    if (!root) return 0;
    return 1 + tree_size(root->left) + tree_size(root->right);
}

// ==========================================
// N-ary Tree Traversals
// ==========================================

inline void nary_preorder_rec(const NaryNode* root, std::vector<int>& out) {
    if (!root) return;
    out.push_back(root->val);
    for (const auto* child : root->children) {
        nary_preorder_rec(child, out);
    }
}

inline void nary_postorder_rec(const NaryNode* root, std::vector<int>& out) {
    if (!root) return;
    for (const auto* child : root->children) {
        nary_postorder_rec(child, out);
    }
    out.push_back(root->val);
}

inline std::vector<int> nary_level_order(const NaryNode* root) {
    std::vector<int> out;
    if (!root) return out;
    std::queue<const NaryNode*> q;
    q.push(root);

    while (!q.empty()) {
        const NaryNode* curr = q.front();
        q.pop();
        out.push_back(curr->val);
        for (const auto* child : curr->children) {
            if (child) q.push(child);
        }
    }
    return out;
}

} // namespace dsa

int main() {
    using namespace dsa;

    std::cout << "[RUNNING] Tree Basics and Traversals C++17 Verification..." << std::endl;

    // 1. Test Empty Tree
    {
        TreeNode* empty = nullptr;
        std::vector<int> res;
        preorder_recursive(empty, res);
        assert(res.empty());
        assert(preorder_iterative(empty).empty());
        assert(inorder_iterative(empty).empty());
        assert(postorder_iterative(empty).empty());
        assert(level_order(empty).empty());
        assert(level_order_by_levels(empty).empty());
        assert(tree_height(empty) == -1);
        assert(tree_size(empty) == 0);
    }

    // 2. Test Single Node Tree
    {
        TreeNode* root = new TreeNode(42);
        std::vector<int> res;
        preorder_recursive(root, res);
        assert((res == std::vector<int>{42}));
        assert((preorder_iterative(root) == std::vector<int>{42}));
        assert((inorder_iterative(root) == std::vector<int>{42}));
        assert((postorder_iterative(root) == std::vector<int>{42}));
        assert((level_order(root) == std::vector<int>{42}));
        assert(tree_height(root) == 0);
        assert(tree_size(root) == 1);
        free_tree(root);
    }

    // 3. Test Balanced Binary Tree:
    //         1
    //       /       //      2     3
    //     / \   /     //    4   5 6   7
    {
        TreeNode* root = new TreeNode(1,
            new TreeNode(2, new TreeNode(4), new TreeNode(5)),
            new TreeNode(3, new TreeNode(6), new TreeNode(7))
        );

        assert(tree_height(root) == 2);
        assert(tree_size(root) == 7);

        // Preorder: 1, 2, 4, 5, 3, 6, 7
        std::vector<int> pre_rec;
        preorder_recursive(root, pre_rec);
        std::vector<int> pre_iter = preorder_iterative(root);
        std::vector<int> pre_exp = {1, 2, 4, 5, 3, 6, 7};
        assert(pre_rec == pre_exp);
        assert(pre_iter == pre_exp);

        // Inorder: 4, 2, 5, 1, 6, 3, 7
        std::vector<int> in_rec;
        inorder_recursive(root, in_rec);
        std::vector<int> in_iter = inorder_iterative(root);
        std::vector<int> in_exp = {4, 2, 5, 1, 6, 3, 7};
        assert(in_rec == in_exp);
        assert(in_iter == in_exp);

        // Postorder: 4, 5, 2, 6, 7, 3, 1
        std::vector<int> post_rec;
        postorder_recursive(root, post_rec);
        std::vector<int> post_iter = postorder_iterative(root);
        std::vector<int> post_exp = {4, 5, 2, 6, 7, 3, 1};
        assert(post_rec == post_exp);
        assert(post_iter == post_exp);

        // Level-order
        std::vector<int> bfs = level_order(root);
        std::vector<int> bfs_exp = {1, 2, 3, 4, 5, 6, 7};
        assert(bfs == bfs_exp);

        // Level-order grouped
        auto levels = level_order_by_levels(root);
        std::vector<std::vector<int>> levels_exp = {
            {1},
            {2, 3},
            {4, 5, 6, 7}
        };
        assert(levels == levels_exp);

        free_tree(root);
    }

    // 4. Test Skewed Tree (Left-skewed: 3 -> 2 -> 1)
    {
        TreeNode* root = new TreeNode(3, new TreeNode(2, new TreeNode(1), nullptr), nullptr);
        assert(tree_height(root) == 2);
        assert(tree_size(root) == 3);

        std::vector<int> in_rec;
        inorder_recursive(root, in_rec);
        assert((in_rec == std::vector<int>{1, 2, 3}));
        assert(inorder_iterative(root) == in_rec);
        assert((preorder_iterative(root) == std::vector<int>{3, 2, 1}));
        assert((postorder_iterative(root) == std::vector<int>{1, 2, 3}));

        free_tree(root);
    }

    // 5. Test N-ary Tree
    //         1
    //      /  |      //     2   3   4
    //    /     //   5   6
    {
        NaryNode* n5 = new NaryNode(5);
        NaryNode* n6 = new NaryNode(6);
        NaryNode* n2 = new NaryNode(2, {n5, n6});
        NaryNode* n3 = new NaryNode(3);
        NaryNode* n4 = new NaryNode(4);
        NaryNode* root = new NaryNode(1, {n2, n3, n4});

        std::vector<int> pre;
        nary_preorder_rec(root, pre);
        assert((pre == std::vector<int>{1, 2, 5, 6, 3, 4}));

        std::vector<int> post;
        nary_postorder_rec(root, post);
        assert((post == std::vector<int>{5, 6, 2, 3, 4, 1}));

        std::vector<int> bfs = nary_level_order(root);
        assert((bfs == std::vector<int>{1, 2, 3, 4, 5, 6}));

        free_nary_tree(root);
    }

    std::cout << "[PASSED] Tree Basics and Traversals C++17 All Tests Passed!" << std::endl;
    return 0;
}
