#pragma once
#include "backend/BST.h"

class AVL : public BST
{
private:
	int max(int a, int b);
	void countUnbalancedHelper(TreeNode* node, int& count);
	void getUnbalancedHelper(TreeNode* node, int* arr, int& index);
	void findDeepestUnbalanced(TreeNode* node, int currentDepth, int& maxDepth, TreeNode*& deepest);
	int updateSizesHelper(TreeNode* node);
	TreeNode* applyRotationHelper(TreeNode* current, int pivotValue, int direction);

public:
	AVL() : BST() {}
	~AVL() {}

	int height(TreeNode* N);
	int getHeight(TreeNode* node); // compatibility wrapper
	int getSize(TreeNode* node);
	void updateHeights(TreeNode* node);
	void updateHeights();
	int getBalance(TreeNode* N);

	TreeNode* rightRotate(TreeNode* z);
	TreeNode* leftRotate(TreeNode* z);
	TreeNode* rotateRight(TreeNode* node); // direct call
	TreeNode* rotateLeft(TreeNode* node);  // direct call
	void applyRotation(int pivotValue, int direction);

	int* getUnbalancedNodes(int& count);
	TreeNode* findUnbalancedNode();
	int getProblem(TreeNode* node);

	void updateSizes();
	TreeNode* findKth(int k);
};
