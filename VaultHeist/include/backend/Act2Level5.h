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

struct L2_5_data
{
	int levelId;
	string levelName;
	int optimalSteps;
	int totalKeys;
	int* keys;
	int totalTargets;
	int* targets;

	L2_5_data()
	{
		levelId = 0;
		levelName = "";
		optimalSteps = 0;
		totalKeys = 0;
		keys = NULL;
		totalTargets = 0;
		targets = NULL;
	}

	~L2_5_data()
	{
		if (keys != NULL)
		{
			delete[] keys;
			keys = NULL;
		}
		if (targets != NULL)
		{
			delete[] targets;
			targets = NULL;
		}
	}
};

class Level2_5DataLoader
{
public:
	L2_5_data data;

	bool loadJSON(const string& filePath);
};

class Act2Level5 : public ILevelLogic
{
private:
	BST bst;
	Level2_5DataLoader loader;

	int currentTargetIdx;
	int* currentPath;
	int currentPathLength;
	int currentPathIdx;
	int validStepsSinceLastShrink;

	void computeCurrentPath();

public:
	Act2Level5();
	~Act2Level5();

	bool initLevel(const string& jsonPath);

	void onValidStep() override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	int getCurrentTarget();
	int getCurrentPathIndex();
	int getCurrentPathLength();
	int getHeight();
	int getCapacityLeft();
	int getCompletedTargets();
	int getTotalTargets();
	TreeNode* getTreeRoot();
};
