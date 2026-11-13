#include<iostream>
#include"BinaryTree.h"
#include"DataStructure.h"


TreeNode*& BinaryTree::getRoot()
{
	return root;
}
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
void BinaryTree::preorderHelper(TreeNode* node, int* arr, int& index)
{
	if (node == NULL)
	{
		return;
	}
	arr[index++] = node->val;
	preorderHelper(node->left, arr, index);
	preorderHelper(node->right, arr, index);
}

void BinaryTree::inorderHelper(TreeNode* node, int* arr, int& index)
{
	if (node == NULL)
	{
		return;
	}
	inorderHelper(node->left, arr, index);
	arr[index++] = node->val;
	inorderHelper(node->right, arr, index);
}

void BinaryTree::postorderHelper(TreeNode* node, int* arr, int& index)
{
	if (node == NULL)
	{
		return;
	}
	postorderHelper(node->left, arr, index);
	postorderHelper(node->right, arr, index);
	arr[index++] = node->val;
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
		TreeNode* temp = s.getTop();
		arr[index] = temp->val;
		index++;
		s.pop();
		if (temp->left != NULL)
		{
			s.push(temp->left);
		}
		if (temp->right != NULL)
		{
			s.push(temp->right);
		}
	}
	int* reversedArr = new int[countNodes(root)];
	for (int i = 0; i < countNodes(root); i++)
	{
		reversedArr[i] = arr[countNodes(root) - 1 - i];
	}
	delete[] arr;
	return reversedArr;
}

int* BinaryTree::inorderIterative()
{
	if (root == NULL)
	{
		return NULL;
	}
	stack s;
	int* arr = new int[countNodes(root)];
	int index = 0;
	TreeNode* temp = root;
	while (temp != NULL || !s.isEmpty())
	{
		while (temp != NULL)
		{
			s.push(temp);
			temp = temp->left;
		}
		temp = s.getTop();
		s.pop();
		arr[index] = temp->val;
		index++;
		temp = temp->right;
	}
	return arr;
}

TreeNode* BinaryTree::buildFromPreInHelper(int pre[], int in[], int& preIndex, int start, int end)
{
	if (start > end)
	{
		return NULL;
	}
	int value = pre[preIndex];
	preIndex++;
	TreeNode* temp = new TreeNode(value);
	int pos = start;
	while (in[pos] != value)
	{
		pos++;
	}
	temp->left = buildFromPreInHelper(pre, in, preIndex, start, pos - 1);
	temp->right = buildFromPreInHelper(pre, in, preIndex, pos + 1, end);
	return temp;
}

void BinaryTree::buildFromPreIn(int pre[], int in[], int size)
{
	if (root != NULL)
	{
		destroyTree();
		root = NULL;
	}
	int preIndex = 0;
	root = buildFromPreInHelper(pre, in, preIndex, 0, size - 1);
	return;
}

TreeNode* BinaryTree::buildFromPosInHelper(int pos[], int in[], int& posIndex, int start, int end)
{
	if (start > end)
	{
		return NULL;
	}
	int value = pos[posIndex];
	posIndex--;
	TreeNode* temp = new TreeNode(value);
	int position = start;
	while (in[position] != value)
	{
		position++;
	}
	temp->right = buildFromPosInHelper(pos, in, posIndex, position + 1, end);
	temp->left = buildFromPosInHelper(pos, in, posIndex, start, position - 1);
	return temp;
}

void BinaryTree::buildFromPosIn(int pos[], int in[], int size)
{
	if (root != NULL)
	{
		destroyTree();
		root = NULL;
	}
	int posIndex = size - 1;
	root = buildFromPosInHelper(pos, in, posIndex, 0, size - 1);
	return;
}

int BinaryTree::TreeHeight(TreeNode* node)
{
	return heightHelper(node);
}

int BinaryTree::heightHelper(TreeNode* root)
{
	if (root == NULL)
	{
		return 0;
	}
	if (root->left == NULL && root->right == NULL)
	{
		return 1;
	}
	return 1 + max(heightHelper(root->left), heightHelper(root->right));
}

int BinaryTree::height() 
{
	return heightHelper(root);
}

int BinaryTree::diameterHelper(TreeNode* root, int& dia)
{
	if (root == NULL)
	{
		return 0;
	}
	int left = diameterHelper(root->left, dia);
	int right = diameterHelper(root->right, dia);
	dia = max(dia, left + right);
	return 1 + max(left, right);
}

int BinaryTree::diameter()
{
	int dia = 0;
	diameterHelper(root, dia);
	return dia;
}

bool BinaryTree::PathToHelper(TreeNode* root, int target,int *arr,int& pathSize )
{
	if (root == NULL)
	{
		return false;
	}
	arr[pathSize] = root->val;
	pathSize++;
	if (root->val == target)
	{
		return true;
	}
	if (PathToHelper(root->left, target, arr, pathSize) || PathToHelper(root->right, target, arr, pathSize))
	{
		return true;
	}
	pathSize--;
	return false;
}

int* BinaryTree::pathTo(int target)
{
	int pathSize = 0;
	int* arr = new int[countNodes(root)];
	if (!PathToHelper(root, target, arr, pathSize))
	{
		delete[]arr;
		return NULL;
	}
	int* path = new int[pathSize];
	for (int i = 0;i < pathSize;i++)
	{
		path[i] = arr[i];
	}
	delete[]arr;
	return path;
}

