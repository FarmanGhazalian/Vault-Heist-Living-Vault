#include "LevelScreenBase.h"
#include "GameEngine.h"
#include "CampaignScreen.h"
#include "SaveSystem.h"
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
    , m_isPanning(false)
    , m_panStartPixel(0, 0)
    , m_panOriginCenter(0.0f, 0.0f)
    , m_spaceHeld(false)
    , m_zoomLevel(1.0f)
    , m_fitBtn("FIT", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(44.0f, 24.0f)), BTN_SECONDARY)
    , m_backBtn("< BACK", sf::FloatRect(sf::Vector2f(20.0f, 12.0f), sf::Vector2f(95.0f, 32.0f)), BTN_BACK)
    , m_hintBtn("HINT", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(80.0f, 34.0f)), BTN_SECONDARY)
    , m_undoBtn("UNDO", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(80.0f, 34.0f)), BTN_SECONDARY)
    , m_replayBtn("REPLAY", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(80.0f, 34.0f)), BTN_SECONDARY)
    , m_resetBtn("RESET", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(80.0f, 34.0f)), BTN_SECONDARY)
    , m_rootNode(nullptr)
{
    sf::Vector2f size = m_engine.getWindowSize();
    relayout(static_cast<unsigned int>(size.x), static_cast<unsigned int>(size.y));
}

void LevelScreenBase::relayout(unsigned int width, unsigned int height)
{
    float w = static_cast<float>(width);
    float h = static_cast<float>(height);

    float headerH = 55.0f;
    float actionBarH = 46.0f;

    m_backBtn.bounds = sf::FloatRect(sf::Vector2f(16.0f, 11.0f), sf::Vector2f(90.0f, 32.0f));

    // Right Side Panel: ~27% width (min 280px)
    float panelW = std::max(280.0f, w * 0.27f);
    float panelX = w - panelW - 14.0f;
    float panelY = headerH + 10.0f;
    float panelH = h - panelY - 14.0f;
    m_sidePanelBounds = sf::FloatRect(sf::Vector2f(panelX, panelY), sf::Vector2f(panelW, panelH));

    // Left Tree Canvas Area
    float canvasW = panelX - 24.0f;
    float canvasH = h - headerH - actionBarH - 24.0f;
    m_canvasBounds = sf::FloatRect(sf::Vector2f(14.0f, headerH + 10.0f), sf::Vector2f(canvasW, canvasH));

    // Bottom Action Bar
    float barY = m_canvasBounds.position.y + m_canvasBounds.size.y + 6.0f;
    m_actionBarBounds = sf::FloatRect(sf::Vector2f(14.0f, barY), sf::Vector2f(canvasW, actionBarH));

    // Position Action Bar Buttons centered horizontally under the canvas
    std::vector<Button*> actionButtons = { &m_hintBtn, &m_undoBtn, &m_replayBtn, &m_resetBtn };
    layoutButtonBar(actionButtons, m_actionBarBounds, 88.0f, 34.0f, 12.0f);

    // Setup Canvas sf::View
    float vx = m_canvasBounds.position.x / w;
    float vy = m_canvasBounds.position.y / h;
    float vw = m_canvasBounds.size.x / w;
    float vh = m_canvasBounds.size.y / h;
    m_canvasView.setViewport(sf::FloatRect(sf::Vector2f(vx, vy), sf::Vector2f(vw, vh)));
    m_canvasView.setSize(sf::Vector2f(m_canvasBounds.size.x * m_zoomLevel, m_canvasBounds.size.y * m_zoomLevel));
    m_canvasView.setCenter(sf::Vector2f(m_canvasBounds.position.x + m_canvasBounds.size.x * 0.5f,
                                        m_canvasBounds.position.y + m_canvasBounds.size.y * 0.5f));

    // FIT button overlay in canvas
    m_fitBtn.bounds = sf::FloatRect(
        sf::Vector2f(m_canvasBounds.position.x + m_canvasBounds.size.x - 48.0f, m_canvasBounds.position.y + 6.0f),
        sf::Vector2f(42.0f, 22.0f));

    if (m_rootNode)
    {
        m_treeRenderer.computeLayout(m_rootNode, m_canvasBounds);
    }
}

