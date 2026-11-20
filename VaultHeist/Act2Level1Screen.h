#pragma once
#include "LevelScreenBase.h"
#include "Act2Level1.h"
#include <vector>

class Act2Level1Screen : public LevelScreenBase
{
public:
    Act2Level1Screen(GameEngine& engine);
    ~Act2Level1Screen() override = default;

    void update(float dt) override;
    void handleEvent(const sf::Event& event) override;

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
    Act2Level1 m_logic;
    int m_target;
    std::vector<int> m_visitedPath;

    // Visual animations
    float m_shakeTimer;
    TreeNode* m_shakingNode;
    float m_pulseTimer;
    TreeNode* m_hoverNode;
};
