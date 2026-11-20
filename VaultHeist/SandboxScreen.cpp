#include "SandboxScreen.h"
#include "MainMenuScreen.h"
#include "GameEngine.h"
#include <cmath>
#include <algorithm>
#include <random>
#include <sstream>
#include <ctime>

SandboxScreen::SandboxScreen(GameEngine& gameEngine)
    : engine(gameEngine),
      windowSize(gameEngine.getWindowSize()),
      m_mode(SB_BST),
      m_currentSeed(1337),
      m_autoBalance(true),
      m_heap(nullptr),
      m_heapActiveSize(0),
      m_root(nullptr),
      m_selectedNode(nullptr),
      m_hoveredNode(nullptr),
      m_panOffset(0.0f, 0.0f),
      m_isPanning(false),
      m_zoomLevel(1.0f),
      m_animCurrentIndex(-1),
      m_animStepTimer(0.0f),
      m_activeInputBox(-1),
      m_inputValue("42"),
      m_inputLow("20"),
      m_inputHigh("60"),
      m_cursorTimer(0.0f),
      m_cursorVisible(true),
      m_logScrollOffset(0),
      btnBack("< BACK", sf::FloatRect(sf::Vector2f(20.0f, 12.0f), sf::Vector2f(95.0f, 32.0f)), BTN_BACK),
      btnModeBT("BINARY TREE", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(115.0f, 30.0f)), BTN_SECONDARY),
      btnModeBST("BST", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(65.0f, 30.0f)), BTN_PRIMARY),
      btnModeAVL("AVL TREE", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(95.0f, 30.0f)), BTN_SECONDARY),
      btnModeHeap("MAX-HEAP", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(95.0f, 30.0f)), BTN_SECONDARY),
      btnRandom("RANDOM", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(80.0f, 30.0f)), BTN_SECONDARY),
      btnClear("CLEAR", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(70.0f, 30.0f)), BTN_DANGER),
      btnAutoBalance("AUTO-BALANCE: ON", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(150.0f, 30.0f)), BTN_PRIMARY),
      btnInsert("INSERT", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(75.0f, 32.0f)), BTN_PRIMARY),
      btnDelete("DELETE", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(75.0f, 32.0f)), BTN_SECONDARY),
      btnSearch("SEARCH", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(75.0f, 32.0f)), BTN_SECONDARY),
      btnInorder("INORDER", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(80.0f, 32.0f)), BTN_SECONDARY),
      btnPreorder("PREORDER", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(85.0f, 32.0f)), BTN_SECONDARY),
      btnPostorder("POSTORDER", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(90.0f, 32.0f)), BTN_SECONDARY),
      btnLevelOrder("LEVEL-ORDER", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(100.0f, 32.0f)), BTN_SECONDARY),
      btnRange("RANGE QUERY", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(110.0f, 32.0f)), BTN_SECONDARY),
      btnRotateLeft("ROTATE LEFT", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(105.0f, 32.0f)), BTN_SECONDARY),
      btnRotateRight("ROTATE RIGHT", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(110.0f, 32.0f)), BTN_SECONDARY),
      btnMirror("MIRROR", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(75.0f, 32.0f)), BTN_SECONDARY),
      btnExtractMax("EXTRACT MAX", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(110.0f, 32.0f)), BTN_PRIMARY),
      btnBuildHeap("BUILD HEAP", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(100.0f, 32.0f)), BTN_SECONDARY),
      btnHeapSort("HEAP SORT", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(95.0f, 32.0f)), BTN_SECONDARY),
      btnUndo("UNDO", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(65.0f, 28.0f)), BTN_SECONDARY),
      btnReplay("REPLAY", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(75.0f, 28.0f)), BTN_SECONDARY),
      btnReset("RESET", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(65.0f, 28.0f)), BTN_SECONDARY),
      btnSpeed05("0.5x", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(50.0f, 28.0f)), BTN_SECONDARY),
      btnSpeed10("1.0x", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(50.0f, 28.0f)), BTN_PRIMARY),
      btnSpeed15("1.5x", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(50.0f, 28.0f)), BTN_SECONDARY),
      btnSpeed20("2.0x", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(50.0f, 28.0f)), BTN_SECONDARY),
      btnClearLog("CLEAR LOG", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(95.0f, 24.0f)), BTN_SECONDARY)
{
    updateModeButtons();
    updateSpeedButtons();
    updateLayout(windowSize.x, windowSize.y);
    generateRandom();
    logEvent("Sandbox mode initialized.");
}

SandboxScreen::~SandboxScreen()
{
    if (m_heap)
    {
        delete m_heap;
        m_heap = nullptr;
    }
}

void SandboxScreen::updateModeButtons()
{
    btnModeBT.style = (m_mode == SB_BINARY_TREE) ? BTN_PRIMARY : BTN_SECONDARY;
    btnModeBST.style = (m_mode == SB_BST) ? BTN_PRIMARY : BTN_SECONDARY;
    btnModeAVL.style = (m_mode == SB_AVL) ? BTN_PRIMARY : BTN_SECONDARY;
    btnModeHeap.style = (m_mode == SB_HEAP) ? BTN_PRIMARY : BTN_SECONDARY;
    btnAutoBalance.style = m_autoBalance ? BTN_PRIMARY : BTN_SECONDARY;
    btnAutoBalance.label = m_autoBalance ? "AUTO-BALANCE: ON" : "AUTO-BALANCE: OFF";
}

void SandboxScreen::updateSpeedButtons()
{
    float cur = AnimationController::getSpeed();
    btnSpeed05.style = (std::abs(cur - 0.5f) < 0.1f) ? BTN_PRIMARY : BTN_SECONDARY;
    btnSpeed10.style = (std::abs(cur - 1.0f) < 0.1f) ? BTN_PRIMARY : BTN_SECONDARY;
    btnSpeed15.style = (std::abs(cur - 1.5f) < 0.1f) ? BTN_PRIMARY : BTN_SECONDARY;
    btnSpeed20.style = (std::abs(cur - 2.0f) < 0.1f) ? BTN_PRIMARY : BTN_SECONDARY;
}

