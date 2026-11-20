#include "SaveSystem.h"

int SaveSystem::s_state[5][7] = {};
bool SaveSystem::s_loaded = false;
bool SaveSystem::s_devUnlockedAll = false;

void SaveSystem::load()
{
    // Fresh start: everything locked
    for (int a = 0; a < 5; a++)
    {
        for (int lv = 0; lv < 7; lv++)
        {
            s_state[a][lv] = -1;
        }
    }

    // Level 1.1 always unlocked, zero stars, no disk progress loaded
    s_state[1][1] = 0;
    s_devUnlockedAll = false;
    s_loaded = true;
}

void SaveSystem::save()
{
    // Fresh start policy: Do not write campaign progress to disk.
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

    // Winning a level unlocks the next level for this session only
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
}

bool SaveSystem::toggleUnlockAll()
{
    if (!s_loaded) load();
    s_devUnlockedAll = !s_devUnlockedAll;
    if (s_devUnlockedAll)
    {
        for (int a = 1; a <= 4; a++)
        {
            for (int lv = 1; lv <= 5; lv++)
            {
                s_state[a][lv] = 0;
            }
        }
    }
    else
    {
        // Revert to pristine fresh start state: all locked except 1.1, stars cleared
        for (int a = 0; a < 5; a++)
        {
            for (int lv = 0; lv < 7; lv++)
            {
                s_state[a][lv] = -1;
            }
        }
        s_state[1][1] = 0;
    }
    return s_devUnlockedAll;
}
