#pragma once
#include "Screen.h"
#include "UIKit.h"
#include "TreeRenderer.h"
#include "TreeNode.h"
#include "BinaryTree.h"
#include "BST.h"
#include "AVL.h"
#include "MaxHeap.h"
#include "AnimationController.h"
#include <vector>
#include <string>
#include <random>

class GameEngine;

struct EndlessConfig
{
    int alarmCapacity = 12;
    int alarmRecoveryPerWave = 2;
    int baseWaveBonus = 500;
    int bonusPerWaveMultiplier = 100;
    int minNodes = 5;
    int maxNodesCap = 12;

    static EndlessConfig load();
};

enum EndlessWaveFamily
{
    E_WAVE_BT   = 0,
    E_WAVE_BST  = 1,
    E_WAVE_AVL  = 2,
    E_WAVE_HEAP = 3
};

class EndlessScreen : public Screen
{
private:
    GameEngine& engine;
    sf::Vector2f windowSize;
    EndlessConfig m_config;

    unsigned int m_masterSeed;
    int m_waveNumber;
    int m_wavesCleared;
    int m_score;
    int m_currentAlarm;
    int m_wrongMoves;
    float m_waveElapsed;
    bool m_hintUsedThisWave;
    bool m_isGameOver;
    std::string m_leaderboardMessage;

    EndlessWaveFamily m_currentFamily;
    std::string m_missionText;
    int m_targetKey;
    int m_avlTargetPivot;
    int m_avlExpectedDirection; // 0=left, 1=right
    int m_heapSwapIdx1;
    int m_heapSwapIdx2;

    // Backend tree and visual node
    TreeNode* m_root;
    TreeNode* m_selectedNode;
    TreeNode* m_firstHeapNode;
    TreeNode* m_hoveredNode;
    TreeRenderer m_treeRenderer;

    AVL m_avlBackend;
    std::vector<int> m_heapValues;
    int m_heapActiveCount;

    // View & Camera
    sf::FloatRect m_canvasBounds;
    sf::Vector2f m_panOffset;
    bool m_isPanning;
    sf::Vector2i m_panStartPixel;
    sf::Vector2f m_panStartOffset;
    float m_zoomLevel;

    // Undo action snapshot
    TreeNode* m_prevSelectedNode;

    // HUD & Action buttons
    Button btnBack;
    Button btnHint;
    Button btnUndo;
    Button btnRotateLeft;
    Button btnRotateRight;
    Button btnExtractMax;

    // Game Over Dialog Buttons
    Button btnRetry;
    Button btnLeaderboard;
    Button btnGameOverBack;

    void updateLayout(float w, float h);
    void generateWave();
    void buildBTWave(int nodeCount, std::mt19937& rng);
    void buildBSTWave(int nodeCount, std::mt19937& rng);
    void buildAVLWave(int nodeCount, std::mt19937& rng);
    void buildHeapWave(int nodeCount, std::mt19937& rng);
    void syncHeapTree();

    void onNodeClicked(TreeNode* node);
    void checkWaveComplete();
    void onGameOver();
    TreeNode* findNodeByVal(TreeNode* node, int val);
    sf::Vector2f screenToWorld(sf::Vector2i pixelPos) const;

public:
    EndlessScreen(GameEngine& gameEngine);
    ~EndlessScreen();

    void handleEvent(const sf::Event& event) override;
    void update(float dt) override;
    void draw(sf::RenderTarget& target) override;
    void onResize(unsigned int width, unsigned int height) override;
};
