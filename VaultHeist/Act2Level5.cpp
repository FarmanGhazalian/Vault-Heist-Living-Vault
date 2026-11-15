#include "Act2Level5.h"

Level2_5Data::Level2_5Data()
{
    levelId = 25;
    levelName = "The Lazy Architect";
    totalKeys = 0;
    keys = nullptr;
    totalTargets = 0;
    targets = nullptr;
}

Level2_5Data::~Level2_5Data()
{
    if (keys != nullptr)
    {
        delete[] keys;
        keys = nullptr;
    }
    if (targets != nullptr)
    {
        delete[] targets;
        targets = nullptr;
    }
}

bool Level2_5Data::loadJSON(const string& filePath)
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

    auto targetsJson = j["targets"];
    totalTargets = (int)targetsJson.size();
    if (targets != nullptr)
    {
        delete[] targets;
    }
    targets = new int[totalTargets];
    for (int i = 0; i < totalTargets; i++)
    {
        targets[i] = targetsJson[i].get<int>();
    }
    return true;
}

Act2Level5::Act2Level5() : ILevelLogic(0)
{
    currentTargetIdx = 0;
    currentPath = nullptr;
    currentPathLength = 0;
    currentPathIdx = 0;
    validStepsSinceLastShrink = 0;
}

Act2Level5::~Act2Level5()
{
    if (currentPath != nullptr)
    {
        delete[] currentPath;
        currentPath = nullptr;
    }
}

void Act2Level5::computeCurrentPath()
{
    if (currentPath != nullptr)
    {
        delete[] currentPath;
        currentPath = nullptr;
    }
    currentPathLength = 0;
    currentPathIdx = 0;

    if (currentTargetIdx < data.totalTargets)
    {
        int target = data.targets[currentTargetIdx];
        currentPath = bst.getPath(target, currentPathLength);
    }
}

bool Act2Level5::initLevel(const string& jsonPath)
{
    if (!data.loadJSON(jsonPath))
    {
        return false;
    }

    bst.clear();
    // Insert keys in sorted order -> creates degenerate right chain corridor of height N
    for (int i = 0; i < data.totalKeys; i++)
    {
        bst.insert(data.keys[i]);
    }

    // Compute optimal steps: sum of path lengths across all targets
    int totalOptimal = 0;
    for (int i = 0; i < data.totalTargets; i++)
    {
        int len = 0;
        int* p = bst.getPath(data.targets[i], len);
        if (p != nullptr)
        {
            delete[] p;
        }
        totalOptimal += len;
    }

    state = LevelState(totalOptimal);
    currentTargetIdx = 0;
    validStepsSinceLastShrink = 0;
    computeCurrentPath();
    return true;
}

void Act2Level5::onValidStep()
{
    state.addStep();
    validStepsSinceLastShrink++;
    // After every 2 valid steps, remaining capacity shrinks by 1
    if (validStepsSinceLastShrink >= 2)
    {
        state.reduceCapacity(1);
        validStepsSinceLastShrink = 0;
    }
}

void Act2Level5::onMistake(int extraAlarm, int penaltyPoints)
{
    state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level5::onHintRequested()
{
    state.useHint();
}

ActionResult Act2Level5::processAction(const PlayerAction& action)
{
    ActionResult result;

    if (isFinished() || state.isFull())
    {
        result.accepted = false;
        result.correct = false;
        result.message = "Level locked or already finished!";
        return result;
    }

    if (currentTargetIdx >= data.totalTargets)
    {
        result.accepted = false;
        result.message = "All boss targets already secured!";
        return result;
    }

    if (action.type == CLICK_HINT)
    {
        result.accepted = true;
        result.correct = true;
        onHintRequested();
        if (currentPathIdx < currentPathLength)
        {
            result.message = "Hint: Boss path node is " + to_string(currentPath[currentPathIdx]) + " (Target: " + to_string(data.targets[currentTargetIdx]) + "). Capacity Left: " + to_string(getCapacityLeft());
        }
        else
        {
            result.message = "Hint: Current target already reached!";
        }
        return result;
    }

    if (action.type == CLICK_NODE)
    {
        result.accepted = true;

        if (currentPathIdx < currentPathLength && action.nodeA == currentPath[currentPathIdx])
        {
            result.correct = true;
            result.alarmDelta = 1;
            result.penaltyPoints = 0;

            onValidStep();
            currentPathIdx++;

            if (currentPathIdx >= currentPathLength)
            {
                currentTargetIdx++;
                if (currentTargetIdx >= data.totalTargets)
                {
                    state.finished = true;
                    state.won = true;
                    result.message = "Boss Defeated! The Lazy Architect's corridor breached.";
                }
                else
                {
                    computeCurrentPath();
                    result.message = "Target secured! Next corridor target: " + to_string(data.targets[currentTargetIdx]) + " (Capacity left: " + to_string(getCapacityLeft()) + ")";
                }
            }
            else
            {
                result.message = "Correct corridor step! Capacity left: " + to_string(getCapacityLeft());
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
                result.message = "ALARM CAP REACHED! Architect locked down the corridor.";
            }
            else
            {
                result.message = "Wrong corridor node! Alarm Increased (+2). Capacity left: " + to_string(getCapacityLeft());
            }
        }
        return result;
    }

    result.accepted = false;
    result.message = "Unsupported action type! Expected CLICK_NODE (0).";
    return result;
}

int Act2Level5::getCurrentTarget() const
{
    if (currentTargetIdx < data.totalTargets)
    {
        return data.targets[currentTargetIdx];
    }
    return -1;
}

int Act2Level5::getCurrentPathIndex() const
{
    return currentPathIdx;
}

int Act2Level5::getCurrentPathLength() const
{
    return currentPathLength;
}

int Act2Level5::getHeight()
{
    return bst.height();
}

int Act2Level5::getCapacityLeft() const
{
    return state.remaining();
}

int Act2Level5::getCompletedTargets() const
{
    return currentTargetIdx;
}

int Act2Level5::getTotalTargets() const
{
    return data.totalTargets;
}

TreeNode* Act2Level5::getTreeRoot() const
{
    return bst.getRoot();
}