void LevelScreenBase::onResize(unsigned int width, unsigned int height)
{
    relayout(width, height);
}

sf::Vector2f LevelScreenBase::canvasToWorld(sf::Vector2i pixelPos) const
{
    return m_engine.window.mapPixelToCoords(pixelPos, m_canvasView);
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

void LevelScreenBase::addToLog(const std::string& entry)
{
    m_operationLog.push_back(entry);
    if (m_operationLog.size() > 20)
    {
        m_operationLog.erase(m_operationLog.begin());
    }
}

void LevelScreenBase::showResultModal(bool victory, int starCount, int stepCount, int optSteps, int penaltyPoints, int totalScore,
                                      std::function<void()> nextCallback, std::function<void()> retryCallback, std::function<void()> backCallback)
{
    if (victory)
    {
        int act = 0, lvl = 0;
        size_t aPos = m_actTitle.find("ACT ");
        if (aPos != std::string::npos)
        {
            act = std::atoi(m_actTitle.c_str() + aPos + 4);
        }
        size_t lPos = m_levelTitle.find("LEVEL ");
        if (lPos != std::string::npos)
        {
            const char* p = m_levelTitle.c_str() + lPos + 6;
            while (*p && *p != '.') p++;
            if (*p == '.') lvl = std::atoi(p + 1);
        }
        if (act >= 1 && act <= 4 && lvl >= 1 && lvl <= 5)
        {
            SaveSystem::setWon(act, lvl, starCount);
        }
    }
    m_resultModal.show(victory, starCount, stepCount, optSteps, penaltyPoints, totalScore, nextCallback, retryCallback, backCallback);
}

void LevelScreenBase::onBackClicked()
{
    if (m_resultModal.active && m_resultModal.won)
    {
        m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine));
        return;
    }

    m_engine.showConfirmModal("LEAVE MISSION", "Are you sure you want to return to Campaign? Current infiltration progress will be lost.",
        [this]() {
            m_engine.changeScreen(std::make_unique<CampaignScreen>(m_engine));
        });
}

void LevelScreenBase::recordSharedAction(const PlayerAction& action)
{
    m_sharedActionHistory.push_back(action);
}

void LevelScreenBase::layoutButtonBar(const std::vector<Button*>& buttons, const sf::FloatRect& containerBounds, float btnW, float btnH, float spacing)
{
    if (buttons.empty()) return;
    float totalW = static_cast<float>(buttons.size()) * btnW + static_cast<float>(buttons.size() - 1) * spacing;
    float startX = containerBounds.position.x + (containerBounds.size.x - totalW) * 0.5f;
    float btnY = containerBounds.position.y + (containerBounds.size.y - btnH) * 0.5f;

    for (size_t i = 0; i < buttons.size(); ++i)
    {
        if (buttons[i])
        {
            buttons[i]->bounds = sf::FloatRect(sf::Vector2f(startX + static_cast<float>(i) * (btnW + spacing), btnY), sf::Vector2f(btnW, btnH));
        }
    }
}

void LevelScreenBase::onFitClicked()
{
    m_zoomLevel = 1.0f;
    m_isPanning = false;
    relayout(m_engine.window.getSize().x, m_engine.window.getSize().y);
}

void LevelScreenBase::onHintClicked()
{
    ILevelLogic* logic = getLevelLogic();

    if (!m_hintUsed)
    {
        m_hintUsed = true;
        if (logic)
        {
            logic->state.hintUsed = true;
        }
        std::string hint = getLevelSpecificHint();
        m_engine.showToast(hint, TOAST_HINT);
        m_engine.audio.playHint();
    }
    else
    {
        // Second press and later: do NOT show a hint.
        // Show message: Hint already used.
        // Add exactly 2 to the alarm meter per press.
        if (logic)
        {
            logic->state.alarm += 2;
            logic->state.checkGameOver();
            m_currentAlarm = logic->state.alarm;
        }
        else
        {
            m_currentAlarm += 2;
        }
        m_engine.showToast("Hint already used.", TOAST_ERROR);
        m_engine.audio.playWrong();
    }

    syncWithState();

    if (m_currentAlarm >= m_alarmCap)
    {
        m_engine.audio.playLose();
        showResultModal(false, 0, m_currentSteps, m_optimalSteps, m_penaltyPoints, 0,
            nullptr,
            [this]() { onResetClicked(); },
            [this]() { onBackClicked(); });
    }
}

