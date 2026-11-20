#include "Act2Level4Screen.h"
#include "Act2Level5Screen.h"
#include "CampaignScreen.h"
#include "GameEngine.h"
#include <filesystem>
#include <algorithm>

Act2Level4Screen::Act2Level4Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 2: THE SORTED WING", "LEVEL 2.4: RANGE RAID")
    , m_lo(0)
    , m_hi(0)
    , m_showPruningView(false)
    , m_pruningTimer(0.0f)
    , m_btnPruningContinue("CONTINUE TO RESULTS", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(220.0f, 40.0f)), BTN_PRIMARY)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

void Act2Level4Screen::resetLevel()
{
    m_collectedKeys.clear();
    m_showPruningView = false;
    m_pruningTimer = 0.0f;
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;

    std::string path = "Act2Level4.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/Act2Level4.json";
    }

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTree();
        m_lo = m_logic.getLo();
        m_hi = m_logic.getHi();

        setInstruction("Collect all rooms with keys in the range [" + std::to_string(m_lo) + ", " + std::to_string(m_hi) + "] in any order.");
        syncWithState();
        if (m_rootNode)
        {
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
        }
    }
}

void Act2Level4Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);
}

std::string Act2Level4Screen::getLevelSpecificHint()
{
    PlayerAction action;
    action.type = CLICK_HINT;
    ActionResult res = m_logic.processAction(action);
    return res.message;
}

ILevelLogic* Act2Level4Screen::getLevelLogic()
{
    return &m_logic;
}

void Act2Level4Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act2Level4Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_logic.isFinished() || m_showPruningView) return;

    PlayerAction action;
    action.type = CLICK_NODE;
    action.nodeA = node->val;

    ActionResult res = m_logic.processAction(action);
    syncWithState();

    if (res.correct)
    {
        m_collectedKeys.push_back(node->val);
        node->isVisited = true;
        m_engine.audio.playCorrect();

        if (m_logic.isWon())
        {
            m_engine.audio.playWin();
            m_showPruningView = true;
            m_pruningTimer = 2.5f;
        }
    }
    else
    {
        m_shakingNode = node;
        m_shakeTimer = 0.35f;

        m_engine.audio.playWrong();
        m_engine.showToast(res.message, TOAST_ERROR);

        if (m_logic.isFinished() && !m_logic.isWon())
        {
            m_engine.audio.playLose();
            m_showPruningView = true;
            m_pruningTimer = 2.5f;
        }
    }
}

void Act2Level4Screen::handleEvent(const sf::Event& event)
{
    if (m_showPruningView)
    {
        m_btnPruningContinue.handleEvent(event);
        if (m_btnPruningContinue.isClicked())
        {
            m_btnPruningContinue.resetClick();
            m_showPruningView = false;
            const LevelState& s = m_logic.getState();
            if (m_logic.isWon())
            {
                showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
                    [this]() {
                        m_engine.changeScreen(std::make_unique<Act2Level5Screen>(m_engine));
                    },
                    [this]() { resetLevel(); },
                    [this]() { onBackClicked(); }
                );
            }
            else
            {
                showResultModal(false, 0, s.steps, s.optimalSteps, s.penaltyPoints, 0,
                    nullptr,
                    [this]() { resetLevel(); },
                    [this]() { onBackClicked(); }
                );
            }
        }
        return;
    }

    LevelScreenBase::handleEvent(event);

    if (auto* mouseMove = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mpos(static_cast<float>(mouseMove->position.x), static_cast<float>(mouseMove->position.y));
        if (m_rootNode && m_canvasBounds.contains(mpos))
        {
            m_hoverNode = m_treeRenderer.getNodeAtPosition(m_rootNode, canvasToWorld(mouseMove->position));
        }
    }
}

void Act2Level4Screen::update(float dt)
{
    LevelScreenBase::update(dt);

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f) m_shakingNode = nullptr;
    }

    if (m_showPruningView && m_pruningTimer > 0.0f)
    {
        m_pruningTimer -= dt;
        if (m_pruningTimer <= 0.0f)
        {
            // Auto continue to result modal after 2.5s
            m_showPruningView = false;
            const LevelState& s = m_logic.getState();
            if (m_logic.isWon())
            {
                showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
                    [this]() {
                        m_engine.changeScreen(std::make_unique<Act2Level5Screen>(m_engine));
                    },
                    [this]() { resetLevel(); },
                    [this]() { onBackClicked(); }
                );
            }
            else
            {
                showResultModal(false, 0, s.steps, s.optimalSteps, s.penaltyPoints, 0,
                    nullptr,
                    [this]() { resetLevel(); },
                    [this]() { onBackClicked(); }
                );
            }
        }
    }
}

