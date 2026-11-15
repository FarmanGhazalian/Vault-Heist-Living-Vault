#pragma once
#include "Screen.h"
#include "UIKit.h"
#include "TreeRenderer.h"
#include "ILevelLogic.h"
#include <string>

class LevelScreenBase : public Screen {
protected:
    std::string levelTitle = "";
    std::string objectiveText = "";
    std::string traversalDesc = "";
    TreeRenderer treeRenderer;
    ModalPopup pausePopup;
    ModalPopup resultPopup;
    Button btnHint;
    Button btnRestart;
    Button btnSound;
    float levelTime = 0.0f;
    bool resultShown = false;
    int levelIndex = 1;

    void setupHUD(const std::string& title, const std::string& objective, const std::string& traversal, int lvlIdx);
    void drawHUD(GameEngine& engine, sf::RenderWindow& window, const LevelState& state);
    void checkLevelEnd(GameEngine& engine, ILevelLogic* logic);

public:
    LevelScreenBase();
    virtual ~LevelScreenBase() = default;

    void handleEvent(GameEngine& engine, const sf::Event& event) override;
    void update(GameEngine& engine, float dt) override;
    void draw(GameEngine& engine, sf::RenderWindow& window) override;

    virtual void onLevelEvent(GameEngine& engine, const sf::Event& event) = 0;
    virtual void updateLevel(GameEngine& engine, float dt) = 0;
    virtual void drawLevel(GameEngine& engine, sf::RenderWindow& window) = 0;
    virtual void restartLevel(GameEngine& engine) = 0;
    virtual void requestHint(GameEngine& engine) = 0;
    virtual ILevelLogic* getLevelLogic() = 0;
    virtual void goToNextLevel(GameEngine& engine) = 0;
};
