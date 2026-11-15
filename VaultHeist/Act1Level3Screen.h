#pragma once
#include "LevelScreenBase.h"
#include "Act1Level3.h"
#include <memory>
#include <string>
#include <vector>

class Act1Level3Screen : public LevelScreenBase {
private:
    std::unique_ptr<Act1Level3> levelLogic;
    std::string jsonPath;
    std::vector<int> preorderList;
    std::vector<int> inorderList;
    int selectedNodeVal = -1;
    std::string inputBuffer = "";
    bool allHeightsDone = false;
    int correctHeightsCount = 0;
    int totalNodes = 5;

    Button btnSubmit;
    Button btnCancel;
    Button btnSubmitDiameter;

    void loadLevelData();
    void restartLevelDirect();

public:
    Act1Level3Screen();

    void onLevelEvent(GameEngine& engine, const sf::Event& event) override;
    void updateLevel(GameEngine& engine, float dt) override;
    void drawLevel(GameEngine& engine, sf::RenderWindow& window) override;
    void restartLevel(GameEngine& engine) override;
    void requestHint(GameEngine& engine) override;
    ILevelLogic* getLevelLogic() override { return levelLogic.get(); }
    void goToNextLevel(GameEngine& engine) override;
};
