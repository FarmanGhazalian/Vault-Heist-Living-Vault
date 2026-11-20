#include "GameEngine.h"
#include "LoaderScreen.h"
#include "SettingsManager.h"
#include "LeaderboardManager.h"
#include <algorithm>

GameEngine::GameEngine()
	: isTransitioning(false), transitionProgress(0.0f)
{
}

bool GameEngine::init()
{
	theme.loadFonts();
	audio.init();
	SettingsManager::init(audio);
	LeaderboardManager::init();

	sf::VideoMode desktop = sf::VideoMode::getDesktopMode();
	unsigned int targetW = 1440;
	unsigned int targetH = 900;
	if (desktop.size.x > 80 && desktop.size.x < targetW + 60)
	{
		targetW = desktop.size.x - 60;
	}
	if (desktop.size.y > 80 && desktop.size.y < targetH + 80)
	{
		targetH = desktop.size.y - 80;
	}
	targetW = std::max(targetW, MinWindowWidth);
	targetH = std::max(targetH, MinWindowHeight);

	window.create(sf::VideoMode(sf::Vector2u(targetW, targetH)), "Vault Heist: The Living Vault", sf::Style::Default);
	window.setView(sf::View(sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(static_cast<float>(targetW), static_cast<float>(targetH)))));
	window.setFramerateLimit(60);

	changeScreen(std::make_unique<LoaderScreen>(*this));
	return true;
}

sf::Vector2f GameEngine::getWindowSize() const
{
	return sf::Vector2f(static_cast<float>(window.getSize().x), static_cast<float>(window.getSize().y));
}

void GameEngine::changeScreen(std::unique_ptr<Screen> newScreen)
{
	if (!currentScreen)
	{
		currentScreen = std::move(newScreen);
		currentScreen->onResize(window.getSize().x, window.getSize().y);
		isTransitioning = false;
	}
	else
	{
		nextScreen = std::move(newScreen);
		isTransitioning = true;
		transitionProgress = 0.0f;
	}
}

void GameEngine::showToast(const std::string& message, int type)
{
	toasts.addToast(message, type);
}

void GameEngine::showInfoModal(const std::string& title, const std::string& message, std::function<void()> onOk)
{
	modal.showInfo(title, message, onOk);
}

void GameEngine::showConfirmModal(const std::string& title, const std::string& message, std::function<void()> onYes, std::function<void()> onNo)
{
	modal.showConfirm(title, message, onYes, onNo);
}

void GameEngine::run()
{
	sf::Clock clock;

	while (window.isOpen())
	{
		float dt = clock.restart().asSeconds();
		if (dt > 0.1f) dt = 0.1f; // clamp delta spike

		while (const auto event = window.pollEvent())
		{
			if (event->is<sf::Event::Closed>())
			{
				window.close();
				break;
			}
			else if (const auto* resized = event->getIf<sf::Event::Resized>())
			{
				unsigned int w = std::max(resized->size.x, MinWindowWidth);
				unsigned int h = std::max(resized->size.y, MinWindowHeight);

				// Update view coordinates to match pixel dimensions
				window.setView(sf::View(sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(static_cast<float>(w), static_cast<float>(h)))));

				if (currentScreen)
				{
					currentScreen->onResize(w, h);
				}
				if (nextScreen)
				{
					nextScreen->onResize(w, h);
				}
			}

			if (modal.active)
			{
				modal.handleEvent(*event);
			}
			else
			{
				toasts.handleEvent(*event, static_cast<float>(window.getSize().x));
				if (!isTransitioning && currentScreen)
				{
					currentScreen->handleEvent(*event);
				}
			}
		}

		// Update
		theme.update(dt);
		toasts.update(dt);
		if (modal.active)
		{
			modal.update(dt);
		}

		if (isTransitioning)
		{
			transitionProgress += dt / TransitionDuration;
			if (transitionProgress >= 1.0f)
			{
				currentScreen = std::move(nextScreen);
				if (currentScreen)
				{
					currentScreen->onResize(window.getSize().x, window.getSize().y);
				}
				isTransitioning = false;
				transitionProgress = 0.0f;
			}
		}
		else if (currentScreen)
		{
			currentScreen->update(dt);
		}

		// Draw
		window.clear(Theme::BgMidnight);

		// Tactical drifting grid background
		theme.drawGridBackground(window, getWindowSize());

		if (currentScreen)
		{
			currentScreen->draw(window);
		}

		// Toasts on top
		toasts.draw(window, theme, static_cast<float>(window.getSize().x));

		// Modals on top
		if (modal.active)
		{
			modal.draw(window, theme, getWindowSize());
		}

		// 0.3s Fade overlay transition
		if (isTransitioning)
		{
			float alpha = 0.0f;
			if (transitionProgress < 0.5f)
			{
				// Fade to black
				alpha = (transitionProgress / 0.5f) * 255.0f;
			}
			else
			{
				// Fade in from black
				alpha = (1.0f - (transitionProgress - 0.5f) / 0.5f) * 255.0f;
			}
			if (alpha > 255.0f) alpha = 255.0f;
			if (alpha < 0.0f) alpha = 0.0f;

			sf::RectangleShape fadeRect(getWindowSize());
			fadeRect.setFillColor(sf::Color(10, 14, 23, static_cast<std::uint8_t>(alpha)));
			window.draw(fadeRect);
		}

		window.display();
	}
}
