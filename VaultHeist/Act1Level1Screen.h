#pragma once
#include "LevelScreenBase.h"
#include "LevelBridge.h"
#include <map>
#include <string>

// Act1Level1Screen: Vault Patrol (Traversal Order) with Side Panel Details & Result Modal
class Act1Level1Screen : public LevelScreenBase
{
public:
    Act1Level1Screen(GameEngine& engine);
    ~Act1Level1Screen() override = default;

    void update(float dt) override;
    void handleEvent(const sf::Event& event) override;

protected:
    void onNodeClicked(TreeNode* node) override;
    void onHintClicked() override;
    void onResetClicked() override;
    void onCustomDraw(sf::RenderTarget& target) override;
    void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds) override;

private:
    LevelBridge m_bridge;

    // Visual order numbering for correct clicks: [val -> stepNumber]
    std::map<int, int> m_visitedOrder;
    int m_stepCounter;

    // Error shake & flash
    float m_shakeTimer;
    TreeNode* m_shakingNode;
    sf::Vector2f m_shakeOffset;

    // Hover tooltip info
    TreeNode* m_hoverNode;

    void syncWithState();
    void resetLevel();
    void drawTooltip(sf::RenderTarget& target, const Theme& theme);
};
