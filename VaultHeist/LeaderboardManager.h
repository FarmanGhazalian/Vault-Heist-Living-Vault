#pragma once
#include <string>
#include <vector>

struct LeaderboardRecord
{
    std::string name;
    std::string mode;
    std::string waves_or_stars;
    int score;
};

class LeaderboardManager
{
public:
    static bool s_loaded;
    static std::vector<LeaderboardRecord> s_records;

    static void init();
    static void load();
    static void save();
    static const std::vector<LeaderboardRecord>& getRecords();

    // Returns rank 1..10 if inserted, or 0 if did not make top 10
    static int submitScore(const std::string& name, const std::string& mode, const std::string& wavesOrStars, int score);
};
