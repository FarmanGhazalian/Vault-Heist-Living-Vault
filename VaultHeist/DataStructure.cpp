#include<iostream>
#include"Queue.h"
using namespace std;



void queue::Push(TreeNode* n)
{
	queueNode* temp = new queueNode(n);
	if (top == NULL)
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
	if (top == NULL)
	{
		return;
	}
	else if (top == bottom)
	{
		delete top;
		top = NULL;
		bottom = NULL;
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
	return top->n;
}

bool queue::isEmpty()
{
	if (top == NULL)
	{
		return true;
	}
	return false;
}

void stack::push(TreeNode* n)
{
	queueNode* temp = new queueNode(n);
	if (top == NULL)
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
	if (top == NULL)
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
	if (top == NULL)
	{
		return true;
	}
	return false;
}


TreeNode* stack::getTop()
{
	return top->n;
}