#include "gui/CampaignScreen.h"
#include "gui/MainMenuScreen.h"
#include "gui/Act1Level1Screen.h"
#include "gui/Act1Level2Screen.h"
#include "gui/Act1Level3Screen.h"
#include "gui/Act1Level4Screen.h"
#include "gui/Act1Level5Screen.h"
#include "gui/Act2Level1Screen.h"
#include "gui/Act2Level2Screen.h"
#include "gui/Act2Level3Screen.h"
#include "gui/Act2Level4Screen.h"
#include "gui/Act2Level5Screen.h"
#include "gui/Act3Level1Screen.h"
#include "gui/Act3Level2Screen.h"
#include "gui/Act3Level3Screen.h"
#include "gui/Act3Level4Screen.h"
#include "gui/Act3Level5Screen.h"
#include "gui/Act4LevelScreen.h"
#include "gui/GameEngine.h"
#include "backend/SaveSystem.h"
#include <algorithm>
#include <cmath>

CampaignScreen::CampaignScreen(GameEngine& gameEngine)
	: engine(gameEngine), windowSize(gameEngine.getWindowSize()),
	  currentView(VIEW_ACTS), selectedAct(1)
{
	SaveSystem::load();
	setupData();
	updateLayout(windowSize.x, windowSize.y);
}

void CampaignScreen::setupData()
{
	acts.clear();
	acts.push_back({ 1, "THE BLUEPRINT WING", "Binary Trees & Hierarchies" });
	acts.push_back({ 2, "THE SORTED WING", "Binary Search Trees & Partitioning" });
	acts.push_back({ 3, "THE ARCHITECT'S WING", "AVL Trees & Balance Rotations" });
	acts.push_back({ 4, "THE HEAP FORTRESS", "Max Heap & Priority Sorting" });

	levels.clear();
	// Act 1
	levels.push_back({ 1, 1, "Vault Patrol Entry", 0, false });
	levels.push_back({ 1, 2, "Decode the Blueprint", 0, false });
	levels.push_back({ 1, 3, "Alarm Spreads Upward", 0, false });
	levels.push_back({ 1, 4, "Where Escape Routes Meet", 0, false });
	levels.push_back({ 1, 5, "Mirror the Vault", 0, false });

	// Act 2
	levels.push_back({ 2, 1, "Search Heist", 0, false });
	levels.push_back({ 2, 2, "Stash Loot", 0, false });
	levels.push_back({ 2, 3, "Steal and Collapse", 0, false });
	levels.push_back({ 2, 4, "Range Raid", 0, false });
	levels.push_back({ 2, 5, "The Lazy Architect (Boss)", 0, false });

	// Act 3
	levels.push_back({ 3, 1, "Read the Badges", 0, false });
	levels.push_back({ 3, 2, "Manual Repair: Single", 0, false });
	levels.push_back({ 3, 3, "Manual Repair: Double", 0, false });
	levels.push_back({ 3, 4, "Auto Mode & Prediction", 0, false });
	levels.push_back({ 3, 5, "Order-Statistic Heist", 0, false });

	// Act 4
	levels.push_back({ 4, 1, "Activate the Vault", 0, false });
	levels.push_back({ 4, 2, "Repair the Security Locks", 0, false });
	levels.push_back({ 4, 3, "Extract the Greatest Treasure", 0, false });
	levels.push_back({ 4, 4, "Race Against the Guards", 0, false });
	levels.push_back({ 4, 5, "Final Vault Boss", 0, false });
}

