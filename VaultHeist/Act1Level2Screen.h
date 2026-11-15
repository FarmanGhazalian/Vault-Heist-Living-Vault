#pragma once
#include "LevelScreenBase.h"
#include "Act1Level2.h"
#include <memory>
#include <vector>
#include <string>

struct TokenSlot {
    int parentVal = -1;
    int direction = 0; // 0 = left, 1 = right
    int expectedVal = -1;
    sf::Vector2f pos;
    bool filled = false;
    int currentVal = -1;
};

struct TokenItem {
    int value = 0;
    sf::Vector2f homePos;
    sf::Vector2f currentPos;
    bool isPlaced = false;
    bool isDragging = false;
    float wrongTimer = 0.0f;
};

class Act1Level2Screen : public LevelScreenBase {
private:
    std::unique_ptr<Act1Level2> levelLogic;
    std::string jsonPath;
    std::vector<int> preorderList;
    std::vector<int> inorderList;
    std::vector<TokenItem> tokens;
    std::vector<TokenSlot> slots;
    int draggingIdx = -1;
    sf::Vector2f dragOffset;

    void loadLevelData();
    void restartLevelDirect();
    void setupTreeSlots();

public:
    Act1Level2Screen();

    void onLevelEvent(GameEngine& engine, const sf::Event& event) override;
    void updateLevel(GameEngine& engine, float dt) override;
    void drawLevel(GameEngine& engine, sf::RenderWindow& window) override;
    void restartLevel(GameEngine& engine) override;
    void requestHint(GameEngine& engine) override;
    ILevelLogic* getLevelLogic() override { return levelLogic.get(); }
    void goToNextLevel(GameEngine& engine) override;
};