static void drawPrunedSubtree(sf::RenderTarget& target, const Theme& theme, TreeNode* node, Act2Level4& logic)
{
    if (!node) return;
    drawPrunedSubtree(target, theme, node->left, logic);
    drawPrunedSubtree(target, theme, node->right, logic);

    if (logic.isNodeSkipped(node->val))
    {
        // Dark desaturating overlay on skipped node
        sf::CircleShape dim(Theme::NodeRadius);
        dim.setOrigin(sf::Vector2f(Theme::NodeRadius, Theme::NodeRadius));
        dim.setPosition(node->position);
        dim.setFillColor(sf::Color(10, 14, 23, 200));
        dim.setOutlineThickness(1.5f);
        dim.setOutlineColor(sf::Color(80, 95, 110, 150));
        target.draw(dim);

        sf::Text sText(theme.fontMono, "SKIPPED", 8);
        sText.setFillColor(sf::Color(160, 170, 185));
        sf::FloatRect b = sText.getLocalBounds();
        sText.setOrigin(sf::Vector2f(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f));
        sText.setPosition(node->position);
        target.draw(sText);
    }
}

void Act2Level4Screen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    // 1. Shake red ring for invalid clicks
    if (m_shakingNode && m_shakeTimer > 0.0f)
    {
        sf::CircleShape shakeRing(Theme::NodeRadius + 5.0f);
        shakeRing.setOrigin(sf::Vector2f(Theme::NodeRadius + 5.0f, Theme::NodeRadius + 5.0f));
        shakeRing.setPosition(m_shakingNode->position + Tween::shake(m_shakeTimer, 7.0f));
        shakeRing.setFillColor(sf::Color::Transparent);
        shakeRing.setOutlineThickness(3.0f);
        shakeRing.setOutlineColor(Theme::CrimsonAlarm);
        target.draw(shakeRing);
    }

    // 2. Post-game Pruning View
    if (m_showPruningView)
    {
        drawPrunedSubtree(target, theme, m_rootNode, m_logic);

        // One-line telemetry card at bottom
        float cw = m_canvasBounds.size.x * 0.8f;
        float ch = 65.0f;
        float cx = m_canvasBounds.position.x + (m_canvasBounds.size.x - cw) * 0.5f;
        float cy = m_canvasBounds.position.y + m_canvasBounds.size.y - ch - 20.0f;

        sf::RectangleShape banner(sf::Vector2f(cw, ch));
        banner.setPosition(sf::Vector2f(cx, cy));
        banner.setFillColor(sf::Color(13, 19, 33, 235));
        banner.setOutlineThickness(1.5f);
        banner.setOutlineColor(Theme::BorderCyan);
        target.draw(banner);

        int visitedCount = m_logic.getVisitedByPruningCount();
        std::string infoStr = "PRUNING ANALYSIS: Visited " + std::to_string(visitedCount) + " rooms.\nSubtrees outside [" + std::to_string(m_lo) + ".." + std::to_string(m_hi) + "] skipped!";
        sf::Text txt(theme.fontMono, infoStr, 12);
        txt.setFillColor(Theme::AccentYellow);
        txt.setPosition(sf::Vector2f(cx + 20.0f, cy + 14.0f));
        target.draw(txt);

        m_btnPruningContinue.bounds = sf::FloatRect(sf::Vector2f(cx + cw - 210.0f, cy + 12.0f), sf::Vector2f(195.0f, 40.0f));
        m_btnPruningContinue.draw(target, theme);
    }

    // Tooltip according to Rule 7
    if (!m_showPruningView)
    {
        TreeRenderer::drawNodeTooltip(target, theme, m_rootNode, m_hoverNode, false);
    }
}

void Act2Level4Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;
    float pw = panelBounds.size.x;

    sf::Text hdr(theme.fontMono, "RANGE TELEMETRY", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    // Range Card
    sf::RectangleShape card(sf::Vector2f(pw - 8.0f, 60.0f));
    card.setPosition(sf::Vector2f(px + 4.0f, py + 26.0f));
    card.setFillColor(sf::Color(17, 24, 39, 230));
    card.setOutlineThickness(1.2f);
    card.setOutlineColor(Theme::BorderCyan);
    target.draw(card);

    std::string sRange = "TARGET RANGE:\n[ " + std::to_string(m_lo) + "  <--->  " + std::to_string(m_hi) + " ]";
    sf::Text tRange(theme.fontMono, sRange, 13);
    tRange.setFillColor(Theme::AccentYellow);
    tRange.setStyle(sf::Text::Bold);
    tRange.setPosition(sf::Vector2f(px + 14.0f, py + 34.0f));
    target.draw(tRange);

    int col = m_logic.getCollectedCount();
    int tot = m_logic.getTotalToCollect();
    std::string sCount = "COLLECTED: " + std::to_string(col) + " / " + std::to_string(tot);
    sf::Text tCount(theme.fontMono, sCount, 13);
    tCount.setFillColor(Theme::SuccessEmerald);
    tCount.setStyle(sf::Text::Bold);
    tCount.setPosition(sf::Vector2f(px + 4.0f, py + 98.0f));
    target.draw(tCount);

    sf::Text tHint(theme.fontMono, "Click all rooms with keys\ninside the target range.\nUnvisited nodes stay secret!", 11);
    tHint.setFillColor(Theme::TextMuted);
    tHint.setPosition(sf::Vector2f(px + 4.0f, py + 128.0f));
    target.draw(tHint);
}
