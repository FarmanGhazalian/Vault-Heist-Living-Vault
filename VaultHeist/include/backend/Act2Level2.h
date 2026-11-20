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

struct L2_2_data
{
	int levelId;
	string levelName;
	int optimalSteps;
	int totalInitialKeys;
	int* initialKeys;
	int totalInsertKeys;
	int* insertKeys;

	L2_2_data()
	{
		levelId = 0;
		levelName = "";
		optimalSteps = 0;
		totalInitialKeys = 0;
		initialKeys = NULL;
		totalInsertKeys = 0;
		insertKeys = NULL;
	}

	~L2_2_data()
	{
		if (initialKeys != NULL)
		{
			delete[] initialKeys;
			initialKeys = NULL;
		}
		if (insertKeys != NULL)
		{
			delete[] insertKeys;
			insertKeys = NULL;
		}
	}
};

class Level2_2DataLoader
{
public:
	L2_2_data data;

	bool loadJSON(const string& filePath);
};

class Act2Level2 : public ILevelLogic
{
private:
	BST bst;
	Level2_2DataLoader loader;

	int currentKeyIdx;
	int* currentPath;
	int currentPathLength;
	int currentPathIdx;

	void computeCurrentPath();

public:
	Act2Level2();
	~Act2Level2();

	bool initLevel(const string& jsonPath);

	void onValidStep() override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	int getCurrentKey();
	int getCurrentPathIndex();
	int getCurrentPathLength();
	int getCompletedInsertions();
	int getTotalInsertions();
	TreeNode* getTreeRoot();
};
