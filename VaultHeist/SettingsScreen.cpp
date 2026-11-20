#include "SettingsScreen.h"
#include "MainMenuScreen.h"
#include "GameEngine.h"
#include "SettingsManager.h"
#include "AnimationController.h"
#include "Benchmark.h"
#include "TestSuite.h"
#include <cmath>
#include <algorithm>
#include <thread>

SettingsScreen::SettingsScreen(GameEngine& gameEngine)
    : engine(gameEngine),
      windowSize(gameEngine.getWindowSize()),
      btnBack("< BACK", sf::FloatRect(sf::Vector2f(24.0f, 24.0f), sf::Vector2f(100.0f, 36.0f)), BTN_BACK),
      btnAudioToggle("AUDIO: ON", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(180.0f, 40.0f)), BTN_PRIMARY),
      btnSpeed05("0.5x", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(70.0f, 38.0f)), BTN_SECONDARY),
      btnSpeed10("1.0x", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(70.0f, 38.0f)), BTN_PRIMARY),
      btnSpeed15("1.5x", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(70.0f, 38.0f)), BTN_SECONDARY),
      btnSpeed20("2.0x", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(70.0f, 38.0f)), BTN_SECONDARY),
      btnResetProgress("RESET CAMPAIGN PROGRESS", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(260.0f, 42.0f)), BTN_DANGER),
      btnBenchmark("RUN BENCHMARK", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(170.0f, 40.0f)), BTN_SECONDARY),
      btnTestSuite("RUN TEST CASES", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(170.0f, 40.0f)), BTN_SECONDARY)
{
    btnAudioToggle.label = engine.audio.isMuted() ? "AUDIO: MUTED" : "AUDIO: ON";
    updateSpeedButtons();
    updateLayout(windowSize.x, windowSize.y);
}

void SettingsScreen::updateSpeedButtons()
{
    float cur = AnimationController::getSpeed();
    btnSpeed05.style = (std::abs(cur - 0.5f) < 0.1f) ? BTN_PRIMARY : BTN_SECONDARY;
    btnSpeed10.style = (std::abs(cur - 1.0f) < 0.1f) ? BTN_PRIMARY : BTN_SECONDARY;
    btnSpeed15.style = (std::abs(cur - 1.5f) < 0.1f) ? BTN_PRIMARY : BTN_SECONDARY;
    btnSpeed20.style = (std::abs(cur - 2.0f) < 0.1f) ? BTN_PRIMARY : BTN_SECONDARY;
}

void SettingsScreen::updateLayout(float w, float h)
{
    windowSize = sf::Vector2f(w, h);
    btnBack.bounds = sf::FloatRect(sf::Vector2f(24.0f, 24.0f), sf::Vector2f(100.0f, 36.0f));

    float cx = w * 0.5f;
    float startY = 170.0f;

    // Centered Audio Toggle
    float audioW = 200.0f;
    btnAudioToggle.bounds = sf::FloatRect(sf::Vector2f(std::round(cx - audioW * 0.5f), startY), sf::Vector2f(audioW, 40.0f));

    // Centered Speed Buttons row
    float spW = 75.0f;
    float spSpacing = 16.0f;
    float totalSpW = 4.0f * spW + 3.0f * spSpacing;
    float startSpX = std::round(cx - totalSpW * 0.5f);
    float spY = startY + 100.0f;

    btnSpeed05.bounds = sf::FloatRect(sf::Vector2f(startSpX, spY), sf::Vector2f(spW, 38.0f));
    btnSpeed10.bounds = sf::FloatRect(sf::Vector2f(startSpX + spW + spSpacing, spY), sf::Vector2f(spW, 38.0f));
    btnSpeed15.bounds = sf::FloatRect(sf::Vector2f(startSpX + (spW + spSpacing) * 2.0f, spY), sf::Vector2f(spW, 38.0f));
    btnSpeed20.bounds = sf::FloatRect(sf::Vector2f(startSpX + (spW + spSpacing) * 3.0f, spY), sf::Vector2f(spW, 38.0f));

    // Centered Reset Progress Button
    float rstW = 260.0f;
    btnResetProgress.bounds = sf::FloatRect(sf::Vector2f(std::round(cx - rstW * 0.5f), spY + 95.0f), sf::Vector2f(rstW, 38.0f));

    // Centered Diagnostics Row (RUN BENCHMARK & RUN TEST CASES)
    float diagW = 175.0f;
    float diagSpacing = 20.0f;
    float diagTotalW = 2.0f * diagW + diagSpacing;
    float diagStartX = std::round(cx - diagTotalW * 0.5f);
    float diagY = spY + 185.0f;
    btnBenchmark.bounds = sf::FloatRect(sf::Vector2f(diagStartX, diagY), sf::Vector2f(diagW, 40.0f));
    btnTestSuite.bounds = sf::FloatRect(sf::Vector2f(diagStartX + diagW + diagSpacing, diagY), sf::Vector2f(diagW, 40.0f));
}

