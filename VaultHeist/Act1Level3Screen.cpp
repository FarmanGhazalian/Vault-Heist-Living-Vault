#include "Act1Level3Screen.h"
#include "Act1Level4Screen.h"
#include "GameEngine.h"
#include <filesystem>
#include <cctype>

Act1Level3Screen::Act1Level3Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 1: THE BLUEPRINT WING", "LEVEL 1.3: ALARM SPREADS UPWARD")
    , m_focusedNode(nullptr)
    , m_diameterBoxFocused(false)
    , m_shakeTimer(0.0f)
    , m_shakingNode(nullptr)
    , m_shakeDiameterBox(false)
    , m_hoverNode(nullptr)
{
    resetLevel();
}

void Act1Level3Screen::resetLevel()
{
    m_solvedHeights.clear();
    m_focusedNode = nullptr;
    m_nodeInputBuffer.clear();
    m_diameterBoxFocused = false;
    m_diameterInputBuffer.clear();
    m_shakeTimer = 0.0f;
    m_shakingNode = nullptr;
    m_shakeDiameterBox = false;

    std::string path = "Act1Level3.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/Act1Level3.json";
    }

    if (m_bridge.loadLevel(1, 3, path))
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

void Act1Level3Screen::syncWithState()
{
    const LevelState& state = m_bridge.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);

    if (m_bridge.isHeightsPhaseDone())
    {
        setInstruction("All heights verified! Phase 2: Enter the Tree Diameter (longest path).");
    }
}

std::string Act1Level3Screen::getLevelSpecificHint()
{
    return m_bridge.getHint();
}

ILevelLogic* Act1Level3Screen::getLevelLogic()
{
    return m_bridge.getLogic();
}

void Act1Level3Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act1Level3Screen::onNodeClicked(TreeNode* node)
{
    if (!node || m_bridge.isFinished()) return;

    if (m_solvedHeights.find(node->val) != m_solvedHeights.end())
    {
        m_engine.showToast("Room height already verified.", TOAST_INFO);
        return;
    }

    m_focusedNode = node;
    m_nodeInputBuffer.clear();
    m_diameterBoxFocused = false;
    m_engine.audio.playClick();
}

void Act1Level3Screen::submitHeight()
{
    if (!m_focusedNode || m_nodeInputBuffer.empty()) return;

    int typedH = 0;
    try {
        typedH = std::stoi(m_nodeInputBuffer);
    } catch (...) {
        m_nodeInputBuffer.clear();
        return;
    }

    PlayerAction action;
    action.type = ENTER_HEIGHT;
    action.nodeA = m_focusedNode->val;
    action.nodeB = typedH;

    ActionResult res = m_bridge.processAction(action);
    syncWithState();

    if (res.correct)
    {
        m_solvedHeights[m_focusedNode->val] = typedH;
        m_focusedNode->isVisited = true;
        m_engine.audio.playCorrect();

        m_focusedNode = nullptr;
        m_nodeInputBuffer.clear();

        if (m_bridge.isHeightsPhaseDone())
        {
            m_diameterBoxFocused = true;
        }
    }
    else
    {
        m_shakingNode = m_focusedNode;
        m_shakeTimer = 0.35f;

        m_engine.audio.playWrong();
        m_engine.showToast(res.message, TOAST_ERROR);
        m_nodeInputBuffer.clear();

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

void Act1Level3Screen::submitDiameter()
{
    if (m_diameterInputBuffer.empty()) return;

    int typedDia = 0;
    try {
        typedDia = std::stoi(m_diameterInputBuffer);
    } catch (...) {
        m_diameterInputBuffer.clear();
        return;
    }

    PlayerAction action;
    action.type = ENTER_DIAMETER;
    action.nodeB = typedDia;

    ActionResult res = m_bridge.processAction(action);
    syncWithState();

    if (res.correct)
    {
        m_engine.audio.playWin();
        m_diameterBoxFocused = false;

        const LevelState& s = m_bridge.getState();
        showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
            [this]() {
                // Next Level -> Level 1.4
                m_engine.changeScreen(std::make_unique<Act1Level4Screen>(m_engine));
            },
            [this]() { resetLevel(); },
            [this]() { onBackClicked(); }
        );
    }
    else
    {
        m_shakeDiameterBox = true;
        m_shakeTimer = 0.35f;

        m_engine.audio.playWrong();
        m_engine.showToast(res.message, TOAST_ERROR);
        m_diameterInputBuffer.clear();

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

void Act1Level3Screen::handleEvent(const sf::Event& event)
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

    if (auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouseBtn->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mpos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));
            if (m_bridge.isHeightsPhaseDone())
            {
                if (m_submitDiaBtn.bounds.contains(mpos))
                {
                    m_engine.audio.playClick();
                    submitDiameter();
                    return;
                }
                for (auto& np : m_numpadBtns)
                {
                    if (np.first.bounds.contains(mpos))
                    {
                        if (np.second == '<')
                        {
                            if (!m_diameterInputBuffer.empty()) m_diameterInputBuffer.pop_back();
                        }
                        else if (m_diameterInputBuffer.size() < 3)
                        {
                            m_diameterInputBuffer.push_back(np.second);
                        }
                        m_diameterBoxFocused = true;
                        m_engine.audio.playClick();
                        return;
                    }
                }
                if (m_diameterBoxBounds.contains(mpos))
                {
                    m_diameterBoxFocused = true;
                    m_focusedNode = nullptr;
                    m_engine.audio.playClick();
                }
            }
        }
    }

    // Handle control keys only via KeyPressed (Enter, Backspace, Escape)
    if (auto* key = event.getIf<sf::Event::KeyPressed>())
    {
        if (m_focusedNode)
        {
            if (key->code == sf::Keyboard::Key::Enter) submitHeight();
            else if (key->code == sf::Keyboard::Key::Backspace) { if (!m_nodeInputBuffer.empty()) m_nodeInputBuffer.pop_back(); }
            else if (key->code == sf::Keyboard::Key::Escape) { m_focusedNode = nullptr; m_nodeInputBuffer.clear(); }
        }
        else if (m_diameterBoxFocused || m_bridge.isHeightsPhaseDone())
        {
            if (key->code == sf::Keyboard::Key::Enter) submitDiameter();
            else if (key->code == sf::Keyboard::Key::Backspace) { if (!m_diameterInputBuffer.empty()) m_diameterInputBuffer.pop_back(); }
        }
    }

    // Handle digit input ONLY via TextEntered to prevent double-entry
    if (auto* textEv = event.getIf<sf::Event::TextEntered>())
    {
        char32_t unicode = textEv->unicode;
        if (unicode >= '0' && unicode <= '9')
        {
            char typedDigit = static_cast<char>(unicode);
            if (m_focusedNode && m_nodeInputBuffer.size() < 3)
            {
                m_nodeInputBuffer.push_back(typedDigit);
                m_engine.audio.playClick();
            }
            else if ((m_diameterBoxFocused || m_bridge.isHeightsPhaseDone()) && m_diameterInputBuffer.size() < 3)
            {
                m_diameterBoxFocused = true;
                m_diameterInputBuffer.push_back(typedDigit);
                m_engine.audio.playClick();
            }
        }
    }
}