void SandboxScreen::updateLayout(float w, float h)
{
    windowSize = sf::Vector2f(w, h);
    btnBack.bounds = sf::FloatRect(sf::Vector2f(20.0f, 12.0f), sf::Vector2f(95.0f, 32.0f));

    // Top Header Switcher (Centered row)
    std::vector<Button*> switchers = { &btnModeBT, &btnModeBST, &btnModeAVL, &btnModeHeap, &btnRandom, &btnClear };
    if (m_mode == SB_AVL) switchers.push_back(&btnAutoBalance);

    float swTotalW = 0.0f;
    float swSpacing = 10.0f;
    for (auto* b : switchers) swTotalW += b->bounds.size.x;
    swTotalW += static_cast<float>(switchers.size() - 1) * swSpacing;
    float swStartX = std::round((w - swTotalW) * 0.5f);
    float swCurX = swStartX;
    for (auto* b : switchers)
    {
        b->bounds.position = sf::Vector2f(swCurX, 14.0f);
        swCurX += b->bounds.size.x + swSpacing;
    }

    // Side Event Log Panel (Right side)
    float panelW = 280.0f;
    float panelX = w - panelW - 16.0f;
    float panelY = 56.0f;
    float panelH = h - panelY - 76.0f;
    btnClearLog.bounds = sf::FloatRect(sf::Vector2f(panelX + panelW - 100.0f, panelY + 8.0f), sf::Vector2f(90.0f, 24.0f));

    // Canvas region (Left to panel)
    m_canvasBounds = sf::FloatRect(sf::Vector2f(16.0f, 56.0f), sf::Vector2f(panelX - 32.0f, h - 56.0f - 110.0f));

    // Operations Button Rows (Centered horizontally under canvas)
    std::vector<Button*> opRow1;
    std::vector<Button*> opRow2;

    opRow1.push_back(&btnInsert);
    opRow1.push_back(&btnDelete);
    opRow1.push_back(&btnSearch);
    opRow1.push_back(&btnInorder);
    opRow1.push_back(&btnPreorder);
    opRow1.push_back(&btnPostorder);
    opRow1.push_back(&btnLevelOrder);

    if (m_mode == SB_BST || m_mode == SB_AVL)
    {
        opRow2.push_back(&btnRange);
    }
    if (m_mode == SB_AVL)
    {
        opRow2.push_back(&btnRotateLeft);
        opRow2.push_back(&btnRotateRight);
    }
    else if (m_mode == SB_BINARY_TREE)
    {
        opRow2.push_back(&btnMirror);
    }
    else if (m_mode == SB_HEAP)
    {
        opRow2.push_back(&btnExtractMax);
        opRow2.push_back(&btnBuildHeap);
        opRow2.push_back(&btnHeapSort);
    }

    // Centered Row 1
    float r1TotalW = 0.0f;
    float r1Spacing = 8.0f;
    for (auto* b : opRow1) r1TotalW += b->bounds.size.x;
    r1TotalW += static_cast<float>(opRow1.size() - 1) * r1Spacing;
    float r1StartX = std::round((m_canvasBounds.size.x - r1TotalW) * 0.5f) + m_canvasBounds.position.x;
    float r1Y = m_canvasBounds.position.y + m_canvasBounds.size.y + 12.0f;
    float curX = r1StartX;
    for (auto* b : opRow1)
    {
        b->bounds.position = sf::Vector2f(curX, r1Y);
        curX += b->bounds.size.x + r1Spacing;
    }

    // Centered Row 2
    if (!opRow2.empty())
    {
        float r2TotalW = 0.0f;
        for (auto* b : opRow2) r2TotalW += b->bounds.size.x;
        r2TotalW += static_cast<float>(opRow2.size() - 1) * r1Spacing;
        float r2StartX = std::round((m_canvasBounds.size.x - r2TotalW) * 0.5f) + m_canvasBounds.position.x;
        float r2Y = r1Y + 38.0f;
        curX = r2StartX;
        for (auto* b : opRow2)
        {
            b->bounds.position = sf::Vector2f(curX, r2Y);
            curX += b->bounds.size.x + r1Spacing;
        }
    }

    // Toolbar (Centered at bottom)
    std::vector<Button*> toolbar = { &btnUndo, &btnReplay, &btnReset, &btnSpeed05, &btnSpeed10, &btnSpeed15, &btnSpeed20 };
    float tbTotalW = 0.0f;
    float tbSpacing = 8.0f;
    for (auto* b : toolbar) tbTotalW += b->bounds.size.x;
    tbTotalW += static_cast<float>(toolbar.size() - 1) * tbSpacing;
    float tbStartX = std::round((w - tbTotalW) * 0.5f);
    float tbY = h - 34.0f;
    curX = tbStartX;
    for (auto* b : toolbar)
    {
        b->bounds.position = sf::Vector2f(curX, tbY);
        curX += b->bounds.size.x + tbSpacing;
    }

    if (m_root)
    {
        m_treeRenderer.computeLayout(m_root, m_canvasBounds);
    }
}

void SandboxScreen::onResize(unsigned int width, unsigned int height)
{
    updateLayout(static_cast<float>(width), static_cast<float>(height));
}

void SandboxScreen::logEvent(const std::string& desc)
{
    std::ostringstream oss;
    oss << "#" << (m_eventLogs.size() + 1) << " " << desc;
    m_eventLogs.push_back(oss.str());
    if (m_eventLogs.size() > 50)
    {
        m_eventLogs.erase(m_eventLogs.begin());
    }
}

void SandboxScreen::saveHistory()
{
    std::vector<int> current;
    collectKeys(m_root, current);
    if (m_mode == SB_HEAP)
    {
        current = m_heapValues;
    }
    m_historyKeys.push_back(current);
    if (m_historyKeys.size() > 20)
    {
        m_historyKeys.erase(m_historyKeys.begin());
    }
}

void SandboxScreen::collectKeys(TreeNode* node, std::vector<int>& keys)
{
    if (!node) return;
    collectKeys(node->left, keys);
    keys.push_back(node->val);
    collectKeys(node->right, keys);
}

TreeNode* SandboxScreen::findNodeByValue(TreeNode* node, int val)
{
    if (!node) return nullptr;
    if (node->val == val) return node;
    TreeNode* leftRes = findNodeByValue(node->left, val);
    if (leftRes) return leftRes;
    return findNodeByValue(node->right, val);
}

