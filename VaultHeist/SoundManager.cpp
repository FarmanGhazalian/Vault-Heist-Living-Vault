#include "SoundManager.h"
#include <cmath>
#include <algorithm>

SoundManager::SoundManager() {
    init();
}

void SoundManager::init() {
    const unsigned int SR = 44100;
    auto makeSine = [&](float freq, float dur, float vol) {
        std::vector<std::int16_t> s(static_cast<size_t>(dur * SR));
        for (size_t i = 0; i < s.size(); ++i) {
            float t = static_cast<float>(i) / SR;
            float env = std::pow(1.0f - (t / dur), 1.5f);
            float v = std::sin(2.0f * 3.14159265f * freq * t) * vol * env;
            s[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
        }
        return s;
    };

    // 1. UI Click (Soft metallic pop)
    auto c = makeSine(950.0f, 0.04f, 0.25f);
    static_cast<void>(bClick.loadFromSamples(c.data(), c.size(), 1, SR, {sf::SoundChannel::Mono}));

    // 2. Correct Move (Gentle vibraphone bell chime)
    std::vector<std::int16_t> cr(static_cast<size_t>(0.35f * SR));
    for (size_t i = 0; i < cr.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float env = std::exp(-t * 8.0f);
        float v = (std::sin(2.0f * 3.14159265f * 659.25f * t) + 0.4f * std::sin(2.0f * 3.14159265f * 1318.5f * t)) * 0.35f * env;
        cr[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
    }
    static_cast<void>(bCorrect.loadFromSamples(cr.data(), cr.size(), 1, SR, {sf::SoundChannel::Mono}));

    // 3. Wrong Move (Soft low thud)
    std::vector<std::int16_t> w(static_cast<size_t>(0.25f * SR));
    for (size_t i = 0; i < w.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float env = std::exp(-t * 9.0f);
        float v = std::sin(2.0f * 3.14159265f * (160.0f - 60.0f * t) * t) * 0.4f * env;
        w[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
    }
    static_cast<void>(bWrong.loadFromSamples(w.data(), w.size(), 1, SR, {sf::SoundChannel::Mono}));

    // 4. Hint (Warm two-tone sparkle)
    std::vector<std::int16_t> hn(static_cast<size_t>(0.3f * SR));
    for (size_t i = 0; i < hn.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float freq = (t < 0.12f) ? 587.33f : 880.0f; // D5 to A5
        float env = std::exp(-std::fmod(t, 0.12f) * 12.0f);
        float v = std::sin(2.0f * 3.14159265f * freq * t) * 0.3f * env;
        hn[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
    }
    static_cast<void>(bHint.loadFromSamples(hn.data(), hn.size(), 1, SR, {sf::SoundChannel::Mono}));

    // 5. Victory Fanfare (Pleasant 4-note chime)
    std::vector<std::int16_t> vf(static_cast<size_t>(0.9f * SR));
    float vicNotes[] = {523.25f, 659.25f, 783.99f, 1046.50f};
    for (size_t i = 0; i < vf.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        int n = std::clamp(static_cast<int>(t / 0.2f), 0, 3);
        float tn = t - n * 0.2f;
        float v = std::sin(2.0f * 3.14159265f * vicNotes[n] * t) * 0.35f * std::exp(-tn * 4.0f);
        vf[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
    }
    static_cast<void>(bVictory.loadFromSamples(vf.data(), vf.size(), 1, SR, {sf::SoundChannel::Mono}));

    // 6. Defeat Sound
    std::vector<std::int16_t> df(static_cast<size_t>(0.6f * SR));
    for (size_t i = 0; i < df.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float v = std::sin(2.0f * 3.14159265f * (200.0f - 140.0f * t) * t) * 0.4f * std::exp(-t * 4.0f);
        df[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
    }
    static_cast<void>(bDefeat.loadFromSamples(df.data(), df.size(), 1, SR, {sf::SoundChannel::Mono}));

    // 7. Metallic Stamp Thud
    std::vector<std::int16_t> th(static_cast<size_t>(0.2f * SR));
    for (size_t i = 0; i < th.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        float v = (std::sin(2.0f * 3.14159265f * 90.0f * t) + 0.3f * std::sin(2.0f * 3.14159265f * 340.0f * t)) * 0.5f * std::exp(-t * 18.0f);
        th[i] = static_cast<std::int16_t>(std::clamp(v, -1.0f, 1.0f) * 32767.0f);
    }
    static_cast<void>(bThud.loadFromSamples(th.data(), th.size(), 1, SR, {sf::SoundChannel::Mono}));

    // 8. Soft, Smooth Cinematic Spy Music Loop (Ambient chords, zero harshness)
    const float dur = 4.8f;
    std::vector<std::int16_t> m(static_cast<size_t>(dur * SR));
    float chords[4][3] = {
        {220.00f, 261.63f, 329.63f}, // Am
        {174.61f, 220.00f, 261.63f}, // F
        {146.83f, 220.00f, 261.63f}, // Dm
        {164.81f, 196.00f, 246.94f}  // Em
    };
    for (size_t i = 0; i < m.size(); ++i) {
        float t = static_cast<float>(i) / SR;
        int cIdx = std::clamp(static_cast<int>(t / 1.2f), 0, 3);
        float tc = std::fmod(t, 1.2f);
        float env = std::sin(3.14159265f * (tc / 1.2f));
        float s = 0.0f;
        for (int k = 0; k < 3; ++k) {
            s += std::sin(2.0f * 3.14159265f * chords[cIdx][k] * t);
        }
        s = (s / 3.0f) * 0.20f * env;
        m[i] = static_cast<std::int16_t>(std::clamp(s, -1.0f, 1.0f) * 32767.0f);
    }
    static_cast<void>(bMusic.loadFromSamples(m.data(), m.size(), 1, SR, {sf::SoundChannel::Mono}));
    sndMusic.setLooping(true);
    sndMusic.setVolume(28.0f);
    sndMusic.play();
}

void SoundManager::playClick() { if (soundOn) sndClick.play(); }
void SoundManager::playCorrect() { if (soundOn) sndCorrect.play(); }
void SoundManager::playWrong() { if (soundOn) sndWrong.play(); }
void SoundManager::playHint() { if (soundOn) sndHint.play(); }
void SoundManager::playVictory() { if (soundOn) sndVictory.play(); }
void SoundManager::playDefeat() { if (soundOn) sndDefeat.play(); }
void SoundManager::playThud() { if (soundOn) sndThud.play(); }

void SoundManager::toggleMute() {
    soundOn = !soundOn;
    if (!soundOn) {
        sndMusic.pause();
    } else {
        sndMusic.play();
    }
}

bool SoundManager::isMuted() const {
    return !soundOn;
}
