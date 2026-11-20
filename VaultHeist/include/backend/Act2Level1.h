#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "backend/ILevelLogic.h"
#include "backend/PlayerAction.h"
#include "backend/BST.h"

using namespace std;
using json = nlohmann::json;

struct L2_1_data
{
	int levelId;
	string levelName;
	int target;
	int optimalSteps;
	int totalKeys;
	int* keys;

	L2_1_data()
	{
		levelId = 0;
		levelName = "";
		target = 0;
		optimalSteps = 0;
		totalKeys = 0;
		keys = NULL;
	}

	~L2_1_data()
	{
		if (keys != NULL)
		{
			delete[] keys;
			keys = NULL;
		}
	}
};

class Level2_1DataLoader
{
public:
	L2_1_data data;

	bool loadJSON(const string& filePath);
};

class Act2Level1 : public ILevelLogic
{
private:
	BST bst;
	Level2_1DataLoader loader;

	int* expectedPath;
	int pathLength;
	int currentIndex;

public:
	Act2Level1();
	~Act2Level1();

	bool initLevel(const string& jsonPath);

	void onValidStep() override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	int getTarget();
	int getCurrentIndex();
	int getPathLength();
	TreeNode* getTreeRoot();
};
