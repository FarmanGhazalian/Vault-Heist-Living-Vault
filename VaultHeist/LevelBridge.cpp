#include "LevelBridge.h"
#include "Act1level1.h"
#include "Act1Level2.h"
#include "Act1Level3.h"
#include "Act1Level4.h"
#include "Act1Level5.h"
#include <filesystem>

static std::string resolveJsonPath(const std::string& baseName)
{
    // Check runtime first (handles integer levelId for Act1Level2)
    std::string runtimeName = baseName;
    size_t dot = runtimeName.find(".json");
    if (dot != std::string::npos)
    {
        runtimeName = runtimeName.substr(0, dot) + "_runtime.json";
    }

    std::vector<std::string> candidates = {
        runtimeName,
        "VaultHeist/" + runtimeName,
        baseName,
        "VaultHeist/" + baseName
    };

    for (const auto& path : candidates)
    {
        if (std::filesystem::exists(path))
        {
            return path;
        }
    }
    return baseName;
}

LevelBridge::LevelBridge()
    : m_act(1)
    , m_level(1)
    , m_logic(nullptr)
    , m_l1(nullptr)
    , m_l2(nullptr)
    , m_l3(nullptr)
    , m_l4(nullptr)
    , m_l5(nullptr)
    , m_fallbackState(0)
{
}

LevelBridge::~LevelBridge() = default;

bool LevelBridge::loadLevel(int act, int level, const std::string& jsonPath)
{
    m_act = act;
    m_level = level;
    m_l1 = nullptr;
    m_l2 = nullptr;
    m_l3 = nullptr;
    m_l4 = nullptr;
    m_l5 = nullptr;

    std::string resolvedPath = resolveJsonPath(jsonPath);

    try
    {
        if (act == 1)
        {
            if (level == 1)
            {
                auto ptr = std::make_unique<Act1Level1>();
                if (ptr->initLevel(resolvedPath) || ptr->initLevel(jsonPath))
                {
                    m_l1 = ptr.get();
                    m_logic = std::move(ptr);
                    return true;
                }
            }
            else if (level == 2)
            {
                auto ptr = std::make_unique<Act1Level2>();
                // initLevel already calls loadJSON + buildFromPreIn internally
                // Do NOT call buildFromPreIn again to avoid double-free/abort
                if (ptr->initLevel(resolvedPath) || ptr->initLevel(jsonPath))
                {
                    m_l2 = ptr.get();
                    m_logic = std::move(ptr);
                    return true;
                }
            }
            else if (level == 3)
            {
                auto ptr = std::make_unique<Act1Level3>();
                if (ptr->initLevel(resolvedPath) || ptr->initLevel(jsonPath))
                {
                    m_l3 = ptr.get();
                    m_logic = std::move(ptr);
                    return true;
                }
            }
            else if (level == 4)
            {
                auto ptr = std::make_unique<Act1Level4>();
                if (ptr->initLevel(resolvedPath) || ptr->initLevel(jsonPath))
                {
                    m_l4 = ptr.get();
                    m_logic = std::move(ptr);
                    return true;
                }
            }
            else if (level == 5)
            {
                auto ptr = std::make_unique<Act1Level5>();
                if (ptr->initLevel(resolvedPath) || ptr->initLevel(jsonPath))
                {
                    m_l5 = ptr.get();
                    m_logic = std::move(ptr);
                    return true;
                }
            }
        }
    }
    catch (...)
    {
        return false;
    }

    return false;
}

ActionResult LevelBridge::processAction(const PlayerAction& action)
{
    if (m_logic)
    {
        return m_logic->processAction(action);
    }
    ActionResult res;
    res.accepted = false;
    res.message = "No active level loaded";
    return res;
}

const LevelState& LevelBridge::getState() const
{
    if (m_logic)
    {
        return m_logic->getState();
    }
    return m_fallbackState;
}

bool LevelBridge::isFinished() const
{
    return m_logic ? m_logic->isFinished() : false;
}

bool LevelBridge::isWon() const
{
    return m_logic ? m_logic->isWon() : false;
}

TreeNode* LevelBridge::getRootNode()
{
    if (m_l1) return const_cast<BinaryTree&>(m_l1->getTree()).getRoot();
    if (m_l2) return m_l2->solTree.getRoot();
    if (m_l3) return const_cast<BinaryTree&>(m_l3->getTree()).getRoot();
    if (m_l4) return const_cast<BinaryTree&>(m_l4->getTree()).getRoot();
    if (m_l5) return const_cast<BinaryTree&>(m_l5->getTree()).getRoot();
    return nullptr;
}

std::string LevelBridge::getObjective() const
{
    if (m_act == 1)
    {
        if (m_level == 1)
        {
            std::string order = getTraversalType();
            return "Patrol the vault in " + (order.empty() ? "PREORDER" : order) + " order.";
        }
        if (m_level == 2) return "Drag blueprint tokens to reconstruct the security tree.";
        if (m_level == 3)
        {
            if (isHeightsPhaseDone()) return "Report the longest path (Diameter) between two rooms.";
            return "Enter the security height for each room bottom-up.";
        }
        if (m_level == 4)
        {
            if (isLCAFound()) return "Trace the escape route between Room A and Room B.";
            return "Locate the Lowest Common Ancestor (Meeting Room) for A and B.";
        }
        if (m_level == 5) return "Mirror the vault: Swap child rooms bottom-up until inverted.";
    }
    return "Complete the vault objectives.";
}

std::string LevelBridge::getHint()
{
    if (m_logic)
    {
        m_logic->onHintRequested();
        if (m_l3) return m_l3->getHint();
        if (m_l5) return m_l5->getHint();
    }
    return "Analyze parent-child relationships and node values.";
}

std::string LevelBridge::getTraversalType() const
{
    if (m_l1) return m_l1->getTraversalType();
    return "PREORDER";
}

int LevelBridge::getTotalNodes() const
{
    if (m_l2) return m_l2->totalNodes;
    if (m_l3)
    {
        TreeNode* root = const_cast<BinaryTree&>(m_l3->getTree()).getRoot();
        return root ? root->size : 0;
    }
    return 0;
}

int LevelBridge::getPreorderValue(int idx) const
{
    if (m_l2 && idx >= 0 && idx < m_l2->totalNodes)
    {
        return m_l2->loader.data.preArr[idx];
    }
    return -1;
}

int LevelBridge::getInorderValue(int idx) const
{
    if (m_l2 && idx >= 0 && idx < m_l2->totalNodes)
    {
        return m_l2->loader.data.inArr[idx];
    }
    return -1;
}

int LevelBridge::getRoomA() const
{
    return m_l4 ? m_l4->getRoomA() : -1;
}

int LevelBridge::getRoomB() const
{
    return m_l4 ? m_l4->getRoomB() : -1;
}

bool LevelBridge::isLCAFound() const
{
    return m_l4 ? m_l4->isLCAFound() : false;
}

int LevelBridge::getCurrentPathIndex() const
{
    return m_l4 ? m_l4->getCurrentPathIndex() : 0;
}

int LevelBridge::getPathCount() const
{
    return m_l4 ? m_l4->getPathCount() : 0;
}

bool LevelBridge::isHeightsPhaseDone() const
{
    return m_l3 ? m_l3->isHeightsPhaseDone() : false;
}
