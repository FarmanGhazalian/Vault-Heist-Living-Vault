#include "gui/LoaderScreen.h"
#include "gui/MainMenuScreen.h"
#include "gui/PlayerNameScreen.h"
#include "gui/GameEngine.h"
#include "gui/UIKit.h"
#include <cmath>

LoaderScreen::LoaderScreen(GameEngine& gameEngine)
	: engine(gameEngine), windowSize(gameEngine.getWindowSize()),
	  elapsed(0.0f), totalDuration(2.6f), vaultAngle(0.0f), finished(false),
	  fullSubtitle("The Living Vault"), visibleSubtitle("")
{
	updateLayout(windowSize.x, windowSize.y);
}

void LoaderScreen::updateLayout(float w, float h)
{
	windowSize = sf::Vector2f(w, h);
}

void LoaderScreen::onResize(unsigned int width, unsigned int height)
{
	updateLayout(static_cast<float>(width), static_cast<float>(height));
}

void LoaderScreen::proceedToMenu()
{
	if (!finished)
	{
		finished = true;
		engine.changeScreen(std::make_unique<PlayerNameScreen>(engine));
	}
}

void LoaderScreen::handleEvent(const sf::Event& event)
{
	// Any key or click skips the loader
	if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::MouseButtonPressed>())
	{
		proceedToMenu();
	}
}

void LoaderScreen::update(float dt)
{
	elapsed += dt;
	vaultAngle += dt * 140.0f; // rotate vault bolts

	// Subtitle typewriter effect: reveals letters progressively
	float subProgress = (elapsed - 0.4f) / 1.2f;
	if (subProgress < 0.0f) subProgress = 0.0f;
	if (subProgress > 1.0f) subProgress = 1.0f;

	size_t charCount = static_cast<size_t>(subProgress * static_cast<float>(fullSubtitle.length()));
	if (charCount > fullSubtitle.length()) charCount = fullSubtitle.length();
	visibleSubtitle = fullSubtitle.substr(0, charCount);

	if (elapsed >= totalDuration)
	{
		proceedToMenu();
	}
}