void LevelScreenBase::onResetClicked()
{
    resetLevel();
    m_sharedActionHistory.clear();
    m_hintUsed = false;
    m_isReplaying = false;
    m_replayTimer = 0.0f;
    m_replayActionIndex = 0;
    m_currentAlarm = 0;
    m_score = 0;
    m_currentSteps = 0;
    m_penaltyPoints = 0;

    ILevelLogic* logic = getLevelLogic();
    if (logic)
    {
        logic->state.alarm = 0;
        logic->state.steps = 0;
        logic->state.penaltyPoints = 0;
        logic->state.penaltyCount = 0;
        logic->state.hintUsed = false;
        logic->state.finished = false;
        logic->state.won = false;
    }

    syncWithState();
    m_engine.showToast("Level reset to initial state.", TOAST_INFO);
    m_engine.audio.playClick();
}

void LevelScreenBase::onUndoClicked()
{
    if (m_sharedActionHistory.empty())
    {
        m_engine.showToast("No moves made yet.", TOAST_INFO);
        return;
    }

    std::vector<PlayerAction> historyCopy = m_sharedActionHistory;
    historyCopy.pop_back();

    // Silently reset level from JSON without emitting reset sound/toast
    resetLevel();
    m_sharedActionHistory.clear();
    m_hintUsed = false;
    m_isReplaying = false;
    m_replayTimer = 0.0f;
    m_replayActionIndex = 0;

    ILevelLogic* logic = getLevelLogic();
    if (logic)
    {
        logic->state.alarm = 0;
        logic->state.steps = 0;
        logic->state.penaltyPoints = 0;
        logic->state.penaltyCount = 0;
        logic->state.hintUsed = false;
        logic->state.finished = false;
        logic->state.won = false;
    }

    // Reapply all actions except the last silently
    for (const auto& act : historyCopy)
    {
        recordSharedAction(act);
        reapplyActionSilently(act);
    }

    syncWithState();
    m_engine.showToast("Last action undone.", TOAST_INFO);
    m_engine.audio.playClick();
}

void LevelScreenBase::onReplayClicked()
{
    if (m_sharedActionHistory.empty())
    {
        m_engine.showToast("No moves to replay.", TOAST_INFO);
        return;
    }

    std::vector<PlayerAction> historyCopy = m_sharedActionHistory;

    // Reset visually & silently
    resetLevel();
    m_sharedActionHistory = historyCopy;

    ILevelLogic* logic = getLevelLogic();
    if (logic)
    {
        logic->state.alarm = 0;
        logic->state.steps = 0;
        logic->state.penaltyPoints = 0;
        logic->state.penaltyCount = 0;
        logic->state.hintUsed = false;
        logic->state.finished = false;
        logic->state.won = false;
    }
    syncWithState();

    m_isReplaying = true;
    m_replayActionIndex = 0;
    m_replayTimer = 0.5f;
    m_engine.showToast("Replaying operation sequence...", TOAST_INFO);
    m_engine.audio.playClick();
}

void LevelScreenBase::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    // Default no-op implementation - derived screens may override to draw custom side-panel UI.
}

void LevelScreenBase::onNodeClicked(TreeNode* node)
{
    if (!node) return;
    m_engine.audio.playClick();
}

