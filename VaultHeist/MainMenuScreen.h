#pragma once
#include "Screen.h"
#include "UIKit.h"
#include <vector>

class GameEngine;

struct AmbientNode
{
	sf::Vector2f pos;
	sf::Vector2f vel;
	float radius;
};

class MainMenuScreen : public Screen
{
private:
	GameEngine& engine;
	sf::Vector2f windowSize;

	Button btnCampaign;
	Button btnEndless;
	Button btnSandbox;
	Button btnLeaderboard;
	Button btnHowToPlay;
	Button btnSettings;
	Button btnQuit;
	Button btnMute;

	std::vector<AmbientNode> ambientNodes;
	float animTimer;

	void updateLayout(float w, float h);

public:
	MainMenuScreen(GameEngine& gameEngine);

	void handleEvent(const sf::Event& event) override;
	void update(float dt) override;
	void draw(sf::RenderTarget& target) override;
	void onResize(unsigned int width, unsigned int height) override;
};
