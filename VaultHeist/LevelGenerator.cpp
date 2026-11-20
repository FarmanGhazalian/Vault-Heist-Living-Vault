#include "LevelGenerator.h"
#include "AVL.h"
#include "MaxHeap.h"
#include <iostream>
#include <set>

bool LevelGenerator::processJson(nlohmann::json& j)
{
    if (!hasGenerator(j))
    {
        return false;
    }

    uint32_t seed = 42;
    if (j.contains("seed"))
    {
        seed = j["seed"].get<uint32_t>();
    }
    else if (j["generator"].contains("seed"))
    {
        seed = j["generator"]["seed"].get<uint32_t>();
    }

    const auto& gen = j["generator"];
    std::string type = gen.value("type", "");

    if (type == "act1_tree")
    {
        return generateAct1Tree(j, seed, gen);
    }
    else if (type == "act2_bst")
    {
        return generateAct2Bst(j, seed, gen);
    }
    else if (type == "act3_avl")
    {
        return generateAct3Avl(j, seed, gen);
    }
    else if (type == "act4_heap")
    {
        return generateAct4Heap(j, seed, gen);
    }

    return false;
}

// ---------------------------------------------------------
// ACT 1: Binary Tree Generator
// ---------------------------------------------------------
struct SimpleGenNode
{
    int id;
    int value;
    int leftId = -1;
    int rightId = -1;
};

static void collectPreIn(int currId, const std::vector<SimpleGenNode>& nodes, std::vector<int>& pre, std::vector<int>& in)
{
    if (currId == -1) return;
    const SimpleGenNode* n = nullptr;
    for (const auto& item : nodes)
    {
        if (item.id == currId)
        {
            n = &item;
            break;
        }
    }
    if (!n) return;

    pre.push_back(n->value);
    collectPreIn(n->leftId, nodes, pre, in);
    in.push_back(n->value);
    collectPreIn(n->rightId, nodes, pre, in);
}

bool LevelGenerator::generateAct1Tree(nlohmann::json& j, uint32_t seed, const nlohmann::json& gen)
{
    std::mt19937 rng(seed);
    int count = gen.value("count", 7);
    int minVal = gen.value("minValue", 1);
    int maxVal = gen.value("maxValue", 99);
    bool distinct = gen.value("distinct", true);

    std::vector<int> values;
    if (distinct)
        values = generateUniqueValues(count, minVal, maxVal, rng);
    else
        values = generateValues(count, minVal, maxVal, rng);

    if (values.size() < static_cast<size_t>(count)) return false;

    std::vector<SimpleGenNode> nodes(count);
    for (int i = 0; i < count; ++i)
    {
        nodes[i].id = 101 + i;
        nodes[i].value = values[i];
    }

    // Connect into a tree structure
    std::string shape = gen.value("shape", "balanced");
    if (shape == "left_skewed")
    {
        for (int i = 0; i < count - 1; ++i)
            nodes[i].leftId = nodes[i + 1].id;
    }
    else if (shape == "right_skewed")
    {
        for (int i = 0; i < count - 1; ++i)
            nodes[i].rightId = nodes[i + 1].id;
    }
    else
    {
        // Balanced / binary heap shape
        for (int i = 0; i < count; ++i)
        {
            int leftIdx = 2 * i + 1;
            int rightIdx = 2 * i + 2;
            if (leftIdx < count) nodes[i].leftId = nodes[leftIdx].id;
            if (rightIdx < count) nodes[i].rightId = nodes[rightIdx].id;
        }
    }

    // Populate Level 1.1 "nodes" array format
    nlohmann::json nodeArr = nlohmann::json::array();
    for (const auto& n : nodes)
    {
        nlohmann::json item;
        item["id"] = n.id;
        item["value"] = n.value;
        item["left"] = n.leftId;
        item["right"] = n.rightId;
        nodeArr.push_back(item);
    }
    j["nodes"] = nodeArr;
    j["rootId"] = nodes[0].id;
    j["totalNodes"] = count;

    // Also populate preorderSequence & inorderSequence (used by 1.2, 1.3, 1.4, 1.5)
    std::vector<int> pre, in;
    collectPreIn(nodes[0].id, nodes, pre, in);
    j["preorderSequence"] = pre;
    j["inorderSequence"] = in;

    // Level 1.4 requires roomA and roomB
    if (j.contains("roomA") || gen.contains("roomA") || j.value("levelId", "") == "1.4")
    {
        // Pick two distinct leaf/descendant nodes deterministically
        if (count >= 4)
        {
            j["roomA"] = values[count - 2];
            j["roomB"] = values[count - 1];
        }
    }

    return true;
}

