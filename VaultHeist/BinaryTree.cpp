#include<iostream>
#include"BinaryTree.h"
#include"DataStructure.h"

int BinaryTree::countNodes(TreeNode* root)
{
	if (root == NULL)
	{
		return 0;
	}
	else
	{
		return 1 + countNodes(root->left) + countNodes(root->right);
	}
}
void BinaryTree::preorderHelper(TreeNode* node, int* arr, int index)
{
	if (node == NULL)
	{
		return;
	}
	arr[index] = node->val;
	preorderHelper(node->left, arr, index+1 );
	preorderHelper(node->right, arr, index+1 );
}

void BinaryTree::inorderHelper(TreeNode* node, int* arr, int index)
{
	if (node == NULL)
	{
		return;
	}
	inorderHelper(node->left, arr, index+1 );
	arr[index] = node->val;
	inorderHelper(node->right, arr, index+1 );
}

void BinaryTree::postorderHelper(TreeNode* node, int* arr, int index)
{
	if (node == NULL)
	{
		return;
	}
	postorderHelper(node->left, arr, index + 1);
	postorderHelper(node->right, arr, index + 1);
	arr[index] = node->val;
}

void BinaryTree::setRoot(TreeNode* newRoot)
{
	if (root == NULL)
	{
		root = newRoot;

	}
	return;
}

void BinaryTree::levelOrderHelper(TreeNode* root, int* arr, int& index)
{
	if (root == NULL)
	{
		return;
	}
	queue q;
	q.Push(root);
	while (!q.isEmpty())
	{
		TreeNode* temp = q.getTop();
		q.pop();
		if (temp->left != NULL)
		{
			q.Push(temp->left);
		}
		if (temp->right != NULL)
		{
			q.Push(temp->right);
		}
		arr[index] = temp->val;
		index++;
	}

}

int* BinaryTree::preorderRecursive()
{
	int* arr = new int[countNodes(root)];
	int index = 0;
	preorderHelper(root, arr, index);
	return arr;
}

int* BinaryTree::inorderRecursive()
{
	int* arr = new int[countNodes(root)];
	int index = 0;
	inorderHelper(root, arr, index);
	return arr;
}

int* BinaryTree::postorderRecursive()
{
	int* arr = new int[countNodes(root)];
	int index = 0;
	postorderHelper(root, arr, index);
	return arr;
}

int* BinaryTree::levelOrder()
{
	int* arr = new int[countNodes(root)];
	int index = 0;
	levelOrderHelper(root, arr, index);
	return arr;
}

void BinaryTree::insert(int val)
{
	if (root == NULL)
	{
		root = new TreeNode(val);
		return;
	}
	else
	{
		queue q;
		q.Push(root);
		while (!q.isEmpty())
		{
			TreeNode* temp = q.getTop();
			q.pop();
			if (temp->left == NULL)
			{
				temp->left = new TreeNode(val);
				return;
			}
			else
			{
				q.Push(temp->left);
			}
			if (temp->right == NULL)
			{
				temp->right = new TreeNode(val);
				return;
			}
			else
			{
				q.Push(temp->right);
			}
		}
	}
}

int* BinaryTree::preorderIterative()
{
	if (root == NULL)
	{
		return NULL;
	}
	stack s;
	s.push(root);
	int* arr = new int[countNodes(root)];
	int index = 0;
	while (!s.isEmpty())
	{
		TreeNode* temp = s.getTop();
		arr[index] = temp->val;
		index++;
		s.pop();
		if (temp->right != NULL)
		{
			s.push(temp->right);
		}
		if (temp->left != NULL)
		{
			s.push(temp->left);
		}
	}
	return arr;
}

int* BinaryTree::postorderIterative()
{
	if (root == NULL)
	{
		return NULL;
	}
	stack s;
	s.push(root);
	int* arr = new int[countNodes(root)];
	int index = 0;

	while (!s.isEmpty())
	{

	}
}

