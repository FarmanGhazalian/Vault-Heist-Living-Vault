#pragma once
#include "TreeNode.h"

class queueNode
{
public:
	TreeNode* n;
	queueNode* next;
	queueNode(TreeNode* node)
	{
		n = node;
		next = nullptr;
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
		top = nullptr;
		bottom = nullptr;
	}
	~queue();
};

class stack
{
public:
	queueNode* top;
	stack()
	{
		top = nullptr;
	}
	~stack();
	void push(TreeNode* n);
	void pop();
	TreeNode* getTop();
	bool isEmpty();
};
