#include "InfoScreen.h"
#include "MainMenuScreen.h"
#include "GameEngine.h"
#include <cmath>
#include <algorithm>

InfoScreen::InfoScreen(GameEngine& gameEngine, int screenMode)
	: engine(gameEngine), windowSize(gameEngine.getWindowSize()), mode(screenMode)
{
	updateLayout(windowSize.x, windowSize.y);
}

void InfoScreen::updateLayout(float w, float h)
{
	windowSize = sf::Vector2f(w, h);

	// Back Button at top-left
	btnBack = Button("< BACK", sf::FloatRect(sf::Vector2f(24.0f, 24.0f), sf::Vector2f(100.0f, 36.0f)), BTN_BACK);

	// Content Panel centered
	float pw = std::min(860.0f, w - 80.0f);
	float ph = std::min(520.0f, h - 140.0f);
	float px = (w - pw) * 0.5f;
	float py = 90.0f;

	std::string pTitle = (mode == MODE_LEADERBOARD) ? "TERMINAL // LOCAL AGENT ARCHIVES" : "SYSTEM MANUAL // FIELD AGENT DIRECTIVES";
	contentPanel = Panel(sf::FloatRect(sf::Vector2f(px, py), sf::Vector2f(pw, ph)), pTitle, Theme::BorderCyan, 1.2f);
}

void InfoScreen::onResize(unsigned int width, unsigned int height)
{
	updateLayout(static_cast<float>(width), static_cast<float>(height));
}

void InfoScreen::handleEvent(const sf::Event& event)
{
	btnBack.handleEvent(event);

	if (btnBack.isClicked())
	{
		btnBack.resetClick();
		engine.audio.playClick();
		engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
		return;
	}

	if (event.is<sf::Event::KeyPressed>())
	{
		if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
		{
			if (kp->code == sf::Keyboard::Key::Escape)
			{
				engine.audio.playClick();
				engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
				return;
			}
		}
	}
}

void InfoScreen::update(float dt)
{
	(void)dt;
}

void InfoScreen::draw(sf::RenderTarget& target)
{
	// Back button
	btnBack.draw(target, engine.theme);

	float cx = windowSize.x * 0.5f;

	// Screen Header Title
	std::string hTitle = (mode == MODE_LEADERBOARD) ? "CYBER-VAULT ARCHIVES // LEADERBOARD" : "OPERATOR MANUAL // HOW TO PLAY";
	sf::Text title(engine.theme.fontUI, hTitle, 26);
	title.setFillColor(Theme::TextPrimary);
	sf::FloatRect tb = title.getLocalBounds();
	title.setPosition(sf::Vector2f(std::round(cx - tb.size.x * 0.5f - tb.position.x), 32.0f));
	target.draw(title);

	// Panel Body
	contentPanel.draw(target, engine.theme);

	float px = contentPanel.bounds.position.x + 24.0f;
	float py = contentPanel.bounds.position.y + 50.0f;

	if (mode == MODE_LEADERBOARD)
	{
		// Table Headers
		sf::Text headers(engine.theme.fontMono, "RANK    AGENT CODE      SECTOR              SCORE    ALARM    STARS", 13);
		headers.setFillColor(Theme::AccentYellow);
		headers.setPosition(sf::Vector2f(px, py));
		target.draw(headers);

		sf::VertexArray div(sf::PrimitiveType::Lines, 2);
		div[0].position = sf::Vector2f(px, py + 24.0f);
		div[0].color = Theme::BorderMuted;
		div[1].position = sf::Vector2f(contentPanel.bounds.position.x + contentPanel.bounds.size.x - 24.0f, py + 24.0f);
		div[1].color = Theme::BorderMuted;
		target.draw(div);

		// Empty state message
		sf::Text emptyMsg(engine.theme.fontUI, "No vault heist records logged in local system storage yet.\nInfiltrate and clear campaign sectors to record your heist breaches!", 16);
		emptyMsg.setFillColor(Theme::TextMuted);
		emptyMsg.setPosition(sf::Vector2f(px, py + 80.0f));
		target.draw(emptyMsg);

		sf::Text tip(engine.theme.fontMono, "TIP: Faster clears with 0 alarms unlock Master Agent tier ratings.", 12);
		tip.setFillColor(Theme::BorderCyan);
		tip.setPosition(sf::Vector2f(px, contentPanel.bounds.position.y + contentPanel.bounds.size.y - 36.0f));
		target.draw(tip);
	}
	else
	{
		// How To Play Sections
		struct Section {
			std::string heading;
			std::string body;
			sf::Color col;
		};

		Section sections[] = {
			{ "1. MISSION OBJECTIVE", "Breach the Living Vault by traversing and manipulating dynamic tree structures:\n- Act 1: Binary Trees  |  Act 2: BST Search & Partition\n- Act 3: AVL Self-Balancing  |  Act 4: Max-Heap Security Matrix", Theme::BorderCyan },
			{ "2. ALARM THREAT METER", "Every mistake adds structural stress: invalid clicks (+1 Alarm), wrong rotations (+2 Alarm).\nIf Alarm reaches MAX CAP, the vault locks down and your infiltration fails!", Theme::CrimsonAlarm },
			{ "3. OPTIMAL EFFICIENCY & STARS", "Solve chambers within the optimal step quota and low alarm to earn up to 3 stars per level.\nHints are available via CLICK_HINT, but overuse will cap your maximum star rating.", Theme::AccentYellow },
			{ "4. CONTROLS & SHORTCUTS", "- Left Mouse Button: Select node, traverse branch, or swap heap items\n- Case / Rotation Buttons: Apply AVL rebalance or BST deletion case\n- ESC Key: Step Back / Open Confirmation / Dismiss Alerts", Theme::SuccessEmerald }
		};

		float curY = py;
		for (int i = 0; i < 4; i++)
		{
			sf::Text head(engine.theme.fontUI, sections[i].heading, 15);
			head.setFillColor(sections[i].col);
			head.setPosition(sf::Vector2f(px, curY));
			target.draw(head);

			sf::Text body(engine.theme.fontUI, sections[i].body, 13);
			body.setFillColor(Theme::TextPrimary);
			body.setPosition(sf::Vector2f(px + 12.0f, curY + 22.0f));
			target.draw(body);

			curY += 78.0f;
		}
	}
}
