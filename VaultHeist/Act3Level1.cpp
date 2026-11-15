#include "Act3Level1.h"

bool Level3_1DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 31;
	}
	else
	{
		data.levelId = j["levelId"];
	}

	data.levelName    = j["levelName"];
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

Act3Level1::Act3Level1() : ILevelLogic(0)
{
	unbalancedNodes = NULL;
	found           = NULL;
	totalUnbalanced = 0;
	foundCount      = 0;
}

Act3Level1::~Act3Level1()
{
	if (unbalancedNodes != NULL)
	{
		delete[] unbalancedNodes;
		unbalancedNodes = NULL;
	}
	if (found != NULL)
	{
		delete[] found;
		found = NULL;
	}
}

int Act3Level1::findValueIndex(int val)
{
	for (int i = 0; i < totalUnbalanced; i++)
	{
		if (unbalancedNodes[i] == val)
		{
			return i;
		}
	}
	return -1;
}

bool Act3Level1::initLevel(const string& jsonPath)
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

	if (unbalancedNodes != NULL)
	{
		delete[] unbalancedNodes;
		unbalancedNodes = NULL;
	}
	if (found != NULL)
	{
		delete[] found;
		found = NULL;
	}

	unbalancedNodes = avl.getUnbalancedNodes(totalUnbalanced);
	found           = new bool[totalUnbalanced];
	for (int i = 0; i < totalUnbalanced; i++)
	{
		found[i] = false;
	}
	foundCount = 0;

	int dynSteps = (totalUnbalanced > 0) ? totalUnbalanced : loader.data.optimalSteps;
	state = LevelState(dynSteps);

	return true;
}

void Act3Level1::onValidStep()
{
	state.addStep();
}

void Act3Level1::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act3Level1::onHintRequested()
{
	state.useHint();
}

ActionResult Act3Level1::processAction(const PlayerAction& action)
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

		// Find first unbalanced node not yet found
		int hintVal = -1;
		for (int i = 0; i < totalUnbalanced; i++)
		{
			if (!found[i])
			{
				hintVal = unbalancedNodes[i];
				break;
			}
		}

		if (hintVal != -1)
		{
			result.message = "Hint: One unbalanced node has value " + to_string(hintVal);
		}
		else
		{
			result.message = "Hint: All unbalanced nodes found!";
		}
		return result;
	}

	if (action.type == CLICK_NODE)
	{
		result.accepted = true;

		int idx = findValueIndex(action.nodeA);

		if (idx == -1)
		{
			// Not an unbalanced node
			result.correct       = false;
			result.alarmDelta    = 1;
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
				result.message = "Node " + to_string(action.nodeA) + " is balanced. Wrong pick!";
			}
			return result;
		}

		if (found[idx])
		{
			// Already clicked — no penalty, just reject
			result.correct  = false;
			result.message  = "Node " + to_string(action.nodeA) + " already marked!";
			return result;
		}

		// Correct unbalanced node, not yet found
		result.correct       = true;
		result.alarmDelta    = 1;
		result.penaltyPoints = 0;

		found[idx] = true;
		foundCount++;
		onValidStep();

		if (foundCount >= totalUnbalanced)
		{
			state.finished = true;
			state.won      = true;
			result.message = "All unbalanced badges found! Level Cleared.";
		}
		else
		{
			result.message = "Badge found! " + to_string(totalUnbalanced - foundCount) + " more to go.";
		}
		return result;
	}

	result.accepted = false;
	result.message  = "Unsupported action type!";
	return result;
}

TreeNode* Act3Level1::getTree()
{
	return avl.getRoot();
}

int Act3Level1::getFoundCount()
{
	return foundCount;
}

int Act3Level1::getTotalToFind()
{
	return totalUnbalanced;
}

int Act3Level1::getBalance(TreeNode* node)
{
	return avl.getBalance(node);
}
