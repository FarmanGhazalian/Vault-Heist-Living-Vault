#include "gui/Theme.h"
#include <filesystem>
#include <cmath>

namespace fs = std::filesystem;

// Palette definitions
const sf::Color Theme::BgMidnight     = sf::Color(10, 14, 23);
const sf::Color Theme::GridLine       = sf::Color(19, 27, 46, 120);
const sf::Color Theme::PanelBg        = sf::Color(17, 24, 39, 230);
const sf::Color Theme::BorderCyan     = sf::Color(0, 240, 255);
const sf::Color Theme::BorderMuted    = sf::Color(55, 65, 81);
const sf::Color Theme::AccentYellow   = sf::Color(255, 214, 10);
const sf::Color Theme::SuccessEmerald = sf::Color(0, 255, 136);
const sf::Color Theme::CrimsonAlarm   = sf::Color(255, 42, 85);
const sf::Color Theme::TextPrimary    = sf::Color(243, 244, 246);
const sf::Color Theme::TextMuted      = sf::Color(156, 163, 175);
const sf::Color Theme::NodeDarkFill   = sf::Color(19, 27, 46);

Theme::Theme()
	: fontsLoaded(false), gridOffset(0.0f)
{
}

bool Theme::loadFonts()
{
	if (fontsLoaded)
	{
		return true;
	}

	// 1. Try local assets folder for sans-serif UI font
	const std::string sansCandidates[] = {
		"assets/fonts/Rajdhani-Bold.ttf",
		"assets/fonts/Inter-SemiBold.ttf",
		"assets/fonts/Rajdhani.ttf",
		"assets/fonts/Inter.ttf",
		"C:/Windows/Fonts/segoeuib.ttf",
		"C:/Windows/Fonts/segoeui.ttf",
		"C:/Windows/Fonts/arialbd.ttf",
		"C:/Windows/Fonts/arial.ttf"
	};

	bool sansFound = false;
	for (const auto& path : sansCandidates)
	{
		if (fs::exists(path) && fontUI.openFromFile(path))
		{
			sansFound = true;
			break;
		}
	}

	// 2. Try local assets folder for monospace font
	const std::string monoCandidates[] = {
		"assets/fonts/JetBrainsMono-Regular.ttf",
		"assets/fonts/RobotoMono-Regular.ttf",
		"assets/fonts/JetBrainsMono.ttf",
		"C:/Windows/Fonts/consola.ttf",
		"C:/Windows/Fonts/consolab.ttf",
		"C:/Windows/Fonts/cour.ttf"
	};

	bool monoFound = false;
	for (const auto& path : monoCandidates)
	{
		if (fs::exists(path) && fontMono.openFromFile(path))
		{
			monoFound = true;
			break;
		}
	}

	fontsLoaded = sansFound && monoFound;
	return fontsLoaded;
}

void Theme::update(float dt)
{
	// Slowly drift grid diagonally
	gridOffset += dt * 15.0f;
	if (gridOffset >= 40.0f)
	{
		gridOffset -= 40.0f;
	}
}

void Theme::drawGridBackground(sf::RenderTarget& target, const sf::Vector2f& windowSize) const
{
	// Base dark background
	sf::RectangleShape bg(windowSize);
	bg.setFillColor(BgMidnight);
	target.draw(bg);

	// Grid spacing
	const float spacing = 40.0f;

	// Vertical grid lines
	for (float x = gridOffset; x < windowSize.x; x += spacing)
	{
		sf::VertexArray line(sf::PrimitiveType::Lines, 2);
		line[0].position = sf::Vector2f(x, 0.0f);
		line[0].color = GridLine;
		line[1].position = sf::Vector2f(x, windowSize.y);
		line[1].color = GridLine;
		target.draw(line);
	}

	// Horizontal grid lines
	for (float y = gridOffset; y < windowSize.y; y += spacing)
	{
		sf::VertexArray line(sf::PrimitiveType::Lines, 2);
		line[0].position = sf::Vector2f(0.0f, y);
		line[0].color = GridLine;
		line[1].position = sf::Vector2f(windowSize.x, y);
		line[1].color = GridLine;
		target.draw(line);
	}
}

void Theme::drawPanel(sf::RenderTarget& target, const sf::FloatRect& bounds, const sf::Color& borderColor, float borderWidth) const
{
	// Panel body
	sf::RectangleShape rect(bounds.size);
	rect.setPosition(bounds.position);
	rect.setFillColor(PanelBg);
	rect.setOutlineColor(borderColor);
	rect.setOutlineThickness(borderWidth);
	target.draw(rect);
}

void Theme::drawGlowOutline(sf::RenderTarget& target, const sf::FloatRect& bounds, const sf::Color& glowColor, float intensity) const
{
	// Concentric semi-transparent outlines for neon glow effect
	for (int i = 1; i <= 3; i++)
	{
		float offset = static_cast<float>(i) * 1.5f;
		float alpha = (40.0f / static_cast<float>(i)) * intensity;
		if (alpha > 255.0f) alpha = 255.0f;

		sf::RectangleShape glow(sf::Vector2f(bounds.size.x + offset * 2.0f, bounds.size.y + offset * 2.0f));
		glow.setPosition(sf::Vector2f(bounds.position.x - offset, bounds.position.y - offset));
		glow.setFillColor(sf::Color::Transparent);
		glow.setOutlineColor(sf::Color(glowColor.r, glowColor.g, glowColor.b, static_cast<std::uint8_t>(alpha)));
		glow.setOutlineThickness(1.0f);
		target.draw(glow);
	}
}
