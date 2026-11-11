#pragma once
#include <fstream>
#include <iostream>
#include <string>
#include"nlohmann/json.hpp"
#include"ILevelLogic.h"
#include "PlayerAction.h"
#include"BinaryTree.h"


using namespace std;
using json = nlohmann::json;


struct TempNodeData {
    int id = -1;
    int value = 0;
    int leftId = -1;
    int rightId = -1;
};



class Level1DataLoader 
{
public:

    int rootId = -1;
    int optimalSteps = 0;
    string traversalType = "";
    int totalNodes = 0;
    
    bool getTotalNodes(const string& filePath);
    
    TempNodeData* rawNodes = new  TempNodeData[totalNodes];
    
    bool loadJSON(const string& filePath);
};




class Act1Level1 : public ILevelLogic
{
private:
    BinaryTree gameTree;
    Level1DataLoader data;

    int* expectedSequence = nullptr;
    int sequenceSize = data.totalNodes;
    int currentIndex = 0;
    int optimalSteps = data.optimalSteps;

    int FindRawIndex(int currentID);


    TreeNode* BuildTree(int currentID);

public:

    Act1Level1() : ILevelLogic(optimalSteps){}

    ~Act1Level1();

    bool initLevel(const std::string& jsonPath);

    void onValidStep() override;
    void onMistake(int extraAlarm = 1, int penaltyPoints = 0) override;
    void onHintRequested() override;

    ActionResult processAction(const PlayerAction& action) override;
};