#include "SaveSystem.h"
#include <fstream>
#include <sstream>
#include <string>
#include <cstring>

int SaveSystem::s_state[5][7] = {};
bool SaveSystem::s_loaded = false;

bool SaveSystem::s_devUnlockedAll = false;

static std::string getSavePath()
{
    return "savegame.dat";
}

void SaveSystem::load()
{
    // Initialize: everything locked
    for (int a = 0; a < 5; a++)
        for (int lv = 0; lv < 7; lv++)
            s_state[a][lv] = -1;

    // Level 1.1 always unlocked
    s_state[1][1] = 0;

    std::ifstream f(getSavePath());
    if (!f.is_open())
    {
        s_loaded = true;
        return;
    }

    std::string line;
    while (std::getline(f, line))
    {
        if (line.empty()) continue;
        int act = 0, lv = 0, stars = -1;
        char dot = 0, eq = 0;
        std::istringstream ss(line);
        ss >> act >> dot >> lv >> eq >> stars;
        if (act >= 1 && act <= 4 && lv >= 1 && lv <= 5)
        {
            s_state[act][lv] = stars;
        }
    }
    // Always ensure 1.1 is at least unlocked
    if (s_state[1][1] < 0) s_state[1][1] = 0;
    s_loaded = true;
}

void SaveSystem::save()
{
    std::ofstream f(getSavePath());
    if (!f.is_open()) return;
    for (int a = 1; a <= 4; a++)
    {
        for (int lv = 1; lv <= 5; lv++)
        {
            if (s_state[a][lv] >= 0)
            {
                f << a << "." << lv << "=" << s_state[a][lv] << "\n";
            }
        }
    }
}

bool SaveSystem::isUnlocked(int act, int level)
{
    if (!s_loaded) load();
    if (act < 1 || act > 4 || level < 1 || level > 5) return false;
    return s_state[act][level] >= 0;
}

int SaveSystem::getStars(int act, int level)
{
    if (!s_loaded) load();
    if (act < 1 || act > 4 || level < 1 || level > 5) return -1;
    return s_state[act][level];
}

void SaveSystem::setWon(int act, int level, int stars)
{
    if (!s_loaded) load();
    if (act < 1 || act > 4 || level < 1 || level > 5) return;

    if (stars > s_state[act][level])
        s_state[act][level] = stars;

    int nextLevel = level + 1;
    int nextAct = act;
    if (nextLevel > 5)
    {
        nextLevel = 1;
        nextAct = act + 1;
    }
    if (nextAct <= 4 && s_state[nextAct][nextLevel] < 0)
    {
        s_state[nextAct][nextLevel] = 0;
    }
    save();
}

bool SaveSystem::toggleUnlockAll()
{
    s_devUnlockedAll = !s_devUnlockedAll;
    if (s_devUnlockedAll)
    {
        for (int a = 1; a <= 4; a++)
            for (int lv = 1; lv <= 5; lv++)
                s_state[a][lv] = 0;
    }
    else
    {
        // Revert to pristine start state: all locked except 1.1
        for (int a = 0; a < 5; a++)
            for (int lv = 0; lv < 7; lv++)
                s_state[a][lv] = -1;
        s_state[1][1] = 0;
    }
    return s_devUnlockedAll;
}
