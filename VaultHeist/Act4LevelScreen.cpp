#include "Act4LevelScreen.h"
#include "Act4Level1.h"
#include "Act4Level2.h"
#include "Act4Level3.h"
#include "Act4Level4.h"
#include "Act4Level5.h"
#include "CampaignScreen.h"
#include "GameEngine.h"
#include "SaveSystem.h"
#include "ScoreSystem.h"
#include "TreeRenderer.h"
#include <filesystem>
#include <cmath>

Act4LevelScreen::Act4LevelScreen(GameEngine& engine, int levelNum)
    : LevelScreenBase(engine, "ACT 4: THE HEAP FORTRESS", "LEVEL 4." + std::to_string(levelNum))
    , m_levelNum(levelNum)
    , m_firstSelectedIdx(-1)
    , m_firstSelectedNode(nullptr)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_correctTimer(0.0f)
    , m_correctNode(nullptr)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

const MaxHeap* Act4LevelScreen::getHeap() const
{
    if (!m_logic) return nullptr;
    if (m_levelNum == 1) return &static_cast<Act4Level1*>(m_logic.get())->getHeap();
    if (m_levelNum == 2) return &static_cast<Act4Level2*>(m_logic.get())->getHeap();
    if (m_levelNum == 3) return &static_cast<Act4Level3*>(m_logic.get())->getHeap();
    if (m_levelNum == 4) return &static_cast<Act4Level4*>(m_logic.get())->getHeap();
    if (m_levelNum == 5) return &static_cast<Act4Level5*>(m_logic.get())->getHeap();
    return nullptr;
}

void Act4LevelScreen::rebuildTreeFromHeap()
{
    m_heapTreeNodes.clear();
    m_rootNode = nullptr;

    const MaxHeap* hp = getHeap();
    if (!hp || hp->getCount() <= 0) return;

    int n = hp->getCount();
    m_heapTreeNodes.reserve(n);

    for (int i = 0; i < n; ++i)
    {
        auto node = std::make_unique<TreeNode>();
        node->val = hp->getAt(i);
        node->left = nullptr;
        node->right = nullptr;
        node->height = 1;
        node->size = 1;
        node->isVisited = false;
        m_heapTreeNodes.push_back(std::move(node));
    }

    for (int i = 0; i < n; ++i)
    {
        int leftIdx = 2 * i + 1;
        int rightIdx = 2 * i + 2;
        if (leftIdx < n)  m_heapTreeNodes[i]->left = m_heapTreeNodes[leftIdx].get();
        if (rightIdx < n) m_heapTreeNodes[i]->right = m_heapTreeNodes[rightIdx].get();
    }

    m_rootNode = m_heapTreeNodes[0].get();
    TreeRenderer::updateNodeMetrics(m_rootNode);
    m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
}

int Act4LevelScreen::getNodeHeapIndex(TreeNode* node) const
{
    if (!node) return -1;
    for (size_t i = 0; i < m_heapTreeNodes.size(); ++i)
    {
        if (m_heapTreeNodes[i].get() == node)
            return static_cast<int>(i);
    }
    return -1;
}

