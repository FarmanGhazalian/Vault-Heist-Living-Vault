#include "Act4Level4.h"

bool Level4_4DataLoader::loadJSON(const string& filePath)
{
	ifstream file(filePath);
	if (!file.is_open())
	{
		string fallback = "VaultHeist/" + filePath;
		file.open(fallback);
		if (!file.is_open())
		{
			return false;
		}
	}

	json j;
	file >> j;
	file.close();

	if (j["levelId"].is_string())
	{
		data.levelId = 44;
	}
	else
	{
		data.levelId = j["levelId"];
	}

	data.levelName    = j["levelName"];
	data.optimalSteps = j["optimalSteps"];
	data.timeLimit    = j["timeLimit"];

	auto valsArr     = j["values"];
	data.totalValues = (int)valsArr.size();

	if (data.values != NULL)
	{
		delete[] data.values;
		data.values = NULL;
	}

	data.values = new int[data.totalValues];
	for (int i = 0; i < data.totalValues; i++)
	{
		data.values[i] = valsArr[i];
	}

	return true;
}

Act4Level4::Act4Level4() : Act4Level3()
{
	timeLeft = 60.0f;
}

bool Act4Level4::initLevel(const string& jsonPath)
{
	if (!loader4.loadJSON(jsonPath))
	{
		return false;
	}

	heap = MaxHeap(loader4.data.values, loader4.data.totalValues);

	if (sortedRef != NULL)
	{
		delete[] sortedRef;
		sortedRef = NULL;
	}

	MaxHeap copyHeap(loader4.data.values, loader4.data.totalValues);
	copyHeap.heapSort();

	sortedRef = new int[loader4.data.totalValues];
	for (int i = 0; i < loader4.data.totalValues; i++)
	{
		sortedRef[i] = copyHeap.getAt(i);
	}

	phase    = 0;
	timeLeft = (float)loader4.data.timeLimit;
	state    = LevelState(loader4.data.optimalSteps);

	return true;
}

void Act4Level4::tick(float dt)
{
	if (state.finished)
	{
		return;
	}

	timeLeft -= dt;
	if (timeLeft <= 0.0f)
	{
		timeLeft = 0.0f;
		// Trigger alarm full & level lost
		state.alarm = state.alarmCap;
		state.finished = true;
		state.won = false;
	}
}

float Act4Level4::getTimeLeft() const
{
	return timeLeft;
}
