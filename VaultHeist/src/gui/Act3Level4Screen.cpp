#include "gui/Act3Level4Screen.h"
#include "gui/Act3Level5Screen.h"
#include "gui/CampaignScreen.h"
#include "gui/GameEngine.h"
#include "backend/SaveSystem.h"
#include "backend/ScoreSystem.h"
#include "gui/TreeRenderer.h"
#include <filesystem>
#include <cmath>
#include <cstdint>

Act3Level4Screen::Act3Level4Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 3: THE ARCHITECT'S WING", "LEVEL 3.4: AUTO MODE & PREDICTION")
    , m_btnLL("LL", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(54, 34)), BTN_SECONDARY)
    , m_btnRR("RR", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(54, 34)), BTN_SECONDARY)
    , m_btnLR("LR", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(54, 34)), BTN_SECONDARY)
    , m_btnRL("RL", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(54, 34)), BTN_SECONDARY)
    , m_btnNone("NONE", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(60, 34)), BTN_PRIMARY)
    , m_wrongTimer(0.0f)
    , m_correctTimer(0.0f)
    , m_animTimer(0.0f)
{
    resetLevel();
}

void Act3Level4Screen::resetLevel()
{
    m_wrongTimer = 0.0f;
    m_correctTimer = 0.0f;

    std::string path = "levels/Act3Level4.json";
    if (!std::filesystem::exists(path))
        path = "VaultHeist/levels/Act3Level4.json";

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTree();
        TreeRenderer::updateNodeMetrics(m_rootNode);
        setInstruction("Predict what rotation is needed when inserting the incoming key!");
        syncWithState();
        if (m_rootNode)
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
    }

    float bx = m_canvasBounds.position.x + 10.0f;
    float by = m_canvasBounds.position.y + m_canvasBounds.size.y - 44.0f;
    m_btnLL.bounds = sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(54, 34));
    m_btnRR.bounds = sf::FloatRect(sf::Vector2f(bx + 60, by), sf::Vector2f(54, 34));
    m_btnLR.bounds = sf::FloatRect(sf::Vector2f(bx + 120, by), sf::Vector2f(54, 34));
    m_btnRL.bounds = sf::FloatRect(sf::Vector2f(bx + 180, by), sf::Vector2f(54, 34));
    m_btnNone.bounds = sf::FloatRect(sf::Vector2f(bx + 240, by), sf::Vector2f(64, 34));
}

void Act3Level4Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setAlarm(state.alarm, state.alarmCap);
    setScore(ScoreSystem::score(state));
    setSteps(state.steps, state.optimalSteps);
    setPenalty(state.penaltyPoints);
    m_rootNode = m_logic.getTree();
    if (m_rootNode)
    {
        TreeRenderer::updateNodeMetrics(m_rootNode);
        m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
    }
}

int Act3Level4Screen::computeBF(TreeNode* node) const
{
    if (!node) return 0;
    int lh = node->left ? node->left->height : 0;
    int rh = node->right ? node->right->height : 0;
    return lh - rh;
}

std::string Act3Level4Screen::getLevelSpecificHint()
{
    int nextKey = m_logic.getCurrentInsertKey();
    return "Incoming key is " + std::to_string(nextKey) + ". Mentally insert it and check if any node's |BF| becomes 2.";
}

ILevelLogic* Act3Level4Screen::getLevelLogic()
{
    return &m_logic;
}