sf::Vector2f SandboxScreen::screenToWorld(sf::Vector2i pixelPos) const
{
    float cx = m_canvasBounds.position.x + m_canvasBounds.size.x * 0.5f;
    float cy = m_canvasBounds.position.y + m_canvasBounds.size.y * 0.5f;
    float wx = (static_cast<float>(pixelPos.x) - cx - m_panOffset.x) / m_zoomLevel + cx;
    float wy = (static_cast<float>(pixelPos.y) - cy - m_panOffset.y) / m_zoomLevel + cy;
    return sf::Vector2f(wx, wy);
}

static TreeNode* buildHeapSubtree(const std::vector<int>& arr, int index, int count)
{
    if (index >= count) return nullptr;
    TreeNode* node = new TreeNode(arr[index]);
    node->left = buildHeapSubtree(arr, 2 * index + 1, count);
    node->right = buildHeapSubtree(arr, 2 * index + 2, count);
    return node;
}

void SandboxScreen::syncHeapTree()
{
    if (m_mode != SB_HEAP) return;
    m_root = buildHeapSubtree(m_heapValues, 0, m_heapActiveSize);
    m_treeRenderer.computeLayout(m_root, m_canvasBounds);
}

void SandboxScreen::rebuildTreeFromKeys(const std::vector<int>& keys)
{
    m_root = nullptr;
    m_selectedNode = nullptr;

    if (m_mode == SB_BST)
    {
        m_bst.destroyTree();
        for (int k : keys) m_bst.insert(k);
        m_root = m_bst.getRoot();
    }
    else if (m_mode == SB_AVL)
    {
        m_avl.destroyTree();
        for (int k : keys)
        {
            m_avl.insert(k);
            if (m_autoBalance) m_avl.updateHeights();
        }
        m_root = m_avl.getRoot();
    }
    else if (m_mode == SB_BINARY_TREE)
    {
        m_bt.destroyTree();
        for (int k : keys) m_bt.insert(k);
        m_root = m_bt.getRoot();
    }
    else if (m_mode == SB_HEAP)
    {
        m_heapValues = keys;
        m_heapActiveSize = static_cast<int>(keys.size());
        if (m_heap) delete m_heap;
        m_heap = new MaxHeap(m_heapValues.data(), m_heapActiveSize);
        syncHeapTree();
        return;
    }

    m_treeRenderer.computeLayout(m_root, m_canvasBounds);
}

void SandboxScreen::generateRandom()
{
    saveHistory();
    m_currentSeed = static_cast<unsigned int>(std::time(nullptr)) % 90000 + 10000;
    std::mt19937 rng(m_currentSeed);
    std::uniform_int_distribution<int> countDist(7, 12);
    std::uniform_int_distribution<int> valDist(10, 99);

    int numNodes = countDist(rng);
    std::vector<int> distinctKeys;
    while (static_cast<int>(distinctKeys.size()) < numNodes)
    {
        int v = valDist(rng);
        if (std::find(distinctKeys.begin(), distinctKeys.end(), v) == distinctKeys.end())
        {
            distinctKeys.push_back(v);
        }
    }

    rebuildTreeFromKeys(distinctKeys);
    logEvent("Generated random structure with " + std::to_string(numNodes) + " nodes (Seed: " + std::to_string(m_currentSeed) + ").");
}

void SandboxScreen::executeInsert()
{
    int val = 0;
    try { val = std::stoi(m_inputValue); } catch (...) { return; }

    saveHistory();
    if (m_mode == SB_BST)
    {
        if (m_bst.insert(val))
        {
            m_root = m_bst.getRoot();
            m_treeRenderer.computeLayout(m_root, m_canvasBounds);
            logEvent("BST insert " + std::to_string(val));
            engine.audio.playClick();
        }
        else
        {
            engine.showToast("Duplicate rejected.", TOAST_ERROR);
            engine.audio.playWrong();
        }
    }
    else if (m_mode == SB_AVL)
    {
        if (m_avl.insert(val))
        {
            m_root = m_avl.getRoot();
            if (m_autoBalance) m_avl.updateHeights();
            m_treeRenderer.computeLayout(m_root, m_canvasBounds);
            logEvent("AVL insert " + std::to_string(val));
            engine.audio.playClick();
        }
        else
        {
            engine.showToast("Duplicate rejected.", TOAST_ERROR);
            engine.audio.playWrong();
        }
    }
    else if (m_mode == SB_BINARY_TREE)
    {
        if (m_selectedNode)
        {
            if (!m_selectedNode->left)
            {
                m_selectedNode->left = new TreeNode(val);
                logEvent("BT insert " + std::to_string(val) + " as left child of " + std::to_string(m_selectedNode->val));
                engine.audio.playClick();
            }
            else if (!m_selectedNode->right)
            {
                m_selectedNode->right = new TreeNode(val);
                logEvent("BT insert " + std::to_string(val) + " as right child of " + std::to_string(m_selectedNode->val));
                engine.audio.playClick();
            }
            else
            {
                engine.showToast("Selected node already has 2 children.", TOAST_INFO);
                return;
            }
        }
        else
        {
            m_bt.insert(val);
            m_root = m_bt.getRoot();
            logEvent("BT insert " + std::to_string(val));
            engine.audio.playClick();
        }
        m_treeRenderer.computeLayout(m_root, m_canvasBounds);
    }
    else if (m_mode == SB_HEAP)
    {
        m_heapValues.push_back(val);
        m_heapActiveSize = static_cast<int>(m_heapValues.size());
        if (m_heap) delete m_heap;
        m_heap = new MaxHeap(m_heapValues.data(), m_heapActiveSize);
        m_heap->siftUp(m_heapActiveSize - 1);
        for (int i = 0; i < m_heapActiveSize; ++i) m_heapValues[i] = m_heap->getAt(i);
        syncHeapTree();
        logEvent("Heap insert " + std::to_string(val));
        engine.audio.playClick();
    }
}

