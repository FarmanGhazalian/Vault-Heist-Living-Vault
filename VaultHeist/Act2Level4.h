#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "ILevelLogic.h"
#include "PlayerAction.h"
#include "BST.h"

using namespace std;
using json = nlohmann::json;

struct L2_4_data
{
	int levelId;
	string levelName;
	int optimalSteps;
	int lo;
	int hi;
	int totalKeys;
	int* keys;

	L2_4_data()
	{
		levelId = 0;
		levelName = "";
		optimalSteps = 0;
		lo = 0;
		hi = 0;
		totalKeys = 0;
		keys = NULL;
	}

	~L2_4_data()
	{
		if (keys != NULL)
		{
			delete[] keys;
			keys = NULL;
		}
	}
};

class Level2_4DataLoader
{
public:
	L2_4_data data;

	bool loadJSON(const string& filePath);
};

class Act2Level4 : public ILevelLogic
{
private:
	BST bst;
	Level2_4DataLoader loader;

	int* rangeValues;
	bool* collected;
	int rangeCount;
	int collectedCount;

	int findValueIndex(int val);

public:
	Act2Level4();
	~Act2Level4();

	bool initLevel(const string& jsonPath);

	void onValidStep() override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	int getLo();
	int getHi();
	int getCollectedCount();
	int getTotalToCollect();
	TreeNode* getTree();
};
