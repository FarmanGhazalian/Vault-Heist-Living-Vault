#pragma once
#include "LevelScreenBase.h"
#include "Act2Level4.h"
#include <vector>

class Act2Level4Screen : public LevelScreenBase
{
public:
    Act2Level4Screen(GameEngine& engine);
    ~Act2Level4Screen() override = default;

    void update(float dt) override;
    void handleEvent(const sf::Event& event) override;

protected:
    void onNodeClicked(TreeNode* node) override;
    void onHintClicked() override;
    void onResetClicked() override;
    void onCustomDraw(sf::RenderTarget& target) override;
    void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds) override;

private:
    Act2Level4 m_logic;
    int m_lo;
    int m_hi;
    std::vector<int> m_collectedKeys;

    // Post-level Pruning View
    bool m_showPruningView;
    float m_pruningTimer;
    Button m_btnPruningContinue;

    float m_shakeTimer;
    TreeNode* m_shakingNode;
    TreeNode* m_hoverNode;

    void resetLevel();
    void syncWithState();
};
