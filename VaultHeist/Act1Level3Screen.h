#pragma once
#include "LevelScreenBase.h"
#include "LevelBridge.h"
#include <map>
#include <string>

// Act1Level3Screen: Alarm Spreads Upward with Side Panel Details & Result Modal
class Act1Level3Screen : public LevelScreenBase
{
public:
    Act1Level3Screen(GameEngine& engine);
    ~Act1Level3Screen() override = default;

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

    // Node solved heights: [val -> enteredHeight]
    std::map<int, int> m_solvedHeights;

    // Currently focused node input
    TreeNode* m_focusedNode;
    std::string m_nodeInputBuffer;

    // Diameter input box (Phase 2)
    bool m_diameterBoxFocused;
    std::string m_diameterInputBuffer;
    sf::FloatRect m_diameterBoxBounds;
    Button m_submitDiaBtn;
    std::vector<std::pair<Button, char>> m_numpadBtns;

    // Shake & Error feedback
    float m_shakeTimer;
    TreeNode* m_shakingNode;
    bool m_shakeDiameterBox;

    // Tooltip hover
    TreeNode* m_hoverNode;

    void submitHeight();
    void submitDiameter();
    void drawTooltip(sf::RenderTarget& target, const Theme& theme);
};
