#pragma once
#include "gui/LevelScreenBase.h"
#include "backend/Act2Level5.h"
#include <vector>

class Act2Level5Screen : public LevelScreenBase
{
public:
    Act2Level5Screen(GameEngine& engine);
    ~Act2Level5Screen() override = default;

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
    void reapplyActionSilently(const PlayerAction& action) override;

private:
    Act2Level5 m_logic;
    int m_currentTarget;
    std::vector<int> m_visitedPath;

    // Shrinking capacity animation
    int m_initialCap;
    int m_lastCapacity;
    float m_capacityPulseTimer;

    // Cutscene Modal
    bool m_showCutscene;
    Button m_btnCutsceneContinue;

    float m_shakeTimer;
    TreeNode* m_shakingNode;
    float m_pulseTimer;
    TreeNode* m_hoverNode;
};
