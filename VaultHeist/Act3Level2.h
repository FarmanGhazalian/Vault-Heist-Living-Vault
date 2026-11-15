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

struct L3_2_data
{
	int    levelId;
	string levelName;
	int    optimalSteps;
	int    totalKeys;
	int*   keys;

	L3_2_data()
	{
		levelId      = 0;
		levelName    = "";
		optimalSteps = 0;
		totalKeys    = 0;
		keys         = NULL;
	}

	~L3_2_data()
	{
		if (keys != NULL)
		{
			delete[] keys;
			keys = NULL;
		}
	}
};

class Level3_2DataLoader
{
public:
	L3_2_data data;

	bool loadJSON(const string& filePath);
};

class Act3Level2 : public ILevelLogic
{
private:
	AVL              avl;
	Level3_2DataLoader loader;

	int unbalancedNodeVal;
	int storedProblem;
	int phase;

public:
	Act3Level2();
	~Act3Level2() {}

	bool initLevel(const string& jsonPath);

	void onValidStep()   override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	TreeNode* getTree();
	int       getPhase();
	int       getProblem();
};
