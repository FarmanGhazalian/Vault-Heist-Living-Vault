#pragma once
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>

class Screen
{
public:
	virtual ~Screen() = default;

	virtual void handleEvent(const sf::Event& event) = 0;
	virtual void update(float dt) = 0;
	virtual void draw(sf::RenderTarget& target) = 0;
	virtual void onResize(unsigned int width, unsigned int height) {}
};
