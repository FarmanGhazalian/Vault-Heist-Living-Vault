#include "Act2Level1.h"

bool Level2_1DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 21;
	}
	else
	{
		data.levelId = j["levelId"];
	}

	data.levelName = j["levelName"];
	data.target = j["target"];
	data.optimalSteps = j["optimalSteps"];

	const json& keysArr = j["keys"];
	data.totalKeys = (int)keysArr.size();

	if (data.keys != NULL)
	{
		delete[] data.keys;
		data.keys = NULL;
	}

	data.keys = new int[data.totalKeys];
	for (int i = 0; i < data.totalKeys; i++)
	{
		data.keys[i] = keysArr[i];
	}

	return true;
}

Act2Level1::Act2Level1() : ILevelLogic(0)
{
	expectedPath = NULL;
	pathLength = 0;
	currentIndex = 0;
}

Act2Level1::~Act2Level1()
{
	if (expectedPath != NULL)
	{
		delete[] expectedPath;
		expectedPath = NULL;
	}
}

bool Act2Level1::initLevel(const string& jsonPath)
{
	if (!loader.loadJSON(jsonPath))
	{
		return false;
	}

	bst.destroyTree();

	for (int i = 0; i < loader.data.totalKeys; i++)
	{
		bst.insert(loader.data.keys[i]);
	}

	if (expectedPath != NULL)
	{
		delete[] expectedPath;
		expectedPath = NULL;
	}

	expectedPath = bst.pathTo(loader.data.target);
	if (expectedPath == NULL)
	{
		return false;
	}

	pathLength = 0;
	TreeNode* curr = bst.getRoot();
	while (curr != NULL)
	{
		pathLength++;
		if (curr->val == loader.data.target)
		{
			break;
		}
		if (loader.data.target < curr->val)
		{
			curr = curr->left;
		}
		else
		{
			curr = curr->right;
		}
	}

	int dynSteps = (pathLength > 0) ? pathLength : loader.data.optimalSteps;
	state = LevelState(dynSteps);
	currentIndex = 0;

	return true;
}

void Act2Level1::onValidStep()
{
	state.addStep();
}

void Act2Level1::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level1::onHintRequested()
{
	state.useHint();
}

ActionResult Act2Level1::processAction(const PlayerAction& action)
{
	ActionResult result;

	if (isFinished() || state.isFull())
	{
		result.accepted = false;
		result.correct = false;
		result.message = "Level locked or already finished!";
		return result;
	}

	if (action.type == CLICK_HINT)
	{
		result.accepted = true;

		if (state.hintUsed)
		{
			result.correct = false;
			result.alarmDelta = 1;
			result.penaltyPoints = 50;
			onMistake(1, 50);

			if (state.isFull())
			{
				state.finished = true;
				state.won = false;
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

		if (currentIndex < pathLength)
		{
			result.message = "Hint: Next patrol node is " + to_string(expectedPath[currentIndex]);
		}
		else
		{
			result.message = "Hint: Target reached!";
		}
		return result;
	}

	if (action.type == CLICK_NODE)
	{
		result.accepted = true;

		if (currentIndex < pathLength && action.nodeA == expectedPath[currentIndex])
		{
			result.correct = true;
			result.alarmDelta = 1;
			result.penaltyPoints = 0;

			onValidStep();
			currentIndex++;

			if (currentIndex >= pathLength)
			{
				state.finished = true;
				state.won = true;
				result.message = "Target Key Found! Search Heist Cleared.";
			}
			else
			{
				result.message = "Correct patrol step! Move to next node.";
			}
		}
		else
		{
			result.correct = false;
			result.alarmDelta = 1;
			result.penaltyPoints = 0;

			onMistake(1, 0);

			if (state.isFull())
			{
				state.finished = true;
				state.won = false;
				result.message = "ALARM CAP REACHED! Vault Lockdown Initiated.";
			}
			else
			{
				result.message = "Wrong patrol step! Alarm Increased (+2).";
			}
		}
		return result;
	}

	result.accepted = false;
	result.message = "Unsupported action type!";
	return result;
}

int Act2Level1::getTarget()
{
	return loader.data.target;
}

int Act2Level1::getCurrentIndex()
{
	return currentIndex;
}

int Act2Level1::getPathLength()
{
	return pathLength;
}

TreeNode* Act2Level1::getTreeRoot()
{
	return bst.getRoot();
}
