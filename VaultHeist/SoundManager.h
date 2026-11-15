#pragma once
#include <SFML/Audio.hpp>
#include <vector>
#include <cstdint>

class SoundManager {
public:
    sf::SoundBuffer bClick;
    sf::SoundBuffer bCorrect;
    sf::SoundBuffer bWrong;
    sf::SoundBuffer bHint;
    sf::SoundBuffer bVictory;
    sf::SoundBuffer bDefeat;
    sf::SoundBuffer bThud;
    sf::SoundBuffer bMusic;

    sf::Sound sndClick{bClick};
    sf::Sound sndCorrect{bCorrect};
    sf::Sound sndWrong{bWrong};
    sf::Sound sndHint{bHint};
    sf::Sound sndVictory{bVictory};
    sf::SoundBuffer dummyBuf;
    sf::Sound sndDefeat{bDefeat};
    sf::Sound sndThud{bThud};
    sf::Sound sndMusic{bMusic};

    bool soundOn = true;

    SoundManager();
    void init();
    void playClick();
    void playCorrect();
    void playWrong();
    void playHint();
    void playVictory();
    void playDefeat();
    void playThud();
    void toggleMute();
    bool isMuted() const;
};
