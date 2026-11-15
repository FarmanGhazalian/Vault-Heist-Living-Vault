#include "Act1Level3Screen.h"
#include "Act1Level4Screen.h"
#include "GameEngine.h"
#include "PlayerAction.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Act1Level3Screen::Act1Level3Screen() {
    jsonPath = GameEngine::resolveJsonPath("Act1Level3.json");
    btnSubmit = Button({{820.0f, 620.0f}, {100.0f, 38.0f}}, "SUBMIT", ButtonStyle::PRIMARY);
    btnCancel = Button({{930.0f, 620.0f}, {100.0f, 38.0f}}, "CANCEL", ButtonStyle::SECONDARY);
    btnSubmitDiameter = Button({{820.0f, 620.0f}, {150.0f, 38.0f}}, "SUBMIT DIA", ButtonStyle::GOLD);
    restartLevelDirect();
}

void Act1Level3Screen::restartLevelDirect() {
    levelLogic = std::make_unique<Act1Level3>();
    levelLogic->initLevel(jsonPath);
    loadLevelData();
    selectedNodeVal = -1;
    inputBuffer = "";
    allHeightsDone = false;
    correctHeightsCount = 0;
    resultShown = false;
}

void Act1Level3Screen::loadLevelData() {
    std::ifstream f(jsonPath);
    if (!f.is_open()) return;
    json j; f >> j;

    std::string name = j.value("levelName", "Alarm Spreads Upward");
    totalNodes = j.value("totalNodes", 5);

    preorderList.clear();
    inorderList.clear();
    for (const auto& v : j["preorderSequence"]) preorderList.push_back(v.get<int>());
    for (const auto& v : j["inorderSequence"]) inorderList.push_back(v.get<int>());

    setupHUD(name, "Click each room to enter its height. Then calculate overall tree diameter.", "Phase: Room Heights (0/" + std::to_string(totalNodes) + ")", 3);

    // Build visual tree for 1.3:
    // Pre: 1, 2, 4, 5, 3
    // In:  4, 2, 5, 1, 3
    // Root = 1
    // Left = 2 (Left of 2 = 4, Right of 2 = 5)
    // Right = 3
    std::vector<RenderNode> raw;
    RenderNode r1; r1.id = 1; r1.value = 1; r1.leftId = 2; r1.rightId = 3; raw.push_back(r1);
    RenderNode r2; r2.id = 2; r2.value = 2; r2.leftId = 4; r2.rightId = 5; raw.push_back(r2);
    RenderNode r3; r3.id = 3; r3.value = 3; r3.leftId = -1; r3.rightId = -1; raw.push_back(r3);
    RenderNode r4; r4.id = 4; r4.value = 4; r4.leftId = -1; r4.rightId = -1; raw.push_back(r4);
    RenderNode r5; r5.id = 5; r5.value = 5; r5.leftId = -1; r5.rightId = -1; raw.push_back(r5);

    treeRenderer.buildFromFlat(raw, 1, 750.0f, 220.0f, 320.0f);
}

void Act1Level3Screen::restartLevel(GameEngine& engine) {
    restartLevelDirect();
    engine.toasts.add("SENSORS RESET", "Height calculations reset.", ToastType::INFO);
}

void Act1Level3Screen::requestHint(GameEngine& engine) {
    if (!levelLogic) return;
    if (levelLogic->getState().hintUsed) {
        engine.toasts.add("HINT EXHAUSTED", "Hint already deployed.", ToastType::HINT);
        return;
    }
    levelLogic->onHintRequested();
    engine.audio.playHint();
    std::string h = levelLogic->getHint();
    if (h.empty()) h = "Leaf node height is 1. Parent height = 1 + max(left height, right height).";
    engine.toasts.add("TACTICAL INTEL", h, ToastType::HINT);
}

