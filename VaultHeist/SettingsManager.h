#pragma once
#include <string>
#include "AudioManager.h"

class SettingsManager
{
public:
    static bool s_audioEnabled;
    static float s_animSpeed;
    static bool s_initialized;

    static void init(AudioManager& audio);
    static void load(AudioManager& audio);
    static void save();
    static void resetProgress();
    static void setAudio(bool enabled, AudioManager& audio);
    static void setSpeed(float speed);
};
