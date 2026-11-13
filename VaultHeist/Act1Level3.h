#include <fstream>
#include <iostream>
#include <string>
#include"nlohmann/json.hpp"
#include"ILevelLogic.h"
#include "PlayerAction.h"
#include"BinaryTree.h"

using namespace std;
using json = nlohmann::json;

class L3_data
{
public:
	double levelID;
	string levelName;
	int totalNodes;
	int optimalSteps;
	int* preArr;
	int* inArr;
	L3_data()
	{
		levelID = 0;
		levelName = "";
		totalNodes = 0;
		optimalSteps = 0;
	}
	void generate();
};


class loadData
{
public:
	L3_data data;
	
	bool loadJSON(const string& filePath);
};


class Act1Level3 : ILevelLogic
{
private:
	BinaryTree solTree;
	int solvedCount =0;
	int totalNodes;
	int currIndex;
	int leftIndex;
	int rightIndex;
	bool heightsPhaseDone = false;
	int actualDiameter = 0;
	int* actualHeight;
	int* playerHeight;
	loadData loader;

	int findIndex(int nodeVal);
public:

	Act1Level3() : ILevelLogic(loader.data.optimalSteps) {}

	void generate();

	TreeNode* GenerateH(TreeNode* root, int index);
	
	void genrateHeight();
	bool initLevel(const string& jsonPath);

	
	void onValidStep();
	void onMistake(int extraAlarm = 1, int penaltyPoints = 0);

	void onHintRequested();

	string getHint();

	ActionResult processAction(const PlayerAction& action);
};

