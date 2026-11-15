#include "Act2Level1.h"

Level2_1Data::Level2_1Data()
{
    levelId = 21;
    levelName = "Search Heist";
    target = 0;
    totalKeys = 0;
    keys = nullptr;
}

Level2_1Data::~Level2_1Data()
{
    if (keys != nullptr)
    {
        delete[] keys;
        keys = nullptr;
    }
}

bool Level2_1Data::loadJSON(const string& filePath)
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
    target = j["target"].get<int>();

    auto keysJson = j["keys"];
    totalKeys = (int)keysJson.size();
    if (keys != nullptr)
    {
        delete[] keys;
    }
    keys = new int[totalKeys];
    for (int i = 0; i < totalKeys; i++)
    {
        keys[i] = keysJson[i].get<int>();
    }
    return true;
}

Act2Level1::Act2Level1() : ILevelLogic(0)
{
    expectedPath = nullptr;
    pathLength = 0;
    currentIndex = 0;
}

Act2Level1::~Act2Level1()
{
    if (expectedPath != nullptr)
    {
        delete[] expectedPath;
        expectedPath = nullptr;
    }
}

bool Act2Level1::initLevel(const string& jsonPath)
{
    if (!data.loadJSON(jsonPath))
    {
        return false;
    }

    bst.clear();
    for (int i = 0; i < data.totalKeys; i++)
    {
        bst.insert(data.keys[i]);
    }

    if (expectedPath != nullptr)
    {
        delete[] expectedPath;
        expectedPath = nullptr;
    }

    expectedPath = bst.getPath(data.target, pathLength);
    if (expectedPath == nullptr || pathLength == 0)
    {
        return false;
    }

    // Optimal steps = path length (model computes this)
    state = LevelState(pathLength);
    currentIndex = 0;
    return true;
}

void Act2Level1::onValidStep()
{
    state.addStep();
}

void Act2Level1::onMistake(int extraAlarm, int penaltyPoints)
{
    state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level1::onHintRequested()
{
    state.useHint();
}

ActionResult Act2Level1::processAction(const PlayerAction& action)
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
        result.correct = true;
        onHintRequested();
        if (currentIndex < pathLength)
        {
            result.message = "Hint: Next patrol node in path has value " + to_string(expectedPath[currentIndex]);
        }
        else
        {
            result.message = "Hint: You have reached the target room!";
        }
        return result;
    }

    if (action.type == CLICK_NODE)
    {
        result.accepted = true;

        if (currentIndex < pathLength && action.nodeA == expectedPath[currentIndex])
        {
            result.correct = true;
            result.alarmDelta = 1;
            result.penaltyPoints = 0;

            onValidStep();
            currentIndex++;

            if (currentIndex >= pathLength)
            {
                state.finished = true;
                state.won = true;
                result.message = "Target Key Found! Search Heist Cleared.";
            }
            else
            {
                result.message = "Correct patrol path step! Move to next node.";
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
                result.message = "Wrong patrol path node! Alarm Increased (+2).";
            }
        }
    }
    else
    {
        result.accepted = false;
        result.message = "Unsupported action type!";
    }

    return result;
}

int Act2Level1::getTarget() const
{
    return data.target;
}

int Act2Level1::getCurrentIndex() const
{
    return currentIndex;
}

int Act2Level1::getPathLength() const
{
    return pathLength;
}

TreeNode* Act2Level1::getTreeRoot() const
{
    return bst.getRoot();
}
