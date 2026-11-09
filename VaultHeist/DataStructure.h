#pragma once
#include"TreeNode.h"
class queueNode
{
public:
	TreeNode* n;
	queueNode* next;
	queueNode(TreeNode* node)
	{
		n = node;
		next = NULL;
	}
};

class queue
{
public:
	queueNode* top;
	queueNode* bottom;
	void Push(TreeNode* n);
	void pop();
	TreeNode* getTop();
	bool isEmpty();

	queue()
	{
		top = NULL;
		bottom = NULL;
	}
};

class stack
{
public:
	queueNode* top;
	void push(TreeNode* n);
	void pop();
	TreeNode* getTop();
	bool isEmpty();

	stack()
	{
		top == NULL;
	}
};