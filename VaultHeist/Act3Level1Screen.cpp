#include "Act3Level1Screen.h"
#include "Act3Level2Screen.h"
#include "CampaignScreen.h"
#include "GameEngine.h"
#include "SaveSystem.h"
#include "ScoreSystem.h"
#include "TreeRenderer.h"
#include <filesystem>
#include <cmath>
#include <cstdint>
#include <algorithm>

static int computeTreeDepth(TreeNode* node)
{
    if (!node) return 0;
    int l = computeTreeDepth(node->left);
    int r = computeTreeDepth(node->right);
    return 1 + (l > r ? l : r);
}

Act3Level1Screen::Act3Level1Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 3: THE ARCHITECT'S WING", "LEVEL 3.1: READ THE BADGES")
    , m_correctNode(nullptr)
    , m_correctTimer(0.0f)
    , m_wrongNode(nullptr)
    , m_wrongTimer(0.0f)
    , m_rippleTimer(0.0f)
    , m_rippleActive(false)
    , m_maxDepth(0)
    , m_foundCount(0)
    , m_totalToFind(0)
{
    resetLevel();
}

void Act3Level1Screen::resetLevel()
{
    m_correctNode = nullptr;
    m_correctTimer = 0.0f;
    m_wrongNode = nullptr;
    m_wrongTimer = 0.0f;

    std::string path = "Act3Level1.json";
    if (!std::filesystem::exists(path))
        path = "VaultHeist/Act3Level1.json";

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTree();
        m_foundCount = m_logic.getFoundCount();
        m_totalToFind = m_logic.getTotalToFind();
        TreeRenderer::updateNodeMetrics(m_rootNode);
        m_maxDepth = computeTreeDepth(m_rootNode);

        m_rippleTimer = 0.0f;
        m_rippleActive = true;

        setInstruction("Click all unbalanced AVL nodes where |Balance Factor| >= 2. Balance Factor shown on each node.");
        syncWithState();

        if (m_rootNode)
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
    }
}

void Act3Level1Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setAlarm(state.alarm, state.alarmCap);
    setScore(ScoreSystem::score(state));
    setSteps(state.steps, state.optimalSteps);
    setPenalty(state.penaltyPoints);
    m_foundCount = m_logic.getFoundCount();
    m_totalToFind = m_logic.getTotalToFind();
}

int Act3Level1Screen::computeBF(TreeNode* node) const
{
    if (!node) return 0;
    int lh = node->left ? node->left->height : 0;
    int rh = node->right ? node->right->height : 0;
    return lh - rh;
}

void Act3Level1Screen::onHintClicked()
{
    m_engine.showToast("Look for nodes with |BF| = 2. BF = left_height - right_height.", TOAST_HINT);
}

void Act3Level1Screen::onResetClicked()
{
    m_engine.showConfirmModal("RESET LEVEL", "Reset this level? All progress will be lost.",
        [this]() { resetLevel(); });
}

void Act3Level1Screen::onNodeClicked(TreeNode* node)
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
        m_correctTimer = 1.2f;
        m_engine.showToast(res.message.empty() ? "IMBALANCE ISOLATED" : res.message, TOAST_CORRECT);

        if (m_logic.isWon())
        {
            const LevelState& state = m_logic.getState();
            int stars = ScoreSystem::stars(state);
            int scr = ScoreSystem::score(state);
            SaveSystem::setWon(3, 1, stars);
            showResultModal(true, stars, state.steps, state.optimalSteps, state.penaltyPoints, scr,
                [this]() { m_engine.changeScreen(std::make_unique<Act3Level2Screen>(m_engine)); },
                [this]() { resetLevel(); m_resultModal.active = false; },
                [this]() { m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine)); });
        }
    }
    else
    {
        m_engine.audio.playWrong();
        m_wrongNode = node;
        m_wrongTimer = 0.5f;
        m_engine.showToast(res.message.empty() ? "BALANCED NODE - NOT A TARGET" : res.message, TOAST_ERROR);
    }
}

void Act3Level1Screen::handleEvent(const sf::Event& event)
{
    LevelScreenBase::handleEvent(event);
}

void Act3Level1Screen::update(float dt)
{
    LevelScreenBase::update(dt);

    if (m_correctTimer > 0.0f) m_correctTimer -= dt;
    if (m_wrongTimer > 0.0f) m_wrongTimer -= dt;
    if (m_rippleActive)
    {
        m_rippleTimer += dt;
        if (m_rippleTimer > static_cast<float>(m_maxDepth) * 0.3f + 0.5f)
            m_rippleActive = false;
    }
}

