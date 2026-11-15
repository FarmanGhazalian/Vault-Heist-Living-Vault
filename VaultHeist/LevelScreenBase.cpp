#include "LevelScreenBase.h"
#include "GameEngine.h"
#include "SelectScreen.h"
#include <algorithm>
#include <iostream>

LevelScreenBase::LevelScreenBase() {
    float W = 1500.0f;
    btnHint = Button({{W - 470.0f, 20.0f}, {100.0f, 38.0f}}, "HINT", ButtonStyle::GOLD);
    btnRestart = Button({{W - 355.0f, 20.0f}, {100.0f, 38.0f}}, "RESTART", ButtonStyle::SECONDARY);
    btnSound = Button({{W - 240.0f, 20.0f}, {105.0f, 38.0f}}, "SOUND", ButtonStyle::SECONDARY);
    btnHint.style = ButtonStyle::GOLD;
}

void LevelScreenBase::setupHUD(const std::string& title, const std::string& objective, const std::string& traversal, int lvlIdx) {
    levelTitle = title;
    objectiveText = objective;
    traversalDesc = traversal;
    levelIndex = lvlIdx;
}

void LevelScreenBase::handleEvent(GameEngine& engine, const sf::Event& event) {
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};

    // 1. Result Popup takes priority
    if (resultPopup.active) {
        int act = resultPopup.handleEvent(event, mouse);
        if (act == PopupAction::RETRY) {
            resultPopup.close();
            resultShown = false;
            restartLevel(engine);
        } else if (act == PopupAction::NEXT) {
            resultPopup.close();
            resultShown = false;
            goToNextLevel(engine);
        } else if (act == PopupAction::MENU) {
            resultPopup.close();
            resultShown = false;
            engine.changeScreen(std::make_unique<SelectScreen>(SelectMode::LEVELS, 1));
        }
        return;
    }

    // 2. Pause Popup
    if (pausePopup.active) {
        int act = pausePopup.handleEvent(event, mouse);
        if (act == PopupAction::MENU) {
            pausePopup.close();
            engine.changeScreen(std::make_unique<SelectScreen>(SelectMode::LEVELS, 1));
        }
        return;
    }

    // 3. Hotkeys
    if (const auto* kp = event.getIf<sf::Event::KeyPressed>()) {
        if (kp->code == sf::Keyboard::Key::Escape) {
            pausePopup.showPause();
            engine.audio.playClick();
            return;
        } else if (kp->code == sf::Keyboard::Key::H) {
            requestHint(engine);
            return;
        } else if (kp->code == sf::Keyboard::Key::R) {
            restartLevel(engine);
            engine.audio.playClick();
            return;
        }
    }

    // 4. Mouse click on buttons
    if (const auto* mp = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (mp->button == sf::Mouse::Button::Left) {
            if (btnHint.handleClick(mouse)) {
                requestHint(engine);
                return;
            }
            if (btnRestart.handleClick(mouse)) {
                engine.audio.playClick();
                restartLevel(engine);
                return;
            }
            if (btnSound.handleClick(mouse)) {
                engine.audio.toggleMute();
                return;
            }
        }
    }

    // 5. Forward to concrete level
    onLevelEvent(engine, event);
}

void LevelScreenBase::update(GameEngine& engine, float dt) {
    levelTime += dt;
    sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(engine.window).x),
                       static_cast<float>(sf::Mouse::getPosition(engine.window).y)};

    if (resultPopup.active) {
        resultPopup.update(dt, mouse, engine.audio);
        return;
    }
    if (pausePopup.active) {
        pausePopup.update(dt, mouse, engine.audio);
        return;
    }

    btnHint.update(mouse);
    btnRestart.update(mouse);
    btnSound.update(mouse);
    btnSound.text = engine.audio.isMuted() ? "SOUND OFF" : "SOUND ON";

    treeRenderer.update(dt, mouse);
    updateLevel(engine, dt);

    // Check game over / level finish
    checkLevelEnd(engine, getLevelLogic());
}

void LevelScreenBase::checkLevelEnd(GameEngine& engine, ILevelLogic* logic) {
    if (!logic || resultShown) return;

    const LevelState& st = logic->getState();
    if (st.finished || st.alarm >= st.alarmCap) {
        resultShown = true;
        LevelResultData rd;
        rd.levelName = levelTitle;
        rd.steps = st.steps;
        rd.optimalSteps = st.optimalSteps;
        rd.alarm = st.alarm;
        rd.alarmCap = st.alarmCap;
        rd.penaltyCount = st.penaltyCount;
        rd.penaltyPoints = st.penaltyPoints;
        rd.hintUsed = st.hintUsed;
        rd.stars = logic->isWon() ? logic->getCalculatedStars() : 0;
        rd.score = logic->getCalculatedScore();
        rd.isWon = logic->isWon();
        rd.isLastLevel = (levelIndex == 5);

        if (rd.isWon) {
            engine.audio.playVictory();
        } else {
            engine.audio.playDefeat();
        }

        resultPopup.showResult(rd);
    }
}

