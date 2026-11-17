#include "MainMenuScreen.h"
#include "CampaignScreen.h"
#include "InfoScreen.h"
#include "GameEngine.h"
#include <cmath>
#include <algorithm>

MainMenuScreen::MainMenuScreen(GameEngine& gameEngine)
	: engine(gameEngine), windowSize(gameEngine.getWindowSize()), animTimer(0.0f)
{
	// Initialize faint ambient nodes for background
	ambientNodes.push_back({ sf::Vector2f(120.0f, 150.0f), sf::Vector2f(8.0f, 6.0f), 28.0f });
	ambientNodes.push_back({ sf::Vector2f(280.0f, 320.0f), sf::Vector2f(-6.0f, 7.0f), 24.0f });
	ambientNodes.push_back({ sf::Vector2f(1000.0f, 180.0f), sf::Vector2f(-7.0f, -5.0f), 30.0f });
	ambientNodes.push_back({ sf::Vector2f(1150.0f, 420.0f), sf::Vector2f(5.0f, -8.0f), 26.0f });
	ambientNodes.push_back({ sf::Vector2f(200.0f, 560.0f), sf::Vector2f(9.0f, -6.0f), 22.0f });
	ambientNodes.push_back({ sf::Vector2f(920.0f, 580.0f), sf::Vector2f(-8.0f, 6.0f), 26.0f });

	updateLayout(windowSize.x, windowSize.y);
}

void MainMenuScreen::updateLayout(float w, float h)
{
	windowSize = sf::Vector2f(w, h);

	// Button dimensions
	float bw = std::min(320.0f, w * 0.40f);
	float bh = 42.0f;
	float gap = 12.0f;
	float startX = (w - bw) * 0.5f;

	// Center column vertically below the title
	float totalButtonsHeight = 7.0f * bh + 6.0f * gap;
	float startY = (h - totalButtonsHeight) * 0.5f + 40.0f;

	btnCampaign    = Button("CAMPAIGN",     sf::FloatRect(sf::Vector2f(startX, startY + 0.0f * (bh + gap)), sf::Vector2f(bw, bh)), BTN_PRIMARY);
	btnEndless     = Button("ENDLESS",      sf::FloatRect(sf::Vector2f(startX, startY + 1.0f * (bh + gap)), sf::Vector2f(bw, bh)), BTN_SECONDARY);
	btnSandbox     = Button("SANDBOX",      sf::FloatRect(sf::Vector2f(startX, startY + 2.0f * (bh + gap)), sf::Vector2f(bw, bh)), BTN_SECONDARY);
	btnLeaderboard = Button("LEADERBOARD",  sf::FloatRect(sf::Vector2f(startX, startY + 3.0f * (bh + gap)), sf::Vector2f(bw, bh)), BTN_SECONDARY);
	btnHowToPlay   = Button("HOW TO PLAY",  sf::FloatRect(sf::Vector2f(startX, startY + 4.0f * (bh + gap)), sf::Vector2f(bw, bh)), BTN_SECONDARY);
	btnSettings    = Button("SETTINGS",     sf::FloatRect(sf::Vector2f(startX, startY + 5.0f * (bh + gap)), sf::Vector2f(bw, bh)), BTN_SECONDARY);
	btnQuit        = Button("QUIT HEIST",   sf::FloatRect(sf::Vector2f(startX, startY + 6.0f * (bh + gap)), sf::Vector2f(bw, bh)), BTN_DANGER);

	// Mute toggle button at top right
	btnMute = Button(engine.audio.isMuted() ? "[AUDIO: MUTED]" : "[AUDIO: ON]",
	                 sf::FloatRect(sf::Vector2f(w - 150.0f, 20.0f), sf::Vector2f(130.0f, 32.0f)), BTN_BACK);
}

void MainMenuScreen::onResize(unsigned int width, unsigned int height)
{
	updateLayout(static_cast<float>(width), static_cast<float>(height));
}

