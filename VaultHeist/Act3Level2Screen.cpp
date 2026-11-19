#include "Act3Level2Screen.h"
#include "Act3Level3Screen.h"
#include "CampaignScreen.h"
#include "GameEngine.h"
#include "SaveSystem.h"
#include "ScoreSystem.h"
#include "TreeRenderer.h"
#include <filesystem>
#include <cmath>
#include <cstdint>

Act3Level2Screen::Act3Level2Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 3: THE ARCHITECT'S WING", "LEVEL 3.2: MANUAL REPAIR - SINGLE")
    , m_btnLL("LL", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(60, 34)), BTN_SECONDARY)
    , m_btnRR("RR", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(60, 34)), BTN_SECONDARY)
    , m_btnLR("LR", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(60, 34)), BTN_SECONDARY)
    , m_btnRL("RL", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(60, 34)), BTN_SECONDARY)
    , m_selectedNode(nullptr)
    , m_wrongNode(nullptr)
    , m_wrongTimer(0.0f)
    , m_correctNode(nullptr)
    , m_correctTimer(0.0f)
    , m_animTimer(0.0f)
{
    resetLevel();
}

void Act3Level2Screen::resetLevel()
{
    m_selectedNode = nullptr;
    m_wrongNode = nullptr;
    m_wrongTimer = 0.0f;
    m_correctNode = nullptr;
    m_correctTimer = 0.0f;

    std::string path = "Act3Level2.json";
    if (!std::filesystem::exists(path))
        path = "VaultHeist/Act3Level2.json";

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTree();
        TreeRenderer::updateNodeMetrics(m_rootNode);
        setInstruction("Phase 1: Click the unbalanced AVL node (|BF|>=2). Phase 2: Select the rotation case.");
        syncWithState();
        if (m_rootNode)
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
    }

    float bx = m_canvasBounds.position.x + 10.0f;
    float by = m_canvasBounds.position.y + m_canvasBounds.size.y - 44.0f;
    m_btnLL.bounds = sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(60, 34));
    m_btnRR.bounds = sf::FloatRect(sf::Vector2f(bx + 66, by), sf::Vector2f(60, 34));
    m_btnLR.bounds = sf::FloatRect(sf::Vector2f(bx + 132, by), sf::Vector2f(60, 34));
    m_btnRL.bounds = sf::FloatRect(sf::Vector2f(bx + 198, by), sf::Vector2f(60, 34));
}

void Act3Level2Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setAlarm(state.alarm, state.alarmCap);
    setScore(ScoreSystem::score(state));
    setSteps(state.steps, state.optimalSteps);
    setPenalty(state.penaltyPoints);
}

int Act3Level2Screen::computeBF(TreeNode* node) const
{
    if (!node) return 0;
    int lh = node->left ? node->left->height : 0;
    int rh = node->right ? node->right->height : 0;
    return lh - rh;
}

void Act3Level2Screen::onHintClicked()
{
    int phase = m_logic.getPhase();
    if (phase == 1)
        m_engine.showToast("Phase 1: Click the node where |BF| = 2 (shown in red).", TOAST_HINT);
    else
        m_engine.showToast("Phase 2: LL=left heavy child left heavy. RR=right heavy. LR/RL=zigzag cases.", TOAST_HINT);
}

void Act3Level2Screen::onResetClicked()
{
    m_engine.showConfirmModal("RESET LEVEL", "Reset this level?",
        [this]() { resetLevel(); });
}

void Act3Level2Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_resultModal.active || m_logic.getPhase() != 1) return;

    PlayerAction action;
    action.type = CLICK_NODE;
    action.nodeA = node->val;

    ActionResult res = m_logic.processAction(action);
    syncWithState();

    if (res.correct)
    {
        m_engine.audio.playCorrect();
        m_selectedNode = node;
        m_correctNode = node;
        m_correctTimer = 1.5f;
        m_engine.showToast(res.message.empty() ? "IMBALANCE FOUND - SELECT ROTATION CASE" : res.message, TOAST_CORRECT);
        setInstruction("Phase 2: Select the rotation case - LL, RR, LR, or RL");
    }
    else
    {
        m_engine.audio.playWrong();
        m_wrongNode = node;
        m_wrongTimer = 0.5f;
        m_engine.showToast(res.message.empty() ? "WRONG NODE - Find the unbalanced one!" : res.message, TOAST_ERROR);
    }
}

void Act3Level2Screen::handleEvent(const sf::Event& event)
{
    LevelScreenBase::handleEvent(event);

    if (m_resultModal.active || m_logic.getPhase() != 2) return;

    m_btnLL.handleEvent(event);
    m_btnRR.handleEvent(event);
    m_btnLR.handleEvent(event);
    m_btnRL.handleEvent(event);

    auto handleCase = [&](int caseVal)
    {
        PlayerAction action;
        action.type = CHOOSE_CASE;
        action.value = caseVal;
        ActionResult res = m_logic.processAction(action);
        syncWithState();

        if (res.correct)
        {
            m_engine.audio.playCorrect();
            m_engine.showToast(res.message.empty() ? "CORRECT ROTATION CASE!" : res.message, TOAST_CORRECT);

            if (m_logic.isWon())
            {
                const LevelState& state = m_logic.getState();
                int stars = ScoreSystem::stars(state);
                int scr = ScoreSystem::score(state);
                SaveSystem::setWon(3, 2, stars);
                showResultModal(true, stars, state.steps, state.optimalSteps, state.penaltyPoints, scr,
                    [this]() { m_engine.changeScreen(std::make_unique<Act3Level3Screen>(m_engine)); },
                    [this]() { resetLevel(); m_resultModal.active = false; },
                    [this]() { m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine)); });
            }
        }
        else
        {
            m_engine.audio.playWrong();
            m_wrongTimer = 0.5f;
            m_engine.showToast(res.message.empty() ? "WRONG CASE - Penalty applied!" : res.message, TOAST_ERROR);
        }
    };

    if (m_btnLL.isClicked()) { m_btnLL.resetClick(); handleCase(0); }
    if (m_btnRR.isClicked()) { m_btnRR.resetClick(); handleCase(1); }
    if (m_btnLR.isClicked()) { m_btnLR.resetClick(); handleCase(2); }
    if (m_btnRL.isClicked()) { m_btnRL.resetClick(); handleCase(3); }
}

