#include "Act1Level2Screen.h"
#include "Act1Level3Screen.h"
#include "GameEngine.h"
#include "PlayerAction.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Act1Level2Screen::Act1Level2Screen() {
    jsonPath = GameEngine::resolveJsonPath("Act1Level2.json");
    restartLevelDirect();
}

void Act1Level2Screen::restartLevelDirect() {
    levelLogic = std::make_unique<Act1Level2>();
    levelLogic->initLevel(jsonPath);
    loadLevelData();
    setupTreeSlots();
    resultShown = false;
}

void Act1Level2Screen::loadLevelData() {
    std::ifstream f(jsonPath);
    if (!f.is_open()) return;
    json j; f >> j;

    std::string name = j.value("levelName", "Decode the Blueprint");
    preorderList.clear();
    inorderList.clear();
    for (const auto& v : j["preorderSequence"]) preorderList.push_back(v.get<int>());
    for (const auto& v : j["inorderSequence"]) inorderList.push_back(v.get<int>());

    std::string travDesc = "Preorder & Inorder Blueprint Reconstruction";
    setupHUD(name, "Drag tokens from inventory into their correct blueprint slots.", travDesc, 2);

    // Setup Token Tray
    tokens.clear();
    float startX = 750.0f - static_cast<float>(preorderList.size() - 1) * 45.0f;
    for (size_t i = 0; i < preorderList.size(); ++i) {
        TokenItem t;
        t.value = preorderList[i];
        t.homePos = {startX + static_cast<float>(i) * 90.0f, 650.0f};
        t.currentPos = t.homePos;
        tokens.push_back(t);
    }
}

void Act1Level2Screen::setupTreeSlots() {
    slots.clear();
    // Slots for the 5-node tree:
    // Root = 1
    TokenSlot sRoot; sRoot.parentVal = -1; sRoot.direction = 0; sRoot.expectedVal = 1; sRoot.pos = {750.0f, 210.0f};
    slots.push_back(sRoot);

    // Left of 1 = 2
    TokenSlot sL1; sL1.parentVal = 1; sL1.direction = 0; sL1.expectedVal = 2; sL1.pos = {540.0f, 320.0f};
    slots.push_back(sL1);

    // Left of 2 = 4
    TokenSlot sL2; sL2.parentVal = 2; sL2.direction = 0; sL2.expectedVal = 4; sL2.pos = {410.0f, 430.0f};
    slots.push_back(sL2);

    // Right of 1 = 3
    TokenSlot sR1; sR1.parentVal = 1; sR1.direction = 1; sR1.expectedVal = 3; sR1.pos = {960.0f, 320.0f};
    slots.push_back(sR1);

    // Right of 3 = 5
    TokenSlot sR3; sR3.parentVal = 3; sR3.direction = 1; sR3.expectedVal = 5; sR3.pos = {1090.0f, 430.0f};
    slots.push_back(sR3);
}

void Act1Level2Screen::restartLevel(GameEngine& engine) {
    restartLevelDirect();
    engine.toasts.add("BLUEPRINT PURGED", "Tokens returned to inventory.", ToastType::INFO);
}

void Act1Level2Screen::requestHint(GameEngine& engine) {
    if (!levelLogic) return;
    if (levelLogic->getState().hintUsed) {
        engine.toasts.add("HINT EXHAUSTED", "Hint already deployed.", ToastType::HINT);
        return;
    }
    levelLogic->onHintRequested();
    engine.audio.playHint();
    std::string h = levelLogic->getHint();
    if (h.empty()) h = "Preorder root is first (1). Find 1 in inorder to split left (4, 2) and right (3, 5).";
    engine.toasts.add("TACTICAL INTEL", h, ToastType::HINT);
}