void SettingsScreen::onResize(unsigned int width, unsigned int height)
{
    updateLayout(static_cast<float>(width), static_cast<float>(height));
}

void SettingsScreen::handleEvent(const sf::Event& event)
{
    btnBack.handleEvent(event);
    btnAudioToggle.handleEvent(event);
    btnSpeed05.handleEvent(event);
    btnSpeed10.handleEvent(event);
    btnSpeed15.handleEvent(event);
    btnSpeed20.handleEvent(event);
    btnResetProgress.handleEvent(event);
    btnBenchmark.handleEvent(event);
    btnTestSuite.handleEvent(event);

    if (btnBack.isClicked())
    {
        btnBack.resetClick();
        engine.audio.playClick();
        engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
        return;
    }

    if (btnAudioToggle.isClicked())
    {
        btnAudioToggle.resetClick();
        bool newAudio = engine.audio.isMuted(); // If currently muted, turn ON
        SettingsManager::setAudio(newAudio, engine.audio);
        btnAudioToggle.label = engine.audio.isMuted() ? "AUDIO: MUTED" : "AUDIO: ON";
        if (!engine.audio.isMuted()) engine.audio.playClick();
        return;
    }

    auto handleSpeed = [this](float spd) {
        SettingsManager::setSpeed(spd);
        updateSpeedButtons();
        engine.audio.playClick();
        engine.showToast("Animation speed set to " + std::to_string(spd).substr(0, 3) + "x", TOAST_INFO);
    };

    if (btnSpeed05.isClicked()) { btnSpeed05.resetClick(); handleSpeed(0.5f); return; }
    if (btnSpeed10.isClicked()) { btnSpeed10.resetClick(); handleSpeed(1.0f); return; }
    if (btnSpeed15.isClicked()) { btnSpeed15.resetClick(); handleSpeed(1.5f); return; }
    if (btnSpeed20.isClicked()) { btnSpeed20.resetClick(); handleSpeed(2.0f); return; }

    if (btnResetProgress.isClicked())
    {
        btnResetProgress.resetClick();
        engine.audio.playClick();
        engine.showConfirmModal("RESET PROGRESS",
            "Are you sure you want to reset all campaign progress?\nOnly Sector 1.1 will remain unlocked.",
            [this]() {
                SettingsManager::resetProgress();
                engine.showToast("Campaign progress reset to fresh state.", TOAST_INFO);
            },
            nullptr);
        return;
    }

    if (btnBenchmark.isClicked() && !m_benchmarkRunning)
    {
        btnBenchmark.resetClick();
        engine.audio.playClick();
        m_benchmarkRunning = true;
        btnBenchmark.enabled = false;
        btnBenchmark.label = "BENCHMARKING...";

        std::thread([this]() {
            Benchmark::runAll("benchmark_results.csv");
            m_benchmarkRunning = false;
            btnBenchmark.enabled = true;
            btnBenchmark.label = "RUN BENCHMARK";
            engine.showToast("Benchmark complete, saved to benchmark_results.csv", TOAST_INFO);
        }).detach();
        return;
    }

    if (btnTestSuite.isClicked() && !m_testsRunning)
    {
        btnTestSuite.resetClick();
        engine.audio.playClick();
        m_testsRunning = true;
        btnTestSuite.enabled = false;
        btnTestSuite.label = "TESTING...";

        std::thread([this]() {
            int p = 0, f = 0;
            TestSuite::runAll("test_report.txt", &p, &f);
            m_testsRunning = false;
            btnTestSuite.enabled = true;
            btnTestSuite.label = "RUN TEST CASES";
            std::string msg = "Tests complete (" + std::to_string(p) + "/" + std::to_string(p + f) + " passed), saved to test_report.txt";
            engine.showToast(msg, f == 0 ? TOAST_CORRECT : TOAST_ERROR);
        }).detach();
        return;
    }

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        if (kp->code == sf::Keyboard::Key::Escape)
        {
            engine.audio.playClick();
            engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
            return;
        }
    }
}

