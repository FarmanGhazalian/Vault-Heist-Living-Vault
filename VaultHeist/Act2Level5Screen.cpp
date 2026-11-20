#include "Act2Level5Screen.h"
#include "Act3Level1Screen.h"
#include "CampaignScreen.h"
#include "GameEngine.h"
#include "SaveSystem.h"
#include <filesystem>
#include <cmath>

Act2Level5Screen::Act2Level5Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 2: THE SORTED WING", "LEVEL 2.5: THE LAZY ARCHITECT")
    , m_currentTarget(0)
    , m_initialCap(10)
    , m_lastCapacity(10)
    , m_capacityPulseTimer(0.0f)
    , m_showCutscene(false)
    , m_btnCutsceneContinue("CONTINUE TO SUMMARY", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(220.0f, 38.0f)), BTN_PRIMARY)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_pulseTimer(0.0f)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

void Act2Level5Screen::resetLevel()
{
    m_visitedPath.clear();
    m_showCutscene = false;
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;

    std::string path = "Act2Level5.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/Act2Level5.json";
    }

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTreeRoot();
        if (m_rootNode)
        {
            // Clear any lingering visited highlights on tree nodes
            std::function<void(TreeNode*)> clearVisited = [&](TreeNode* n) {
                if (!n) return;
                n->isVisited = false;
                clearVisited(n->left);
                clearVisited(n->right);
            };
            clearVisited(m_rootNode);
        }

        m_currentTarget = m_logic.getCurrentTarget();
        m_initialCap = m_logic.getCapacityLeft();
        m_lastCapacity = m_initialCap;

        setInstruction("Boss: Search targets in degenerate corridor tree before capacity depletes!");
        syncWithState();

        if (m_rootNode)
        {
            // Set auto-fit layout scaling for long right-skewed corridor
            m_treeRenderer.setVerticalSpacing(45.0f);
            m_treeRenderer.setMinHorizontalSpacing(28.0f);
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
        }
    }
}

void Act2Level5Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);
    m_currentTarget = m_logic.getCurrentTarget();

    int curCap = m_logic.getCapacityLeft();
    if (curCap < m_lastCapacity)
    {
        m_lastCapacity = curCap;
        m_capacityPulseTimer = 0.5f;
    }
}

std::string Act2Level5Screen::getLevelSpecificHint()
{
    PlayerAction action;
    action.type = CLICK_HINT;
    ActionResult res = m_logic.processAction(action);
    return res.message;
}

ILevelLogic* Act2Level5Screen::getLevelLogic()
{
    return &m_logic;
}

void Act2Level5Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act2Level5Screen::reapplyActionSilently(const PlayerAction& action)
{
    ActionResult res = m_logic.processAction(action);
    if (res.correct)
    {
        m_visitedPath.push_back(action.nodeA);
        if (m_rootNode)
        {
            std::function<TreeNode*(TreeNode*, int)> findN = [&](TreeNode* curr, int val) -> TreeNode* {
                if (!curr) return nullptr;
                if (curr->val == val) return curr;
                TreeNode* l = findN(curr->left, val);
                if (l) return l;
                return findN(curr->right, val);
            };
            TreeNode* n = findN(m_rootNode, action.nodeA);
            if (n) n->isVisited = true;
        }
        if (action.nodeA == m_currentTarget)
        {
            m_visitedPath.clear();
            if (m_rootNode)
            {
                std::function<void(TreeNode*)> clearVisited = [&](TreeNode* n) {
                    if (!n) return;
                    n->isVisited = false;
                    clearVisited(n->left);
                    clearVisited(n->right);
                };
                clearVisited(m_rootNode);
            }
        }
    }
    syncWithState();
}

void Act2Level5Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_logic.isFinished() || m_showCutscene) return;

    PlayerAction action;
    action.type = CLICK_NODE;
    action.nodeA = node->val;

    ActionResult res = m_logic.processAction(action);
    syncWithState();

    if (res.correct)
    {
        recordSharedAction(action);
        m_visitedPath.push_back(node->val);
        node->isVisited = true;
        m_engine.audio.playCorrect();

        if (node->val == m_currentTarget)
        {
            m_visitedPath.clear();
            if (m_rootNode)
            {
                std::function<void(TreeNode*)> clearVisited = [&](TreeNode* n) {
                    if (!n) return;
                    n->isVisited = false;
                    clearVisited(n->left);
                    clearVisited(n->right);
                };
                clearVisited(m_rootNode);
            }
        }

        if (m_logic.isWon())
        {
            m_engine.audio.playWin();
            m_showCutscene = true;
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
            m_showCutscene = true;
        }
    }
}