// ---------------------------------------------------------
// ACT 2: BST Generator
// ---------------------------------------------------------
bool LevelGenerator::generateAct2Bst(nlohmann::json& j, uint32_t seed, const nlohmann::json& gen)
{
    std::mt19937 rng(seed);
    int count = gen.value("count", 7);
    int minVal = gen.value("minValue", 10);
    int maxVal = gen.value("maxValue", 90);
    std::string shape = gen.value("shape", "random");

    std::vector<int> keys;
    if (shape == "skewed")
    {
        // Sorted ascending or descending for skewed tree
        keys = generateUniqueValues(count, minVal, maxVal, rng);
        std::sort(keys.begin(), keys.end());
    }
    else
    {
        keys = generateUniqueValues(count, minVal, maxVal, rng);
    }

    if (keys.size() < static_cast<size_t>(count)) return false;

    // Check specific Level needs:
    // Level 2.1: keys + target
    if (j.contains("target") || gen.contains("hasTarget"))
    {
        j["keys"] = keys;
        // Target is guaranteed to be in keys
        std::uniform_int_distribution<size_t> tDist(1, keys.size() - 1);
        j["target"] = keys[tDist(rng)];
    }
    // Level 2.2: initialKeys + insertKeys
    else if (j.contains("initialKeys") && j.contains("insertKeys"))
    {
        int initialCount = gen.value("initialCount", 5);
        int insertCount = gen.value("insertCount", 3);
        int totalNeeded = initialCount + insertCount;
        std::vector<int> allKeys = generateUniqueValues(totalNeeded, minVal, maxVal, rng);
        std::vector<int> initK(allKeys.begin(), allKeys.begin() + initialCount);
        std::vector<int> insK(allKeys.begin() + initialCount, allKeys.end());
        j["initialKeys"] = initK;
        j["insertKeys"] = insK;
    }
    // Level 2.3: initialKeys + deleteKeys
    else if (j.contains("initialKeys") && j.contains("deleteKeys"))
    {
        int initialCount = gen.value("initialCount", 7);
        int deleteCount = gen.value("deleteCount", 3);
        keys = generateUniqueValues(initialCount, minVal, maxVal, rng);
        j["initialKeys"] = keys;
        std::vector<int> delCandidates = keys;
        deterministicShuffle(delCandidates, rng);
        delCandidates.resize(deleteCount);
        j["deleteKeys"] = delCandidates;
    }
    // Level 2.4: Range Raid: keys, lo, hi
    else if (j.contains("lo") && j.contains("hi"))
    {
        j["keys"] = keys;
        std::vector<int> sortedK = keys;
        std::sort(sortedK.begin(), sortedK.end());
        int loIdx = count / 4;
        int hiIdx = (count * 3) / 4;
        if (loIdx >= hiIdx) hiIdx = loIdx + 1;
        if (hiIdx >= count) hiIdx = count - 1;
        j["lo"] = sortedK[loIdx];
        j["hi"] = sortedK[hiIdx];
    }
    // Level 2.5: The Lazy Architect: skewed keys + targets
    else if (j.contains("targets"))
    {
        j["keys"] = keys;
        int tCount = gen.value("targetCount", 2);
        std::vector<int> targets;
        std::vector<int> cand = keys;
        deterministicShuffle(cand, rng);
        for (int i = 0; i < tCount && i < static_cast<int>(cand.size()); ++i)
        {
            targets.push_back(cand[i]);
        }
        j["targets"] = targets;
    }
    else
    {
        j["keys"] = keys;
    }

    return true;
}

