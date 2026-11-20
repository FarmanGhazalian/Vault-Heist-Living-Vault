#include "gui/Act3Level5Screen.h"
#include "gui/Act4LevelScreen.h"
#include "gui/CampaignScreen.h"
#include "gui/GameEngine.h"
#include "backend/SaveSystem.h"
#include "backend/ScoreSystem.h"
#include "gui/TreeRenderer.h"
#include <filesystem>
#include <cmath>
#include <cstdint>

Act3Level5Screen::Act3Level5Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 3: THE ARCHITECT'S WING", "LEVEL 3.5: ORDER-STATISTIC HEIST")
    , m_wrongNode(nullptr)
    , m_wrongTimer(0.0f)
    , m_correctNode(nullptr)
    , m_correctTimer(0.0f)
    , m_animTimer(0.0f)
{
    resetLevel();
}

void Act3Level5Screen::resetLevel()
{
    m_wrongNode = nullptr;
    m_wrongTimer = 0.0f;
    m_correctNode = nullptr;
    m_correctTimer = 0.0f;

    std::string path = "levels/Act3Level5.json";
    if (!std::filesystem::exists(path))
        path = "VaultHeist/levels/Act3Level5.json";

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTree();
        TreeRenderer::updateNodeMetrics(m_rootNode);
        int k = m_logic.getK();
        setInstruction("Find the " + std::to_string(k) + "-th smallest node in the AVL tree by clicking along the search path from root!");
        syncWithState();
        if (m_rootNode)
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
    }
}

void Act3Level5Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setAlarm(state.alarm, state.alarmCap);
    setScore(ScoreSystem::score(state));
    setSteps(state.steps, state.optimalSteps);
    setPenalty(state.penaltyPoints);
}

std::string Act3Level5Screen::getLevelSpecificHint()
{
    int k = m_logic.getK();
    return "k=" + std::to_string(k) + ". If left size is S: if k <= S go left; if k == S+1 target is here; if k > S+1 go right with k' = k - (S+1).";
}

ILevelLogic* Act3Level5Screen::getLevelLogic()
{
    return &m_logic;
}

void Act3Level5Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act3Level5Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_resultModal.active) return;

    PlayerAction action;
    action.type = CLICK_NODE;
    action.nodeA = node->val;

    ActionResult res = m_logic.processAction(action);
    syncWithState();

    if (res.correct)
    {
        m_engine.audio.playCorrect();
        m_correctNode = node;
        m_correctTimer = 1.0f;
        m_engine.showToast(res.message.empty() ? "PATH STEP CONFIRMED" : res.message, TOAST_CORRECT);

        if (m_logic.isWon())
        {
            const LevelState& state = m_logic.getState();
            int stars = ScoreSystem::stars(state);
            int scr = ScoreSystem::score(state);
            SaveSystem::setWon(3, 5, stars);
            showResultModal(true, stars, state.steps, state.optimalSteps, state.penaltyPoints, scr,
                [this]() { m_engine.changeScreen(std::make_unique<Act4LevelScreen>(m_engine, 1)); },
                [this]() { resetLevel(); m_resultModal.active = false; },
                [this]() { m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine)); });
        }
    }
    else
    {
        m_engine.audio.playWrong();
        m_wrongNode = node;
        m_wrongTimer = 0.5f;
        m_engine.showToast(res.message.empty() ? "DEVIATION DETECTED! Wrong node in path." : res.message, TOAST_ERROR);
    }
}

void Act3Level5Screen::handleEvent(const sf::Event& event)
{
    LevelScreenBase::handleEvent(event);
}

void Act3Level5Screen::update(float dt)
{
    LevelScreenBase::update(dt);
    m_animTimer += dt;
    if (m_correctTimer > 0.0f) m_correctTimer -= dt;
    if (m_wrongTimer > 0.0f)   m_wrongTimer -= dt;
}

void Act3Level5Screen::drawSubtreeSizes(sf::RenderTarget& target, TreeNode* node)
{
    if (!node) return;
    drawSubtreeSizes(target, node->left);
    drawSubtreeSizes(target, node->right);

    std::string szStr = "sz:" + std::to_string(node->size);
    sf::Text sz(m_engine.theme.fontMono, szStr, 10);
    sz.setFillColor(Theme::BorderCyan);
    sf::FloatRect lb = sz.getLocalBounds();
    sz.setPosition(sf::Vector2f(
        node->position.x - lb.size.x * 0.5f - lb.position.x,
        node->position.y - Theme::NodeRadius - 16.0f));
    target.draw(sz);
}

void Act3Level5Screen::onCustomDraw(sf::RenderTarget& target)
{
    if (m_rootNode)
        drawSubtreeSizes(target, m_rootNode);

    if (m_correctNode && m_correctTimer > 0.0f)
    {
        float alpha = m_correctTimer / 1.0f;
        sf::CircleShape glow(Theme::NodeRadius + 7.0f);
        glow.setOrigin(sf::Vector2f(glow.getRadius(), glow.getRadius()));
        glow.setPosition(m_correctNode->position);
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(3.0f);
        glow.setOutlineColor(sf::Color(0, 255, 120, static_cast<uint8_t>(220 * alpha)));
        target.draw(glow);
    }

    if (m_wrongTimer > 0.0f)
    {
        float alpha = m_wrongTimer / 0.5f;
        sf::RectangleShape flash(m_canvasBounds.size);
        flash.setPosition(m_canvasBounds.position);
        flash.setFillColor(sf::Color(255, 0, 0, static_cast<uint8_t>(30 * alpha)));
        target.draw(flash);
    }

    TreeRenderer::drawNodeTooltip(target, m_engine.theme, m_rootNode, nullptr);
}

void Act3Level5Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    int k = m_logic.getK();
    int curIdx = m_logic.getCurrentIndex();
    int pathLen = m_logic.getPathLength();

    sf::Text header(m_engine.theme.fontMono, "ORDER-STATISTIC RADAR", 12);
    header.setFillColor(Theme::BorderCyan);
    header.setStyle(sf::Text::Bold);
    header.setPosition(sf::Vector2f(px, py));
    target.draw(header);

    sf::Text txtK(m_engine.theme.fontMono, "TARGET: " + std::to_string(k) + "-th SMALLEST", 13);
    txtK.setFillColor(Theme::AccentYellow);
    txtK.setPosition(sf::Vector2f(px, py + 26.0f));
    target.draw(txtK);

    sf::Text txtProg(m_engine.theme.fontMono, "PATH DEPTH: " + std::to_string(curIdx) + " / " + std::to_string(pathLen), 12);
    txtProg.setFillColor(Theme::TextPrimary);
    txtProg.setPosition(sf::Vector2f(px, py + 50.0f));
    target.draw(txtProg);
}
