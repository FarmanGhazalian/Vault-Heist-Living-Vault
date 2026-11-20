#include "Act1Level2Screen.h"
#include "Act1Level3Screen.h"
#include "GameEngine.h"
#include <filesystem>
#include <cmath>

Act1Level2Screen::Act1Level2Screen(GameEngine& engine)
    : LevelScreenBase(engine, "ACT 1: THE BLUEPRINT WING", "LEVEL 1.2: DECODE THE BLUEPRINT")
    , m_draggedIndex(-1)
    , m_hoveredSlotIndex(-1)
    , m_shakeTimer(0.0f)
    , m_shakingTokenVal(-1)
{
    resetLevel();
}

void Act1Level2Screen::resetLevel()
{
    m_tokens.clear();
    m_slots.clear();
    m_preorder.clear();
    m_inorder.clear();
    m_draggedIndex = -1;
    m_hoveredSlotIndex = -1;
    m_shakeTimer = 0.0f;
    m_shakingTokenVal = -1;

    std::string path = "Act1Level2.json";
    if (!std::filesystem::exists(path))
    {
        path = "VaultHeist/Act1Level2.json";
    }

    if (m_bridge.loadLevel(1, 2, path))
    {
        setInstruction(m_bridge.getObjective());

        int total = m_bridge.getTotalNodes();
        for (int i = 0; i < total; ++i)
        {
            m_preorder.push_back(m_bridge.getPreorderValue(i));
            m_inorder.push_back(m_bridge.getInorderValue(i));

            TrayToken tok;
            tok.val = m_preorder.back();
            tok.homePos = sf::Vector2f(0.0f, 0.0f);
            tok.currPos = sf::Vector2f(0.0f, 0.0f);
            tok.isPlaced = false;
            tok.isDragging = false;
            tok.returnTimer = 0.0f;
            tok.returnStart = sf::Vector2f(0.0f, 0.0f);
            m_tokens.push_back(tok);
        }

        // Initial root slot
        TreeSlot rootSlot;
        rootSlot.parentVal = -1;
        rootSlot.direction = -1;
        rootSlot.pos = sf::Vector2f(0.0f, 0.0f);
        rootSlot.placedVal = -1;
        rootSlot.active = true;
        m_slots.push_back(rootSlot);

        sf::Vector2f size = m_engine.getWindowSize();
        layoutTrayAndSlots(static_cast<unsigned int>(size.x), static_cast<unsigned int>(size.y));
        syncWithState();
    }
}

void Act1Level2Screen::layoutTrayAndSlots(unsigned int width, unsigned int height)
{
    (void)width;
    float h = static_cast<float>(height);

    // Layout token tray inside canvas bottom
    float trayY = h - 65.0f;
    float tokenSpacing = 52.0f;
    float trayStartX = m_canvasBounds.position.x + 35.0f;

    for (size_t i = 0; i < m_tokens.size(); ++i)
    {
        sf::Vector2f home(trayStartX + (i * tokenSpacing), trayY);
        m_tokens[i].homePos = home;
        if (!m_tokens[i].isDragging && m_tokens[i].returnTimer <= 0.0f && !m_tokens[i].isPlaced)
        {
            m_tokens[i].currPos = home;
        }
    }

    updateSlotPositions();
}

void Act1Level2Screen::updateSlotPositions()
{
    if (m_slots.empty()) return;

    float rootX = m_canvasBounds.position.x + (m_canvasBounds.size.x * 0.5f);
    m_slots[0].pos = sf::Vector2f(rootX, m_canvasBounds.position.y + 60.0f);

    for (size_t i = 0; i < m_slots.size(); ++i)
    {
        if (m_slots[i].placedVal != -1)
        {
            for (size_t j = 0; j < m_slots.size(); ++j)
            {
                if (m_slots[j].parentVal == m_slots[i].placedVal)
                {
                    float xOffset = (m_slots[j].direction == 0) ? -75.0f : 75.0f;
                    m_slots[j].pos = m_slots[i].pos + sf::Vector2f(xOffset, 70.0f);
                }
            }
        }
    }
}

