#include "Act1Level1Screen.h"
#include "Act1Level2Screen.h"
#include "GameEngine.h"
#include <filesystem>

Act1Level1Screen::Act1Level1Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 1: THE BLUEPRINT WING", "LEVEL 1.1: VAULT PATROL")
    , m_stepCounter(0)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_shakeOffset(0.0f, 0.0f)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

void Act1Level1Screen::resetLevel()
{
    m_visitedOrder.clear();
    m_stepCounter = 0;
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;

    std::string path = "Act1Level1.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/Act1Level1.json";
    }

    if (m_bridge.loadLevel(1, 1, path))
    {
        m_rootNode = m_bridge.getRootNode();
        setInstruction(m_bridge.getObjective());
        syncWithState();
        if (m_rootNode)
        {
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
        }
    }
}

void Act1Level1Screen::syncWithState()
{
    const LevelState& state = m_bridge.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);
}

std::string Act1Level1Screen::getLevelSpecificHint()
{
    return m_bridge.getHint();
}

ILevelLogic* Act1Level1Screen::getLevelLogic()
{
    return m_bridge.getLogic();
}

void Act1Level1Screen::reapplyActionSilently(const PlayerAction& action)
{
    TreeNode* target = nullptr;
    std::function<TreeNode*(TreeNode*, int)> findN = [&](TreeNode* curr, int val) -> TreeNode* {
        if (!curr) return nullptr;
        if (curr->val == val) return curr;
        TreeNode* l = findN(curr->left, val);
        if (l) return l;
        return findN(curr->right, val);
    };
    target = findN(m_rootNode, action.nodeA);
    if (target)
    {
        ActionResult res = m_bridge.processAction(action);
        if (res.correct)
        {
            m_stepCounter++;
            m_visitedOrder[target->val] = m_stepCounter;
            target->isVisited = true;
        }
        syncWithState();
    }
}

void Act1Level1Screen::reapplyActionAnimated(const PlayerAction& action)
{
    reapplyActionSilently(action);
    m_engine.audio.playCorrect();
}

void Act1Level1Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act1Level1Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_bridge.isFinished()) return;

    if (m_visitedOrder.find(node->val) != m_visitedOrder.end())
    {
        m_engine.showToast("Room already patrolled in sequence.", TOAST_INFO);
        return;
    }

    PlayerAction action;
    action.type = CLICK_NODE;
    action.nodeA = node->val;

    ActionResult res = m_bridge.processAction(action);
    syncWithState();

    if (res.correct)
    {
        recordSharedAction(action);
        m_stepCounter++;
        m_visitedOrder[node->val] = m_stepCounter;
        node->isVisited = true;

        m_engine.audio.playCorrect();

        if (m_bridge.isWon())
        {
            m_engine.audio.playWin();
            const LevelState& s = m_bridge.getState();
            int stars = ScoreSystem::stars(s);
            int score = ScoreSystem::score(s);

            showResultModal(true, stars, s.steps, s.optimalSteps, s.penaltyPoints, score,
                [this]() {
                    // Next Level Button: Switch to Level 1.2
                    m_engine.changeScreen(std::make_unique<Act1Level2Screen>(m_engine));
                },
                [this]() {
                    // Retry Button
                    onResetClicked();
                },
                [this]() {
                    // Back to Campaign Button
                    onBackClicked();
                }
            );
        }
    }
    else
    {
        m_shakingNode = node;
        m_shakeTimer = 0.35f;

        m_engine.audio.playWrong();
        m_engine.showToast(res.message, TOAST_ERROR);

        if (m_bridge.isFinished() && !m_bridge.isWon())
        {
            m_engine.audio.playLose();
            const LevelState& s = m_bridge.getState();
            showResultModal(false, 0, s.steps, s.optimalSteps, s.penaltyPoints, 0,
                nullptr,
                [this]() { onResetClicked(); },
                [this]() { onBackClicked(); }
            );
        }
    }
}

void Act1Level1Screen::handleEvent(const sf::Event& event)
{
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

void Act1Level1Screen::update(float dt)
{
    LevelScreenBase::update(dt);

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        m_shakeOffset = Tween::shake(m_shakeTimer, 7.0f);
        if (m_shakeTimer <= 0.0f)
        {
            m_shakeOffset = sf::Vector2f(0.0f, 0.0f);
            m_shakingNode = nullptr;
        }
    }
}

