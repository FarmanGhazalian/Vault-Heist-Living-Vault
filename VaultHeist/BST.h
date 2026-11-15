#pragma once
#include "BinaryTree.h"

class BST : public BinaryTree
{
private:
	TreeNode* insertHelper(TreeNode* node, int key, bool& inserted);
	TreeNode* removeHelper(TreeNode* node, int key, bool& removed);
	void rangeHelper(TreeNode* node, int lo, int hi, int* arr, int& count) const;
	int countRangeHelper(TreeNode* node, int lo, int hi) const;
	TreeNode* findMin(TreeNode* node) const;

public:
	BST() : BinaryTree() {}
	~BST() {}

	bool insert(int key);
	bool remove(int key);
	int deleteCase(int key);
	TreeNode* inorderSuccessor(TreeNode* node);
	int* rangeToArray(int lo, int hi, int& count);
	int* getInsertPath(int key, int& length);
};
