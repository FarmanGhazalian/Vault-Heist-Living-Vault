#include "LevelScreenBase.h"
#include "GameEngine.h"
#include "CampaignScreen.h"
#include <algorithm>
#include <cmath>

LevelScreenBase::LevelScreenBase(GameEngine& engine, const std::string& actTitle, const std::string& levelTitle)
    : m_engine(engine)
    , m_actTitle(actTitle)
    , m_levelTitle(levelTitle)
    , m_instructionText("Infiltrate the node structures. Avoid triggering security locks.")
    , m_currentAlarm(0)
    , m_alarmCap(10)
    , m_score(0)
    , m_currentSteps(0)
    , m_optimalSteps(0)
    , m_penaltyPoints(0)
    , m_backBtn("< BACK", sf::FloatRect(sf::Vector2f(20.0f, 15.0f), sf::Vector2f(95.0f, 34.0f)), BTN_BACK)
    , m_hintBtn("HINT [H]", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(100.0f, 36.0f)), BTN_SECONDARY)
    , m_resetBtn("RESET [R]", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(100.0f, 36.0f)), BTN_SECONDARY)
    , m_rootNode(nullptr)
{
    sf::Vector2f size = m_engine.getWindowSize();
    relayout(static_cast<unsigned int>(size.x), static_cast<unsigned int>(size.y));
}

void LevelScreenBase::relayout(unsigned int width, unsigned int height)
{
    float w = static_cast<float>(width);
    float h = static_cast<float>(height);

    // Top Header: 60px
    float headerH = 60.0f;
    m_backBtn.bounds = sf::FloatRect(sf::Vector2f(20.0f, 13.0f), sf::Vector2f(95.0f, 34.0f));

    // Right Side Panel: ~26% of screen width (min 260px)
    float panelW = std::max(270.0f, w * 0.26f);
    float panelX = w - panelW - 15.0f;
    float panelY = headerH + 10.0f;
    float panelH = h - panelY - 15.0f;
    m_sidePanelBounds = sf::FloatRect(sf::Vector2f(panelX, panelY), sf::Vector2f(panelW, panelH));

    // Left Tree Canvas Area: 72% width
    float canvasW = panelX - 25.0f;
    float canvasH = h - headerH - 25.0f;
    m_canvasBounds = sf::FloatRect(sf::Vector2f(15.0f, headerH + 10.0f), sf::Vector2f(canvasW, canvasH));

    // Side panel buttons (at the bottom of side panel)
    float btnY = panelY + panelH - 48.0f;
    float halfBtnW = (panelW - 24.0f) * 0.5f;
    m_hintBtn.bounds = sf::FloatRect(sf::Vector2f(panelX + 8.0f, btnY), sf::Vector2f(halfBtnW, 38.0f));
    m_resetBtn.bounds = sf::FloatRect(sf::Vector2f(panelX + 16.0f + halfBtnW, btnY), sf::Vector2f(halfBtnW, 38.0f));

    if (m_rootNode)
    {
        m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
    }
}

void LevelScreenBase::onResize(unsigned int width, unsigned int height)
{
    relayout(width, height);
}

void LevelScreenBase::setAlarm(int currentAlarm, int alarmCapacity)
{
    m_currentAlarm = currentAlarm;
    m_alarmCap = alarmCapacity > 0 ? alarmCapacity : 10;
}

void LevelScreenBase::setScore(int score)
{
    m_score = score;
}

void LevelScreenBase::setSteps(int currentSteps, int optimalSteps)
{
    m_currentSteps = currentSteps;
    m_optimalSteps = optimalSteps;
}

void LevelScreenBase::setPenalty(int penaltyPoints)
{
    m_penaltyPoints = penaltyPoints;
}

void LevelScreenBase::setInstruction(const std::string& text)
{
    m_instructionText = text;
}

void LevelScreenBase::showResultModal(bool victory, int starCount, int stepCount, int optSteps, int penaltyPoints, int totalScore,
                                      std::function<void()> nextCallback, std::function<void()> retryCallback, std::function<void()> backCallback)
{
    m_resultModal.show(victory, starCount, stepCount, optSteps, penaltyPoints, totalScore, nextCallback, retryCallback, backCallback);
}

void LevelScreenBase::onBackClicked()
{
    m_engine.showConfirmModal("LEAVE MISSION", "Are you sure you want to return to Campaign? Current infiltration progress will be lost.",
        [this]() {
            m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine));
        });
}

