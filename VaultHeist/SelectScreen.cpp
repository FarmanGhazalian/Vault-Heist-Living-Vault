#include "SelectScreen.h"
#include "MainMenuScreen.h"
#include "Act1Level1Screen.h"
#include "Act1Level2Screen.h"
#include "Act1Level3Screen.h"
#include "Act1Level4Screen.h"
#include "Act1Level5Screen.h"
#include "GameEngine.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

SelectScreen::SelectScreen(int m, int act) : mode(m), currentAct(act) {
    btnBack = Button({{60.0f, 30.0f}, {120.0f, 40.0f}}, "< BACK", ButtonStyle::SECONDARY);
    btnSound = Button({{1500.0f - 180.0f, 30.0f}, {120.0f, 40.0f}}, "SOUND", ButtonStyle::SECONDARY);
    initTiles();
}

void SelectScreen::initTiles() {
    tiles.clear();
    tileButtons.clear();

    if (mode == SelectMode::ACTS) {
        tiles.push_back({"ACT 1", "The Blueprint Wing", false, 1});
        tiles.push_back({"ACT 2", "The Sorted Wing", true, 2});
        tiles.push_back({"ACT 3", "The Architect's Wing", true, 3});
        tiles.push_back({"ACT 4", "The Heap Fortress", true, 4});

        // Arrange in 2 rows of 2, brick-wall style
        float tileW = 340.0f;
        float tileH = 130.0f;
        float startY = 280.0f;

        // Row 1 (2 tiles)
        float row1X = (1500.0f - (2 * tileW + 40.0f)) * 0.5f;
        for (int i = 0; i < 2; ++i) {
            Button b({{row1X + i * (tileW + 40.0f), startY}, {tileW, tileH}}, tiles[i].title, tiles[i].isLocked ? ButtonStyle::SECONDARY : ButtonStyle::GOLD);
            b.tagText = tiles[i].tag;
            b.isLocked = false; // We allow opening Act 2-4 to see their locked levels!
            tileButtons.push_back(b);
        }

        // Row 2 (2 tiles, shifted brick-wall offset)
        float row2X = row1X + 40.0f;
        for (int i = 2; i < 4; ++i) {
            Button b({{row2X + (i - 2) * (tileW + 40.0f), startY + tileH + 40.0f}, {tileW, tileH}}, tiles[i].title, tiles[i].isLocked ? ButtonStyle::SECONDARY : ButtonStyle::GOLD);
            b.tagText = tiles[i].tag;
            b.isLocked = false;
            tileButtons.push_back(b);
        }
    } else {
        // MODE_LEVELS
        if (currentAct == 1) {
            auto readLevelName = [](const std::string& filename, const std::string& defName) {
                std::string path = GameEngine::resolveJsonPath(filename);
                std::ifstream f(path);
                if (f.is_open()) {
                    json j; f >> j;
                    if (j.contains("levelName")) return j["levelName"].get<std::string>();
                    if (j.contains("title")) return j["title"].get<std::string>();
                }
                return defName;
            };

            tiles.push_back({"LEVEL 1.1", readLevelName("Act1Level1.json", "Vault Patrol Entry"), false, 1});
            tiles.push_back({"LEVEL 1.2", readLevelName("Act1Level2.json", "Decode the Blueprint"), false, 2});
            tiles.push_back({"LEVEL 1.3", readLevelName("Act1Level3.json", "Alarm Spreads Upward"), false, 3});
            tiles.push_back({"LEVEL 1.4", readLevelName("Act1Level4.json", "Where Escape Routes Meet"), false, 4});
            tiles.push_back({"LEVEL 1.5", readLevelName("Act1Level5.json", "Mirror the Vault"), false, 5});
        } else if (currentAct == 2) {
            tiles.push_back({"LEVEL 2.1", "Search Heist", true, 1});
            tiles.push_back({"LEVEL 2.2", "Stash Loot", true, 2});
            tiles.push_back({"LEVEL 2.3", "Steal and Collapse", true, 3});
            tiles.push_back({"LEVEL 2.4", "Range Raid", true, 4});
            tiles.push_back({"LEVEL 2.5", "The Lazy Architect (Boss)", true, 5});
        } else if (currentAct == 3) {
            tiles.push_back({"LEVEL 3.1", "Read the Badges", true, 1});
            tiles.push_back({"LEVEL 3.2", "Single Rotations", true, 2});
            tiles.push_back({"LEVEL 3.3", "Double Rotations", true, 3});
            tiles.push_back({"LEVEL 3.4", "Auto Mode", true, 4});
            tiles.push_back({"LEVEL 3.5", "Order-Statistic Heist (Boss)", true, 5});
        } else if (currentAct == 4) {
            tiles.push_back({"LEVEL 4.1", "Activate the Vault", true, 1});
            tiles.push_back({"LEVEL 4.2", "Repair the Locks", true, 2});
            tiles.push_back({"LEVEL 4.3", "Extract the Treasure", true, 3});
            tiles.push_back({"LEVEL 4.4", "Race the Guards", true, 4});
            tiles.push_back({"LEVEL 4.5", "Final Vault Boss", true, 5});
        }

        // Brick-wall layout for 5 levels (Row 1: 3 tiles, Row 2: 2 tiles shifted by half-tile)
        float tileW = 320.0f;
        float tileH = 110.0f;
        float gap = 35.0f;
        float startY = 280.0f;

        // Row 1 (3 tiles)
        float row1X = (1500.0f - (3 * tileW + 2 * gap)) * 0.5f;
        for (int i = 0; i < 3 && i < static_cast<int>(tiles.size()); ++i) {
            Button b({{row1X + i * (tileW + gap), startY}, {tileW, tileH}}, tiles[i].title, tiles[i].isLocked ? ButtonStyle::SECONDARY : ButtonStyle::PRIMARY);
            b.tagText = tiles[i].tag;
            b.isLocked = tiles[i].isLocked;
            tileButtons.push_back(b);
        }

        // Row 2 (2 tiles, shifted by half tile)
        float row2X = (1500.0f - (2 * tileW + gap)) * 0.5f;
        for (int i = 3; i < 5 && i < static_cast<int>(tiles.size()); ++i) {
            Button b({{row2X + (i - 3) * (tileW + gap), startY + tileH + gap}, {tileW, tileH}}, tiles[i].title, tiles[i].isLocked ? ButtonStyle::SECONDARY : ButtonStyle::PRIMARY);
            b.tagText = tiles[i].tag;
            b.isLocked = tiles[i].isLocked;
            tileButtons.push_back(b);
        }
    }
}

