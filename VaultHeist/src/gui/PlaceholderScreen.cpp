#include "gui/PlaceholderScreen.h"
#include "gui/GameEngine.h"

PlaceholderScreen::PlaceholderScreen(GameEngine& gameEngine)
	: engine(gameEngine), windowSize(gameEngine.getWindowSize())
{
	updateLayout(windowSize.x, windowSize.y);
}

void PlaceholderScreen::updateLayout(float w, float h)
{
	windowSize = sf::Vector2f(w, h);

	// Centered panel: 50% width (min 420px), 300px height
	float pw = std::max(420.0f, w * 0.5f);
	float ph = 260.0f;
	float px = (w - pw) * 0.5f;
	float py = (h - ph) * 0.5f;
	centerPanel = Panel(sf::FloatRect(sf::Vector2f(px, py), sf::Vector2f(pw, ph)), "SYSTEM FRAMEWORK // STAGE 1 READY");

	// Button inside panel
	float bw = 240.0f;
	float bh = 44.0f;
	float bx = px + (pw - bw) * 0.5f;
	float by = py + ph - 70.0f;
	btnReady = Button("INITIALIZE VAULT", sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(bw, bh)), BTN_PRIMARY);

	// Back button at top-left anchor: 24px margin
	btnBack = Button("< BACK", sf::FloatRect(sf::Vector2f(24.0f, 24.0f), sf::Vector2f(100.0f, 36.0f)), BTN_BACK);
}

void PlaceholderScreen::onResize(unsigned int width, unsigned int height)
{
	updateLayout(static_cast<float>(width), static_cast<float>(height));
}

void PlaceholderScreen::handleEvent(const sf::Event& event)
{
	btnReady.handleEvent(event);
	btnBack.handleEvent(event);

	if (btnReady.isClicked())
	{
		engine.audio.playCorrect();
		btnReady.resetClick();
	}

	if (btnBack.isClicked())
	{
		engine.audio.playClick();
		btnBack.resetClick();
	}
}

void PlaceholderScreen::update(float dt)
{
	(void)dt;
}

void PlaceholderScreen::draw(sf::RenderTarget& target)
{
	// Panel
	centerPanel.draw(target, engine.theme);

	// Information text inside panel
	sf::Text desc(engine.theme.fontUI, "Cyber-Vault Neon Framework Active.\nWindow resizing, Theme palette, and Audio initialized.\nBackend intact.", 16);
	desc.setFillColor(Theme::TextMuted);
	desc.setPosition(sf::Vector2f(centerPanel.bounds.position.x + 24.0f, centerPanel.bounds.position.y + 60.0f));
	target.draw(desc);

	// Buttons
	btnReady.draw(target, engine.theme);
	btnBack.draw(target, engine.theme);
}