void LevelScreenBase::onHintClicked()
{
    m_engine.showToast("Observe node structure carefully.", TOAST_HINT);
}

void LevelScreenBase::onResetClicked()
{
    m_engine.showToast("Level configuration reset.", TOAST_INFO);
}

void LevelScreenBase::onNodeClicked(TreeNode* node)
{
    if (!node) return;
    m_engine.audio.playClick();
}

void LevelScreenBase::handleEvent(const sf::Event& event)
{
    // Result modal takes precedence
    if (m_resultModal.active)
    {
        m_resultModal.handleEvent(event);
        return;
    }

    if (auto* resized = event.getIf<sf::Event::Resized>())
    {
        onResize(resized->size.x, resized->size.y);
    }

    if (auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Escape)
        {
            onBackClicked();
            return;
        }
        else if (key->code == sf::Keyboard::Key::H)
        {
            onHintClicked();
            return;
        }
        else if (key->code == sf::Keyboard::Key::R)
        {
            onResetClicked();
            return;
        }
    }

    m_backBtn.handleEvent(event);
    m_hintBtn.handleEvent(event);
    m_resetBtn.handleEvent(event);

    if (m_backBtn.isClicked())
    {
        m_backBtn.resetClick();
        onBackClicked();
    }
    if (m_hintBtn.isClicked())
    {
        m_hintBtn.resetClick();
        onHintClicked();
    }
    if (m_resetBtn.isClicked())
    {
        m_resetBtn.resetClick();
        onResetClicked();
    }

    // Node interaction inside canvas bounds only
    if (auto* mouseMove = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mpos(static_cast<float>(mouseMove->position.x), static_cast<float>(mouseMove->position.y));
        if (m_rootNode && m_canvasBounds.contains(mpos))
        {
            TreeNode* hover = m_treeRenderer.getNodeAtPosition(m_rootNode, mpos);
            m_treeRenderer.setHoveredNode(hover);
        }
    }

    if (auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouseBtn->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mpos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
            if (m_rootNode && m_canvasBounds.contains(mpos))
            {
                TreeNode* clicked = m_treeRenderer.getNodeAtPosition(m_rootNode, mpos);
                if (clicked)
                {
                    onNodeClicked(clicked);
                }
            }
        }
    }
}

void LevelScreenBase::update(float dt)
{
    if (m_resultModal.active)
    {
        m_resultModal.update(dt);
        return;
    }
    m_treeRenderer.update(dt);
}

