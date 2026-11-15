#pragma once
#include <SFML/Graphics.hpp>

class GameEngine;

class Screen {
public:
    virtual ~Screen() = default;
    virtual void handleEvent(GameEngine& engine, const sf::Event& event) = 0;
    virtual void update(GameEngine& engine, float dt) = 0;
    virtual void draw(GameEngine& engine, sf::RenderWindow& window) = 0;
};
