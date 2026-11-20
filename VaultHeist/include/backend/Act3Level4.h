#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "backend/ILevelLogic.h"
#include "backend/PlayerAction.h"
#include "backend/AVL.h"

using namespace std;
using json = nlohmann::json;

struct L3_4_data
{
	int    levelId;
	string levelName;
	int    optimalSteps;
	int    totalBaseKeys;
	int*   baseKeys;
	int    totalInsertKeys;
	int*   insertKeys;

	L3_4_data()
	{
		levelId         = 0;
		levelName       = "";
		optimalSteps    = 0;
		totalBaseKeys   = 0;
		baseKeys        = NULL;
		totalInsertKeys = 0;
		insertKeys      = NULL;
	}

	~L3_4_data()
	{
		if (baseKeys != NULL)
		{
			delete[] baseKeys;
			baseKeys = NULL;
		}
		if (insertKeys != NULL)
		{
			delete[] insertKeys;
			insertKeys = NULL;
		}
	}
};

class Level3_4DataLoader
{
public:
	L3_4_data data;

	bool loadJSON(const string& filePath);
};

class Act3Level4 : public ILevelLogic
{
private:
	AVL              avl;
	Level3_4DataLoader loader;

	int currentRound;
	int storedAnswer; // 0=LL, 1=RR, 2=LR, 3=RL, 4=none

	void setupRound();

public:
	Act3Level4();
	~Act3Level4() {}

	bool initLevel(const string& jsonPath);

	void onValidStep()   override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	TreeNode* getTree();
	int       getCurrentRound();
	int       getTotalRounds();
	int       getCurrentInsertKey();
};
