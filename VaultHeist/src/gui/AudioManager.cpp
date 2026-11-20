#include "gui/AudioManager.h"
#include <cmath>

AudioManager::AudioManager()
	: muted(false)
{
}

void AudioManager::generateTone(sf::SoundBuffer& buffer, float freqStart, float freqEnd, float durationSeconds, float volume)
{
	const unsigned int sampleRate = 44100;
	const unsigned int sampleCount = static_cast<unsigned int>(sampleRate * durationSeconds);
	if (sampleCount == 0) return;

	std::vector<std::int16_t> samples(sampleCount);
	const double pi = 3.14159265358979323846;
	double phase = 0.0;

	for (unsigned int i = 0; i < sampleCount; i++)
	{
		float t = static_cast<float>(i) / static_cast<float>(sampleCount);
		float currentFreq = freqStart + (freqEnd - freqStart) * t;

		// Apply simple envelope to avoid clicks
		float envelope = 1.0f;
		if (t < 0.1f)
		{
			envelope = t / 0.1f;
		}
		else if (t > 0.8f)
		{
			envelope = (1.0f - t) / 0.2f;
		}

		double sampleVal = std::sin(phase) * 32767.0 * volume * envelope;
		samples[i] = static_cast<std::int16_t>(sampleVal);

		phase += 2.0 * pi * currentFreq / sampleRate;
		if (phase > 2.0 * pi) phase -= 2.0 * pi;
	}

	(void)buffer.loadFromSamples(samples.data(), sampleCount, 1, sampleRate, { sf::SoundChannel::Mono });
}

void AudioManager::init()
{
	// Generate synthesized game tones
	generateTone(bufClick,   800.0f, 950.0f,  0.04f, 0.35f);
	generateTone(bufCorrect, 520.0f, 880.0f,  0.14f, 0.45f);
	generateTone(bufWrong,   240.0f, 140.0f,  0.18f, 0.50f);
	generateTone(bufHint,    750.0f, 1050.0f, 0.15f, 0.35f);
	generateTone(bufWin,     440.0f, 1100.0f, 0.35f, 0.50f);
	generateTone(bufLose,    400.0f, 110.0f,  0.35f, 0.50f);

	soundClick.emplace(bufClick);
	soundCorrect.emplace(bufCorrect);
	soundWrong.emplace(bufWrong);
	soundHint.emplace(bufHint);
	soundWin.emplace(bufWin);
	soundLose.emplace(bufLose);
}

void AudioManager::setMuted(bool m)
{
	muted = m;
}

bool AudioManager::isMuted() const
{
	return muted;
}

void AudioManager::toggleMute()
{
	muted = !muted;
}

void AudioManager::playClick()
{
	if (!muted && soundClick)
	{
		soundClick->play();
	}
}

void AudioManager::playCorrect()
{
	if (!muted && soundCorrect)
	{
		soundCorrect->play();
	}
}

void AudioManager::playWrong()
{
	if (!muted && soundWrong)
	{
		soundWrong->play();
	}
}

void AudioManager::playHint()
{
	if (!muted && soundHint)
	{
		soundHint->play();
	}
}

void AudioManager::playWin()
{
	if (!muted && soundWin)
	{
		soundWin->play();
	}
}

void AudioManager::playLose()
{
	if (!muted && soundLose)
	{
		soundLose->play();
	}
}
