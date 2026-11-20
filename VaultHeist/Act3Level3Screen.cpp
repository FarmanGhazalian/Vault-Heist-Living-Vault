#include "Act3Level3Screen.h"
#include "Act3Level4Screen.h"
#include "CampaignScreen.h"
#include "GameEngine.h"
#include "SaveSystem.h"
#include "ScoreSystem.h"
#include "TreeRenderer.h"
#include <filesystem>
#include <cmath>
#include <cstdint>

Act3Level3Screen::Act3Level3Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 3: THE ARCHITECT'S WING", "LEVEL 3.3: MANUAL REPAIR - DOUBLE")
    , m_btnLL("LL", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(60, 34)), BTN_SECONDARY)
    , m_btnRR("RR", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(60, 34)), BTN_SECONDARY)
    , m_btnLR("LR", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(60, 34)), BTN_SECONDARY)
    , m_btnRL("RL", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(60, 34)), BTN_SECONDARY)
    , m_btnRotateLeft("ROTATE LEFT", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(110, 34)), BTN_PRIMARY)
    , m_btnRotateRight("ROTATE RIGHT", sf::FloatRect(sf::Vector2f(0, 0), sf::Vector2f(110, 34)), BTN_PRIMARY)
    , m_selectedPivot(nullptr)
    , m_wrongNode(nullptr)
    , m_wrongTimer(0.0f)
    , m_correctNode(nullptr)
    , m_correctTimer(0.0f)
    , m_animTimer(0.0f)
{
    resetLevel();
}

void Act3Level3Screen::resetLevel()
{
    m_selectedPivot = nullptr;
    m_wrongNode = nullptr;
    m_wrongTimer = 0.0f;
    m_correctNode = nullptr;
    m_correctTimer = 0.0f;

    std::string path = "Act3Level3.json";
    if (!std::filesystem::exists(path))
        path = "VaultHeist/Act3Level3.json";

    if (m_logic.initLevel(path))
    {
        m_rootNode = m_logic.getTree();
        TreeRenderer::updateNodeMetrics(m_rootNode);
        setInstruction("Phase 1: Identify the rotation case. Phase 2: Select pivot node & apply correct rotation(s).");
        syncWithState();
        if (m_rootNode)
            m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
    }

    float bx = m_canvasBounds.position.x + 10.0f;
    float by = m_canvasBounds.position.y + m_canvasBounds.size.y - 44.0f;
    m_btnLL.bounds = sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(60, 34));
    m_btnRR.bounds = sf::FloatRect(sf::Vector2f(bx + 66, by), sf::Vector2f(60, 34));
    m_btnLR.bounds = sf::FloatRect(sf::Vector2f(bx + 132, by), sf::Vector2f(60, 34));
    m_btnRL.bounds = sf::FloatRect(sf::Vector2f(bx + 198, by), sf::Vector2f(60, 34));

    m_btnRotateLeft.bounds = sf::FloatRect(sf::Vector2f(bx, by), sf::Vector2f(115, 34));
    m_btnRotateRight.bounds = sf::FloatRect(sf::Vector2f(bx + 125, by), sf::Vector2f(115, 34));
}

void Act3Level3Screen::syncWithState()
{
    const LevelState& state = m_logic.getState();
    setAlarm(state.alarm, state.alarmCap);
    setScore(ScoreSystem::score(state));
    setSteps(state.steps, state.optimalSteps);
    setPenalty(state.penaltyPoints);
    m_rootNode = m_logic.getTree();
    if (m_rootNode)
    {
        TreeRenderer::updateNodeMetrics(m_rootNode);
        m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
    }
}

int Act3Level3Screen::computeBF(TreeNode* node) const
{
    if (!node) return 0;
    int lh = node->left ? node->left->height : 0;
    int rh = node->right ? node->right->height : 0;
    return lh - rh;
}

std::string Act3Level3Screen::getLevelSpecificHint()
{
    int phase = m_logic.getPhase();
    if (phase == 1)
        return "Phase 1: Observe the sign of BF for root and its heavy child. LR/RL are double rotations.";
    else
        return "Phase 2: For double rotations, first rotate the child, then the root.";
}