int Act1Level2Screen::getSlotAtPosition(sf::Vector2f pos)
{
    float radiusSq = 28.0f * 28.0f;
    for (size_t i = 0; i < m_slots.size(); ++i)
    {
        if (m_slots[i].active && m_slots[i].placedVal == -1)
        {
            float dx = pos.x - m_slots[i].pos.x;
            float dy = pos.y - m_slots[i].pos.y;
            if ((dx * dx + dy * dy) <= radiusSq)
            {
                return static_cast<int>(i);
            }
        }
    }
    return -1;
}

void Act1Level2Screen::syncWithState()
{
    const LevelState& state = m_bridge.getState();
    setSteps(state.steps, state.optimalSteps);
    setScore(ScoreSystem::score(state));
    setPenalty(state.penaltyPoints);
    setAlarm(state.alarm, state.alarmCap);
}

std::string Act1Level2Screen::getLevelSpecificHint()
{
    return m_bridge.getHint();
}

ILevelLogic* Act1Level2Screen::getLevelLogic()
{
    return m_bridge.getLogic();
}

void Act1Level2Screen::onResetClicked()
{
    LevelScreenBase::onResetClicked();
    resetLevel();
}

void Act1Level2Screen::reapplyActionSilently(const PlayerAction& act)
{
    ActionResult res = m_bridge.processAction(act);
    syncWithState();

    if (res.correct)
    {
        // Find matching token and slot, apply placement visually
        for (auto& tok : m_tokens)
        {
            if (tok.val == act.nodeB && !tok.isPlaced)
            {
                for (auto& slot : m_slots)
                {
                    if (slot.parentVal == act.nodeA && slot.direction == act.direction && slot.placedVal == -1)
                    {
                        tok.isPlaced = true;
                        tok.currPos = slot.pos;
                        slot.placedVal = tok.val;

                        TreeSlot leftSlot{ tok.val, 0, slot.pos + sf::Vector2f(-75.0f, 70.0f), -1, true };
                        TreeSlot rightSlot{ tok.val, 1, slot.pos + sf::Vector2f(75.0f, 70.0f), -1, true };
                        m_slots.push_back(leftSlot);
                        m_slots.push_back(rightSlot);
                        break;
                    }
                }
                break;
            }
        }
    }
}

void Act1Level2Screen::handleEvent(const sf::Event& event)
{
    LevelScreenBase::handleEvent(event);

    if (auto* resized = event.getIf<sf::Event::Resized>())
    {
        layoutTrayAndSlots(resized->size.x, resized->size.y);
    }

    if (auto* mouseBtn = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mouseBtn->button == sf::Mouse::Button::Left && !m_bridge.isFinished())
        {
            sf::Vector2f mpos(static_cast<float>(mouseBtn->position.x), static_cast<float>(mouseBtn->position.y));

            for (size_t i = 0; i < m_tokens.size(); ++i)
            {
                if (!m_tokens[i].isPlaced && m_tokens[i].returnTimer <= 0.0f)
                {
                    float dx = mpos.x - m_tokens[i].currPos.x;
                    float dy = mpos.y - m_tokens[i].currPos.y;
                    if ((dx * dx + dy * dy) <= (26.0f * 26.0f))
                    {
                        m_draggedIndex = static_cast<int>(i);
                        m_tokens[i].isDragging = true;
                        m_engine.audio.playClick();
                        break;
                    }
                }
            }
        }
    }

    if (auto* mouseMove = event.getIf<sf::Event::MouseMoved>())
    {
        sf::Vector2f mpos(static_cast<float>(mouseMove->position.x), static_cast<float>(mouseMove->position.y));
        if (m_draggedIndex != -1)
        {
            m_tokens[m_draggedIndex].currPos = mpos;
            m_hoveredSlotIndex = getSlotAtPosition(mpos);
        }
    }

    if (auto* mouseRelease = event.getIf<sf::Event::MouseButtonReleased>())
    {
        if (mouseRelease->button == sf::Mouse::Button::Left && m_draggedIndex != -1)
        {
            TrayToken& tok = m_tokens[m_draggedIndex];
            tok.isDragging = false;

            int targetSlot = getSlotAtPosition(tok.currPos);
            if (targetSlot != -1)
            {
                TreeSlot& slot = m_slots[targetSlot];

                PlayerAction action;
                action.type = PLACE_TOKEN;
                action.nodeA = slot.parentVal;
                action.direction = slot.direction;
                action.nodeB = tok.val;

                ActionResult res = m_bridge.processAction(action);
                syncWithState();

                if (res.correct)
                {
                    recordSharedAction(action);

                    tok.isPlaced = true;
                    tok.currPos = slot.pos;
                    slot.placedVal = tok.val;

                    // Spawn child slots
                    TreeSlot leftSlot{ tok.val, 0, slot.pos + sf::Vector2f(-75.0f, 70.0f), -1, true };
                    TreeSlot rightSlot{ tok.val, 1, slot.pos + sf::Vector2f(75.0f, 70.0f), -1, true };
                    m_slots.push_back(leftSlot);
                    m_slots.push_back(rightSlot);

                    m_engine.audio.playCorrect();

                    if (m_bridge.isWon())
                    {
                        m_engine.audio.playWin();
                        const LevelState& s = m_bridge.getState();
                        showResultModal(true, ScoreSystem::stars(s), s.steps, s.optimalSteps, s.penaltyPoints, ScoreSystem::score(s),
                            [this]() {
                                m_engine.changeScreen(std::make_unique<Act1Level3Screen>(m_engine));
                            },
                            [this]() { resetLevel(); },
                            [this]() { onBackClicked(); }
                        );
                    }
                }
                else
                {
                    m_shakingTokenVal = tok.val;
                    m_shakeTimer = 0.35f;

                    tok.returnTimer = 0.3f;
                    tok.returnStart = tok.currPos;

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
                tok.returnTimer = 0.25f;
                tok.returnStart = tok.currPos;
            }

            m_draggedIndex = -1;
            m_hoveredSlotIndex = -1;
        }
    }
}

