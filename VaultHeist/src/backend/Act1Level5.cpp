#include<iostream>
#include"backend/Act1Level5.h"
#include"backend/LevelGenerator.h"
using namespace std;


void L5_data::generate()
{
    preArr = new int[totalNodes];
    inArr = new int[totalNodes];
}


bool Level5DataLoader::loadJSON(const string& filePath)
{
	ifstream file(filePath);
	if (!file.is_open())
	{
		return false;
	}

	json j;
	file >> j;
	LevelGenerator::processJson(j);

    if (j.contains("levelId"))
    {
        data.levelId = j["levelId"].is_string() ? j["levelId"].get<std::string>() : j["levelId"].dump();
    }
	data.levelName = j["levelName"];
	data.totalNodes = j["totalNodes"];
	data.optimalSteps = j.contains("optimalSteps") ? j["optimalSteps"].get<int>() : 0;
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


bool Act1Level5::isLeaf(TreeNode* node) const 
{
	return (node != nullptr && node->left == nullptr && node->right == nullptr);
}

bool Act1Level5::isNodeSwapped(TreeNode* node) const 
{
	for (int i = 0; i < swappedCount; i++) {
		if (swappedNodes[i] == node) return true;
	}
	return false;
}

bool Act1Level5::areChildrenReady(TreeNode* node) const
{
	if (node == nullptr)
	{
		return false;
	}

	if (node->left != nullptr && !isLeaf(node->left))
	{
		if (!isNodeSwapped(node->left))
		{
			return false;
		}
	}

	if (node->right != nullptr && !isLeaf(node->right))
	{
		if (!isNodeSwapped(node->right))
		{
			return false;
		}
	}

	return true;
}

static int countNonLeafNodes(TreeNode* node)
{
	if (node == nullptr || (node->left == nullptr && node->right == nullptr))
		return 0;
	return 1 + countNonLeafNodes(node->left) + countNonLeafNodes(node->right);
}

bool Act1Level5::initLevel(const std::string& jsonPath)
{
	if (!loader.loadJSON(jsonPath)) return false;

	swappedCount = 0;
	if (swappedNodes != nullptr)
	{
		delete[] swappedNodes;
	}
	swappedNodes = new TreeNode*[loader.data.totalNodes];
	for (int i = 0; i < loader.data.totalNodes; i++) swappedNodes[i] = nullptr;

	currentTree.buildFromPreIn(loader.data.preArr, loader.data.inArr, loader.data.totalNodes);
	int dynSteps = countNonLeafNodes(currentTree.getRoot());
	if (dynSteps <= 0) dynSteps = 1;
	state = LevelState(dynSteps);

	mirrorTree.buildFromPreIn(loader.data.preArr, loader.data.inArr, loader.data.totalNodes);
	mirrorTree.Mirror();

	return true;
}

ActionResult Act1Level5::processAction(const PlayerAction& action) 
{
    ActionResult res;
    if (isFinished()) return res;

    // 1. HINT REQUEST
    if (action.type == CLICK_HINT)
    {
        if (!state.hintUsed) {
            onHintRequested();
            res.correct = true;
            res.message = getHint();
        }
        else
        {
            // Second hint incurs alarm penalty only (handled in LevelState::useHint)
            res.alarmDelta = 2;
            res.penaltyPoints = 0;
            res.correct = true;
            res.message = "[Hint Already Used] " + getHint();
        }

        if (!isWon() && state.alarm >= state.alarmCap) {
            state.finished = true;
            state.won = false;
            res.message = "Alarm Triggered! Vault Lockdown!";
        }
        return res;
    }

    // 2. AUTO SOLVE ("or press Auto")
    if (action.type == AUTO_MIRROR) {
        currentTree.MirrorHelper(currentTree.getRoot());
        onValidStep();

        if (state.alarm >= state.alarmCap)
        {
            state.finished = true;
            state.won = false;
            res.correct = false;
            res.message = "Auto-Mirror triggered the Alarm! Vault Lockdown!";
        }
        else {
            state.finished = true;
            state.won = true;
            res.correct = true;
            res.message = "Vault Auto-Mirrored recursively! Sector Cleared!";
        }
        return res;
    }

    if (action.type == SWAP_CHILDREN)
    {
        TreeNode* target = currentTree.findNode(action.nodeA);
        if (target == nullptr) {
            onMistake(2, 50);
            res.penaltyPoints = 50;
            res.alarmDelta = 2;
            res.correct = false;
            res.message = "Invalid Room selected!";
        }
        else if (isLeaf(target)) {
            onMistake(2, 50);
            res.penaltyPoints = 50;
            res.alarmDelta = 2;
            res.correct = false;
            res.message = "Room #" + std::to_string(action.nodeA) + " is a leaf node - no children to swap!";
        }
        else if (isNodeSwapped(target)) {
            onMistake(2, 50);
            res.penaltyPoints = 50;
            res.alarmDelta = 2;
            res.correct = false;
            res.message = "Room #" + to_string(action.nodeA) + " is already mirrored!";
        }
        else if (!areChildrenReady(target)) {
            onMistake(2, 50);
            res.penaltyPoints = 50;
            res.alarmDelta = 2;
            res.correct = false;
            res.message = "Bottom-Up Rule Violated! Mirror the child subtrees below Room #" + to_string(action.nodeA) + " first!";
        }
        else
        {
            TreeNode* temp = target->left;
            target->left = target->right;
            target->right = temp;
            swappedNodes[swappedCount++] = target;
            onValidStep();
            res.correct = true;
            res.accepted = true;

            if (currentTree.isSame(mirrorTree)) {
                res.message = "Vault Successfully Mirrored! All chambers inverted. Sector Cleared!";
                state.finished = true;
                state.won = true;
            }
            else {
                res.message = "Room #" + std::to_string(action.nodeA) + " children swapped!";
            }
        }
    }

    if (!isWon() && state.alarm >= state.alarmCap) {
        state.finished = true;
        state.won = false;
        res.message = "Alarm Reached Maximum Capacity! Vault Lockdown!";
    }
    return res;
}