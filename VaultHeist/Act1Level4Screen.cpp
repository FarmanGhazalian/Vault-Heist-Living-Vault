#include "Act1Level4Screen.h"
#include "Act1Level5Screen.h"
#include "GameEngine.h"
#include "PlayerAction.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Act1Level4Screen::Act1Level4Screen() {
    jsonPath = GameEngine::resolveJsonPath("Act1Level4.json");
    restartLevelDirect();
}

void Act1Level4Screen::restartLevelDirect() {
    levelLogic = std::make_unique<Act1Level4>();
    levelLogic->initLevel(jsonPath);
    loadLevelData();
    resultShown = false;
}

void Act1Level4Screen::loadLevelData() {
    roomA = levelLogic->getRoomA();
    roomB = levelLogic->getRoomB();

    std::string name = "Where Escape Routes Meet";
    std::ifstream f(jsonPath);
    if (f.is_open()) {
        json j; f >> j;
        name = j.value("levelName", name);
    }

    std::string obj = "Find the LCA meeting room for Target A (" + std::to_string(roomA) +
                      ") and Target B (" + std::to_string(roomB) + ").";
    setupHUD(name, obj, "Phase 1: Identify Lowest Common Ancestor (LCA)", 4);

    // Build visual tree from backend tree root
    BinaryTree& btree = const_cast<BinaryTree&>(levelLogic->getTree());
    treeRenderer.buildFromTree(btree.getRoot(), 750.0f, 220.0f, 320.0f);

    auto* nA = treeRenderer.findByValue(roomA);
    if (nA) {
        nA->isTarget = true;
        nA->stateLabel = "TARGET A";
    }
    auto* nB = treeRenderer.findByValue(roomB);
    if (nB) {
        nB->isTarget = true;
        nB->stateLabel = "TARGET B";
    }
}

void Act1Level4Screen::restartLevel(GameEngine& engine) {
    restartLevelDirect();
    engine.toasts.add("RADAR RESET", "Rendezvous and escape routes reset.", ToastType::INFO);
}

void Act1Level4Screen::requestHint(GameEngine& engine) {
    if (!levelLogic) return;
    if (levelLogic->getState().hintUsed) {
        engine.toasts.add("HINT EXHAUSTED", "Hint already deployed.", ToastType::HINT);
        return;
    }
    levelLogic->onHintRequested();
    engine.audio.playHint();
    std::string h = "First click the Lowest Common Ancestor where paths from Room #" + std::to_string(roomA) +
                    " and Room #" + std::to_string(roomB) + " meet. Then click path nodes in order.";
    engine.toasts.add("TACTICAL INTEL", h, ToastType::HINT);
}

void Act1Level4Screen::onLevelEvent(GameEngine& engine, const sf::Event& event) {
    if (resultPopup.active || pausePopup.active) return;

    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mp->button == sf::Mouse::Button::Left) {
            sf::Vector2f mouse{static_cast<float>(mp->position.x), static_cast<float>(mp->position.y)};
            int clickedVal = treeRenderer.getNodeValueAt(mouse);
            if (clickedVal != -1) {
                PlayerAction act{};
                act.direction = 0;

                if (!levelLogic->isLCAFound()) {
                    act.type = CLICK_LCA;
                    act.nodeA = clickedVal;

                    ActionResult res = levelLogic->processAction(act);
                    if (res.accepted) {
                        if (res.correct) {
                            auto* n = treeRenderer.findByValue(clickedVal);
                            if (n) {
                                n->isLCA = true;
                                n->stateLabel = "LCA";
                            }
                            engine.audio.playCorrect();
                            engine.toasts.add("LCA CONFIRMED", res.message.empty() ? "Rendezvous point located." : res.message, ToastType::CORRECT);

                            std::string obj = "Trace escape path: click rooms in sequence from Target #" +
                                              std::to_string(roomA) + " to Target #" + std::to_string(roomB) + ".";
                            setupHUD(levelTitle, obj, "Phase 2: Trace Escape Corridor", 4);
                        } else {
                            treeRenderer.triggerWrong(clickedVal);
                            engine.audio.playWrong();
                            engine.toasts.add("NOT THE LCA", res.message.empty() ? "Incorrect LCA room!" : res.message, ToastType::ERROR_MSG);
                        }
                    }
                } else {
                    act.type = TRACE_PATH;
                    act.nodeA = clickedVal;

                    ActionResult res = levelLogic->processAction(act);
                    if (res.accepted) {
                        if (res.correct) {
                            auto* n = treeRenderer.findByValue(clickedVal);
                            if (n) {
                                n->isVisited = true;
                                if (!n->isTarget && !n->isLCA) n->stateLabel = "PATH";
                            }
                            engine.audio.playCorrect();
                            engine.toasts.add("ROUTE STEP CLEARED", res.message.empty() ? "Path waypoint unlocked." : res.message, ToastType::CORRECT);
                        } else {
                            treeRenderer.triggerWrong(clickedVal);
                            engine.audio.playWrong();
                            engine.toasts.add("OFF-COURSE STEP", res.message.empty() ? "Wrong waypoint selected!" : res.message, ToastType::ERROR_MSG);
                        }
                    }
                }
            }
        }
    }
}

void Act1Level4Screen::updateLevel(GameEngine& engine, float dt) {
}

void Act1Level4Screen::drawLevel(GameEngine& engine, sf::RenderWindow& window) {
    treeRenderer.draw(window, engine.font);
}

void Act1Level4Screen::goToNextLevel(GameEngine& engine) {
    engine.changeScreen(std::make_unique<Act1Level5Screen>());
}
