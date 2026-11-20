#pragma once
#include "LevelScreenBase.h"
#include "LevelBridge.h"
#include <vector>
#include <string>

// Act1Level2Screen: Decode the Blueprint with Side Panel Details & Result Modal
class Act1Level2Screen : public LevelScreenBase
{
public:
    Act1Level2Screen(GameEngine& engine);
    ~Act1Level2Screen() override = default;

    void update(float dt) override;
    void handleEvent(const sf::Event& event) override;

protected:
    void onResetClicked() override;
    void onCustomDraw(sf::RenderTarget& target) override;
    void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds) override;
    std::string getLevelSpecificHint() override;
    ILevelLogic* getLevelLogic() override;
    void resetLevel() override;
    void syncWithState() override;
    void reapplyActionSilently(const PlayerAction& action) override;

private:
    LevelBridge m_bridge;

    // Blueprint Sequences
    std::vector<int> m_preorder;
    std::vector<int> m_inorder;

    // Draggable Tray Tokens
    struct TrayToken
    {
        int val;
        sf::Vector2f homePos;
        sf::Vector2f currPos;
        bool isPlaced;
        bool isDragging;
        float returnTimer;
        sf::Vector2f returnStart;
    };
    std::vector<TrayToken> m_tokens;
    int m_draggedIndex;

    // Canvas Tree Slots
    struct TreeSlot
    {
        int parentVal; // -1 for root
        int direction; // 0 = left, 1 = right, -1 for root
        sf::Vector2f pos;
        int placedVal; // -1 if empty
        bool active;   // visible/droppable
    };
    std::vector<TreeSlot> m_slots;
    int m_hoveredSlotIndex;

    // Shake & Error feedback
    float m_shakeTimer;
    int m_shakingTokenVal;

    void layoutTrayAndSlots(unsigned int width, unsigned int height);
    void updateSlotPositions();
    int getSlotAtPosition(sf::Vector2f pos);
};