void SandboxScreen::executeDelete()
{
    int val = 0;
    try { val = std::stoi(m_inputValue); } catch (...) { return; }

    saveHistory();
    if (m_mode == SB_BST)
    {
        if (m_bst.remove(val))
        {
            m_root = m_bst.getRoot();
            m_treeRenderer.computeLayout(m_root, m_canvasBounds);
            logEvent("BST delete " + std::to_string(val));
            engine.audio.playClick();
        }
        else
        {
            engine.showToast("Key not found.", TOAST_ERROR);
            engine.audio.playWrong();
        }
    }
    else if (m_mode == SB_AVL)
    {
        if (m_avl.remove(val))
        {
            m_root = m_avl.getRoot();
            if (m_autoBalance) m_avl.updateHeights();
            m_treeRenderer.computeLayout(m_root, m_canvasBounds);
            logEvent("AVL delete " + std::to_string(val));
            engine.audio.playClick();
        }
        else
        {
            engine.showToast("Key not found.", TOAST_ERROR);
            engine.audio.playWrong();
        }
    }
    else if (m_mode == SB_BINARY_TREE)
    {
        if (m_selectedNode)
        {
            int sVal = m_selectedNode->val;
            // Delete selected subtree
            std::vector<int> remaining;
            std::vector<int> all;
            collectKeys(m_root, all);
            for (int k : all) if (k != sVal) remaining.push_back(k);
            rebuildTreeFromKeys(remaining);
            m_selectedNode = nullptr;
            logEvent("BT delete node " + std::to_string(sVal));
            engine.audio.playClick();
        }
        else
        {
            engine.showToast("Select a node to delete.", TOAST_INFO);
        }
    }
}

void SandboxScreen::executeSearch()
{
    int val = 0;
    try { val = std::stoi(m_inputValue); } catch (...) { return; }

    TreeNode* found = findNodeByValue(m_root, val);
    if (found)
    {
        m_selectedNode = found;
        m_treeRenderer.setSelectedNode(found);
        engine.showToast("Found key " + std::to_string(val), TOAST_CORRECT);
        engine.audio.playWin();
        logEvent("Search key " + std::to_string(val) + ": Found");
    }
    else
    {
        engine.showToast("Key not found.", TOAST_ERROR);
        engine.audio.playWrong();
        logEvent("Search key " + std::to_string(val) + ": Not Found");
    }
}

void SandboxScreen::executeTraversal(int type)
{
    m_animSequence.clear();
    std::string name = "";
    if (type == 0) // Inorder
    {
        name = "Inorder";
        std::function<void(TreeNode*)> inOrder = [&](TreeNode* n) {
            if (!n) return;
            inOrder(n->left);
            m_animSequence.push_back(n->val);
            inOrder(n->right);
        };
        inOrder(m_root);
    }
    else if (type == 1) // Preorder
    {
        name = "Preorder";
        std::function<void(TreeNode*)> preOrder = [&](TreeNode* n) {
            if (!n) return;
            m_animSequence.push_back(n->val);
            preOrder(n->left);
            preOrder(n->right);
        };
        preOrder(m_root);
    }
    else if (type == 2) // Postorder
    {
        name = "Postorder";
        std::function<void(TreeNode*)> postOrder = [&](TreeNode* n) {
            if (!n) return;
            postOrder(n->left);
            postOrder(n->right);
            m_animSequence.push_back(n->val);
        };
        postOrder(m_root);
    }
    else if (type == 3) // Level-Order
    {
        name = "Level-Order";
        std::vector<TreeNode*> q;
        if (m_root) q.push_back(m_root);
        while (!q.empty())
        {
            TreeNode* cur = q.front();
            q.erase(q.begin());
            m_animSequence.push_back(cur->val);
            if (cur->left) q.push_back(cur->left);
            if (cur->right) q.push_back(cur->right);
        }
    }

    if (m_animSequence.empty())
    {
        engine.showToast("Tree is empty.", TOAST_INFO);
        return;
    }

    m_animCurrentIndex = 0;
    m_animStepTimer = 0.5f;

    std::ostringstream oss;
    oss << name << ": ";
    for (size_t i = 0; i < m_animSequence.size(); ++i)
    {
        oss << m_animSequence[i] << (i + 1 < m_animSequence.size() ? ", " : "");
    }
    m_lastResultText = oss.str();
    logEvent(m_lastResultText);
    engine.showToast(m_lastResultText, TOAST_INFO);
    engine.audio.playClick();
}

void SandboxScreen::executeRange()
{
    int lo = 0, hi = 100;
    try { lo = std::stoi(m_inputLow); } catch (...) {}
    try { hi = std::stoi(m_inputHigh); } catch (...) {}

    m_inRangeKeys.clear();
    m_prunedKeys.clear();

    std::function<void(TreeNode*)> checkRange = [&](TreeNode* n) {
        if (!n) return;
        if (n->val >= lo && n->val <= hi)
        {
            m_inRangeKeys.push_back(n->val);
        }
        else
        {
            m_prunedKeys.push_back(n->val);
        }
        checkRange(n->left);
        checkRange(n->right);
    };
    checkRange(m_root);

    std::ostringstream oss;
    oss << "Range " << lo << " to " << hi << ": " << m_inRangeKeys.size() << " nodes in range";
    m_lastResultText = oss.str();
    logEvent(m_lastResultText);
    engine.showToast(m_lastResultText, TOAST_INFO);
    engine.audio.playClick();
}

void SandboxScreen::executeRotate(bool left)
{
    if (!m_selectedNode)
    {
        engine.showToast("Select a node to rotate.", TOAST_INFO);
        return;
    }

    saveHistory();
    int sVal = m_selectedNode->val;
    if (left)
    {
        m_root = m_avl.rotateLeft(m_selectedNode);
        logEvent("Rotate Left on node " + std::to_string(sVal));
    }
    else
    {
        m_root = m_avl.rotateRight(m_selectedNode);
        logEvent("Rotate Right on node " + std::to_string(sVal));
    }
    m_avl.setRoot(m_root);
    m_avl.updateHeights();
    m_treeRenderer.computeLayout(m_root, m_canvasBounds);
    engine.audio.playClick();
}

