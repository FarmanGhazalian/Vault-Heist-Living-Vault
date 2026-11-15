#pragma once
#include "Act4Level3.h"

struct L4_4_data
{
	int    levelId;
	string levelName;
	int    optimalSteps;
	int    timeLimit;
	int    totalValues;
	int*   values;

	L4_4_data()
	{
		levelId      = 0;
		levelName    = "";
		optimalSteps = 0;
		timeLimit    = 60;
		totalValues  = 0;
		values       = NULL;
	}

	~L4_4_data()
	{
		if (values != NULL)
		{
			delete[] values;
			values = NULL;
		}
	}
};

class Level4_4DataLoader
{
public:
	L4_4_data data;

	bool loadJSON(const string& filePath);
};

class Act4Level4 : public Act4Level3
{
protected:
	Level4_4DataLoader loader4;
	float              timeLeft;

public:
	Act4Level4();
	virtual ~Act4Level4() {}

	bool initLevel(const string& jsonPath) override;
	void tick(float dt);

	float getTimeLeft() const;
};