void CampaignScreen::updateLayout(float w, float h)
{
	windowSize = sf::Vector2f(w, h);

	btnBack = Button("< BACK", sf::FloatRect(sf::Vector2f(24.0f, 24.0f), sf::Vector2f(100.0f, 36.0f)), BTN_BACK);

	// Layout 4 Act Cards
	actButtons.clear();
	float cardW = std::min(440.0f, (w - 120.0f) * 0.5f);
	float cardH = 135.0f;
	float gapX = 30.0f;
	float gapY = 24.0f;
	float totalGridW = cardW * 2.0f + gapX;
	float startX = (w - totalGridW) * 0.5f;
	float startY = (h - (cardH * 2.0f + gapY)) * 0.5f + 30.0f;

	for (size_t i = 0; i < acts.size(); i++)
	{
		float col = static_cast<float>(i % 2);
		float row = static_cast<float>(i / 2);
		float x = startX + col * (cardW + gapX);
		float y = startY + row * (cardH + gapY);

		std::string label = "ACT " + std::to_string(acts[i].actNum) + " - " + acts[i].name;
		actButtons.push_back(Button(label, sf::FloatRect(sf::Vector2f(x, y), sf::Vector2f(cardW, cardH)), BTN_PRIMARY));
	}

	// Layout 5 Level Cards
	levelButtons.clear();
	float lvlW = std::min(190.0f, (w - 120.0f) / 5.0f);
	float lvlH = 220.0f;
	float lGap = 16.0f;
	float totalLvlW = 5.0f * lvlW + 4.0f * lGap;
	float lStartX = (w - totalLvlW) * 0.5f;
	float lStartY = (h - lvlH) * 0.5f + 20.0f;

	for (int i = 0; i < 5; i++)
	{
		float lx = lStartX + static_cast<float>(i) * (lvlW + lGap);
		std::string lLabel = "LEVEL " + std::to_string(selectedAct) + "." + std::to_string(i + 1);
		levelButtons.push_back(Button(lLabel, sf::FloatRect(sf::Vector2f(lx, lStartY), sf::Vector2f(lvlW, lvlH)), BTN_SECONDARY));
	}
}

void CampaignScreen::onResize(unsigned int width, unsigned int height)
{
	updateLayout(static_cast<float>(width), static_cast<float>(height));
}

