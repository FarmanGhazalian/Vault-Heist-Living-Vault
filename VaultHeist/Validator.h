#pragma once
#include "BinaryTree.h"
#include "BST.h"
#include "AVL.h"
#include "MaxHeap.h"

class Validator
{
public:
    static bool isBinaryTreeValid(TreeNode* root);
    static bool isBST(TreeNode* root, int minVal = -2147483647, int maxVal = 2147483647);
    static bool isAVL(TreeNode* root);
    static bool isHeap(const MaxHeap& heap);
    static int recomputeHeight(TreeNode* node);
    static int recomputeSize(TreeNode* node);
};
