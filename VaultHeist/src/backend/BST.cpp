#include "backend/BST.h"

TreeNode* BST::insertHelper(TreeNode* node, int key, bool& inserted)
{
	if (node == NULL)
	{
		inserted = true;
		return new TreeNode(key, key);
	}
	if (key < node->val)
	{
		node->left = insertHelper(node->left, key, inserted);
	}
	else if (key > node->val)
	{
		node->right = insertHelper(node->right, key, inserted);
	}
	else
	{
		inserted = false;
	}
	return node;
}

bool BST::insert(int key)
{
	bool inserted = false;
	TreeNode*& r = getRoot();
	r = insertHelper(r, key, inserted);
	return inserted;
}

TreeNode* BST::findMin(TreeNode* node) const
{
	if (node == NULL)
	{
		return NULL;
	}
	while (node->left != NULL)
	{
		node = node->left;
	}
	return node;
}

TreeNode* BST::inorderSuccessor(TreeNode* node)
{
	if (node == NULL || node->right == NULL)
	{
		return NULL;
	}
	return findMin(node->right);
}

int BST::deleteCase(int key)
{
	TreeNode* node = findNode(key);
	if (node == NULL)
	{
		return -1;
	}
	if (node->left == NULL && node->right == NULL)
	{
		return 0;
	}
	if (node->left != NULL && node->right != NULL)
	{
		return 2;
	}
	return 1;
}

TreeNode* BST::removeHelper(TreeNode* node, int key, bool& removed)
{
	if (node == NULL)
	{
		return NULL;
	}

	if (key < node->val)
	{
		node->left = removeHelper(node->left, key, removed);
	}
	else if (key > node->val)
	{
		node->right = removeHelper(node->right, key, removed);
	}
	else
	{
		removed = true;
		if (node->left == NULL && node->right == NULL)
		{
			delete node;
			return NULL;
		}
		if (node->left == NULL)
		{
			TreeNode* temp = node->right;
			delete node;
			return temp;
		}
		if (node->right == NULL)
		{
			TreeNode* temp = node->left;
			delete node;
			return temp;
		}
		TreeNode* succ = findMin(node->right);
		node->val = succ->val;
		node->NodeId = succ->NodeId;
		node->right = removeHelper(node->right, succ->val, removed);
	}
	return node;
}

bool BST::remove(int key)
{
	bool removed = false;
	TreeNode*& r = getRoot();
	r = removeHelper(r, key, removed);
	return removed;
}

int BST::countRangeHelper(TreeNode* node, int lo, int hi) const
{
	if (node == NULL)
	{
		return 0;
	}
	int count = 0;
	if (node->val > lo)
	{
		count += countRangeHelper(node->left, lo, hi);
	}
	if (node->val >= lo && node->val <= hi)
	{
		count++;
	}
	if (node->val < hi)
	{
		count += countRangeHelper(node->right, lo, hi);
	}
	return count;
}

void BST::rangeHelper(TreeNode* node, int lo, int hi, int* arr, int& count) const
{
	if (node == NULL)
	{
		return;
	}
	if (node->val > lo)
	{
		rangeHelper(node->left, lo, hi, arr, count);
	}
	if (node->val >= lo && node->val <= hi)
	{
		arr[count] = node->val;
		count++;
	}
	if (node->val < hi)
	{
		rangeHelper(node->right, lo, hi, arr, count);
	}
}

int* BST::rangeToArray(int lo, int hi, int& count)
{
	int total = countRangeHelper(getRoot(), lo, hi);
	count = 0;
	if (total == 0)
	{
		return NULL;
	}
	int* arr = new int[total];
	rangeHelper(getRoot(), lo, hi, arr, count);
	return arr;
}

int* BST::getInsertPath(int key, int& length)
{
	length = 0;
	TreeNode* curr = getRoot();
	if (curr == NULL)
	{
		return NULL;
	}

	int count = 0;
	while (curr != NULL)
	{
		count++;
		if (key < curr->val)
		{
			if (curr->left == NULL)
			{
				break;
			}
			curr = curr->left;
		}
		else if (key > curr->val)
		{
			if (curr->right == NULL)
			{
				break;
			}
			curr = curr->right;
		}
		else
		{
			break;
		}
	}

	int* path = new int[count];
	curr = getRoot();
	for (int i = 0; i < count; i++)
	{
		path[i] = curr->val;
		if (key < curr->val)
		{
			if (curr->left == NULL)
			{
				break;
			}
			curr = curr->left;
		}
		else if (key > curr->val)
		{
			if (curr->right == NULL)
			{
				break;
			}
			curr = curr->right;
		}
		else
		{
			break;
		}
	}
	length = count;
	return path;
}

bool BST::isVisitedByPrunedRange(int key, int lo, int hi)
{
	TreeNode* curr = getRoot();
	if (curr == NULL) return false;

	while (curr != NULL)
	{
		if (curr->val == key)
		{
			return true;
		}

		if (key < curr->val)
		{
			// Left step allowed only if curr->val > lo
			if (curr->val <= lo)
			{
				return false;
			}
			curr = curr->left;
		}
		else
		{
			// Right step allowed only if curr->val < hi
			if (curr->val >= hi)
			{
				return false;
			}
			curr = curr->right;
		}
	}
	return false;
}
