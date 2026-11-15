#include "Act1Level1Screen.h"
#include "Act1Level2Screen.h"
#include "GameEngine.h"
#include "PlayerAction.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

using json = nlohmann::json;

Act1Level1Screen::Act1Level1Screen() {
    jsonPath = GameEngine::resolveJsonPath("Act1Level1.json");
    restartLevelDirect();
}

void Act1Level1Screen::restartLevelDirect() {
    levelLogic = std::make_unique<Act1Level1>();
    levelLogic->initLevel(jsonPath);
    loadLevelData();
    resultShown = false;
}

void Act1Level1Screen::loadLevelData() {
    std::ifstream f(jsonPath);
    if (!f.is_open()) return;
    json j; f >> j;

    std::string name = "Vault Patrol Entry";
    if (j.contains("levelName")) name = j["levelName"].get<std::string>();
    if (j.contains("traversalType")) traversalType = j["traversalType"].get<std::string>();

    std::string travDesc = "PREORDER (Root -> Left -> Right)";
    if (traversalType == "INORDER") {
        travDesc = "INORDER (Left -> Root -> Right)";
        hintMessage = "Inorder traverses the Left branch first, accesses the Root node, then sweeps the Right branch.";
    } else if (traversalType == "POSTORDER") {
        travDesc = "POSTORDER (Left -> Right -> Root)";
        hintMessage = "Postorder sweeps Left, then Right subtree, and accesses the Root at the very end.";
    } else {
        travDesc = "PREORDER (Root -> Left -> Right)";
        hintMessage = "Preorder visits the Root room first, then traverses Left subtree, then Right.";
    }

    setupHUD(name, "Access vault rooms in " + travDesc + " sequence.", "Sequence: " + travDesc, 1);

    std::vector<RenderNode> raw;
    for (const auto& item : j["nodes"]) {
        RenderNode rn;
        rn.id = item["id"];
        rn.value = item["value"];
        rn.leftId = item["left"];
        rn.rightId = item["right"];
        raw.push_back(rn);
    }
    int rootId = j.value("rootId", 101);
    treeRenderer.buildFromFlat(raw, rootId, 750.0f, 230.0f, 320.0f);
}

void Act1Level1Screen::restartLevel(GameEngine& engine) {
    restartLevelDirect();
    engine.toasts.add("SYSTEM REBOOT", "Patrol route and alarms reset.", ToastType::INFO);
}

void Act1Level1Screen::requestHint(GameEngine& engine) {
    if (!levelLogic) return;
    if (levelLogic->getState().hintUsed) {
        engine.toasts.add("HINT EXHAUSTED", "Hint already deployed for this level.", ToastType::HINT);
        return;
    }
    levelLogic->onHintRequested();
    engine.audio.playHint();
    engine.toasts.add("TACTICAL INTEL", hintMessage, ToastType::HINT);
}

void Act1Level1Screen::onLevelEvent(GameEngine& engine, const sf::Event& event) {
    if (resultPopup.active || pausePopup.active) return;

    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mp->button == sf::Mouse::Button::Left) {
            sf::Vector2f mouse{static_cast<float>(mp->position.x), static_cast<float>(mp->position.y)};
            int clickedVal = treeRenderer.getNodeValueAt(mouse);
            if (clickedVal != -1) {
                PlayerAction act{};
                act.type = CLICK_NODE;
                act.nodeA = clickedVal;
                act.direction = 0;

                ActionResult res = levelLogic->processAction(act);
                if (res.accepted) {
                    if (res.correct) {
                        treeRenderer.markVisited(clickedVal);
                        engine.audio.playCorrect();
                        engine.toasts.add("NODE CLEARED", res.message.empty() ? "Access granted." : res.message, ToastType::CORRECT);
                    } else {
                        treeRenderer.triggerWrong(clickedVal);
                        engine.audio.playWrong();
                        std::string msg = res.message.empty() ? "Security breach: wrong sequence!" : res.message;
                        engine.toasts.add("ALARM TRIP", msg, ToastType::ERROR_MSG);
                    }
                }
            }
        }
    }
}

void Act1Level1Screen::updateLevel(GameEngine& engine, float dt) {
    // TreeRenderer is updated in base
}

void Act1Level1Screen::drawLevel(GameEngine& engine, sf::RenderWindow& window) {
    treeRenderer.draw(window, engine.font);
}

void Act1Level1Screen::goToNextLevel(GameEngine& engine) {
    engine.changeScreen(std::make_unique<Act1Level2Screen>());
}
