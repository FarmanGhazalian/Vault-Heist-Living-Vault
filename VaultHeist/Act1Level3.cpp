#include<iostream>
#include"Act1Level3.h"


void L3_data::generate()
{
	preArr = new int[totalNodes];
	inArr = new int[totalNodes];
	return;
}

bool Level3DataLoader::loadJSON(const string& filePath)
{
	ifstream file(filePath);
	if (!file.is_open())
	{
		return false;
	}

	json j;
	file >> j;

	data.levelID = j["levelId"];
	data.levelName = j["levelName"];
	data.totalNodes = j["totalNodes"];
	data.optimalSteps = j["optimalSteps"];
	data.generate();

	for (int i = 0; i < data.totalNodes; i++)
	{
		data.inArr[i] = j["inorderSequence"][i].get<int>();
	}

	for (int i = 0; i < data.totalNodes; i++)
	{
		data.preArr[i] = j["preorderSequence"][i].get<int>();
	}
	file.close();
	return true;
}


int Act1Level3::findIndex(int nodeVal)
{
	for (int i = 0;i < totalNodes;i++)
	{
		if (loader.data.preArr[i] == nodeVal)
		{
			return i;
		}
	}
	return -1;
}


void Act1Level3::generate()
{
	actualHeight = new int[totalNodes];
	playerHeight = new int[totalNodes];
	for (int i = 0;i < totalNodes;i++)
	{
		actualHeight[i] = -1;
		playerHeight[i] = -1;
	}
	return;
}


TreeNode* Act1Level3::GenerateH(TreeNode* root, int index)
{
	if (root == NULL)
	{
		return NULL;
	}
	int idx = findIndex(root->val);
	if (idx >= 0 && idx < totalNodes)
	{
		actualHeight[idx] = solTree.TreeHeight(root);
	}
	GenerateH(root->left, 0);
	GenerateH(root->right, 0);
	return root;
}

void Act1Level3::genrateHeight()
{
	GenerateH(solTree.getRoot(), 0);
	return;
}

bool Act1Level3::initLevel(const string& jsonPath)
{
	if (!loader.loadJSON(jsonPath))
	{
		return false;
	}
	totalNodes = loader.data.totalNodes;
	solTree.buildFromPreIn(loader.data.preArr, loader.data.inArr, totalNodes);
	state = LevelState(loader.data.optimalSteps);
	generate();
	genrateHeight();
	actualDiameter = solTree.diameter();
	return true;
}


void Act1Level3::onValidStep()
{
	state.addStep();
}

void Act1Level3::onMistake(int extraAlarm, int penaltyPoints)
{
	state.addpenalty(extraAlarm, penaltyPoints);
}
void Act1Level3::onHintRequested()
{
	state.useHint();
}
string Act1Level3::getHint()
{
	return "Hint: Start from the bottom leaves  (height 0) and work upward—each parent's height is 1 + max(left, right). Once all rooms are solved, report the longest path (Diameter) between any two nodes.";
}


ActionResult Act1Level3::processAction(const PlayerAction& action)
{
	ActionResult res;
	if (state.finished)
	{
		return res;
	}

	if (action.type == CLICK_HINT)
	{
		if (state.hintUsed)
		{
			res.correct = false;
			res.accepted = false;
			res.message = "Hint already Used!";
			onMistake(2, 50);
			res.penaltyPoints = 50;
			res.alarmDelta = 2;
		}
		else
		{
			res.correct = true;
			res.accepted = false;
			res.message = getHint();
			onHintRequested();
		}

		if (!state.won && state.alarm >= state.alarmCap) {
			state.finished = true;
			state.won = false;
			res.message = "Alarm Triggered! Vault Lockdown!";
		}
		return res;
	}
	if(action.type == ENTER_HEIGHT)
	{
		currIndex = findIndex(action.nodeA);
		if (currIndex == -1)
		{
			onMistake(2, 50);
			res.penaltyPoints = 50;
			res.alarmDelta = 2;
			res.message = "Node not found";
			return res;
		}

		if (playerHeight[currIndex] != -1)
		{
			onMistake(2, 50);
			res.penaltyPoints = 50;
			res.alarmDelta = 2;
			res.correct = true;
			res.message = "Already solved!";
			return res;
		}

		leftIndex = 2 * currIndex + 1;
		rightIndex = 2 * currIndex + 2;

		bool leftReady = (leftIndex >= totalNodes) || (playerHeight[leftIndex] != -1);
		bool rightReady = (rightIndex >= totalNodes) || (playerHeight[rightIndex] != -1);

		bool ready = (leftReady && rightReady);

		if (!ready) {
			res.message = "Wrong Order! Solve children nodes first!";
		}
		else if (action.nodeB != actualHeight[currIndex])
		{
			onMistake(2, 50);
			res.penaltyPoints = 50;
			res.alarmDelta = 2;
			res.message = "Incorrect Height! Alarm increased!";
		}
		else
		{
			// Sahi height enter hui! Player ke array mein save kar dein
			playerHeight[currIndex] = action.nodeB;
			solvedCount++;
			onValidStep();

			if (solvedCount == totalNodes)
			{
				heightsPhaseDone = true;
				res.message = "All heights verified! Phase 2: Enter the Tree's Diameter.";
			}
			else {
				res.message = "Height verified!";
			}
		}
	}
	else if (heightsPhaseDone && action.type == ENTER_DIAMETER)
	{
		if (action.nodeB == actualDiameter)
		{
			res.message = "Diameter Verified! Alarm Neutralized!";
			state.finished = true;
			state.won = true;
			onValidStep();

		}
		else {

			res.accepted = false;
			res.correct = false;
			res.alarmDelta = 2;
			res.penaltyPoints = 50;
			res.message = "Wrong Diameter! Recalculate the longest path!";
		}

	}
	if (!isWon() && state.alarm >= state.alarmCap) {
		state.finished = true;
		state.won = false;
		res.message = "Alarm Reached the Root! Vault Lockdown!";
	}
	return res;
}