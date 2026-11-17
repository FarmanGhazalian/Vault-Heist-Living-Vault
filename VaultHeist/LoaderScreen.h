#pragma once
#include "Screen.h"
#include <SFML/Graphics.hpp>
#include <string>

class GameEngine;

class LoaderScreen : public Screen
{
private:
	GameEngine& engine;
	sf::Vector2f windowSize;

	float elapsed;
	float totalDuration;
	float vaultAngle;
	bool finished;

	std::string fullSubtitle;
	std::string visibleSubtitle;

	void updateLayout(float w, float h);
	void proceedToMenu();

public:
	LoaderScreen(GameEngine& gameEngine);

	void handleEvent(const sf::Event& event) override;
	void update(float dt) override;
	void draw(sf::RenderTarget& target) override;
	void onResize(unsigned int width, unsigned int height) override;
};
