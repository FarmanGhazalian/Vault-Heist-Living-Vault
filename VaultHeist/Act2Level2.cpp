#include "Act2Level2.h"

Level2_2Data::Level2_2Data()
{
    levelId = 22;
    levelName = "Stash Loot";
    totalInitialKeys = 0;
    initialKeys = nullptr;
    totalInsertKeys = 0;
    insertKeys = nullptr;
}

Level2_2Data::~Level2_2Data()
{
    if (initialKeys != nullptr)
    {
        delete[] initialKeys;
        initialKeys = nullptr;
    }
    if (insertKeys != nullptr)
    {
        delete[] insertKeys;
        insertKeys = nullptr;
    }
}

bool Level2_2Data::loadJSON(const string& filePath)
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

    auto insJson = j["insertKeys"];
    totalInsertKeys = (int)insJson.size();
    if (insertKeys != nullptr)
    {
        delete[] insertKeys;
    }
    insertKeys = new int[totalInsertKeys];
    for (int i = 0; i < totalInsertKeys; i++)
    {
        insertKeys[i] = insJson[i].get<int>();
    }
    return true;
}

Act2Level2::Act2Level2() : ILevelLogic(0)
{
    currentKeyIdx = 0;
    currentPath = nullptr;
    currentPathLength = 0;
    currentPathIdx = 0;
}

Act2Level2::~Act2Level2()
{
    if (currentPath != nullptr)
    {
        delete[] currentPath;
        currentPath = nullptr;
    }
}

void Act2Level2::computeCurrentPath()
{
    if (currentPath != nullptr)
    {
        delete[] currentPath;
        currentPath = nullptr;
    }
    currentPathLength = 0;
    currentPathIdx = 0;

    if (currentKeyIdx < data.totalInsertKeys)
    {
        int key = data.insertKeys[currentKeyIdx];
        currentPath = bst.getInsertPath(key, currentPathLength);
    }
}

bool Act2Level2::initLevel(const string& jsonPath)
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

    // Optimal steps compute karein: sum over keys of (pathLength + 1)
    BST simBst;
    for (int i = 0; i < data.totalInitialKeys; i++)
    {
        simBst.insert(data.initialKeys[i]);
    }
    int totalOptimal = 0;
    for (int i = 0; i < data.totalInsertKeys; i++)
    {
        int len = 0;
        int* p = simBst.getInsertPath(data.insertKeys[i], len);
        if (p != nullptr)
        {
            delete[] p;
        }
        totalOptimal += (len + 1);
        simBst.insert(data.insertKeys[i]);
    }

    state = LevelState(totalOptimal);
    currentKeyIdx = 0;
    computeCurrentPath();
    return true;
}

void Act2Level2::onValidStep()
{
    state.addStep();
}

void Act2Level2::onMistake(int extraAlarm, int penaltyPoints)
{
    state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level2::onHintRequested()
{
    state.useHint();
}

ActionResult Act2Level2::processAction(const PlayerAction& action)
{
    ActionResult result;

    if (isFinished() || state.isFull())
    {
        result.accepted = false;
        result.correct = false;
        result.message = "Level locked or already finished!";
        return result;
    }

    if (currentKeyIdx >= data.totalInsertKeys)
    {
        result.accepted = false;
        result.message = "All keys already inserted!";
        return result;
    }

    int currentKey = data.insertKeys[currentKeyIdx];

    // Hint handling
    if (action.type == CLICK_HINT)
    {
        result.accepted = true;
        result.correct = true;
        onHintRequested();
        if (currentPathIdx < currentPathLength)
        {
            result.message = "Hint: Click node " + to_string(currentPath[currentPathIdx]) + " on the insertion path.";
        }
        else
        {
            int parentVal = currentPath[currentPathLength - 1];
            string dirStr = (currentKey < parentVal) ? "LEFT" : "RIGHT";
            result.message = "Hint: Place key " + to_string(currentKey) + " as " + dirStr + " child of " + to_string(parentVal);
        }
        return result;
    }

    // Step 1: Patrol down insertion path
    if (action.type == CLICK_NODE)
    {
        result.accepted = true;

        if (currentPathIdx < currentPathLength)
        {
            if (action.nodeA == currentPath[currentPathIdx])
            {
                result.correct = true;
                result.alarmDelta = 1;
                result.penaltyPoints = 0;
                onValidStep();
                currentPathIdx++;

                if (currentPathIdx == currentPathLength)
                {
                    result.message = "Reached insertion point! Choose LEFT or RIGHT to attach key.";
                }
                else
                {
                    result.message = "Correct path step! Move to next node.";
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
                    result.message = "Wrong node on insertion path! Alarm Increased (+2).";
                }
            }
        }
        else
        {
            // Already at parent, player should use PLACE_TOKEN
            result.correct = false;
            result.message = "Already at insertion leaf! Place key using Left/Right.";
        }
        return result;
    }

    // Step 2: Place token at parent leaf
    if (action.type == PLACE_TOKEN)
    {
        result.accepted = true;

        if (currentPathIdx < currentPathLength)
        {
            result.correct = false;
            result.message = "Patrol path not completed yet! Click the path nodes first.";
            return result;
        }

        int parentVal = currentPath[currentPathLength - 1];
        int expectedDir = (currentKey < parentVal) ? 0 : 1; // 0 = left, 1 = right

        // Check direction
        if (action.direction == expectedDir)
        {
            // Valid direction
            result.correct = true;
            result.alarmDelta = 1;
            result.penaltyPoints = 0;
            onValidStep();

            // Actually insert into model BST
            bool inserted = bst.insert(currentKey);
            if (!inserted)
            {
                // Duplicate key: rejected with visible message, skip key without penalty
                result.message = "Duplicate key " + to_string(currentKey) + " rejected! Key skipped.";
            }
            else
            {
                result.message = "Key " + to_string(currentKey) + " stashed successfully!";
            }

            currentKeyIdx++;
            if (currentKeyIdx >= data.totalInsertKeys)
            {
                state.finished = true;
                state.won = true;
                result.message = "All Loot Stashed! Level Cleared.";
            }
            else
            {
                computeCurrentPath();
            }
        }
        else
        {
            // Wrong direction: mistake and do NOT insert
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
                result.message = "Wrong placement direction! BST invariant violated (+2 Alarm).";
            }
        }
        return result;
    }

    result.accepted = false;
    result.message = "Unsupported action type!";
    return result;
}

int Act2Level2::getCurrentKey() const
{
    if (currentKeyIdx < data.totalInsertKeys)
    {
        return data.insertKeys[currentKeyIdx];
    }
    return -1;
}

int Act2Level2::getCurrentPathIndex() const
{
    return currentPathIdx;
}

int Act2Level2::getCurrentPathLength() const
{
    return currentPathLength;
}

int Act2Level2::getCompletedInsertions() const
{
    return currentKeyIdx;
}

int Act2Level2::getTotalInsertions() const
{
    return data.totalInsertKeys;
}

TreeNode* Act2Level2::getTreeRoot() const
{
    return bst.getRoot();
}