void LevelScreenBase::drawHUD(GameEngine& engine, sf::RenderWindow& window, const LevelState& state) {
    float W = static_cast<float>(window.getSize().x);

    // Top HUD bar
    sf::RectangleShape hud({W, 75.0f});
    hud.setFillColor(sf::Color(14, 20, 36, 240));
    hud.setOutlineThickness(1.0f);
    hud.setOutlineColor(Theme::PanelBorder);
    window.draw(hud);

    // Tag and Title
    sf::Text tag(engine.font, "ACT 1: THE BLUEPRINT WING", 11);
    tag.setStyle(sf::Text::Bold);
    tag.setFillColor(Theme::AccentCyan);
    tag.setPosition({40.0f, 12.0f});
    window.draw(tag);

    sf::Text title(engine.font, levelTitle, 19);
    title.setStyle(sf::Text::Bold);
    title.setFillColor(Theme::TextWhite);
    title.setPosition({40.0f, 28.0f});
    window.draw(title);

    // Traversal or Mode
    if (!traversalDesc.empty()) {
        sf::Text trav(engine.font, traversalDesc, 11);
        trav.setFillColor(Theme::TextMuted);
        trav.setPosition({40.0f, 52.0f});
        window.draw(trav);
    }

    // Steps Counter
    sf::Text sLbl(engine.font, "STEPS", 11);
    sLbl.setStyle(sf::Text::Bold);
    sLbl.setFillColor(Theme::TextMuted);
    sLbl.setPosition({480.0f, 15.0f});
    window.draw(sLbl);

    std::string stepsVal = std::to_string(state.steps) + " / " + std::to_string(state.optimalSteps);
    sf::Text sVal(engine.font, stepsVal, 18);
    sVal.setStyle(sf::Text::Bold);
    sVal.setFillColor(Theme::AccentCyan);
    sVal.setPosition({480.0f, 32.0f});
    window.draw(sVal);

    // Alarm Bar Gauge
    sf::Text aLbl(engine.font, "VAULT ALARM LEVEL", 11);
    aLbl.setStyle(sf::Text::Bold);
    aLbl.setFillColor(Theme::TextMuted);
    aLbl.setPosition({640.0f, 15.0f});
    window.draw(aLbl);

    float barW = 160.0f;
    float barH = 12.0f;
    sf::RectangleShape barBg({barW, barH});
    barBg.setPosition({640.0f, 38.0f});
    barBg.setFillColor(sf::Color(22, 32, 54));
    barBg.setOutlineThickness(1.0f);
    barBg.setOutlineColor(Theme::PanelBorder);
    window.draw(barBg);

    float alarmRatio = (state.alarmCap > 0) ? std::clamp(static_cast<float>(state.alarm) / static_cast<float>(state.alarmCap), 0.0f, 1.0f) : 0.0f;
    sf::Color alarmBarCol = Theme::Green;
    if (alarmRatio > 0.65f) alarmBarCol = Theme::Red;
    else if (alarmRatio > 0.35f) alarmBarCol = sf::Color(245, 158, 11);

    sf::RectangleShape barFill({barW * alarmRatio, barH});
    barFill.setPosition({640.0f, 38.0f});
    barFill.setFillColor(alarmBarCol);
    window.draw(barFill);

    std::string alarmVal = std::to_string(state.alarm) + " / " + std::to_string(state.alarmCap);
    sf::Text aVal(engine.font, alarmVal, 13);
    aVal.setStyle(sf::Text::Bold);
    aVal.setFillColor(alarmBarCol);
    aVal.setPosition({815.0f, 36.0f});
    window.draw(aVal);

    // Score Counter
    sf::Text scLbl(engine.font, "EST. SCORE", 11);
    scLbl.setStyle(sf::Text::Bold);
    scLbl.setFillColor(Theme::TextMuted);
    scLbl.setPosition({905.0f, 15.0f});
    window.draw(scLbl);

    sf::Text scVal(engine.font, std::to_string(ScoreSystem::score(state)) + " PTS", 18);
    scVal.setStyle(sf::Text::Bold);
    scVal.setFillColor(Theme::Gold);
    scVal.setPosition({905.0f, 32.0f});
    window.draw(scVal);

    // Top buttons
    btnHint.draw(window, engine.font);
    btnRestart.draw(window, engine.font);
    btnSound.draw(window, engine.font);

    // Bottom Objective Banner
    float H = static_cast<float>(window.getSize().y);
    sf::RectangleShape ban({W - 80.0f, 44.0f});
    ban.setPosition({40.0f, H - 60.0f});
    ban.setFillColor(sf::Color(14, 20, 36, 230));
    ban.setOutlineThickness(1.0f);
    ban.setOutlineColor(Theme::PanelBorder);
    window.draw(ban);

    // Objective text
    sf::Text obj(engine.font, "OBJECTIVE: " + objectiveText, 13);
    obj.setStyle(sf::Text::Bold);
    obj.setFillColor(Theme::AccentCyan);
    auto ob = obj.getLocalBounds();
    obj.setPosition({40.0f + (W - 80.0f - ob.size.x) * 0.5f, H - 47.0f});
    window.draw(obj);
}

void LevelScreenBase::draw(GameEngine& engine, sf::RenderWindow& window) {
    float W = static_cast<float>(window.getSize().x);
    float H = static_cast<float>(window.getSize().y);

    window.clear(Theme::BgDark);
    drawBackgroundGrid(window, W, H, levelTime);

    // Level custom drawing (tree, tokens, inputs)
    drawLevel(engine, window);

    // Common HUD
    if (getLevelLogic()) {
        drawHUD(engine, window, getLevelLogic()->getState());
    }

    // Popups
    pausePopup.draw(window, engine.font);
    resultPopup.draw(window, engine.font);
}
