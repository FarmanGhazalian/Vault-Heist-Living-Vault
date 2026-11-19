#pragma once
#include "LevelScreenBase.h"
#include "Act3Level5.h"

class GameEngine;

// Act3Level5Screen: Order-Statistic Heist (Boss)
// Navigate to the k-th smallest element using subtree sizes.
class Act3Level5Screen : public LevelScreenBase
{
public:
    explicit Act3Level5Screen(GameEngine& engine);

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
    void drawSubtreeSizes(sf::RenderTarget& target, TreeNode* node);

    Act3Level5 m_logic;

    TreeNode* m_wrongNode;
    float     m_wrongTimer;
    TreeNode* m_correctNode;
    float     m_correctTimer;
    float     m_animTimer;
};
