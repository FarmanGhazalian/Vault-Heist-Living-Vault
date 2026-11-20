#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "backend/ILevelLogic.h"
#include "backend/PlayerAction.h"
#include "backend/MaxHeap.h"

using namespace std;
using json = nlohmann::json;

struct L4_1_data
{
	int    levelId;
	string levelName;
	int    optimalSteps;
	int    totalValues;
	int*   values;

	L4_1_data()
	{
		levelId      = 0;
		levelName    = "";
		optimalSteps = 0;
		totalValues  = 0;
		values       = NULL;
	}

	~L4_1_data()
	{
		if (values != NULL)
		{
			delete[] values;
			values = NULL;
		}
	}
};

class Level4_1DataLoader
{
public:
	L4_1_data data;

	bool loadJSON(const string& filePath);
};

class Act4Level1 : public ILevelLogic
{
protected:
	MaxHeap            heap;
	Level4_1DataLoader loader;

public:
	Act4Level1();
	virtual ~Act4Level1() {}

	virtual bool initLevel(const string& jsonPath);

	void onValidStep()   override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	const MaxHeap& getHeap() const;
};