void LoaderScreen::draw(sf::RenderTarget& target)
{
	float cx = windowSize.x * 0.5f;
	float cy = windowSize.y * 0.5f - 20.0f;

	float progress = elapsed / totalDuration;
	if (progress > 1.0f) progress = 1.0f;

	// Vault Door Opening Expansion when progress > 0.75
	float doorExpand = 0.0f;
	if (progress > 0.75f)
	{
		doorExpand = Tween::easeOutQuad((progress - 0.75f) / 0.25f) * 18.0f;
	}

	// 1. Vault Door Graphic
	float outerR = 60.0f + doorExpand;
	float innerR = 38.0f;

	// Outer ring
	sf::CircleShape outerRing(outerR);
	outerRing.setOrigin(sf::Vector2f(outerR, outerR));
	outerRing.setPosition(sf::Vector2f(cx, cy - 80.0f));
	outerRing.setFillColor(sf::Color(17, 24, 39, 200));
	outerRing.setOutlineColor(Theme::BorderCyan);
	outerRing.setOutlineThickness(2.0f);
	target.draw(outerRing);

	// Inner core
	sf::CircleShape innerRing(innerR);
	innerRing.setOrigin(sf::Vector2f(innerR, innerR));
	innerRing.setPosition(sf::Vector2f(cx, cy - 80.0f));
	innerRing.setFillColor(sf::Color(10, 14, 23, 230));
	innerRing.setOutlineColor(Theme::BorderMuted);
	innerRing.setOutlineThickness(1.5f);
	target.draw(innerRing);

	// Radial bolts on the rotating ring
	const int boltCount = 8;
	const double pi = 3.14159265358979323846;
	for (int i = 0; i < boltCount; i++)
	{
		float angleDeg = vaultAngle + (static_cast<float>(i) * 360.0f / static_cast<float>(boltCount));
		float angleRad = static_cast<float>(angleDeg * pi / 180.0);
		float bx = cx + std::cos(angleRad) * (outerR - 12.0f);
		float by = (cy - 80.0f) + std::sin(angleRad) * (outerR - 12.0f);

		sf::CircleShape bolt(4.0f);
		bolt.setOrigin(sf::Vector2f(4.0f, 4.0f));
		bolt.setPosition(sf::Vector2f(bx, by));
		bolt.setFillColor(Theme::AccentYellow);
		target.draw(bolt);
	}

	// Center lock iris (radiating triangle spokes)
	for (int i = 0; i < 4; i++)
	{
		float angleDeg = -vaultAngle * 0.5f + static_cast<float>(i) * 90.0f;
		float angleRad = static_cast<float>(angleDeg * pi / 180.0);
		sf::VertexArray spoke(sf::PrimitiveType::Lines, 2);
		spoke[0].position = sf::Vector2f(cx, cy - 80.0f);
		spoke[0].color = Theme::BorderCyan;
		spoke[1].position = sf::Vector2f(cx + std::cos(angleRad) * 26.0f, (cy - 80.0f) + std::sin(angleRad) * 26.0f);
		spoke[1].color = sf::Color(0, 240, 255, 40);
		target.draw(spoke);
	}

	// 2. Title "VAULT HEIST"
	sf::Text title(engine.theme.fontUI, "VAULT HEIST", 38);
	title.setFillColor(Theme::TextPrimary);
	sf::FloatRect tb = title.getLocalBounds();
	float titleX = cx - tb.size.x * 0.5f - tb.position.x;
	float titleY = cy + 10.0f;
	title.setPosition(sf::Vector2f(std::round(titleX), std::round(titleY)));
	target.draw(title);

	// Light scan across title
	float scanCycle = std::fmod(elapsed * 1.2f, 1.0f);
	float scanX = titleX + scanCycle * tb.size.x;
	sf::VertexArray scanLine(sf::PrimitiveType::Lines, 2);
	scanLine[0].position = sf::Vector2f(scanX, titleY - 4.0f);
	scanLine[0].color = sf::Color(0, 240, 255, 180);
	scanLine[1].position = sf::Vector2f(scanX, titleY + tb.size.y + 4.0f);
	scanLine[1].color = sf::Color(0, 240, 255, 180);
	target.draw(scanLine);

	// 3. Subtitle "The Living Vault" (typewriter)
	if (!visibleSubtitle.empty())
	{
		sf::Text sub(engine.theme.fontUI, visibleSubtitle, 17);
		sub.setFillColor(Theme::AccentYellow);
		sf::FloatRect sb = sub.getLocalBounds();
		float subX = cx - sb.size.x * 0.5f - sb.position.x;
		sub.setPosition(sf::Vector2f(std::round(subX), titleY + 50.0f));
		target.draw(sub);
	}

	// 4. Progress bar
	float barW = 320.0f;
	float barH = 5.0f;
	float barX = cx - barW * 0.5f;
	float barY = cy + 105.0f;

	// Background track
	sf::RectangleShape track(sf::Vector2f(barW, barH));
	track.setPosition(sf::Vector2f(barX, barY));
	track.setFillColor(sf::Color(17, 24, 39, 220));
	track.setOutlineColor(Theme::BorderMuted);
	track.setOutlineThickness(1.0f);
	target.draw(track);

	// Fill bar
	float fillW = barW * progress;
	if (fillW > 0.0f)
	{
		sf::RectangleShape fill(sf::Vector2f(fillW, barH));
		fill.setPosition(sf::Vector2f(barX, barY));
		fill.setFillColor(Theme::BorderCyan);
		target.draw(fill);
	}

	// Status text below progress bar
	int pct = static_cast<int>(progress * 100.0f);
	std::string statusStr = "Cracking the vault... " + std::to_string(pct) + "%";
	sf::Text status(engine.theme.fontMono, statusStr, 13);
	status.setFillColor(Theme::TextMuted);
	sf::FloatRect stB = status.getLocalBounds();
	status.setPosition(sf::Vector2f(std::round(cx - stB.size.x * 0.5f - stB.position.x), barY + 18.0f));
	target.draw(status);

	// Skip prompt hint
	sf::Text skipTxt(engine.theme.fontMono, "PRESS ANY KEY TO SKIP", 11);
	skipTxt.setFillColor(sf::Color(156, 163, 175, 120));
	sf::FloatRect skB = skipTxt.getLocalBounds();
	skipTxt.setPosition(sf::Vector2f(std::round(cx - skB.size.x * 0.5f - skB.position.x), windowSize.y - 36.0f));
	target.draw(skipTxt);
}
