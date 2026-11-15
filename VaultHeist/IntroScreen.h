#pragma once
#include "Screen.h"
#include "UIKit.h"

class IntroScreen : public Screen {
public:
    float timer = 0.0f;
    const float totalDuration = 2.6f;
    bool skipped = false;

    IntroScreen() = default;
    void handleEvent(class GameEngine& engine, const sf::Event& event) override;
    void update(class GameEngine& engine, float dt) override;
    void draw(class GameEngine& engine, sf::RenderWindow& window) override;
};
