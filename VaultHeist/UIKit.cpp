#include "UIKit.h"
#include "SoundManager.h"
#include <cmath>
#include <algorithm>
#include <iomanip>
#include <sstream>

// -------------------------------------------------------------
// Tween
// -------------------------------------------------------------
float Tween::easeOutQuad(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return 1.0f - (1.0f - t) * (1.0f - t);
}

float Tween::easeOutCubic(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    float f = t - 1.0f;
    return f * f * f + 1.0f;
}

float Tween::easeInOutQuad(float t) {
    t = std::clamp(t, 0.0f, 1.0f);
    return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f;
}

// -------------------------------------------------------------
// Button
// -------------------------------------------------------------
Button::Button(const sf::FloatRect& b, const std::string& t, int s)
    : bounds(b), text(t), style(s) {}

void Button::update(const sf::Vector2f& mousePos) {
    if (isLocked) {
        isHovered = false;
        return;
    }
    isHovered = bounds.contains(mousePos);
}

bool Button::handleClick(const sf::Vector2f& mousePos) {
    if (isLocked) return false;
    return bounds.contains(mousePos);
}

void Button::draw(sf::RenderWindow& window, const sf::Font& font) const {
    sf::RectangleShape rect({bounds.size.x, bounds.size.y});
    rect.setPosition({bounds.position.x, bounds.position.y});

    // Dark glass fill
    if (isLocked) {
        rect.setFillColor(sf::Color(14, 20, 36, 180));
    } else if (isHovered) {
        rect.setFillColor(sf::Color(26, 38, 66, 230));
    } else {
        rect.setFillColor(sf::Color(18, 26, 46, 220));
    }

    // 1px Border
    rect.setOutlineThickness(1.0f);
    if (isLocked) {
        rect.setOutlineColor(sf::Color(51, 65, 85, 120));
    } else {
        sf::Color borderCol = Theme::PanelBorder;
        if (style == ButtonStyle::PRIMARY) borderCol = isHovered ? sf::Color(125, 211, 252) : Theme::AccentCyan;
        else if (style == ButtonStyle::SECONDARY) borderCol = isHovered ? sf::Color(148, 163, 184) : Theme::Grey;
        else if (style == ButtonStyle::DANGER) borderCol = isHovered ? sf::Color(248, 113, 113) : Theme::Red;
        else if (style == ButtonStyle::GOLD) borderCol = isHovered ? sf::Color(250, 204, 21) : Theme::Gold;
        rect.setOutlineColor(borderCol);
    }
    window.draw(rect);

    // Optional tag on top
    if (!tagText.empty()) {
        sf::Text tag(font, tagText, 11);
        tag.setStyle(sf::Text::Bold);
        tag.setFillColor(isLocked ? sf::Color(100, 116, 139) : Theme::AccentCyan);
        tag.setPosition({bounds.position.x + 16.0f, bounds.position.y + 10.0f});
        window.draw(tag);
    }

    // Button label
    unsigned int charSize = tagText.empty() ? 15 : 14;
    sf::Text txt(font, text, charSize);
    txt.setStyle(sf::Text::Bold);
    if (isLocked) {
        txt.setFillColor(sf::Color(100, 116, 139));
    } else if (isHovered) {
        txt.setFillColor(sf::Color::White);
    } else {
        txt.setFillColor(Theme::TextWhite);
    }

    auto tb = txt.getLocalBounds();
    float txtY = tagText.empty() ? (bounds.position.y + bounds.size.y * 0.5f - tb.size.y * 0.5f - tb.position.y)
                                 : (bounds.position.y + bounds.size.y * 0.62f - tb.size.y * 0.5f - tb.position.y);
    float txtX = bounds.position.x + bounds.size.x * 0.5f - tb.size.x * 0.5f - tb.position.x;
    txt.setPosition({txtX, txtY});
    window.draw(txt);

    // Small lock tag if locked
    if (isLocked) {
        sf::Text lockTxt(font, "[LOCKED]", 11);
        lockTxt.setFillColor(sf::Color(148, 163, 184));
        auto lb = lockTxt.getLocalBounds();
        lockTxt.setPosition({bounds.position.x + bounds.size.x - lb.size.x - 16.0f, bounds.position.y + 10.0f});
        window.draw(lockTxt);
    }
}

