#pragma once
#include "Screen.h"
#include "UIKit.h"
#include <vector>

class GameEngine;

class SettingsScreen : public Screen
{
private:
    GameEngine& engine;
    sf::Vector2f windowSize;

    Button btnBack;
    Button btnAudioToggle;

    Button btnSpeed05;
    Button btnSpeed10;
    Button btnSpeed15;
    Button btnSpeed20;

    Button btnResetProgress;

    Button btnBenchmark;
    Button btnTestSuite;

    bool m_benchmarkRunning = false;
    bool m_testsRunning = false;

    void updateLayout(float w, float h);
    void updateSpeedButtons();

public:
    SettingsScreen(GameEngine& gameEngine);

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void onResize(unsigned int width, unsigned int height) override;
};
