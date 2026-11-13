#pragma once
#include<iostream>
#include"TreeNode.h"
using namespace std;

class BinaryTree
{
private:

	TreeNode* root;
	int countNodes(TreeNode* root);
	void destroytreeHelper(TreeNode* root);
	void preorderHelper(TreeNode* node, int * arr, int index);
	void inorderHelper(TreeNode* node, int* arr, int index);
	void postorderHelper(TreeNode* node, int* arr, int index);
	void levelOrderHelper(TreeNode* root, int* arr, int& index);
	TreeNode* buildFromPreInHelper(int pre[], int in[], int& preIndex, int start, int end);
	TreeNode* buildFromPosInHelper(int pre[], int in[], int& preIndex, int start, int end);
	int heightHelper(TreeNode* root);
	int diameterHelper(TreeNode* root, int& dia);
	bool PathToHelper(TreeNode* root, int target, int* arr, int& pathSize);
	TreeNode* lcaHelper(TreeNode* root, int a, int b);
	void MirrorHelper(TreeNode* root);
	bool isSameTreeHelper(TreeNode* a, TreeNode* b);
	bool isSymmetricHelper(TreeNode* a, TreeNode* b);
	TreeNode* cloneHelper(TreeNode* root);
	TreeNode* findNodeHelper(TreeNode* root,int val);

public:
	BinaryTree()
	{
		root = NULL;
	}
	~BinaryTree()
	{
		destroyTree();
	}

	TreeNode*& getRoot();

	void setRoot(TreeNode* newRoot);
	int *preorderRecursive();
	int *inorderRecursive();
	int *postorderRecursive();
	int *levelOrder();

	int* preorderIterative();
	int* inorderIterative();
	int* postorderIterative();
	void insert(int val);
	void buildFromPreIn(int pre[], int in[], int size);
	void buildFromPreIn(int pos[], int in[], int size);
	int TreeHeight(TreeNode* root);
	int height();
	int diameter();
	int* pathTo(int target);
	TreeNode* lca(int a, int b);
	void Mirror();
	bool isSame(BinaryTree& other);
	bool isSymmetric();
	BinaryTree clone();
	void destroyTree();
	TreeNode* generateRandomtree(int n);
	TreeNode* findNode(int value);
};



