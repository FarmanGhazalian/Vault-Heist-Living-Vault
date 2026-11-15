#include "Act2Level5.h"

bool Level2_5DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 25;
	}
	else
	{
		data.levelId = j["levelId"];
	}

	data.levelName = j["levelName"];
	data.optimalSteps = j["optimalSteps"];

	auto keysArr = j["keys"];
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

	auto targetsArr = j["targets"];
	data.totalTargets = (int)targetsArr.size();

	if (data.targets != NULL)
	{
		delete[] data.targets;
		data.targets = NULL;
	}

	data.targets = new int[data.totalTargets];
	for (int i = 0; i < data.totalTargets; i++)
	{
		data.targets[i] = targetsArr[i];
	}

	return true;
}

Act2Level5::Act2Level5() : ILevelLogic(0)
{
	currentTargetIdx = 0;
	currentPath = NULL;
	currentPathLength = 0;
	currentPathIdx = 0;
	validStepsSinceLastShrink = 0;
}

Act2Level5::~Act2Level5()
{
	if (currentPath != NULL)
	{
		delete[] currentPath;
		currentPath = NULL;
	}
}

void Act2Level5::computeCurrentPath()
{
	if (currentPath != NULL)
	{
		delete[] currentPath;
		currentPath = NULL;
	}

	currentPathLength = 0;
	currentPathIdx = 0;

	if (currentTargetIdx < loader.data.totalTargets)
	{
		int target = loader.data.targets[currentTargetIdx];
		currentPath = bst.pathTo(target);

		if (currentPath != NULL)
		{
			TreeNode* curr = bst.getRoot();
			while (curr != NULL)
			{
				currentPathLength++;
				if (curr->val == target)
				{
					break;
				}
				if (target < curr->val)
				{
					curr = curr->left;
				}
				else
				{
					curr = curr->right;
				}
			}
		}
	}
}

bool Act2Level5::initLevel(const string& jsonPath)
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

	state = LevelState(loader.data.optimalSteps);
	currentTargetIdx = 0;
	validStepsSinceLastShrink = 0;
	computeCurrentPath();

	return true;
}

void Act2Level5::onValidStep()
{
	state.addStep();
	validStepsSinceLastShrink++;

	if (validStepsSinceLastShrink >= 2)
	{
		state.reduceCapacity(1);
		validStepsSinceLastShrink = 0;
	}
}

void Act2Level5::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level5::onHintRequested()
{
	state.useHint();
}

ActionResult Act2Level5::processAction(const PlayerAction& action)
{
	ActionResult result;

	if (isFinished() || state.isFull())
	{
		result.accepted = false;
		result.correct = false;
		result.message = "Level locked or already finished!";
		return result;
	}

	if (currentTargetIdx >= loader.data.totalTargets)
	{
		result.accepted = false;
		result.message = "All boss targets already secured!";
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

		if (currentPathIdx < currentPathLength)
		{
			result.message = "Hint: Next corridor node is " + to_string(currentPath[currentPathIdx]);
		}
		else
		{
			result.message = "Hint: Current target reached!";
		}
		return result;
	}

	if (action.type == CLICK_NODE)
	{
		result.accepted = true;

		if (currentPathIdx < currentPathLength && action.nodeA == currentPath[currentPathIdx])
		{
			result.correct = true;
			result.alarmDelta = 1;
			result.penaltyPoints = 0;

			onValidStep();
			currentPathIdx++;

			if (currentPathIdx >= currentPathLength)
			{
				currentTargetIdx++;

				if (currentTargetIdx >= loader.data.totalTargets)
				{
					state.finished = true;
					state.won = true;
					result.message = "Boss Defeated! The Lazy Architect's corridor breached.";
				}
				else
				{
					computeCurrentPath();
					result.message = "Target secured! Next corridor target: " + to_string(loader.data.targets[currentTargetIdx]);
				}
			}
			else
			{
				result.message = "Correct corridor step! Capacity left: " + to_string(getCapacityLeft());
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
				result.message = "ALARM CAP REACHED! Architect locked down the corridor.";
			}
			else
			{
				result.message = "Wrong corridor node! Alarm Increased (+2).";
			}
		}
		return result;
	}

	result.accepted = false;
	result.message = "Unsupported action type! Expected CLICK_NODE (0).";
	return result;
}

int Act2Level5::getCurrentTarget()
{
	if (currentTargetIdx < loader.data.totalTargets)
	{
		return loader.data.targets[currentTargetIdx];
	}
	return -1;
}

int Act2Level5::getCurrentPathIndex()
{
	return currentPathIdx;
}

int Act2Level5::getCurrentPathLength()
{
	return currentPathLength;
}

int Act2Level5::getHeight()
{
	return bst.height();
}

int Act2Level5::getCapacityLeft()
{
	return state.remaining();
}

int Act2Level5::getCompletedTargets()
{
	return currentTargetIdx;
}

int Act2Level5::getTotalTargets()
{
	return loader.data.totalTargets;
}

TreeNode* Act2Level5::getTreeRoot()
{
	return bst.getRoot();
}
