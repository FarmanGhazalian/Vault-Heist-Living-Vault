#pragma once
#include "Screen.h"
#include "UIKit.h"

class GameEngine;

constexpr int MODE_LEADERBOARD = 0;
constexpr int MODE_HOW_TO_PLAY = 1;

class InfoScreen : public Screen
{
private:
	GameEngine& engine;
	sf::Vector2f windowSize;
	int mode;

	Button btnBack;
	Panel contentPanel;

	void updateLayout(float w, float h);

public:
	InfoScreen(GameEngine& gameEngine, int screenMode);

	void handleEvent(const sf::Event& event) override;
	void update(float dt) override;
	void draw(sf::RenderTarget& target) override;
	void onResize(unsigned int width, unsigned int height) override;
};
