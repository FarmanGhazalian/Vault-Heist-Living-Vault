#pragma once
#include <vector>
#include <string>
#include <random>
#include <cstdint>
#include <algorithm>
#include "nlohmann/json.hpp"

class LevelGenerator
{
public:
    // Custom deterministic Fisher-Yates shuffle using std::mt19937
    template<typename T>
    static void deterministicShuffle(std::vector<T>& vec, std::mt19937& rng)
    {
        if (vec.empty()) return;
        for (size_t i = vec.size() - 1; i > 0; --i)
        {
            std::uniform_int_distribution<size_t> dist(0, i);
            size_t j = dist(rng);
            std::swap(vec[i], vec[j]);
        }
    }

    // Generate unique random integers in range [minVal, maxVal]
    static std::vector<int> generateUniqueValues(int count, int minVal, int maxVal, std::mt19937& rng)
    {
        std::vector<int> pool;
        for (int v = minVal; v <= maxVal; ++v)
        {
            pool.push_back(v);
        }
        deterministicShuffle(pool, rng);
        if (count < static_cast<int>(pool.size()))
        {
            pool.resize(count);
        }
        return pool;
    }

    // Generate random integers (duplicates allowed) in range [minVal, maxVal]
    static std::vector<int> generateValues(int count, int minVal, int maxVal, std::mt19937& rng)
    {
        std::vector<int> res;
        res.reserve(count);
        std::uniform_int_distribution<int> dist(minVal, maxVal);
        for (int i = 0; i < count; ++i)
        {
            res.push_back(dist(rng));
        }
        return res;
    }

    // Check if JSON contains a "generator" configuration block
    static bool hasGenerator(const nlohmann::json& j)
    {
        return j.contains("generator") && j["generator"].is_object();
    }

    // Main entry point: process a JSON object in place. If it has a generator block,
    // generate the level data deterministically from its seed and fill the JSON fields.
    static bool processJson(nlohmann::json& j);

private:
    static bool generateAct1Tree(nlohmann::json& j, uint32_t seed, const nlohmann::json& gen);
    static bool generateAct2Bst(nlohmann::json& j, uint32_t seed, const nlohmann::json& gen);
    static bool generateAct3Avl(nlohmann::json& j, uint32_t seed, const nlohmann::json& gen);
    static bool generateAct4Heap(nlohmann::json& j, uint32_t seed, const nlohmann::json& gen);
};