void SandboxScreen::executeMirror()
{
    saveHistory();
    m_bt.Mirror();
    m_root = m_bt.getRoot();
    m_treeRenderer.computeLayout(m_root, m_canvasBounds);
    logEvent("Mirrored Binary Tree");
    engine.audio.playClick();
}

void SandboxScreen::executeExtractMax()
{
    if (!m_heap || m_heapActiveSize <= 0)
    {
        engine.showToast("Heap is empty.", TOAST_INFO);
        return;
    }

    saveHistory();
    int maxVal = m_heap->extractMax();
    m_heapActiveSize = m_heap->getHeapSize();
    for (int i = 0; i < static_cast<int>(m_heapValues.size()); ++i)
    {
        m_heapValues[i] = m_heap->getAt(i);
    }
    syncHeapTree();
    logEvent("Extracted Max: " + std::to_string(maxVal));
    engine.showToast("Extracted Max: " + std::to_string(maxVal), TOAST_CORRECT);
    engine.audio.playClick();
}

void SandboxScreen::executeBuildHeap()
{
    if (!m_heap || m_heapValues.empty()) return;

    saveHistory();
    m_heap->buildMaxHeap();
    m_heapActiveSize = m_heap->getHeapSize();
    for (int i = 0; i < static_cast<int>(m_heapValues.size()); ++i)
    {
        m_heapValues[i] = m_heap->getAt(i);
    }
    syncHeapTree();
    logEvent("Built Max-Heap");
    engine.showToast("Max-Heap built successfully.", TOAST_INFO);
    engine.audio.playClick();
}

void SandboxScreen::executeHeapSort()
{
    if (!m_heap || m_heapValues.empty()) return;

    saveHistory();
    m_heap->heapSort();
    m_heapActiveSize = m_heap->getHeapSize(); // 0 when fully sorted
    for (int i = 0; i < static_cast<int>(m_heapValues.size()); ++i)
    {
        m_heapValues[i] = m_heap->getAt(i);
    }
    syncHeapTree();
    logEvent("Executed Heap Sort: Array sorted ascending");
    engine.showToast("Heap Sort complete. Array sorted.", TOAST_CORRECT);
    engine.audio.playWin();
}

