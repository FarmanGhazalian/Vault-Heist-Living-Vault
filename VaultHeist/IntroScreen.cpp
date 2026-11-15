#include "IntroScreen.h"
#include "MainMenuScreen.h"
#include "GameEngine.h"
#include <cmath>
#include <algorithm>

void IntroScreen::handleEvent(GameEngine& engine, const sf::Event& event) {
    if (event.is<sf::Event::KeyPressed>() || event.is<sf::Event::MouseButtonPressed>()) {
        if (!skipped) {
            skipped = true;
            engine.audio.playClick();
            engine.changeScreen(std::make_unique<MainMenuScreen>());
        }
    }
}

void IntroScreen::update(GameEngine& engine, float dt) {
    timer += dt;
    if (timer >= totalDuration && !skipped) {
        skipped = true;
        engine.changeScreen(std::make_unique<MainMenuScreen>());
    }
}

void IntroScreen::draw(GameEngine& engine, sf::RenderWindow& window) {
    float W = static_cast<float>(window.getSize().x);
    float H = static_cast<float>(window.getSize().y);

    window.clear(Theme::BgDark);
    drawBackgroundGrid(window, W, H, timer);

    float centerX = W * 0.5f;
    float doorY = 310.0f;

    // 1. Animated Rotating Vault Door
    float rotAngle = timer * 45.0f; // degrees
    float outerR = 90.0f;

    // Vault Outer Rim
    sf::CircleShape outer(outerR);
    outer.setOrigin({outerR, outerR});
    outer.setPosition({centerX, doorY});
    outer.setFillColor(sf::Color(14, 20, 36));
    outer.setOutlineThickness(2.0f);
    outer.setOutlineColor(Theme::PanelBorder);
    window.draw(outer);

    // Inner bevel ring
    float innerR = 70.0f;
    sf::CircleShape inner(innerR);
    inner.setOrigin({innerR, innerR});
    inner.setPosition({centerX, doorY});
    inner.setFillColor(sf::Color(18, 26, 46));
    inner.setOutlineThickness(1.5f);
    inner.setOutlineColor(Theme::AccentCyan);
    window.draw(inner);

    // Rotating Vault Spokes & Bolts
    for (int i = 0; i < 8; ++i) {
        float a = (i * 45.0f + rotAngle) * 3.14159265f / 180.0f;
        float bx = centerX + std::cos(a) * 80.0f;
        float by = doorY + std::sin(a) * 80.0f;

        // Bolt
        sf::CircleShape bolt(4.5f);
        bolt.setOrigin({4.5f, 4.5f});
        bolt.setPosition({bx, by});
        bolt.setFillColor(Theme::Gold);
        bolt.setOutlineThickness(1.0f);
        bolt.setOutlineColor(Theme::PanelBorder);
        window.draw(bolt);

        // Radial lock spoke
        sf::RectangleShape spoke({35.0f, 2.0f});
        spoke.setOrigin({0.0f, 1.0f});
        spoke.setPosition({centerX, doorY});
        spoke.setRotation(sf::degrees(i * 45.0f + rotAngle));
        spoke.setFillColor(sf::Color(56, 189, 248, 120));
        window.draw(spoke);
    }

    // Center lock hub
    float hubR = 24.0f;
    sf::CircleShape hub(hubR);
    hub.setOrigin({hubR, hubR});
    hub.setPosition({centerX, doorY});
    hub.setFillColor(sf::Color(26, 38, 66));
    hub.setOutlineThickness(1.5f);
    hub.setOutlineColor(Theme::Gold);
    window.draw(hub);

    // 2. Light scan sweep across Title
    sf::Text title(engine.font, "VAULT HEIST", 64);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(Theme::Gold);
    auto tb = title.getLocalBounds();
    title.setPosition({centerX - tb.size.x * 0.5f, 435.0f});
    window.draw(title);

    // Laser scanline sweeping across title
    float scanProg = std::fmod(timer * 1.5f, 1.0f);
    float scanX = (centerX - tb.size.x * 0.5f - 40.0f) + (tb.size.x + 80.0f) * scanProg;
    sf::RectangleShape scan({3.0f, 75.0f});
    scan.setPosition({scanX, 435.0f});
    scan.setFillColor(sf::Color(56, 189, 248, 220));
    window.draw(scan);

    // 3. Subtitle fading in letter by letter
    std::string fullSub = "The Living Vault";
    float letterProgress = std::clamp((timer - 0.5f) / 1.2f, 0.0f, 1.0f);
    size_t lettersCount = static_cast<size_t>(letterProgress * fullSub.size());
    std::string currentSub = fullSub.substr(0, lettersCount);

    sf::Text sub(engine.font, currentSub, 20);
    sub.setStyle(sf::Text::Bold);
    sub.setFillColor(Theme::TextWhite);
    auto sb = sub.getLocalBounds();
    sub.setPosition({centerX - (tb.size.x * 0.35f), 520.0f});
    window.draw(sub);

    // 4. Progress bar with "Cracking the vault..."
    float barW = 320.0f;
    float barH = 4.0f;
    float barY = 600.0f;
    sf::RectangleShape pBg({barW, barH});
    pBg.setPosition({centerX - barW * 0.5f, barY});
    pBg.setFillColor(sf::Color(22, 32, 54));
    window.draw(pBg);

    float prog = std::clamp(timer / (totalDuration - 0.4f), 0.0f, 1.0f);
    sf::RectangleShape pFill({barW * prog, barH});
    pFill.setPosition({centerX - barW * 0.5f, barY});
    pFill.setFillColor(Theme::AccentCyan);
    window.draw(pFill);

    sf::Text status(engine.font, "Cracking security protocols...", 12);
    status.setFillColor(Theme::TextMuted);
    auto stB = status.getLocalBounds();
    status.setPosition({centerX - stB.size.x * 0.5f, barY + 16.0f});
    window.draw(status);

    // Skip prompt in bottom corner
    sf::Text skip(engine.font, "[Press any key or click to skip]", 11);
    skip.setFillColor(sf::Color(100, 116, 139));
    auto skB = skip.getLocalBounds();
    skip.setPosition({centerX - skB.size.x * 0.5f, H - 45.0f});
    window.draw(skip);
}
