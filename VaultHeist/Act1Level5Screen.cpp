#include "Act1Level5Screen.h"
#include "CampaignScreen.h"
#include "GameEngine.h"
#include <filesystem>
#include <cmath>

Act1Level5Screen::Act1Level5Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 1: THE BLUEPRINT WING", "LEVEL 1.5: MIRROR THE VAULT")
    , m_autoMirrorBtn("AUTO MIRROR", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(130.0f, 36.0f)), BTN_PRIMARY)
    , m_selectedInspectNode(nullptr)
    , m_swapAnimTimer(0.0f)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_neutralNode(nullptr)
    , m_neutralTimer(0.0f)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

void Act1Level5Screen::resetLevel()
{
    m_nodeAnims.clear();
    m_swapAnimTimer = 0.0f;
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;
    m_neutralNode = nullptr;
    m_neutralTimer = 0.0f;
    m_selectedInspectNode = nullptr;

    std::string path = "Act1Level5.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/Act1Level5.json";
    }

    if (m_bridge.loadLevel(1, 5, path))
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

void Act1Level5Screen::syncWithState()
{
    const LevelState& state = m_bridge.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);
}

void Act1Level5Screen::onHintClicked()
{
    std::string hint = m_bridge.getHint();
    m_engine.showToast(hint, TOAST_HINT);
    m_engine.audio.playHint();
    syncWithState();
}

void Act1Level5Screen::onResetClicked()
{
    resetLevel();
    m_engine.showToast("Vault structure reset to original orientation.", TOAST_INFO);
    m_engine.audio.playClick();
}

void Act1Level5Screen::startSwapAnimation()
{
    if (!m_rootNode) return;

    std::map<int, sf::Vector2f> oldPos;
    std::function<void(TreeNode*)> recordPos = [&](TreeNode* n) {
        if (!n) return;
        oldPos[n->val] = n->position;
        recordPos(n->left);
        recordPos(n->right);
    };
    recordPos(m_rootNode);

    m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);

    m_nodeAnims.clear();
    std::function<void(TreeNode*)> setupAnim = [&](TreeNode* n) {
        if (!n) return;
        auto it = oldPos.find(n->val);
        if (it != oldPos.end())
        {
            m_nodeAnims[n->val] = NodeAnim{ it->second, n->position };
            n->position = it->second;
        }
        setupAnim(n->left);
        setupAnim(n->right);
    };
    setupAnim(m_rootNode);

    m_swapAnimTimer = SwapAnimDuration;
}

void Act1Level5Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_bridge.isFinished() || m_swapAnimTimer > 0.0f) return;

    m_selectedInspectNode = node;

    PlayerAction action;
    action.type = SWAP_CHILDREN;
    action.nodeA = node->val;

    ActionResult res = m_bridge.processAction(action);
    syncWithState();

    if (res.correct)
    {
        m_engine.audio.playCorrect();

        node->isVisited = true;
        startSwapAnimation();

        if (m_bridge.isWon())
        {
            m_engine.audio.playWin();
            const LevelState& s = m_bridge.getState();
            showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
                [this]() {
                    // Next Level -> All Act 1 complete, return to Campaign sector
                    m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine));
                },
                [this]() { resetLevel(); },
                [this]() { onBackClicked(); }
            );
        }
    }
    else
    {
        // All wrong steps: shake + wrong audio + error toast
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

void Act1Level5Screen::triggerAutoMirror()
{
    if (m_bridge.isFinished() || m_swapAnimTimer > 0.0f) return;

    PlayerAction action;
    action.type = AUTO_MIRROR;

    ActionResult res = m_bridge.processAction(action);
    syncWithState();

    m_engine.audio.playHint();
    m_engine.showToast("Executing Auto-Mirror subroutine...", TOAST_HINT);

    startSwapAnimation();

    if (m_bridge.isWon())
    {
        m_engine.audio.playWin();
        const LevelState& s = m_bridge.getState();
        showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
            [this]() {
                m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine));
            },
            [this]() { resetLevel(); },
            [this]() { onBackClicked(); }
        );
    }
}

void Act1Level5Screen::handleEvent(const sf::Event& event)
{
    LevelScreenBase::handleEvent(event);
    m_autoMirrorBtn.handleEvent(event);

    if (m_autoMirrorBtn.isClicked())
    {
        m_autoMirrorBtn.resetClick();
        triggerAutoMirror();
    }

    if (auto* mouseMove = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mpos(static_cast<float>(mouseMove->position.x), static_cast<float>(mouseMove->position.y));
        if (m_rootNode && m_canvasBounds.contains(mpos))
        {
            m_hoverNode = m_treeRenderer.getNodeAtPosition(m_rootNode, mpos);
        }
    }
}

