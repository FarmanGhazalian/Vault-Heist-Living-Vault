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

struct Level2_2Data
{
    int levelId = 22;
    string levelName = "Stash Loot";
    int totalInitialKeys = 0;
    int* initialKeys = nullptr;
    int totalInsertKeys = 0;
    int* insertKeys = nullptr;

    Level2_2Data();
    ~Level2_2Data();
    bool loadJSON(const string& filePath);
};

class Act2Level2 : public ILevelLogic
{
private:
    BST bst;
    Level2_2Data data;

    int currentKeyIdx = 0;
    int* currentPath = nullptr;
    int currentPathLength = 0;
    int currentPathIdx = 0;

    void computeCurrentPath();

public:
    Act2Level2();
    ~Act2Level2();

    bool initLevel(const string& jsonPath);

    void onValidStep() override;
    void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
    void onHintRequested() override;

    ActionResult processAction(const PlayerAction& action) override;

    // Getters for GUI
    int getCurrentKey() const;
    int getCurrentPathIndex() const;
    int getCurrentPathLength() const;
    int getCompletedInsertions() const;
    int getTotalInsertions() const;
    TreeNode* getTreeRoot() const;
};