void SelectScreen::handleEvent(GameEngine& engine, const sf::Event& event) {
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape) {
            engine.audio.playClick();
            if (mode == SelectMode::LEVELS) {
                engine.changeScreen(std::make_unique<SelectScreen>(SelectMode::ACTS));
            } else {
                engine.changeScreen(std::make_unique<MainMenuScreen>());
            }
            return;
        }
    }

    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mp->button == sf::Mouse::Button::Left) {
            if (btnBack.handleClick(mouse)) {
                engine.audio.playClick();
                if (mode == SelectMode::LEVELS) {
                    engine.changeScreen(std::make_unique<SelectScreen>(SelectMode::ACTS));
                } else {
                    engine.changeScreen(std::make_unique<MainMenuScreen>());
                }
                return;
            }

            if (btnSound.handleClick(mouse)) {
                engine.audio.toggleMute();
                return;
            }

            for (size_t i = 0; i < tileButtons.size(); ++i) {
                if (tileButtons[i].handleClick(mouse)) {
                    engine.audio.playClick();
                    if (mode == SelectMode::ACTS) {
                        int act = tiles[i].index;
                        engine.changeScreen(std::make_unique<SelectScreen>(SelectMode::LEVELS, act));
                    } else {
                        if (tiles[i].isLocked) {
                            engine.showComingSoon(tiles[i].title);
                        } else {
                            int lvl = tiles[i].index;
                            if (lvl == 1) engine.changeScreen(std::make_unique<Act1Level1Screen>());
                            else if (lvl == 2) engine.changeScreen(std::make_unique<Act1Level2Screen>());
                            else if (lvl == 3) engine.changeScreen(std::make_unique<Act1Level3Screen>());
                            else if (lvl == 4) engine.changeScreen(std::make_unique<Act1Level4Screen>());
                            else if (lvl == 5) engine.changeScreen(std::make_unique<Act1Level5Screen>());
                        }
                    }
                    return;
                }
            }
        }
    }
}

void SelectScreen::update(GameEngine& engine, float dt) {
    time += dt;
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};

    btnBack.update(mouse);
    btnSound.update(mouse);
    btnSound.text = engine.audio.isMuted() ? "SOUND OFF" : "SOUND ON";

    for (auto& b : tileButtons) {
        b.update(mouse);
    }
}

void SelectScreen::draw(GameEngine& engine, sf::RenderWindow& window) {
    float W = static_cast<float>(window.getSize().x);
    float H = static_cast<float>(window.getSize().y);

    window.clear(Theme::BgDark);
    drawBackgroundGrid(window, W, H, time);

    btnBack.draw(window, engine.font);
    btnSound.draw(window, engine.font);

    // Header Title
    std::string headerStr = (mode == SelectMode::ACTS) ? "CAMPAIGN // SELECT OPERATION WING" :
                            ("ACT " + std::to_string(currentAct) + " // SELECT INFILTRATION LEVEL");
    sf::Text tH(engine.font, headerStr, 13);
    tH.setStyle(sf::Text::Bold);
    tH.setFillColor(Theme::AccentCyan);
    tH.setPosition({W * 0.5f - tH.getLocalBounds().size.x * 0.5f, 130.0f});
    window.draw(tH);

    std::string subTitle = (mode == SelectMode::ACTS) ? "THE LIVING VAULT ARCHITECTURE" :
                           (currentAct == 1 ? "THE BLUEPRINT WING (BINARY TREES)" :
                            (currentAct == 2 ? "THE SORTED WING (BINARY SEARCH TREES)" :
                             (currentAct == 3 ? "THE ARCHITECT'S WING (AVL TREES)" : "THE HEAP FORTRESS")));
    sf::Text tSub(engine.font, subTitle, 26);
    tSub.setStyle(sf::Text::Bold);
    tSub.setFillColor(Theme::TextWhite);
    tSub.setPosition({W * 0.5f - tSub.getLocalBounds().size.x * 0.5f, 160.0f});
    window.draw(tSub);

    // Subtitle note
    std::string note = (mode == SelectMode::ACTS) ? "Select an operational wing to view sector blueprints." :
                       "Select an active security terminal to deploy.";
    sf::Text tNote(engine.font, note, 14);
    tNote.setFillColor(Theme::TextMuted);
    tNote.setPosition({W * 0.5f - tNote.getLocalBounds().size.x * 0.5f, 205.0f});
    window.draw(tNote);

    // Draw Wall Tiles
    for (const auto& b : tileButtons) {
        b.draw(window, engine.font);
    }
}
