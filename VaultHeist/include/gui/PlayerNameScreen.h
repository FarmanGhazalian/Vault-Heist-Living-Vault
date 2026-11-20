#pragma once
#include "gui/Screen.h"
#include "gui/UIKit.h"

class GameEngine;

class PlayerNameScreen : public Screen
{
private:
    GameEngine& engine;
    sf::Vector2f windowSize;

    std::string m_inputText;
    Button btnConfirm;
    float m_cursorTimer;
    bool m_cursorVisible;

    void updateLayout(float w, float h);

public:
    PlayerNameScreen(GameEngine& gameEngine);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void onResize(unsigned int width, unsigned int height) override;
};
