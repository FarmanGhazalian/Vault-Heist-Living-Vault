#pragma once
#include<iostream>
#include"TreeNode.h"
using namespace std;

class BinaryTree
{
private:

	TreeNode* root;
	int countNodes(TreeNode* root);
	void detroytree(TreeNode* root);
	void preorderHelper(TreeNode* node, int * arr, int index);
	void inorderHelper(TreeNode* node, int* arr, int index);
	void postorderHelper(TreeNode* node, int* arr, int index);
	void levelOrderHelper(TreeNode* root, int* arr, int& index);

public:
	BinaryTree()
	{
		root = NULL;
	}
	~BinaryTree();

	TreeNode getRoot();

	void setRoot(TreeNode* newRoot);
	int *preorderRecursive();
	int *inorderRecursive();
	int *postorderRecursive();
	int *levelOrder();

	int* preorderIterative();
	int* inorderIterative();
	int* postorderIterative();
	void insert(int val);




};



