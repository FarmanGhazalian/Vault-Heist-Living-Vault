#include<iostream>
#include"Act1Level2.h"
using namespace std;

void L2_data::generate()
{
    preArr = new int[totalNodes];
    inArr = new int[totalNodes];
    return;
}
bool loadData::loadJSON(const string& filePath)
{
    ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    json j;
    file >> j;

    // Metadata Parse
    data.id = j["levelId"];
    data.levelName = j["levelName"];
    data.optimalSteps = j["optimalSteps"];
    data.traversalType = j["traversalType"];
    data.totalNodes = j["totalNodes"];
    data.generate();
    for (size_t i = 0; i < data.totalNodes; i++)
    {
        data.inArr[i] = j["inorderSequence"][i].get<int>();
    }

    for (size_t i = 0; i < data.totalNodes; i++)
    {
        data.inArr[i] = j["preorderSequence"][i].get<int>();
    }

    file.close();
    return true;
}


bool Act1Level2::verifyPlacementNode(int parentVal, int direction, int placedVal)
{
    if (parentVal == -1)
    {
        return solTree.getRoot() != NULL && solTree.getRoot()->val == placedVal;
    }
    TreeNode* parentNode = solTree.findNode(parentVal);
    if (direction == 0 && parentNode->left != NULL)
    {
        return (parentNode->left->val == placedVal);
    }
    if (direction == 1 && parentNode->right != NULL)
    {
        return (parentNode->right->val == placedVal);
    }
    return false;
}



bool Act1Level2::initLevel(const string& jsonPath) 
{
    if (!loader.loadJSON(jsonPath))
    {
        return false;
    }
    totalNodes = loader.data.totalNodes;
    correctlyPlacementCount = 0;

    solTree.buildFromPreIn(loader.data.preArr, loader.data.inArr, totalNodes);
    return true;
}
void Act1Level2::onValidStep()
{
    state.addStep();
}

void Act1Level2::onMistake(int extraAlarm = 1, int penaltyPoints = 0)
{
    state.addpenalty(extraAlarm, penaltyPoints);
}

void Act1Level2::onHintRequested()
{
    state.useHint();
}

string getHint()
{
    return "The first node in Preorder is always the Root. Use Inorder to see which nodes go to its Left and Right!";
}

ActionResult Act1Level2::processAction(const PlayerAction& action)
{
    ActionResult res;
    if (state.finished) return res;

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
    if(action.type == PLACE_TOKEN)
    {

        bool isCorrect = verifyPlacementNode(action.nodeA, action.direction, action.nodeB);

        if (isCorrect) {
            onValidStep();
            res.correct = true;
            res.message = "Blueprint Node Verified!";
            if (correctlyPlacementCount == totalNodes) {
                state.finished = true;
                state.won = true;
                res.message = "Blueprint Decoded Successfully!";
            }
        }
        else {
            onMistake(2, 50);
            res.penaltyPoints = 50;
            res.alarmDelta = 2;
            res.correct = false;
            res.message = "Wrong Slot! Security Alert!";
        }

        if (!state.won && state.alarm >= state.alarmCap) {
            state.finished = true;
            state.won = false;
            res.message = "Alarm Triggered! Vault Lockdown!";
        }
    }

    return res;
}