// -------------------------------------------------------------
// ToastManager
// -------------------------------------------------------------
void ToastManager::add(const std::string& title, const std::string& message, int type) {
    if (toasts.size() >= 3) {
        toasts.erase(toasts.begin());
    }
    ToastItem item;
    item.title = title;
    item.message = message;
    item.type = type;
    item.timer = 3.5f;
    item.totalTime = 3.5f;
    item.currentY = -70.0f;
    item.targetY = 20.0f + static_cast<float>(toasts.size()) * 74.0f;
    toasts.push_back(item);
}

void ToastManager::update(float dt) {
    for (auto it = toasts.begin(); it != toasts.end();) {
        it->timer -= dt;
        if (it->timer <= 0.0f) {
            it = toasts.erase(it);
        } else {
            // Smooth slide down
            float progress = 1.0f - (it->timer / it->totalTime);
            if (progress < 0.15f) {
                float inT = progress / 0.15f;
                it->currentY = -70.0f + (it->targetY + 70.0f) * Tween::easeOutQuad(inT);
            } else if (it->timer < 0.3f) {
                float outT = 1.0f - (it->timer / 0.3f);
                it->currentY = it->targetY - (it->targetY + 80.0f) * outT;
            } else {
                it->currentY = it->targetY;
            }
            ++it;
        }
    }
}

void ToastManager::draw(sf::RenderWindow& window, const sf::Font& font) {
    float width = 460.0f;
    float height = 62.0f;
    float startX = (window.getSize().x - width) * 0.5f;

    for (const auto& t : toasts) {
        // Container card
        sf::RectangleShape card({width, height});
        card.setPosition({startX, t.currentY});
        card.setFillColor(sf::Color(18, 26, 46, 245));
        card.setOutlineThickness(1.0f);
        card.setOutlineColor(Theme::PanelBorder);
        window.draw(card);

        // Thin colored left bar
        sf::Color barColor = Theme::AccentCyan;
        if (t.type == ToastType::HINT) barColor = Theme::Gold;
        else if (t.type == ToastType::CORRECT) barColor = Theme::Green;
        else if (t.type == ToastType::ERROR_MSG) barColor = Theme::Red;

        sf::RectangleShape bar({4.0f, height});
        bar.setPosition({startX, t.currentY});
        bar.setFillColor(barColor);
        window.draw(bar);

        // Title
        sf::Text title(font, t.title, 13);
        title.setStyle(sf::Text::Bold);
        title.setFillColor(barColor);
        title.setPosition({startX + 16.0f, t.currentY + 10.0f});
        window.draw(title);

        // Message
        sf::Text msg(font, t.message, 12);
        msg.setFillColor(Theme::TextWhite);
        msg.setPosition({startX + 16.0f, t.currentY + 30.0f});
        window.draw(msg);

        // Remaining time line at bottom
        float linePct = std::clamp(t.timer / t.totalTime, 0.0f, 1.0f);
        sf::RectangleShape timeLine({(width - 4.0f) * linePct, 2.0f});
        timeLine.setPosition({startX + 4.0f, t.currentY + height - 2.0f});
        timeLine.setFillColor(sf::Color(barColor.r, barColor.g, barColor.b, 160));
        window.draw(timeLine);
    }
}

bool ToastManager::handleClick(const sf::Vector2f& mousePos) {
    float width = 460.0f;
    float height = 62.0f;
    float startX = 520.0f; // Approx center on 1500w
    for (auto it = toasts.begin(); it != toasts.end(); ++it) {
        sf::FloatRect b({startX, it->currentY}, {width, height});
        if (b.contains(mousePos)) {
            toasts.erase(it);
            return true;
        }
    }
    return false;
}

void ToastManager::clear() {
    toasts.clear();
}

// -------------------------------------------------------------
// Medallions
// -------------------------------------------------------------
void drawMetallicMedallion(sf::RenderWindow& window, const sf::Font& font, sf::Vector2f center, float radius, bool isEarned, float scale, int index) {
    float r = radius * scale;

    // Outer metal rim
    sf::CircleShape outer(r);
    outer.setOrigin({r, r});
    outer.setPosition(center);
    if (isEarned) {
        outer.setFillColor(sf::Color(180, 130, 20));
        outer.setOutlineThickness(2.0f * scale);
        outer.setOutlineColor(sf::Color(250, 204, 21));
    } else {
        outer.setFillColor(sf::Color(30, 41, 59));
        outer.setOutlineThickness(1.5f * scale);
        outer.setOutlineColor(sf::Color(71, 85, 105));
    }
    window.draw(outer);

    // Inner bevel
    sf::CircleShape inner(r * 0.82f);
    inner.setOrigin({r * 0.82f, r * 0.82f});
    inner.setPosition(center);
    if (isEarned) {
        inner.setFillColor(sf::Color(212, 167, 44));
    } else {
        inner.setFillColor(sf::Color(40, 53, 76));
    }
    window.draw(inner);

    // Roman numeral or star emboss
    std::string roman = (index == 0) ? "I" : (index == 1 ? "II" : "III");
    sf::Text t(font, roman, static_cast<unsigned int>(16 * scale));
    t.setStyle(sf::Text::Bold);
    t.setFillColor(isEarned ? sf::Color(254, 240, 138) : sf::Color(100, 116, 139));
    auto tb = t.getLocalBounds();
    t.setOrigin({tb.position.x + tb.size.x * 0.5f, tb.position.y + tb.size.y * 0.5f});
    t.setPosition(center);
    window.draw(t);
}

