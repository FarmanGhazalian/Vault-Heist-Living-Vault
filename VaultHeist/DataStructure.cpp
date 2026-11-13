#include <iostream>
#include "DataStructure.h"
using namespace std;

queue::~queue()
{
	while (!isEmpty())
	{
		pop();
	}
}

void queue::Push(TreeNode* n)
{
	queueNode* temp = new queueNode(n);
	if (top == nullptr)
	{
		top = temp;
		bottom = temp;
		return;
	}
	else
	{
		bottom->next = temp;
		bottom = temp;
		return;
	}
}

void queue::pop()
{
	if (top == nullptr)
	{
		return;
	}
	else if (top == bottom)
	{
		delete top;
		top = nullptr;
		bottom = nullptr;
		return;
	}
	else
	{
		queueNode* temp = top;
		top = top->next;
		delete temp;
		return;
	}
}

TreeNode* queue::getTop()
{
	if (top == nullptr) return nullptr;
	return top->n;
}

bool queue::isEmpty()
{
	return (top == nullptr);
}

stack::~stack()
{
	while (!isEmpty())
	{
		pop();
	}
}

void stack::push(TreeNode* n)
{
	queueNode* temp = new queueNode(n);
	if (top == nullptr)
	{
		top = temp;
		return;
	}
	else
	{
		temp->next = top;
		top = temp;
		return;
	}
}

void stack::pop()
{
	if (top == nullptr)
	{
		return;
	}
	else
	{
		queueNode* temp = top;
		top = top->next;
		delete temp;
		return;
	}
}

bool stack::isEmpty()
{
	return (top == nullptr);
}

TreeNode* stack::getTop()
{
	if (top == nullptr) return nullptr;
	return top->n;
}
