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

struct Level2_4Data
{
    int levelId = 24;
    string levelName = "Range Raid";
    int lo = 0;
    int hi = 0;
    int totalKeys = 0;
    int* keys = nullptr;

    Level2_4Data();
    ~Level2_4Data();
    bool loadJSON(const string& filePath);
};

class Act2Level4 : public ILevelLogic
{
private:
    BST bst;
    Level2_4Data data;

    int* rangeValues = nullptr;
    bool* collected = nullptr;
    int rangeCount = 0;
    int collectedCount = 0;

    int findValueIndex(int val) const;

public:
    Act2Level4();
    ~Act2Level4();

    bool initLevel(const string& jsonPath);

    void onValidStep() override;
    void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
    void onHintRequested() override;

    ActionResult processAction(const PlayerAction& action) override;

    // Getters for GUI
    int getLo() const;
    int getHi() const;
    int getCollectedCount() const;
    int getTotalToCollect() const;
    TreeNode* getTree() const;
};