// -------------------------------------------------------------
// ModalPopup
// -------------------------------------------------------------
void ModalPopup::showComingSoon(const std::string& featureName) {
    type = PopupType::COMING_SOON;
    title = "CLASSIFIED ARCHIVE";
    message = featureName + " is not built yet.\nIt is coming soon in the next wing deployment!";
    active = true;
    animTimer = 0.0f;
    btn1 = Button({{670.0f, 495.0f}, {160.0f, 44.0f}}, "DISMISS", ButtonStyle::PRIMARY);
}

void ModalPopup::showPause() {
    type = PopupType::PAUSE;
    title = "OPERATION PAUSED";
    message = "Abort current vault operation and return to command center?";
    active = true;
    animTimer = 0.0f;
    btn1 = Button({{600.0f, 490.0f}, {140.0f, 44.0f}}, "RESUME", ButtonStyle::PRIMARY);
    btn2 = Button({{760.0f, 490.0f}, {140.0f, 44.0f}}, "QUIT", ButtonStyle::DANGER);
}

void ModalPopup::showResult(const LevelResultData& data) {
    type = PopupType::RESULT;
    result = data;
    title = data.isWon ? "VAULT CRACKED" : "SECURITY LOCKDOWN";
    message = data.isWon ? "Objective accomplished. Security nodes bypassed."
                         : "The alarm reached its limit. Security seals engaged.";
    active = true;
    animTimer = 0.0f;
    starThudsPlayed = 0;

    btn1 = Button({{510.0f, 605.0f}, {140.0f, 44.0f}}, "RETRY", ButtonStyle::SECONDARY);
    btn2 = Button({{670.0f, 605.0f}, {160.0f, 44.0f}}, data.isLastLevel ? "TO BLUEPRINT" : "NEXT LEVEL", ButtonStyle::GOLD);
    btn3 = Button({{850.0f, 605.0f}, {140.0f, 44.0f}}, "MENU", ButtonStyle::SECONDARY);
}

void ModalPopup::close() {
    active = false;
    type = PopupType::NONE;
}

void ModalPopup::update(float dt, const sf::Vector2f& mousePos, SoundManager& audio) {
    if (!active) return;
    animTimer += dt;

    if (type == PopupType::RESULT) {
        // Sequential medallion thuds
        for (int i = 0; i < 3; ++i) {
            float triggerTime = 0.35f + static_cast<float>(i) * 0.25f;
            if (animTimer >= triggerTime && starThudsPlayed == i) {
                starThudsPlayed++;
                if (i < result.stars) audio.playThud();
            }
        }
    }

    btn1.update(mousePos);
    btn2.update(mousePos);
    btn3.update(mousePos);
}

int ModalPopup::handleEvent(const sf::Event& event, const sf::Vector2f& mousePos) {
    if (!active) return PopupAction::NONE;

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape) {
            if (type == PopupType::PAUSE) { close(); return PopupAction::CANCEL; }
            if (type == PopupType::COMING_SOON) { close(); return PopupAction::OK; }
        }
    }

    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mp->button == sf::Mouse::Button::Left) {
            if (type == PopupType::COMING_SOON) {
                if (btn1.handleClick(mousePos)) { close(); return PopupAction::OK; }
            } else if (type == PopupType::PAUSE) {
                if (btn1.handleClick(mousePos)) { close(); return PopupAction::CANCEL; }
                if (btn2.handleClick(mousePos)) { close(); return PopupAction::MENU; }
            } else if (type == PopupType::RESULT) {
                if (btn1.handleClick(mousePos)) { close(); return PopupAction::RETRY; }
                if (btn2.handleClick(mousePos)) { close(); return PopupAction::NEXT; }
                if (btn3.handleClick(mousePos)) { close(); return PopupAction::MENU; }
            }
        }
    }
    return PopupAction::NONE;
}