TreeNode* BinaryTree::lcaHelper(TreeNode* root, int a, int b)
{
	if (root == NULL)
	{
		return NULL;
	}
	if (root->val == a || root->val == b)
	{
		return root;
	}

	TreeNode* left = lcaHelper(root->left, a, b);
	TreeNode* right = lcaHelper(root->right, a, b);

	if (left != NULL && right !=NULL)
	{
		return root;
	}
	if (left != NULL)
	{
		return left;
	}
	return right;
}

TreeNode* BinaryTree::lca(int a, int b)
{
	return lcaHelper(root, a, b);
}

void BinaryTree::MirrorHelper(TreeNode* root)
{
	if (root == NULL)
	{
		return;
	}
	MirrorHelper(root->left);
	MirrorHelper(root->right);

	TreeNode* temp = root->left;
	root->left = root->right;
	root->right = temp;
	return;
}

void BinaryTree::Mirror()
{
	MirrorHelper(root);
}

bool BinaryTree::isSameTreeHelper(TreeNode* a, TreeNode* b)
{
	if (a == NULL && b == NULL)
	{
		return true;
	}
	if (a == NULL || b == NULL)
	{
		return false;
	}
	if (a->val != b->val)
	{
		return false;
	}
	return (isSameTreeHelper(a->left, b->left) && isSameTreeHelper(a->right, b->right));
}

bool BinaryTree::isSame(BinaryTree& other)
{
	return isSameTreeHelper(root, other.getRoot());
}

bool BinaryTree::isSymmetricHelper(TreeNode* a, TreeNode* b)
{
	if (a == NULL && b == NULL)
	{
		return true;
	}
	if (a == NULL || b == NULL)
	{
		return false;
	}
	if (a->val != b->val)
	{
		return false;
	}
	return (isSymmetricHelper(a->left, b->right) && isSymmetricHelper(a->right, b->left));
}

bool BinaryTree::isSymmetric()
{
	if (root == NULL)
	{
		return false;
	}
	if (root->left != NULL && root->right != NULL)
	{
		return isSymmetricHelper(root->left, root->right);
	}
	else if (root->left != NULL || root->right != NULL)
	{
		return false;
	}
	return true;
}

TreeNode* BinaryTree::cloneHelper(TreeNode* root)
{
	if (root == NULL)
	{
		return NULL;
	}
	TreeNode* temp = new TreeNode(root->val);
	temp->left = cloneHelper(root->left);
	temp->right = cloneHelper(root->right);

	return temp;
}

BinaryTree BinaryTree::clone()
{
	BinaryTree newTree;
	newTree.getRoot() = cloneHelper(root);
	return newTree;
}

void BinaryTree::destroytreeHelper(TreeNode* root)
{
	if (root == NULL)
	{
		return;
	}
	destroytreeHelper(root->left);
	destroytreeHelper(root->right);
	delete root;
	return;
}
void BinaryTree::destroyTree()
{
	destroytreeHelper(root);
}

TreeNode* BinaryTree::generateRandomtree(int n)
{
	if (n <= 0)
	{
		return NULL;
	}
	TreeNode* temp = new TreeNode(1);
	queue q;
	q.Push(temp);
	int currVal = 2;
	while (currVal <= n && !q.isEmpty())
	{
		TreeNode* temp1 = q.getTop();
		q.pop();
		if (currVal <= n)
		{
			temp1->left = new TreeNode(currVal);
			currVal++;
			q.Push(temp1->left);
		}
		if (currVal <= n)
		{
			temp1->right = new TreeNode(currVal);
			currVal++;
			q.Push(temp1->right);
		}
	}
	return temp;
}

TreeNode* BinaryTree::findNodeHelper(TreeNode* root, int value)
{
	if (root == NULL)
	{
		return NULL;
	}
	if (root->val == value)
	{
		return root;
	}
	TreeNode* left = findNodeHelper(root->left, value);
	if (left != NULL)
	{
		return left;
	}
	return findNodeHelper(root->right, value);
}

TreeNode* BinaryTree::findNode(int value)
{
	if (root == NULL)
	{
		return NULL;
	}
	return findNodeHelper(root, value);
}

bool BinaryTree::fillPath(TreeNode* curr, int target, int arr[], int& idx) const
{
	if (curr == nullptr) 
		return false;

	arr[idx++] = curr->val;
	if (curr->val == target)
		return true;

	if (fillPath(curr->left, target, arr, idx) || fillPath(curr->right, target, arr, idx))
	{
		return true;
	}

	idx--;
	return false;
}

int* BinaryTree::getPath(TreeNode* lca, int nodeA, int nodeB, int& length) const 
{
	if (lca == nullptr)
		return nullptr;

	int pathA[64], lenA = 0;
	int pathB[64], lenB = 0;

	if (!fillPath(lca, nodeA, pathA, lenA) || !fillPath(lca, nodeB, pathB, lenB)) {
		return nullptr;
	}

	int totalSize = lenA + lenB - 1;
	int* finalPath = new int[totalSize];
	length = totalSize;
	int k = 0;
	for (int i = lenA - 1; i >= 0; i--) {
		finalPath[k++] = pathA[i];
	}
	for (int i = 1; i < lenB; i++) {
		finalPath[k++] = pathB[i];
	}

	return finalPath;
}
