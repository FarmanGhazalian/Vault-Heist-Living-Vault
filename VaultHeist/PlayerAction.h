#pragma once

#include <string>

// Maximum array limit (No vectors)
const int MAX_PATH_SIZE = 64;

// -------------------------------------------------------------
// Action Codes (Har Level ke Action ka Number)
// -------------------------------------------------------------
const int CLICK_NODE = 0;   // Level 1.1: Room click
const int PLACE_TOKEN = 1;   // Level 1.2: Token slot mein rakhna
const int REMOVE_TOKEN = 2;   // Level 1.2: Token slot se hatana
const int ENTER_HEIGHT = 3;   // Level 1.3: Tree height
const int ENTER_DIAMETER = 4;   // Level 1.3: Tree diameter
const int SELECT_NODE = 5;   // Level 1.4: Room select
const int CLICK_LCA = 6;   // Level 1.4: Meeting room click
const int TRACE_PATH = 7;   // Level 1.4: Path trace
const int SWAP_CHILDREN = 8;   // Level 1.5: Swap rooms
const int AUTO_MIRROR = 9;   // Level 1.5: Auto solve

// -------------------------------------------------------------
// 1. Player Input Struct (GUI -> Backend)
// -------------------------------------------------------------
struct PlayerAction
{
    int type = CLICK_NODE;       // Action code (CLICK_NODE = 0)
    int nodeA = -1;              // Click kiya hua Room ID
    int nodeB = -1;              // Doosra Room ID (agar chahiye ho)
    int value = 0;               // Extra numbers (height/diameter)
    int slot = -1;              // Slot number

    int path[MAX_PATH_SIZE];     // Normal array (bina vector ke)
    int pathSize = 0;            // Total elements in path
};

// -------------------------------------------------------------
// 2. Action Result Struct (Backend -> GUI)
// -------------------------------------------------------------
struct ActionResult
{
    bool accepted = false;       // Move process hua? (true/false)
    bool correct = false;       // Click sahi tha? (true/false)
    int  alarmDelta = 0;         // Alarm kitna badha (0 ya +1 ya +2)
    int  penaltyPoints = 0;      // Kitne points katay
    std::string message;         // Screen par dikhane wala text
};


