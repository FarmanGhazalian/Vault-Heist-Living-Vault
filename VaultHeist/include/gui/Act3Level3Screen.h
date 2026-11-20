#pragma once
#include "gui/LevelScreenBase.h"
#include "backend/Act3Level3.h"

class GameEngine;

// Act3Level3Screen: Manual Repair - Double Rotations
// Phase 1: CHOOSE_CASE (LL=0, RR=1, LR=2, RL=3)
// Phase 2: ROTATE actions (ROTATE LEFT / ROTATE RIGHT on selected pivot node)
class Act3Level3Screen : public LevelScreenBase
{
public:
    explicit Act3Level3Screen(GameEngine& engine);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;

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
    void drawBFBadges(sf::RenderTarget& target, TreeNode* node);
    int  computeBF(TreeNode* node) const;

    Act3Level3 m_logic;

    // Phase 1 Case Buttons
    Button m_btnLL;
    Button m_btnRR;
    Button m_btnLR;
    Button m_btnRL;

    // Phase 2 Rotation Buttons
    Button m_btnRotateLeft;
    Button m_btnRotateRight;

    TreeNode* m_selectedPivot;
    TreeNode* m_wrongNode;
    float     m_wrongTimer;
    TreeNode* m_correctNode;
    float     m_correctTimer;
    float     m_animTimer;
};
