#pragma once
#include "LevelScreenBase.h"
#include "Act3Level1.h"

class GameEngine;

// Act3Level1Screen: Player clicks all unbalanced AVL nodes (|BF|>=2).
// Shows balance factor badge on every node. BF ripple animation on load.
class Act3Level1Screen : public LevelScreenBase
{
public:
    explicit Act3Level1Screen(GameEngine& engine);

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

    Act3Level1 m_logic;

    // Which node is currently flagged (correct click)
    TreeNode*  m_correctNode;
    float      m_correctTimer;    // glow timer
    TreeNode*  m_wrongNode;
    float      m_wrongTimer;      // shake/flash timer

    // BF ripple animation (bottom-up)
    float      m_rippleTimer;
    bool       m_rippleActive;
    int        m_maxDepth;        // cached for ripple

    int        m_foundCount;
    int        m_totalToFind;
};
