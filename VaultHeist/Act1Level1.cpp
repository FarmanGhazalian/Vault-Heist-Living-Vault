#include<iostream>
#include"Act1Level1.h"
#include"nlohmann/json.hpp"


bool Level1DataLoader::getTotalNodes(const string& filePath)
{
    ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }
    json data;
    file >> data;
    file.close();
    totalNodes = data["totalNodes"];
    return true;
}

bool Level1DataLoader::loadJSON(const string& filePath)
{
    // 1. Disk se file open karein
    ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }
    json data;
    file >> data;
    file.close();

    // 3. Metadata Extract karein
    rootId = data["rootId"];
    optimalSteps = data["optimalSteps"];
    traversalType = data["traversalType"];

    int i = 0;
    for (const auto& item : data["nodes"]) {
        if (i < totalNodes)
        {
            rawNodes[i].id = item["id"];
            rawNodes[i].value = item["value"];
            rawNodes[i].leftId = item["left"];
            rawNodes[i].rightId = item["right"];
            i++;
        }
    }
    return true;
}


int Act1Level1::FindRawIndex(int currentID)
{
    for (int i = 0;i < data.totalNodes;i++)
    {
        if (data.rawNodes[i].id == currentID)
        {
            return i;
        }
    }
    return -1;
}

TreeNode* Act1Level1::BuildTree(int currentID)
{
    if (currentID == -1) return NULL;
    int idx = FindRawIndex(currentID);
    if (idx == -1)return NULL;

    TreeNode* root = new TreeNode(data.rawNodes[idx].value, data.rawNodes[idx].id);

    root->left = BuildTree(data.rawNodes[idx].leftId);
    root->right = BuildTree(data.rawNodes[idx].rightId);

    return root;
}

Act1Level1::~Act1Level1()
{
    if (expectedSequence != NULL)
    {
        delete[]expectedSequence;
        expectedSequence = NULL;
    }
}

bool Act1Level1::initLevel(const std::string& jsonPath)
{
    // Step 1: Parse disk file into raw flat array
    if (!data.loadJSON(jsonPath)) {
        return false;
    }

    TreeNode* root = BuildTree(data.rootId);
    gameTree.setRoot(root);
    if (expectedSequence != NULL)
    {
        delete[]expectedSequence;
        expectedSequence = NULL;
    }

    if (data.traversalType == "PREORDER")
    {
        expectedSequence = gameTree.preorderRecursive();
    }
    else if (data.traversalType == "INORDER")
    {
        expectedSequence = gameTree.inorderRecursive();
    }
    else if (data.traversalType == "POSTORDER")
    {
        expectedSequence = gameTree.postorderRecursive();
    }
    else if (data.traversalType == "LEVELORDER")
    {
        expectedSequence = gameTree.levelOrder();
    }
    return true;
}

void Act1Level1::onValidStep()
{
    state.addStep();
}

void Act1Level1::onMistake(int extraAlarm = 1, int penaltyPoints = 0)
{
    state.addpenalty(extraAlarm, penaltyPoints);
}

void Act1Level1::onHintRequested()
{
    state.useHint();
}

ActionResult Act1Level1::processAction(const PlayerAction& action)
{
    ActionResult result;

    if (isFinished() || state.isFull()) {
        result.accepted = false;
        result.correct = false;
        result.message = "Level locked or already finished!";
        return result;
    }

    if (action.type == CLICK_NODE) {
        result.accepted = true;

        if (action.nodeA == expectedSequence[currentIndex]) {
            result.correct = true;
            result.alarmDelta = 1;
            result.penaltyPoints = 0;

            onValidStep();
            currentIndex++;

            if (currentIndex >= sequenceSize) {
                state.finished = true;
                state.won = true;
                result.message = "Traversal Complete! Level Cleared.";
            }
            else {
                result.message = "Correct patrol step! Move to next node.";
            }
        }
        else
        {
            result.correct = false;
            result.alarmDelta = 2;
            result.penaltyPoints = 50;

            onMistake(2, 50);

            if (state.isFull()) {
                state.finished = true;
                state.won = false;
                result.message = "ALARM CAP REACHED! Vault Lockdown Initiated.";
            }
            else {
                result.message = "Wrong sequence step! Alarm Increased (+2).";
            }
        }
    }
    else {
        result.accepted = false;
        result.message = "Unsupported action type!";
    }

    return result;
}