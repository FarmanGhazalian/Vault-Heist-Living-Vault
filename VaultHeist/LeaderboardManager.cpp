#include "LeaderboardManager.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <algorithm>

bool LeaderboardManager::s_loaded = false;
std::vector<LeaderboardRecord> LeaderboardManager::s_records;

static const std::string LEADERBOARD_FILE = "leaderboard.json";

void LeaderboardManager::init()
{
    if (!s_loaded)
    {
        load();
        s_loaded = true;
    }
}

void LeaderboardManager::load()
{
    s_records.clear();
    std::ifstream file(LEADERBOARD_FILE);
    if (!file.is_open())
    {
        save();
        return;
    }

    try
    {
        nlohmann::json j;
        file >> j;
        if (j.is_array())
        {
            for (const auto& item : j)
            {
                LeaderboardRecord rec;
                if (item.contains("name") && item["name"].is_string())
                    rec.name = item["name"].get<std::string>();
                else
                    rec.name = "Player";

                if (item.contains("mode") && item["mode"].is_string())
                    rec.mode = item["mode"].get<std::string>();
                else
                    rec.mode = "Standard";

                if (item.contains("waves_or_stars"))
                {
                    if (item["waves_or_stars"].is_string())
                        rec.waves_or_stars = item["waves_or_stars"].get<std::string>();
                    else if (item["waves_or_stars"].is_number())
                        rec.waves_or_stars = std::to_string(item["waves_or_stars"].get<int>());
                }
                else
                {
                    rec.waves_or_stars = "-";
                }

                if (item.contains("score") && item["score"].is_number())
                    rec.score = item["score"].get<int>();
                else
                    rec.score = 0;

                s_records.push_back(rec);
            }
        }
    }
    catch (...)
    {
        s_records.clear();
    }
    file.close();

    std::sort(s_records.begin(), s_records.end(), [](const LeaderboardRecord& a, const LeaderboardRecord& b) {
        return a.score > b.score;
    });
    if (s_records.size() > 10)
    {
        s_records.resize(10);
    }
}

void LeaderboardManager::save()
{
    try
    {
        nlohmann::json j = nlohmann::json::array();
        for (const auto& rec : s_records)
        {
            nlohmann::json item;
            item["name"] = rec.name;
            item["mode"] = rec.mode;
            item["waves_or_stars"] = rec.waves_or_stars;
            item["score"] = rec.score;
            j.push_back(item);
        }
        std::ofstream file(LEADERBOARD_FILE);
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

const std::vector<LeaderboardRecord>& LeaderboardManager::getRecords()
{
    init();
    return s_records;
}

int LeaderboardManager::submitScore(const std::string& name, const std::string& mode, const std::string& wavesOrStars, int score)
{
    init();

    // Insert only if makes top 10 (or table has fewer than 10)
    if (s_records.size() < 10 || score > s_records.back().score)
    {
        LeaderboardRecord newRec;
        newRec.name = name.empty() ? "Player" : name;
        newRec.mode = mode;
        newRec.waves_or_stars = wavesOrStars;
        newRec.score = score;

        s_records.push_back(newRec);

        std::stable_sort(s_records.begin(), s_records.end(), [](const LeaderboardRecord& a, const LeaderboardRecord& b) {
            return a.score > b.score;
        });

        int rank = 0;
        for (size_t i = 0; i < s_records.size(); ++i)
        {
            if (s_records[i].name == newRec.name && s_records[i].score == newRec.score && s_records[i].mode == newRec.mode)
            {
                rank = static_cast<int>(i) + 1;
                break;
            }
        }

        if (s_records.size() > 10)
        {
            s_records.resize(10);
        }

        save();
        return (rank <= 10) ? rank : 0;
    }

    return 0;
}