void LevelScreenBase::handleEvent(const sf::Event& event)
{
    m_eventConsumed = false;

    if (m_resultModal.active)
    {
        m_resultModal.handleEvent(event);
        m_eventConsumed = true;
        return;
    }

    if (auto* resized = event.getIf<sf::Event::Resized>())
    {
        onResize(resized->size.x, resized->size.y);
        return;
    }

    if (auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (key->code == sf::Keyboard::Key::Escape)
        {
            onBackClicked();
            m_eventConsumed = true;
            return;
        }
        else if (key->code == sf::Keyboard::Key::H)
        {
            onHintClicked();
            m_eventConsumed = true;
            return;
        }
        else if (key->code == sf::Keyboard::Key::R)
        {
            onResetClicked();
            m_eventConsumed = true;
            return;
        }
        else if (key->code == sf::Keyboard::Key::Space)
        {
            m_spaceHeld = true;
        }
        else if (key->code == sf::Keyboard::Key::Num0 || key->code == sf::Keyboard::Key::Numpad0)
        {
            onFitClicked();
            m_eventConsumed = true;
            return;
        }
        else if (key->code == sf::Keyboard::Key::U && key->control && key->shift)
        {
            bool unlocked = SaveSystem::toggleUnlockAll();
            if (unlocked)
            {
                m_engine.audio.playWin();
                m_engine.showToast("DEV OVERRIDE: All levels unlocked!", TOAST_INFO);
            }
            else
            {
                m_engine.audio.playClick();
                m_engine.showToast("DEV OVERRIDE: Levels locked back to start state.", TOAST_INFO);
            }
            m_eventConsumed = true;
            return;
        }
    }

    if (auto* keyRel = event.getIf<sf::Event::KeyReleased>())
    {
        if (keyRel->code == sf::Keyboard::Key::Space)
        {
            m_spaceHeld = false;
        }
    }

    // Topmost hit-test routing for buttons:
    // One click triggers exactly one handler. Topmost visible button consumes the click.
    std::vector<Button*> activeBtns = { &m_backBtn, &m_fitBtn, &m_hintBtn, &m_undoBtn, &m_replayBtn, &m_resetBtn };

    if (auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouseBtn->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mpos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
            for (auto* btn : activeBtns)
            {
                if (btn && btn->enabled && btn->bounds.contains(mpos))
                {
                    m_pressedBtn = btn;
                    btn->handleEvent(event);
                    m_eventConsumed = true;
                    return;
                }
            }
        }
    }
    else if (auto* mouseRel = event.getIf<sf::Event::MouseButtonReleased>())
    {
        if (mouseRel->button == sf::Mouse::Button::Left && m_pressedBtn)
        {
            Button* btn = m_pressedBtn;
            m_pressedBtn = nullptr;
            btn->handleEvent(event);
            m_eventConsumed = true;

            if (btn->isClicked())
            {
                btn->resetClick();
                if (btn == &m_backBtn)        onBackClicked();
                else if (btn == &m_hintBtn)   onHintClicked();
                else if (btn == &m_undoBtn)   onUndoClicked();
                else if (btn == &m_replayBtn) onReplayClicked();
                else if (btn == &m_resetBtn)  onResetClicked();
                else if (btn == &m_fitBtn)    onFitClicked();
            }
            return;
        }
    }

    // Mouse hover for buttons
    if (event.getIf<sf::Event::MouseMoved>())
    {
        for (auto* btn : activeBtns)
        {
            if (btn) btn->handleEvent(event);
        }
    }

    // Mouse Wheel Zoom centered on cursor
    if (auto* wheel = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        sf::Vector2f mpos(static_cast<float>(wheel->position.x), static_cast<float>(wheel->position.y));
        if (m_canvasBounds.contains(mpos))
        {
            float factor = (wheel->delta > 0) ? (1.0f / 1.15f) : 1.15f;
            float newZoom = std::clamp(m_zoomLevel * factor, 0.4f, 4.0f);
            factor = newZoom / m_zoomLevel;
            m_zoomLevel = newZoom;

            sf::Vector2f worldBefore = canvasToWorld(sf::Vector2i(wheel->position.x, wheel->position.y));
            m_canvasView.zoom(factor);
            sf::Vector2f worldAfter = canvasToWorld(sf::Vector2i(wheel->position.x, wheel->position.y));
            m_canvasView.move(worldBefore - worldAfter);
            m_eventConsumed = true;
            return;
        }
    }

    // Canvas Pan and Node Click
    if (auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>())
    {
        sf::Vector2f pixelPos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
        bool inCanvas = m_canvasBounds.contains(pixelPos);

        if (inCanvas)
        {
            if (mouseBtn->button == sf::Mouse::Button::Middle ||
                mouseBtn->button == sf::Mouse::Button::Right ||
                (mouseBtn->button == sf::Mouse::Button::Left && m_spaceHeld))
            {
                m_isPanning = true;
                m_panStartPixel = mouseBtn->position;
                m_panOriginCenter = m_canvasView.getCenter();
                m_eventConsumed = true;
                return;
            }
            else if (mouseBtn->button == sf::Mouse::Button::Left && !m_spaceHeld)
            {
                sf::Vector2f worldPos = canvasToWorld(mouseBtn->position);
                TreeNode* clicked = (m_rootNode) ? m_treeRenderer.getNodeAtPosition(m_rootNode, worldPos) : nullptr;
                if (clicked)
                {
                    onNodeClicked(clicked);
                    m_eventConsumed = true;
                    return;
                }
                else
                {
                    // Dragging canvas background freely (no clamping)
                    m_isPanning = true;
                    m_panStartPixel = mouseBtn->position;
                    m_panOriginCenter = m_canvasView.getCenter();
                    m_eventConsumed = true;
                    return;
                }
            }
        }
    }

    // Mouse Move (Pan update and Node hover)
    if (auto* mouseMove = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mpos(static_cast<float>(mouseMove->position.x), static_cast<float>(mouseMove->position.y));
        if (m_isPanning)
        {
            sf::Vector2i cur = mouseMove->position;
            sf::Vector2i delta = m_panStartPixel - cur;
            float scaleX = m_canvasView.getSize().x / m_canvasBounds.size.x;
            float scaleY = m_canvasView.getSize().y / m_canvasBounds.size.y;
            m_canvasView.setCenter(m_panOriginCenter + sf::Vector2f(delta.x * scaleX, delta.y * scaleY));
            m_eventConsumed = true;
            return;
        }
        else if (m_rootNode && m_canvasBounds.contains(mpos))
        {
            sf::Vector2f worldPos = canvasToWorld(mouseMove->position);
            TreeNode* hover = m_treeRenderer.getNodeAtPosition(m_rootNode, worldPos);
            m_treeRenderer.setHoveredNode(hover);
        }
    }

    // Mouse Release (Stop Pan)
    if (auto* mouseRel = event.getIf<sf::Event::MouseButtonReleased>())
    {
        if (mouseRel->button == sf::Mouse::Button::Middle ||
            mouseRel->button == sf::Mouse::Button::Right ||
            mouseRel->button == sf::Mouse::Button::Left)
        {
            m_isPanning = false;
        }
    }
}