void Act1Level3Screen::update(float dt)
{
    LevelScreenBase::update(dt);

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f)
        {
            m_shakingNode = nullptr;
            m_shakeDiameterBox = false;
        }
    }
}

void Act1Level3Screen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    std::function<void(TreeNode*)> drawNodeHeights = [&](TreeNode* curr) {
        if (!curr) return;

        sf::Vector2f boxPos = curr->position + sf::Vector2f(-18.0f, -Theme::NodeRadius - 28.0f);
        if (curr == m_shakingNode && m_shakeTimer > 0.0f)
        {
            boxPos += Tween::shake(m_shakeTimer, 6.0f);
        }

        auto it = m_solvedHeights.find(curr->val);
        if (it != m_solvedHeights.end())
        {
            sf::RectangleShape solvedBox(sf::Vector2f(36.0f, 20.0f));
            solvedBox.setPosition(boxPos);
            solvedBox.setFillColor(sf::Color(10, 14, 23, 230));
            solvedBox.setOutlineThickness(1.2f);
            solvedBox.setOutlineColor(Theme::SuccessEmerald);
            target.draw(solvedBox);

            sf::Text hText(theme.fontMono, "H:" + std::to_string(it->second), 11);
            hText.setFillColor(Theme::SuccessEmerald);
            hText.setStyle(sf::Text::Bold);
            sf::FloatRect b = hText.getLocalBounds();
            hText.setOrigin(sf::Vector2f(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f));
            hText.setPosition(boxPos + sf::Vector2f(18.0f, 10.0f));
            target.draw(hText);
        }
        else if (curr == m_focusedNode)
        {
            sf::RectangleShape editBox(sf::Vector2f(42.0f, 22.0f));
            editBox.setPosition(boxPos + sf::Vector2f(-3.0f, -1.0f));
            editBox.setFillColor(sf::Color(19, 27, 46, 240));
            editBox.setOutlineThickness(1.8f);
            editBox.setOutlineColor(Theme::AccentYellow);
            target.draw(editBox);

            std::string displayText = m_nodeInputBuffer + "|";
            sf::Text inputText(theme.fontMono, displayText, 12);
            inputText.setFillColor(Theme::AccentYellow);
            inputText.setStyle(sf::Text::Bold);
            sf::FloatRect b = inputText.getLocalBounds();
            inputText.setOrigin(sf::Vector2f(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f));
            inputText.setPosition(boxPos + sf::Vector2f(18.0f, 10.0f));
            target.draw(inputText);
        }
        else
        {
            sf::RectangleShape dimBox(sf::Vector2f(30.0f, 16.0f));
            dimBox.setPosition(boxPos + sf::Vector2f(3.0f, 2.0f));
            dimBox.setFillColor(sf::Color(10, 14, 23, 140));
            dimBox.setOutlineThickness(1.0f);
            dimBox.setOutlineColor(Theme::BorderMuted);
            target.draw(dimBox);

            sf::Text qText(theme.fontMono, "h=?", 9);
            qText.setFillColor(Theme::TextMuted);
            sf::FloatRect b = qText.getLocalBounds();
            qText.setOrigin(sf::Vector2f(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f));
            qText.setPosition(boxPos + sf::Vector2f(18.0f, 10.0f));
            target.draw(qText);
        }

        drawNodeHeights(curr->left);
        drawNodeHeights(curr->right);
    };

    if (m_rootNode)
    {
        drawNodeHeights(m_rootNode);
    }

    drawTooltip(target, theme);
}

