#include "EndlessScreen.h"
#include "MainMenuScreen.h"
#include "InfoScreen.h"
#include "GameEngine.h"
#include "LeaderboardManager.h"
#include "nlohmann/json.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <cmath>

EndlessConfig EndlessConfig::load()
{
    EndlessConfig cfg;
    std::ifstream f("endless_config.json");
    if (f.is_open())
    {
        try
        {
            nlohmann::json j;
            f >> j;
            if (j.contains("alarm_capacity") && j["alarm_capacity"].is_number())
                cfg.alarmCapacity = j["alarm_capacity"].get<int>();
            if (j.contains("alarm_recovery_per_wave") && j["alarm_recovery_per_wave"].is_number())
                cfg.alarmRecoveryPerWave = j["alarm_recovery_per_wave"].get<int>();
            if (j.contains("base_wave_bonus") && j["base_wave_bonus"].is_number())
                cfg.baseWaveBonus = j["base_wave_bonus"].get<int>();
            if (j.contains("bonus_per_wave_multiplier") && j["bonus_per_wave_multiplier"].is_number())
                cfg.bonusPerWaveMultiplier = j["bonus_per_wave_multiplier"].get<int>();
            if (j.contains("min_nodes") && j["min_nodes"].is_number())
                cfg.minNodes = j["min_nodes"].get<int>();
            if (j.contains("max_nodes_cap") && j["max_nodes_cap"].is_number())
                cfg.maxNodesCap = j["max_nodes_cap"].get<int>();
        }
        catch (...)
        {
        }
        f.close();
    }
    return cfg;
}

EndlessScreen::EndlessScreen(GameEngine& gameEngine)
    : engine(gameEngine),
      windowSize(gameEngine.getWindowSize()),
      m_config(EndlessConfig::load()),
      m_masterSeed(static_cast<unsigned int>(std::time(nullptr))),
      m_waveNumber(1),
      m_wavesCleared(0),
      m_score(0),
      m_currentAlarm(0),
      m_wrongMoves(0),
      m_waveElapsed(0.0f),
      m_hintUsedThisWave(false),
      m_isGameOver(false),
      m_currentFamily(E_WAVE_BT),
      m_targetKey(0),
      m_avlTargetPivot(0),
      m_avlExpectedDirection(0),
      m_heapSwapIdx1(-1),
      m_heapSwapIdx2(-1),
      m_root(nullptr),
      m_selectedNode(nullptr),
      m_firstHeapNode(nullptr),
      m_hoveredNode(nullptr),
      m_heapActiveCount(0),
      m_panOffset(0.0f, 0.0f),
      m_isPanning(false),
      m_zoomLevel(1.0f),
      m_prevSelectedNode(nullptr),
      btnBack("< ABORT", sf::FloatRect(sf::Vector2f(20.0f, 12.0f), sf::Vector2f(95.0f, 32.0f)), BTN_BACK),
      btnHint("HINT", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(85.0f, 34.0f)), BTN_SECONDARY),
      btnUndo("UNDO", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(85.0f, 34.0f)), BTN_SECONDARY),
      btnRotateLeft("ROTATE LEFT", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(115.0f, 34.0f)), BTN_PRIMARY),
      btnRotateRight("ROTATE RIGHT", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(120.0f, 34.0f)), BTN_PRIMARY),
      btnExtractMax("EXTRACT MAX", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(120.0f, 34.0f)), BTN_PRIMARY),
      btnRetry("RETRY", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(120.0f, 38.0f)), BTN_PRIMARY),
      btnLeaderboard("LEADERBOARD", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(130.0f, 38.0f)), BTN_SECONDARY),
      btnGameOverBack("MAIN MENU", sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(120.0f, 38.0f)), BTN_BACK)
{
    updateLayout(windowSize.x, windowSize.y);
    generateWave();
}

EndlessScreen::~EndlessScreen()
{
}