ILevelLogic* Act3Level3Screen::getLevelLogic()
{
    return &m_logic;
}

void Act3Level3Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act3Level3Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_resultModal.active) return;
    if (m_logic.getPhase() == 2)
    {
        m_selectedPivot = node;
        m_engine.audio.playClick();
        m_engine.showToast("Selected Pivot: " + std::to_string(node->val), TOAST_INFO);
    }
}

void Act3Level3Screen::handleEvent(const sf::Event& event)
{
    LevelScreenBase::handleEvent(event);
    if (m_resultModal.active) return;

    int phase = m_logic.getPhase();
    if (phase == 1)
    {
        m_btnLL.handleEvent(event);
        m_btnRR.handleEvent(event);
        m_btnLR.handleEvent(event);
        m_btnRL.handleEvent(event);

        auto handleCase = [&](int caseVal) {
            PlayerAction action;
            action.type = CHOOSE_CASE;
            action.value = caseVal;
            ActionResult res = m_logic.processAction(action);
            syncWithState();

            if (res.correct)
            {
                m_engine.audio.playCorrect();
                m_engine.showToast(res.message.empty() ? "CORRECT CASE! Now execute the rotation(s)." : res.message, TOAST_CORRECT);
                setInstruction("Phase 2: Click a node to select it as pivot, then click ROTATE LEFT or ROTATE RIGHT.");
            }
            else
            {
                m_engine.audio.playWrong();
                m_wrongTimer = 0.5f;
                m_engine.showToast(res.message.empty() ? "WRONG CASE!" : res.message, TOAST_ERROR);
            }
        };

        if (m_btnLL.isClicked()) { m_btnLL.resetClick(); handleCase(0); }
        if (m_btnRR.isClicked()) { m_btnRR.resetClick(); handleCase(1); }
        if (m_btnLR.isClicked()) { m_btnLR.resetClick(); handleCase(2); }
        if (m_btnRL.isClicked()) { m_btnRL.resetClick(); handleCase(3); }
    }
    else if (phase == 2)
    {
        m_btnRotateLeft.handleEvent(event);
        m_btnRotateRight.handleEvent(event);

        auto executeRotation = [&](int dir) {
            if (!m_selectedPivot)
            {
                m_engine.showToast("Select a pivot node in the tree first!", TOAST_HINT);
                return;
            }

            PlayerAction action;
            action.type = ROTATE;
            action.nodeA = m_selectedPivot->val;
            action.direction = dir;

            ActionResult res = m_logic.processAction(action);
            syncWithState();

            if (res.correct)
            {
                m_engine.audio.playCorrect();
                m_correctNode = m_selectedPivot;
                m_correctTimer = 1.0f;
                m_engine.showToast(res.message.empty() ? "ROTATION EXECUTED!" : res.message, TOAST_CORRECT);
                m_selectedPivot = nullptr;

                if (m_logic.isWon())
                {
                    const LevelState& state = m_logic.getState();
                    int stars = ScoreSystem::stars(state);
                    int scr = ScoreSystem::score(state);
                    SaveSystem::setWon(3, 3, stars);
                    showResultModal(true, stars, state.steps, state.optimalSteps, state.penaltyPoints, scr,
                        [this]() { m_engine.changeScreen(std::make_unique<Act3Level4Screen>(m_engine)); },
                        [this]() { resetLevel(); m_resultModal.active = false; },
                        [this]() { m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine)); });
                }
            }
            else
            {
                m_engine.audio.playWrong();
                m_wrongTimer = 0.5f;
                m_engine.showToast(res.message.empty() ? "INVALID ROTATION OR PIVOT!" : res.message, TOAST_ERROR);
            }
        };

        if (m_btnRotateLeft.isClicked()) { m_btnRotateLeft.resetClick(); executeRotation(0); }
        if (m_btnRotateRight.isClicked()) { m_btnRotateRight.resetClick(); executeRotation(1); }
    }
}