void Act3Level2Screen::update(float dt)
{
    LevelScreenBase::update(dt);
    m_animTimer += dt;
    if (m_correctTimer > 0.0f) m_correctTimer -= dt;
    if (m_wrongTimer > 0.0f)   m_wrongTimer -= dt;
}

void Act3Level2Screen::drawBFBadges(sf::RenderTarget& target, TreeNode* node)
{
    if (!node) return;
    drawBFBadges(target, node->left);
    drawBFBadges(target, node->right);

    int bf = computeBF(node);
    int absBF = bf < 0 ? -bf : bf;
    sf::Color badgeCol = (absBF >= 2) ? Theme::CrimsonAlarm : Theme::BorderCyan;
    std::string bfStr = (bf >= 0 ? "+" : "") + std::to_string(bf);
    if (absBF >= 2) bfStr += "!";

    sf::Text badge(m_engine.theme.fontMono, bfStr, 10);
    badge.setFillColor(badgeCol);
    badge.setStyle(sf::Text::Bold);
    sf::FloatRect lb = badge.getLocalBounds();
    badge.setPosition(sf::Vector2f(
        node->position.x - lb.size.x * 0.5f - lb.position.x,
        node->position.y - Theme::NodeRadius - 18.0f));
    target.draw(badge);

    if (absBF >= 2)
    {
        float pulse = 0.5f + 0.5f * std::sin(m_animTimer * 4.0f);
        sf::CircleShape ring(Theme::NodeRadius + 5.0f + pulse * 3.0f);
        ring.setOrigin(sf::Vector2f(ring.getRadius(), ring.getRadius()));
        ring.setPosition(node->position);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(2.0f);
        ring.setOutlineColor(sf::Color(220, 60, 60, static_cast<uint8_t>(160 + 80 * pulse)));
        target.draw(ring);
    }
}

void Act3Level2Screen::onCustomDraw(sf::RenderTarget& target)
{
    if (m_rootNode)
        drawBFBadges(target, m_rootNode);

    if (m_logic.getPhase() == 2)
    {
        m_btnLL.draw(target, m_engine.theme);
        m_btnRR.draw(target, m_engine.theme);
        m_btnLR.draw(target, m_engine.theme);
        m_btnRL.draw(target, m_engine.theme);

        sf::Text label(m_engine.theme.fontMono, "SELECT ROTATION CASE:", 11);
        label.setFillColor(Theme::AccentYellow);
        label.setPosition(sf::Vector2f(m_btnLL.bounds.position.x, m_btnLL.bounds.position.y - 18.0f));
        target.draw(label);
    }

    if (m_selectedNode && m_logic.getPhase() == 2)
    {
        float pulse = 0.5f + 0.5f * std::sin(m_animTimer * 3.0f);
        sf::CircleShape sel(Theme::NodeRadius + 7.0f + pulse * 2.0f);
        sel.setOrigin(sf::Vector2f(sel.getRadius(), sel.getRadius()));
        sel.setPosition(m_selectedNode->position);
        sel.setFillColor(sf::Color::Transparent);
        sel.setOutlineThickness(2.5f);
        sel.setOutlineColor(sf::Color(0, 220, 255, static_cast<uint8_t>(200 * pulse)));
        target.draw(sel);
    }

    if (m_correctNode && m_correctTimer > 0.0f)
    {
        float alpha = m_correctTimer / 1.5f;
        sf::CircleShape glow(Theme::NodeRadius + 8.0f);
        glow.setOrigin(sf::Vector2f(glow.getRadius(), glow.getRadius()));
        glow.setPosition(m_correctNode->position);
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(3.0f);
        glow.setOutlineColor(sf::Color(0, 255, 100, static_cast<uint8_t>(200 * alpha)));
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

void Act3Level2Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    int phase = m_logic.getPhase();
    std::string phaseStr = "PHASE " + std::to_string(phase) + ": " + (phase == 1 ? "SELECT NODE" : "SELECT ROTATION");
    sf::Text phaseLabel(m_engine.theme.fontMono, phaseStr, 12);
    phaseLabel.setFillColor(phase == 1 ? Theme::AccentYellow : Theme::BorderCyan);
    phaseLabel.setStyle(sf::Text::Bold);
    phaseLabel.setPosition(sf::Vector2f(px, py));
    target.draw(phaseLabel);

    if (phase == 2)
    {
        static const char* cases[4] = { "LL: Left-Left", "RR: Right-Right", "LR: Left-Right", "RL: Right-Left" };
        for (int c = 0; c < 4; c++)
        {
            sf::Text ct(m_engine.theme.fontMono, cases[c], 11);
            ct.setFillColor(Theme::TextMuted);
            ct.setPosition(sf::Vector2f(px, py + 24.0f + c * 18.0f));
            target.draw(ct);
        }
    }
}
