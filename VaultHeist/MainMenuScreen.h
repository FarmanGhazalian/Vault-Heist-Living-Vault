#pragma once
#include "Screen.h"
#include "UIKit.h"

class MainMenuScreen : public Screen {
public:
    Button btnCampaign;
    Button btnEndless;
    Button btnSandbox;
    Button btnLeaderboard;
    Button btnSettings;
    Button btnQuit;
    Button btnSound;
    float time = 0.0f;

    MainMenuScreen();
    void handleEvent(class GameEngine& engine, const sf::Event& event) override;
    void update(class GameEngine& engine, float dt) override;
    void draw(class GameEngine& engine, sf::RenderWindow& window) override;
};