void Act1Level3Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;
    float pw = panelBounds.size.x;

    sf::Text hdr(theme.fontMono, "HEIGHT & DIAMETER", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    int total = m_bridge.getTotalNodes();
    std::string sSolved = "HEIGHTS VERIFIED: " + std::to_string(m_solvedHeights.size()) + " / " + std::to_string(total);
    sf::Text tSol(theme.fontMono, sSolved, 12);
    tSol.setFillColor(Theme::SuccessEmerald);
    tSol.setPosition(sf::Vector2f(px + 4.0f, py + 26.0f));
    target.draw(tSol);

    std::string phaseStr = m_bridge.isHeightsPhaseDone() ? "PHASE: 2 (ENTER DIAMETER)" : "PHASE: 1 (BOTTOM-UP HEIGHTS)";
    sf::Text tPh(theme.fontMono, phaseStr, 11);
    tPh.setFillColor(Theme::AccentYellow);
    tPh.setPosition(sf::Vector2f(px + 4.0f, py + 48.0f));
    target.draw(tPh);

    // In Side Panel: Render Diameter Input Box when unlocked
    if (m_bridge.isHeightsPhaseDone())
    {
        float boxW = pw - 12.0f;
        float boxH = 42.0f;
        sf::Vector2f diaPos(px + 6.0f, py + 80.0f);
        if (m_shakeDiameterBox && m_shakeTimer > 0.0f)
        {
            diaPos += Tween::shake(m_shakeTimer, 7.0f);
        }
        m_diameterBoxBounds = sf::FloatRect(diaPos, sf::Vector2f(boxW, boxH));

        sf::RectangleShape diaBox(sf::Vector2f(boxW, boxH));
        diaBox.setPosition(diaPos);
        diaBox.setFillColor(sf::Color(10, 14, 23, 240));
        diaBox.setOutlineThickness(2.0f);
        diaBox.setOutlineColor(Theme::AccentYellow);
        target.draw(diaBox);

        std::string str = "DIAMETER: " + m_diameterInputBuffer + "|";
        sf::Text diaText(theme.fontMono, str, 12);
        diaText.setFillColor(Theme::AccentYellow);
        diaText.setPosition(diaPos + sf::Vector2f(10.0f, 12.0f));
        target.draw(diaText);

        // Instructions
        sf::Text tipText(theme.fontMono, "Type number + ENTER to submit", 10);
        tipText.setFillColor(Theme::TextMuted);
        tipText.setPosition(diaPos + sf::Vector2f(4.0f, boxH + 4.0f));
        target.draw(tipText);

        // SUBMIT button
        float sbW = boxW;
        float sbH = 30.0f;
        sf::Vector2f sbPos(px + 6.0f, py + 80.0f + boxH + 24.0f);
        m_submitDiaBtn.bounds = sf::FloatRect(sbPos, sf::Vector2f(sbW, sbH));
        m_submitDiaBtn.label = "SUBMIT DIAMETER";
        m_submitDiaBtn.draw(target, theme);

        // Numpad 0-9 and backspace
        float npW = (boxW - 8.0f) / 5.0f;
        float npH = 24.0f;
        float npStartY = py + 80.0f + boxH + 24.0f + sbH + 6.0f;
        m_numpadBtns.clear();
        for (int i = 0; i <= 9; i++)
        {
            int col = i % 5;
            int row = i / 5;
            Button nb;
            nb.bounds = sf::FloatRect(sf::Vector2f(px + 6.0f + col * (npW + 2.0f), npStartY + row * (npH + 2.0f)), sf::Vector2f(npW, npH));
            nb.label = std::to_string(i);
            nb.draw(target, theme);
            m_numpadBtns.push_back({ nb, '0' + static_cast<char>(i) });
        }
        // Backspace button
        Button bkBtn;
        bkBtn.bounds = sf::FloatRect(sf::Vector2f(px + 6.0f + 5.0f * (npW + 2.0f) - npW, npStartY), sf::Vector2f(npW, npH));
        bkBtn.label = "<";
        bkBtn.draw(target, theme);
        m_numpadBtns.push_back({ bkBtn, '<' });
    }
}

void Act1Level3Screen::drawTooltip(sf::RenderTarget& target, const Theme& theme)
{
    if (!m_hoverNode) return;
    // In Level 1.3, hide the height line in the tooltip so it never shows or contradicts the puzzle answer.
    TreeRenderer::drawNodeTooltip(target, theme, m_rootNode, m_hoverNode, true);
}
