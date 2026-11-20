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

struct L2_3_data
{
	int levelId;
	string levelName;
	int optimalSteps;
	int totalInitialKeys;
	int* initialKeys;
	int totalDeleteKeys;
	int* deleteKeys;

	L2_3_data()
	{
		levelId = 0;
		levelName = "";
		optimalSteps = 0;
		totalInitialKeys = 0;
		initialKeys = NULL;
		totalDeleteKeys = 0;
		deleteKeys = NULL;
	}

	~L2_3_data()
	{
		if (initialKeys != NULL)
		{
			delete[] initialKeys;
			initialKeys = NULL;
		}
		if (deleteKeys != NULL)
		{
			delete[] deleteKeys;
			deleteKeys = NULL;
		}
	}
};

class Level2_3DataLoader
{
public:
	L2_3_data data;

	bool loadJSON(const string& filePath);
};

class Act2Level3 : public ILevelLogic
{
private:
	BST bst;
	Level2_3DataLoader loader;

	int currentTargetIdx;
	int lastDeletedKey;
	int lastDeleteCase;
	int lastSuccessorKey;

public:
	Act2Level3();
	~Act2Level3();

	bool initLevel(const string& jsonPath);

	void onValidStep() override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	int getCurrentTarget();
	int getCompletedDeletions();
	int getTotalDeletions();
	int getLastDeletedKey();
	int getLastDeleteCase();
	int getLastSuccessorKey();
	TreeNode* getTreeRoot();
};
