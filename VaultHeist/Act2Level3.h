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

struct Level2_3Data
{
    int levelId = 23;
    string levelName = "Steal and Collapse";
    int totalInitialKeys = 0;
    int* initialKeys = nullptr;
    int totalDeleteKeys = 0;
    int* deleteKeys = nullptr;

    Level2_3Data();
    ~Level2_3Data();
    bool loadJSON(const string& filePath);
};

class Act2Level3 : public ILevelLogic
{
private:
    BST bst;
    Level2_3Data data;

    int currentTargetIdx = 0;

    // Last deletion info for GUI animation
    int lastDeletedKey = -1;
    int lastDeleteCase = -1;
    int lastSuccessorKey = -1;

public:
    Act2Level3();
    ~Act2Level3();

    bool initLevel(const string& jsonPath);

    void onValidStep() override;
    void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
    void onHintRequested() override;

    ActionResult processAction(const PlayerAction& action) override;

    // Getters for GUI
    int getCurrentTarget() const;
    int getCompletedDeletions() const;
    int getTotalDeletions() const;
    int getLastDeletedKey() const;
    int getLastDeleteCase() const;
    int getLastSuccessorKey() const;
    TreeNode* getTreeRoot() const;
};
