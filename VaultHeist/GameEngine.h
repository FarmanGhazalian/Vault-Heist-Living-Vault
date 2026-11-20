#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include <functional>
#include "Theme.h"
#include "AudioManager.h"
#include "UIKit.h"
#include "Screen.h"

class GameEngine
{
public:
	sf::RenderWindow window;
	Theme theme;
	AudioManager audio;
	ToastManager toasts;
	ModalPopup modal;

	static constexpr unsigned int MinWindowWidth  = 960;
	static constexpr unsigned int MinWindowHeight = 600;

	static inline std::string s_playerName = "Player";
	static std::string getPlayerName() { return s_playerName.empty() ? "Player" : s_playerName; }

	std::unique_ptr<Screen> currentScreen;
	std::unique_ptr<Screen> nextScreen;

	// Transition state (0.3s fade)
	bool isTransitioning;
	float transitionProgress; // 0.0 to 1.0
	static constexpr float TransitionDuration = 0.3f;

	GameEngine();
	~GameEngine() = default;

	bool init();
	void run();

	void changeScreen(std::unique_ptr<Screen> newScreen);
	sf::Vector2f getWindowSize() const;

	void showToast(const std::string& message, int type = TOAST_INFO);
	void showInfoModal(const std::string& title, const std::string& message, std::function<void()> onOk = nullptr);
	void showConfirmModal(const std::string& title, const std::string& message, std::function<void()> onYes = nullptr, std::function<void()> onNo = nullptr);
};
