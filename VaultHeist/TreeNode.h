#pragma once
#include <iostream>
#include <string>
#include <SFML/System/Vector2.hpp>

class TreeNode
{
public:
	static const int LOCKED = 0;
	static const int UNLOCKED = 1;
	static const int TARGET = 2;
	static const int INVALID = 3;

	int val;
	std::string NodeId;
	TreeNode* left;
	TreeNode* right;
	int state;
	sf::Vector2f position;
	bool isVisited;

	TreeNode(int value = 0, int name = 0)
	{
		val = value;
		NodeId = std::to_string(name);
		left = nullptr;
		right = nullptr;
		state = LOCKED;
		position.x = 0.0f;
		position.y = 0.0f;
		isVisited = false;
	}
};