void EndlessScreen::updateLayout(float w, float h)
{
    windowSize = sf::Vector2f(w, h);
    btnBack.bounds = sf::FloatRect(sf::Vector2f(20.0f, 12.0f), sf::Vector2f(95.0f, 32.0f));

    // Canvas region
    m_canvasBounds = sf::FloatRect(sf::Vector2f(20.0f, 85.0f), sf::Vector2f(w - 40.0f, h - 165.0f));

    // Centered Action Buttons under canvas
    std::vector<Button*> actions;
    actions.push_back(&btnHint);
    actions.push_back(&btnUndo);
    if (m_currentFamily == E_WAVE_AVL)
    {
        actions.push_back(&btnRotateLeft);
        actions.push_back(&btnRotateRight);
    }
    else if (m_currentFamily == E_WAVE_HEAP)
    {
        actions.push_back(&btnExtractMax);
    }

    float totalW = 0.0f;
    float spacing = 12.0f;
    for (auto* b : actions) totalW += b->bounds.size.x;
    totalW += static_cast<float>(actions.size() - 1) * spacing;
    float startX = std::round((w - totalW) * 0.5f);
    float btnY = h - 60.0f;
    float curX = startX;
    for (auto* b : actions)
    {
        b->bounds.position = sf::Vector2f(curX, btnY);
        curX += b->bounds.size.x + spacing;
    }

    if (m_root)
    {
        m_treeRenderer.computeLayout(m_root, m_canvasBounds);
    }
}

void EndlessScreen::onResize(unsigned int width, unsigned int height)
{
    updateLayout(static_cast<float>(width), static_cast<float>(height));
}

TreeNode* EndlessScreen::findNodeByVal(TreeNode* node, int val)
{
    if (!node) return nullptr;
    if (node->val == val) return node;
    TreeNode* l = findNodeByVal(node->left, val);
    if (l) return l;
    return findNodeByVal(node->right, val);
}

sf::Vector2f EndlessScreen::screenToWorld(sf::Vector2i pixelPos) const
{
    float cx = m_canvasBounds.position.x + m_canvasBounds.size.x * 0.5f;
    float cy = m_canvasBounds.position.y + m_canvasBounds.size.y * 0.5f;
    float wx = (static_cast<float>(pixelPos.x) - cx - m_panOffset.x) / m_zoomLevel + cx;
    float wy = (static_cast<float>(pixelPos.y) - cy - m_panOffset.y) / m_zoomLevel + cy;
    return sf::Vector2f(wx, wy);
}

static TreeNode* buildHeapSubtreeEndless(const std::vector<int>& arr, int index, int count)
{
    if (index >= count) return nullptr;
    TreeNode* node = new TreeNode(arr[index]);
    node->left = buildHeapSubtreeEndless(arr, 2 * index + 1, count);
    node->right = buildHeapSubtreeEndless(arr, 2 * index + 2, count);
    return node;
}

void EndlessScreen::syncHeapTree()
{
    m_root = buildHeapSubtreeEndless(m_heapValues, 0, m_heapActiveCount);
    m_treeRenderer.computeLayout(m_root, m_canvasBounds);
}

void EndlessScreen::generateWave()
{
    m_selectedNode = nullptr;
    m_firstHeapNode = nullptr;
    m_prevSelectedNode = nullptr;
    m_hintUsedThisWave = false;
    m_waveElapsed = 0.0f;

    unsigned int waveSeed = m_masterSeed + static_cast<unsigned int>(m_waveNumber * 37);
    std::mt19937 rng(waveSeed);

    m_currentFamily = static_cast<EndlessWaveFamily>((m_waveNumber - 1) % 4);
    int nodeCount = std::min(m_config.minNodes + (m_waveNumber - 1) / 2, m_config.maxNodesCap);

    if (m_currentFamily == E_WAVE_BT)
    {
        buildBTWave(nodeCount, rng);
    }
    else if (m_currentFamily == E_WAVE_BST)
    {
        buildBSTWave(nodeCount, rng);
    }
    else if (m_currentFamily == E_WAVE_AVL)
    {
        buildAVLWave(nodeCount, rng);
    }
    else if (m_currentFamily == E_WAVE_HEAP)
    {
        buildHeapWave(nodeCount, rng);
    }

    updateLayout(windowSize.x, windowSize.y);
}

void EndlessScreen::buildBTWave(int nodeCount, std::mt19937& rng)
{
    std::uniform_int_distribution<int> valDist(10, 99);
    std::vector<int> vals;
    while (static_cast<int>(vals.size()) < nodeCount)
    {
        int v = valDist(rng);
        if (std::find(vals.begin(), vals.end(), v) == vals.end()) vals.push_back(v);
    }

    BinaryTree bt;
    for (int v : vals) bt.insert(v);
    m_root = bt.getRoot();

    std::uniform_int_distribution<int> idxDist(0, nodeCount - 1);
    m_targetKey = vals[idxDist(rng)];
    m_missionText = "Infiltrate Binary Tree: Locate and select terminal node Key " + std::to_string(m_targetKey);
}

