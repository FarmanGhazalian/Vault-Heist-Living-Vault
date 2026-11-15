#include "Act3Level4.h"

bool Level3_4DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 34;
	}
	else
	{
		data.levelId = j["levelId"];
	}

	data.levelName    = j["levelName"];
	data.optimalSteps = j["optimalSteps"];

	auto baseArr      = j["keys"];
	data.totalBaseKeys = (int)baseArr.size();

	if (data.baseKeys != NULL)
	{
		delete[] data.baseKeys;
		data.baseKeys = NULL;
	}

	data.baseKeys = new int[data.totalBaseKeys];
	for (int i = 0; i < data.totalBaseKeys; i++)
	{
		data.baseKeys[i] = baseArr[i];
	}

	auto insArr         = j["insertKeys"];
	data.totalInsertKeys = (int)insArr.size();

	if (data.insertKeys != NULL)
	{
		delete[] data.insertKeys;
		data.insertKeys = NULL;
	}

	data.insertKeys = new int[data.totalInsertKeys];
	for (int i = 0; i < data.totalInsertKeys; i++)
	{
		data.insertKeys[i] = insArr[i];
	}

	return true;
}

Act3Level4::Act3Level4() : ILevelLogic(0)
{
	currentRound = 0;
	storedAnswer = 4;
}

void Act3Level4::setupRound()
{
	avl.destroyTree();

	// Insert base keys
	for (int i = 0; i < loader.data.totalBaseKeys; i++)
	{
		avl.insert(loader.data.baseKeys[i]);
	}

	// Insert current candidate key
	if (currentRound < loader.data.totalInsertKeys)
	{
		avl.insert(loader.data.insertKeys[currentRound]);
	}

	avl.updateHeights();

	TreeNode* unbalanced = avl.findUnbalancedNode();
	if (unbalanced != NULL)
	{
		storedAnswer = avl.getProblem(unbalanced);
	}
	else
	{
		storedAnswer = 4; // no rotation needed
	}
}

bool Act3Level4::initLevel(const string& jsonPath)
{
	if (!loader.loadJSON(jsonPath))
	{
		return false;
	}

	currentRound = 0;
	setupRound();

	state = LevelState(loader.data.optimalSteps);

	return true;
}

void Act3Level4::onValidStep()
{
	state.addStep();
}

void Act3Level4::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act3Level4::onHintRequested()
{
	state.useHint();
}

ActionResult Act3Level4::processAction(const PlayerAction& action)
{
	ActionResult result;

	if (isFinished() || state.isFull())
	{
		result.accepted = false;
		result.correct  = false;
		result.message  = "Level locked or already finished!";
		return result;
	}

	if (action.type == CLICK_HINT)
	{
		result.accepted = true;

		if (state.hintUsed)
		{
			result.correct       = false;
			result.alarmDelta    = 1;
			result.penaltyPoints = 50;
			onMistake(1, 50);

			if (state.isFull())
			{
				state.finished = true;
				state.won      = false;
				result.message = "Hint already used. ALARM CAP REACHED!";
			}
			else
			{
				result.message = "Hint already used.";
			}
			return result;
		}

		result.correct = true;
		onHintRequested();

		string cases[5] = {"LL", "RR", "LR", "RL", "NONE"};
		result.message = "Hint: If we insert " + to_string(loader.data.insertKeys[currentRound]) +
			", rotation required is " + cases[storedAnswer] + ".";
		return result;
	}

	if (action.type == CHOOSE_CASE)
	{
		result.accepted = true;

		if (action.value == storedAnswer)
		{
			result.correct       = true;
			result.alarmDelta    = 1;
			result.penaltyPoints = 0;
			onValidStep();
			currentRound++;

			if (currentRound >= loader.data.totalInsertKeys)
			{
				state.finished = true;
				state.won      = true;
				result.message = "All predictions correct! Level Cleared.";
			}
			else
			{
				setupRound();
				result.message = "Prediction correct! Next candidate ready.";
			}
		}
		else
		{
			result.correct       = false;
			result.alarmDelta    = 2;
			result.penaltyPoints = 50;
			onMistake(2, 50);

			if (state.isFull())
			{
				state.finished = true;
				state.won      = false;
				result.message = "ALARM CAP REACHED! Vault Lockdown Initiated.";
			}
			else
			{
				result.message = "Incorrect rotation prediction! Check subtree balance.";
			}
		}
		return result;
	}

	result.accepted = false;
	result.message  = "Unsupported action type!";
	return result;
}

TreeNode* Act3Level4::getTree()
{
	return avl.getRoot();
}

int Act3Level4::getCurrentRound()
{
	return currentRound;
}

int Act3Level4::getTotalRounds()
{
	return loader.data.totalInsertKeys;
}

int Act3Level4::getCurrentInsertKey()
{
	if (currentRound < loader.data.totalInsertKeys)
	{
		return loader.data.insertKeys[currentRound];
	}
	return -1;
}
