#include "backend/AVL.h"

int AVL::max(int a, int b)
{
	return (a > b) ? a : b;
}

int AVL::height(TreeNode* N)
{
	if (N == NULL)
		return 0;
	return N->height;
}

int AVL::getHeight(TreeNode* node)
{
	return height(node);
}

int AVL::getSize(TreeNode* node)
{
	if (node == NULL)
		return 0;
	return node->size;
}

void AVL::updateHeights(TreeNode* node)
{
	if (node == NULL)
		return;
	updateHeights(node->left);
	updateHeights(node->right);
	node->height = 1 + max(height(node->left), height(node->right));
}

void AVL::updateHeights()
{
	updateHeights(getRoot());
}

int AVL::getBalance(TreeNode* N)
{
	if (N == NULL)
		return 0;
	return height(N->left) - height(N->right);
}

TreeNode* AVL::rightRotate(TreeNode* z)
{
	TreeNode* y = z->left;
	TreeNode* T3 = y->right;

	// Perform rotation
	y->right = z;
	z->left = T3;

	// Update heights
	z->height = max(height(z->left), height(z->right)) + 1;
	y->height = max(height(y->left), height(y->right)) + 1;

	// Update sizes
	z->size = getSize(z->left) + getSize(z->right) + 1;
	y->size = getSize(y->left) + getSize(y->right) + 1;

	// Return new root
	return y;
}

TreeNode* AVL::leftRotate(TreeNode* z)
{
	TreeNode* y = z->right;
	TreeNode* T2 = y->left;

	// Perform rotation
	y->left = z;
	z->right = T2;

	// Update heights
	z->height = max(height(z->left), height(z->right)) + 1;
	y->height = max(height(y->left), height(y->right)) + 1;

	// Update sizes
	z->size = getSize(z->left) + getSize(z->right) + 1;
	y->size = getSize(y->left) + getSize(y->right) + 1;

	// Return new root
	return y;
}

TreeNode* AVL::rotateRight(TreeNode* node)
{
	return rightRotate(node);
}

TreeNode* AVL::rotateLeft(TreeNode* node)
{
	return leftRotate(node);
}

void AVL::countUnbalancedHelper(TreeNode* node, int& count)
{
	if (node == NULL)
		return;

	int bal = getBalance(node);
	if (bal < -1 || bal > 1)
		count++;

	countUnbalancedHelper(node->left, count);
	countUnbalancedHelper(node->right, count);
}

void AVL::getUnbalancedHelper(TreeNode* node, int* arr, int& index)
{
	if (node == NULL)
		return;

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
		return NULL;

	int* arr = new int[count];
	int index = 0;
	getUnbalancedHelper(getRoot(), arr, index);

	return arr;
}

void AVL::findDeepestUnbalanced(TreeNode* node, int currentDepth, int& maxDepth, TreeNode*& deepest)
{
	if (node == NULL)
		return;

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
		return -1;

	int bal = getBalance(node);

	// Left Left or Left Right
	if (bal > 1)
	{
		if (getBalance(node->left) >= 0)
			return 0; // LL
		else
			return 2; // LR
	}
	// Right Right or Right Left
	else if (bal < -1)
	{
		if (getBalance(node->right) <= 0)
			return 1; // RR
		else
			return 3; // RL
	}

	return -1;
}

int AVL::updateSizesHelper(TreeNode* node)
{
	if (node == NULL)
		return 0;

	node->size = updateSizesHelper(node->left) + updateSizesHelper(node->right) + 1;
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
			leftSize = curr->left->size;

		if (k == leftSize + 1)
			return curr;
		else if (k <= leftSize)
			curr = curr->left;
		else
		{
			k = k - (leftSize + 1);
			curr = curr->right;
		}
	}

	return NULL;
}

TreeNode* AVL::applyRotationHelper(TreeNode* current, int pivotValue, int direction)
{
	if (current == NULL)
		return NULL;

	if (current->val == pivotValue)
	{
		if (direction == 1)
			return rightRotate(current);
		else if (direction == 0)
			return leftRotate(current);
		return current;
	}

	if (pivotValue < current->val)
	{
		current->left = applyRotationHelper(current->left, pivotValue, direction);
	}
	else
	{
		current->right = applyRotationHelper(current->right, pivotValue, direction);
	}

	current->height = max(height(current->left), height(current->right)) + 1;
	current->size = getSize(current->left) + getSize(current->right) + 1;
	return current;
}

void AVL::applyRotation(int pivotValue, int direction)
{
	getRoot() = applyRotationHelper(getRoot(), pivotValue, direction);
}