void MainMenuScreen::handleEvent(const sf::Event& event)
{
	if (event.is<sf::Event::KeyPressed>())
	{
		if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
		{
			if (kp->code == sf::Keyboard::Key::Escape)
			{
				engine.audio.playClick();
				engine.showConfirmModal("CONFIRM ABORT MISSION", "Are you sure you want to abort the heist and exit?",
					[this]() { engine.window.close(); },
					nullptr);
				return;
			}
		}
	}

	btnCampaign.handleEvent(event);
	btnEndless.handleEvent(event);
	btnSandbox.handleEvent(event);
	btnLeaderboard.handleEvent(event);
	btnHowToPlay.handleEvent(event);
	btnSettings.handleEvent(event);
	btnQuit.handleEvent(event);
	btnMute.handleEvent(event);

	if (btnMute.isClicked())
	{
		engine.audio.toggleMute();
		btnMute.label = engine.audio.isMuted() ? "[AUDIO: MUTED]" : "[AUDIO: ON]";
		btnMute.resetClick();
	}

	if (btnCampaign.isClicked())
	{
		engine.audio.playClick();
		btnCampaign.resetClick();
		engine.changeScreen(std::make_unique<CampaignScreen>(engine));
	}

	if (btnEndless.isClicked())
	{
		engine.audio.playClick();
		btnEndless.resetClick();
		engine.showInfoModal("ENDLESS MODE // CLASSIFIED", "Infinite procedural security vaults will unlock in Phase 3.\nTest your endurance and survival under constant alarm pressure!");
	}

	if (btnSandbox.isClicked())
	{
		engine.audio.playClick();
		btnSandbox.resetClick();
		engine.showInfoModal("SANDBOX MODE // LAB ACCESS", "Interactive Tree Construction Sandbox will unlock in Phase 3.\nExperiment freely with Binary Trees, BSTs, AVL, and Heaps.");
	}

	if (btnLeaderboard.isClicked())
	{
		engine.audio.playClick();
		btnLeaderboard.resetClick();
		engine.changeScreen(std::make_unique<InfoScreen>(engine, MODE_LEADERBOARD));
	}

	if (btnHowToPlay.isClicked())
	{
		engine.audio.playClick();
		btnHowToPlay.resetClick();
		engine.changeScreen(std::make_unique<InfoScreen>(engine, MODE_HOW_TO_PLAY));
	}

	if (btnSettings.isClicked())
	{
		engine.audio.playClick();
		btnSettings.resetClick();
		engine.showInfoModal("SETTINGS & AUDIO", "Audio mute can be toggled using the top-right button.\nDisplay, resolution, and keybind preferences coming in Phase 3.");
	}

	if (btnQuit.isClicked())
	{
		engine.audio.playClick();
		btnQuit.resetClick();
		engine.showConfirmModal("CONFIRM ABORT MISSION", "Are you sure you want to abort the heist and exit?",
			[this]() { engine.window.close(); },
			nullptr);
	}
}

void MainMenuScreen::update(float dt)
{
	animTimer += dt;

	// Drift ambient background nodes
	for (auto& node : ambientNodes)
	{
		node.pos += node.vel * dt;
		if (node.pos.x < 40.0f || node.pos.x > windowSize.x - 40.0f) node.vel.x = -node.vel.x;
		if (node.pos.y < 40.0f || node.pos.y > windowSize.y - 40.0f) node.vel.y = -node.vel.y;
	}
}

