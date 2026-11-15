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

struct Level2_1Data
{
    int levelId = 21;
    string levelName = "Search Heist";
    int target = 0;
    int totalKeys = 0;
    int* keys = nullptr;

    Level2_1Data();
    ~Level2_1Data();
    bool loadJSON(const string& filePath);
};

class Act2Level1 : public ILevelLogic
{
private:
    BST bst;
    Level2_1Data data;

    int* expectedPath = nullptr;
    int pathLength = 0;
    int currentIndex = 0;

public:
    Act2Level1();
    ~Act2Level1();

    bool initLevel(const string& jsonPath);

    void onValidStep() override;
    void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
    void onHintRequested() override;

    ActionResult processAction(const PlayerAction& action) override;

    // Getters for GUI
    int getTarget() const;
    int getCurrentIndex() const;
    int getPathLength() const;
    TreeNode* getTreeRoot() const;
};
