#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class Theme
{
public:
	// Palette Colors
	static const sf::Color BgMidnight;
	static const sf::Color GridLine;
	static const sf::Color PanelBg;
	static const sf::Color BorderCyan;
	static const sf::Color BorderMuted;
	static const sf::Color AccentYellow;
	static const sf::Color SuccessEmerald;
	static const sf::Color CrimsonAlarm;
	static const sf::Color TextPrimary;
	static const sf::Color TextMuted;
	static const sf::Color NodeDarkFill;

	// Node defaults for future phases
	static constexpr float NodeRadius = 30.0f;

	// Shared Fonts
	sf::Font fontUI;
	sf::Font fontMono;

	bool fontsLoaded;
	float gridOffset;

	Theme();

	bool loadFonts();

	// Helpers
	void update(float dt);
	void drawGridBackground(sf::RenderTarget& target, const sf::Vector2f& windowSize) const;
	void drawPanel(sf::RenderTarget& target, const sf::FloatRect& bounds, const sf::Color& borderColor, float borderWidth = 1.0f) const;
	void drawGlowOutline(sf::RenderTarget& target, const sf::FloatRect& bounds, const sf::Color& glowColor, float intensity = 1.0f) const;
};
