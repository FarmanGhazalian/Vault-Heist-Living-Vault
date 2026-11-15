#include "Act2Level3.h"

bool Level2_3DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 23;
	}
	else
	{
		data.levelId = j["levelId"];
	}

	data.levelName = j["levelName"];
	data.optimalSteps = j["optimalSteps"];

	auto initArr = j["initialKeys"];
	data.totalInitialKeys = (int)initArr.size();

	if (data.initialKeys != NULL)
	{
		delete[] data.initialKeys;
		data.initialKeys = NULL;
	}

	data.initialKeys = new int[data.totalInitialKeys];
	for (int i = 0; i < data.totalInitialKeys; i++)
	{
		data.initialKeys[i] = initArr[i];
	}

	auto delArr = j["deleteKeys"];
	data.totalDeleteKeys = (int)delArr.size();

	if (data.deleteKeys != NULL)
	{
		delete[] data.deleteKeys;
		data.deleteKeys = NULL;
	}

	data.deleteKeys = new int[data.totalDeleteKeys];
	for (int i = 0; i < data.totalDeleteKeys; i++)
	{
		data.deleteKeys[i] = delArr[i];
	}

	return true;
}

Act2Level3::Act2Level3() : ILevelLogic(0)
{
	currentTargetIdx = 0;
	lastDeletedKey = -1;
	lastDeleteCase = -1;
	lastSuccessorKey = -1;
}

Act2Level3::~Act2Level3()
{
}

bool Act2Level3::initLevel(const string& jsonPath)
{
	if (!loader.loadJSON(jsonPath))
	{
		return false;
	}

	bst.destroyTree();

	for (int i = 0; i < loader.data.totalInitialKeys; i++)
	{
		bst.insert(loader.data.initialKeys[i]);
	}

	state = LevelState(loader.data.optimalSteps);
	currentTargetIdx = 0;
	lastDeletedKey = -1;
	lastDeleteCase = -1;
	lastSuccessorKey = -1;

	return true;
}

void Act2Level3::onValidStep()
{
	state.addStep();
}

void Act2Level3::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level3::onHintRequested()
{
	state.useHint();
}

ActionResult Act2Level3::processAction(const PlayerAction& action)
{
	ActionResult result;

	if (isFinished() || state.isFull())
	{
		result.accepted = false;
		result.correct = false;
		result.message = "Level locked or already finished!";
		return result;
	}

	if (currentTargetIdx >= loader.data.totalDeleteKeys)
	{
		result.accepted = false;
		result.message = "All targets already stolen and collapsed!";
		return result;
	}

	int currentTarget = loader.data.deleteKeys[currentTargetIdx];

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

		int expectedCase = bst.deleteCase(currentTarget);
		string caseName = "unknown";
		if (expectedCase == 0)
		{
			caseName = "Leaf (0 children)";
		}
		else if (expectedCase == 1)
		{
			caseName = "One child (1 child)";
		}
		else if (expectedCase == 2)
		{
			caseName = "Two children (2 children)";
		}

		result.message = "Hint: Target " + to_string(currentTarget) + " is a " + caseName + " node.";
		return result;
	}

	if (action.type == CHOOSE_CASE)
	{
		result.accepted = true;
		int expectedCase = bst.deleteCase(currentTarget);

		if (action.value == expectedCase)
		{
			result.correct = true;
			result.alarmDelta = 1;
			result.penaltyPoints = 0;

			onValidStep();

			lastDeletedKey = currentTarget;
			lastDeleteCase = expectedCase;
			lastSuccessorKey = -1;

			if (expectedCase == 2)
			{
				TreeNode* targetNode = bst.findNode(currentTarget);
				TreeNode* succ = bst.inorderSuccessor(targetNode);
				if (succ != NULL)
				{
					lastSuccessorKey = succ->val;
				}
			}

			bst.remove(currentTarget);
			currentTargetIdx++;

			if (currentTargetIdx >= loader.data.totalDeleteKeys)
			{
				state.finished = true;
				state.won = true;
				result.message = "All targets stolen! Tree collapsed successfully.";
			}
			else
			{
				result.message = "Correct deletion case! Target collapsed. Next target: " + to_string(loader.data.deleteKeys[currentTargetIdx]);
			}
		}
		else
		{
			result.correct = false;
			result.alarmDelta = 2;
			result.penaltyPoints = 50;

			onMistake(2, 50);

			if (state.isFull())
			{
				state.finished = true;
				state.won = false;
				result.message = "ALARM CAP REACHED! Vault Lockdown Initiated.";
			}
			else
			{
				result.message = "Wrong deletion case chosen! Vault structural stress (+2 Alarm).";
			}
		}
		return result;
	}

	result.accepted = false;
	result.message = "Unsupported action type! Expected CHOOSE_CASE (11).";
	return result;
}

int Act2Level3::getCurrentTarget()
{
	if (currentTargetIdx < loader.data.totalDeleteKeys)
	{
		return loader.data.deleteKeys[currentTargetIdx];
	}
	return -1;
}

int Act2Level3::getCompletedDeletions()
{
	return currentTargetIdx;
}

int Act2Level3::getTotalDeletions()
{
	return loader.data.totalDeleteKeys;
}

int Act2Level3::getLastDeletedKey()
{
	return lastDeletedKey;
}

int Act2Level3::getLastDeleteCase()
{
	return lastDeleteCase;
}

int Act2Level3::getLastSuccessorKey()
{
	return lastSuccessorKey;
}

TreeNode* Act2Level3::getTreeRoot()
{
	return bst.getRoot();
}