void ModalPopup::draw(sf::RenderWindow& window, const sf::Font& font) {
    if (!active) return;

    float W = static_cast<float>(window.getSize().x);
    float H = static_cast<float>(window.getSize().y);

    // Dim background overlay
    sf::RectangleShape dim({W, H});
    dim.setFillColor(sf::Color(8, 12, 22, 215));
    window.draw(dim);

    float scale = Tween::easeOutCubic(std::min(1.0f, animTimer / 0.3f));
    float alpha = std::min(1.0f, animTimer / 0.25f);

    if (type == PopupType::COMING_SOON || type == PopupType::PAUSE) {
        float pW = 520.0f;
        float pH = 260.0f;
        float pX = (W - pW) * 0.5f;
        float pY = (H - pH) * 0.5f + (1.0f - scale) * 20.0f;

        // Dossier panel
        sf::RectangleShape panel({pW, pH});
        panel.setPosition({pX, pY});
        panel.setFillColor(sf::Color(18, 26, 46, static_cast<std::uint8_t>(250 * alpha)));
        panel.setOutlineThickness(1.0f);
        panel.setOutlineColor(Theme::PanelBorder);
        window.draw(panel);

        // Accent top bar
        sf::RectangleShape bar({pW, 3.0f});
        bar.setPosition({pX, pY});
        bar.setFillColor(Theme::AccentCyan);
        window.draw(bar);

        // Title
        sf::Text tTitle(font, title, 20);
        tTitle.setStyle(sf::Text::Bold);
        tTitle.setFillColor(Theme::AccentCyan);
        auto tb = tTitle.getLocalBounds();
        tTitle.setPosition({pX + (pW - tb.size.x) * 0.5f, pY + 36.0f});
        window.draw(tTitle);

        // Message
        sf::Text tMsg(font, message, 15);
        tMsg.setFillColor(Theme::TextWhite);
        auto mb = tMsg.getLocalBounds();
        tMsg.setPosition({pX + (pW - mb.size.x) * 0.5f, pY + 85.0f});
        window.draw(tMsg);

        btn1.bounds.position = {pX + (pW - btn1.bounds.size.x) * 0.5f, pY + pH - 65.0f};
        if (type == PopupType::PAUSE) {
            btn1.bounds.position = {pX + 80.0f, pY + pH - 65.0f};
            btn2.bounds.position = {pX + pW - 80.0f - btn2.bounds.size.x, pY + pH - 65.0f};
            btn1.draw(window, font);
            btn2.draw(window, font);
        } else {
            btn1.draw(window, font);
        }
    } else if (type == PopupType::RESULT) {
        float pW = 660.0f;
        float pH = 510.0f;
        float pX = (W - pW) * 0.5f;
        float pY = (H - pH) * 0.5f + (1.0f - scale) * 25.0f;

        // Metal Dossier Plate
        sf::RectangleShape panel({pW, pH});
        panel.setPosition({pX, pY});
        panel.setFillColor(sf::Color(18, 26, 46, static_cast<std::uint8_t>(252 * alpha)));
        panel.setOutlineThickness(1.0f);
        panel.setOutlineColor(result.isWon ? Theme::Gold : Theme::Red);
        window.draw(panel);

        // Header Accent line
        sf::RectangleShape bar({pW, 3.0f});
        bar.setPosition({pX, pY});
        bar.setFillColor(result.isWon ? Theme::Gold : Theme::Red);
        window.draw(bar);

        // Dossier header label
        sf::Text subH(font, "OPERATIONAL DEBRIEFING // LEVEL COMPLETION", 11);
        subH.setStyle(sf::Text::Bold);
        subH.setFillColor(Theme::TextMuted);
        subH.setPosition({pX + 35.0f, pY + 22.0f});
        window.draw(subH);

        // Header Title
        sf::Text tTitle(font, title, 26);
        tTitle.setStyle(sf::Text::Bold);
        tTitle.setFillColor(result.isWon ? Theme::Gold : Theme::Red);
        tTitle.setPosition({pX + 35.0f, pY + 42.0f});
        window.draw(tTitle);

        // Message
        sf::Text tMsg(font, message, 13);
        tMsg.setFillColor(Theme::TextWhite);
        tMsg.setPosition({pX + 35.0f, pY + 80.0f});
        window.draw(tMsg);

        // Three Stamped Metallic Medallions
        float medY = pY + 145.0f;
        float medX[3] = {pX + pW * 0.5f - 90.0f, pX + pW * 0.5f, pX + pW * 0.5f + 90.0f};
        for (int i = 0; i < 3; ++i) {
            float triggerTime = 0.35f + static_cast<float>(i) * 0.25f;
            float medScale = 0.0f;
            if (animTimer > triggerTime) {
                float prog = (animTimer - triggerTime) / 0.18f;
                if (prog >= 1.0f) medScale = 1.0f;
                else medScale = 1.5f - 0.5f * Tween::easeOutQuad(prog);
            }
            bool earned = (i < result.stars);
            drawMetallicMedallion(window, font, {medX[i], medY}, 28.0f, earned, medScale, i);
        }

        // Stats Box (2 Columns)
        sf::RectangleShape statBox({pW - 70.0f, 175.0f});
        statBox.setPosition({pX + 35.0f, pY + 205.0f});
        statBox.setFillColor(sf::Color(12, 17, 30, 220));
        statBox.setOutlineThickness(1.0f);
        statBox.setOutlineColor(Theme::PanelBorder);
        window.draw(statBox);

        // Column 1: Steps & Alarm
        float c1X = pX + 55.0f;
        float sY = pY + 225.0f;

        auto drawStatRow = [&](float x, float y, const std::string& lbl, const std::string& val, const sf::Color& valCol) {
            sf::Text tLbl(font, lbl, 13);
            tLbl.setFillColor(Theme::TextMuted);
            tLbl.setPosition({x, y});
            window.draw(tLbl);

            sf::Text tVal(font, val, 14);
            tVal.setStyle(sf::Text::Bold);
            tVal.setFillColor(valCol);
            tVal.setPosition({x + 130.0f, y});
            window.draw(tVal);
        };

        drawStatRow(c1X, sY, "LEVEL:", result.levelName, Theme::TextWhite);
        drawStatRow(c1X, sY + 36.0f, "STEPS:", std::to_string(result.steps) + " / " + std::to_string(result.optimalSteps), Theme::AccentCyan);
        drawStatRow(c1X, sY + 72.0f, "ALARM LEVEL:", std::to_string(result.alarm) + " / " + std::to_string(result.alarmCap), result.alarm > 0 ? Theme::Red : Theme::Green);
        drawStatRow(c1X, sY + 108.0f, "PENALTIES:", std::to_string(result.penaltyCount) + " (-" + std::to_string(result.penaltyPoints) + " pts)", result.penaltyCount > 0 ? Theme::Red : Theme::Green);

        // Column 2: Hint, Stars, Final Score
        float c2X = pX + 355.0f;
        drawStatRow(c2X, sY, "HINT USED:", result.hintUsed ? "YES (Star Cap)" : "NO (Clean Run)", result.hintUsed ? Theme::Gold : Theme::Green);
        drawStatRow(c2X, sY + 36.0f, "MEDALLIONS:", std::to_string(result.stars) + " / 3 EARNED", Theme::Gold);
        drawStatRow(c2X, sY + 72.0f, "CALCULATED SCORE:", std::to_string(result.score) + " PTS", Theme::AccentCyan);

        // Buttons at bottom
        btn1.bounds.position = {pX + 50.0f, pY + pH - 65.0f};
        btn2.bounds.position = {pX + (pW - btn2.bounds.size.x) * 0.5f, pY + pH - 65.0f};
        btn3.bounds.position = {pX + pW - 50.0f - btn3.bounds.size.x, pY + pH - 65.0f};

        btn1.draw(window, font);
        btn2.draw(window, font);
        btn3.draw(window, font);
    }
}

// -------------------------------------------------------------
// Subtle moving grid
// -------------------------------------------------------------
void drawBackgroundGrid(sf::RenderWindow& window, float width, float height, float time) {
    float shiftX = std::fmod(time * 6.0f, 60.0f);
    float shiftY = std::fmod(time * 3.0f, 60.0f);

    for (float x = shiftX; x < width; x += 60.0f) {
        sf::RectangleShape line({1.0f, height});
        line.setPosition({x, 0});
        line.setFillColor(sf::Color(38, 50, 77, 45));
        window.draw(line);
    }
    for (float y = shiftY; y < height; y += 60.0f) {
        sf::RectangleShape line({width, 1.0f});
        line.setPosition({0, y});
        line.setFillColor(sf::Color(38, 50, 77, 45));
        window.draw(line);
    }
}
