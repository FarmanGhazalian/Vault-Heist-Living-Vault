#pragma once
#include "gui/Screen.h"
#include "gui/UIKit.h"
#include "gui/TreeRenderer.h"
#include "backend/TreeNode.h"
#include "backend/BinaryTree.h"
#include "backend/BST.h"
#include "backend/AVL.h"
#include "backend/MaxHeap.h"
#include "gui/AnimationController.h"
#include <vector>
#include <string>

class GameEngine;

enum SandboxMode
{
    SB_BINARY_TREE = 0,
    SB_BST         = 1,
    SB_AVL         = 2,
    SB_HEAP        = 3
};

struct SandboxSnapshot
{
    SandboxMode mode;
    std::vector<int> values;
    // For BT, parent-child relations can be reconstructed or stored as tree copy
};

class SandboxScreen : public Screen
{
private:
    GameEngine& engine;
    sf::Vector2f windowSize;

    SandboxMode m_mode;
    unsigned int m_currentSeed;
    bool m_autoBalance;

    // Backend structures
    BinaryTree m_bt;
    BST m_bst;
    AVL m_avl;
    MaxHeap* m_heap;
    std::vector<int> m_heapValues;
    int m_heapActiveSize;

    // Visual tree representation
    TreeNode* m_root;
    TreeNode* m_selectedNode;
    TreeNode* m_hoveredNode;
    TreeRenderer m_treeRenderer;

    // Camera view & canvas
    sf::FloatRect m_canvasBounds;
    sf::Vector2f m_panOffset;
    bool m_isPanning;
    sf::Vector2i m_panStartPixel;
    sf::Vector2f m_panStartOffset;
    float m_zoomLevel;

    // Traversal / Search animation state
    std::vector<int> m_animSequence;
    int m_animCurrentIndex;
    float m_animStepTimer;
    std::string m_lastResultText;
    std::vector<int> m_inRangeKeys;
    std::vector<int> m_prunedKeys;

    // Input boxes: 0 = Value, 1 = Low, 2 = High
    int m_activeInputBox; // -1 = none
    std::string m_inputValue;
    std::string m_inputLow;
    std::string m_inputHigh;
    float m_cursorTimer;
    bool m_cursorVisible;

    // Event log
    std::vector<std::string> m_eventLogs;
    int m_logScrollOffset;

    // History for Undo
    std::vector<std::vector<int>> m_historyKeys;

    // GUI Buttons
    Button btnBack;
    Button btnModeBT;
    Button btnModeBST;
    Button btnModeAVL;
    Button btnModeHeap;

    Button btnRandom;
    Button btnClear;
    Button btnAutoBalance;

    // Operations buttons
    Button btnInsert;
    Button btnDelete;
    Button btnSearch;
    Button btnInorder;
    Button btnPreorder;
    Button btnPostorder;
    Button btnLevelOrder;
    Button btnRange;
    Button btnRotateLeft;
    Button btnRotateRight;
    Button btnMirror;
    Button btnExtractMax;
    Button btnBuildHeap;
    Button btnHeapSort;

    // Toolbar buttons
    Button btnUndo;
    Button btnReplay;
    Button btnReset;
    Button btnSpeed05;
    Button btnSpeed10;
    Button btnSpeed15;
    Button btnSpeed20;
    Button btnClearLog;

    void updateLayout(float w, float h);
    void rebuildTreeFromKeys(const std::vector<int>& keys);
    void syncHeapTree();
    void logEvent(const std::string& desc);
    void saveHistory();
    void updateModeButtons();
    void updateSpeedButtons();
    void executeInsert();
    void executeDelete();
    void executeSearch();
    void executeTraversal(int type); // 0=inorder, 1=preorder, 2=postorder, 3=levelorder
    void executeRange();
    void executeRotate(bool left);
    void executeMirror();
    void executeExtractMax();
    void executeBuildHeap();
    void executeHeapSort();
    void generateRandom();

    TreeNode* findNodeByValue(TreeNode* node, int val);
    void collectKeys(TreeNode* node, std::vector<int>& keys);
    sf::Vector2f screenToWorld(sf::Vector2i pixelPos) const;

public:
    SandboxScreen(GameEngine& gameEngine);
    ~SandboxScreen();

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void onResize(unsigned int width, unsigned int height) override;
};
