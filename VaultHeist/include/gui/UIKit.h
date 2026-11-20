#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>
#include "gui/Theme.h"

// Shared text utilities
namespace UIText
{
	// Word-wrap a string into lines that each fit within maxWidth pixels.
	// Uses the given font and characterSize for measurement.
	std::vector<std::string> wrapText(const sf::Font& font, const std::string& text,
	                                  unsigned int charSize, float maxWidth);

	// Draw multiple wrapped lines into a bounding box, returning total height used.
	float drawWrapped(sf::RenderTarget& target, const sf::Font& font,
	                  const std::string& text, unsigned int charSize,
	                  sf::Color color, sf::Vector2f topLeft, float maxWidth);
}

// Button Style constants (no enum)
constexpr int BTN_PRIMARY   = 0;
constexpr int BTN_SECONDARY = 1;
constexpr int BTN_DANGER    = 2;
constexpr int BTN_BACK      = 3;

// Toast Types
constexpr int TOAST_HINT    = 0;
constexpr int TOAST_CORRECT = 1;
constexpr int TOAST_ERROR   = 2;
constexpr int TOAST_INFO    = 3;

// Modal Types
constexpr int MODAL_INFO    = 0;
constexpr int MODAL_CONFIRM = 1;

class Button
{
public:
	sf::FloatRect bounds;
	std::string label;
	int style;
	bool hovered;
	bool pressed;
	bool clicked;
	bool enabled;

	Button();
	Button(const std::string& text, const sf::FloatRect& rect, int btnStyle = BTN_PRIMARY);

	void handleEvent(const sf::Event& event);
	void draw(sf::RenderTarget& target, const Theme& theme) const;
	bool isClicked() const { return clicked; }
	void resetClick() { clicked = false; }
};

class Panel
{
public:
	sf::FloatRect bounds;
	std::string title;
	sf::Color borderColor;
	float borderWidth;

	Panel();
	Panel(const sf::FloatRect& rect, const std::string& panelTitle = "", const sf::Color& bColor = Theme::BorderCyan, float bWidth = 1.0f);

	void draw(sf::RenderTarget& target, const Theme& theme) const;
};

struct Toast
{
	std::string message;
	int type;
	float timeRemaining;
	float totalTime;
	float animProgress; // 0 (hidden) to 1 (visible)
};

class ToastManager
{
public:
	std::vector<Toast> toasts;
	static constexpr float ToastWidth = 380.0f;
	static constexpr float ToastHeight = 46.0f;
	static constexpr float ToastDuration = 3.5f;

	ToastManager();

	void addToast(const std::string& message, int type = TOAST_INFO);
	void update(float dt);
	void handleEvent(const sf::Event& event, float windowWidth);
	void draw(sf::RenderTarget& target, const Theme& theme, float windowWidth) const;
};

class ModalPopup
{
public:
	bool active;
	int type;
	std::string title;
	std::string message;

	Button btnConfirm;
	Button btnCancel;

	std::function<void()> onConfirm;
	std::function<void()> onCancel;

	ModalPopup();

	void showInfo(const std::string& modalTitle, const std::string& modalMsg, std::function<void()> onOk = nullptr);
	void showConfirm(const std::string& modalTitle, const std::string& modalMsg, std::function<void()> onYes = nullptr, std::function<void()> onNo = nullptr);
	void close();

	void handleEvent(const sf::Event& event);
	void update(float dt);
	void updateLayout(const sf::Vector2f& windowSize);
	void draw(sf::RenderTarget& target, const Theme& theme, const sf::Vector2f& windowSize);
};

// Level Completion / Failure Summary Modal with Stars and 3 Buttons
class LevelResultModal
{
public:
	bool active;
	bool won;
	std::string title;
	std::string message;
	std::string leaderboardText;
	int stars;
	int steps;
	int optimalSteps;
	int penalties;
	int finalScore;

	Button btnNext;
	Button btnRetry;
	Button btnReplay;
	Button btnBack;

	std::function<void()> onNext;
	std::function<void()> onRetry;
	std::function<void()> onReplay;
	std::function<void()> onBack;

	LevelResultModal();

	void show(bool victory, int starCount, int stepCount, int optSteps, int penaltyPoints, int totalScore,
	          std::function<void()> nextCallback, std::function<void()> retryCallback, std::function<void()> backCallback,
	          std::function<void()> replayCallback = nullptr, const std::string& lbMsg = "");
	void close();

	void handleEvent(const sf::Event& event);
	void update(float dt);
	void updateLayout(const sf::Vector2f& windowSize);
	void draw(sf::RenderTarget& target, const Theme& theme, const sf::Vector2f& windowSize);
};

// Reusable Tween & FX Helpers
namespace Tween
{
	float lerp(float a, float b, float t);
	float easeOutQuad(float t);
	float easeInOutQuad(float t);
	float pulse(float time, float speed = 3.0f);
	sf::Vector2f shake(float time, float intensity = 5.0f);
}
