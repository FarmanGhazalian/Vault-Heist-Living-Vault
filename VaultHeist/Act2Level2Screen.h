#pragma once
#include "LevelScreenBase.h"
#include "Act2Level2.h"
#include <vector>

class Act2Level2Screen : public LevelScreenBase
{
public:
    Act2Level2Screen(GameEngine& engine);
    ~Act2Level2Screen() override = default;

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
    Act2Level2 m_logic;
    int m_currentKey;
    std::vector<int> m_visitedPath;

    // Vacant child slots for placement
    struct VacantSlot
    {
        int parentVal;
        int direction; // 0 = left, 1 = right
        sf::Vector2f pos;
        sf::FloatRect bounds;
        bool hovered;
    };
    std::vector<VacantSlot> m_vacantSlots;

    // Visual animations
    float m_shakeTimer;
    TreeNode* m_shakingNode;
    int m_shakingSlotIdx;
    float m_newGrowthTimer;
    int m_grownNodeVal;
    TreeNode* m_hoverNode;

    void computeVacantSlots();
};