void Act3Level3Screen::update(float dt)
{
    LevelScreenBase::update(dt);
    m_animTimer += dt;
    if (m_correctTimer > 0.0f) m_correctTimer -= dt;
    if (m_wrongTimer > 0.0f) m_wrongTimer -= dt;
}

void Act3Level3Screen::drawBFBadges(sf::RenderTarget& target, TreeNode* node)
{
    if (!node) return;
    drawBFBadges(target, node->left);
    drawBFBadges(target, node->right);

    int bf = computeBF(node);
    int absBF = bf < 0 ? -bf : bf;
    sf::Color badgeCol = (absBF >= 2) ? Theme::CrimsonAlarm : Theme::BorderCyan;
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
}

void Act3Level3Screen::onCustomDraw(sf::RenderTarget& target)
{
    if (m_rootNode)
        drawBFBadges(target, m_rootNode);

    int phase = m_logic.getPhase();
    if (phase == 1)
    {
        m_btnLL.draw(target, m_engine.theme);
        m_btnRR.draw(target, m_engine.theme);
        m_btnLR.draw(target, m_engine.theme);
        m_btnRL.draw(target, m_engine.theme);

        sf::Text label(m_engine.theme.fontMono, "IDENTIFY CASE:", 11);
        label.setFillColor(Theme::AccentYellow);
        label.setPosition(sf::Vector2f(m_btnLL.bounds.position.x, m_btnLL.bounds.position.y - 18.0f));
        target.draw(label);
    }
    else if (phase == 2)
    {
        m_btnRotateLeft.draw(target, m_engine.theme);
        m_btnRotateRight.draw(target, m_engine.theme);

        sf::Text label(m_engine.theme.fontMono, m_selectedPivot ? ("PIVOT: " + std::to_string(m_selectedPivot->val)) : "SELECT A PIVOT NODE:", 11);
        label.setFillColor(m_selectedPivot ? Theme::SuccessEmerald : Theme::AccentYellow);
        label.setPosition(sf::Vector2f(m_btnRotateLeft.bounds.position.x, m_btnRotateLeft.bounds.position.y - 18.0f));
        target.draw(label);
    }

    if (m_selectedPivot && phase == 2)
    {
        float pulse = 0.5f + 0.5f * std::sin(m_animTimer * 4.0f);
        sf::CircleShape sel(Theme::NodeRadius + 7.0f + pulse * 2.0f);
        sel.setOrigin(sf::Vector2f(sel.getRadius(), sel.getRadius()));
        sel.setPosition(m_selectedPivot->position);
        sel.setFillColor(sf::Color::Transparent);
        sel.setOutlineThickness(2.5f);
        sel.setOutlineColor(sf::Color(0, 240, 255, static_cast<uint8_t>(180 + 75 * pulse)));
        target.draw(sel);
    }

    if (m_wrongTimer > 0.0f)
    {
        float alpha = m_wrongTimer / 0.5f;
        sf::RectangleShape flash(m_canvasBounds.size);
        flash.setPosition(m_canvasBounds.position);
        flash.setFillColor(sf::Color(255, 0, 0, static_cast<uint8_t>(30 * alpha)));
        target.draw(flash);
    }

    TreeRenderer::drawNodeTooltip(target, m_engine.theme, m_rootNode, nullptr);
}

void Act3Level3Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    int phase = m_logic.getPhase();
    std::string phaseStr = "PHASE " + std::to_string(phase) + ": " + (phase == 1 ? "IDENTIFY CASE" : "EXECUTE ROTATION");
    sf::Text phaseLabel(m_engine.theme.fontMono, phaseStr, 12);
    phaseLabel.setFillColor(phase == 1 ? Theme::AccentYellow : Theme::BorderCyan);
    phaseLabel.setStyle(sf::Text::Bold);
    phaseLabel.setPosition(sf::Vector2f(px, py));
    target.draw(phaseLabel);

    if (phase == 2)
    {
        sf::Text hint(m_engine.theme.fontUI, "Double Rotation Order:\n1. Rotate Child\n2. Rotate Root", 12);
        hint.setFillColor(Theme::TextMuted);
        hint.setPosition(sf::Vector2f(px, py + 26.0f));
        target.draw(hint);
    }
}
