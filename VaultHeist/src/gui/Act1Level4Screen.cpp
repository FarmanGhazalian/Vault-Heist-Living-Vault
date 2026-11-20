#include "gui/Act1Level4Screen.h"
#include "gui/Act1Level5Screen.h"
#include "gui/GameEngine.h"
#include <filesystem>
#include <cmath>

Act1Level4Screen::Act1Level4Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 1: THE BLUEPRINT WING", "LEVEL 1.4: WHERE ESCAPE ROUTES MEET")
    , m_roomA(-1)
    , m_roomB(-1)
    , m_lcaNodeVal(-1)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

void Act1Level4Screen::resetLevel()
{
    m_tracedPath.clear();
    m_lcaNodeVal = -1;
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;

    std::string path = "levels/Act1Level4.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/levels/Act1Level4.json";
    }

    if (m_bridge.loadLevel(1, 4, path))
    {
        m_rootNode = m_bridge.getRootNode();
        m_roomA = m_bridge.getRoomA();
        m_roomB = m_bridge.getRoomB();

        setInstruction(m_bridge.getObjective());
        syncWithState();
        if (m_rootNode)
        {
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
        }
    }
}

void Act1Level4Screen::syncWithState()
{
    const LevelState& state = m_bridge.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);

    if (m_bridge.isLCAFound())
    {
        int cur = m_bridge.getCurrentPathIndex();
        int total = m_bridge.getPathCount();
        setInstruction("LCA identified. Trace escape route (" + std::to_string(cur) + "/" + std::to_string(total) + ").");
    }
    else
    {
        setInstruction("Locate the Lowest Common Ancestor (Meeting Room) for A and B.");
    }
}

std::string Act1Level4Screen::getLevelSpecificHint()
{
    return m_bridge.getHint();
}

ILevelLogic* Act1Level4Screen::getLevelLogic()
{
    return m_bridge.getLogic();
}

void Act1Level4Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act1Level4Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_bridge.isFinished()) return;

    if (!m_bridge.isLCAFound())
    {
        PlayerAction action;
        action.type = CLICK_LCA;
        action.nodeA = node->val;

        ActionResult res = m_bridge.processAction(action);
        syncWithState();

        if (res.correct)
        {
            m_lcaNodeVal = node->val;
            m_engine.audio.playCorrect();
            // No success toast — audio feedback only
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
                    [this]() { resetLevel(); },
                    [this]() { onBackClicked(); }
                );
            }
        }
    }
    else
    {
        PlayerAction action;
        action.type = TRACE_PATH;
        action.nodeA = node->val;

        ActionResult res = m_bridge.processAction(action);
        syncWithState();

        if (res.correct)
        {
            m_tracedPath.push_back(node->val);
            node->isVisited = true;

            m_engine.audio.playCorrect();

            if (m_bridge.isWon())
            {
                m_engine.audio.playWin();
                const LevelState& s = m_bridge.getState();
                showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
                    [this]() {
                        // Next Level -> Level 1.5
                        m_engine.changeScreen(std::make_unique<Act1Level5Screen>(m_engine));
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

            if (m_bridge.isFinished() && !m_bridge.isWon())
            {
                m_engine.audio.playLose();
                const LevelState& s = m_bridge.getState();
                showResultModal(false, 0, s.steps, s.optimalSteps, s.penaltyPoints, 0,
                    nullptr,
                    [this]() { resetLevel(); },
                    [this]() { onBackClicked(); }
                );
            }
        }
    }
}

void Act1Level4Screen::handleEvent(const sf::Event& event)
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

void Act1Level4Screen::update(float dt)
{
    LevelScreenBase::update(dt);

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f)
        {
            m_shakingNode = nullptr;
        }
    }
}

