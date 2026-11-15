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

struct L3_5_data
{
	int    levelId;
	string levelName;
	int    optimalSteps;
	int    k;
	int    totalKeys;
	int*   keys;

	L3_5_data()
	{
		levelId      = 0;
		levelName    = "";
		optimalSteps = 0;
		k            = 0;
		totalKeys    = 0;
		keys         = NULL;
	}

	~L3_5_data()
	{
		if (keys != NULL)
		{
			delete[] keys;
			keys = NULL;
		}
	}
};

class Level3_5DataLoader
{
public:
	L3_5_data data;

	bool loadJSON(const string& filePath);
};

class Act3Level5 : public ILevelLogic
{
private:
	AVL              avl;
	Level3_5DataLoader loader;

	int* expectedPath;
	int  pathLength;
	int  currentIndex;
	int  targetVal;

public:
	Act3Level5();
	~Act3Level5();

	bool initLevel(const string& jsonPath);

	void onValidStep()   override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	TreeNode* getTree();
	int       getK();
	int       getTargetVal();
	int       getCurrentIndex();
	int       getPathLength();
};
