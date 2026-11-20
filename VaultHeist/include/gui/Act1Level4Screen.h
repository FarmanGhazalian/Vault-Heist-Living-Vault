#pragma once
#include "gui/LevelScreenBase.h"
#include "backend/LevelBridge.h"
#include <vector>
#include <string>

// Act1Level4Screen: Where Escape Routes Meet with Side Panel Details & Result Modal
class Act1Level4Screen : public LevelScreenBase
{
public:
    Act1Level4Screen(GameEngine& engine);
    ~Act1Level4Screen() override = default;

    void update(float dt) override;
    void handleEvent(const sf::Event& event) override;

protected:
    void onResetClicked() override;
    void onNodeClicked(TreeNode* node) override;
    void onCustomDraw(sf::RenderTarget& target) override;
    void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds) override;
    std::string getLevelSpecificHint() override;
    ILevelLogic* getLevelLogic() override;
    void resetLevel() override;
    void syncWithState() override;

private:
    LevelBridge m_bridge;

    int m_roomA;
    int m_roomB;
    int m_lcaNodeVal;

    // Traced path rooms in order
    std::vector<int> m_tracedPath;

    // Shake & Error feedback
    float m_shakeTimer;
    TreeNode* m_shakingNode;

    // Tooltip hover
    TreeNode* m_hoverNode;

    void drawTooltip(sf::RenderTarget& target, const Theme& theme);
};
