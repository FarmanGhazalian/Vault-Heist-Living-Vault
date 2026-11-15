#include "Act3Level2.h"

bool Level3_2DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 32;
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

Act3Level2::Act3Level2() : ILevelLogic(0)
{
	unbalancedNodeVal = 0;
	storedProblem     = 0;
	phase             = 1;
}

bool Act3Level2::initLevel(const string& jsonPath)
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

	TreeNode* unbalanced = avl.findUnbalancedNode();
	if (unbalanced == NULL)
	{
		return false;
	}

	unbalancedNodeVal = unbalanced->val;
	storedProblem     = avl.getProblem(unbalanced);
	phase             = 1;

	int dynSteps = 2; // 1 step select unbalanced node + 1 step choose rotation case
	state = LevelState(dynSteps);

	return true;
}

void Act3Level2::onValidStep()
{
	state.addStep();
}

void Act3Level2::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act3Level2::onHintRequested()
{
	state.useHint();
}

ActionResult Act3Level2::processAction(const PlayerAction& action)
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

		if (phase == 1)
		{
			result.message = "Hint: Click the unbalanced node (" + to_string(unbalancedNodeVal) + ").";
		}
		else
		{
			string problems[4] = {"LL", "RR", "LR", "RL"};
			result.message = "Hint: The rotation type is " + problems[storedProblem] + ".";
		}
		return result;
	}

	if (phase == 1)
	{
		if (action.type == CLICK_NODE)
		{
			result.accepted = true;

			if (action.nodeA == unbalancedNodeVal)
			{
				result.correct       = true;
				result.alarmDelta    = 1;
				result.penaltyPoints = 0;
				onValidStep();
				phase          = 2;
				result.message = "Correct! Now identify the rotation type (0=LL, 1=RR, 2=LR, 3=RL).";
			}
			else
			{
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
					result.message = "Wrong node! Find the unbalanced one.";
				}
			}
			return result;
		}
	}
	else
	{
		if (action.type == CHOOSE_CASE)
		{
			result.accepted = true;

			if (action.value == storedProblem)
			{
				result.correct       = true;
				result.alarmDelta    = 1;
				result.penaltyPoints = 0;
				onValidStep();
				state.finished = true;
				state.won      = true;
				result.message = "Correct rotation type! Problem Identified. Level Cleared.";
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
					result.message = "Wrong rotation type! Try again.";
				}
			}
			return result;
		}
	}

	result.accepted = false;
	result.message  = "Unsupported action type!";
	return result;
}

TreeNode* Act3Level2::getTree()
{
	return avl.getRoot();
}

int Act3Level2::getPhase()
{
	return phase;
}

int Act3Level2::getProblem()
{
	return storedProblem;
}
