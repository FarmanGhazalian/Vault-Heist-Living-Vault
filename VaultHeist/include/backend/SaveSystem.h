#pragma once
#include <string>

// SaveSystem: Tracks level unlock status and star ratings.
// Level 1.1 is always unlocked. Winning a level unlocks the next.
// Data persisted to savegame.dat next to executable.
class SaveSystem
{
public:
    static void load();
    static void save();

    // Returns true if the level is accessible (unlocked or won)
    static bool isUnlocked(int act, int level);

    // Returns star count (0-3), or -1 if not yet won
    static int getStars(int act, int level);

    // Mark a level as won with given stars; also unlocks the next level
    static void setWon(int act, int level, int stars);

    // Dev shortcut: toggle unlock all levels <-> lock to start state
    static bool toggleUnlockAll();
    static bool s_devUnlockedAll;

    static bool s_loaded;

private:
    // s_state[act][level]: -1 = locked, 0 = unlocked (not won), 1-3 = won with N stars
    static int s_state[5][7]; // [1..4][1..5], index 0 unused
};