void SettingsScreen::update(float dt)
{
    (void)dt;
}

void SettingsScreen::draw(sf::RenderTarget& target)
{
    btnBack.draw(target, engine.theme);

    float cx = windowSize.x * 0.5f;

    // Header Title
    sf::Text title(engine.theme.fontUI, "SYSTEM SETTINGS", 26);
    title.setFillColor(Theme::TextPrimary);
    sf::FloatRect tb = title.getLocalBounds();
    title.setPosition(sf::Vector2f(std::round(cx - tb.size.x * 0.5f - tb.position.x), 36.0f));
    target.draw(title);

    // Main Panel Container
    float pw = std::min(680.0f, windowSize.x - 60.0f);
    float ph = 480.0f;
    float px = std::round(cx - pw * 0.5f);
    float py = 90.0f;
    sf::FloatRect panelBounds(sf::Vector2f(px, py), sf::Vector2f(pw, ph));

    engine.theme.drawPanel(target, panelBounds, Theme::BorderCyan, 1.2f);
    engine.theme.drawGlowOutline(target, panelBounds, Theme::BorderCyan, 0.6f);

    // Section 1: Audio Control
    sf::Text audioLabel(engine.theme.fontUI, "MASTER AUDIO SYSTEM", 15);
    audioLabel.setFillColor(Theme::BorderCyan);
    sf::FloatRect alb = audioLabel.getLocalBounds();
    audioLabel.setPosition(sf::Vector2f(std::round(cx - alb.size.x * 0.5f - alb.position.x), py + 25.0f));
    target.draw(audioLabel);

    btnAudioToggle.draw(target, engine.theme);

    // Section 2: Animation Speed
    sf::Text speedLabel(engine.theme.fontUI, "GLOBAL ANIMATION SPEED", 15);
    speedLabel.setFillColor(Theme::BorderCyan);
    sf::FloatRect slb = speedLabel.getLocalBounds();
    speedLabel.setPosition(sf::Vector2f(std::round(cx - slb.size.x * 0.5f - slb.position.x), py + 115.0f));
    target.draw(speedLabel);

    btnSpeed05.draw(target, engine.theme);
    btnSpeed10.draw(target, engine.theme);
    btnSpeed15.draw(target, engine.theme);
    btnSpeed20.draw(target, engine.theme);

    // Section 3: Reset Campaign Progress
    sf::Text resetLabel(engine.theme.fontUI, "SESSION PROGRESSION", 15);
    resetLabel.setFillColor(Theme::BorderCyan);
    sf::FloatRect rlb = resetLabel.getLocalBounds();
    resetLabel.setPosition(sf::Vector2f(std::round(cx - rlb.size.x * 0.5f - rlb.position.x), py + 205.0f));
    target.draw(resetLabel);

    btnResetProgress.draw(target, engine.theme);

    // Section 4: Diagnostics & Benchmarks
    sf::Text diagLabel(engine.theme.fontUI, "SYSTEM DIAGNOSTICS & BENCHMARKS", 15);
    diagLabel.setFillColor(Theme::BorderCyan);
    sf::FloatRect dlb = diagLabel.getLocalBounds();
    diagLabel.setPosition(sf::Vector2f(std::round(cx - dlb.size.x * 0.5f - dlb.position.x), py + 295.0f));
    target.draw(diagLabel);

    btnBenchmark.draw(target, engine.theme);
    btnTestSuite.draw(target, engine.theme);
}