void Act1Level2Screen::onLevelEvent(GameEngine& engine, const sf::Event& event) {
    if (resultPopup.active || pausePopup.active) return;

    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mp->button == sf::Mouse::Button::Left) {
            sf::Vector2f mouse{static_cast<float>(mp->position.x), static_cast<float>(mp->position.y)};
            for (size_t i = 0; i < tokens.size(); ++i) {
                if (tokens[i].isPlaced) continue;
                float dx = mouse.x - tokens[i].currentPos.x;
                float dy = mouse.y - tokens[i].currentPos.y;
                if (dx * dx + dy * dy <= 32.0f * 32.0f) {
                    draggingIdx = static_cast<int>(i);
                    dragOffset = tokens[i].currentPos - mouse;
                    tokens[i].isDragging = true;
                    engine.audio.playClick();
                    break;
                }
            }
        }
    } else if (const auto* mr = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (mr->button == sf::Mouse::Button::Left && draggingIdx != -1) {
            auto& tok = tokens[draggingIdx];
            tok.isDragging = false;

            // Check if dropped onto a slot
            int matchedSlotIdx = -1;
            for (size_t i = 0; i < slots.size(); ++i) {
                if (slots[i].filled) continue;
                float dx = tok.currentPos.x - slots[i].pos.x;
                float dy = tok.currentPos.y - slots[i].pos.y;
                if (dx * dx + dy * dy <= 42.0f * 42.0f) {
                    matchedSlotIdx = static_cast<int>(i);
                    break;
                }
            }

            if (matchedSlotIdx != -1) {
                const auto& sl = slots[matchedSlotIdx];
                PlayerAction act{};
                act.type = PLACE_TOKEN;
                act.nodeA = sl.parentVal;
                act.direction = sl.direction;
                act.nodeB = tok.value;

                ActionResult res = levelLogic->processAction(act);
                if (res.accepted) {
                    if (res.correct) {
                        tok.isPlaced = true;
                        tok.currentPos = sl.pos;
                        slots[matchedSlotIdx].filled = true;
                        slots[matchedSlotIdx].currentVal = tok.value;
                        engine.audio.playCorrect();
                        engine.toasts.add("TOKEN SECURED", res.message.empty() ? "Token placed correctly." : res.message, ToastType::CORRECT);
                    } else {
                        tok.wrongTimer = 0.5f;
                        tok.currentPos = tok.homePos;
                        engine.audio.playWrong();
                        std::string msg = res.message.empty() ? "Invalid placement: structure mismatch!" : res.message;
                        engine.toasts.add("STRUCTURAL MISMATCH", msg, ToastType::ERROR_MSG);
                    }
                } else {
                    tok.currentPos = tok.homePos;
                }
            } else {
                tok.currentPos = tok.homePos;
            }
            draggingIdx = -1;
        }
    }
}

void Act1Level2Screen::updateLevel(GameEngine& engine, float dt) {
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};

    if (draggingIdx != -1) {
        tokens[draggingIdx].currentPos = mouse + dragOffset;
    }

    for (auto& tok : tokens) {
        if (!tok.isDragging && !tok.isPlaced) {
            sf::Vector2f diff = tok.homePos - tok.currentPos;
            tok.currentPos += diff * std::min(1.0f, dt * 12.0f);
        }
        if (tok.wrongTimer > 0.0f) {
            tok.wrongTimer -= dt;
        }
    }
}

