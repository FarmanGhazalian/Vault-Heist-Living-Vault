#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include <vector>
#include <functional>

namespace Theme {
    const sf::Color BgDark(10, 14, 26);         // #0A0E1A
    const sf::Color PanelBg(18, 26, 46);       // #121A2E
    const sf::Color PanelBorder(38, 50, 77);   // #26324D
    const sf::Color AccentCyan(56, 189, 248);  // #38BDF8
    const sf::Color Gold(212, 167, 44);        // #D4A72C
    const sf::Color Green(34, 160, 107);       // #22A06B
    const sf::Color Red(214, 69, 69);          // #D64545
    const sf::Color Grey(91, 103, 133);        // #5B6785
    const sf::Color TextWhite(230, 234, 242);  // #E6EAF2
    const sf::Color TextMuted(139, 150, 173);  // #8B96AD
}

namespace ButtonStyle {
    constexpr int PRIMARY = 0;   // Accent Cyan border
    constexpr int SECONDARY = 1; // Grey border
    constexpr int DANGER = 2;    // Red border
    constexpr int GOLD = 3;      // Gold border
}

namespace ToastType {
    constexpr int HINT = 0;      // Gold
    constexpr int CORRECT = 1;   // Green
    constexpr int ERROR_MSG = 2; // Red
    constexpr int INFO = 3;      // Cyan
}

namespace PopupType {
    constexpr int NONE = 0;
    constexpr int COMING_SOON = 1;
    constexpr int PAUSE = 2;
    constexpr int RESULT = 3;
}

namespace PopupAction {
    constexpr int NONE = 0;
    constexpr int OK = 1;
    constexpr int RETRY = 2;
    constexpr int NEXT = 3;
    constexpr int MENU = 4;
    constexpr int CANCEL = 5;
}

class Tween {
public:
    static float easeOutQuad(float t);
    static float easeOutCubic(float t);
    static float easeInOutQuad(float t);
};

class Button {
public:
    sf::FloatRect bounds;
    std::string text;
    int style = ButtonStyle::PRIMARY;
    bool isHovered = false;
    bool isPressed = false;
    bool isLocked = false;
    std::string tagText = "";

    Button() = default;
    Button(const sf::FloatRect& b, const std::string& t, int s = ButtonStyle::PRIMARY);

    void update(const sf::Vector2f& mousePos);
    bool handleClick(const sf::Vector2f& mousePos);
    void draw(sf::RenderWindow& window, const sf::Font& font) const;
};

struct ToastItem {
    std::string title;
    std::string message;
    int type = ToastType::INFO;
    float timer = 3.5f;
    float totalTime = 3.5f;
    float currentY = -80.0f;
    float targetY = 20.0f;
};

class ToastManager {
public:
    std::vector<ToastItem> toasts;

    void add(const std::string& title, const std::string& message, int type);
    void update(float dt);
    void draw(sf::RenderWindow& window, const sf::Font& font);
    bool handleClick(const sf::Vector2f& mousePos);
    void clear();
};

struct LevelResultData {
    std::string levelName;
    int steps = 0;
    int optimalSteps = 0;
    int alarm = 0;
    int alarmCap = 0;
    int penaltyCount = 0;
    int penaltyPoints = 0;
    bool hintUsed = false;
    int stars = 0;
    int score = 0;
    bool isWon = false;
    bool isLastLevel = false;
};

class ModalPopup {
public:
    int type = PopupType::NONE;
    std::string title;
    std::string message;
    LevelResultData result;

    float animTimer = 0.0f;
    bool active = false;
    int starThudsPlayed = 0;

    Button btn1;
    Button btn2;
    Button btn3;

    void showComingSoon(const std::string& featureName);
    void showPause();
    void showResult(const LevelResultData& data);
    void close();

    void update(float dt, const sf::Vector2f& mousePos, class SoundManager& audio);
    int handleEvent(const sf::Event& event, const sf::Vector2f& mousePos);
    void draw(sf::RenderWindow& window, const sf::Font& font);
};

void drawMetallicMedallion(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f center, float radius, bool isEarned, float scale, int index);
void drawBackgroundGrid(sf::RenderWindow& window, float width, float height, float time);