void EndlessScreen::buildBSTWave(int nodeCount, std::mt19937& rng)
{
    std::uniform_int_distribution<int> valDist(10, 99);
    std::vector<int> vals;
    while (static_cast<int>(vals.size()) < nodeCount)
    {
        int v = valDist(rng);
        if (std::find(vals.begin(), vals.end(), v) == vals.end()) vals.push_back(v);
    }

    BST bst;
    for (int v : vals) bst.insert(v);
    m_root = bst.getRoot();

    std::uniform_int_distribution<int> idxDist(0, nodeCount - 1);
    m_targetKey = vals[idxDist(rng)];
    m_missionText = "Infiltrate BST: Verify binary search property to extract Key " + std::to_string(m_targetKey);
}

void EndlessScreen::buildAVLWave(int nodeCount, std::mt19937& rng)
{
    // Generate AVL with deliberate imbalance that can be fixed with 1 rotation
    std::uniform_int_distribution<int> valDist(10, 99);
    std::vector<int> vals;
    while (static_cast<int>(vals.size()) < nodeCount)
    {
        int v = valDist(rng);
        if (std::find(vals.begin(), vals.end(), v) == vals.end()) vals.push_back(v);
    }
    std::sort(vals.begin(), vals.end());

    m_avlBackend.destroyTree();
    for (int v : vals)
    {
        m_avlBackend.insert(v);
    }
    m_root = m_avlBackend.getRoot();

    // Pick pivot node and determine direction
    m_avlTargetPivot = vals[vals.size() / 2];
    m_avlExpectedDirection = (rng() % 2 == 0) ? 0 : 1; // 0=left, 1=right

    std::string dirStr = (m_avlExpectedDirection == 0) ? "Left" : "Right";
    m_missionText = "Rebalance AVL: Apply " + dirStr + " Rotation on Pivot Node " + std::to_string(m_avlTargetPivot);
}

void EndlessScreen::buildHeapWave(int nodeCount, std::mt19937& rng)
{
    std::uniform_int_distribution<int> valDist(10, 99);
    m_heapValues.clear();
    while (static_cast<int>(m_heapValues.size()) < nodeCount)
    {
        int v = valDist(rng);
        if (std::find(m_heapValues.begin(), m_heapValues.end(), v) == m_heapValues.end()) m_heapValues.push_back(v);
    }
    m_heapActiveCount = static_cast<int>(m_heapValues.size());

    // Build valid max heap first
    MaxHeap hp(m_heapValues.data(), m_heapActiveCount);
    hp.buildMaxHeap();
    for (int i = 0; i < m_heapActiveCount; ++i) m_heapValues[i] = hp.getAt(i);

    // Introduce one solvable swap violation between parent 0 and child 1 or 2
    int childIdx = (m_heapActiveCount > 2 && (rng() % 2 == 1)) ? 2 : 1;
    m_heapSwapIdx1 = 0;
    m_heapSwapIdx2 = childIdx;
    std::swap(m_heapValues[m_heapSwapIdx1], m_heapValues[m_heapSwapIdx2]);

    syncHeapTree();
    m_missionText = "Restore Max-Heap: Select and swap parent and child to fix heap order";
}

