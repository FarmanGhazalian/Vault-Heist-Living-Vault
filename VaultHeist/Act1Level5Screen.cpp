#include "Act1Level5Screen.h"
#include "SelectScreen.h"
#include "GameEngine.h"
#include "PlayerAction.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Act1Level5Screen::Act1Level5Screen() {
    jsonPath = GameEngine::resolveJsonPath("Act1Level5.json");
    btnAutoMirror = Button({{680.0f, 620.0f}, {140.0f, 38.0f}}, "AUTO MIRROR", ButtonStyle::PRIMARY);
    restartLevelDirect();
}

void Act1Level5Screen::restartLevelDirect() {
    levelLogic = std::make_unique<Act1Level5>();
    levelLogic->initLevel(jsonPath);
    loadLevelData();
    resultShown = false;
}

void Act1Level5Screen::loadLevelData() {
    std::string name = "Mirror the Vault";
    std::ifstream f(jsonPath);
    if (f.is_open()) {
        json j; f >> j;
        if (j.contains("title")) name = j["title"].get<std::string>();
        if (j.contains("levelName")) name = j["levelName"].get<std::string>();
    }

    setupHUD(name, "Mirror the vault: click internal rooms bottom-up to invert their children branches.", "Mode: Bottom-Up Tree Reflection", 5);

    // Initial tree build
    BinaryTree& btree = const_cast<BinaryTree&>(levelLogic->getTree());
    treeRenderer.buildFromTree(btree.getRoot(), 750.0f, 220.0f, 320.0f);
}

void Act1Level5Screen::restartLevel(GameEngine& engine) {
    restartLevelDirect();
    engine.toasts.add("REFLECTION RESET", "Vault structure returned to original state.", ToastType::INFO);
}

void Act1Level5Screen::requestHint(GameEngine& engine) {
    if (!levelLogic) return;
    if (levelLogic->getState().hintUsed) {
        engine.toasts.add("HINT EXHAUSTED", "Hint already deployed.", ToastType::HINT);
        return;
    }
    levelLogic->onHintRequested();
    engine.audio.playHint();
    std::string h = levelLogic->getHint();
    if (h.empty()) h = "Work bottom-up: swap lowest parent rooms whose children are leaves first, swap Root at the end!";
    engine.toasts.add("TACTICAL INTEL", h, ToastType::HINT);
}

void Act1Level5Screen::onLevelEvent(GameEngine& engine, const sf::Event& event) {
    if (resultPopup.active || pausePopup.active) return;
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};

    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mp->button == sf::Mouse::Button::Left) {
            // Auto mirror button
            if (btnAutoMirror.handleClick(mouse)) {
                PlayerAction act{};
                act.type = AUTO_MIRROR;
                act.direction = 0;

                ActionResult res = levelLogic->processAction(act);
                if (res.accepted) {
                    BinaryTree& btree = const_cast<BinaryTree&>(levelLogic->getTree());
                    treeRenderer.buildFromTree(btree.getRoot(), 750.0f, 220.0f, 320.0f);
                    engine.audio.playCorrect();
                    engine.toasts.add("AUTO REFLECT", res.message.empty() ? "Vault mirrored automatically." : res.message, ToastType::CORRECT);
                }
                return;
            }

            // Click node in tree
            int clickedVal = treeRenderer.getNodeValueAt(mouse);
            if (clickedVal != -1) {
                PlayerAction act{};
                act.type = SWAP_CHILDREN;
                act.nodeA = clickedVal;
                act.direction = 0;

                ActionResult res = levelLogic->processAction(act);
                if (res.accepted) {
                    if (res.correct) {
                        BinaryTree& btree = const_cast<BinaryTree&>(levelLogic->getTree());
                        treeRenderer.buildFromTree(btree.getRoot(), 750.0f, 220.0f, 320.0f);
                        auto* n = treeRenderer.findByValue(clickedVal);
                        if (n) {
                            n->isVisited = true;
                            n->stateLabel = "SWAPPED";
                        }
                        engine.audio.playCorrect();
                        engine.toasts.add("BRANCH INVERTED", res.message.empty() ? "Children swapped." : res.message, ToastType::CORRECT);
                    } else {
                        treeRenderer.triggerWrong(clickedVal);
                        engine.audio.playWrong();
                        std::string msg = res.message.empty() ? "Cannot swap yet: child subtrees not mirrored!" : res.message;
                        engine.toasts.add("ORDER VIOLATION", msg, ToastType::ERROR_MSG);
                    }
                }
            }
        }
    }
}

void Act1Level5Screen::updateLevel(GameEngine& engine, float dt) {
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};
    btnAutoMirror.update(mouse);
}

void Act1Level5Screen::drawLevel(GameEngine& engine, sf::RenderWindow& window) {
    treeRenderer.draw(window, engine.font);
    btnAutoMirror.draw(window, engine.font);
}

void Act1Level5Screen::goToNextLevel(GameEngine& engine) {
    engine.changeScreen(std::make_unique<SelectScreen>(SelectMode::LEVELS, 1));
    engine.toasts.add("ACT 1 CONQUERED", "The Blueprint Wing successfully infiltrated!", ToastType::CORRECT);
}
