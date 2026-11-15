#include "Act2Level3.h"

Level2_3Data::Level2_3Data()
{
    levelId = 23;
    levelName = "Steal and Collapse";
    totalInitialKeys = 0;
    initialKeys = nullptr;
    totalDeleteKeys = 0;
    deleteKeys = nullptr;
}

Level2_3Data::~Level2_3Data()
{
    if (initialKeys != nullptr)
    {
        delete[] initialKeys;
        initialKeys = nullptr;
    }
    if (deleteKeys != nullptr)
    {
        delete[] deleteKeys;
        deleteKeys = nullptr;
    }
}

bool Level2_3Data::loadJSON(const string& filePath)
{
    ifstream file(filePath);
    if (!file.is_open())
    {
        return false;
    }

    json j;
    file >> j;
    file.close();

    if (j.contains("levelName"))
    {
        levelName = j["levelName"].get<string>();
    }

    auto initJson = j["initialKeys"];
    totalInitialKeys = (int)initJson.size();
    if (initialKeys != nullptr)
    {
        delete[] initialKeys;
    }
    initialKeys = new int[totalInitialKeys];
    for (int i = 0; i < totalInitialKeys; i++)
    {
        initialKeys[i] = initJson[i].get<int>();
    }

    auto delJson = j["deleteKeys"];
    totalDeleteKeys = (int)delJson.size();
    if (deleteKeys != nullptr)
    {
        delete[] deleteKeys;
    }
    deleteKeys = new int[totalDeleteKeys];
    for (int i = 0; i < totalDeleteKeys; i++)
    {
        deleteKeys[i] = delJson[i].get<int>();
    }
    return true;
}

Act2Level3::Act2Level3() : ILevelLogic(0)
{
    currentTargetIdx = 0;
    lastDeletedKey = -1;
    lastDeleteCase = -1;
    lastSuccessorKey = -1;
}

Act2Level3::~Act2Level3()
{
}

bool Act2Level3::initLevel(const string& jsonPath)
{
    if (!data.loadJSON(jsonPath))
    {
        return false;
    }

    bst.clear();
    for (int i = 0; i < data.totalInitialKeys; i++)
    {
        bst.insert(data.initialKeys[i]);
    }

    // Optimal steps = number of targets to delete
    state = LevelState(data.totalDeleteKeys);
    currentTargetIdx = 0;
    lastDeletedKey = -1;
    lastDeleteCase = -1;
    lastSuccessorKey = -1;
    return true;
}

void Act2Level3::onValidStep()
{
    state.addStep();
}

void Act2Level3::onMistake(int extraAlarm, int penaltyPoints)
{
    state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level3::onHintRequested()
{
    state.useHint();
}

ActionResult Act2Level3::processAction(const PlayerAction& action)
{
    ActionResult result;

    if (isFinished() || state.isFull())
    {
        result.accepted = false;
        result.correct = false;
        result.message = "Level locked or already finished!";
        return result;
    }

    if (currentTargetIdx >= data.totalDeleteKeys)
    {
        result.accepted = false;
        result.message = "All targets already stolen and collapsed!";
        return result;
    }

    int currentTarget = data.deleteKeys[currentTargetIdx];

    if (action.type == CLICK_HINT)
    {
        result.accepted = true;
        result.correct = true;
        onHintRequested();
        int expectedCase = bst.deleteCase(currentTarget);
        string caseName = "unknown";
        if (expectedCase == 0) caseName = "Leaf (0 children)";
        else if (expectedCase == 1) caseName = "One child (1 child)";
        else if (expectedCase == 2) caseName = "Two children (2 children)";

        result.message = "Hint: Target " + to_string(currentTarget) + " is a " + caseName + " node.";
        return result;
    }

    if (action.type == CHOOSE_CASE)
    {
        result.accepted = true;
        int expectedCase = bst.deleteCase(currentTarget);

        if (action.value == expectedCase)
        {
            result.correct = true;
            result.alarmDelta = 1;
            result.penaltyPoints = 0;
            onValidStep();

            // Store animation details
            lastDeletedKey = currentTarget;
            lastDeleteCase = expectedCase;
            lastSuccessorKey = -1;

            if (expectedCase == 2)
            {
                TreeNode* targetNode = bst.search(currentTarget);
                TreeNode* succ = bst.inorderSuccessor(targetNode);
                if (succ != nullptr)
                {
                    lastSuccessorKey = succ->val;
                }
            }

            // Actually delete node from model BST
            bst.remove(currentTarget);

            currentTargetIdx++;
            if (currentTargetIdx >= data.totalDeleteKeys)
            {
                state.finished = true;
                state.won = true;
                result.message = "All targets stolen! Tree collapsed successfully.";
            }
            else
            {
                result.message = "Correct deletion case! Target collapsed. Next target: " + to_string(data.deleteKeys[currentTargetIdx]);
            }
        }
        else
        {
            result.correct = false;
            result.alarmDelta = 2;
            result.penaltyPoints = 50;
            onMistake(2, 50);

            if (state.isFull())
            {
                state.finished = true;
                state.won = false;
                result.message = "ALARM CAP REACHED! Vault Lockdown Initiated.";
            }
            else
            {
                result.message = "Wrong deletion case chosen! Vault structural stress (+2 Alarm).";
            }
        }
        return result;
    }

    result.accepted = false;
    result.message = "Unsupported action type! Expected CHOOSE_CASE (11).";
    return result;
}

int Act2Level3::getCurrentTarget() const
{
    if (currentTargetIdx < data.totalDeleteKeys)
    {
        return data.deleteKeys[currentTargetIdx];
    }
    return -1;
}

int Act2Level3::getCompletedDeletions() const
{
    return currentTargetIdx;
}

int Act2Level3::getTotalDeletions() const
{
    return data.totalDeleteKeys;
}

int Act2Level3::getLastDeletedKey() const
{
    return lastDeletedKey;
}

int Act2Level3::getLastDeleteCase() const
{
    return lastDeleteCase;
}

int Act2Level3::getLastSuccessorKey() const
{
    return lastSuccessorKey;
}

TreeNode* Act2Level3::getTreeRoot() const
{
    return bst.getRoot();
}
