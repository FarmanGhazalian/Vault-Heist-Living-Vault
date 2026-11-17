#pragma once
#include <SFML/Audio.hpp>
#include <vector>
#include <optional>
#include <cstdint>

class AudioManager
{
private:
	bool muted;

	sf::SoundBuffer bufClick;
	sf::SoundBuffer bufCorrect;
	sf::SoundBuffer bufWrong;
	sf::SoundBuffer bufHint;
	sf::SoundBuffer bufWin;
	sf::SoundBuffer bufLose;

	std::optional<sf::Sound> soundClick;
	std::optional<sf::Sound> soundCorrect;
	std::optional<sf::Sound> soundWrong;
	std::optional<sf::Sound> soundHint;
	std::optional<sf::Sound> soundWin;
	std::optional<sf::Sound> soundLose;

	void generateTone(sf::SoundBuffer& buffer, float freqStart, float freqEnd, float durationSeconds, float volume = 0.5f);

public:
	AudioManager();

	void init();
	void setMuted(bool m);
	bool isMuted() const;
	void toggleMute();

	void playClick();
	void playCorrect();
	void playWrong();
	void playHint();
	void playWin();
	void playLose();
};