void Act3Level4Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act3Level4Screen::handleEvent(const sf::Event& event)
{
    LevelScreenBase::handleEvent(event);
    if (m_resultModal.active) return;

    m_btnLL.handleEvent(event);
    m_btnRR.handleEvent(event);
    m_btnLR.handleEvent(event);
    m_btnRL.handleEvent(event);
    m_btnNone.handleEvent(event);

    auto handlePrediction = [&](int choice) {
        PlayerAction action;
        action.type = CHOOSE_CASE;
        action.value = choice;

        ActionResult res = m_logic.processAction(action);
        syncWithState();

        if (res.correct)
        {
            m_engine.audio.playCorrect();
            m_correctTimer = 1.0f;
            m_engine.showToast(res.message.empty() ? "ACCURATE PREDICTION!" : res.message, TOAST_CORRECT);

            if (m_logic.isWon())
            {
                const LevelState& state = m_logic.getState();
                int stars = ScoreSystem::stars(state);
                int scr = ScoreSystem::score(state);
                SaveSystem::setWon(3, 4, stars);
                showResultModal(true, stars, state.steps, state.optimalSteps, state.penaltyPoints, scr,
                    [this]() { m_engine.changeScreen(std::make_unique<Act3Level5Screen>(m_engine)); },
                    [this]() { resetLevel(); m_resultModal.active = false; },
                    [this]() { m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine)); });
            }
        }
        else
        {
            m_engine.audio.playWrong();
            m_wrongTimer = 0.5f;
            m_engine.showToast(res.message.empty() ? "PREDICTION FAILED!" : res.message, TOAST_ERROR);
        }
    };

    if (m_btnLL.isClicked()) { m_btnLL.resetClick(); handlePrediction(0); }
    if (m_btnRR.isClicked()) { m_btnRR.resetClick(); handlePrediction(1); }
    if (m_btnLR.isClicked()) { m_btnLR.resetClick(); handlePrediction(2); }
    if (m_btnRL.isClicked()) { m_btnRL.resetClick(); handlePrediction(3); }
    if (m_btnNone.isClicked()) { m_btnNone.resetClick(); handlePrediction(4); }
}

void Act3Level4Screen::update(float dt)
{
    LevelScreenBase::update(dt);
    m_animTimer += dt;
    if (m_correctTimer > 0.0f) m_correctTimer -= dt;
    if (m_wrongTimer > 0.0f)   m_wrongTimer -= dt;
}

void Act3Level4Screen::drawBFBadges(sf::RenderTarget& target, TreeNode* node)
{
    if (!node) return;
    drawBFBadges(target, node->left);
    drawBFBadges(target, node->right);

    int bf = computeBF(node);
    int absBF = bf < 0 ? -bf : bf;
    sf::Color badgeCol = (absBF >= 2) ? Theme::CrimsonAlarm : Theme::BorderCyan;
    std::string bfStr = (bf >= 0 ? "+" : "") + std::to_string(bf);

    sf::Text badge(m_engine.theme.fontMono, bfStr, 10);
    badge.setFillColor(badgeCol);
    badge.setStyle(sf::Text::Bold);
    sf::FloatRect lb = badge.getLocalBounds();
    badge.setPosition(sf::Vector2f(
        node->position.x - lb.size.x * 0.5f - lb.position.x,
        node->position.y - Theme::NodeRadius - 18.0f));
    target.draw(badge);
}

void Act3Level4Screen::onCustomDraw(sf::RenderTarget& target)
{
    if (m_rootNode)
        drawBFBadges(target, m_rootNode);

    m_btnLL.draw(target, m_engine.theme);
    m_btnRR.draw(target, m_engine.theme);
    m_btnLR.draw(target, m_engine.theme);
    m_btnRL.draw(target, m_engine.theme);
    m_btnNone.draw(target, m_engine.theme);

    sf::Text label(m_engine.theme.fontMono, "PREDICT REQUIRED ROTATION:", 11);
    label.setFillColor(Theme::AccentYellow);
    label.setPosition(sf::Vector2f(m_btnLL.bounds.position.x, m_btnLL.bounds.position.y - 18.0f));
    target.draw(label);

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

void Act3Level4Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    int round = m_logic.getCurrentRound();
    int total = m_logic.getTotalRounds();
    int nextKey = m_logic.getCurrentInsertKey();

    sf::Text header(m_engine.theme.fontMono, "PREDICTION RADAR", 12);
    header.setFillColor(Theme::BorderCyan);
    header.setStyle(sf::Text::Bold);
    header.setPosition(sf::Vector2f(px, py));
    target.draw(header);

    std::string sRound = "ROUND: " + std::to_string(round) + " / " + std::to_string(total);
    sf::Text txtRound(m_engine.theme.fontMono, sRound, 12);
    txtRound.setFillColor(Theme::TextPrimary);
    txtRound.setPosition(sf::Vector2f(px, py + 24.0f));
    target.draw(txtRound);

    sf::Text txtKey(m_engine.theme.fontMono, "NEXT KEY: " + std::to_string(nextKey), 14);
    txtKey.setFillColor(Theme::AccentYellow);
    txtKey.setStyle(sf::Text::Bold);
    txtKey.setPosition(sf::Vector2f(px, py + 48.0f));
    target.draw(txtKey);
}