void Act3Level1Screen::drawBFBadges(sf::RenderTarget& target, TreeNode* node)
{
    if (!node) return;
    drawBFBadges(target, node->left);
    drawBFBadges(target, node->right);

    int bf = computeBF(node);
    int absBF = bf < 0 ? -bf : bf;

    bool visible = !m_rippleActive;
    if (m_rippleActive)
    {
        int nodeDepth = m_maxDepth - node->height + 1;
        if (nodeDepth < 1) nodeDepth = 1;
        float visibleAt = static_cast<float>(m_maxDepth - nodeDepth) * 0.3f;
        visible = m_rippleTimer >= visibleAt;
    }

    if (!visible) return;

    sf::Color badgeCol = (absBF >= 2) ? Theme::CrimsonAlarm : Theme::BorderCyan;
    if (absBF == 1) badgeCol = sf::Color(100, 200, 100, 220);

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
        float pulse = 0.5f + 0.5f * std::sin(m_rippleTimer * 4.0f);
        sf::CircleShape ring(Theme::NodeRadius + 5.0f + pulse * 3.0f);
        ring.setOrigin(sf::Vector2f(ring.getRadius(), ring.getRadius()));
        ring.setPosition(node->position);
        ring.setFillColor(sf::Color::Transparent);
        ring.setOutlineThickness(2.0f);
        ring.setOutlineColor(sf::Color(220, 60, 60, static_cast<uint8_t>(160 + 80 * pulse)));
        target.draw(ring);
    }
}

void Act3Level1Screen::onCustomDraw(sf::RenderTarget& target)
{
    if (m_rootNode)
    {
        drawBFBadges(target, m_rootNode);
    }

    if (m_correctNode && m_correctTimer > 0.0f)
    {
        float alpha = m_correctTimer / 1.2f;
        sf::CircleShape glow(Theme::NodeRadius + 8.0f);
        glow.setOrigin(sf::Vector2f(glow.getRadius(), glow.getRadius()));
        glow.setPosition(m_correctNode->position);
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(3.0f);
        glow.setOutlineColor(sf::Color(0, 255, 100, static_cast<uint8_t>(200 * alpha)));
        target.draw(glow);
    }

    if (m_wrongNode && m_wrongTimer > 0.0f)
    {
        float alpha = m_wrongTimer / 0.5f;
        sf::CircleShape flash(Theme::NodeRadius + 6.0f);
        flash.setOrigin(sf::Vector2f(flash.getRadius(), flash.getRadius()));
        float shakeX = std::sin(m_wrongTimer * 50.0f) * 4.0f * alpha;
        flash.setPosition(m_wrongNode->position + sf::Vector2f(shakeX, 0.0f));
        flash.setFillColor(sf::Color(255, 50, 50, static_cast<uint8_t>(120 * alpha)));
        flash.setOutlineThickness(2.0f);
        flash.setOutlineColor(sf::Color(255, 50, 50, static_cast<uint8_t>(200 * alpha)));
        target.draw(flash);
    }

    if (m_rootNode)
    {
        TreeRenderer::drawNodeTooltip(target, m_engine.theme, m_rootNode, nullptr);
    }
}

void Act3Level1Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    sf::Text header(m_engine.theme.fontMono, "BALANCE TRACKER", 11);
    header.setFillColor(Theme::BorderCyan);
    header.setStyle(sf::Text::Bold);
    header.setPosition(sf::Vector2f(px, py));
    target.draw(header);

    std::string progress = "Found: " + std::to_string(m_foundCount) + " / " + std::to_string(m_totalToFind);
    sf::Text prog(m_engine.theme.fontMono, progress, 13);
    prog.setFillColor(Theme::AccentYellow);
    prog.setPosition(sf::Vector2f(px, py + 22.0f));
    target.draw(prog);

    sf::Text legend1(m_engine.theme.fontMono, "RED badge = |BF| >= 2 (CLICK!)", 11);
    legend1.setFillColor(Theme::CrimsonAlarm);
    legend1.setPosition(sf::Vector2f(px, py + 50.0f));
    target.draw(legend1);

    sf::Text legend2(m_engine.theme.fontMono, "CYAN badge = balanced", 11);
    legend2.setFillColor(Theme::BorderCyan);
    legend2.setPosition(sf::Vector2f(px, py + 68.0f));
    target.draw(legend2);
}
