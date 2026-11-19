#pragma once
#include "LevelScreenBase.h"
#include "Act3Level4.h"

class GameEngine;

// Act3Level4Screen: Auto Mode & Prediction
// Player predicts the rebalance rotation needed for incoming keys:
// LL=0, RR=1, LR=2, RL=3, NONE=4
class Act3Level4Screen : public LevelScreenBase
{
public:
    explicit Act3Level4Screen(GameEngine& engine);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;

protected:
    void onHintClicked() override;
    void onResetClicked() override;
    void onCustomDraw(sf::RenderTarget& target) override;
    void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds) override;

private:
    void resetLevel();
    void syncWithState();
    void drawBFBadges(sf::RenderTarget& target, TreeNode* node);
    int  computeBF(TreeNode* node) const;

    Act3Level4 m_logic;

    Button m_btnLL;
    Button m_btnRR;
    Button m_btnLR;
    Button m_btnRL;
    Button m_btnNone;

    float  m_wrongTimer;
    float  m_correctTimer;
    float  m_animTimer;
};
