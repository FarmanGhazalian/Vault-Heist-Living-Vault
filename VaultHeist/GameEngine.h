#pragma once
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>
#include "SoundManager.h"
#include "UIKit.h"
#include "Screen.h"

class GameEngine {
public:
    sf::RenderWindow window;
    sf::Font font;
    SoundManager audio;
    ToastManager toasts;
    ModalPopup sharedPopup;

    std::unique_ptr<Screen> currentScreen;
    std::unique_ptr<Screen> nextScreen;
    float transitionTimer = 0.0f;
    float transitionDuration = 0.3f;
    bool inTransition = false;
    float runTime = 0.0f;

    GameEngine();
    void run();
    void changeScreen(std::unique_ptr<Screen> newScreen);
    void showComingSoon(const std::string& feature);

    static std::string resolveJsonPath(const std::string& filename);
};