void EndlessScreen::onNodeClicked(TreeNode* node)
{
    if (!node || m_isGameOver) return;
    m_prevSelectedNode = m_selectedNode;
    m_selectedNode = node;
    m_treeRenderer.setSelectedNode(node);

    if (m_currentFamily == E_WAVE_BT || m_currentFamily == E_WAVE_BST)
    {
        if (node->val == m_targetKey)
        {
            checkWaveComplete();
        }
        else
        {
            m_currentAlarm += 1;
            m_wrongMoves++;
            engine.audio.playWrong();
            engine.showToast("Wrong node in path. +1 Alarm", TOAST_ERROR);
            if (m_currentAlarm >= m_config.alarmCapacity) onGameOver();
        }
    }
    else if (m_currentFamily == E_WAVE_HEAP)
    {
        if (!m_firstHeapNode)
        {
            m_firstHeapNode = node;
            engine.audio.playClick();
        }
        else
        {
            if (m_firstHeapNode != node)
            {
                // Perform swap attempt
                int v1 = m_firstHeapNode->val;
                int v2 = node->val;
                int idx1 = -1, idx2 = -1;
                for (int i = 0; i < m_heapActiveCount; ++i)
                {
                    if (m_heapValues[i] == v1) idx1 = i;
                    if (m_heapValues[i] == v2) idx2 = i;
                }

                if ((idx1 == m_heapSwapIdx1 && idx2 == m_heapSwapIdx2) ||
                    (idx1 == m_heapSwapIdx2 && idx2 == m_heapSwapIdx1))
                {
                    std::swap(m_heapValues[idx1], m_heapValues[idx2]);
                    syncHeapTree();
                    checkWaveComplete();
                }
                else
                {
                    m_currentAlarm += 1;
                    m_wrongMoves++;
                    engine.audio.playWrong();
                    engine.showToast("Invalid heap swap. +1 Alarm", TOAST_ERROR);
                    if (m_currentAlarm >= m_config.alarmCapacity) onGameOver();
                }
            }
            m_firstHeapNode = nullptr;
        }
    }
}

void EndlessScreen::checkWaveComplete()
{
    int waveBonus = m_config.baseWaveBonus + (m_waveNumber * m_config.bonusPerWaveMultiplier);
    if (m_hintUsedThisWave) waveBonus = std::max(200, waveBonus / 2);

    m_score += waveBonus;
    m_currentAlarm = std::max(0, m_currentAlarm - m_config.alarmRecoveryPerWave);
    m_wavesCleared++;
    m_waveNumber++;

    engine.showToast("Wave cleared! Security threshold reduced.", TOAST_CORRECT);
    engine.audio.playWin();

    generateWave();
}

void EndlessScreen::onGameOver()
{
    m_isGameOver = true;
    engine.audio.playLose();

    int rank = LeaderboardManager::submitScore(GameEngine::getPlayerName(), "Endless", "Wave " + std::to_string(m_wavesCleared), m_score);
    if (rank > 0)
    {
        m_leaderboardMessage = "New high score, rank " + std::to_string(rank);
    }
    else
    {
        m_leaderboardMessage = "Score did not reach the top 10";
    }
}