void SandboxScreen::handleEvent(const sf::Event& event)
{
    btnBack.handleEvent(event);
    btnModeBT.handleEvent(event);
    btnModeBST.handleEvent(event);
    btnModeAVL.handleEvent(event);
    btnModeHeap.handleEvent(event);
    btnRandom.handleEvent(event);
    btnClear.handleEvent(event);
    if (m_mode == SB_AVL) btnAutoBalance.handleEvent(event);

    btnInsert.handleEvent(event);
    btnDelete.handleEvent(event);
    btnSearch.handleEvent(event);
    btnInorder.handleEvent(event);
    btnPreorder.handleEvent(event);
    btnPostorder.handleEvent(event);
    btnLevelOrder.handleEvent(event);

    if (m_mode == SB_BST || m_mode == SB_AVL) btnRange.handleEvent(event);
    if (m_mode == SB_AVL) { btnRotateLeft.handleEvent(event); btnRotateRight.handleEvent(event); }
    if (m_mode == SB_BINARY_TREE) btnMirror.handleEvent(event);
    if (m_mode == SB_HEAP) { btnExtractMax.handleEvent(event); btnBuildHeap.handleEvent(event); btnHeapSort.handleEvent(event); }

    btnUndo.handleEvent(event);
    btnReplay.handleEvent(event);
    btnReset.handleEvent(event);
    btnSpeed05.handleEvent(event);
    btnSpeed10.handleEvent(event);
    btnSpeed15.handleEvent(event);
    btnSpeed20.handleEvent(event);
    btnClearLog.handleEvent(event);

    if (btnBack.isClicked())
    {
        btnBack.resetClick();
        engine.audio.playClick();
        engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
        return;
    }

    if (btnClearLog.isClicked())
    {
        btnClearLog.resetClick();
        m_eventLogs.clear();
        return;
    }

    // Structure Switchers
    auto switchMode = [this](SandboxMode newMode) {
        if (m_mode == newMode) return;
        m_mode = newMode;
        m_root = nullptr;
        m_selectedNode = nullptr;
        m_hoveredNode = nullptr;
        m_animSequence.clear();
        m_inRangeKeys.clear();
        m_prunedKeys.clear();
        m_historyKeys.clear();
        updateModeButtons();
        updateLayout(windowSize.x, windowSize.y);
        generateRandom();
        engine.audio.playClick();
    };

    if (btnModeBT.isClicked()) { btnModeBT.resetClick(); switchMode(SB_BINARY_TREE); return; }
    if (btnModeBST.isClicked()) { btnModeBST.resetClick(); switchMode(SB_BST); return; }
    if (btnModeAVL.isClicked()) { btnModeAVL.resetClick(); switchMode(SB_AVL); return; }
    if (btnModeHeap.isClicked()) { btnModeHeap.resetClick(); switchMode(SB_HEAP); return; }

    if (btnAutoBalance.isClicked())
    {
        btnAutoBalance.resetClick();
        m_autoBalance = !m_autoBalance;
        btnAutoBalance.label = m_autoBalance ? "AUTO-BALANCE: ON" : "AUTO-BALANCE: OFF";
        btnAutoBalance.style = m_autoBalance ? BTN_PRIMARY : BTN_SECONDARY;
        engine.audio.playClick();
        return;
    }

    if (btnRandom.isClicked())
    {
        btnRandom.resetClick();
        generateRandom();
        engine.audio.playClick();
        return;
    }

    if (btnClear.isClicked())
    {
        btnClear.resetClick();
        saveHistory();
        m_root = nullptr;
        m_selectedNode = nullptr;
        m_heapValues.clear();
        m_heapActiveSize = 0;
        if (m_heap) { delete m_heap; m_heap = nullptr; }
        logEvent("Cleared structure");
        engine.audio.playClick();
        return;
    }

    if (btnInsert.isClicked()) { btnInsert.resetClick(); executeInsert(); return; }
    if (btnDelete.isClicked()) { btnDelete.resetClick(); executeDelete(); return; }
    if (btnSearch.isClicked()) { btnSearch.resetClick(); executeSearch(); return; }
    if (btnInorder.isClicked()) { btnInorder.resetClick(); executeTraversal(0); return; }
    if (btnPreorder.isClicked()) { btnPreorder.resetClick(); executeTraversal(1); return; }
    if (btnPostorder.isClicked()) { btnPostorder.resetClick(); executeTraversal(2); return; }
    if (btnLevelOrder.isClicked()) { btnLevelOrder.resetClick(); executeTraversal(3); return; }

    if (btnRange.isClicked()) { btnRange.resetClick(); executeRange(); return; }
    if (btnRotateLeft.isClicked()) { btnRotateLeft.resetClick(); executeRotate(true); return; }
    if (btnRotateRight.isClicked()) { btnRotateRight.resetClick(); executeRotate(false); return; }
    if (btnMirror.isClicked()) { btnMirror.resetClick(); executeMirror(); return; }
    if (btnExtractMax.isClicked()) { btnExtractMax.resetClick(); executeExtractMax(); return; }
    if (btnBuildHeap.isClicked()) { btnBuildHeap.resetClick(); executeBuildHeap(); return; }
    if (btnHeapSort.isClicked()) { btnHeapSort.resetClick(); executeHeapSort(); return; }

    if (btnUndo.isClicked())
    {
        btnUndo.resetClick();
        if (!m_historyKeys.empty())
        {
            auto last = m_historyKeys.back();
            m_historyKeys.pop_back();
            rebuildTreeFromKeys(last);
            logEvent("Undid last operation");
            engine.audio.playClick();
        }
        else
        {
            engine.showToast("No moves made yet.", TOAST_INFO);
        }
        return;
    }

    if (btnReplay.isClicked())
    {
        btnReplay.resetClick();
        if (!m_animSequence.empty())
        {
            m_animCurrentIndex = 0;
            m_animStepTimer = 0.5f;
            logEvent("Replaying animation");
            engine.audio.playClick();
        }
        else
        {
            executeTraversal(0);
        }
        return;
    }

    if (btnReset.isClicked())
    {
        btnReset.resetClick();
        saveHistory();
        m_root = nullptr;
        m_selectedNode = nullptr;
        m_heapValues.clear();
        m_heapActiveSize = 0;
        if (m_heap) { delete m_heap; m_heap = nullptr; }
        logEvent("Reset structure to empty");
        engine.audio.playClick();
        return;
    }

    auto handleSpeed = [this](float spd) {
        AnimationController::setSpeed(spd);
        updateSpeedButtons();
        engine.audio.playClick();
    };
    if (btnSpeed05.isClicked()) { btnSpeed05.resetClick(); handleSpeed(0.5f); return; }
    if (btnSpeed10.isClicked()) { btnSpeed10.resetClick(); handleSpeed(1.0f); return; }
    if (btnSpeed15.isClicked()) { btnSpeed15.resetClick(); handleSpeed(1.5f); return; }
    if (btnSpeed20.isClicked()) { btnSpeed20.resetClick(); handleSpeed(2.0f); return; }

    // TextEntered for input boxes
    if (const auto* te = event.getIf<sf::Event::TextEntered>())
    {
        if (m_activeInputBox >= 0)
        {
            std::string* targetStr = (m_activeInputBox == 0) ? &m_inputValue : ((m_activeInputBox == 1) ? &m_inputLow : &m_inputHigh);
            uint32_t uc = te->unicode;
            if (uc == 8) // Backspace
            {
                if (!targetStr->empty()) targetStr->pop_back();
            }
            else if (uc == 13) // Enter
            {
                if (m_activeInputBox == 0) executeInsert();
                else executeRange();
            }
            else if ((uc >= '0' && uc <= '9') || (uc == '-' && targetStr->empty()))
            {
                if (targetStr->length() < 5)
                {
                    targetStr->push_back(static_cast<char>(uc));
                }
            }
        }
    }

    // Mouse Press / Pan / Input Box focus
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mb->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mpos(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));

            // Check input box click
            float inY = m_canvasBounds.position.y + m_canvasBounds.size.y + 14.0f;
            float valBoxX = 20.0f;
            sf::FloatRect boxValRect(sf::Vector2f(valBoxX + 45.0f, inY), sf::Vector2f(55.0f, 26.0f));
            sf::FloatRect boxLowRect(sf::Vector2f(valBoxX + 140.0f, inY), sf::Vector2f(50.0f, 26.0f));
            sf::FloatRect boxHiRect(sf::Vector2f(valBoxX + 225.0f, inY), sf::Vector2f(50.0f, 26.0f));

            if (boxValRect.contains(mpos)) m_activeInputBox = 0;
            else if (boxLowRect.contains(mpos)) m_activeInputBox = 1;
            else if (boxHiRect.contains(mpos)) m_activeInputBox = 2;
            else m_activeInputBox = -1;

            if (m_canvasBounds.contains(mpos))
            {
                sf::Vector2f wPos = screenToWorld(mb->position);
                TreeNode* clicked = m_treeRenderer.getNodeAtPosition(m_root, wPos);
                if (clicked)
                {
                    m_selectedNode = clicked;
                    m_treeRenderer.setSelectedNode(clicked);
                    m_inputValue = std::to_string(clicked->val);
                    engine.audio.playClick();
                }
                else
                {
                    m_isPanning = true;
                    m_panStartPixel = mb->position;
                    m_panStartOffset = m_panOffset;
                }
            }
        }
        else if (mb->button == sf::Mouse::Button::Right)
        {
            m_isPanning = true;
            m_panStartPixel = mb->position;
            m_panStartOffset = m_panOffset;
        }
    }

    if (const auto* mr = event.getIf<sf::Event::MouseButtonReleased>())
    {
        m_isPanning = false;
    }

    if (const auto* mm = event.getIf<sf::Event::MouseMoved>())
    {
        if (m_isPanning)
        {
            m_panOffset = m_panStartOffset + sf::Vector2f(static_cast<float>(mm->position.x - m_panStartPixel.x), static_cast<float>(mm->position.y - m_panStartPixel.y));
        }
        else
        {
            sf::Vector2f wPos = screenToWorld(mm->position);
            m_hoveredNode = m_treeRenderer.getNodeAtPosition(m_root, wPos);
            m_treeRenderer.setHoveredNode(m_hoveredNode);
        }
    }

    if (const auto* wh = event.getIf<sf::Event::MouseWheelScrolled>())
    {
        if (m_canvasBounds.contains(sf::Vector2f(static_cast<float>(wh->position.x), static_cast<float>(wh->position.y))))
        {
            float delta = wh->delta;
            float factor = (delta > 0.0f) ? 1.15f : 0.85f;
            m_zoomLevel = std::clamp(m_zoomLevel * factor, 0.4f, 3.0f);
        }
    }

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        if (kp->code == sf::Keyboard::Key::Escape)
        {
            engine.audio.playClick();
            engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
            return;
        }
    }
}