// ---------------------------------------------------------
// ACT 3: AVL Tree Generator (with retry loop for required cases)
// ---------------------------------------------------------
bool LevelGenerator::generateAct3Avl(nlohmann::json& j, uint32_t baseSeed, const nlohmann::json& gen)
{
    std::string targetCase = gen.value("targetCase", "any"); // "LL", "RR", "LR", "RL", "badges", "kth"
    int count = gen.value("count", 5);
    int minVal = gen.value("minValue", 10);
    int maxVal = gen.value("maxValue", 80);

    for (uint32_t attempt = 0; attempt < 50; ++attempt)
    {
        uint32_t curSeed = baseSeed + attempt;
        std::mt19937 rng(curSeed);

        if (targetCase == "badges")
        {
            // Level 3.1: Needs at least one unbalanced node in unrotated BST/AVL
            std::vector<int> keys = generateUniqueValues(count, minVal, maxVal, rng);
            AVL avl;
            for (int k : keys) avl.insert(k);
            avl.updateHeights();
            int unbCount = 0;
            int* unbArr = avl.getUnbalancedNodes(unbCount);
            if (unbArr) delete[] unbArr;

            if (unbCount >= 1)
            {
                j["keys"] = keys;
                return true;
            }
        }
        else if (targetCase == "LL" || targetCase == "RR" || targetCase == "LR" || targetCase == "RL")
        {
            // Levels 3.2, 3.3: Insertion sequence that creates exactly this rotation case
            int expectedProb = (targetCase == "LL") ? 0 : (targetCase == "RR") ? 1 : (targetCase == "LR") ? 2 : 3;

            std::vector<int> keys = generateUniqueValues(count, minVal, maxVal, rng);
            AVL avl;
            for (int k : keys) avl.insert(k);
            avl.updateHeights();

            TreeNode* unb = avl.findUnbalancedNode();
            if (unb != nullptr)
            {
                int p = avl.getProblem(unb);
                if (p == expectedProb)
                {
                    j["keys"] = keys;
                    return true;
                }
            }
        }
        else if (targetCase == "insert_predict")
        {
            // Level 3.4: Initial keys + insert keys
            int initCount = gen.value("initialCount", 5);
            int insCount = gen.value("insertCount", 3);
            std::vector<int> allKeys = generateUniqueValues(initCount + insCount, minVal, maxVal, rng);
            std::vector<int> initK(allKeys.begin(), allKeys.begin() + initCount);
            std::vector<int> insK(allKeys.begin() + initCount, allKeys.end());
            j["keys"] = initK;
            j["insertKeys"] = insK;
            return true;
        }
        else if (targetCase == "kth")
        {
            // Level 3.5: Keys + k
            std::vector<int> keys = generateUniqueValues(count, minVal, maxVal, rng);
            int k = gen.value("k", 3);
            if (k > count) k = count;
            j["keys"] = keys;
            j["k"] = k;
            return true;
        }
        else
        {
            std::vector<int> keys = generateUniqueValues(count, minVal, maxVal, rng);
            j["keys"] = keys;
            return true;
        }
    }

    return false; // None of 50 seeds produced required AVL case
}

// ---------------------------------------------------------
// ACT 4: Heap Generator
// ---------------------------------------------------------
bool LevelGenerator::generateAct4Heap(nlohmann::json& j, uint32_t seed, const nlohmann::json& gen)
{
    std::mt19937 rng(seed);
    int count = gen.value("count", 7);
    int minVal = gen.value("minValue", 10);
    int maxVal = gen.value("maxValue", 99);
    std::string condition = gen.value("condition", "random"); // "random", "reverse_sorted", "already_heap", "duplicate_heavy"

    std::vector<int> values;
    if (condition == "duplicate_heavy")
    {
        // Smaller range to force duplicates
        values = generateValues(count, minVal, minVal + (count / 2), rng);
    }
    else if (condition == "reverse_sorted")
    {
        values = generateUniqueValues(count, minVal, maxVal, rng);
        std::sort(values.begin(), values.end()); // ascending order is worst-case for max-heap build
    }
    else if (condition == "already_heap")
    {
        values = generateUniqueValues(count, minVal, maxVal, rng);
        MaxHeap mh(values.data(), static_cast<int>(values.size()));
        mh.buildMaxHeap();
        for (int i = 0; i < count; ++i)
        {
            values[i] = mh.getAt(i);
        }
    }
    else
    {
        // Standard random non-heap array
        values = generateUniqueValues(count, minVal, maxVal, rng);
    }

    // Ensure it's not accidentally already a max-heap unless requested
    if (condition != "already_heap" && count >= 3)
    {
        MaxHeap testHeap(values.data(), static_cast<int>(values.size()));
        if (testHeap.isMaxHeap())
        {
            // Swap root with smaller element to break max-heap property
            std::swap(values[0], values[count - 1]);
        }
    }

    j["values"] = values;
    return true;
}