void Act1Level4Screen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    std::function<TreeNode*(TreeNode*, int)> findNode = [&](TreeNode* curr, int val) -> TreeNode* {
        if (!curr) return nullptr;
        if (curr->val == val) return curr;
        TreeNode* l = findNode(curr->left, val);
        if (l) return l;
        return findNode(curr->right, val);
    };

    if (m_tracedPath.size() >= 2)
    {
        for (size_t i = 0; i + 1 < m_tracedPath.size(); ++i)
        {
            TreeNode* n1 = findNode(m_rootNode, m_tracedPath[i]);
            TreeNode* n2 = findNode(m_rootNode, m_tracedPath[i + 1]);
            if (n1 && n2)
            {
                sf::Vertex line[4] = {
                    sf::Vertex(n1->position + sf::Vector2f(-1.5f, -1.5f), Theme::SuccessEmerald),
                    sf::Vertex(n2->position + sf::Vector2f(-1.5f, -1.5f), Theme::SuccessEmerald),
                    sf::Vertex(n2->position + sf::Vector2f(1.5f, 1.5f), Theme::SuccessEmerald),
                    sf::Vertex(n1->position + sf::Vector2f(1.5f, 1.5f), Theme::SuccessEmerald)
                };
                target.draw(line, 4, sf::PrimitiveType::TriangleStrip);
            }
        }
    }

    auto drawBadge = [&](int roomVal, const std::string& label, sf::Color col) {
        TreeNode* node = findNode(m_rootNode, roomVal);
        if (node)
        {
            sf::Vector2f badgePos = node->position + sf::Vector2f(Theme::NodeRadius * 0.72f, -Theme::NodeRadius * 0.72f);
            sf::CircleShape badge(11.0f);
            badge.setOrigin(sf::Vector2f(11.0f, 11.0f));
            badge.setPosition(badgePos);
            badge.setFillColor(col);
            badge.setOutlineThickness(1.5f);
            badge.setOutlineColor(sf::Color::White);
            target.draw(badge);

            sf::Text bText(theme.fontMono, label, 10);
            bText.setFillColor(sf::Color(10, 14, 23));
            bText.setStyle(sf::Text::Bold);
            sf::FloatRect b = bText.getLocalBounds();
            bText.setOrigin(sf::Vector2f(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f));
            bText.setPosition(badgePos);
            target.draw(bText);
        }
    };

    drawBadge(m_roomA, "A", Theme::AccentYellow);
    drawBadge(m_roomB, "B", Theme::BorderCyan);

    if (m_lcaNodeVal != -1)
    {
        drawBadge(m_lcaNodeVal, "LCA", Theme::AccentYellow);
    }

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

    drawTooltip(target, theme);
}

void Act1Level4Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    sf::Text hdr(theme.fontMono, "ESCAPE ROUTE TRACKER", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    std::string sRooms = "TARGET A: Room " + std::to_string(m_roomA) + "\n" +
                         "TARGET B: Room " + std::to_string(m_roomB);
    sf::Text tRooms(theme.fontMono, sRooms, 12);
    tRooms.setFillColor(Theme::AccentYellow);
    tRooms.setPosition(sf::Vector2f(px + 4.0f, py + 26.0f));
    target.draw(tRooms);

    std::string lcaStr = "LCA STATUS: " + (m_lcaNodeVal != -1 ? ("FOUND (Room " + std::to_string(m_lcaNodeVal) + ")") : "SEARCHING...");
    sf::Text tLca(theme.fontMono, lcaStr, 12);
    tLca.setFillColor(m_lcaNodeVal != -1 ? Theme::SuccessEmerald : Theme::TextMuted);
    tLca.setPosition(sf::Vector2f(px + 4.0f, py + 68.0f));
    target.draw(tLca);

    int cur = m_bridge.getCurrentPathIndex();
    int tot = m_bridge.getPathCount();
    std::string pathInfo = "PATH STEP: " + std::to_string(cur) + " / " + std::to_string(tot);
    sf::Text tPath(theme.fontMono, pathInfo, 12);
    tPath.setFillColor(Theme::TextPrimary);
    tPath.setPosition(sf::Vector2f(px + 4.0f, py + 92.0f));
    target.draw(tPath);
}

void Act1Level4Screen::drawTooltip(sf::RenderTarget& target, const Theme& theme)
{
    TreeRenderer::drawNodeTooltip(target, theme, m_rootNode, m_hoverNode, false);
}
