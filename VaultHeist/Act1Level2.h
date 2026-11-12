#pragma once
#include<iostream>
#include <fstream>
#include<string>
#include"nlohmann/json.hpp"
#include"ILevelLogic.h"
#include "PlayerAction.h"
#include"BinaryTree.h"

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
        totalNodes =0;
        preArr = new int[totalNodes];
        inArr = new int[totalNodes];
    }

};




class loadData
{
	L2_data data;
public:

    bool loadJSON(const string& filePath) 
    {
        ifstream file(filePath);
        if (!file.is_open()) 
        {
            return false;
        }

        json j;
        file >> j;

        // Metadata Parse
        data.id = j["levelId"];
        data.levelName = j["levelName"];
        data.optimalSteps = j["optimalSteps"];
        data.traversalType = j["traversalType"];
        data.totalNodes = j["totalNodes"];

        for (size_t i = 0; i < data.totalNodes ; i++)
        {
            data.preArr[i] = j;
        }

        for (size_t i = 0; i < data.totalNodes ; i++)
        {
            data.preArr[i] = j["preorderSequence"][i].get<int>();
        }

        file.close();
        return true;
    }
};

class Act1Level2
{
public:

};