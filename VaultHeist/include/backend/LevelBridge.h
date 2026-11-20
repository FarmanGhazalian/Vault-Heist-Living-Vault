#pragma once
#include <string>
#include <memory>
#include "backend/ILevelLogic.h"
#include "backend/PlayerAction.h"
#include "backend/TreeNode.h"

// Forward declarations of Act 1 levels
class Act1Level1;
class Act1Level2;
class Act1Level3;
class Act1Level4;
class Act1Level5;

// LevelBridge: Mediator between GUI screens and Backend Level Logic.
// Keeps GUI code decoupled from backend implementation details.
class LevelBridge
{
public:
    LevelBridge();
    ~LevelBridge();

    bool loadLevel(int act, int level, const std::string& jsonPath);

    // Core Interaction
    ActionResult processAction(const PlayerAction& action);
    const LevelState& getState() const;
    bool isFinished() const;
    bool isWon() const;

    // Tree Access for Visualizer
    TreeNode* getRootNode();

    // Contextual Info for GUI HUD
    std::string getObjective() const;
    std::string getHint();
    ILevelLogic* getLogic() const { return m_logic.get(); }

    // Level-specific queries
    int getAct() const { return m_act; }
    int getLevel() const { return m_level; }

    // Act 1.1 Traversal type
    std::string getTraversalType() const;

    // Act 1.2 Sequences
    int getTotalNodes() const;
    int getPreorderValue(int idx) const;
    int getInorderValue(int idx) const;

    // Act 1.4 Rooms
    int getRoomA() const;
    int getRoomB() const;
    bool isLCAFound() const;
    int getCurrentPathIndex() const;
    int getPathCount() const;

    // Act 1.3 Heights phase done
    bool isHeightsPhaseDone() const;

private:
    int m_act;
    int m_level;
    std::unique_ptr<ILevelLogic> m_logic;

    // Cached pointers for specific levels
    Act1Level1* m_l1;
    Act1Level2* m_l2;
    Act1Level3* m_l3;
    Act1Level4* m_l4;
    Act1Level5* m_l5;

    LevelState m_fallbackState;
};
