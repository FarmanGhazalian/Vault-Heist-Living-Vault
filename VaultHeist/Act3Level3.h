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

struct L3_3_data
{
	int    levelId;
	string levelName;
	int    optimalSteps;
	int    totalKeys;
	int*   keys;

	L3_3_data()
	{
		levelId      = 0;
		levelName    = "";
		optimalSteps = 0;
		totalKeys    = 0;
		keys         = NULL;
	}

	~L3_3_data()
	{
		if (keys != NULL)
		{
			delete[] keys;
			keys = NULL;
		}
	}
};

class Level3_3DataLoader
{
public:
	L3_3_data data;

	bool loadJSON(const string& filePath);
};

class Act3Level3 : public ILevelLogic
{
private:
	AVL              avl;
	Level3_3DataLoader loader;

	int problem;
	int rootNodeVal;
	int childNodeVal;
	int rootRotation;  // 0 = left, 1 = right
	int childRotation; // -1 = none, 0 = left, 1 = right
	int phase;         // 1 = CHOOSE_CASE, 2 = ROTATE
	int rotIdx;        // current rotation step in sequence
	int totalRotations;// 1 for single, 2 for double

public:
	Act3Level3();
	~Act3Level3() {}

	bool initLevel(const string& jsonPath);

	void onValidStep()   override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	TreeNode* getTree();
	int       getPhase();
	int       getProblem();
};
