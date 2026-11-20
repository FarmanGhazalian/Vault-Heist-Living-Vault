#include "Act2Level4.h"

bool Level2_4DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 24;
	}
	else
	{
		data.levelId = j["levelId"];
	}

	data.levelName = j["levelName"];
	data.optimalSteps = j["optimalSteps"];
	data.lo = j["lo"];
	data.hi = j["hi"];

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

Act2Level4::Act2Level4() : ILevelLogic(0)
{
	rangeValues = NULL;
	collected = NULL;
	rangeCount = 0;
	collectedCount = 0;
}

Act2Level4::~Act2Level4()
{
	if (rangeValues != NULL)
	{
		delete[] rangeValues;
		rangeValues = NULL;
	}
	if (collected != NULL)
	{
		delete[] collected;
		collected = NULL;
	}
}

int Act2Level4::findValueIndex(int val)
{
	for (int i = 0; i < rangeCount; i++)
	{
		if (rangeValues[i] == val)
		{
			return i;
		}
	}
	return -1;
}

bool Act2Level4::initLevel(const string& jsonPath)
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

	if (rangeValues != NULL)
	{
		delete[] rangeValues;
		rangeValues = NULL;
	}
	if (collected != NULL)
	{
		delete[] collected;
		collected = NULL;
	}

	rangeValues = bst.rangeToArray(loader.data.lo, loader.data.hi, rangeCount);
	if (rangeCount > 0)
	{
		collected = new bool[rangeCount];
		for (int i = 0; i < rangeCount; i++)
		{
			collected[i] = false;
		}
	}

	int dynSteps = (rangeCount > 0) ? rangeCount : loader.data.optimalSteps;
	state = LevelState(dynSteps);
	collectedCount = 0;

	return true;
}

void Act2Level4::onValidStep()
{
	state.addStep();
}

void Act2Level4::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level4::onHintRequested()
{
	state.useHint();
}

ActionResult Act2Level4::processAction(const PlayerAction& action)
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
			result.alarmDelta = 2;
			result.penaltyPoints = 0;
			onMistake(2, 0);

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

		int remainingToFind = rangeCount - collectedCount;
		for (int i = 0; i < rangeCount; i++)
		{
			if (!collected[i])
			{
				result.message = "Hint: Node " + to_string(rangeValues[i]) + " is within range (" + to_string(remainingToFind) + " left).";
				return result;
			}
		}

		result.message = "Hint: All range nodes already collected!";
		return result;
	}

	if (action.type == CLICK_NODE)
	{
		result.accepted = true;
		int idx = findValueIndex(action.nodeA);

		if (idx != -1)
		{
			if (!collected[idx])
			{
				collected[idx] = true;
				collectedCount++;

				result.correct = true;
				result.alarmDelta = 2;
				result.penaltyPoints = 0;

				onValidStep();

				if (collectedCount >= rangeCount)
				{
					state.finished = true;
					state.won = true;
					result.message = "Range Raid Complete! All loot in range secured.";
				}
				else
				{
					result.message = "Secured " + to_string(action.nodeA) + "! (" + to_string(collectedCount) + "/" + to_string(rangeCount) + " collected)";
				}
			}
			else
			{
				result.correct = false;
				result.alarmDelta = 0;
				result.penaltyPoints = 0;
				result.message = "Node " + to_string(action.nodeA) + " already collected!";
			}
		}
		else
		{
			result.correct = false;
			result.alarmDelta = 2;
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
				result.message = "Node " + to_string(action.nodeA) + " is outside range [" + to_string(loader.data.lo) + ", " + to_string(loader.data.hi) + "]! Alarm Increased (+2).";
			}
		}
		return result;
	}

	result.accepted = false;
	result.message = "Unsupported action type! Expected CLICK_NODE (0).";
	return result;
}

int Act2Level4::getLo()
{
	return loader.data.lo;
}

int Act2Level4::getHi()
{
	return loader.data.hi;
}

int Act2Level4::getCollectedCount()
{
	return collectedCount;
}

int Act2Level4::getTotalToCollect()
{
	return rangeCount;
}

TreeNode* Act2Level4::getTree()
{
	return bst.getRoot();
}

bool Act2Level4::isNodeSkipped(int value)
{
	return !bst.isVisitedByPrunedRange(value, loader.data.lo, loader.data.hi);
}

static int countVisitedHelper(BST& bst, TreeNode* node, int lo, int hi)
{
	if (node == NULL) return 0;
	int cnt = bst.isVisitedByPrunedRange(node->val, lo, hi) ? 1 : 0;
	return cnt + countVisitedHelper(bst, node->left, lo, hi) + countVisitedHelper(bst, node->right, lo, hi);
}

int Act2Level4::getVisitedByPruningCount()
{
	return countVisitedHelper(bst, bst.getRoot(), loader.data.lo, loader.data.hi);
}