#pragma once
#include "Screen.h"
#include "UIKit.h"
#include <vector>
#include <string>

namespace SelectMode {
    constexpr int ACTS = 0;
    constexpr int LEVELS = 1;
}

struct TileData {
    std::string tag;
    std::string title;
    bool isLocked = false;
    int index = 0;
};

class SelectScreen : public Screen {
public:
    int mode = SelectMode::ACTS;
    int currentAct = 1;
    std::vector<TileData> tiles;
    std::vector<Button> tileButtons;
    Button btnBack;
    Button btnSound;
    float time = 0.0f;

    SelectScreen(int mode = SelectMode::ACTS, int act = 1);

    void initTiles();
    void handleEvent(class GameEngine& engine, const sf::Event& event) override;
    void update(class GameEngine& engine, float dt) override;
    void draw(class GameEngine& engine, sf::RenderWindow& window) override;
};