void Act4LevelScreen::resetLevel()
{
    m_firstSelectedIdx = -1;
    m_firstSelectedNode = nullptr;
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;
    m_correctTimer = 0.0f;
    m_correctNode = nullptr;

    switch (m_levelNum)
    {
        case 1: m_logic = std::make_unique<Act4Level1>(); break;
        case 2: m_logic = std::make_unique<Act4Level2>(); break;
        case 3: m_logic = std::make_unique<Act4Level3>(); break;
        case 4: m_logic = std::make_unique<Act4Level4>(); break;
        case 5: m_logic = std::make_unique<Act4Level5>(); break;
        default: m_logic = std::make_unique<Act4Level1>(); break;
    }

    std::string path = "Act4Level" + std::to_string(m_levelNum) + ".json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/Act4Level" + std::to_string(m_levelNum) + ".json";
    }

    bool loaded = false;
    if (m_levelNum == 1) loaded = static_cast<Act4Level1*>(m_logic.get())->initLevel(path);
    else if (m_levelNum == 2) loaded = static_cast<Act4Level2*>(m_logic.get())->initLevel(path);
    else if (m_levelNum == 3) loaded = static_cast<Act4Level3*>(m_logic.get())->initLevel(path);
    else if (m_levelNum == 4) loaded = static_cast<Act4Level4*>(m_logic.get())->initLevel(path);
    else if (m_levelNum == 5) loaded = static_cast<Act4Level5*>(m_logic.get())->initLevel(path);

    if (loaded)
    {
        rebuildTreeFromHeap();
        syncWithState();

        if (m_levelNum == 1) setInstruction("Heapify the vault chambers into a valid Max-Heap.");
        else if (m_levelNum == 2) setInstruction("Repair locks by sifting down violated heap properties.");
        else if (m_levelNum == 3) setInstruction("Extract the maximum element and heap-sort the vault.");
        else if (m_levelNum == 4) setInstruction("Swiftly sort the vault security codes before time elapses.");
        else if (m_levelNum == 5) setInstruction("Final Boss: Build the heap and execute full heap-sort.");
    }
}

void Act4LevelScreen::syncWithState()
{
    if (!m_logic) return;
    const LevelState& state = m_logic->getState();
    setAlarm(state.alarm, state.alarmCap);
    setScore(ScoreSystem::score(state));
    setSteps(state.steps, state.optimalSteps);
    setPenalty(state.penaltyPoints);
}

std::string Act4LevelScreen::getLevelSpecificHint()
{
    if (!m_logic) return "Select parent and child to swap and satisfy heap order.";
    PlayerAction action;
    action.type = CLICK_HINT;
    ActionResult res = m_logic->processAction(action);
    return res.message.empty() ? "Select parent and child to swap and satisfy heap order." : res.message;
}

ILevelLogic* Act4LevelScreen::getLevelLogic()
{
    return m_logic.get();
}

void Act4LevelScreen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
    rebuildTreeFromHeap();
}

void Act4LevelScreen::reapplyActionSilently(const PlayerAction& action)
{
    if (m_logic)
    {
        m_logic->processAction(action);
        rebuildTreeFromHeap();
    }
    syncWithState();
}

void Act4LevelScreen::onNodeClicked(TreeNode* node)
{
    if (!node || !m_logic || m_logic->isFinished() || m_resultModal.active) return;

    int idx = getNodeHeapIndex(node);
    if (idx < 0) return;

    if (m_firstSelectedIdx < 0)
    {
        // First selection
        m_firstSelectedIdx = idx;
        m_firstSelectedNode = node;
        m_engine.audio.playClick();
        return;
    }

    if (m_firstSelectedIdx == idx)
    {
        // Deselect
        m_firstSelectedIdx = -1;
        m_firstSelectedNode = nullptr;
        m_engine.audio.playClick();
        return;
    }

    // Two nodes selected: perform swap action
    PlayerAction action;
    action.type = SWAP_ELEMENTS;
    action.nodeA = m_firstSelectedIdx;
    action.nodeB = idx;

    ActionResult res = m_logic->processAction(action);
    syncWithState();

    if (res.correct)
    {
        recordSharedAction(action);
        m_engine.audio.playCorrect();
        m_correctNode = node;
        m_correctTimer = 0.5f;

        rebuildTreeFromHeap();

        if (m_logic->isWon())
        {
            m_engine.audio.playWin();
            const LevelState& s = m_logic->getState();
            int stars = ScoreSystem::stars(s);
            int score = ScoreSystem::score(s);
            SaveSystem::setWon(4, m_levelNum, stars);

            showResultModal(true, stars, s.steps, s.optimalSteps, s.penaltyPoints, score,
                [this]() {
                    if (m_levelNum < 5)
                        m_engine.changeScreen(std::make_unique<Act4LevelScreen>(m_engine, m_levelNum + 1));
                    else
                        m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine));
                },
                [this]() { resetLevel(); },
                [this]() { onBackClicked(); }
            );
        }
    }
    else
    {
        m_engine.audio.playWrong();
        m_shakingNode = node;
        m_shakeTimer = 0.35f;
        m_engine.showToast(res.message.empty() ? "Invalid swap operation!" : res.message, TOAST_ERROR);

        if (m_logic->isFinished() && !m_logic->isWon())
        {
            m_engine.audio.playLose();
            const LevelState& s = m_logic->getState();
            showResultModal(false, 0, s.steps, s.optimalSteps, s.penaltyPoints, 0,
                nullptr,
                [this]() { resetLevel(); },
                [this]() { onBackClicked(); }
            );
        }
    }

    m_firstSelectedIdx = -1;
    m_firstSelectedNode = nullptr;
}

