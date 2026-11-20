#pragma once
#include "gui/LevelScreenBase.h"
#include "backend/PlayerAction.h"
#include <memory>
#include <vector>

class ILevelLogic;
class MaxHeap;

// Act4LevelScreen: Unified Screen for Act 4 (The Heap Fortress, Levels 4.1 to 4.5)
// Visualizes the complete binary tree structure of the MaxHeap with array mapping underneath.
// Supports:
// - Clicking pairs of nodes to trigger SWAP_ELEMENTS
// - Extracting root / sift operations
// - Full Action Bar (Hint, Undo, Replay, Reset)
// - Centered zoom/pan, tooltip inspection, and result modal
class Act4LevelScreen : public LevelScreenBase
{
public:
    Act4LevelScreen(GameEngine& engine, int levelNum);
    ~Act4LevelScreen() override = default;

    void update(float dt) override;
    void handleEvent(const sf::Event& event) override;

protected:
    void onNodeClicked(TreeNode* node) override;
    void onResetClicked() override;
    void onCustomDraw(sf::RenderTarget& target) override;
    void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds) override;
    std::string getLevelSpecificHint() override;
    ILevelLogic* getLevelLogic() override;
    void resetLevel() override;
    void syncWithState() override;
    void reapplyActionSilently(const PlayerAction& action) override;

private:
    int m_levelNum;
    std::unique_ptr<ILevelLogic> m_logic;

    // Selection for swap
    int m_firstSelectedIdx;
    TreeNode* m_firstSelectedNode;

    // Transient feedback
    float m_shakeTimer;
    TreeNode* m_shakingNode;
    float m_correctTimer;
    TreeNode* m_correctNode;
    TreeNode* m_hoverNode;

    // Dynamic heap tree structure
    std::vector<std::unique_ptr<TreeNode>> m_heapTreeNodes;

    const MaxHeap* getHeap() const;
    void rebuildTreeFromHeap();
    int getNodeHeapIndex(TreeNode* node) const;
};
