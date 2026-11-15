#include "Act2Level2.h"

bool Level2_2DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 22;
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

	auto insArr = j["insertKeys"];
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

Act2Level2::Act2Level2() : ILevelLogic(0)
{
	currentKeyIdx = 0;
	currentPath = NULL;
	currentPathLength = 0;
	currentPathIdx = 0;
}

Act2Level2::~Act2Level2()
{
	if (currentPath != NULL)
	{
		delete[] currentPath;
		currentPath = NULL;
	}
}

void Act2Level2::computeCurrentPath()
{
	if (currentPath != NULL)
	{
		delete[] currentPath;
		currentPath = NULL;
	}

	currentPathLength = 0;
	currentPathIdx = 0;

	if (currentKeyIdx < loader.data.totalInsertKeys)
	{
		int key = loader.data.insertKeys[currentKeyIdx];
		currentPath = bst.getInsertPath(key, currentPathLength);
	}
}

bool Act2Level2::initLevel(const string& jsonPath)
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
	currentKeyIdx = 0;
	computeCurrentPath();

	return true;
}

void Act2Level2::onValidStep()
{
	state.addStep();
}

void Act2Level2::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level2::onHintRequested()
{
	state.useHint();
}

ActionResult Act2Level2::processAction(const PlayerAction& action)
{
	ActionResult result;

	if (isFinished() || state.isFull())
	{
		result.accepted = false;
		result.correct = false;
		result.message = "Level locked or already finished!";
		return result;
	}

	if (currentKeyIdx >= loader.data.totalInsertKeys)
	{
		result.accepted = false;
		result.message = "All keys already inserted!";
		return result;
	}

	int currentKey = loader.data.insertKeys[currentKeyIdx];

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

		if (currentPathIdx < currentPathLength)
		{
			result.message = "Hint: Click node " + to_string(currentPath[currentPathIdx]);
		}
		else
		{
			int parentVal = currentPath[currentPathLength - 1];
			string dirStr = (currentKey < parentVal) ? "LEFT" : "RIGHT";
			result.message = "Hint: Attach " + to_string(currentKey) + " as " + dirStr + " of " + to_string(parentVal);
		}
		return result;
	}

	if (action.type == CLICK_NODE)
	{
		result.accepted = true;

		if (currentPathIdx < currentPathLength)
		{
			if (action.nodeA == currentPath[currentPathIdx])
			{
				result.correct = true;
				result.alarmDelta = 1;
				result.penaltyPoints = 0;

				onValidStep();
				currentPathIdx++;

				if (currentPathIdx == currentPathLength)
				{
					result.message = "Reached insertion leaf! Place key using Left/Right.";
				}
				else
				{
					result.message = "Correct path step! Move to next node.";
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
					result.message = "Wrong node on insertion path! Alarm Increased (+2).";
				}
			}
		}
		else
		{
			result.correct = false;
			result.message = "Already at leaf! Use Place Token.";
		}
		return result;
	}

	if (action.type == PLACE_TOKEN)
	{
		result.accepted = true;

		if (currentPathIdx < currentPathLength)
		{
			result.correct = false;
			result.message = "Patrol path not completed yet! Click path nodes first.";
			return result;
		}

		int parentVal = currentPath[currentPathLength - 1];
		int expectedDir = (currentKey < parentVal) ? 0 : 1;

		if (action.direction == expectedDir)
		{
			result.correct = true;
			result.alarmDelta = 1;
			result.penaltyPoints = 0;

			onValidStep();

			bool inserted = bst.insert(currentKey);
			if (!inserted)
			{
				result.message = "Duplicate key " + to_string(currentKey) + " rejected! Key skipped.";
			}
			else
			{
				result.message = "Key " + to_string(currentKey) + " stashed successfully!";
			}

			currentKeyIdx++;

			if (currentKeyIdx >= loader.data.totalInsertKeys)
			{
				state.finished = true;
				state.won = true;
				result.message = "All Loot Stashed! Level Cleared.";
			}
			else
			{
				computeCurrentPath();
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
				result.message = "Wrong direction! BST invariant violated (+2 Alarm).";
			}
		}
		return result;
	}

	result.accepted = false;
	result.message = "Unsupported action type!";
	return result;
}

int Act2Level2::getCurrentKey()
{
	if (currentKeyIdx < loader.data.totalInsertKeys)
	{
		return loader.data.insertKeys[currentKeyIdx];
	}
	return -1;
}

int Act2Level2::getCurrentPathIndex()
{
	return currentPathIdx;
}

int Act2Level2::getCurrentPathLength()
{
	return currentPathLength;
}

int Act2Level2::getCompletedInsertions()
{
	return currentKeyIdx;
}

int Act2Level2::getTotalInsertions()
{
	return loader.data.totalInsertKeys;
}

TreeNode* Act2Level2::getTreeRoot()
{
	return bst.getRoot();
}