void Act4LevelScreen::handleEvent(const sf::Event& event)
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

void Act4LevelScreen::update(float dt)
{
    LevelScreenBase::update(dt);
    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f) m_shakingNode = nullptr;
    }
    if (m_correctTimer > 0.0f)
    {
        m_correctTimer -= dt;
        if (m_correctTimer <= 0.0f) m_correctNode = nullptr;
    }
}

void Act4LevelScreen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    // Selection ring
    if (m_firstSelectedNode)
    {
        sf::CircleShape selRing(Theme::NodeRadius + 4.0f);
        selRing.setOrigin(sf::Vector2f(Theme::NodeRadius + 4.0f, Theme::NodeRadius + 4.0f));
        selRing.setPosition(m_firstSelectedNode->position);
        selRing.setFillColor(sf::Color::Transparent);
        selRing.setOutlineThickness(2.5f);
        selRing.setOutlineColor(Theme::AccentYellow);
        target.draw(selRing);
    }

    // Wrong shake ring
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

    // Tooltip
    if (m_hoverNode)
    {
        TreeRenderer::drawNodeTooltip(target, theme, m_rootNode, m_hoverNode, false);
    }
}

void Act4LevelScreen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;
    float pw = panelBounds.size.x;

    sf::Text hdr(theme.fontMono, "HEAP TELEMETRY", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    const MaxHeap* hp = getHeap();
    int count = hp ? hp->getCount() : 0;
    int heapSize = hp ? hp->getHeapSize() : 0;

    std::string sCount = "TOTAL ELEMENTS: " + std::to_string(count);
    sf::Text tCount(theme.fontMono, sCount, 12);
    tCount.setFillColor(Theme::TextPrimary);
    tCount.setPosition(sf::Vector2f(px + 4.0f, py + 26.0f));
    target.draw(tCount);

    std::string sActive = "ACTIVE HEAP SIZE: " + std::to_string(heapSize);
    sf::Text tActive(theme.fontMono, sActive, 12);
    tActive.setFillColor(Theme::SuccessEmerald);
    tActive.setPosition(sf::Vector2f(px + 4.0f, py + 48.0f));
    target.draw(tActive);

    // Array representation underneath
    float cardW = pw - 8.0f;
    float cardH = 90.0f;
    sf::Vector2f cardPos(px + 4.0f, py + 80.0f);

    sf::RectangleShape card(sf::Vector2f(cardW, cardH));
    card.setPosition(cardPos);
    card.setFillColor(sf::Color(17, 24, 39, 235));
    card.setOutlineThickness(1.0f);
    card.setOutlineColor(Theme::BorderCyan);
    target.draw(card);

    sf::Text arrHdr(theme.fontMono, "ARRAY MAPPING", 10);
    arrHdr.setFillColor(Theme::AccentYellow);
    arrHdr.setStyle(sf::Text::Bold);
    arrHdr.setPosition(cardPos + sf::Vector2f(8.0f, 6.0f));
    target.draw(arrHdr);

    std::string arrStr = "";
    if (hp)
    {
        for (int i = 0; i < count && i < 10; ++i)
        {
            arrStr += std::to_string(hp->getAt(i)) + (i + 1 < count ? ", " : "");
        }
        if (count > 10) arrStr += "...";
    }

    sf::Text arrText(theme.fontMono, arrStr, 11);
    arrText.setFillColor(Theme::TextPrimary);
    arrText.setPosition(cardPos + sf::Vector2f(8.0f, 26.0f));
    target.draw(arrText);
}
