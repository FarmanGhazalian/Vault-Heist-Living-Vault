#pragma once
#include "LevelScreenBase.h"
#include "Act1level1.h"
#include <memory>
#include <string>

class Act1Level1Screen : public LevelScreenBase {
private:
    std::unique_ptr<Act1Level1> levelLogic;
    std::string jsonPath;
    std::string traversalType = "PREORDER";
    std::string hintMessage = "";

    void loadLevelData();
    void restartLevelDirect();

public:
    Act1Level1Screen();

    void onLevelEvent(GameEngine& engine, const sf::Event& event) override;
    void updateLevel(GameEngine& engine, float dt) override;
    void drawLevel(GameEngine& engine, sf::RenderWindow& window) override;
    void restartLevel(GameEngine& engine) override;
    void requestHint(GameEngine& engine) override;
    ILevelLogic* getLevelLogic() override { return levelLogic.get(); }
    void goToNextLevel(GameEngine& engine) override;
};
