#include "gui/Act2Level1Screen.h"
#include "gui/Act2Level2Screen.h"
#include "gui/CampaignScreen.h"
#include "gui/GameEngine.h"
#include <filesystem>
#include <cmath>

Act2Level1Screen::Act2Level1Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 2: THE SORTED WING", "LEVEL 2.1: SEARCH HEIST")
    , m_target(0)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_pulseTimer(0.0f)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

void Act2Level1Screen::resetLevel()
{
    m_visitedPath.clear();
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;

    std::string path = "levels/Act2Level1.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/levels/Act2Level1.json";
    }

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTreeRoot();
        m_target = m_logic.getTarget();

        setInstruction("Find room " + std::to_string(m_target) + " by following BST search comparisons from root.");
        syncWithState();
        if (m_rootNode)
        {
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
        }
    }
}

void Act2Level1Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);
}

std::string Act2Level1Screen::getLevelSpecificHint()
{
    PlayerAction action;
    action.type = CLICK_HINT;
    ActionResult res = m_logic.processAction(action);
    return res.message;
}

ILevelLogic* Act2Level1Screen::getLevelLogic()
{
    return &m_logic;
}

void Act2Level1Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act2Level1Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_logic.isFinished()) return;

    PlayerAction action;
    action.type = CLICK_NODE;
    action.nodeA = node->val;

    ActionResult res = m_logic.processAction(action);
    syncWithState();

    if (res.correct)
    {
        m_visitedPath.push_back(node->val);
        node->isVisited = true;
        m_engine.audio.playCorrect();

        if (m_logic.isWon())
        {
            m_engine.audio.playWin();
            const LevelState& s = m_logic.getState();
            showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
                [this]() {
                    m_engine.changeScreen(std::make_unique<Act2Level2Screen>(m_engine));
                },
                [this]() { resetLevel(); },
                [this]() { onBackClicked(); }
            );
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
            const LevelState& s = m_logic.getState();
            showResultModal(false, 0, s.steps, s.optimalSteps, s.penaltyPoints, 0,
                nullptr,
                [this]() { resetLevel(); },
                [this]() { onBackClicked(); }
            );
        }
    }
}

void Act2Level1Screen::handleEvent(const sf::Event& event)
{
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

void Act2Level1Screen::update(float dt)
{
    LevelScreenBase::update(dt);
    m_pulseTimer += dt;

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f) m_shakingNode = nullptr;
    }
}

static TreeNode* findBstNode(TreeNode* root, int val)
{
    if (!root) return nullptr;
    if (root->val == val) return root;
    if (val < root->val) return findBstNode(root->left, val);
    return findBstNode(root->right, val);
}

void Act2Level1Screen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    // 1. Draw glowing green lines along verified search path with < or > comparison labels
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

                // Small comparison label "<" or ">"
                std::string compLabel = (m_target < n1->val) ? "<" : ">";
                sf::Vector2f midPoint = (n1->position + n2->position) * 0.5f;
                sf::Text cText(theme.fontMono, compLabel, 12);
                cText.setFillColor(Theme::AccentYellow);
                cText.setStyle(sf::Text::Bold);
                cText.setPosition(midPoint + sf::Vector2f(6.0f, -8.0f));
                target.draw(cText);
            }
        }
    }

    // 2. Pulsing gold ring on Target Room
    TreeNode* targetNode = findBstNode(m_rootNode, m_target);
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

    // 3. Shake red ring for mistakes
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

    // Tooltip according to Rule 7
    TreeRenderer::drawNodeTooltip(target, theme, m_rootNode, m_hoverNode, false);
}

void Act2Level1Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    sf::Text hdr(theme.fontMono, "TARGET HEIST TELEMETRY", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    std::string sTarget = "TARGET KEY: Room " + std::to_string(m_target);
    sf::Text tTarget(theme.fontMono, sTarget, 13);
    tTarget.setFillColor(Theme::AccentYellow);
    tTarget.setStyle(sf::Text::Bold);
    tTarget.setPosition(sf::Vector2f(px + 4.0f, py + 26.0f));
    target.draw(tTarget);

    int cur = m_logic.getCurrentIndex();
    int tot = m_logic.getPathLength();
    std::string sProg = "PATH DEPTH: " + std::to_string(cur) + " / " + std::to_string(tot);
    sf::Text tProg(theme.fontMono, sProg, 12);
    tProg.setFillColor(Theme::TextPrimary);
    tProg.setPosition(sf::Vector2f(px + 4.0f, py + 52.0f));
    target.draw(tProg);

    std::string sHintRule = "BST RULE:\nIf Target < Node: Go LEFT\nIf Target > Node: Go RIGHT";
    sf::Text tRule(theme.fontMono, sHintRule, 11);
    tRule.setFillColor(Theme::TextMuted);
    tRule.setPosition(sf::Vector2f(px + 4.0f, py + 80.0f));
    target.draw(tRule);
}