void LevelScreenBase::update(float dt)
{
    if (m_resultModal.active)
    {
        m_resultModal.update(dt);
        // If a screen change was requested while the modal was active, defer it until now.
        if (m_pendingScreenChange && m_pendingCallback)
        {
            // Reset flag before invoking to avoid re‑entrancy issues.
            m_pendingScreenChange = false;
            auto cb = std::move(m_pendingCallback);
            m_pendingCallback = nullptr;
            cb();
        }
        return;
    }
    m_treeRenderer.update(dt);

    // Shared Replay Player Animation Loop
    if (m_isReplaying)
    {
        m_replayTimer -= dt;
        if (m_replayTimer <= 0.0f)
        {
            if (m_replayActionIndex < m_sharedActionHistory.size())
            {
                reapplyActionAnimated(m_sharedActionHistory[m_replayActionIndex]);
                m_replayActionIndex++;
                m_replayTimer = 0.5f;
            }
            else
            {
                m_isReplaying = false;
                m_engine.showToast("Replay completed.", TOAST_INFO);
            }
        }
    }
}

void LevelScreenBase::draw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;
    float w = static_cast<float>(target.getSize().x);
    float h = static_cast<float>(target.getSize().y);

    // 1. Top HUD Header Bar
    sf::RectangleShape hudBar(sf::Vector2f(w, 55.0f));
    hudBar.setFillColor(sf::Color(13, 19, 33, 240));
    hudBar.setOutlineThickness(1.0f);
    hudBar.setOutlineColor(Theme::BorderCyan);
    target.draw(hudBar);

    m_backBtn.draw(target, theme);

    sf::Text titleText(theme.fontUI, m_levelTitle, 19);
    titleText.setFillColor(Theme::TextPrimary);
    titleText.setStyle(sf::Text::Bold);
    titleText.setPosition(sf::Vector2f(125.0f, 10.0f));
    target.draw(titleText);

    sf::Text actText(theme.fontMono, m_actTitle, 11);
    actText.setFillColor(Theme::BorderCyan);
    actText.setPosition(sf::Vector2f(125.0f, 32.0f));
    target.draw(actText);

    // 2. Tree Canvas Area
    theme.drawPanel(target, m_canvasBounds, sf::Color(0, 240, 255, 35), 1.0f);

    // Render tree with canvas view
    target.setView(m_canvasView);
    if (m_rootNode)
    {
        m_treeRenderer.draw(target, theme, m_rootNode);
    }
    onCustomDraw(target);

    // Restore default view for GUI panels
    target.setView(target.getDefaultView());

    // Fit button overlay on canvas
    m_fitBtn.draw(target, theme);

    // 3. Bottom Action Bar
    theme.drawPanel(target, m_actionBarBounds, Theme::BorderCyan, 1.0f);
    m_hintBtn.draw(target, theme);
    m_undoBtn.draw(target, theme);
    m_replayBtn.draw(target, theme);
    m_resetBtn.draw(target, theme);

    // 4. Right Side Panel (3 Bordered Cards)
    float px = m_sidePanelBounds.position.x;
    float py = m_sidePanelBounds.position.y;
    float pw = m_sidePanelBounds.size.x;
    float ph = m_sidePanelBounds.size.y;

    // Card 1: MISSION OBJECTIVE (height ~115px)
    float c1H = 115.0f;
    sf::FloatRect card1Bounds(sf::Vector2f(px, py), sf::Vector2f(pw, c1H));
    theme.drawPanel(target, card1Bounds, Theme::BorderCyan, 1.0f);

    sf::Text objHeader(theme.fontMono, "MISSION OBJECTIVE", 11);
    objHeader.setFillColor(Theme::BorderCyan);
    objHeader.setStyle(sf::Text::Bold);
    objHeader.setPosition(sf::Vector2f(px + 12.0f, py + 10.0f));
    target.draw(objHeader);

    UIText::drawWrapped(target, theme.fontUI, m_instructionText, 12, Theme::TextPrimary,
        sf::Vector2f(px + 12.0f, py + 28.0f), pw - 24.0f);

    // Card 2: TELEMETRY & ALARM (height ~140px)
    float c2Y = py + c1H + 10.0f;
    float c2H = 140.0f;
    sf::FloatRect card2Bounds(sf::Vector2f(px, c2Y), sf::Vector2f(pw, c2H));
    theme.drawPanel(target, card2Bounds, Theme::BorderCyan, 1.0f);

    sf::Text statsHeader(theme.fontMono, "TELEMETRY", 11);
    statsHeader.setFillColor(Theme::BorderCyan);
    statsHeader.setStyle(sf::Text::Bold);
    statsHeader.setPosition(sf::Vector2f(px + 12.0f, c2Y + 10.0f));
    target.draw(statsHeader);

    std::string sSteps = "STEPS: " + std::to_string(m_currentSteps) + " / OPTIMAL: " + std::to_string(m_optimalSteps);
    sf::Text txtSteps(theme.fontMono, sSteps, 12);
    txtSteps.setFillColor(Theme::TextPrimary);
    txtSteps.setPosition(sf::Vector2f(px + 12.0f, c2Y + 28.0f));
    target.draw(txtSteps);

    std::string sScore = "SCORE: " + std::to_string(m_score) + " pts  (-" + std::to_string(m_penaltyPoints) + ")";
    sf::Text txtScore(theme.fontMono, sScore, 12);
    txtScore.setFillColor(Theme::AccentYellow);
    txtScore.setPosition(sf::Vector2f(px + 12.0f, c2Y + 48.0f));
    target.draw(txtScore);

    // Segmented Alarm Bar (10 segments)
    sf::Text alarmLabel(theme.fontMono, "ALARM STATUS (" + std::to_string(m_currentAlarm) + "/" + std::to_string(m_alarmCap) + "):", 11);
    alarmLabel.setFillColor(Theme::TextMuted);
    alarmLabel.setPosition(sf::Vector2f(px + 12.0f, c2Y + 72.0f));
    target.draw(alarmLabel);

    float segBarW = pw - 24.0f;
    float segH = 14.0f;
    int numSegments = 10;
    float segGap = 3.0f;
    float segW = (segBarW - (numSegments - 1) * segGap) / static_cast<float>(numSegments);
    int activeSegs = std::clamp(static_cast<int>(std::round(static_cast<float>(m_currentAlarm) / static_cast<float>(m_alarmCap) * numSegments)), 0, numSegments);

    for (int s = 0; s < numSegments; s++)
    {
        sf::RectangleShape seg(sf::Vector2f(segW, segH));
        seg.setPosition(sf::Vector2f(px + 12.0f + s * (segW + segGap), c2Y + 92.0f));
        if (s < activeSegs)
        {
            sf::Color col = (s >= 7) ? Theme::CrimsonAlarm : ((s >= 4) ? Theme::AccentYellow : Theme::SuccessEmerald);
            seg.setFillColor(col);
        }
        else
        {
            seg.setFillColor(sf::Color(25, 33, 44, 200));
        }
        seg.setOutlineThickness(1.0f);
        seg.setOutlineColor(Theme::BorderMuted);
        target.draw(seg);
    }

    // Card 3: OPERATION LOG & CUSTOM LEVEL CONTROLS
    float c3Y = c2Y + c2H + 10.0f;
    float c3H = ph - (c3Y - py);
    sf::FloatRect card3Bounds(sf::Vector2f(px, c3Y), sf::Vector2f(pw, c3H));
    theme.drawPanel(target, card3Bounds, Theme::BorderCyan, 1.0f);

    sf::Text logHeader(theme.fontMono, "OPERATION LOG", 11);
    logHeader.setFillColor(Theme::BorderCyan);
    logHeader.setStyle(sf::Text::Bold);
    logHeader.setPosition(sf::Vector2f(px + 12.0f, c3Y + 10.0f));
    target.draw(logHeader);

    // Render log lines or custom level slot
    float logY = c3Y + 28.0f;
    int displayCount = std::min(static_cast<int>(m_operationLog.size()), 4);
    int startIdx = static_cast<int>(m_operationLog.size()) - displayCount;
    for (int i = 0; i < displayCount; i++)
    {
        sf::Text line(theme.fontMono, "> " + m_operationLog[startIdx + i], 10);
        line.setFillColor(Theme::TextMuted);
        line.setPosition(sf::Vector2f(px + 12.0f, logY + i * 15.0f));
        target.draw(line);
    }

    // Slot for custom derived level panels inside Card 3
    float customSlotY = logY + displayCount * 15.0f + 6.0f;
    sf::FloatRect customSlot(sf::Vector2f(px + 10.0f, customSlotY), sf::Vector2f(pw - 20.0f, c3H - (customSlotY - c3Y) - 6.0f));
    onDrawSidePanel(target, customSlot);

    // 5. Result Modal (top layer)
    if (m_resultModal.active)
    {
        m_resultModal.draw(target, theme, sf::Vector2f(w, h));
    }
}