void Act1Level2Screen::update(float dt)
{
    LevelScreenBase::update(dt);

    if (m_shakeTimer > 0.0f)
    {
        m_shakeTimer -= dt;
        if (m_shakeTimer <= 0.0f)
        {
            m_shakingTokenVal = -1;
        }
    }

    for (auto& tok : m_tokens)
    {
        if (tok.returnTimer > 0.0f)
        {
            tok.returnTimer -= dt;
            float progress = 1.0f - std::clamp(tok.returnTimer / 0.3f, 0.0f, 1.0f);
            float ease = Tween::easeOutQuad(progress);
            tok.currPos = sf::Vector2f(
                Tween::lerp(tok.returnStart.x, tok.homePos.x, ease),
                Tween::lerp(tok.returnStart.y, tok.homePos.y, ease)
            );
        }
    }
}

void Act1Level2Screen::onCustomDraw(sf::RenderTarget& target)
{
    const Theme& theme = m_engine.theme;

    // Draw lines between parent and child slots
    for (const auto& slot : m_slots)
    {
        if (slot.parentVal != -1)
        {
            for (const auto& parentSlot : m_slots)
            {
                if (parentSlot.placedVal == slot.parentVal)
                {
                    sf::Vertex line[2] = {
                        sf::Vertex(parentSlot.pos, sf::Color(0, 240, 255, 90)),
                        sf::Vertex(slot.pos, sf::Color(0, 240, 255, 90))
                    };
                    target.draw(line, 2, sf::PrimitiveType::Lines);
                    break;
                }
            }
        }
    }

    // Draw tree slots
    for (size_t i = 0; i < m_slots.size(); ++i)
    {
        const auto& slot = m_slots[i];
        if (!slot.active) continue;

        bool isHovered = (static_cast<int>(i) == m_hoveredSlotIndex);
        float r = 26.0f;

        sf::CircleShape slotCircle(r);
        slotCircle.setOrigin(sf::Vector2f(r, r));
        slotCircle.setPosition(slot.pos);

        if (slot.placedVal != -1)
        {
            slotCircle.setFillColor(Theme::NodeDarkFill);
            slotCircle.setOutlineThickness(2.0f);
            slotCircle.setOutlineColor(Theme::SuccessEmerald);
            target.draw(slotCircle);

            sf::Text valText(theme.fontMono, std::to_string(slot.placedVal), 16);
            valText.setFillColor(sf::Color::White);
            valText.setStyle(sf::Text::Bold);
            sf::FloatRect b = valText.getLocalBounds();
            valText.setOrigin(sf::Vector2f(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f));
            valText.setPosition(slot.pos);
            target.draw(valText);
        }
        else
        {
            slotCircle.setFillColor(sf::Color(19, 27, 46, 120));
            slotCircle.setOutlineThickness(isHovered ? 2.5f : 1.2f);
            slotCircle.setOutlineColor(isHovered ? Theme::AccentYellow : sf::Color(0, 240, 255, 120));
            target.draw(slotCircle);

            sf::Text plusText(theme.fontMono, isHovered ? "DROP" : "+", isHovered ? 9 : 14);
            plusText.setFillColor(isHovered ? Theme::AccentYellow : sf::Color(0, 240, 255, 160));
            sf::FloatRect b = plusText.getLocalBounds();
            plusText.setOrigin(sf::Vector2f(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f));
            plusText.setPosition(slot.pos);
            target.draw(plusText);
        }
    }

    // Draw draggable tokens
    for (size_t i = 0; i < m_tokens.size(); ++i)
    {
        const auto& tok = m_tokens[i];
        if (tok.isPlaced) continue;

        sf::Vector2f drawPos = tok.currPos;
        if (tok.val == m_shakingTokenVal && m_shakeTimer > 0.0f)
        {
            drawPos += Tween::shake(m_shakeTimer, 6.0f);
        }

        float r = tok.isDragging ? 26.0f : 21.0f;
        sf::CircleShape tokCircle(r);
        tokCircle.setOrigin(sf::Vector2f(r, r));
        tokCircle.setPosition(drawPos);
        tokCircle.setFillColor(Theme::NodeDarkFill);
        tokCircle.setOutlineThickness(tok.isDragging ? 2.5f : 1.5f);
        tokCircle.setOutlineColor(tok.val == m_shakingTokenVal ? Theme::CrimsonAlarm : Theme::BorderCyan);
        target.draw(tokCircle);

        sf::Text numText(theme.fontMono, std::to_string(tok.val), tok.isDragging ? 16 : 13);
        numText.setFillColor(sf::Color::White);
        numText.setStyle(sf::Text::Bold);
        sf::FloatRect b = numText.getLocalBounds();
        numText.setOrigin(sf::Vector2f(b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f));
        numText.setPosition(drawPos);
        target.draw(numText);
    }
}