void LevelScreenBase::draw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;
    float w = static_cast<float>(target.getSize().x);
    float h = static_cast<float>(target.getSize().y);

    // 1. Top HUD Header Bar
    sf::RectangleShape hudBar(sf::Vector2f(w, 55.0f));
    hudBar.setFillColor(sf::Color(13, 19, 33, 235));
    hudBar.setOutlineThickness(1.0f);
    hudBar.setOutlineColor(Theme::BorderCyan);
    target.draw(hudBar);

    m_backBtn.draw(target, theme);

    sf::Text titleText(theme.fontUI, m_levelTitle, 19);
    titleText.setFillColor(Theme::TextPrimary);
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(sf::Vector2f(130.0f, 10.0f));
    target.draw(titleText);

    sf::Text actText(theme.fontMono, m_actTitle, 11);
    actText.setFillColor(Theme::BorderCyan);
    actText.setPosition(sf::Vector2f(130.0f, 32.0f));
    target.draw(actText);

    // 2. Tree Canvas Area
    theme.drawPanel(target, m_canvasBounds, sf::Color(0, 240, 255, 40), 1.0f);
    if (m_rootNode)
    {
        m_treeRenderer.draw(target, theme, m_rootNode);
    }
    onCustomDraw(target);

    // 3. Right Side Panel HUD (25% Width)
    theme.drawPanel(target, m_sidePanelBounds, Theme::BorderCyan, 1.2f);
    theme.drawGlowOutline(target, m_sidePanelBounds, Theme::BorderCyan, 0.4f);

    float px = m_sidePanelBounds.position.x;
    float py = m_sidePanelBounds.position.y;
    float pw = m_sidePanelBounds.size.x;

    // Panel Header: MISSION OBJECTIVE
    sf::Text objHeader(theme.fontMono, "// MISSION BRIEFING", 12);
    objHeader.setFillColor(Theme::BorderCyan);
    objHeader.setStyle(sf::Text::Bold);
    objHeader.setPosition(sf::Vector2f(px + 14.0f, py + 14.0f));
    target.draw(objHeader);

    sf::Text objBody(theme.fontUI, m_instructionText, 13);
    objBody.setFillColor(Theme::TextPrimary);
    objBody.setPosition(sf::Vector2f(px + 14.0f, py + 34.0f));
    target.draw(objBody);

    // Separator line
    sf::Vertex sep1[2] = {
        sf::Vertex(sf::Vector2f(px + 12.0f, py + 88.0f), Theme::BorderMuted),
        sf::Vertex(sf::Vector2f(px + pw - 12.0f, py + 88.0f), Theme::BorderMuted)
    };
    target.draw(sep1, 2, sf::PrimitiveType::Lines);

    // Stats Section: Steps vs Optimal, Score, Penalties
    sf::Text statsHeader(theme.fontMono, "// TELEMETRY & STATS", 11);
    statsHeader.setFillColor(Theme::TextMuted);
    statsHeader.setPosition(sf::Vector2f(px + 14.0f, py + 98.0f));
    target.draw(statsHeader);

    std::string sSteps = "STEPS: " + std::to_string(m_currentSteps) + " / OPTIMAL: " + std::to_string(m_optimalSteps);
    sf::Text txtSteps(theme.fontMono, sSteps, 12);
    txtSteps.setFillColor(Theme::TextPrimary);
    txtSteps.setPosition(sf::Vector2f(px + 14.0f, py + 118.0f));
    target.draw(txtSteps);

    std::string sScore = "SCORE: " + std::to_string(m_score) + " pts  (PENALTY: -" + std::to_string(m_penaltyPoints) + ")";
    sf::Text txtScore(theme.fontMono, sScore, 12);
    txtScore.setFillColor(Theme::AccentYellow);
    txtScore.setPosition(sf::Vector2f(px + 14.0f, py + 138.0f));
    target.draw(txtScore);

    // Alarm Meter Bar
    sf::Text alarmLabel(theme.fontMono, "ALARM THRESHOLD (" + std::to_string(m_currentAlarm) + "/" + std::to_string(m_alarmCap) + "):", 11);
    alarmLabel.setFillColor(Theme::TextMuted);
    alarmLabel.setPosition(sf::Vector2f(px + 14.0f, py + 165.0f));
    target.draw(alarmLabel);

    float barW = pw - 28.0f;
    float barH = 14.0f;
    sf::RectangleShape alarmTrack(sf::Vector2f(barW, barH));
    alarmTrack.setPosition(sf::Vector2f(px + 14.0f, py + 185.0f));
    alarmTrack.setFillColor(sf::Color(10, 14, 23, 220));
    alarmTrack.setOutlineThickness(1.0f);
    alarmTrack.setOutlineColor(Theme::BorderMuted);
    target.draw(alarmTrack);

    float alarmPct = std::clamp(static_cast<float>(m_currentAlarm) / static_cast<float>(m_alarmCap), 0.0f, 1.0f);
    if (alarmPct > 0.0f)
    {
        sf::RectangleShape alarmFill(sf::Vector2f(barW * alarmPct, barH));
        alarmFill.setPosition(sf::Vector2f(px + 14.0f, py + 185.0f));
        sf::Color fillCol = (alarmPct > 0.7f) ? Theme::CrimsonAlarm : ((alarmPct > 0.4f) ? Theme::AccentYellow : Theme::SuccessEmerald);
        alarmFill.setFillColor(fillCol);
        target.draw(alarmFill);
    }

    // Separator line before custom level area
    sf::Vertex sep2[2] = {
        sf::Vertex(sf::Vector2f(px + 12.0f, py + 215.0f), Theme::BorderMuted),
        sf::Vertex(sf::Vector2f(px + pw - 12.0f, py + 215.0f), Theme::BorderMuted)
    };
    target.draw(sep2, 2, sf::PrimitiveType::Lines);

    // 4. Custom Level Details Slot in Side Panel
    sf::FloatRect customSlot(sf::Vector2f(px + 12.0f, py + 225.0f), sf::Vector2f(pw - 24.0f, m_sidePanelBounds.size.y - 285.0f));
    onDrawSidePanel(target, customSlot);

    // 5. Side Panel Action Buttons
    m_hintBtn.draw(target, theme);
    m_resetBtn.draw(target, theme);

    // 6. Result Modal (Rendered on top of everything when level finished)
    if (m_resultModal.active)
    {
        m_resultModal.draw(target, theme, sf::Vector2f(w, h));
    }
}
