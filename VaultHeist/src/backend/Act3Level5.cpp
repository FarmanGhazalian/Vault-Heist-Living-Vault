#include "backend/Act3Level5.h"
#include "backend/LevelGenerator.h"

bool Level3_5DataLoader::loadJSON(const string& filePath)
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
	LevelGenerator::processJson(j);

	if (j["levelId"].is_string())
	{
		data.levelId = 35;
	}
	else
	{
		data.levelId = j["levelId"];
	}

	data.levelName    = j["levelName"];
	data.optimalSteps = j["optimalSteps"];
	data.k            = j["k"];

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

Act3Level5::Act3Level5() : ILevelLogic(0)
{
	expectedPath = NULL;
	pathLength   = 0;
	currentIndex = 0;
	targetVal    = 0;
}

Act3Level5::~Act3Level5()
{
	if (expectedPath != NULL)
	{
		delete[] expectedPath;
		expectedPath = NULL;
	}
}

bool Act3Level5::initLevel(const string& jsonPath)
{
	if (!loader.loadJSON(jsonPath))
	{
		return false;
	}

	avl.destroyTree();

	for (int i = 0; i < loader.data.totalKeys; i++)
	{
		avl.insert(loader.data.keys[i]);
	}

	avl.updateHeights();
	avl.updateSizes();

	TreeNode* targetNode = avl.findKth(loader.data.k);
	if (targetNode == NULL)
	{
		return false;
	}

	targetVal = targetNode->val;

	if (expectedPath != NULL)
	{
		delete[] expectedPath;
		expectedPath = NULL;
	}

	expectedPath = avl.pathTo(targetVal);
	if (expectedPath == NULL)
	{
		return false;
	}

	pathLength = 0;
	TreeNode* curr = avl.getRoot();
	while (curr != NULL)
	{
		pathLength++;
		if (curr->val == targetVal)
		{
			break;
		}
		if (targetVal < curr->val)
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

void Act3Level5::onValidStep()
{
	state.addStep();
}

void Act3Level5::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act3Level5::onHintRequested()
{
	state.useHint();
}

ActionResult Act3Level5::processAction(const PlayerAction& action)
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
			result.alarmDelta = 2;
			result.penaltyPoints = 0;
			onMistake(2, 0);

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

		if (currentIndex < pathLength)
		{
			result.message = "Hint: Next node on path to " + to_string(loader.data.k) +
				"-th smallest is " + to_string(expectedPath[currentIndex]);
		}
		else
		{
			result.message = "Hint: Target treasure node reached!";
		}
		return result;
	}

	if (action.type == CLICK_NODE)
	{
		result.accepted = true;

		if (currentIndex < pathLength && action.nodeA == expectedPath[currentIndex])
		{
			result.correct       = true;
			result.alarmDelta = 2;
			result.penaltyPoints = 0;
			onValidStep();
			currentIndex++;

			if (currentIndex >= pathLength)
			{
				state.finished = true;
				state.won      = true;
				result.message = "Treasure chamber unlocked! Act 3 Cleared.";
			}
			else
			{
				result.message = "Correct path step! Proceed deeper.";
			}
		}
		else
		{
			result.correct       = false;
			result.alarmDelta = 2;
			result.penaltyPoints = 0;
			onMistake(1, 0);

			if (state.isFull())
			{
				state.finished = true;
				state.won      = false;
				result.message = "ALARM CAP REACHED! Vault Lockdown Initiated.";
			}
			else
			{
				result.message = "Wrong step along the k-th path! Alarm Increased (+2).";
			}
		}
		return result;
	}

	result.accepted = false;
	result.message  = "Unsupported action type!";
	return result;
}

TreeNode* Act3Level5::getTree()
{
	return avl.getRoot();
}

int Act3Level5::getK()
{
	return loader.data.k;
}

int Act3Level5::getTargetVal()
{
	return targetVal;
}

int Act3Level5::getCurrentIndex()
{
	return currentIndex;
}

int Act3Level5::getPathLength()
{
	return pathLength;
}