void Act1Level3Screen::onLevelEvent(GameEngine& engine, const sf::Event& event) {
    if (resultPopup.active || pausePopup.active) return;
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};

    // Text typing for input buffer
    if (const auto* ti = event.getIf<sf::Event::TextEntered>()) {
        if (ti->unicode >= '0' && ti->unicode <= '9') {
            if (inputBuffer.size() < 3) {
                inputBuffer += static_cast<char>(ti->unicode);
                engine.audio.playClick();
            }
        } else if (ti->unicode == 8) { // Backspace
            if (!inputBuffer.empty()) {
                inputBuffer.pop_back();
                engine.audio.playClick();
            }
        } else if (ti->unicode == 13) { // Enter
            if (!inputBuffer.empty()) {
                int enteredVal = std::stoi(inputBuffer);
                inputBuffer = "";

                if (!allHeightsDone && selectedNodeVal != -1) {
                    PlayerAction act{};
                    act.type = ENTER_HEIGHT;
                    act.nodeA = selectedNodeVal;
                    act.nodeB = enteredVal;
                    act.direction = 0;

                    ActionResult res = levelLogic->processAction(act);
                    if (res.accepted) {
                        if (res.correct) {
                            auto* n = treeRenderer.findByValue(selectedNodeVal);
                            if (n) {
                                n->isVisited = true;
                                n->stateLabel = "H=" + std::to_string(enteredVal);
                            }
                            correctHeightsCount++;
                            engine.audio.playCorrect();
                            engine.toasts.add("HEIGHT ACCEPTED", res.message.empty() ? "Room height verified." : res.message, ToastType::CORRECT);

                            if (correctHeightsCount >= totalNodes) {
                                allHeightsDone = true;
                                setupHUD(levelTitle, "All heights verified! Enter the overall vault tree DIAMETER.", "Phase: Enter Diameter", 3);
                                engine.toasts.add("DIAMETER PHASE", "Calculate tree diameter: longest path between any two rooms.", ToastType::INFO);
                            } else {
                                setupHUD(levelTitle, objectiveText, "Phase: Room Heights (" + std::to_string(correctHeightsCount) + "/" + std::to_string(totalNodes) + ")", 3);
                            }
                        } else {
                            treeRenderer.triggerWrong(selectedNodeVal);
                            engine.audio.playWrong();
                            std::string msg = res.message.empty() ? "Incorrect height value!" : res.message;
                            engine.toasts.add("CALCULATION ERROR", msg, ToastType::ERROR_MSG);
                        }
                    }
                    selectedNodeVal = -1;
                } else if (allHeightsDone) {
                    PlayerAction act{};
                    act.type = ENTER_DIAMETER;
                    act.nodeB = enteredVal;
                    act.direction = 0;

                    ActionResult res = levelLogic->processAction(act);
                    if (res.accepted) {
                        if (res.correct) {
                            engine.audio.playCorrect();
                            engine.toasts.add("DIAMETER VERIFIED", res.message.empty() ? "Vault diameter cracked!" : res.message, ToastType::CORRECT);
                        } else {
                            engine.audio.playWrong();
                            std::string msg = res.message.empty() ? "Incorrect tree diameter!" : res.message;
                            engine.toasts.add("DIAMETER MISMATCH", msg, ToastType::ERROR_MSG);
                        }
                    }
                }
            }
        }
    }

    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mp->button == sf::Mouse::Button::Left) {
            // Click Submit / Cancel
            if (!allHeightsDone && selectedNodeVal != -1) {
                if (btnSubmit.handleClick(mouse) && !inputBuffer.empty()) {
                    int enteredVal = std::stoi(inputBuffer);
                    inputBuffer = "";

                    PlayerAction act{};
                    act.type = ENTER_HEIGHT;
                    act.nodeA = selectedNodeVal;
                    act.nodeB = enteredVal;
                    act.direction = 0;

                    ActionResult res = levelLogic->processAction(act);
                    if (res.accepted) {
                        if (res.correct) {
                            auto* n = treeRenderer.findByValue(selectedNodeVal);
                            if (n) {
                                n->isVisited = true;
                                n->stateLabel = "H=" + std::to_string(enteredVal);
                            }
                            correctHeightsCount++;
                            engine.audio.playCorrect();
                            engine.toasts.add("HEIGHT ACCEPTED", res.message.empty() ? "Room height verified." : res.message, ToastType::CORRECT);

                            if (correctHeightsCount >= totalNodes) {
                                allHeightsDone = true;
                                setupHUD(levelTitle, "All heights verified! Enter the overall vault tree DIAMETER.", "Phase: Enter Diameter", 3);
                            } else {
                                setupHUD(levelTitle, objectiveText, "Phase: Room Heights (" + std::to_string(correctHeightsCount) + "/" + std::to_string(totalNodes) + ")", 3);
                            }
                        } else {
                            treeRenderer.triggerWrong(selectedNodeVal);
                            engine.audio.playWrong();
                            engine.toasts.add("CALCULATION ERROR", res.message.empty() ? "Incorrect height value!" : res.message, ToastType::ERROR_MSG);
                        }
                    }
                    selectedNodeVal = -1;
                    return;
                }
                if (btnCancel.handleClick(mouse)) {
                    selectedNodeVal = -1;
                    inputBuffer = "";
                    engine.audio.playClick();
                    return;
                }
            } else if (allHeightsDone) {
                if (btnSubmitDiameter.handleClick(mouse) && !inputBuffer.empty()) {
                    int enteredVal = std::stoi(inputBuffer);
                    inputBuffer = "";

                    PlayerAction act{};
                    act.type = ENTER_DIAMETER;
                    act.nodeB = enteredVal;
                    act.direction = 0;

                    ActionResult res = levelLogic->processAction(act);
                    if (res.accepted) {
                        if (res.correct) {
                            engine.audio.playCorrect();
                            engine.toasts.add("DIAMETER VERIFIED", res.message.empty() ? "Vault diameter cracked!" : res.message, ToastType::CORRECT);
                        } else {
                            engine.audio.playWrong();
                            engine.toasts.add("DIAMETER MISMATCH", res.message.empty() ? "Incorrect diameter!" : res.message, ToastType::ERROR_MSG);
                        }
                    }
                    return;
                }
            }

            // Click node in tree
            if (!allHeightsDone) {
                int clickedVal = treeRenderer.getNodeValueAt(mouse);
                if (clickedVal != -1) {
                    selectedNodeVal = clickedVal;
                    inputBuffer = "";
                    engine.audio.playClick();
                    for (auto& n : treeRenderer.nodes) {
                        n.isSelected = (n.value == clickedVal);
                    }
                }
            }
        }
    }
}

