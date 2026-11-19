#pragma once
#include "Screen.h"
#include "UIKit.h"
#include "TreeRenderer.h"
#include <string>
#include <vector>
#include <functional>

class GameEngine;

// LevelScreenBase: Standardized Canvas + Right Side Panel + Action Bar layout.
// Handles HUD, 3-Card Side Panel (Objective, Telemetry+Segmented Alarm, Operation Log),
// Bottom Action Bar (HINT, UNDO, REPLAY, RESET), Pan/Zoom Canvas View, and Result Modal.
class LevelScreenBase : public Screen
{

    // ... existing members ...
    // New members for deferred screen changes (to avoid use‑after‑free when a result modal
    // invokes a screen transition while it is still active).
    bool m_pendingScreenChange = false;
    std::function<void()> m_pendingCallback;


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
    void addToLog(const std::string& entry);

    // Shows end-of-level Victory/Defeat dialog with Stars and buttons
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

    // Viewport & Layout
    sf::FloatRect m_canvasBounds;
    sf::FloatRect m_sidePanelBounds;
    sf::FloatRect m_actionBarBounds;

    // Canvas pan/zoom
    sf::View     m_canvasView;
    bool         m_isPanning;
    sf::Vector2i m_panStartPixel;
    sf::Vector2f m_panOriginCenter;
    bool         m_spaceHeld;
    float        m_zoomLevel;
    Button       m_fitBtn;

    // Top & Bottom Controls
    Button m_backBtn;
    Button m_hintBtn;
    Button m_undoBtn;
    Button m_replayBtn;
    Button m_resetBtn;

    // Operation Log
    std::vector<std::string> m_operationLog;

    LevelResultModal m_resultModal;

    TreeRenderer m_treeRenderer;
    TreeNode*    m_rootNode;

    sf::Vector2f canvasToWorld(sf::Vector2i pixelPos) const;

    // Virtual hooks for derived levels to customize
    virtual void onBackClicked();
    virtual void onHintClicked();
    virtual void onResetClicked();
    virtual void onUndoClicked();
    virtual void onReplayClicked();
    virtual void onNodeClicked(TreeNode* node);
    virtual void onCustomDraw(sf::RenderTarget& target) {}
    virtual void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds);

    void relayout(unsigned int width, unsigned int height);
};
