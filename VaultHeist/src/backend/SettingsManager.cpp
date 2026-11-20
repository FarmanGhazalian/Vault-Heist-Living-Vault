#include "backend/SettingsManager.h"
#include "gui/AnimationController.h"
#include "backend/SaveSystem.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <iostream>

bool SettingsManager::s_audioEnabled = true;
float SettingsManager::s_animSpeed = 1.0f;
bool SettingsManager::s_initialized = false;

static const std::string SETTINGS_FILE = "settings.json";

void SettingsManager::init(AudioManager& audio)
{
    if (!s_initialized)
    {
        load(audio);
        s_initialized = true;
    }
}

void SettingsManager::load(AudioManager& audio)
{
    s_audioEnabled = true;
    s_animSpeed = 1.0f;

    std::ifstream file(SETTINGS_FILE);
    if (file.is_open())
    {
        try
        {
            nlohmann::json j;
            file >> j;
            if (j.contains("audio") && j["audio"].is_boolean())
            {
                s_audioEnabled = j["audio"].get<bool>();
            }
            if (j.contains("speed") && j["speed"].is_number())
            {
                s_animSpeed = j["speed"].get<float>();
                if (s_animSpeed <= 0.1f) s_animSpeed = 1.0f;
            }
        }
        catch (...)
        {
            s_audioEnabled = true;
            s_animSpeed = 1.0f;
        }
        file.close();
    }
    else
    {
        save();
    }

    audio.setMuted(!s_audioEnabled);
    AnimationController::setSpeed(s_animSpeed);
}

void SettingsManager::save()
{
    try
    {
        nlohmann::json j;
        j["audio"] = s_audioEnabled;
        j["speed"] = s_animSpeed;
        std::ofstream file(SETTINGS_FILE);
        if (file.is_open())
        {
            file << j.dump(4);
            file.close();
        }
    }
    catch (...)
    {
    }
}

void SettingsManager::setAudio(bool enabled, AudioManager& audio)
{
    s_audioEnabled = enabled;
    audio.setMuted(!s_audioEnabled);
    save();
}

void SettingsManager::setSpeed(float speed)
{
    s_animSpeed = speed;
    AnimationController::setSpeed(speed);
    save();
}

void SettingsManager::resetProgress()
{
    SaveSystem::load();
}
