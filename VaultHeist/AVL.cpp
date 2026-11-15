#include "AVL.h"

int AVL::getHeight(TreeNode* node)
{
	if (node == NULL)
	{
		return 0;
	}
	return node->height;
}

int AVL::updateHeightsHelper(TreeNode* node)
{
	if (node == NULL)
	{
		return 0;
	}

	int lh = updateHeightsHelper(node->left);
	int rh = updateHeightsHelper(node->right);

	if (lh > rh)
	{
		node->height = 1 + lh;
	}
	else
	{
		node->height = 1 + rh;
	}

	return node->height;
}

void AVL::updateHeights()
{
	updateHeightsHelper(getRoot());
}

int AVL::getBalance(TreeNode* node)
{
	if (node == NULL)
	{
		return 0;
	}
	return getHeight(node->left) - getHeight(node->right);
}

void AVL::countUnbalancedHelper(TreeNode* node, int& count)
{
	if (node == NULL)
	{
		return;
	}

	int bal = getBalance(node);
	if (bal < -1 || bal > 1)
	{
		count++;
	}

	countUnbalancedHelper(node->left, count);
	countUnbalancedHelper(node->right, count);
}

void AVL::getUnbalancedHelper(TreeNode* node, int* arr, int& index)
{
	if (node == NULL)
	{
		return;
	}

	int bal = getBalance(node);
	if (bal < -1 || bal > 1)
	{
		arr[index] = node->val;
		index++;
	}

	getUnbalancedHelper(node->left, arr, index);
	getUnbalancedHelper(node->right, arr, index);
}

int* AVL::getUnbalancedNodes(int& count)
{
	updateHeights();
	count = 0;
	countUnbalancedHelper(getRoot(), count);

	if (count == 0)
	{
		return NULL;
	}

	int* arr = new int[count];
	int index = 0;
	getUnbalancedHelper(getRoot(), arr, index);

	return arr;
}

void AVL::findDeepestUnbalanced(TreeNode* node, int currentDepth, int& maxDepth, TreeNode*& deepest)
{
	if (node == NULL)
	{
		return;
	}

	findDeepestUnbalanced(node->left, currentDepth + 1, maxDepth, deepest);
	findDeepestUnbalanced(node->right, currentDepth + 1, maxDepth, deepest);

	int bal = getBalance(node);
	if (bal < -1 || bal > 1)
	{
		if (currentDepth > maxDepth)
		{
			maxDepth = currentDepth;
			deepest = node;
		}
	}
}

TreeNode* AVL::findUnbalancedNode()
{
	updateHeights();
	int maxDepth = -1;
	TreeNode* deepest = NULL;
	findDeepestUnbalanced(getRoot(), 0, maxDepth, deepest);
	return deepest;
}

int AVL::getProblem(TreeNode* node)
{
	if (node == NULL)
	{
		return -1;
	}

	int bal = getBalance(node);

	if (bal > 1)
	{
		int leftBal = getBalance(node->left);
		if (leftBal >= 0)
		{
			return 0; // LL
		}
		else
		{
			return 2; // LR
		}
	}
	else if (bal < -1)
	{
		int rightBal = getBalance(node->right);
		if (rightBal <= 0)
		{
			return 1; // RR
		}
		else
		{
			return 3; // RL
		}
	}

	return -1;
}

TreeNode* AVL::rotateLeft(TreeNode* node)
{
	if (node == NULL || node->right == NULL)
	{
		return node;
	}

	TreeNode* newRoot = node->right;
	TreeNode* temp = newRoot->left;

	newRoot->left = node;
	node->right = temp;

	int lh = getHeight(node->left);
	int rh = getHeight(node->right);
	if (lh > rh)
	{
		node->height = 1 + lh;
	}
	else
	{
		node->height = 1 + rh;
	}

	lh = getHeight(newRoot->left);
	rh = getHeight(newRoot->right);
	if (lh > rh)
	{
		newRoot->height = 1 + lh;
	}
	else
	{
		newRoot->height = 1 + rh;
	}

	return newRoot;
}

TreeNode* AVL::rotateRight(TreeNode* node)
{
	if (node == NULL || node->left == NULL)
	{
		return node;
	}

	TreeNode* newRoot = node->left;
	TreeNode* temp = newRoot->right;

	newRoot->right = node;
	node->left = temp;

	int lh = getHeight(node->left);
	int rh = getHeight(node->right);
	if (lh > rh)
	{
		node->height = 1 + lh;
	}
	else
	{
		node->height = 1 + rh;
	}

	lh = getHeight(newRoot->left);
	rh = getHeight(newRoot->right);
	if (lh > rh)
	{
		newRoot->height = 1 + lh;
	}
	else
	{
		newRoot->height = 1 + rh;
	}

	return newRoot;
}

TreeNode* AVL::rotateSubtree(TreeNode* current, int targetVal, int direction)
{
	if (current == NULL)
	{
		return NULL;
	}

	if (current->val == targetVal)
	{
		if (direction == 0)
		{
			return rotateLeft(current);
		}
		else
		{
			return rotateRight(current);
		}
	}

	if (targetVal < current->val)
	{
		current->left = rotateSubtree(current->left, targetVal, direction);
	}
	else
	{
		current->right = rotateSubtree(current->right, targetVal, direction);
	}

	return current;
}

void AVL::applyRotation(int pivotVal, int direction)
{
	TreeNode*& r = getRoot();
	r = rotateSubtree(r, pivotVal, direction);
	updateHeights();
}

int AVL::updateSizesHelper(TreeNode* node)
{
	if (node == NULL)
	{
		return 0;
	}

	int ls = updateSizesHelper(node->left);
	int rs = updateSizesHelper(node->right);
	node->size = ls + rs + 1;

	return node->size;
}

void AVL::updateSizes()
{
	updateSizesHelper(getRoot());
}

TreeNode* AVL::findKth(int k)
{
	TreeNode* curr = getRoot();

	while (curr != NULL)
	{
		int leftSize = 0;
		if (curr->left != NULL)
		{
			leftSize = curr->left->size;
		}

		if (k == leftSize + 1)
		{
			return curr;
		}
		else if (k <= leftSize)
		{
			curr = curr->left;
		}
		else
		{
			k = k - (leftSize + 1);
			curr = curr->right;
		}
	}

	return NULL;
}