void EndlessScreen::handleEvent(const sf::Event& event)
{
    if (m_isGameOver)
    {
        btnRetry.handleEvent(event);
        btnLeaderboard.handleEvent(event);
        btnGameOverBack.handleEvent(event);

        if (btnRetry.isClicked())
        {
            btnRetry.resetClick();
            m_waveNumber = 1;
            m_wavesCleared = 0;
            m_score = 0;
            m_currentAlarm = 0;
            m_wrongMoves = 0;
            m_isGameOver = false;
            m_masterSeed = static_cast<unsigned int>(std::time(nullptr));
            generateWave();
            engine.audio.playClick();
            return;
        }
        if (btnLeaderboard.isClicked())
        {
            btnLeaderboard.resetClick();
            engine.audio.playClick();
            engine.changeScreen(std::make_unique<InfoScreen>(engine, MODE_LEADERBOARD));
            return;
        }
        if (btnGameOverBack.isClicked())
        {
            btnGameOverBack.resetClick();
            engine.audio.playClick();
            engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
            return;
        }
        return;
    }

    btnBack.handleEvent(event);
    btnHint.handleEvent(event);
    btnUndo.handleEvent(event);

    if (m_currentFamily == E_WAVE_AVL)
    {
        btnRotateLeft.handleEvent(event);
        btnRotateRight.handleEvent(event);
    }
    else if (m_currentFamily == E_WAVE_HEAP)
    {
        btnExtractMax.handleEvent(event);
    }

    if (btnBack.isClicked())
    {
        btnBack.resetClick();
        engine.audio.playClick();
        engine.showConfirmModal("ABORT ENDLESS RUN",
            "Are you sure you want to abort the current endless run?\nYour score will be logged in archives.",
            [this]() {
                LeaderboardManager::submitScore(GameEngine::getPlayerName(), "Endless", "Wave " + std::to_string(m_wavesCleared), m_score);
                engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
            },
            nullptr);
        return;
    }

    if (btnHint.isClicked())
    {
        btnHint.resetClick();
        if (!m_hintUsedThisWave)
        {
            m_hintUsedThisWave = true;
            std::string hint = "";
            if (m_currentFamily == E_WAVE_BT || m_currentFamily == E_WAVE_BST)
                hint = "Hint: Target key " + std::to_string(m_targetKey) + " is inside the active quadrant.";
            else if (m_currentFamily == E_WAVE_AVL)
                hint = "Hint: Pivot " + std::to_string(m_avlTargetPivot) + " requires " + ((m_avlExpectedDirection == 0) ? "Left" : "Right") + " rotation.";
            else if (m_currentFamily == E_WAVE_HEAP)
                hint = "Hint: Swap values " + std::to_string(m_heapValues[m_heapSwapIdx1]) + " and " + std::to_string(m_heapValues[m_heapSwapIdx2]);

            engine.showToast(hint, TOAST_HINT);
            engine.audio.playHint();
        }
        else
        {
            m_currentAlarm += 2;
            m_wrongMoves++;
            engine.showToast("Hint already used.", TOAST_ERROR);
            engine.audio.playWrong();
            if (m_currentAlarm >= m_config.alarmCapacity) onGameOver();
        }
        return;
    }

    if (btnUndo.isClicked())
    {
        btnUndo.resetClick();
        if (m_firstHeapNode)
        {
            m_firstHeapNode = nullptr;
            engine.showToast("Selection cleared.", TOAST_INFO);
        }
        else if (m_selectedNode)
        {
            m_selectedNode = m_prevSelectedNode;
            m_treeRenderer.setSelectedNode(m_selectedNode);
            engine.showToast("Move undone.", TOAST_INFO);
        }
        else
        {
            engine.showToast("No moves made yet.", TOAST_INFO);
        }
        engine.audio.playClick();
        return;
    }

    if (m_currentFamily == E_WAVE_AVL)
    {
        auto handleRotation = [this](int dir) {
            if (!m_selectedNode)
            {
                engine.showToast("Select a node to rotate.", TOAST_INFO);
                return;
            }
            if (m_selectedNode->val == m_avlTargetPivot && dir == m_avlExpectedDirection)
            {
                checkWaveComplete();
            }
            else
            {
                m_currentAlarm += 2;
                m_score = std::max(0, m_score - 50);
                m_wrongMoves++;
                engine.audio.playWrong();
                engine.showToast("Wrong rotation on pivot. +2 Alarm, -50 pts", TOAST_ERROR);
                if (m_currentAlarm >= m_config.alarmCapacity) onGameOver();
            }
        };

        if (btnRotateLeft.isClicked()) { btnRotateLeft.resetClick(); handleRotation(0); return; }
        if (btnRotateRight.isClicked()) { btnRotateRight.resetClick(); handleRotation(1); return; }
    }
    else if (m_currentFamily == E_WAVE_HEAP)
    {
        if (btnExtractMax.isClicked())
        {
            btnExtractMax.resetClick();
            if (m_heapActiveCount > 0)
            {
                checkWaveComplete();
            }
            return;
        }
    }

    // Node click
    if (const auto* mb = event.getIf<sf::Event::MouseButtonPressed>())
    {
        if (mb->button == sf::Mouse::Button::Left)
        {
            sf::Vector2f mpos(static_cast<float>(mb->position.x), static_cast<float>(mb->position.y));
            if (m_canvasBounds.contains(mpos))
            {
                sf::Vector2f wPos = screenToWorld(mb->position);
                TreeNode* clicked = m_treeRenderer.getNodeAtPosition(m_root, wPos);
                if (clicked)
                {
                    onNodeClicked(clicked);
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

    if (const auto* kp = event.getIf<sf::Event::KeyPressed>())
    {
        if (kp->code == sf::Keyboard::Key::Escape)
        {
            engine.audio.playClick();
            engine.showConfirmModal("ABORT ENDLESS RUN",
                "Are you sure you want to abort the current endless run?\nYour score will be logged in archives.",
                [this]() {
                    LeaderboardManager::submitScore(GameEngine::getPlayerName(), "Endless", "Wave " + std::to_string(m_wavesCleared), m_score);
                    engine.changeScreen(std::make_unique<MainMenuScreen>(engine));
                },
                nullptr);
            return;
        }
    }
}

void EndlessScreen::update(float dt)
{
    float scaledDt = AnimationController::apply(dt);
    m_waveElapsed += dt;
    m_treeRenderer.update(scaledDt);
}

void EndlessScreen::draw(sf::RenderTarget& target)
{
    btnBack.draw(target, engine.theme);

    float cx = windowSize.x * 0.5f;

    // HUD Header Top Bar
    std::string waveStr = "WAVE " + std::to_string(m_waveNumber);
    sf::Text tWave(engine.theme.fontUI, waveStr, 22);
    tWave.setFillColor(Theme::AccentYellow);
    tWave.setStyle(sf::Text::Bold);
    tWave.setPosition(sf::Vector2f(130.0f, 16.0f));
    target.draw(tWave);

    std::string scoreStr = "SCORE: " + std::to_string(m_score);
    sf::Text tScore(engine.theme.fontMono, scoreStr, 16);
    tScore.setFillColor(Theme::TextPrimary);
    sf::FloatRect scb = tScore.getLocalBounds();
    tScore.setPosition(sf::Vector2f(std::round(cx - scb.size.x * 0.5f - scb.position.x), 18.0f));
    target.draw(tScore);

    // Alarm Meter
    std::string alarmStr = "ALARM: " + std::to_string(m_currentAlarm) + " / " + std::to_string(m_config.alarmCapacity);
    sf::Text tAlarm(engine.theme.fontMono, alarmStr, 15);
    tAlarm.setFillColor(m_currentAlarm >= (m_config.alarmCapacity - 2) ? Theme::CrimsonAlarm : Theme::BorderCyan);
    tAlarm.setPosition(sf::Vector2f(windowSize.x - 240.0f, 18.0f));
    target.draw(tAlarm);

    // Mission Objective Subtitle
    sf::Text tMission(engine.theme.fontUI, m_missionText, 14);
    tMission.setFillColor(Theme::BorderCyan);
    sf::FloatRect mb = tMission.getLocalBounds();
    tMission.setPosition(sf::Vector2f(std::round(cx - mb.size.x * 0.5f - mb.position.x), 52.0f));
    target.draw(tMission);

    // Canvas Frame
    engine.theme.drawPanel(target, m_canvasBounds, Theme::BorderCyan, 1.2f);
    engine.theme.drawGlowOutline(target, m_canvasBounds, Theme::BorderCyan, 0.5f);

    // Render tree inside canvas viewport
    sf::View originalView = target.getView();
    sf::View canvasView;
    canvasView.setViewport(sf::FloatRect(
        sf::Vector2f(m_canvasBounds.position.x / windowSize.x, m_canvasBounds.position.y / windowSize.y),
        sf::Vector2f(m_canvasBounds.size.x / windowSize.x, m_canvasBounds.size.y / windowSize.y)
    ));
    float canvasCenterX = m_canvasBounds.position.x + m_canvasBounds.size.x * 0.5f;
    float canvasCenterY = m_canvasBounds.position.y + m_canvasBounds.size.y * 0.5f;
    canvasView.setCenter(sf::Vector2f(canvasCenterX - m_panOffset.x, canvasCenterY - m_panOffset.y));
    canvasView.setSize(sf::Vector2f(m_canvasBounds.size.x / m_zoomLevel, m_canvasBounds.size.y / m_zoomLevel));

    target.setView(canvasView);
    if (m_root)
    {
        m_treeRenderer.draw(target, engine.theme, m_root);
    }
    target.setView(originalView);

    // Action Buttons
    btnHint.draw(target, engine.theme);
    btnUndo.draw(target, engine.theme);
    if (m_currentFamily == E_WAVE_AVL)
    {
        btnRotateLeft.draw(target, engine.theme);
        btnRotateRight.draw(target, engine.theme);
    }
    else if (m_currentFamily == E_WAVE_HEAP)
    {
        btnExtractMax.draw(target, engine.theme);
    }

    // Hover Tooltip
    if (m_hoveredNode && m_canvasBounds.contains(sf::Vector2f(static_cast<float>(sf::Mouse::getPosition(engine.window).x), static_cast<float>(sf::Mouse::getPosition(engine.window).y))))
    {
        TreeRenderer::drawNodeTooltip(target, engine.theme, m_root, m_hoveredNode);
    }

    // Game Over Modal Dialog
    if (m_isGameOver)
    {
        // Dark backdrop
        sf::RectangleShape backdrop(windowSize);
        backdrop.setFillColor(sf::Color(10, 14, 23, 215));
        target.draw(backdrop);

        float mw = 480.0f;
        float mh = 350.0f;
        float mx = std::round((windowSize.x - mw) * 0.5f);
        float my = std::round((windowSize.y - mh) * 0.5f);
        sf::FloatRect modalBounds(sf::Vector2f(mx, my), sf::Vector2f(mw, mh));

        engine.theme.drawPanel(target, modalBounds, Theme::CrimsonAlarm, 1.5f);
        engine.theme.drawGlowOutline(target, modalBounds, Theme::CrimsonAlarm, 1.0f);

        // Header Title
        sf::Text tOver(engine.theme.fontUI, "ENDLESS PROTOCOL ABORTED", 22);
        tOver.setFillColor(Theme::CrimsonAlarm);
        tOver.setStyle(sf::Text::Bold);
        sf::FloatRect tob = tOver.getLocalBounds();
        tOver.setPosition(sf::Vector2f(std::round(mx + (mw - tob.size.x) * 0.5f - tob.position.x), my + 20.0f));
        target.draw(tOver);

        sf::Text tSub(engine.theme.fontUI, "Alarm critical threshold breached. System locked.", 13);
        tSub.setFillColor(Theme::TextMuted);
        sf::FloatRect subb = tSub.getLocalBounds();
        tSub.setPosition(sf::Vector2f(std::round(mx + (mw - subb.size.x) * 0.5f - subb.position.x), my + 50.0f));
        target.draw(tSub);

        // Stats summary box
        float sbW = mw - 50.0f;
        float sbH = 95.0f;
        float sbX = mx + 25.0f;
        float sbY = my + 85.0f;

        sf::RectangleShape sBox(sf::Vector2f(sbW, sbH));
        sBox.setPosition(sf::Vector2f(sbX, sbY));
        sBox.setFillColor(sf::Color(13, 19, 33, 230));
        sBox.setOutlineThickness(1.0f);
        sBox.setOutlineColor(Theme::BorderMuted);
        target.draw(sBox);

        std::string col1 = "WAVES CLEARED: " + std::to_string(m_wavesCleared) + "\n" +
                           "WRONG MOVES:   " + std::to_string(m_wrongMoves);
        sf::Text sc1(engine.theme.fontMono, col1, 13);
        sc1.setFillColor(Theme::TextPrimary);
        sc1.setPosition(sf::Vector2f(sbX + 20.0f, sbY + 22.0f));
        target.draw(sc1);

        std::string col2 = "FINAL SCORE: " + std::to_string(m_score) + " pts\n" +
                           "RUN SEED:    " + std::to_string(m_masterSeed);
        sf::Text sc2(engine.theme.fontMono, col2, 13);
        sc2.setFillColor(Theme::AccentYellow);
        sc2.setStyle(sf::Text::Bold);
        sc2.setPosition(sf::Vector2f(sbX + sbW * 0.48f, sbY + 22.0f));
        target.draw(sc2);

        // Leaderboard notification
        if (!m_leaderboardMessage.empty())
        {
            sf::Text tLb(engine.theme.fontMono, m_leaderboardMessage, 13);
            bool isHigh = (m_leaderboardMessage.find("New high score") != std::string::npos);
            tLb.setFillColor(isHigh ? Theme::AccentYellow : Theme::TextMuted);
            tLb.setStyle(isHigh ? sf::Text::Bold : sf::Text::Regular);
            sf::FloatRect lbb = tLb.getLocalBounds();
            tLb.setPosition(sf::Vector2f(std::round(mx + (mw - lbb.size.x) * 0.5f - lbb.position.x), my + 200.0f));
            target.draw(tLb);
        }

        // Action Buttons (Centered row)
        float totalBW = 120.0f + 130.0f + 120.0f + 24.0f;
        float startBX = mx + (mw - totalBW) * 0.5f;
        float gBtnY = my + mh - 55.0f;

        btnRetry.bounds = sf::FloatRect(sf::Vector2f(startBX, gBtnY), sf::Vector2f(120.0f, 38.0f));
        btnLeaderboard.bounds = sf::FloatRect(sf::Vector2f(startBX + 132.0f, gBtnY), sf::Vector2f(130.0f, 38.0f));
        btnGameOverBack.bounds = sf::FloatRect(sf::Vector2f(startBX + 274.0f, gBtnY), sf::Vector2f(120.0f, 38.0f));

        btnRetry.draw(target, engine.theme);
        btnLeaderboard.draw(target, engine.theme);
        btnGameOverBack.draw(target, engine.theme);
    }
}
