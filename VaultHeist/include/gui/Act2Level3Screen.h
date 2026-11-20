#pragma once
#include "gui/LevelScreenBase.h"
#include "backend/Act2Level3.h"

class Act2Level3Screen : public LevelScreenBase
{
public:
    Act2Level3Screen(GameEngine& engine);
    ~Act2Level3Screen() override = default;

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

private:
    Act2Level3 m_logic;
    int m_currentTarget;

    // Choice buttons: Leaf (0), One child (1), Two children (2)
    Button m_btnLeaf;
    Button m_btnOneChild;
    Button m_btnTwoChildren;

    // Deletion animation states
    float m_animTimer;
    int m_animCase; // 0=leaf, 1=one child, 2=two children
    int m_fadingNodeVal;
    int m_successorNodeVal;
    sf::Vector2f m_deletedNodePos;
    float m_shakeTimer;
    TreeNode* m_shakingNode;
    float m_pulseTimer;
    TreeNode* m_hoverNode;

    void submitCase(int caseVal);
};
