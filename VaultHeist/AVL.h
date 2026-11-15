#pragma once
#include "BST.h"

class AVL : public BST
{
private:
	int updateHeightsHelper(TreeNode* node);
	void countUnbalancedHelper(TreeNode* node, int& count);
	void getUnbalancedHelper(TreeNode* node, int* arr, int& index);
	void findDeepestUnbalanced(TreeNode* node, int currentDepth, int& maxDepth, TreeNode*& deepest);
	int updateSizesHelper(TreeNode* node);
	TreeNode* rotateSubtree(TreeNode* current, int targetVal, int direction);

public:
	AVL() : BST() {}
	~AVL() {}

	int getHeight(TreeNode* node);
	void updateHeights();
	int getBalance(TreeNode* node);

	int* getUnbalancedNodes(int& count);
	TreeNode* findUnbalancedNode();
	int getProblem(TreeNode* node);

	TreeNode* rotateLeft(TreeNode* node);
	TreeNode* rotateRight(TreeNode* node);
	void applyRotation(int pivotVal, int direction);

	void updateSizes();
	TreeNode* findKth(int k);
};