void MainMenuScreen::draw(sf::RenderTarget& target)
{
	// 1. Draw faint ambient node silhouettes and connecting lines
	for (size_t i = 0; i < ambientNodes.size(); i++)
	{
		for (size_t j = i + 1; j < ambientNodes.size(); j++)
		{
			float dx = ambientNodes[i].pos.x - ambientNodes[j].pos.x;
			float dy = ambientNodes[i].pos.y - ambientNodes[j].pos.y;
			float dist = std::sqrt(dx * dx + dy * dy);
			if (dist < 320.0f)
			{
				float alpha = (1.0f - dist / 320.0f) * 35.0f;
				sf::VertexArray edge(sf::PrimitiveType::Lines, 2);
				edge[0].position = ambientNodes[i].pos;
				edge[0].color = sf::Color(0, 240, 255, static_cast<std::uint8_t>(alpha));
				edge[1].position = ambientNodes[j].pos;
				edge[1].color = sf::Color(0, 240, 255, static_cast<std::uint8_t>(alpha));
				target.draw(edge);
			}
		}

		sf::CircleShape circle(ambientNodes[i].radius);
		circle.setOrigin(sf::Vector2f(ambientNodes[i].radius, ambientNodes[i].radius));
		circle.setPosition(ambientNodes[i].pos);
		circle.setFillColor(sf::Color(17, 24, 39, 45));
		circle.setOutlineColor(sf::Color(0, 240, 255, 30));
		circle.setOutlineThickness(1.5f);
		target.draw(circle);
	}

	float cx = windowSize.x * 0.5f;

	// 2. Stylized Title & Subtitle
	sf::Text title(engine.theme.fontUI, "VAULT HEIST", 44);
	title.setFillColor(Theme::TextPrimary);
	sf::FloatRect tb = title.getLocalBounds();
	float titleX = cx - tb.size.x * 0.5f - tb.position.x;
	float titleY = std::max(20.0f, windowSize.y * 0.08f);
	title.setPosition(sf::Vector2f(std::round(titleX), std::round(titleY)));

	// Subtle glowing outline for title
	engine.theme.drawGlowOutline(target, sf::FloatRect(sf::Vector2f(titleX - 10.0f, titleY - 4.0f), sf::Vector2f(tb.size.x + 20.0f, tb.size.y + 12.0f)), Theme::BorderCyan, 0.45f);
	target.draw(title);

	// Subtitle
	sf::Text sub(engine.theme.fontUI, "THE LIVING VAULT // SYSTEM CORE", 15);
	sub.setFillColor(Theme::AccentYellow);
	sf::FloatRect sb = sub.getLocalBounds();
	sub.setPosition(sf::Vector2f(std::round(cx - sb.size.x * 0.5f - sb.position.x), titleY + 54.0f));
	target.draw(sub);

	// 3. Status label: "Save: none yet"
	sf::Text saveStatus(engine.theme.fontMono, "SAVE: NONE YET (MEMORY)", 12);
	saveStatus.setFillColor(Theme::TextMuted);
	saveStatus.setPosition(sf::Vector2f(24.0f, windowSize.y - 32.0f));
	target.draw(saveStatus);

	// Version / Info label
	sf::Text ver(engine.theme.fontMono, "CYBER-VAULT v2.0 // FAST & LEAN", 12);
	ver.setFillColor(Theme::TextMuted);
	sf::FloatRect vb = ver.getLocalBounds();
	ver.setPosition(sf::Vector2f(windowSize.x - vb.size.x - 24.0f, windowSize.y - 32.0f));
	target.draw(ver);

	// 4. Staggered slide-in animation for 7 buttons
	Button* buttons[] = { &btnCampaign, &btnEndless, &btnSandbox, &btnLeaderboard, &btnHowToPlay, &btnSettings, &btnQuit };
	for (int i = 0; i < 7; i++)
	{
		float delay = static_cast<float>(i) * 0.05f;
		float progress = (animTimer - delay) / 0.35f;
		if (progress < 0.0f) progress = 0.0f;
		if (progress > 1.0f) progress = 1.0f;

		float slideFactor = Tween::easeOutQuad(progress);
		float originalX = (windowSize.x - buttons[i]->bounds.size.x) * 0.5f;

		// Offset from -50px to 0
		buttons[i]->bounds.position.x = originalX - (1.0f - slideFactor) * 50.0f;
		buttons[i]->draw(target, engine.theme);
	}

	// Mute button
	btnMute.draw(target, engine.theme);
}
