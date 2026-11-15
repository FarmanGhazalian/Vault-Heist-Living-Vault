#include "Act2Level4.h"

Level2_4Data::Level2_4Data()
{
    levelId = 24;
    levelName = "Range Raid";
    lo = 0;
    hi = 0;
    totalKeys = 0;
    keys = nullptr;
}

Level2_4Data::~Level2_4Data()
{
    if (keys != nullptr)
    {
        delete[] keys;
        keys = nullptr;
    }
}

bool Level2_4Data::loadJSON(const string& filePath)
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
    lo = j["lo"].get<int>();
    hi = j["hi"].get<int>();

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

Act2Level4::Act2Level4() : ILevelLogic(0)
{
    rangeValues = nullptr;
    collected = nullptr;
    rangeCount = 0;
    collectedCount = 0;
}

Act2Level4::~Act2Level4()
{
    if (rangeValues != nullptr)
    {
        delete[] rangeValues;
        rangeValues = nullptr;
    }
    if (collected != nullptr)
    {
        delete[] collected;
        collected = nullptr;
    }
}

int Act2Level4::findValueIndex(int val) const
{
    for (int i = 0; i < rangeCount; i++)
    {
        if (rangeValues[i] == val)
        {
            return i;
        }
    }
    return -1;
}

bool Act2Level4::initLevel(const string& jsonPath)
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

    if (rangeValues != nullptr)
    {
        delete[] rangeValues;
        rangeValues = nullptr;
    }
    if (collected != nullptr)
    {
        delete[] collected;
        collected = nullptr;
    }

    rangeValues = bst.rangeToArray(data.lo, data.hi, rangeCount);
    if (rangeCount > 0)
    {
        collected = new bool[rangeCount];
        for (int i = 0; i < rangeCount; i++)
        {
            collected[i] = false;
        }
    }

    // Optimal steps = count of values in range
    state = LevelState(rangeCount);
    collectedCount = 0;
    return true;
}

void Act2Level4::onValidStep()
{
    state.addStep();
}

void Act2Level4::onMistake(int extraAlarm, int penaltyPoints)
{
    state.addpenalty(extraAlarm, penaltyPoints);
}

void Act2Level4::onHintRequested()
{
    state.useHint();
}

ActionResult Act2Level4::processAction(const PlayerAction& action)
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
        int remainingToFind = rangeCount - collectedCount;
        for (int i = 0; i < rangeCount; i++)
        {
            if (!collected[i])
            {
                result.message = "Hint: Node " + to_string(rangeValues[i]) + " is within [" + to_string(data.lo) + ", " + to_string(data.hi) + "]. (" + to_string(remainingToFind) + " remaining)";
                return result;
            }
        }
        result.message = "Hint: All range nodes already collected!";
        return result;
    }

    if (action.type == CLICK_NODE)
    {
        result.accepted = true;
        int idx = findValueIndex(action.nodeA);

        if (idx != -1)
        {
            // Value is within range array
            if (!collected[idx])
            {
                // First time collecting this value -> valid step
                collected[idx] = true;
                collectedCount++;

                result.correct = true;
                result.alarmDelta = 1;
                result.penaltyPoints = 0;
                onValidStep();

                if (collectedCount >= rangeCount)
                {
                    state.finished = true;
                    state.won = true;
                    result.message = "Range Raid Complete! All loot in range secured.";
                }
                else
                {
                    result.message = "Secured " + to_string(action.nodeA) + "! (" + to_string(collectedCount) + "/" + to_string(rangeCount) + " collected)";
                }
            }
            else
            {
                // Already collected -> reject with message, NO penalty
                result.correct = false;
                result.alarmDelta = 0;
                result.penaltyPoints = 0;
                result.message = "Node " + to_string(action.nodeA) + " already collected! Click another uncollected node.";
            }
        }
        else
        {
            // Not in range array -> mistake (alarm + penalty)
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
                result.message = "Node " + to_string(action.nodeA) + " is outside range [" + to_string(data.lo) + ", " + to_string(data.hi) + "]! Alarm Increased (+2).";
            }
        }
        return result;
    }

    result.accepted = false;
    result.message = "Unsupported action type! Expected CLICK_NODE (0).";
    return result;
}

int Act2Level4::getLo() const
{
    return data.lo;
}

int Act2Level4::getHi() const
{
    return data.hi;
}

int Act2Level4::getCollectedCount() const
{
    return collectedCount;
}

int Act2Level4::getTotalToCollect() const
{
    return rangeCount;
}

TreeNode* Act2Level4::getTree() const
{
    return bst.getRoot();
}
