#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "ILevelLogic.h"
#include "PlayerAction.h"
#include "MaxHeap.h"

using namespace std;
using json = nlohmann::json;

struct L4_3_data
{
	int    levelId;
	string levelName;
	int    optimalSteps;
	int    totalValues;
	int*   values;

	L4_3_data()
	{
		levelId      = 0;
		levelName    = "";
		optimalSteps = 0;
		totalValues  = 0;
		values       = NULL;
	}

	~L4_3_data()
	{
		if (values != NULL)
		{
			delete[] values;
			values = NULL;
		}
	}
};

class Level4_3DataLoader
{
public:
	L4_3_data data;

	bool loadJSON(const string& filePath);
};

class Act4Level3 : public ILevelLogic
{
protected:
	MaxHeap            heap;
	Level4_3DataLoader loader;

	int* sortedRef;
	int  phase; // 0 = EXTRACT, 1 = SIFT

	bool isFullySorted() const;

public:
	Act4Level3();
	virtual ~Act4Level3();

	virtual bool initLevel(const string& jsonPath);

	void onValidStep()   override;
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
	void onHintRequested() override;

	ActionResult processAction(const PlayerAction& action) override;

	const MaxHeap& getHeap() const;
	int getPhase() const;
	int getHeapSize() const;
};
