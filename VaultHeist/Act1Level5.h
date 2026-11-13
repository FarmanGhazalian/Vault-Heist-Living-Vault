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

class loadData
{
public:
    L5_data data;

    bool loadJSON(const string& filePath);
};

class Act1Level5 : public ILevelLogic {
private:
    loadData loader;
    BinaryTree currentTree;
    BinaryTree mirrorTree;

    TreeNode** swappedNodes;
    int swappedCount = 0;


    Act1Level5() : ILevelLogic(loader.data.optimalSteps)
    {
        swappedCount = 0;
        swappedNodes = new TreeNode*[loader.data.totalNodes];
        for (int i = 0; i < loader.data.totalNodes; i++) 
            swappedNodes[i] = nullptr;
    }

    bool isLeaf(TreeNode* node) const {
        return (node != nullptr && node->left == nullptr && node->right == nullptr);
    }

    bool isNodeSwapped(TreeNode* node) const {
        for (int i = 0; i < swappedCount; i++) {
            if (swappedNodes[i] == node) return true;
        }
        return false;
    }

    bool areChildrenReady(TreeNode* node)const;

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

