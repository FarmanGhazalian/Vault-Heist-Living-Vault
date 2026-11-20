#pragma once
#include "gui/Screen.h"
#include "gui/UIKit.h"

class GameEngine;

class PlaceholderScreen : public Screen
{
private:
	GameEngine& engine;
	Panel centerPanel;
	Button btnReady;
	Button btnBack;
	sf::Vector2f windowSize;

	void updateLayout(float w, float h);

public:
	PlaceholderScreen(GameEngine& gameEngine);

	void handleEvent(const sf::Event& event) override;
	void update(float dt) override;
	void draw(sf::RenderTarget& target) override;
	void onResize(unsigned int width, unsigned int height) override;
};
