#pragma once
#include <iostream>
#include <string>
#include "TreeNode.h"

class BST {
private:
    TreeNode* root;

    void destroyHelper(TreeNode* node);
    TreeNode* insertHelper(TreeNode* node, int key, bool& inserted);
    TreeNode* removeHelper(TreeNode* node, int key, bool& removed);
    void rangeHelper(TreeNode* node, int lo, int hi, int* arr, int& count) const;
    int countRangeHelper(TreeNode* node, int lo, int hi) const;
    int heightHelper(TreeNode* node) const;
    int sizeHelper(TreeNode* node) const;
    TreeNode* findMin(TreeNode* node) const;

public:
    BST();
    ~BST();

    // BST rule: smaller keys left, larger keys right, no duplicates
    bool insert(int key);
    TreeNode* search(int key);

    // Returns new int[] containing path from root to key (inclusive).
    // length is set. Returns nullptr if key not found.
    int* getPath(int key, int& length);

    // For insertion: returns path from root to future parent.
    int* getInsertPath(int key, int& length);

    // Remove key (leaf, 1 child, 2 children via inorder successor)
    bool remove(int key);

    // 0 = leaf, 1 = one child, 2 = two children, -1 = not found
    int deleteCase(int key);

    // Smallest node in right subtree
    TreeNode* inorderSuccessor(TreeNode* node);

    // Inorder range values in [lo, hi] populated into new int[]
    int* rangeToArray(int lo, int hi, int& count);

    int height();
    int size();
    TreeNode* getRoot() const;

    void clear();
};