void SandboxScreen::update(float dt)
{
    float scaledDt = AnimationController::apply(dt);
    m_treeRenderer.update(scaledDt);

    m_cursorTimer += dt;
    if (m_cursorTimer >= 0.5f)
    {
        m_cursorTimer = 0.0f;
        m_cursorVisible = !m_cursorVisible;
    }

    // Traversal Animation Step
    if (m_animCurrentIndex >= 0 && m_animCurrentIndex < static_cast<int>(m_animSequence.size()))
    {
        m_animStepTimer -= scaledDt;
        if (m_animStepTimer <= 0.0f)
        {
            int curVal = m_animSequence[m_animCurrentIndex];
            TreeNode* cur = findNodeByValue(m_root, curVal);
            if (cur)
            {
                m_selectedNode = cur;
                m_treeRenderer.setSelectedNode(cur);
            }
            m_animCurrentIndex++;
            m_animStepTimer = 0.4f;
        }
    }
    else if (m_animCurrentIndex >= static_cast<int>(m_animSequence.size()))
    {
        m_animCurrentIndex = -1;
    }
}

void SandboxScreen::draw(sf::RenderTarget& target)
{
    btnBack.draw(target, engine.theme);

    // Header switcher buttons
    btnModeBT.draw(target, engine.theme);
    btnModeBST.draw(target, engine.theme);
    btnModeAVL.draw(target, engine.theme);
    btnModeHeap.draw(target, engine.theme);
    btnRandom.draw(target, engine.theme);
    btnClear.draw(target, engine.theme);
    if (m_mode == SB_AVL) btnAutoBalance.draw(target, engine.theme);

    // Current Structure Seed Display
    std::string seedStr = "SEED: " + std::to_string(m_currentSeed);
    sf::Text tSeed(engine.theme.fontMono, seedStr, 13);
    tSeed.setFillColor(Theme::TextMuted);
    tSeed.setPosition(sf::Vector2f(windowSize.x - 420.0f, 20.0f));
    target.draw(tSeed);

    // Canvas Frame
    engine.theme.drawPanel(target, m_canvasBounds, Theme::BorderCyan, 1.2f);
    engine.theme.drawGlowOutline(target, m_canvasBounds, Theme::BorderCyan, 0.5f);

    // Render Tree inside canvas viewport
    sf::View originalView = target.getView();
    sf::View canvasView;
    canvasView.setViewport(sf::FloatRect(
        sf::Vector2f(m_canvasBounds.position.x / windowSize.x, m_canvasBounds.position.y / windowSize.y),
        sf::Vector2f(m_canvasBounds.size.x / windowSize.x, m_canvasBounds.size.y / windowSize.y)
    ));
    float cx = m_canvasBounds.position.x + m_canvasBounds.size.x * 0.5f;
    float cy = m_canvasBounds.position.y + m_canvasBounds.size.y * 0.5f;
    canvasView.setCenter(sf::Vector2f(cx - m_panOffset.x, cy - m_panOffset.y));
    canvasView.setSize(sf::Vector2f(m_canvasBounds.size.x / m_zoomLevel, m_canvasBounds.size.y / m_zoomLevel));

    target.setView(canvasView);
    if (m_root)
    {
        m_treeRenderer.draw(target, engine.theme, m_root);

        // Highlight in-range / pruned nodes in range query
        if (!m_inRangeKeys.empty())
        {
            for (int k : m_inRangeKeys)
            {
                TreeNode* n = findNodeByValue(m_root, k);
                if (n)
                {
                    sf::CircleShape glow(22.0f);
                    glow.setOrigin(sf::Vector2f(22.0f, 22.0f));
                    glow.setPosition(sf::Vector2f(n->position.x, n->position.y));
                    glow.setFillColor(sf::Color::Transparent);
                    glow.setOutlineThickness(2.5f);
                    glow.setOutlineColor(Theme::AccentYellow);
                    target.draw(glow);
                }
            }
        }

        // Draw AVL Balance badges if in AVL mode
        if (m_mode == SB_AVL)
        {
            std::function<void(TreeNode*)> drawBf = [&](TreeNode* n) {
                if (!n) return;
                int bf = m_avl.getBalance(n);
                sf::Text bfText(engine.theme.fontMono, std::to_string(bf), 10);
                bfText.setFillColor(std::abs(bf) > 1 ? Theme::CrimsonAlarm : Theme::SuccessEmerald);
                bfText.setStyle(sf::Text::Bold);
                bfText.setPosition(sf::Vector2f(n->position.x + 14.0f, n->position.y - 18.0f));
                target.draw(bfText);
                drawBf(n->left);
                drawBf(n->right);
            };
            drawBf(m_root);
        }
    }
    target.setView(originalView);

    // Max-Heap Synchronized Array Strip
    if (m_mode == SB_HEAP && !m_heapValues.empty())
    {
        float stripX = m_canvasBounds.position.x + 20.0f;
        float stripY = m_canvasBounds.position.y + m_canvasBounds.size.y - 75.0f;
        sf::Text stripHdr(engine.theme.fontMono, "SYNCHRONIZED ARRAY STRIP (ACTIVE HEAP vs SORTED)", 11);
        stripHdr.setFillColor(Theme::AccentYellow);
        stripHdr.setPosition(sf::Vector2f(stripX, stripY - 18.0f));
        target.draw(stripHdr);

        float cellW = 38.0f;
        float cellH = 34.0f;
        for (size_t i = 0; i < m_heapValues.size() && i < 18; ++i)
        {
            bool isActiveHeap = (static_cast<int>(i) < m_heapActiveSize);
            sf::RectangleShape cell(sf::Vector2f(cellW, cellH));
            cell.setPosition(sf::Vector2f(stripX + static_cast<float>(i) * (cellW + 4.0f), stripY));
            cell.setFillColor(isActiveHeap ? sf::Color(17, 30, 48, 230) : sf::Color(38, 20, 40, 230));
            cell.setOutlineThickness(1.5f);
            cell.setOutlineColor(isActiveHeap ? Theme::BorderCyan : sf::Color(180, 80, 220));
            target.draw(cell);

            // Index
            sf::Text idxText(engine.theme.fontMono, std::to_string(i), 9);
            idxText.setFillColor(Theme::TextMuted);
            idxText.setPosition(sf::Vector2f(stripX + static_cast<float>(i) * (cellW + 4.0f) + 4.0f, stripY + 2.0f));
            target.draw(idxText);

            // Value
            sf::Text valText(engine.theme.fontMono, std::to_string(m_heapValues[i]), 12);
            valText.setFillColor(isActiveHeap ? Theme::TextPrimary : Theme::AccentYellow);
            valText.setStyle(sf::Text::Bold);
            valText.setPosition(sf::Vector2f(stripX + static_cast<float>(i) * (cellW + 4.0f) + 8.0f, stripY + 14.0f));
            target.draw(valText);
        }
    }

    // Input Boxes on left of operations rows
    float inY = m_canvasBounds.position.y + m_canvasBounds.size.y + 14.0f;
    float valBoxX = 20.0f;

    auto drawInput = [&](const std::string& label, const std::string& text, float x, float w, bool focused) {
        sf::Text lbl(engine.theme.fontMono, label, 11);
        lbl.setFillColor(Theme::TextMuted);
        lbl.setPosition(sf::Vector2f(x, inY + 5.0f));
        target.draw(lbl);

        float bx = x + lbl.getLocalBounds().size.x + 8.0f;
        sf::RectangleShape box(sf::Vector2f(w, 26.0f));
        box.setPosition(sf::Vector2f(bx, inY + 2.0f));
        box.setFillColor(sf::Color(10, 14, 23, 220));
        box.setOutlineThickness(1.0f);
        box.setOutlineColor(focused ? Theme::BorderCyan : Theme::BorderMuted);
        target.draw(box);

        std::string disp = text + (focused && m_cursorVisible ? "_" : "");
        sf::Text txt(engine.theme.fontMono, disp, 12);
        txt.setFillColor(focused ? Theme::AccentYellow : Theme::TextPrimary);
        txt.setPosition(sf::Vector2f(bx + 6.0f, inY + 6.0f));
        target.draw(txt);
    };

    drawInput("VAL:", m_inputValue, valBoxX, 48.0f, m_activeInputBox == 0);
    if (m_mode == SB_BST || m_mode == SB_AVL)
    {
        drawInput("LO:", m_inputLow, valBoxX + 95.0f, 44.0f, m_activeInputBox == 1);
        drawInput("HI:", m_inputHigh, valBoxX + 180.0f, 44.0f, m_activeInputBox == 2);
    }

    // Operation Buttons
    btnInsert.draw(target, engine.theme);
    btnDelete.draw(target, engine.theme);
    btnSearch.draw(target, engine.theme);
    btnInorder.draw(target, engine.theme);
    btnPreorder.draw(target, engine.theme);
    btnPostorder.draw(target, engine.theme);
    btnLevelOrder.draw(target, engine.theme);

    if (m_mode == SB_BST || m_mode == SB_AVL) btnRange.draw(target, engine.theme);
    if (m_mode == SB_AVL) { btnRotateLeft.draw(target, engine.theme); btnRotateRight.draw(target, engine.theme); }
    if (m_mode == SB_BINARY_TREE) btnMirror.draw(target, engine.theme);
    if (m_mode == SB_HEAP) { btnExtractMax.draw(target, engine.theme); btnBuildHeap.draw(target, engine.theme); btnHeapSort.draw(target, engine.theme); }

    // Bottom Toolbar
    btnUndo.draw(target, engine.theme);
    btnReplay.draw(target, engine.theme);
    btnReset.draw(target, engine.theme);
    btnSpeed05.draw(target, engine.theme);
    btnSpeed10.draw(target, engine.theme);
    btnSpeed15.draw(target, engine.theme);
    btnSpeed20.draw(target, engine.theme);

    // Event Log Panel on Right
    float panelW = 280.0f;
    float panelX = windowSize.x - panelW - 16.0f;
    float panelY = 56.0f;
    float panelH = windowSize.y - panelY - 76.0f;
    sf::FloatRect logPanelBounds(sf::Vector2f(panelX, panelY), sf::Vector2f(panelW, panelH));

    engine.theme.drawPanel(target, logPanelBounds, Theme::BorderMuted, 1.0f);
    sf::Text logTitle(engine.theme.fontUI, "OPERATION LOG", 13);
    logTitle.setFillColor(Theme::BorderCyan);
    logTitle.setStyle(sf::Text::Bold);
    logTitle.setPosition(sf::Vector2f(panelX + 12.0f, panelY + 10.0f));
    target.draw(logTitle);

    btnClearLog.draw(target, engine.theme);

    // Log items
    float curLogY = panelY + 40.0f;
    size_t startIdx = (m_eventLogs.size() > 16) ? (m_eventLogs.size() - 16) : 0;
    for (size_t i = startIdx; i < m_eventLogs.size(); ++i)
    {
        sf::Text item(engine.theme.fontMono, m_eventLogs[i], 11);
        item.setFillColor(Theme::TextPrimary);
        item.setPosition(sf::Vector2f(panelX + 12.0f, curLogY));
        target.draw(item);
        curLogY += 20.0f;
    }

    // Hover Tooltip with Key, Height (leaf=0), Balance Factor
    if (m_hoveredNode && m_canvasBounds.contains(sf::Vector2f(static_cast<float>(sf::Mouse::getPosition(engine.window).x), static_cast<float>(sf::Mouse::getPosition(engine.window).y))))
    {
        TreeRenderer::drawNodeTooltip(target, engine.theme, m_root, m_hoveredNode);
    }
}
