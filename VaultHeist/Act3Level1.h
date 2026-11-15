#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "ILevelLogic.h"
#include "PlayerAction.h"
#include "AVL.h"

using namespace std;
using json = nlohmann::json;

struct L3_1_data
{
	int levelId;
	string levelName;
	int optimalSteps;
	int totalKeys;
	int* keys;

	L3_1_data()
	{
		levelId = 0;
		levelName = "";
		optimalSteps = 0;
		totalKeys = 0;
		keys = NULL;
	}

	~L3_1_data()
	{
		if (keys != NULL)
		{
			delete[] keys;
			keys = NULL;
		}
	}
};

class Level3_1DataLoader
{
public:
	L3_1_data data;

	bool loadJSON(const string& filePath);
};

class Act3Level1 : public ILevelLogic
{
private:
	AVL avl;
	Level3_1DataLoader loader;

	int* unbalancedNodes;
	bool* found;
	int totalUnbalanced;
	int foundCount;

	int findValueIndex(int val);

public:
	Act3Level1();
	~Act3Level1();

	bool initLevel(const string& jsonPath);

	void onValidStep() override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	TreeNode* getTree();
	int getFoundCount();
	int getTotalToFind();
	int getBalance(TreeNode* node);
};