void Act1Level5Screen::update(float dt)
{
    LevelScreenBase::update(dt);

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f) m_shakingNode = nullptr;
    }

    if (m_neutralTimer > 0.0f)
    {
        m_neutralTimer -= dt;
        if (m_neutralTimer <= 0.0f) m_neutralNode = nullptr;
    }

    if (m_swapAnimTimer > 0.0f && m_rootNode)
    {
        m_swapAnimTimer -= dt;
        float progress = 1.0f - std::clamp(m_swapAnimTimer / SwapAnimDuration, 0.0f, 1.0f);
        float ease = Tween::easeInOutQuad(progress);

        std::function<void(TreeNode*)> updateAnim = [&](TreeNode* n) {
            if (!n) return;
            auto it = m_nodeAnims.find(n->val);
            if (it != m_nodeAnims.end())
            {
                n->position = sf::Vector2f(
                    Tween::lerp(it->second.startPos.x, it->second.targetPos.x, ease),
                    Tween::lerp(it->second.startPos.y, it->second.targetPos.y, ease)
                );
            }
            updateAnim(n->left);
            updateAnim(n->right);
        };
        updateAnim(m_rootNode);
    }
}

void Act1Level5Screen::onCustomDraw(sf::RenderTarget& target)
{
    if (m_neutralNode && m_neutralTimer > 0.0f)
    {
        sf::CircleShape yellowRing(Theme::NodeRadius + 5.0f);
        yellowRing.setOrigin(sf::Vector2f(Theme::NodeRadius + 5.0f, Theme::NodeRadius + 5.0f));
        yellowRing.setPosition(m_neutralNode->position);
        yellowRing.setFillColor(sf::Color::Transparent);
        yellowRing.setOutlineThickness(2.5f);
        yellowRing.setOutlineColor(Theme::AccentYellow);
        target.draw(yellowRing);
    }

    if (m_shakingNode && m_shakeTimer > 0.0f)
    {
        sf::CircleShape redRing(Theme::NodeRadius + 5.0f);
        redRing.setOrigin(sf::Vector2f(Theme::NodeRadius + 5.0f, Theme::NodeRadius + 5.0f));
        redRing.setPosition(m_shakingNode->position + Tween::shake(m_shakeTimer, 7.0f));
        redRing.setFillColor(sf::Color::Transparent);
        redRing.setOutlineThickness(3.0f);
        redRing.setOutlineColor(Theme::CrimsonAlarm);
        target.draw(redRing);
    }
}

void Act1Level5Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;
    float pw = panelBounds.size.x;

    sf::Text hdr(theme.fontMono, "// INVERSION TELEMETRY", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    // Auto-Mirror button inside side panel
    m_autoMirrorBtn.bounds = sf::FloatRect(sf::Vector2f(px + 4.0f, py + 26.0f), sf::Vector2f(pw - 8.0f, 34.0f));
    m_autoMirrorBtn.draw(target, theme);

    // Selected node inspection card
    if (m_selectedInspectNode)
    {
        float cardW = pw - 8.0f;
        float cardH = 80.0f;
        sf::Vector2f pos(px + 4.0f, py + 72.0f);

        sf::RectangleShape card(sf::Vector2f(cardW, cardH));
        card.setPosition(pos);
        card.setFillColor(sf::Color(17, 24, 39, 235));
        card.setOutlineThickness(1.0f);
        card.setOutlineColor(Theme::BorderCyan);
        target.draw(card);

        std::string lVal = m_selectedInspectNode->left ? std::to_string(m_selectedInspectNode->left->val) : "None";
        std::string rVal = m_selectedInspectNode->right ? std::to_string(m_selectedInspectNode->right->val) : "None";

        std::string details = "ROOM: " + std::to_string(m_selectedInspectNode->val) + "\n" +
                              "LEFT CHILD:  " + lVal + "\n" +
                              "RIGHT CHILD: " + rVal + "\n" +
                              "SUBTREE:     " + std::to_string(m_selectedInspectNode->size) + " nodes";

        sf::Text text(theme.fontMono, details, 10);
        text.setFillColor(Theme::TextPrimary);
        text.setPosition(pos + sf::Vector2f(10.0f, 8.0f));
        target.draw(text);
    }
    else
    {
        sf::Text tPrompt(theme.fontMono, "(Click a parent node\nto view swap parameters)", 11);
        tPrompt.setFillColor(Theme::TextMuted);
        tPrompt.setPosition(sf::Vector2f(px + 8.0f, py + 85.0f));
        target.draw(tPrompt);
    }
}
