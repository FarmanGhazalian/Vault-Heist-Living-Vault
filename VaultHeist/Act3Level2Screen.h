#pragma once
#include "LevelScreenBase.h"
#include "Act3Level2.h"

class GameEngine;

// Act3Level2Screen: Two-phase identification puzzle.
// Phase 1: Click the unbalanced node.
// Phase 2: Choose the rotation case (LL/RR/LR/RL).
class Act3Level2Screen : public LevelScreenBase
{
public:
    explicit Act3Level2Screen(GameEngine& engine);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;

protected:
    void onNodeClicked(TreeNode* node) override;
    void onHintClicked() override;
    void onResetClicked() override;
    void onCustomDraw(sf::RenderTarget& target) override;
    void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds) override;

private:
    void resetLevel();
    void syncWithState();
    void drawBFBadges(sf::RenderTarget& target, TreeNode* node);
    int  computeBF(TreeNode* node) const;

    Act3Level2 m_logic;

    // Case selection buttons (shown in Phase 2)
    Button m_btnLL;
    Button m_btnRR;
    Button m_btnLR;
    Button m_btnRL;

    TreeNode*  m_selectedNode;   // Phase 1 selection
    TreeNode*  m_wrongNode;
    float      m_wrongTimer;
    TreeNode*  m_correctNode;
    float      m_correctTimer;

    float      m_animTimer;
};