void Act1Level1Screen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    // Draw order badges for patrolled nodes
    for (const auto& pair : m_visitedOrder)
    {
        int val = pair.first;
        int order = pair.second;

        std::function<TreeNode*(TreeNode*)> findNode = [&](TreeNode* curr) -> TreeNode* {
            if (!curr) return nullptr;
            if (curr->val == val) return curr;
            TreeNode* leftRes = findNode(curr->left);
            if (leftRes) return leftRes;
            return findNode(curr->right);
        };

        TreeNode* targetNode = findNode(m_rootNode);
        if (targetNode)
        {
            sf::Vector2f badgePos = targetNode->position + sf::Vector2f(-Theme::NodeRadius * 0.75f, -Theme::NodeRadius * 0.75f);
            sf::CircleShape badge(10.0f);
            badge.setOrigin(sf::Vector2f(10.0f, 10.0f));
            badge.setPosition(badgePos);
            badge.setFillColor(Theme::SuccessEmerald);
            badge.setOutlineThickness(1.5f);
            badge.setOutlineColor(sf::Color::White);
            target.draw(badge);

            sf::Text numText(theme.fontMono, std::to_string(order), 11);
            numText.setFillColor(sf::Color(10, 14, 23));
            numText.setStyle(sf::Text::Bold);
            sf::FloatRect b = numText.getLocalBounds();
            numText.setOrigin(sf::Vector2f(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f));
            numText.setPosition(badgePos);
            target.draw(numText);
        }
    }

    // Shaking node red alert
    if (m_shakingNode && m_shakeTimer > 0.0f)
    {
        sf::CircleShape shakeRing(Theme::NodeRadius + 5.0f);
        shakeRing.setOrigin(sf::Vector2f(Theme::NodeRadius + 5.0f, Theme::NodeRadius + 5.0f));
        shakeRing.setPosition(m_shakingNode->position + m_shakeOffset);
        shakeRing.setFillColor(sf::Color::Transparent);
        shakeRing.setOutlineThickness(3.0f);
        shakeRing.setOutlineColor(Theme::CrimsonAlarm);
        target.draw(shakeRing);
    }

    drawTooltip(target, theme);
}

void Act1Level1Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    sf::Text hdr(theme.fontMono, "PATROL LOG", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    std::string orderName = m_bridge.getTraversalType();
    std::string typeInfo = "REQUIRED ORDER: " + orderName;
    sf::Text tType(theme.fontMono, typeInfo, 12);
    tType.setFillColor(Theme::AccentYellow);
    tType.setPosition(sf::Vector2f(px + 4.0f, py + 26.0f));
    target.draw(tType);

    std::string visitedInfo = "ROOMS CLEARED: " + std::to_string(m_stepCounter) + " / " + std::to_string(m_optimalSteps);
    sf::Text tVis(theme.fontMono, visitedInfo, 12);
    tVis.setFillColor(Theme::TextPrimary);
    tVis.setPosition(sf::Vector2f(px + 4.0f, py + 48.0f));
    target.draw(tVis);

    // List of patrolled rooms in order
    std::string seqStr = "PATROL PATH:\n";
    int count = 0;
    for (const auto& p : m_visitedOrder)
    {
        seqStr += "[" + std::to_string(p.second) + "] Room " + std::to_string(p.first) + "  ";
        count++;
        if (count % 2 == 0) seqStr += "\n";
    }
    if (m_visitedOrder.empty()) seqStr += "(No rooms breached yet)";

    sf::Text tSeq(theme.fontMono, seqStr, 11);
    tSeq.setFillColor(Theme::TextMuted);
    tSeq.setPosition(sf::Vector2f(px + 4.0f, py + 78.0f));
    target.draw(tSeq);
}

void Act1Level1Screen::drawTooltip(sf::RenderTarget& target, const Theme& theme)
{
    TreeRenderer::drawNodeTooltip(target, theme, m_rootNode, m_hoverNode, false);
}
