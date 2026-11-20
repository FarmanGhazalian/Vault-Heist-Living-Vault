#include "backend/Act3Level3.h"
#include "backend/LevelGenerator.h"

bool Level3_3DataLoader::loadJSON(const string& filePath)
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
		data.levelId = 33;
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

Act3Level3::Act3Level3() : ILevelLogic(0)
{
	problem        = 0;
	rootNodeVal    = 0;
	childNodeVal   = 0;
	rootRotation   = 0;
	childRotation  = -1;
	phase          = 1;
	rotIdx         = 0;
	totalRotations = 1;
}

bool Act3Level3::initLevel(const string& jsonPath)
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

	rootNodeVal = unbalanced->val;
	problem     = avl.getProblem(unbalanced);

	// 0 = LL, 1 = RR, 2 = LR, 3 = RL
	// Direction: 0 = left, 1 = right
	if (problem == 0) // LL
	{
		rootRotation   = 1; // right rotation on root
		childRotation  = -1;
		childNodeVal   = 0;
		totalRotations = 1;
	}
	else if (problem == 1) // RR
	{
		rootRotation   = 0; // left rotation on root
		childRotation  = -1;
		childNodeVal   = 0;
		totalRotations = 1;
	}
	else if (problem == 2) // LR
	{
		childNodeVal   = unbalanced->left->val;
		childRotation  = 0; // left rotation on left child
		rootRotation   = 1; // right rotation on root
		totalRotations = 2;
	}
	else if (problem == 3) // RL
	{
		childNodeVal   = unbalanced->right->val;
		childRotation  = 1; // right rotation on right child
		rootRotation   = 0; // left rotation on root
		totalRotations = 2;
	}

	phase  = 1;
	rotIdx = 0;

	int dynSteps = 1 + totalRotations; // 1 step choose problem + totalRotations rotation steps
	state = LevelState(dynSteps);

	return true;
}

void Act3Level3::onValidStep()
{
	state.addStep();
}

void Act3Level3::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}

void Act3Level3::onHintRequested()
{
	state.useHint();
}

ActionResult Act3Level3::processAction(const PlayerAction& action)
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

		if (phase == 1)
		{
			string problems[4] = {"LL", "RR", "LR", "RL"};
			result.message = "Hint: Case is " + problems[problem] + ".";
		}
		else
		{
			int expNode = (rotIdx == 0 && totalRotations == 2) ? childNodeVal : rootNodeVal;
			int expDir  = (rotIdx == 0 && totalRotations == 2) ? childRotation : rootRotation;
			string dirStr = (expDir == 0) ? "LEFT" : "RIGHT";
			result.message = "Hint: Rotate node " + to_string(expNode) + " to the " + dirStr + ".";
		}
		return result;
	}

	if (phase == 1)
	{
		if (action.type == CHOOSE_CASE)
		{
			result.accepted = true;

			if (action.value == problem)
			{
				result.correct       = true;
				result.alarmDelta = 2;
				result.penaltyPoints = 0;
				onValidStep();
				phase          = 2;
				rotIdx         = 0;
				result.message = "Correct problem type! Now apply rotation(s).";
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
					result.message = "Wrong problem type! Check balance factors.";
				}
			}
			return result;
		}
	}
	else
	{
		if (action.type == ROTATE)
		{
			result.accepted = true;

			int expNode = (rotIdx == 0 && totalRotations == 2) ? childNodeVal : rootNodeVal;
			int expDir  = (rotIdx == 0 && totalRotations == 2) ? childRotation : rootRotation;

			if (action.nodeA == expNode && action.direction == expDir)
			{
				result.correct       = true;
				result.alarmDelta = 2;
				result.penaltyPoints = 0;
				avl.applyRotation(action.nodeA, action.direction);
				onValidStep();
				rotIdx++;

				if (rotIdx >= totalRotations)
				{
					state.finished = true;
					state.won      = true;
					result.message = "Tree balanced successfully! Level Cleared.";
				}
				else
				{
					result.message = "First rotation complete! Now apply second rotation.";
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
					result.message = "Wrong rotation! Check pivot node and direction.";
				}
			}
			return result;
		}
	}

	result.accepted = false;
	result.message  = "Unsupported action type!";
	return result;
}

TreeNode* Act3Level3::getTree()
{
	return avl.getRoot();
}

int Act3Level3::getPhase()
{
	return phase;
}

int Act3Level3::getProblem()
{
	return problem;
}