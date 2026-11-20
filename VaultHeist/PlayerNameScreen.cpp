#include "PlayerNameScreen.h"
#include "MainMenuScreen.h"
#include "GameEngine.h"
#include <algorithm>

PlayerNameScreen::PlayerNameScreen(GameEngine& gameEngine)
    : engine(gameEngine),
      windowSize(gameEngine.getWindowSize()),
      m_inputText("Player"),
      btnConfirm("CONFIRM IDENTITY", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(200.0f, 42.0f)), BTN_PRIMARY),
      m_cursorTimer(0.0f),
      m_cursorVisible(true)
{
    updateLayout(windowSize.x, windowSize.y);
}

void PlayerNameScreen::updateLayout(float w, float h)
{
    windowSize = sf::Vector2f(w, h);
    float btnW = 200.0f;
    float btnH = 42.0f;
    float btnX = std::round((w - btnW) * 0.5f);
    float btnY = std::round(h * 0.5f + 80.0f);
    btnConfirm.bounds = sf::FloatRect(sf::Vector2f(btnX, btnY), sf::Vector2f(btnW, btnH));
}

void PlayerNameScreen::onResize(unsigned int width, unsigned int height)
{
    updateLayout(static_cast<float>(width), static_cast<float>(height));
}

void PlayerNameScreen::handleEvent(const sf::Event& event)
{
    btnConfirm.handleEvent(event);

    if (btnConfirm.isClicked())
    {
        btnConfirm.resetClick();
        if (m_inputText.empty())
        {
            m_inputText = "Player";
        }
        GameEngine::s_playerName = m_inputText;
        engine.audio.playClick();
        engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
        return;
    }

    if (const auto* te = event.getIf<sf::Event::TextEntered>())
    {
        uint32_t unicode = te->unicode;
        if (unicode == 8) // Backspace
        {
            if (!m_inputText.empty())
            {
                m_inputText.pop_back();
            }
        }
        else if (unicode == 13) // Enter
        {
            if (m_inputText.empty())
            {
                m_inputText = "Player";
            }
            GameEngine::s_playerName = m_inputText;
            engine.audio.playClick();
            engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
            return;
        }
        else if (unicode >= 32 && unicode <= 126) // Printable ASCII
        {
            if (m_inputText.length() < 12)
            {
                m_inputText.push_back(static_cast<char>(unicode));
            }
        }
    }
}

void PlayerNameScreen::update(float dt)
{
    m_cursorTimer += dt;
    if (m_cursorTimer >= 0.5f)
    {
        m_cursorTimer = 0.0f;
        m_cursorVisible = !m_cursorVisible;
    }
}

void PlayerNameScreen::draw(sf::RenderTarget& target)
{
    float cx = windowSize.x * 0.5f;
    float cy = windowSize.y * 0.5f;

    // Background panel
    float pw = 480.0f;
    float ph = 280.0f;
    float px = std::round(cx - pw * 0.5f);
    float py = std::round(cy - ph * 0.5f);
    sf::FloatRect panelBounds(sf::Vector2f(px, py), sf::Vector2f(pw, ph));

    engine.theme.drawPanel(target, panelBounds, Theme::BorderCyan, 1.5f);
    engine.theme.drawGlowOutline(target, panelBounds, Theme::BorderCyan, 0.8f);

    // Header Title
    sf::Text title(engine.theme.fontUI, "AGENT IDENTIFICATION", 22);
    title.setFillColor(Theme::BorderCyan);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(sf::Vector2f(std::round(cx - tb.size.x * 0.5f - tb.position.x), py + 24.0f));
    target.draw(title);

    // Subtitle
    sf::Text sub(engine.theme.fontUI, "Enter Agent Codename - Max 12 Characters:", 14);
    sub.setFillColor(Theme::TextMuted);
    sf::FloatRect sb = sub.getLocalBounds();
    sub.setPosition(sf::Vector2f(std::round(cx - sb.size.x * 0.5f - sb.position.x), py + 62.0f));
    target.draw(sub);

    // Input box
    float ibW = 320.0f;
    float ibH = 46.0f;
    float ibX = std::round(cx - ibW * 0.5f);
    float ibY = py + 100.0f;

    sf::RectangleShape ibBox(sf::Vector2f(ibW, ibH));
    ibBox.setPosition(sf::Vector2f(ibX, ibY));
    ibBox.setFillColor(sf::Color(10, 14, 23, 240));
    ibBox.setOutlineThickness(1.5f);
    ibBox.setOutlineColor(Theme::BorderCyan);
    target.draw(ibBox);

    // Input text with blinking cursor
    std::string disp = m_inputText + (m_cursorVisible ? "_" : " ");
    sf::Text inText(engine.theme.fontMono, disp, 20);
    inText.setFillColor(Theme::AccentYellow);
    sf::FloatRect itb = inText.getLocalBounds();
    inText.setPosition(sf::Vector2f(std::round(cx - itb.size.x * 0.5f - itb.position.x), ibY + 11.0f));
    target.draw(inText);

    // Confirm button
    btnConfirm.draw(target, engine.theme);
}
