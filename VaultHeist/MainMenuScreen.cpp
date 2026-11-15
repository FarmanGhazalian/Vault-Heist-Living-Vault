#include "MainMenuScreen.h"
#include "SelectScreen.h"
#include "GameEngine.h"
#include <cmath>

MainMenuScreen::MainMenuScreen() {
    float W = 1500.0f;
    float btnW = 320.0f;
    float btnH = 50.0f;
    float startX = (W - btnW) * 0.5f;
    float startY = 360.0f;
    float gap = 58.0f;

    btnCampaign = Button({{startX, startY}, {btnW, btnH}}, "CAMPAIGN // HEIST WINGS", ButtonStyle::GOLD);
    btnEndless = Button({{startX, startY + gap}, {btnW, btnH}}, "ENDLESS VAULT", ButtonStyle::SECONDARY);
    btnSandbox = Button({{startX, startY + gap * 2.0f}, {btnW, btnH}}, "SANDBOX SIMULATOR", ButtonStyle::SECONDARY);
    btnLeaderboard = Button({{startX, startY + gap * 3.0f}, {btnW, btnH}}, "OPERATOR LEADERBOARD", ButtonStyle::SECONDARY);
    btnSettings = Button({{startX, startY + gap * 4.0f}, {btnW, btnH}}, "SYSTEM SETTINGS", ButtonStyle::SECONDARY);
    btnQuit = Button({{startX, startY + gap * 5.0f}, {btnW, btnH}}, "QUIT TERMINAL", ButtonStyle::DANGER);

    btnSound = Button({{W - 180.0f, 30.0f}, {120.0f, 40.0f}}, "SOUND", ButtonStyle::SECONDARY);
}

void MainMenuScreen::handleEvent(GameEngine& engine, const sf::Event& event) {
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};

    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mp->button == sf::Mouse::Button::Left) {
            if (btnSound.handleClick(mouse)) {
                engine.audio.toggleMute();
                return;
            }
            if (btnCampaign.handleClick(mouse)) {
                engine.audio.playClick();
                engine.changeScreen(std::make_unique<SelectScreen>(SelectMode::ACTS));
                return;
            }
            if (btnEndless.handleClick(mouse)) {
                engine.audio.playClick();
                engine.showComingSoon("Endless Vault Protocol");
                return;
            }
            if (btnSandbox.handleClick(mouse)) {
                engine.audio.playClick();
                engine.showComingSoon("Sandbox Tree Lab");
                return;
            }
            if (btnLeaderboard.handleClick(mouse)) {
                engine.audio.playClick();
                engine.showComingSoon("Global Infiltration Leaderboard");
                return;
            }
            if (btnSettings.handleClick(mouse)) {
                engine.audio.playClick();
                engine.showComingSoon("Terminal Audio/Visual Preferences");
                return;
            }
            if (btnQuit.handleClick(mouse)) {
                engine.audio.playClick();
                engine.window.close();
                return;
            }
        }
    }
}

void MainMenuScreen::update(GameEngine& engine, float dt) {
    time += dt;
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};

    btnCampaign.update(mouse);
    btnEndless.update(mouse);
    btnSandbox.update(mouse);
    btnLeaderboard.update(mouse);
    btnSettings.update(mouse);
    btnQuit.update(mouse);

    btnSound.update(mouse);
    btnSound.text = engine.audio.isMuted() ? "SOUND OFF" : "SOUND ON";
}

void MainMenuScreen::draw(GameEngine& engine, sf::RenderWindow& window) {
    float W = static_cast<float>(window.getSize().x);
    float H = static_cast<float>(window.getSize().y);

    window.clear(Theme::BgDark);
    drawBackgroundGrid(window, W, H, time);

    // Faint decorative tree silhouettes in background
    auto drawGhostNode = [&](sf::Vector2f pos, float rad) {
        sf::CircleShape c(rad);
        c.setOrigin({rad, rad});
        c.setPosition(pos);
        c.setFillColor(sf::Color(18, 26, 46, 70));
        c.setOutlineThickness(1.0f);
        c.setOutlineColor(sf::Color(38, 50, 77, 80));
        window.draw(c);
    };
    drawGhostNode({240.0f, 400.0f}, 45.0f);
    drawGhostNode({160.0f, 550.0f}, 35.0f);
    drawGhostNode({320.0f, 550.0f}, 35.0f);
    drawGhostNode({1260.0f, 400.0f}, 45.0f);
    drawGhostNode({1180.0f, 550.0f}, 35.0f);
    drawGhostNode({1340.0f, 550.0f}, 35.0f);

    btnSound.draw(window, engine.font);

    // Title Block with Soft Glow
    sf::Text tag(engine.font, "SECURE OPERATIONAL INTERFACE // V 2.4", 12);
    tag.setStyle(sf::Text::Bold);
    tag.setFillColor(Theme::AccentCyan);
    auto tb = tag.getLocalBounds();
    tag.setPosition({W * 0.5f - tb.size.x * 0.5f, 130.0f});
    window.draw(tag);

    // Title "VAULT HEIST"
    sf::Text title(engine.font, "VAULT HEIST", 68);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(Theme::Gold);
    auto titb = title.getLocalBounds();
    title.setPosition({W * 0.5f - titb.size.x * 0.5f, 155.0f});
    window.draw(title);

    // Subtitle "THE LIVING VAULT"
    sf::Text sub(engine.font, "THE LIVING VAULT", 20);
    sub.setStyle(sf::Text::Bold);
    sub.setFillColor(Theme::TextWhite);
    auto sb = sub.getLocalBounds();
    sub.setPosition({W * 0.5f - sb.size.x * 0.5f, 245.0f});
    window.draw(sub);

    // Thin accent line below title
    sf::RectangleShape line({320.0f, 2.0f});
    line.setPosition({W * 0.5f - 160.0f, 280.0f});
    line.setFillColor(sf::Color(56, 189, 248, 120));
    window.draw(line);

    // Draw Menu Buttons
    btnCampaign.draw(window, engine.font);
    btnEndless.draw(window, engine.font);
    btnSandbox.draw(window, engine.font);
    btnLeaderboard.draw(window, engine.font);
    btnSettings.draw(window, engine.font);
    btnQuit.draw(window, engine.font);
}
