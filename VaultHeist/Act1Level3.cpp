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

	if (j.contains("levelId"))
	{
		if (j["levelId"].is_number())
			data.levelID = j["levelId"].get<double>();
		else if (j["levelId"].is_string())
		{
			try { data.levelID = std::stod(j["levelId"].get<std::string>()); } catch (...) { data.levelID = 1.3; }
		}
	}
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


static int calculateNodeHeight(TreeNode* node)
{
	if (node == NULL) return -1;
	if (node->left == NULL && node->right == NULL) return 0; // Leaves always have height 0
	int lh = calculateNodeHeight(node->left);
	int rh = calculateNodeHeight(node->right);
	return 1 + std::max(lh, rh);
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
		actualHeight[idx] = calculateNodeHeight(root);
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
			onMistake(1, 25);
			res.penaltyPoints = 25;
			res.alarmDelta = 1;
			res.correct = false;
			res.message = "Room not found!";
			return res;
		}

		if (playerHeight[currIndex] != -1)
		{
			onMistake(1, 25);
			res.penaltyPoints = 25;
			res.alarmDelta = 1;
			res.correct = false;
			res.message = "Already solved!";
			return res;
		}

		TreeNode* currNode = solTree.findNode(action.nodeA);
		if (currNode == NULL)
		{
			onMistake(1, 25);
			res.penaltyPoints = 25;
			res.alarmDelta = 1;
			res.correct = false;
			res.message = "Room node not located in blueprint!";
			return res;
		}

		// Only check direct children of this specific parent node!
		bool leftReady = (currNode->left == NULL);
		if (currNode->left != NULL)
		{
			int leftIdx = findIndex(currNode->left->val);
			leftReady = (leftIdx != -1 && playerHeight[leftIdx] != -1);
		}

		bool rightReady = (currNode->right == NULL);
		if (currNode->right != NULL)
		{
			int rightIdx = findIndex(currNode->right->val);
			rightReady = (rightIdx != -1 && playerHeight[rightIdx] != -1);
		}

		bool ready = (leftReady && rightReady);

		if (!ready) {
			onMistake(1, 25);
			res.penaltyPoints = 25;
			res.alarmDelta = 1;
			res.correct = false;
			res.message = "Wrong Order! Solve children of this room first!";
		}
		else if (action.nodeB != actualHeight[currIndex])
		{
			onMistake(1, 25);
			res.penaltyPoints = 25;
			res.alarmDelta = 1;
			res.correct = false;
			res.message = "Incorrect Height! Alarm increased!";
		}
		else
		{
			// Correct height entered! Save in player array
			playerHeight[currIndex] = action.nodeB;
			solvedCount++;
			onValidStep();
			res.correct = true;

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
			onMistake(2, 50);
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