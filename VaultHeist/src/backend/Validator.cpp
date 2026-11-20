#include "backend/Validator.h"
#include <cmath>
#include <algorithm>

bool Validator::isBinaryTreeValid(TreeNode* root)
{
    if (!root) return true;
    return isBinaryTreeValid(root->left) && isBinaryTreeValid(root->right);
}

bool Validator::isBST(TreeNode* root, int minVal, int maxVal)
{
    if (!root) return true;
    if (root->val <= minVal || root->val >= maxVal) return false;
    return isBST(root->left, minVal, root->val) && isBST(root->right, root->val, maxVal);
}

int Validator::recomputeHeight(TreeNode* node)
{
    if (!node) return 0;
    return 1 + std::max(recomputeHeight(node->left), recomputeHeight(node->right));
}

int Validator::recomputeSize(TreeNode* node)
{
    if (!node) return 0;
    return 1 + recomputeSize(node->left) + recomputeSize(node->right);
}

bool Validator::isAVL(TreeNode* root)
{
    if (!root) return true;
    if (!isBST(root)) return false;

    int lh = recomputeHeight(root->left);
    int rh = recomputeHeight(root->right);
    if (std::abs(lh - rh) > 1) return false;

    if (root->height != 1 + std::max(lh, rh)) return false;

    return isAVL(root->left) && isAVL(root->right);
}

bool Validator::isHeap(const MaxHeap& heap)
{
    return heap.isMaxHeap();
}
