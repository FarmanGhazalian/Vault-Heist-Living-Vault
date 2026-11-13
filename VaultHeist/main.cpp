#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <fstream>
#include <memory>
#include <filesystem>
#include "nlohmann/json.hpp"
#include "Act1Level1.h"
#include "PlayerAction.h"
#include "ILevelLogic.h"

using json = nlohmann::json;

enum class GameScreen { START_MENU, PLAYING, LEVEL_END };

struct VisualNode {
    int id = -1, value = 0, leftId = -1, rightId = -1;
    sf::Vector2f pos;
    bool isVisited = false, isWrong = false;
    float wrongTimer = 0.0f, hoverScale = 1.0f;
};

// -------------------------------------------------------------
// Soothing Cinematic Stealth Audio Engine (Zero .wav files)
// -------------------------------------------------------------
class SoundManager {
public:
    sf::SoundBuffer bClick, bCorrect, bWrong, bVictory, bDefeat, bMusic;
    sf::Sound sndClick{bClick}, sndCorrect{bCorrect}, sndWrong{bWrong};
    sf::Sound sndVictory{bVictory}, sndDefeat{bDefeat}, sndMusic{bMusic};
    bool soundOn = true;

    void init() {
        const unsigned int SR = 44100;
        auto makeSine = [&](float freq, float dur, float vol) {
            std::vector<std::int16_t> s(static_cast<size_t>(dur * SR));
            for (size_t i = 0; i < s.size(); ++i) {
                float t = static_cast<float>(i) / SR;
                float env = std::pow(1.0f - (t / dur), 1.5f);
                float v = std::sin(2.0f * 3.14159f * freq * t) * vol * env;
                s[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
            }
            return s;
        };

        // UI Click (Soft metallic pop)
        auto c = makeSine(950.0f, 0.04f, 0.25f);
        static_cast<void>(bClick.loadFromSamples(c.data(), c.size(), 1, SR, {sf::SoundChannel::Mono}));

        // Correct Move (Gentle vibraphone bell chime)
        std::vector<std::int16_t> cr(static_cast<size_t>(0.35f * SR));
        for (size_t i = 0; i < cr.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            float env = std::exp(-t * 8.0f);
            float v = (std::sin(2.0f * 3.14159f * 659.25f * t) + 0.4f * std::sin(2.0f * 3.14159f * 1318.5f * t)) * 0.35f * env;
            cr[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
        }
        static_cast<void>(bCorrect.loadFromSamples(cr.data(), cr.size(), 1, SR, {sf::SoundChannel::Mono}));

        // Wrong Move (Soft low thud)
        std::vector<std::int16_t> w(static_cast<size_t>(0.25f * SR));
        for (size_t i = 0; i < w.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            float env = std::exp(-t * 9.0f);
            float v = std::sin(2.0f * 3.14159f * (160.0f - 60.0f * t) * t) * 0.4f * env;
            w[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
        }
        static_cast<void>(bWrong.loadFromSamples(w.data(), w.size(), 1, SR, {sf::SoundChannel::Mono}));

        // Victory Fanfare (Pleasant 4-note chime)
        std::vector<std::int16_t> vf(static_cast<size_t>(0.9f * SR));
        float vicNotes[] = {523.25f, 659.25f, 783.99f, 1046.50f};
        for (size_t i = 0; i < vf.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            int n = std::clamp(static_cast<int>(t / 0.2f), 0, 3);
            float tn = t - n * 0.2f;
            float v = std::sin(2.0f * 3.14159f * vicNotes[n] * t) * 0.35f * std::exp(-tn * 4.0f);
            vf[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
        }
        static_cast<void>(bVictory.loadFromSamples(vf.data(), vf.size(), 1, SR, {sf::SoundChannel::Mono}));

        // Defeat Sound
        std::vector<std::int16_t> df(static_cast<size_t>(0.6f * SR));
        for (size_t i = 0; i < df.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            float v = std::sin(2.0f * 3.14159f * (200.0f - 140.0f * t) * t) * 0.4f * std::exp(-t * 4.0f);
            df[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
        }
        static_cast<void>(bDefeat.loadFromSamples(df.data(), df.size(), 1, SR, {sf::SoundChannel::Mono}));

        // Soft, Smooth Cinematic Spy Music Loop (Warm ambient electric chords, zero harshness)
        const float dur = 4.8f; // 4 chords
        std::vector<std::int16_t> m(static_cast<size_t>(dur * SR));
        float chords[4][3] = {
            {220.00f, 261.63f, 329.63f}, // Am (A3, C4, E4)
            {174.61f, 220.00f, 261.63f}, // F  (F3, A3, C4)
            {146.83f, 220.00f, 261.63f}, // Dm (D3, A3, C4)
            {164.81f, 196.00f, 246.94f}  // Em (E3, G3, B3)
        };
        for (size_t i = 0; i < m.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            int cIdx = std::clamp(static_cast<int>(t / 1.2f), 0, 3);
            float tc = std::fmod(t, 1.2f);
            float env = std::sin(3.14159f * (tc / 1.2f)); // Soft swell
            float s = 0.0f;
            for (int k = 0; k < 3; ++k) {
                s += std::sin(2.0f * 3.14159f * chords[cIdx][k] * t);
            }
            s = (s / 3.0f) * 0.22f * env;
            m[i] = static_cast<std::int16_t>(std::clamp(s, -1.0f, 1.0f) * 32767.0f);
        }
        static_cast<void>(bMusic.loadFromSamples(m.data(), m.size(), 1, SR, {sf::SoundChannel::Mono}));
        sndMusic.setLooping(true);
        sndMusic.setVolume(30.0f);
        sndMusic.play();
    }
    void click() { if (soundOn) sndClick.play(); }
    void correct() { if (soundOn) sndCorrect.play(); }
    void wrong() { if (soundOn) sndWrong.play(); }
    void victory() { if (soundOn) sndVictory.play(); }
    void defeat() { if (soundOn) sndDefeat.play(); }
    void toggle() {
        soundOn = !soundOn;
        if (!soundOn) sndMusic.pause();
        else sndMusic.play();
    }
};

// -------------------------------------------------------------
// UI Helpers
// -------------------------------------------------------------
struct Button {
    sf::FloatRect bounds;
    std::string text;
    bool isHovered = false;
    void update(const sf::Vector2f& m) { isHovered = bounds.contains(m); }
    void draw(sf::RenderWindow& w, const sf::Font& f, sf::Color col = sf::Color(37, 99, 235)) const {
        sf::RectangleShape r({bounds.size.x, bounds.size.y});
        r.setPosition({bounds.position.x, bounds.position.y + (isHovered ? 1.0f : 3.0f)});
        r.setFillColor(isHovered ? sf::Color(col.r + 25, col.g + 25, col.b + 25) : col);
        r.setOutlineThickness(3.0f);
        r.setOutlineColor(isHovered ? sf::Color(255, 234, 0) : sf::Color(15, 23, 42));
        w.draw(r);
        sf::Text t(f, text, 19);
        t.setStyle(sf::Text::Bold);
        t.setFillColor(sf::Color::White);
        auto b = t.getLocalBounds();
        t.setOrigin({b.position.x + b.size.x * 0.5f, b.position.y + b.size.y * 0.5f});
        t.setPosition({bounds.position.x + bounds.size.x * 0.5f, bounds.position.y + bounds.size.y * 0.5f + (isHovered ? 1.0f : 3.0f)});
        w.draw(t);
    }
};

void drawStar(sf::RenderWindow& w, sf::Vector2f center, float radius, sf::Color fill, float scale = 1.0f) {
    sf::ConvexShape star(10);
    float rOuter = radius * scale, rInner = rOuter * 0.45f;
    for (int i = 0; i < 10; ++i) {
        float r = (i % 2 == 0) ? rOuter : rInner;
        float a = (i * 36.0f - 90.0f) * 3.14159f / 180.0f;
        star.setPoint(i, {center.x + r * std::cos(a), center.y + r * std::sin(a)});
    }
    star.setFillColor(fill);
    star.setOutlineThickness(3.5f * scale);
    star.setOutlineColor(sf::Color(15, 23, 42));
    w.draw(star);
}

// -------------------------------------------------------------
// Traversal Helpers (Dynamic from JSON)
// -------------------------------------------------------------
std::string getTraversalDesc(const std::string& type) {
    if (type == "INORDER") return "INORDER (Left -> Root -> Right)";
    if (type == "POSTORDER") return "POSTORDER (Left -> Right -> Root)";
    if (type == "LEVELORDER") return "LEVEL ORDER (Top to Bottom, Left to Right)";
    return "PREORDER (Root -> Left -> Right)";
}

std::string getTraversalHintMsg(const std::string& type) {
    if (type == "INORDER") return "HINT: Inorder traverses the Left subtree first, visits the Root, then the Right subtree.";
    if (type == "POSTORDER") return "HINT: Postorder traverses Left, then Right subtree, and visits the Root at the very end.";
    if (type == "LEVELORDER") return "HINT: Level Order traverses all rooms on the current level from left to right.";
    return "HINT: Preorder visits the Root room first, then traverses Left subtree, then Right.";
}

// -------------------------------------------------------------
// MAIN GAME CONTROLLER
// -------------------------------------------------------------
int main() {
    const unsigned int W = 1500, H = 850;
    sf::RenderWindow window(sf::VideoMode({W, H}), "Vault Heist: The Living Vault", sf::Style::Titlebar | sf::Style::Close);
    window.setFramerateLimit(60);

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arialbd.ttf") && !font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        font.openFromFile("C:/Windows/Fonts/segoeui.ttf");
    }

    SoundManager audio;
    audio.init();

    GameScreen screen = GameScreen::START_MENU;
    std::string jsonPath = "Act1Level1.json";
    if (!std::filesystem::exists(jsonPath) && std::filesystem::exists("VaultHeist/Act1Level1.json")) {
        jsonPath = "VaultHeist/Act1Level1.json";
    }

    std::unique_ptr<Act1Level1> level = std::make_unique<Act1Level1>();
    level->initLevel(jsonPath);

    std::vector<VisualNode> visualNodes;
    std::string levelTitle = "VAULT PATROL";
    std::string traversalType = "PREORDER";
    std::string banner = "Infiltrate the vault rooms following the patrol route.";
    sf::Color bannerCol = sf::Color(14, 165, 233);
    float starAnimTimer = 0.0f;

    // Load JSON dynamically
    auto loadNodes = [&]() {
        visualNodes.clear();
        std::ifstream f(jsonPath);
        if (!f.is_open()) return;
        json j; f >> j;

        if (j.contains("levelName")) levelTitle = j["levelName"].get<std::string>();
        if (j.contains("traversalType")) traversalType = j["traversalType"].get<std::string>();

        for (const auto& item : j["nodes"]) {
            visualNodes.push_back({item["id"], item["value"], item["left"], item["right"]});
        }
        auto findIdx = [&](int id) {
            for (size_t i = 0; i < visualNodes.size(); ++i) if (visualNodes[i].id == id) return (int)i;
            return -1;
        };
        auto place = [&](auto& self, int idx, float x, float y, float xOff) -> void {
            if (idx == -1) return;
            visualNodes[idx].pos = {x, y};
            int l = findIdx(visualNodes[idx].leftId), r = findIdx(visualNodes[idx].rightId);
            if (l != -1) self(self, l, x - xOff, y + 130.0f, xOff * 0.54f);
            if (r != -1) self(self, r, x + xOff, y + 130.0f, xOff * 0.54f);
        };
        int rootIdx = findIdx(j["rootId"]);
        if (rootIdx != -1) place(place, rootIdx, W * 0.5f, 250.0f, 320.0f);
    };

    auto resetGame = [&]() {
        level = std::make_unique<Act1Level1>();
        level->initLevel(jsonPath);
        loadNodes();
        banner = "Patrol Active: Click rooms in " + getTraversalDesc(traversalType) + ".";
        bannerCol = sf::Color(14, 165, 233);
        starAnimTimer = 0.0f;
    };

    Button btnStart{{{W * 0.5f - 150.0f, 530.0f}, {300.0f, 65.0f}}, "START HEIST"};
    Button btnSound{{{W - 210.0f, 25.0f}, {170.0f, 45.0f}}, "SOUND: ON"};
    Button btnHint{{{W - 530.0f, 25.0f}, {140.0f, 45.0f}}, "HINT"};
    Button btnRestart{{{W - 370.0f, 25.0f}, {140.0f, 45.0f}}, "RESTART"};
    Button btnRetry{{{W * 0.5f - 150.0f, 540.0f}, {300.0f, 58.0f}}, "PLAY AGAIN"};

    sf::Clock clock;
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        sf::Vector2f mouse{static_cast<float>(sf::Mouse::getPosition(window).x), static_cast<float>(sf::Mouse::getPosition(window).y)};

        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();
            if (const auto* mp = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (mp->button == sf::Mouse::Button::Left) {
                    if (screen == GameScreen::START_MENU) {
                        if (btnStart.bounds.contains(mouse)) { audio.click(); resetGame(); screen = GameScreen::PLAYING; }
                        if (btnSound.bounds.contains(mouse)) { audio.toggle(); btnSound.text = audio.soundOn ? "SOUND: ON" : "SOUND: OFF"; }
                    }
                    else if (screen == GameScreen::PLAYING) {
                        if (btnSound.bounds.contains(mouse)) { audio.toggle(); btnSound.text = audio.soundOn ? "SOUND: ON" : "SOUND: OFF"; }
                        else if (btnRestart.bounds.contains(mouse)) { audio.click(); resetGame(); }
                        else if (btnHint.bounds.contains(mouse)) {
                            audio.click();
                            if (level->getState().hintUsed) {
                                banner = "Hint already used for this level!";
                                bannerCol = sf::Color(245, 158, 11);
                            } else {
                                level->onHintRequested();
                                banner = getTraversalHintMsg(traversalType);
                                bannerCol = sf::Color(250, 204, 21);
                            }
                        }
                        else {
                            for (auto& vn : visualNodes) {
                                float dx = mouse.x - vn.pos.x, dy = mouse.y - vn.pos.y;
                                if (dx * dx + dy * dy <= 40.0f * 40.0f) {
                                    PlayerAction act; act.type = CLICK_NODE; act.nodeA = vn.value;
                                    ActionResult res = level->processAction(act);
                                    banner = res.message;
                                    if (res.correct) {
                                        vn.isVisited = true;
                                        bannerCol = sf::Color(34, 197, 94);
                                        audio.correct();
                                    } else {
                                        vn.isWrong = true;
                                        vn.wrongTimer = 0.8f;
                                        bannerCol = sf::Color(239, 68, 68);
                                        audio.wrong();
                                    }
                                    break;
                                }
                            }
                        }
                    }
                    else if (screen == GameScreen::LEVEL_END) {
                        if (btnRetry.bounds.contains(mouse)) { audio.click(); resetGame(); screen = GameScreen::PLAYING; }
                    }
                }
            }
        }

        // Instant Game Over transition when alarm reaches cap or finished
        if (screen == GameScreen::PLAYING && (level->isFinished() || level->getState().alarm >= level->getState().alarmCap)) {
            screen = GameScreen::LEVEL_END;
            starAnimTimer = 0.0f;
            if (level->isWon()) audio.victory();
            else audio.defeat();
        }

        btnStart.update(mouse); btnSound.update(mouse); btnHint.update(mouse); btnRestart.update(mouse); btnRetry.update(mouse);
        for (auto& vn : visualNodes) {
            if (vn.wrongTimer > 0.0f) { vn.wrongTimer -= dt; if (vn.wrongTimer <= 0.0f) vn.isWrong = false; }
            float dx = mouse.x - vn.pos.x, dy = mouse.y - vn.pos.y;
            vn.hoverScale = (dx * dx + dy * dy <= 40.0f * 40.0f) ? 1.15f : 1.0f;
        }

        // -------------------------------------------------------------
        // RENDERING
        // -------------------------------------------------------------
        window.clear(sf::Color(18, 24, 38));

        // Background Grid
        for (float x = 0; x < W; x += 60.0f) {
            sf::RectangleShape l({1.5f, (float)H}); l.setPosition({x, 0}); l.setFillColor(sf::Color(30, 41, 59, 70)); window.draw(l);
        }
        for (float y = 0; y < H; y += 60.0f) {
            sf::RectangleShape l({(float)W, 1.5f}); l.setPosition({0, y}); l.setFillColor(sf::Color(30, 41, 59, 70)); window.draw(l);
        }

        if (screen == GameScreen::START_MENU) {
            btnSound.bounds = {{W - 210.0f, 25.0f}, {170.0f, 45.0f}};
            btnSound.draw(window, font, sf::Color(30, 41, 59));

            sf::Text t(font, "VAULT HEIST", 85);
            t.setStyle(sf::Text::Bold); t.setFillColor(sf::Color(255, 234, 0));
            auto b = t.getLocalBounds(); t.setOrigin({b.size.x * 0.5f, b.size.y * 0.5f}); t.setPosition({W * 0.5f, 250.0f});
            window.draw(t);

            sf::Text sub(font, "ACT 1: THE LIVING BINARY VAULT", 24);
            sub.setStyle(sf::Text::Bold); sub.setFillColor(sf::Color(56, 189, 248));
            auto sb = sub.getLocalBounds(); sub.setOrigin({sb.size.x * 0.5f, sb.size.y * 0.5f}); sub.setPosition({W * 0.5f, 335.0f});
            window.draw(sub);

            std::string dynamicMission = "Mission: Predict the patrol route using " + getTraversalDesc(traversalType) + ".\nClear all rooms before tripping the alarms!";
            sf::Text info(font, dynamicMission, 18);
            info.setFillColor(sf::Color(226, 232, 240));
            auto ib = info.getLocalBounds(); info.setOrigin({ib.size.x * 0.5f, ib.size.y * 0.5f}); info.setPosition({W * 0.5f, 420.0f});
            window.draw(info);

            btnStart.draw(window, font, sf::Color(16, 185, 129));
        }
        else {
            auto findPos = [&](int id) {
                for (const auto& vn : visualNodes) if (vn.id == id) return vn.pos;
                return sf::Vector2f{0, 0};
            };

            // Pipes
            for (const auto& vn : visualNodes) {
                auto drawPipe = [&](int cId) {
                    if (cId == -1) return;
                    sf::Vector2f diff = findPos(cId) - vn.pos;
                    float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
                    sf::RectangleShape p({len, 10.0f});
                    p.setOrigin({0, 5.0f}); p.setPosition(vn.pos);
                    p.setRotation(sf::degrees(std::atan2(diff.y, diff.x) * 180.0f / 3.14159f));
                    p.setFillColor(sf::Color(71, 85, 105)); p.setOutlineThickness(2.5f); p.setOutlineColor(sf::Color(15, 23, 42));
                    window.draw(p);
                };
                drawPipe(vn.leftId); drawPipe(vn.rightId);
            }

            // Nodes
            for (const auto& vn : visualNodes) {
                float rad = 36.0f * vn.hoverScale;
                sf::Vector2f p = vn.pos;
                if (vn.isWrong) p.x += std::sin(vn.wrongTimer * 50.0f) * 6.0f;

                sf::CircleShape c(rad);
                c.setOrigin({rad, rad}); c.setPosition(p);
                if (vn.isWrong) c.setFillColor(sf::Color(239, 68, 68));
                else if (vn.isVisited) c.setFillColor(sf::Color(34, 197, 94));
                else if (vn.hoverScale > 1.0f) c.setFillColor(sf::Color(14, 165, 233));
                else c.setFillColor(sf::Color(2, 132, 199));
                c.setOutlineThickness(4.0f); c.setOutlineColor(vn.hoverScale > 1.0f ? sf::Color(255, 234, 0) : sf::Color(15, 23, 42));
                window.draw(c);

                sf::Text num(font, std::to_string(vn.value), static_cast<unsigned int>(24 * vn.hoverScale));
                num.setStyle(sf::Text::Bold); num.setFillColor(sf::Color::White);
                auto nb = num.getLocalBounds(); num.setOrigin({nb.position.x + nb.size.x * 0.5f, nb.position.y + nb.size.y * 0.5f});
                num.setPosition(p);
                window.draw(num);
            }

            // Top HUD
            sf::RectangleShape hud({(float)W, 85.0f});
            hud.setFillColor(sf::Color(15, 23, 42, 240)); hud.setOutlineThickness(2.5f); hud.setOutlineColor(sf::Color(30, 41, 59));
            window.draw(hud);

            std::string titleStr = "ACT 1: " + levelTitle;
            sf::Text title(font, titleStr, 20);
            title.setStyle(sf::Text::Bold); title.setFillColor(sf::Color(255, 234, 0)); title.setPosition({40.0f, 15.0f}); window.draw(title);

            std::string subStr = "Traversal: " + getTraversalDesc(traversalType);
            sf::Text sub(font, subStr, 14);
            sub.setFillColor(sf::Color(148, 163, 184)); sub.setPosition({40.0f, 48.0f}); window.draw(sub);

            std::string st = "STEPS: " + std::to_string(level->getState().steps) + " / " + std::to_string(level->getState().optimalSteps);
            sf::Text txtS(font, st, 18); txtS.setStyle(sf::Text::Bold); txtS.setFillColor(sf::Color(56, 189, 248)); txtS.setPosition({520.0f, 30.0f}); window.draw(txtS);

            std::string al = "ALARMS: " + std::to_string(level->getState().alarm) + " / " + std::to_string(level->getState().alarmCap);
            sf::Text txtA(font, al, 18); txtA.setStyle(sf::Text::Bold);
            txtA.setFillColor(level->getState().alarm > 0 ? sf::Color(239, 68, 68) : sf::Color(34, 197, 94));
            txtA.setPosition({710.0f, 30.0f}); window.draw(txtA);

            btnHint.draw(window, font, sf::Color(217, 119, 6));
            btnRestart.draw(window, font, sf::Color(51, 65, 85));
            btnSound.bounds = {{W - 210.0f, 25.0f}, {170.0f, 45.0f}};
            btnSound.draw(window, font, sf::Color(51, 65, 85));

            // Bottom Banner
            sf::RectangleShape ban({(float)W - 80.0f, 60.0f});
            ban.setPosition({40.0f, (float)H - 85.0f});
            ban.setFillColor(sf::Color(15, 23, 42, 235)); ban.setOutlineThickness(3.0f); ban.setOutlineColor(bannerCol);
            window.draw(ban);

            sf::Text txtBan(font, banner, 18); txtBan.setStyle(sf::Text::Bold); txtBan.setFillColor(bannerCol);
            auto bb = txtBan.getLocalBounds(); txtBan.setOrigin({bb.position.x + bb.size.x * 0.5f, bb.position.y + bb.size.y * 0.5f});
            txtBan.setPosition({W * 0.5f, H - 55.0f}); window.draw(txtBan);

            // ---------------------------------------------------------
            // SCREEN 3: LEVEL END MODAL WITH ANIMATED STARS
            // ---------------------------------------------------------
            if (screen == GameScreen::LEVEL_END) {
                starAnimTimer += dt;
                sf::RectangleShape dim({(float)W, (float)H});
                dim.setFillColor(sf::Color(0, 0, 0, 195)); window.draw(dim);

                bool won = level->isWon();
                sf::RectangleShape modal({560.0f, 400.0f});
                modal.setPosition({W * 0.5f - 280.0f, 180.0f});
                modal.setFillColor(sf::Color(15, 23, 42)); modal.setOutlineThickness(5.0f);
                modal.setOutlineColor(won ? sf::Color(34, 197, 94) : sf::Color(239, 68, 68));
                window.draw(modal);

                sf::Text mTitle(font, won ? "VAULT UNLOCKED!" : "ALARM TRIGGERED - VAULT LOCKED!", 28);
                mTitle.setStyle(sf::Text::Bold); mTitle.setFillColor(won ? sf::Color(255, 234, 0) : sf::Color(239, 68, 68));
                auto mtb = mTitle.getLocalBounds(); mTitle.setOrigin({mtb.size.x * 0.5f, mtb.size.y * 0.5f});
                mTitle.setPosition({W * 0.5f, 235.0f}); window.draw(mTitle);

                // Animated Stars
                int stars = won ? level->getCalculatedStars() : 0;
                float starX[3] = {W * 0.5f - 85.0f, W * 0.5f, W * 0.5f + 85.0f};
                float starY[3] = {315.0f, 300.0f, 315.0f};
                for (int i = 0; i < 3; ++i) {
                    float delay = 0.2f * (i + 1);
                    float scale = 0.0f;
                    if (starAnimTimer > delay) {
                        float prog = (starAnimTimer - delay) / 0.25f;
                        scale = std::min(1.0f, prog);
                        if (scale < 1.0f) scale = 1.0f + 0.3f * std::sin(scale * 3.14159f);
                    }
                    sf::Color starCol = (i < stars) ? sf::Color(255, 215, 0) : sf::Color(71, 85, 105);
                    drawStar(window, {starX[i], starY[i]}, 28.0f, starCol, scale);
                }

                std::string stats = "Steps: " + std::to_string(level->getState().steps) + " (Optimal: " + std::to_string(level->getState().optimalSteps) + ")\n" +
                                    "Alarms Raised: " + std::to_string(level->getState().alarm) + " / " + std::to_string(level->getState().alarmCap) + "\n" +
                                    "Final Score: " + std::to_string(level->getCalculatedScore()) + " Points";
                sf::Text txtStats(font, stats, 19);
                txtStats.setFillColor(sf::Color(241, 245, 249));
                txtStats.setPosition({W * 0.5f - 220.0f, 385.0f}); window.draw(txtStats);

                btnRetry.bounds = {{W * 0.5f - 140.0f, 490.0f}, {280.0f, 52.0f}};
                btnRetry.text = won ? "PLAY AGAIN" : "RETRY LEVEL";
                btnRetry.draw(window, font, won ? sf::Color(16, 185, 129) : sf::Color(220, 38, 38));
            }
        }
        window.display();
    }
    return 0;
}
