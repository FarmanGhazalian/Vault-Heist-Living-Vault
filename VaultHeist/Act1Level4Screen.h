#pragma once
#include "LevelScreenBase.h"
#include "Act1Level4.h"
#include <memory>
#include <string>

class Act1Level4Screen : public LevelScreenBase {
private:
    std::unique_ptr<Act1Level4> levelLogic;
    std::string jsonPath;
    int roomA = -1;
    int roomB = -1;

    void loadLevelData();
    void restartLevelDirect();

public:
    Act1Level4Screen();

    void onLevelEvent(GameEngine& engine, const sf::Event& event) override;
    void updateLevel(GameEngine& engine, float dt) override;
    void drawLevel(GameEngine& engine, sf::RenderWindow& window) override;
    void restartLevel(GameEngine& engine) override;
    void requestHint(GameEngine& engine) override;
    ILevelLogic* getLevelLogic() override { return levelLogic.get(); }
    void goToNextLevel(GameEngine& engine) override;
};
