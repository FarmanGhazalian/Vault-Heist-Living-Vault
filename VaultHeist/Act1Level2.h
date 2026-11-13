#pragma once
#include<iostream>
#include <fstream>
#include<string>
#include"nlohmann/json.hpp"
#include"ILevelLogic.h"
#include "PlayerAction.h"
#include"BinaryTree.h"
#include"ScoreSystem.h"

using json = nlohmann::json;
using namespace std;




struct L2_data
{
	int id;
	string levelName;
    int optimalSteps;
	string traversalType;
	int totalNodes;
	int* preArr;
	int* inArr;
	L2_data()
    {
        id = 0;
        levelName ="";
        optimalSteps = 0;
        traversalType = "";
        totalNodes = 0;
    }
    void generate();

};





class loadData
{
public:
    L2_data data;

    bool loadJSON(const string& filePath);
};




class Act1Level2 : public ILevelLogic
{
public:

    BinaryTree solTree;
    int totalNodes;
    int correctlyPlacementCount;
    loadData loader;

    bool verifyPlacementNode(int parentVal, int direction, int placedVal);

public:
    Act1Level2() : ILevelLogic(loader.data.optimalSteps){}

    bool initLevel(const string& jsonPath);
    void onValidStep();
    void onMistake(int extraAlarm = 1, int penaltyPoints = 0);

    void onHintRequested();
    string getHint();

    ActionResult processAction(const PlayerAction& action);
};