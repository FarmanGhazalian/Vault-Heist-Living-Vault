#pragma once
#include "LevelScreenBase.h"
#include "LevelBridge.h"
#include <map>
#include <string>

// Act1Level5Screen: Mirror the Vault with Side Panel Details & Result Modal
class Act1Level5Screen : public LevelScreenBase
{
public:
    Act1Level5Screen(GameEngine& engine);
    ~Act1Level5Screen() override = default;

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

    // Auto-Mirror Button (Placed in side panel)
    Button m_autoMirrorBtn;

    // Selected node inspection in side panel
    TreeNode* m_selectedInspectNode;

    // Subtree Swap Animation Tween
    struct NodeAnim
    {
        sf::Vector2f startPos;
        sf::Vector2f targetPos;
    };
    std::map<int, NodeAnim> m_nodeAnims;
    float m_swapAnimTimer;
    static constexpr float SwapAnimDuration = 0.5f;

    // Shake & Feedback
    float m_shakeTimer;
    TreeNode* m_shakingNode;
    TreeNode* m_neutralNode;
    float m_neutralTimer;

    // Tooltip
    TreeNode* m_hoverNode;

    void triggerAutoMirror();
    void startSwapAnimation();
};
