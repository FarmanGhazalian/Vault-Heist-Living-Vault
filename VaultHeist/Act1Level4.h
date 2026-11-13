#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include"nlohmann/json.hpp"
#include"ILevelLogic.h"
#include "PlayerAction.h"
#include"BinaryTree.h"

using namespace std;
using json = nlohmann::json;



struct L4_data
{
    string levelId = "";
    string levelName = "";
    int optimalSteps = 0;
    int totalNodes;
    int* preArr;
    int* inArr;
    int roomA = -1;
    int roomB = -1;
	void generate();
};




class loadData
{
public:
    L4_data data;

	bool loadJSON(const string& filePath);
};




class Act1Level4 : public ILevelLogic {
private:
	loadData loader;
	BinaryTree solTree;

	int roomA = -1;
	int roomB = -1;
	int actualLCA = -1;

	int* expectedPath;
	int pathCount = 0;
	int currentPathIndex = 0;
	int expectedPathLength = 0;

	bool lcaFound = false;

protected:
	string getHint();
public:
	Act1Level4() : ILevelLogic(loader.data.optimalSteps)
	{
		expectedPath = new int[loader.data.totalNodes];
	}

	bool initLevel(const std::string& jsonPath);

	ActionResult processAction(const PlayerAction& action);

	const BinaryTree& getTree() const { return solTree; }
	int getRoomA() const { return roomA; }
	int getRoomB() const { return roomB; }
	bool isLCAFound() const { return lcaFound; }
	int getCurrentPathIndex() const { return currentPathIndex; }
	int getPathCount() const { return pathCount; }
};

