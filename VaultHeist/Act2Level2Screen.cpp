#include "Act2Level2Screen.h"
#include "Act2Level3Screen.h"
#include "CampaignScreen.h"
#include "GameEngine.h"
#include "ScoreSystem.h"
#include <filesystem>
#include <cmath>

Act2Level2Screen::Act2Level2Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 2: THE SORTED WING", "LEVEL 2.2: STASH LOOT")
    , m_currentKey(0)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_shakingSlotIdx(-1)
    , m_newGrowthTimer(0.0f)
    , m_grownNodeVal(-1)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

void Act2Level2Screen::resetLevel()
{
    m_visitedPath.clear();
    m_vacantSlots.clear();
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;
    m_shakingSlotIdx = -1;
    m_newGrowthTimer = 0.0f;
    m_grownNodeVal = -1;

    std::string path = "Act2Level2.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/Act2Level2.json";
    }

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTreeRoot();
        m_currentKey = m_logic.getCurrentKey();

        setInstruction("Insert key " + std::to_string(m_currentKey) + ": Follow BST path to leaf, then choose slot.");
        syncWithState();
        if (m_rootNode)
        {
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
        }
        computeVacantSlots();
    }
}

void Act2Level2Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);
    m_currentKey = m_logic.getCurrentKey();
}

static TreeNode* findBstNode(TreeNode* root, int val)
{
    if (!root) return nullptr;
    if (root->val == val) return root;
    if (val < root->val) return findBstNode(root->left, val);
    return findBstNode(root->right, val);
}

void Act2Level2Screen::computeVacantSlots()
{
    m_vacantSlots.clear();
    if (!m_rootNode) return;

    int pIdx = m_logic.getCurrentPathIndex();
    int pLen = m_logic.getCurrentPathLength();

    // Show vacant slots only when at leaf step
    if (pIdx >= pLen && !m_visitedPath.empty())
    {
        int leafVal = m_visitedPath.back();
        TreeNode* leaf = findBstNode(m_rootNode, leafVal);
        if (leaf)
        {
            float spacingX = 54.0f;
            float spacingY = 70.0f;
            float slotR = Theme::NodeRadius;

            if (leaf->left == nullptr)
            {
                sf::Vector2f lPos(leaf->position.x - spacingX, leaf->position.y + spacingY);
                VacantSlot s;
                s.parentVal = leaf->val;
                s.direction = 0; // Left
                s.pos = lPos;
                s.bounds = sf::FloatRect(sf::Vector2f(lPos.x - slotR, lPos.y - slotR), sf::Vector2f(slotR * 2.0f, slotR * 2.0f));
                s.hovered = false;
                m_vacantSlots.push_back(s);
            }
            if (leaf->right == nullptr)
            {
                sf::Vector2f rPos(leaf->position.x + spacingX, leaf->position.y + spacingY);
                VacantSlot s;
                s.parentVal = leaf->val;
                s.direction = 1; // Right
                s.pos = rPos;
                s.bounds = sf::FloatRect(sf::Vector2f(rPos.x - slotR, rPos.y - slotR), sf::Vector2f(slotR * 2.0f, slotR * 2.0f));
                s.hovered = false;
                m_vacantSlots.push_back(s);
            }
        }
    }
}

void Act2Level2Screen::onHintClicked()
{
    PlayerAction action;
    action.type = CLICK_HINT;
    ActionResult res = m_logic.processAction(action);
    m_engine.showToast(res.message, TOAST_HINT);
    m_engine.audio.playHint();
    syncWithState();
}

void Act2Level2Screen::onResetClicked()
{
    resetLevel();
    m_engine.showToast("Insertion sequence reset to initial state.", TOAST_INFO);
    m_engine.audio.playClick();
}

void Act2Level2Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_logic.isFinished()) return;

    int pIdx = m_logic.getCurrentPathIndex();
    int pLen = m_logic.getCurrentPathLength();

    if (pIdx < pLen)
    {
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
            computeVacantSlots();
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
}

void Act2Level2Screen::handleEvent(const sf::Event& event)
{
    LevelScreenBase::handleEvent(event);

    if (auto* mouseMove = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mpos(static_cast<float>(mouseMove->position.x), static_cast<float>(mouseMove->position.y));
        if (m_rootNode && m_canvasBounds.contains(mpos))
        {
            m_hoverNode = m_treeRenderer.getNodeAtPosition(m_rootNode, mpos);
        }
        for (auto& s : m_vacantSlots)
        {
            s.hovered = s.bounds.contains(mpos);
        }
    }

    if (auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mb->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mpos(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));

            // Check if player clicked a vacant placement slot
            for (size_t i = 0; i < m_vacantSlots.size(); ++i)
            {
                if (m_vacantSlots[i].bounds.contains(mpos))
                {
                    PlayerAction action;
                    action.type = PLACE_TOKEN;
                    action.nodeA = m_vacantSlots[i].parentVal;
                    action.direction = m_vacantSlots[i].direction;
                    action.nodeB = m_currentKey;

                    ActionResult res = m_logic.processAction(action);
                    syncWithState();

                    if (res.correct)
                    {
                        m_engine.audio.playCorrect();
                        m_grownNodeVal = m_currentKey;
                        m_newGrowthTimer = 0.5f;

                        // Tree structure updated, recompute layout
                        m_rootNode = m_logic.getTreeRoot();
                        if (m_rootNode)
                        {
                            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
                        }

                        // Start next key insertion
                        m_visitedPath.clear();
                        m_currentKey = m_logic.getCurrentKey();
                        setInstruction("Insert key " + std::to_string(m_currentKey) + ": Trace from root, then place in vacant slot.");
                        computeVacantSlots();

                        if (m_logic.isWon())
                        {
                            m_engine.audio.playWin();
                            const LevelState& s = m_logic.getState();
                            showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
                                [this]() {
                                    m_engine.changeScreen(std::make_unique<Act2Level3Screen>(m_engine));
                                },
                                [this]() { resetLevel(); },
                                [this]() { onBackClicked(); }
                            );
                        }
                    }
                    else
                    {
                        m_shakingSlotIdx = static_cast<int>(i);
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
                    return;
                }
            }
        }
    }
}

