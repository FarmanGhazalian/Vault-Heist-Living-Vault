#include "Act2Level3Screen.h"
#include "Act2Level4Screen.h"
#include "CampaignScreen.h"
#include "GameEngine.h"
#include <filesystem>
#include <cmath>

Act2Level3Screen::Act2Level3Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 2: THE SORTED WING", "LEVEL 2.3: STEAL AND COLLAPSE")
    , m_currentTarget(0)
    , m_btnLeaf("1. LEAF (0 CH)", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(120.0f, 36.0f)), BTN_PRIMARY)
    , m_btnOneChild("2. ONE CHILD", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(120.0f, 36.0f)), BTN_PRIMARY)
    , m_btnTwoChildren("3. TWO CH (SUCC)", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(120.0f, 36.0f)), BTN_PRIMARY)
    , m_animTimer(0.0f)
    , m_animCase(-1)
    , m_fadingNodeVal(-1)
    , m_successorNodeVal(-1)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_pulseTimer(0.0f)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

void Act2Level3Screen::resetLevel()
{
    m_animTimer = 0.0f;
    m_animCase = -1;
    m_fadingNodeVal = -1;
    m_successorNodeVal = -1;
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;

    std::string path = "Act2Level3.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/Act2Level3.json";
    }

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTreeRoot();
        m_currentTarget = m_logic.getCurrentTarget();

        setInstruction("Classify removal case for Target Room " + std::to_string(m_currentTarget) + ": Leaf, 1 Child, or 2 Children.");
        syncWithState();
        if (m_rootNode)
        {
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
        }
    }
}

void Act2Level3Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);
    m_currentTarget = m_logic.getCurrentTarget();
}

static TreeNode* findBstNode(TreeNode* root, int val)
{
    if (!root) return nullptr;
    if (root->val == val) return root;
    if (val < root->val) return findBstNode(root->left, val);
    return findBstNode(root->right, val);
}