void CampaignScreen::handleEvent(const sf::Event& event)
{
	if (const auto* r = event.getIf<sf::Event::Resized>())
	{
		onResize(r->size.x, r->size.y);
	}

	btnBack.handleEvent(event);

	if (btnBack.isClicked())
	{
		btnBack.resetClick();
		engine.audio.playClick();
		if (currentView == VIEW_LEVELS)
		{
			currentView = VIEW_ACTS;
			updateLayout(windowSize.x, windowSize.y);
		}
		else
		{
			engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
		}
		return;
	}

	if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
	{
		if (kp->code == sf::Keyboard::Key::Escape)
		{
			engine.audio.playClick();
			if (currentView == VIEW_LEVELS)
			{
				currentView = VIEW_ACTS;
			}
			else
			{
				engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
			}
			return;
		}
		else if (kp->code == sf::Keyboard::Key::U && kp->control && kp->shift)
		{
			bool unlocked = SaveSystem::toggleUnlockAll();
			if (unlocked)
			{
				engine.audio.playWin();
				engine.showToast("DEV OVERRIDE: All levels unlocked!", TOAST_INFO);
			}
			else
			{
				engine.audio.playClick();
				engine.showToast("DEV OVERRIDE: Levels locked back to start state.", TOAST_INFO);
			}
			return;
		}
	}

	if (currentView == VIEW_ACTS)
	{
		for (size_t i = 0; i < actButtons.size(); i++)
		{
			actButtons[i].handleEvent(event);
			bool clicked = actButtons[i].isClicked();
			if (!clicked)
			{
				if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
				{
					if (mb->button == sf::Mouse::Button::Left && actButtons[i].bounds.contains(sf::Vector2f(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y))))
					{
						clicked = true;
					}
				}
			}

			if (clicked)
			{
				actButtons[i].resetClick();
				engine.audio.playClick();
				selectedAct = static_cast<int>(i) + 1;
				currentView = VIEW_LEVELS;
				updateLayout(windowSize.x, windowSize.y);
				return;
			}
		}
	}
	else
	{
		for (size_t i = 0; i < levelButtons.size(); i++)
		{
			levelButtons[i].handleEvent(event);
			bool clicked = levelButtons[i].isClicked();
			if (!clicked)
			{
				if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
				{
					if (mb->button == sf::Mouse::Button::Left && levelButtons[i].bounds.contains(sf::Vector2f(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y))))
					{
						clicked = true;
					}
				}
			}

			if (clicked)
			{
				levelButtons[i].resetClick();
				engine.audio.playClick();

				int lvlIdx = static_cast<int>(i) + 1;
				if (!SaveSystem::isUnlocked(selectedAct, lvlIdx))
				{
					engine.audio.playWrong();
					engine.showToast("CHAMBER LOCKED - Complete previous level first!", TOAST_ERROR);
					return;
				}

				if (selectedAct == 1)
				{
					if (i == 0) { engine.changeScreen(std::make_unique<Act1Level1Screen>(engine)); return; }
					if (i == 1) { engine.changeScreen(std::make_unique<Act1Level2Screen>(engine)); return; }
					if (i == 2) { engine.changeScreen(std::make_unique<Act1Level3Screen>(engine)); return; }
					if (i == 3) { engine.changeScreen(std::make_unique<Act1Level4Screen>(engine)); return; }
					if (i == 4) { engine.changeScreen(std::make_unique<Act1Level5Screen>(engine)); return; }
				}
				else if (selectedAct == 2)
				{
					if (i == 0) { engine.changeScreen(std::make_unique<Act2Level1Screen>(engine)); return; }
					if (i == 1) { engine.changeScreen(std::make_unique<Act2Level2Screen>(engine)); return; }
					if (i == 2) { engine.changeScreen(std::make_unique<Act2Level3Screen>(engine)); return; }
					if (i == 3) { engine.changeScreen(std::make_unique<Act2Level4Screen>(engine)); return; }
					if (i == 4) { engine.changeScreen(std::make_unique<Act2Level5Screen>(engine)); return; }
				}
				else if (selectedAct == 3)
				{
					if (i == 0) { engine.changeScreen(std::make_unique<Act3Level1Screen>(engine)); return; }
					if (i == 1) { engine.changeScreen(std::make_unique<Act3Level2Screen>(engine)); return; }
					if (i == 2) { engine.changeScreen(std::make_unique<Act3Level3Screen>(engine)); return; }
					if (i == 3) { engine.changeScreen(std::make_unique<Act3Level4Screen>(engine)); return; }
					if (i == 4) { engine.changeScreen(std::make_unique<Act3Level5Screen>(engine)); return; }
				}
				else if (selectedAct == 4)
				{
					if (i >= 0 && i < 5)
					{
						engine.changeScreen(std::make_unique<Act4LevelScreen>(engine, static_cast<int>(i) + 1));
						return;
					}
				}
				else
				{
					std::string actTitle = acts[selectedAct - 1].name;
					engine.showToast("Act " + std::to_string(selectedAct) + " (" + actTitle + ") unlocks in future acts!", TOAST_INFO);
					return;
				}
			}
		}
	}
}

void CampaignScreen::update(float dt)
{
	(void)dt;
}