void Act2Level2Screen::update(float dt)
{
    LevelScreenBase::update(dt);

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f)
        {
            m_shakingNode = nullptr;
            m_shakingSlotIdx = -1;
        }
    }

    if (m_newGrowthTimer > 0.0f)
    {
        m_newGrowthTimer -= dt;
        if (m_newGrowthTimer <= 0.0f)
        {
            m_grownNodeVal = -1;
        }
    }
}

void Act2Level2Screen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    // 1. Draw verified search path edges with comparison labels
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

                std::string compLabel = (m_currentKey < n1->val) ? "<" : ">";
                sf::Vector2f midPoint = (n1->position + n2->position) * 0.5f;
                sf::Text cText(theme.fontMono, compLabel, 12);
                cText.setFillColor(Theme::AccentYellow);
                cText.setStyle(sf::Text::Bold);
                cText.setPosition(midPoint + sf::Vector2f(6.0f, -8.0f));
                target.draw(cText);
            }
        }
    }

    // 2. Draw Vacant Child Slots (Hollow / Dashed Circles)
    for (size_t i = 0; i < m_vacantSlots.size(); ++i)
    {
        const auto& s = m_vacantSlots[i];
        float r = Theme::NodeRadius * 0.9f;
        sf::CircleShape slotShape(r);
        slotShape.setOrigin(sf::Vector2f(r, r));
        slotShape.setPosition(s.pos);
        slotShape.setFillColor(sf::Color(10, 14, 23, 140));
        slotShape.setOutlineThickness(s.hovered ? 2.5f : 1.5f);

        sf::Color slotColor = s.hovered ? Theme::AccentYellow : Theme::BorderCyan;
        if (static_cast<int>(i) == m_shakingSlotIdx && m_shakeTimer > 0.0f)
        {
            slotColor = Theme::CrimsonAlarm;
            slotShape.setPosition(s.pos + Tween::shake(m_shakeTimer, 7.0f));
        }
        slotShape.setOutlineColor(slotColor);
        target.draw(slotShape);

        std::string slotLabel = (s.direction == 0) ? "LEFT" : "RIGHT";
        sf::Text labelTxt(theme.fontMono, slotLabel, 10);
        labelTxt.setFillColor(slotColor);
        sf::FloatRect lb = labelTxt.getLocalBounds();
        labelTxt.setOrigin(sf::Vector2f(lb.position.x + lb.size.x * 0.5f, lb.position.y + lb.size.y * 0.5f));
        labelTxt.setPosition(slotShape.getPosition());
        target.draw(labelTxt);
    }

    // 3. Shake animation for wrong path clicks
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

    // 4. Growth Scale-In Ring for newly inserted node
    if (m_grownNodeVal != -1 && m_newGrowthTimer > 0.0f)
    {
        TreeNode* grownNode = findBstNode(m_rootNode, m_grownNodeVal);
        if (grownNode)
        {
            float prog = 1.0f - (m_newGrowthTimer / 0.5f);
            float growR = (Theme::NodeRadius + 4.0f) * (0.8f + 0.4f * Tween::easeOutQuad(prog));
            sf::CircleShape growRing(growR);
            growRing.setOrigin(sf::Vector2f(growR, growR));
            growRing.setPosition(grownNode->position);
            growRing.setFillColor(sf::Color::Transparent);
            growRing.setOutlineThickness(3.0f);
            growRing.setOutlineColor(Theme::SuccessEmerald);
            target.draw(growRing);
        }
    }

    // Tooltip according to Rule 7
    TreeRenderer::drawNodeTooltip(target, theme, m_rootNode, m_hoverNode, false);
}

void Act2Level2Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    sf::Text hdr(theme.fontMono, "STASH TELEMETRY", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    std::string sKey = "CURRENT KEY: " + std::to_string(m_currentKey);
    sf::Text tKey(theme.fontMono, sKey, 14);
    tKey.setFillColor(Theme::AccentYellow);
    tKey.setStyle(sf::Text::Bold);
    tKey.setPosition(sf::Vector2f(px + 4.0f, py + 26.0f));
    target.draw(tKey);

    int done = m_logic.getCompletedInsertions();
    int total = m_logic.getTotalInsertions();
    std::string sProg = "STASHED: " + std::to_string(done) + " / " + std::to_string(total) + " Keys";
    sf::Text tProg(theme.fontMono, sProg, 12);
    tProg.setFillColor(Theme::SuccessEmerald);
    tProg.setPosition(sf::Vector2f(px + 4.0f, py + 52.0f));
    target.draw(tProg);

    int curIdx = m_logic.getCurrentPathIndex();
    int pathLen = m_logic.getCurrentPathLength();
    std::string sPhase = (curIdx < pathLen) ? ("PATH PROGRESS: " + std::to_string(curIdx) + " / " + std::to_string(pathLen))
                                            : "AT INSERTION LEAF: Click Slot";
    sf::Text tPhase(theme.fontMono, sPhase, 11);
    tPhase.setFillColor(Theme::TextPrimary);
    tPhase.setPosition(sf::Vector2f(px + 4.0f, py + 78.0f));
    target.draw(tPhase);
}