void Act1Level3Screen::updateLevel(GameEngine& engine, float dt) {
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};
    if (!allHeightsDone && selectedNodeVal != -1) {
        btnSubmit.update(mouse);
        btnCancel.update(mouse);
    } else if (allHeightsDone) {
        btnSubmitDiameter.update(mouse);
    }
}

void Act1Level3Screen::drawLevel(GameEngine& engine, sf::RenderWindow& window) {
    treeRenderer.draw(window, engine.font);

    // Draw Input Box at bottom
    if (!allHeightsDone && selectedNodeVal != -1) {
        sf::RectangleShape box({540.0f, 75.0f});
        box.setPosition({480.0f, 600.0f});
        box.setFillColor(sf::Color(14, 20, 36, 240));
        box.setOutlineThickness(1.0f);
        box.setOutlineColor(Theme::PanelBorder);
        window.draw(box);

        std::string prompt = "ENTER HEIGHT FOR ROOM #" + std::to_string(selectedNodeVal) + ":  [" + (inputBuffer.empty() ? "_" : inputBuffer) + "]";
        sf::Text pTxt(engine.font, prompt, 15);
        pTxt.setStyle(sf::Text::Bold);
        pTxt.setFillColor(Theme::Gold);
        pTxt.setPosition({505.0f, 626.0f});
        window.draw(pTxt);

        btnSubmit.draw(window, engine.font);
        btnCancel.draw(window, engine.font);
    } else if (allHeightsDone) {
        sf::RectangleShape box({580.0f, 75.0f});
        box.setPosition({460.0f, 600.0f});
        box.setFillColor(sf::Color(14, 20, 36, 240));
        box.setOutlineThickness(1.0f);
        box.setOutlineColor(Theme::Gold);
        window.draw(box);

        std::string prompt = "ENTER TREE DIAMETER:  [" + (inputBuffer.empty() ? "_" : inputBuffer) + "]";
        sf::Text pTxt(engine.font, prompt, 15);
        pTxt.setStyle(sf::Text::Bold);
        pTxt.setFillColor(Theme::Gold);
        pTxt.setPosition({490.0f, 626.0f});
        window.draw(pTxt);

        btnSubmitDiameter.draw(window, engine.font);
    }
}

void Act1Level3Screen::goToNextLevel(GameEngine& engine) {
    engine.changeScreen(std::make_unique<Act1Level4Screen>());
}