void CampaignScreen::draw(sf::RenderTarget& target)
{
	btnBack.draw(target, engine.theme);
	float cx = windowSize.x * 0.5f;

	if (currentView == VIEW_ACTS)
	{
		sf::Text title(engine.theme.fontUI, "CAMPAIGN - SELECT OPERATION SECTOR", 28);
		title.setFillColor(Theme::TextPrimary);
		sf::FloatRect tb = title.getLocalBounds();
		title.setPosition(sf::Vector2f(std::round(cx - tb.size.x * 0.5f - tb.position.x), 36.0f));
		target.draw(title);

		sf::Text sub(engine.theme.fontUI, "Choose an Act to access vault data structure chambers", 15);
		sub.setFillColor(Theme::TextMuted);
		sf::FloatRect sb = sub.getLocalBounds();
		sub.setPosition(sf::Vector2f(std::round(cx - sb.size.x * 0.5f - sb.position.x), 74.0f));
		target.draw(sub);

		for (size_t i = 0; i < actButtons.size(); i++)
		{
			actButtons[i].draw(target, engine.theme);

			sf::Text actSub(engine.theme.fontUI, acts[i].subtitle, 13);
			actSub.setFillColor(Theme::TextMuted);
			actSub.setPosition(sf::Vector2f(actButtons[i].bounds.position.x + 18.0f, actButtons[i].bounds.position.y + 45.0f));
			target.draw(actSub);

			// Count stars in this act
			int actStars = 0;
			int actNum = acts[i].actNum;
			for (int l = 1; l <= 5; l++)
			{
				int s = SaveSystem::getStars(actNum, l);
				if (s > 0) actStars += s;
			}
			std::string badgeStr = "5 MISSIONS - " + std::to_string(actStars) + "/15 STARS";
			sf::Text badge(engine.theme.fontMono, badgeStr, 12);
			badge.setFillColor(Theme::AccentYellow);
			badge.setPosition(sf::Vector2f(actButtons[i].bounds.position.x + 18.0f, actButtons[i].bounds.position.y + actButtons[i].bounds.size.y - 30.0f));
			target.draw(badge);
		}
	}
	else
	{
		std::string actName = acts[selectedAct - 1].name;
		sf::Text title(engine.theme.fontUI, "ACT " + std::to_string(selectedAct) + " - " + actName, 26);
		title.setFillColor(Theme::BorderCyan);
		sf::FloatRect tb = title.getLocalBounds();
		title.setPosition(sf::Vector2f(std::round(cx - tb.size.x * 0.5f - tb.position.x), 36.0f));
		target.draw(title);

		sf::Text sub(engine.theme.fontUI, "Select security chamber to initiate breach", 15);
		sub.setFillColor(Theme::TextMuted);
		sf::FloatRect sb = sub.getLocalBounds();
		sub.setPosition(sf::Vector2f(std::round(cx - sb.size.x * 0.5f - sb.position.x), 72.0f));
		target.draw(sub);

		for (size_t i = 0; i < levelButtons.size(); i++)
		{
			levelButtons[i].draw(target, engine.theme);

			std::string lTitle = "";
			for (const auto& lvl : levels)
			{
				if (lvl.act == selectedAct && lvl.levelNum == static_cast<int>(i) + 1)
				{
					lTitle = lvl.title;
					break;
				}
			}

			// Wrapped / fitted title inside card
			float maxTextW = levelButtons[i].bounds.size.x - 24.0f;
			UIText::drawWrapped(target, engine.theme.fontUI, lTitle, 13, Theme::TextPrimary,
				sf::Vector2f(levelButtons[i].bounds.position.x + 12.0f, levelButtons[i].bounds.position.y + 48.0f), maxTextW);

			// Stars
			int earnedStars = SaveSystem::getStars(selectedAct, static_cast<int>(i) + 1);
			std::string starsStr = "";
			for (int s = 0; s < 3; s++)
			{
				starsStr += (s < earnedStars) ? "* " : "- ";
			}
			sf::Text starsTxt(engine.theme.fontMono, starsStr, 15);
			starsTxt.setFillColor(earnedStars > 0 ? Theme::AccentYellow : sf::Color(100, 116, 139));
			starsTxt.setPosition(sf::Vector2f(levelButtons[i].bounds.position.x + 12.0f, levelButtons[i].bounds.position.y + levelButtons[i].bounds.size.y - 58.0f));
			target.draw(starsTxt);

			// Lock status
			bool unlocked = SaveSystem::isUnlocked(selectedAct, static_cast<int>(i) + 1);
			sf::Text statusTxt(engine.theme.fontMono, unlocked ? "UNLOCKED" : "LOCKED", 11);
			statusTxt.setFillColor(unlocked ? Theme::SuccessEmerald : Theme::CrimsonAlarm);
			statusTxt.setPosition(sf::Vector2f(levelButtons[i].bounds.position.x + 12.0f, levelButtons[i].bounds.position.y + levelButtons[i].bounds.size.y - 28.0f));
			target.draw(statusTxt);
		}
	}
}
