#include <iostream>
#include <fstream>
#include <string>
#include"nlohmann/json.hpp"
#include"ILevelLogic.h"
#include "PlayerAction.h"
#include"BinaryTree.h"

using namespace std;
using json = nlohmann::json;



struct L5_data
{
    string levelId = "";
    string levelName = "";
    int optimalSteps = 0;
    int totalNodes;
    int* preArr;
    int* inArr;
    
    void generate();
};

class Level5DataLoader
{
public:
    L5_data data;

    bool loadJSON(const string& filePath);
};

class Act1Level5 : public ILevelLogic {
private:
    Level5DataLoader loader;
    BinaryTree currentTree;
    BinaryTree mirrorTree;

    TreeNode** swappedNodes;
    int swappedCount = 0;

    bool isLeaf(TreeNode* node) const;
    bool isNodeSwapped(TreeNode* node) const;
    bool areChildrenReady(TreeNode* node) const;

public:
    Act1Level5() : ILevelLogic(loader.data.optimalSteps)
    {
        swappedCount = 0;
        swappedNodes = nullptr;
    }

    ~Act1Level5()
    {
        if (swappedNodes != nullptr)
        {
            delete[] swappedNodes;
            swappedNodes = nullptr;
        }
    }

    void onValidStep()
    {
        state.addStep();
    }

    void onMistake(int extraAlarm = 1, int penaltyPoints = 0)
    {
        state.addpenalty(extraAlarm, penaltyPoints);
    }

    void onHintRequested()
    {
        state.useHint();
    }
    
    string getHint() 
    {
        return "Hint: Work Bottom-Up! Swap the lowest parent nodes first (whose children are leaves), and swap the Root node at the very end!";
    }

    bool initLevel(const std::string& jsonPath);

    ActionResult processAction(const PlayerAction& action);
    const BinaryTree& getTree() const { return currentTree; }
};