void Act2Level5Screen::handleEvent(const sf::Event& event)
{
    if (m_showCutscene)
    {
        m_btnCutsceneContinue.handleEvent(event);
        if (m_btnCutsceneContinue.isClicked())
        {
            m_btnCutsceneContinue.resetClick();
            m_showCutscene = false;

            const LevelState& s = m_logic.getState();
            if (m_logic.isWon())
            {
                int stars = ScoreSystem::stars(s);
                SaveSystem::setWon(2, 5, stars);
                showResultModal(true, stars, s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
                    [this]() { m_engine.changeScreen(std::make_unique<Act3Level1Screen>(m_engine)); },
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
            m_hoverNode = m_treeRenderer.getNodeAtPosition(m_rootNode, mpos);
        }
    }
}

void Act2Level5Screen::update(float dt)
{
    LevelScreenBase::update(dt);
    m_pulseTimer += dt;

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f) m_shakingNode = nullptr;
    }

    if (m_capacityPulseTimer > 0.0f)
    {
        m_capacityPulseTimer -= dt;
    }
}

static TreeNode* findBstNode(TreeNode* root, int val)
{
    if (!root) return nullptr;
    if (root->val == val) return root;
    if (val < root->val) return findBstNode(root->left, val);
    return findBstNode(root->right, val);
}

void Act2Level5Screen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    // 1. Draw verified search path edges
    if (m_visitedPath.size() >= 2)
    {
        for (size_t i = 0; i + 1 < m_visitedPath.size(); ++i)
        {
            TreeNode* n1 = findBstNode(m_rootNode, m_visitedPath[i]);
            TreeNode* n2 = findBstNode(m_rootNode, m_visitedPath[i + 1]);
            if (n1 && n2)
            {
                sf::Vertex line[4] = {
                    sf::Vertex(n1->position + sf::Vector2f(-2.0f, -2.0f), Theme::SuccessEmerald),
                    sf::Vertex(n2->position + sf::Vector2f(-2.0f, -2.0f), Theme::SuccessEmerald),
                    sf::Vertex(n2->position + sf::Vector2f(2.0f, 2.0f), Theme::SuccessEmerald),
                    sf::Vertex(n1->position + sf::Vector2f(2.0f, 2.0f), Theme::SuccessEmerald)
                };
                target.draw(line, 4, sf::PrimitiveType::TriangleStrip);
            }
        }
    }

    // 2. Pulsing target indicator
    TreeNode* targetNode = findBstNode(m_rootNode, m_currentTarget);
    if (targetNode)
    {
        float pulseScale = 1.0f + 0.15f * std::sin(m_pulseTimer * 4.0f);
        float ringRadius = (Theme::NodeRadius + 6.0f) * pulseScale;
        sf::CircleShape goldRing(ringRadius);
        goldRing.setOrigin(sf::Vector2f(ringRadius, ringRadius));
        goldRing.setPosition(targetNode->position);
        goldRing.setFillColor(sf::Color::Transparent);
        goldRing.setOutlineThickness(2.5f);
        goldRing.setOutlineColor(Theme::AccentYellow);
        target.draw(goldRing);
    }

    // 3. Shake red ring for invalid moves
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

    // 4. Cutscene Card Overlay
    if (m_showCutscene)
    {
        sf::Vector2f wSize(static_cast<float>(target.getSize().x), static_cast<float>(target.getSize().y));
        sf::RectangleShape backdrop(wSize);
        backdrop.setFillColor(sf::Color(10, 14, 23, 220));
        target.draw(backdrop);

        float cardW = std::min(540.0f, wSize.x * 0.88f);
        float cardH = 340.0f;
        float cardX = (wSize.x - cardW) * 0.5f;
        float cardY = (wSize.y - cardH) * 0.5f;

        sf::FloatRect cardBounds(sf::Vector2f(cardX, cardY), sf::Vector2f(cardW, cardH));
        theme.drawPanel(target, cardBounds, Theme::BorderCyan, 1.5f);
        theme.drawGlowOutline(target, cardBounds, Theme::BorderCyan, 0.8f);

        sf::Text header(theme.fontUI, "DEBRIEF: THE SKEWED ARCHITECT", 18);
        header.setFillColor(Theme::BorderCyan);
        header.setStyle(sf::Text::Bold);
        header.setPosition(sf::Vector2f(cardX + 24.0f, cardY + 20.0f));
        target.draw(header);

        int n = m_logic.getHeight();
        int logN = (n > 1) ? static_cast<int>(std::round(std::log2(static_cast<double>(n)))) : 1;

        std::string debriefText = 
            "Vault analysis reveals why tree balancing is paramount:\n"
            "Sorted insertions created a degraded single-corridor BST with Height N = " + std::to_string(n) + ".\n"
            "Each lookup degraded to O(N) linear scan instead of optimal O(log2 N) = ~" + std::to_string(logN) + " operations!\n\n"
            "Self-balancing structures (AVL Rotations in Act 3) enforce strict balance factors to restore logarithmic security bounds.";

        float textW = cardW - 48.0f;
        UIText::drawWrapped(target, theme.fontUI, debriefText, 13, Theme::TextPrimary, sf::Vector2f(cardX + 24.0f, cardY + 60.0f), textW);

        m_btnCutsceneContinue.bounds = sf::FloatRect(sf::Vector2f(cardX + (cardW - 220.0f) * 0.5f, cardY + cardH - 52.0f), sf::Vector2f(220.0f, 38.0f));
        m_btnCutsceneContinue.draw(target, theme);
    }

    // Tooltip according to Rule 7
    if (!m_showCutscene)
    {
        TreeRenderer::drawNodeTooltip(target, theme, m_rootNode, m_hoverNode, false);
    }
}

