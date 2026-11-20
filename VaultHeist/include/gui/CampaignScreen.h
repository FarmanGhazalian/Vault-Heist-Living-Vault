#pragma once
#include "gui/Screen.h"
#include "gui/UIKit.h"
#include <string>
#include <vector>

class GameEngine;

// View constants (no enum)
constexpr int VIEW_ACTS   = 0;
constexpr int VIEW_LEVELS = 1;

struct LevelInfo
{
	int act;
	int levelNum;
	std::string title;
	int stars;
	bool locked;
};

struct ActInfo
{
	int actNum;
	std::string name;
	std::string subtitle;
};

class CampaignScreen : public Screen
{
private:
	GameEngine& engine;
	sf::Vector2f windowSize;

	int currentView;
	int selectedAct;

	Button btnBack;

	// Act cards
	std::vector<ActInfo> acts;
	std::vector<Button> actButtons;

	// Level cards for the selected act
	std::vector<LevelInfo> levels;
	std::vector<Button> levelButtons;

	void setupData();
	void updateLayout(float w, float h);

public:
	CampaignScreen(GameEngine& gameEngine);

	void handleEvent(const sf::Event& event) override;
	void update(float dt) override;
	void draw(sf::RenderTarget& target) override;
	void onResize(unsigned int width, unsigned int height) override;
};
