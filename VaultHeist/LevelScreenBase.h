#pragma once
#include "Screen.h"
#include "UIKit.h"
#include "TreeRenderer.h"
#include <string>
#include <vector>

class GameEngine;

// LevelScreenBase: Standardized 75% Canvas + 25% Right Side Panel layout for all Act gameplay levels.
// Handles HUD, Side Panel stats/alarm/controls, Tree viewport, and Result Modal.
class LevelScreenBase : public Screen
{
public:
    LevelScreenBase(GameEngine& engine, const std::string& actTitle, const std::string& levelTitle);
    virtual ~LevelScreenBase() = default;

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void onResize(unsigned int width, unsigned int height) override;

    // Level state mutators
    void setAlarm(int currentAlarm, int alarmCapacity);
    void setScore(int score);
    void setSteps(int currentSteps, int optimalSteps);
    void setPenalty(int penaltyPoints);
    void setInstruction(const std::string& text);

    // Shows end-of-level Victory/Defeat dialog with Stars and 3 buttons
    void showResultModal(bool victory, int starCount, int stepCount, int optSteps, int penaltyPoints, int totalScore,
                         std::function<void()> nextCallback, std::function<void()> retryCallback, std::function<void()> backCallback);

protected:
    GameEngine& m_engine;
    std::string m_actTitle;
    std::string m_levelTitle;
    std::string m_instructionText;

    int   m_currentAlarm;
    int   m_alarmCap;
    int   m_score;
    int   m_currentSteps;
    int   m_optimalSteps;
    int   m_penaltyPoints;

    // Viewport & Layout (75% Canvas, 25% Side Panel)
    sf::FloatRect m_canvasBounds;
    sf::FloatRect m_sidePanelBounds;

    Button m_backBtn;
    Button m_hintBtn;
    Button m_resetBtn;

    LevelResultModal m_resultModal;

    TreeRenderer m_treeRenderer;
    TreeNode*    m_rootNode;  // Pointer to active tree

    // Virtual hooks for derived levels to customize
    virtual void onBackClicked();
    virtual void onHintClicked();
    virtual void onResetClicked();
    virtual void onNodeClicked(TreeNode* node);
    virtual void onCustomDraw(sf::RenderTarget& target) {}
    virtual void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds) {}

    void relayout(unsigned int width, unsigned int height);
};