void Act2Level5Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;
    float pw = panelBounds.size.x;

    sf::Text hdr(theme.fontMono, "BOSS TELEMETRY", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    std::string sHeight = "TREE HEIGHT: " + std::to_string(m_logic.getHeight()) + " (CORRIDOR)";
    sf::Text tHeight(theme.fontMono, sHeight, 13);
    tHeight.setFillColor(Theme::CrimsonAlarm);
    tHeight.setStyle(sf::Text::Bold);
    tHeight.setPosition(sf::Vector2f(px + 4.0f, py + 26.0f));
    target.draw(tHeight);

    std::string sTarget = "CURRENT TARGET: Room " + std::to_string(m_currentTarget);
    sf::Text tTarget(theme.fontMono, sTarget, 12);
    tTarget.setFillColor(Theme::AccentYellow);
    tTarget.setPosition(sf::Vector2f(px + 4.0f, py + 52.0f));
    target.draw(tTarget);

    int curCap = m_logic.getCapacityLeft();
    std::string sCap = "SHRINKING CAPACITY: " + std::to_string(curCap);
    sf::Text tCap(theme.fontMono, sCap, 12);
    tCap.setFillColor(m_capacityPulseTimer > 0.0f ? Theme::CrimsonAlarm : Theme::TextPrimary);
    tCap.setStyle(sf::Text::Bold);
    tCap.setPosition(sf::Vector2f(px + 4.0f, py + 78.0f));
    target.draw(tCap);

    // Shrinking capacity bar
    float barW = pw - 8.0f;
    float barH = 12.0f;
    sf::RectangleShape barBg(sf::Vector2f(barW, barH));
    barBg.setPosition(sf::Vector2f(px + 4.0f, py + 100.0f));
    barBg.setFillColor(sf::Color(10, 14, 23, 200));
    barBg.setOutlineThickness(1.0f);
    barBg.setOutlineColor(Theme::BorderMuted);
    target.draw(barBg);

    float denom = (m_initialCap > 0) ? static_cast<float>(m_initialCap) : 10.0f;
    float capPct = std::clamp(static_cast<float>(curCap) / denom, 0.0f, 1.0f);
    if (capPct > 0.0f)
    {
        sf::RectangleShape barFill(sf::Vector2f(barW * capPct, barH));
        barFill.setPosition(sf::Vector2f(px + 4.0f, py + 100.0f));
        barFill.setFillColor((m_capacityPulseTimer > 0.0f) ? Theme::CrimsonAlarm : Theme::AccentYellow);
        target.draw(barFill);
    }
}
