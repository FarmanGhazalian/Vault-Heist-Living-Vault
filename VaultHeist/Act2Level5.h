#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include "nlohmann/json.hpp"
#include "ILevelLogic.h"
#include "PlayerAction.h"
#include "BST.h"
#include "ScoreSystem.h"

using json = nlohmann::json;
using namespace std;

struct Level2_5Data
{
    int levelId = 25;
    string levelName = "The Lazy Architect";
    int totalKeys = 0;
    int* keys = nullptr; // Sorted keys (deliveries)
    int totalTargets = 0;
    int* targets = nullptr;

    Level2_5Data();
    ~Level2_5Data();
    bool loadJSON(const string& filePath);
};

class Act2Level5 : public ILevelLogic
{
private:
    BST bst;
    Level2_5Data data;

    int currentTargetIdx = 0;
    int* currentPath = nullptr;
    int currentPathLength = 0;
    int currentPathIdx = 0;

    int validStepsSinceLastShrink = 0;

    void computeCurrentPath();

public:
    Act2Level5();
    ~Act2Level5();

    bool initLevel(const string& jsonPath);

    void onValidStep() override;
    void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
    void onHintRequested() override;

    ActionResult processAction(const PlayerAction& action) override;

    // Getters for GUI
    int getCurrentTarget() const;
    int getCurrentPathIndex() const;
    int getCurrentPathLength() const;
    int getHeight();
    int getCapacityLeft() const;
    int getCompletedTargets() const;
    int getTotalTargets() const;
    TreeNode* getTreeRoot() const;
};