void Act1Level2Screen::drawLevel(GameEngine& engine, sf::RenderWindow& window) {
    // 1. Draw Sequences Header
    std::string preStr = "PREORDER: [ ";
    for (size_t i = 0; i < preorderList.size(); ++i) {
        preStr += std::to_string(preorderList[i]) + (i + 1 < preorderList.size() ? ", " : " ]");
    }
    std::string inStr = "INORDER: [ ";
    for (size_t i = 0; i < inorderList.size(); ++i) {
        inStr += std::to_string(inorderList[i]) + (i + 1 < inorderList.size() ? ", " : " ]");
    }

    sf::Text tPre(engine.font, preStr, 14);
    tPre.setStyle(sf::Text::Bold);
    tPre.setFillColor(Theme::Gold);
    tPre.setPosition({100.0f, 95.0f});
    window.draw(tPre);

    sf::Text tIn(engine.font, inStr, 14);
    tIn.setStyle(sf::Text::Bold);
    tIn.setFillColor(Theme::AccentCyan);
    tIn.setPosition({520.0f, 95.0f});
    window.draw(tIn);

    // 2. Draw Slot Connection Lines
    auto drawLine = [&](const sf::Vector2f& a, const sf::Vector2f& b) {
        sf::Vector2f diff = b - a;
        float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
        sf::RectangleShape line({len, 3.0f});
        line.setOrigin({0.0f, 1.5f});
        line.setPosition(a);
        line.setRotation(sf::degrees(std::atan2(diff.y, diff.x) * 180.0f / 3.14159265f));
        line.setFillColor(sf::Color(45, 60, 90, 160));
        window.draw(line);
    };

    if (slots.size() >= 5) {
        drawLine(slots[0].pos, slots[1].pos);
        drawLine(slots[1].pos, slots[2].pos);
        drawLine(slots[0].pos, slots[3].pos);
        drawLine(slots[3].pos, slots[4].pos);
    }

    // 3. Draw Empty / Filled Blueprint Slots
    for (const auto& sl : slots) {
        float r = 32.0f;
        sf::CircleShape slotShape(r);
        slotShape.setOrigin({r, r});
        slotShape.setPosition(sl.pos);

        if (sl.filled) {
            slotShape.setFillColor(sf::Color(16, 75, 45));
            slotShape.setOutlineThickness(1.5f);
            slotShape.setOutlineColor(Theme::Green);
            window.draw(slotShape);

            sf::Text vTxt(engine.font, std::to_string(sl.currentVal), 20);
            vTxt.setStyle(sf::Text::Bold);
            vTxt.setFillColor(Theme::TextWhite);
            auto vb = vTxt.getLocalBounds();
            vTxt.setOrigin({vb.position.x + vb.size.x * 0.5f, vb.position.y + vb.size.y * 0.5f});
            vTxt.setPosition(sl.pos);
            window.draw(vTxt);
        } else {
            slotShape.setFillColor(sf::Color(14, 20, 36, 160));
            slotShape.setOutlineThickness(1.5f);
            slotShape.setOutlineColor(Theme::PanelBorder);
            window.draw(slotShape);

            std::string slotHint = (sl.parentVal == -1) ? "ROOT" : (sl.direction == 0 ? "L" : "R");
            sf::Text sTxt(engine.font, slotHint, 12);
            sTxt.setFillColor(Theme::TextMuted);
            auto sb = sTxt.getLocalBounds();
            sTxt.setOrigin({sb.position.x + sb.size.x * 0.5f, sb.position.y + sb.size.y * 0.5f});
            sTxt.setPosition(sl.pos);
            window.draw(sTxt);
        }
    }

    // 4. Token Tray Container
    sf::RectangleShape tray({680.0f, 75.0f});
    tray.setPosition({410.0f, 615.0f});
    tray.setFillColor(sf::Color(14, 20, 36, 230));
    tray.setOutlineThickness(1.0f);
    tray.setOutlineColor(Theme::PanelBorder);
    window.draw(tray);

    sf::Text trayLbl(engine.font, "TOKEN INVENTORY (DRAG TO BLUEPRINT)", 11);
    trayLbl.setStyle(sf::Text::Bold);
    trayLbl.setFillColor(Theme::TextMuted);
    trayLbl.setPosition({430.0f, 624.0f});
    window.draw(trayLbl);

    // 5. Draw Tokens
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (tokens[i].isPlaced) continue;
        const auto& tok = tokens[i];
        float r = 26.0f;
        sf::CircleShape tokShape(r);
        tokShape.setOrigin({r, r});

        sf::Vector2f p = tok.currentPos;
        if (tok.wrongTimer > 0.0f) {
            p.x += std::sin(tok.wrongTimer * 50.0f) * 6.0f;
        }
        tokShape.setPosition(p);

        if (tok.isDragging) {
            tokShape.setFillColor(sf::Color(35, 60, 100));
            tokShape.setOutlineThickness(2.0f);
            tokShape.setOutlineColor(Theme::AccentCyan);
        } else {
            tokShape.setFillColor(sf::Color(22, 32, 54));
            tokShape.setOutlineThickness(1.0f);
            tokShape.setOutlineColor(Theme::AccentCyan);
        }
        window.draw(tokShape);

        sf::Text num(engine.font, std::to_string(tok.value), 18);
        num.setStyle(sf::Text::Bold);
        num.setFillColor(Theme::TextWhite);
        auto nb = num.getLocalBounds();
        num.setOrigin({nb.position.x + nb.size.x * 0.5f, nb.position.y + nb.size.y * 0.5f});
        num.setPosition(p);
        window.draw(num);
    }
}

void Act1Level2Screen::goToNextLevel(GameEngine& engine) {
    engine.changeScreen(std::make_unique<Act1Level3Screen>());
}