void Act1Level2Screen::onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)
{
    const Theme& theme = m_engine.theme;
    float px = panelBounds.position.x;
    float py = panelBounds.position.y;

    sf::Text hdr(theme.fontMono, "BLUEPRINT SEQUENCES", 11);
    hdr.setFillColor(Theme::BorderCyan);
    hdr.setStyle(sf::Text::Bold);
    hdr.setPosition(sf::Vector2f(px + 4.0f, py + 4.0f));
    target.draw(hdr);

    std::string preStr = "PREORDER:\n[ ";
    for (size_t i = 0; i < m_preorder.size(); ++i) preStr += std::to_string(m_preorder[i]) + (i + 1 < m_preorder.size() ? ", " : " ]");
    sf::Text tPre(theme.fontMono, preStr, 11);
    tPre.setFillColor(Theme::AccentYellow);
    tPre.setPosition(sf::Vector2f(px + 4.0f, py + 26.0f));
    target.draw(tPre);

    std::string inStr = "INORDER:\n[ ";
    for (size_t i = 0; i < m_inorder.size(); ++i) inStr += std::to_string(m_inorder[i]) + (i + 1 < m_inorder.size() ? ", " : " ]");
    sf::Text tIn(theme.fontMono, inStr, 11);
    tIn.setFillColor(Theme::BorderCyan);
    tIn.setPosition(sf::Vector2f(px + 4.0f, py + 72.0f));
    target.draw(tIn);

    int placedCount = 0;
    for (const auto& t : m_tokens) if (t.isPlaced) placedCount++;
    std::string progStr = "RECONSTRUCTION:\n" + std::to_string(placedCount) + " / " + std::to_string(m_tokens.size()) + " Nodes Locked";
    sf::Text tProg(theme.fontMono, progStr, 12);
    tProg.setFillColor(Theme::SuccessEmerald);
    tProg.setPosition(sf::Vector2f(px + 4.0f, py + 124.0f));
    target.draw(tProg);
}
