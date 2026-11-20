# GUI STATE & SYSTEM ARCHITECTURE

## 1. Status & Completed Stages
- **Stage 0**: Read-only inspection report complete.
- **Stage 1**: Backend optimal steps computed directly in C++ via tree/algorithm inspection; alarm capacity calculation unified (`ceil(1.5 * optimalSteps) + 2`); game-over rule on capacity check unified.
- **Stage 2**: Shared GUI features in `LevelScreenBase` (Pan/Zoom clamping, cursor-anchored zoom, deferred screen transitions to prevent crashes on win/lose result modal buttons, standardized HUD cards, single height display convention).
- **Stage 3**: Act 1 level-specific bug fixes completed.
- **Stage 4**: Act 2 bug fixes completed.
- **Stage 5**: Act 3 bug fixes completed.
- **Stage 6**: Act 4 GUI implementation completed.
- **Stage 7**: Full regression & verification complete across all 20 levels.
- **Part 1 — Shared Bug Fixes**:
  - **1. Hint**: Fixed root causes of +1 alarm penalty (backend `Act2Level1..5`, `Act3Level1..5`, `Act4Level1..5` previously called `onMistake(1, 50)` and GUI local alarm increments were desynchronized from backend `LevelState`). Unified in `LevelScreenBase::onHintClicked()`: first press marks `state.hintUsed = true` and shows level-specific hint; second and subsequent presses show `"Hint already used."`, apply exactly `+2` to backend `LevelState::alarm`, and trigger `checkGameOver()`.
  - **2. Undo & Event Routing**: Completely overhauled button hit-testing and event routing. Replaced multi-button polling with single hit-test per click (topmost visible button consumes click, sets `m_eventConsumed = true`, preventing event fallthrough to other buttons or canvas node clicks). Eliminated duplicate messages on Undo by decoupling internal level reload from `onResetClicked()`. Undo now resets silently from JSON, rewinds action history, restores tokens/positions, and emits only `"Last action undone."`. Replay runs stored actions animated without altering finished score. Reset restarts from JSON with alarm 0 and history cleared.
  - **3. Hover Tooltip Height**: Implemented shared conversion function `TreeRenderer::getTooltipHeight(const TreeNode* node)`. In tooltip only, leaf nodes display height 0 (`(node->height > 0) ? node->height - 1 : 0`). Stored AVL heights remain 1, and balance factor calculation remains `height(left) - height(right)` using stored values. Level 1.3 expected answers use `leaf = 0` convention (`calculateNodeHeight` returns 0 for leaves); tooltip height line is completely hidden in Level 1.3 so it never contradicts or reveals answers.
  - **4. Button Bar Layout**: Implemented shared `LevelScreenBase::layoutButtonBar(...)` helper. Centered bottom action buttons (`HINT`, `UNDO`, `REPLAY`, `RESET`) in one horizontally centered row directly under the canvas bounding box with equal dimensions and spacing.
  - **5. Fresh Start & Dev Unlock Toggle**: Removed disk file reading (`savegame.dat`) at startup. Every session starts fresh with only Level 1.1 unlocked, 0 stars, and no progress written to disk. Added session-wide `Ctrl+Shift+U` toggle which unlocks all levels (1.1–4.5) and toggles back to the fresh start state. Winning a level unlocks the next level for the active session only.
  - **6. Zoom & Pan**: Mouse wheel zoom is anchored to the mouse cursor using world coordinates delta shift. Dragging empty canvas background with left mouse (or middle/right/space+drag) pans freely in all directions without bounding clamping. Fit-to-view button resets zoom to 1.0 and recenters view.

## 2. Public Interface of Shared Classes

### `LevelScreenBase` (Derived from `Screen`)
- `void handleEvent(const sf::Event& event)`: Centralized event dispatcher with topmost hit-testing, cursor-anchored zoom, free background panning, and modal dialog consumption.
- `void update(float dt)`: Central animation timer, toast updates, action replay driver, and deferred screen transitions.
- `void draw(sf::RenderTarget& target)`: Canvas drawing with pan/zoom view, overlay HUD cards, side panel, centered action bar, and modal popups.
- `void onResize(unsigned int width, unsigned int height)`: Responsive repositioning of canvas, action bar, and right side telemetry panel.
- `void setAlarm(int currentAlarm, int alarmCapacity)`: Mutates segmented alarm meter.
- `void setScore(int score)`: Updates current score display.
- `void setSteps(int currentSteps, int optimalSteps)`: Updates step telemetry counters.
- `void setPenalty(int penaltyPoints)`: Updates penalty deduction tally.
- `void setInstruction(const std::string& text)`: Sets mission objective text.
- `void addToLog(const std::string& entry)`: Appends an event to the scrollable operational log.
- `void showResultModal(...)`: Displays standard end-of-level victory/defeat dialog with retry, replay, next level, and return to campaign handlers.
- `static void layoutButtonBar(...)`: Shared helper to center an array of buttons horizontally in a bounding container.

### Shared Virtual Hooks for Derived Level Screens
- `virtual ILevelLogic* getLevelLogic()`: Returns pointer to backend level logic instance for unified alarm/state synchronization.
- `virtual void resetLevel()`: Re-initializes level state from JSON without emitting user notifications (used silently by Undo and Replay).
- `virtual void syncWithState()`: Synchronizes HUD telemetry with backend `LevelState`.
- `virtual std::string getLevelSpecificHint()`: Returns contextual, level-specific hint text.
- `virtual void reapplyActionSilently(const PlayerAction& action)`: Silently reapplies a past action (for undo history rewind).
- `virtual void reapplyActionAnimated(const PlayerAction& action)`: Replays an action with audio/visual feedback.
- `virtual void onBackClicked()`
- `virtual void onFitClicked()`
- `virtual void onHintClicked()`
- `virtual void onResetClicked()`
- `virtual void onUndoClicked()`
- `virtual void onReplayClicked()`
- `virtual void onNodeClicked(TreeNode* node)`
- `virtual void onCustomDraw(sf::RenderTarget& target)`
- `virtual void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)`

### `TreeRenderer`
- `static int getTooltipHeight(const TreeNode* node)`: Shared height conversion (leaf node displays 0).
- `static void drawNodeTooltip(sf::RenderTarget& target, const Theme& theme, TreeNode* root, TreeNode* hoverNode, bool hideHeight = false)`: Displays `KEY`, `HEIGHT` (if not hidden), and `BALANCE` (`leftH - rightH`).

### `SaveSystem`
- `static void load()`: In-memory pristine session initialization (only Level 1.1 unlocked, 0 stars).
- `static void save()`: No-op; campaign progress is never written to disk.
- `static bool toggleUnlockAll()`: Toggles all levels unlocked <-> reset to fresh start.

## 3. Frozen Rules (Do Not Break)
1. **Never alter visual theme, color schemes, font styling, or layout look** of operational screens.
2. **Never execute synchronous screen deletions** inside UI callbacks or modal buttons; use deferred screen transitions (`m_pendingCallback`) via `LevelScreenBase`.
3. **No double-slashes (`//`) and no square brackets (`[]`)** in user-facing UI text, toasts, and telemetry messages.
4. **Backend logic remains completely decoupled** from GUI rendering. All game state evaluations, validations, and step costs are performed by backend logic.
5. **Button event exclusivity**: One click triggers exactly one handler. The topmost visible button consumes the click and blocks canvas/node fallthrough.
6. **Isolated feedback**: Each button sets only its own feedback message. Undo must never trigger Reset feedback toasts.