void Act2Level3Screen::submitCase(int caseVal)
{
    if (m_logic.isFinished() || m_animTimer > 0.0f) return;

    TreeNode* targetNode = findBstNode(m_rootNode, m_currentTarget);
    if (targetNode)
    {
        m_deletedNodePos = targetNode->position;
    }

    PlayerAction action;
    action.type = CHOOSE_CASE;
    action.value = caseVal;

    ActionResult res = m_logic.processAction(action);
    syncWithState();

    if (res.correct)
    {
        m_engine.audio.playCorrect();

        m_animCase = m_logic.getLastDeleteCase();
        m_fadingNodeVal = m_logic.getLastDeletedKey();
        m_successorNodeVal = m_logic.getLastSuccessorKey();
        m_animTimer = 0.6f;

        m_rootNode = m_logic.getTreeRoot();
        if (m_rootNode)
        {
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
        }
        setInstruction("Classify removal case for Target Room " + std::to_string(m_currentTarget) + ": Leaf, 1 Child, or 2 Children.");

        if (m_logic.isWon())
        {
            m_engine.audio.playWin();
            const LevelState& s = m_logic.getState();
            showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
                [this]() {
                    m_engine.changeScreen(std::make_unique<Act2Level4Screen>(m_engine));
                },
                [this]() { resetLevel(); },
                [this]() { onBackClicked(); }
            );
        }
    }
    else
    {
        m_shakingNode = findBstNode(m_rootNode, m_currentTarget);
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

std::string Act2Level3Screen::getLevelSpecificHint()
{
    PlayerAction action;
    action.type = CLICK_HINT;
    ActionResult res = m_logic.processAction(action);
    return res.message;
}

ILevelLogic* Act2Level3Screen::getLevelLogic()
{
    return &m_logic;
}

void Act2Level3Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act2Level3Screen::onNodeClicked(TreeNode* node)
{
    (void)node;
}

void Act2Level3Screen::handleEvent(const sf::Event& event)
{
    LevelScreenBase::handleEvent(event);

    m_btnLeaf.handleEvent(event);
    m_btnOneChild.handleEvent(event);
    m_btnTwoChildren.handleEvent(event);

    if (m_btnLeaf.isClicked())
    {
        m_btnLeaf.resetClick();
        submitCase(0);
    }
    else if (m_btnOneChild.isClicked())
    {
        m_btnOneChild.resetClick();
        submitCase(1);
    }
    else if (m_btnTwoChildren.isClicked())
    {
        m_btnTwoChildren.resetClick();
        submitCase(2);
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

void Act2Level3Screen::update(float dt)
{
    LevelScreenBase::update(dt);
    m_pulseTimer += dt;

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f) m_shakingNode = nullptr;
    }

    if (m_animTimer > 0.0f)
    {
        m_animTimer -= dt;
        if (m_animTimer <= 0.0f)
        {
            m_animCase = -1;
            m_fadingNodeVal = -1;
            m_successorNodeVal = -1;
        }
    }
}

void Act2Level3Screen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    // 1. Pulsing gold ring on Target Node to delete
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

    // 2. Case-specific deletion transition animations
    if (m_animTimer > 0.0f)
    {
        float t = m_animTimer / 0.6f; // 1.0 down to 0.0
        uint8_t alpha = static_cast<uint8_t>(std::clamp(t * 255.0f, 0.0f, 255.0f));

        if (m_animCase == 0) // Case 0: Leaf removed
        {
            float expandR = Theme::NodeRadius + (1.0f - t) * 16.0f;
            sf::CircleShape dissolve(expandR);
            dissolve.setOrigin(sf::Vector2f(expandR, expandR));
            dissolve.setPosition(m_deletedNodePos);
            dissolve.setFillColor(sf::Color::Transparent);
            dissolve.setOutlineThickness(2.0f);
            dissolve.setOutlineColor(sf::Color(0, 240, 255, alpha));
            target.draw(dissolve);
        }
        else if (m_animCase == 1) // Case 1: One child promoted
        {
            sf::CircleShape promoteRing(Theme::NodeRadius + 4.0f);
            promoteRing.setOrigin(sf::Vector2f(Theme::NodeRadius + 4.0f, Theme::NodeRadius + 4.0f));
            promoteRing.setPosition(m_deletedNodePos);
            promoteRing.setFillColor(sf::Color::Transparent);
            promoteRing.setOutlineThickness(2.5f);
            promoteRing.setOutlineColor(sf::Color(0, 255, 136, alpha));
            target.draw(promoteRing);

            sf::Text promoTxt(theme.fontMono, "PROMOTED", 11);
            promoTxt.setFillColor(sf::Color(0, 255, 136, alpha));
            promoTxt.setStyle(sf::Text::Bold);
            promoTxt.setPosition(sf::Vector2f(m_deletedNodePos.x - 30.0f, m_deletedNodePos.y - Theme::NodeRadius - 18.0f));
            target.draw(promoTxt);
        }
        else if (m_animCase == 2) // Case 2: Two children -> Inorder successor
        {
            TreeNode* succ = findBstNode(m_rootNode, m_successorNodeVal);
            if (succ)
            {
                // Line from successor to deleted position
                sf::VertexArray line(sf::PrimitiveType::Lines, 2);
                line[0].position = succ->position;
                line[0].color = sf::Color(0, 255, 136, alpha);
                line[1].position = m_deletedNodePos;
                line[1].color = sf::Color(255, 230, 0, alpha);
                target.draw(line);

                sf::CircleShape succRing(Theme::NodeRadius + 5.0f);
                succRing.setOrigin(sf::Vector2f(Theme::NodeRadius + 5.0f, Theme::NodeRadius + 5.0f));
                succRing.setPosition(succ->position);
                succRing.setFillColor(sf::Color::Transparent);
                succRing.setOutlineThickness(2.0f);
                succRing.setOutlineColor(sf::Color(0, 255, 136, alpha));
                target.draw(succRing);

                sf::Text succTxt(theme.fontMono, "SUCCESSOR", 11);
                succTxt.setFillColor(sf::Color(0, 255, 136, alpha));
                succTxt.setStyle(sf::Text::Bold);
                succTxt.setPosition(sf::Vector2f(succ->position.x - 30.0f, succ->position.y + Theme::NodeRadius + 4.0f));
                target.draw(succTxt);
            }
        }
    }

    // 3. Shake red ring for incorrect case choice
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

void Act2Level3Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;
    float pw = panelBounds.size.x;

    sf::Text hdr(theme.fontMono, "DELETION TELEMETRY", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    std::string sTarget = "TARGET KEY: Room " + std::to_string(m_currentTarget);
    sf::Text tTarget(theme.fontMono, sTarget, 14);
    tTarget.setFillColor(Theme::AccentYellow);
    tTarget.setStyle(sf::Text::Bold);
    tTarget.setPosition(sf::Vector2f(px + 4.0f, py + 24.0f));
    target.draw(tTarget);

    int done = m_logic.getCompletedDeletions();
    int tot = m_logic.getTotalDeletions();
    std::string sProg = "PROGRESS: " + std::to_string(done) + " / " + std::to_string(tot) + " Deleted";
    sf::Text tProg(theme.fontMono, sProg, 12);
    tProg.setFillColor(Theme::SuccessEmerald);
    tProg.setPosition(sf::Vector2f(px + 4.0f, py + 46.0f));
    target.draw(tProg);

    sf::Text pHeader(theme.fontMono, "CHOOSE CASE:", 12);
    pHeader.setFillColor(Theme::TextPrimary);
    pHeader.setPosition(sf::Vector2f(px + 4.0f, py + 72.0f));
    target.draw(pHeader);

    // Layout 3 choice buttons inside side panel
    float btnW = pw - 8.0f;
    float btnH = 34.0f;
    m_btnLeaf.bounds = sf::FloatRect(sf::Vector2f(px + 4.0f, py + 94.0f), sf::Vector2f(btnW, btnH));
    m_btnOneChild.bounds = sf::FloatRect(sf::Vector2f(px + 4.0f, py + 134.0f), sf::Vector2f(btnW, btnH));
    m_btnTwoChildren.bounds = sf::FloatRect(sf::Vector2f(px + 4.0f, py + 174.0f), sf::Vector2f(btnW, btnH));

    m_btnLeaf.draw(target, theme);
    m_btnOneChild.draw(target, theme);
    m_btnTwoChildren.draw(target, theme);
}
