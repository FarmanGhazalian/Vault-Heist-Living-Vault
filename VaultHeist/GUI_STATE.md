# GUI STATE & SYSTEM ARCHITECTURE

## 1. Status & Completed Stages
- **Stage 0**: Read-only inspection report complete.
- **Stage 1**: Backend optimal steps computed directly in C++ via tree/algorithm inspection; alarm capacity calculation unified (`ceil(1.5 * optimalSteps) + 2`); game-over rule on capacity check unified.
- **Stage 2**: Shared GUI features in `LevelScreenBase` (Pan/Zoom clamping, cursor-anchored zoom, deferred screen transitions to prevent crashes on win/lose result modal buttons, standardized HUD cards, single height display convention).
- **Stage 3**: Act 1 level-specific bug fixes completed:
  - **Level 1.2**: Implemented full action history undo support (`recordAction` and `onUndoClicked`), returning dragged and placed tokens cleanly to home tray slots on undo without corrupting placement slots.
  - **Level 1.3**: Consolidated keyboard input handling exclusively to `sf::Event::TextEntered` for numbers, eliminating duplicate digit entry from `sf::Event::KeyPressed`.
  - **Level 1.4**: Verified zoom and free camera panning compliance with `LevelScreenBase`.
  - **Level 1.5**: Verified bottom-up recursive mirror logic and UI side-panel parameters.
- **Stage 4**: Act 2 bug fixes completed:
  - Verified alarm capacity enforcement across all Act 2 levels (2.1 to 2.5).
  - Verified result-button flow (Next Level, Retry, Replay, Back to Campaign) on both victory and defeat dialogs.
  - Fixed path highlight persistence in Level 2.5: past corridor search paths and `isVisited` node flags are cleared upon reaching a target, on Reset, and on Undo.
- **Stage 5**: Act 3 bug fixes completed:
  - Verified button hitboxes and isolation in Level 3.2 (LL, RR, LR, RL buttons only trigger their respective action code and reset click state).
  - Verified Level 3.4 prediction tree state: null pointers prevented, AVL reconstruction per candidate key validated.
  - Verified Hint, Undo, Replay, and Reset actions across all Act 3 screens (3.1 to 3.5).
  - Confirmed modal result transitions use non-blocking deferred screen switches.
- **Stage 6**: Act 4 GUI implementation completed:
  - Created `Act4LevelScreen.h` and `Act4LevelScreen.cpp` supporting all Heap Fortress levels (4.1 to 4.5).
  - Rendered complete binary tree representation of the MaxHeap dynamically with array mapping telemetries in the side panel.
  - Interactive element pair selection for `SWAP_ELEMENTS` (parent-child heapify/sift repairs).
  - Integrated Action Bar hooks (Hint, Undo with history rewind, Replay, Reset) and victory/defeat modal flows.
  - Connected Act 4 level launches seamlessly into `CampaignScreen.cpp`.
- **Stage 7**: Full regression & verification complete across all 20 levels:
  - Full clean rebuild succeeded with **0 warnings and 0 errors**.
  - All 4 Acts and 20 levels verified against the shared base layer, action bar controls (Hint, Undo, Replay, Reset), result modals, and session-based unlocking.

## 2. Public Interface of Shared Classes

### `LevelScreenBase` (Derived from `Screen`)
- `void handleEvent(const sf::Event& event)`: Centralized event dispatcher for pan, zoom, action bar buttons, and modal dialogs.
- `void update(float dt)`: Central animation timer, toast updates, and deferred screen transitions.
- `void draw(sf::RenderTarget& target)`: Canvas drawing with pan/zoom view, overlay HUD cards, side panel, action bar, and modal popups.
- `void onResize(unsigned int width, unsigned int height)`: Responsive repositioning of canvas, action bar, and right side telemetry panel.
- `void setAlarm(int currentAlarm, int alarmCapacity)`: Mutates segmented alarm meter.
- `void setScore(int score)`: Updates current score display.
- `void setSteps(int currentSteps, int optimalSteps)`: Updates step telemetry counters.
- `void setPenalty(int penaltyPoints)`: Updates penalty deduction tally.
- `void setInstruction(const std::string& text)`: Sets mission objective text.
- `void addToLog(const std::string& entry)`: Appends an event to the scrollable operational log.
- `void showResultModal(...)`: Displays standard end-of-level victory/defeat dialog with retry, replay, next level, and return to campaign handlers.

### Shared Hooks for Derived Level Screens
- `virtual void onBackClicked()`
- `virtual void onHintClicked()`
- `virtual void onResetClicked()`
- `virtual void onUndoClicked()`
- `virtual void onReplayClicked()`
- `virtual void onNodeClicked(TreeNode* node)`
- `virtual void onCustomDraw(sf::RenderTarget& target)`
- `virtual void onDrawSidePanel(sf::RenderTarget& target, const sf::FloatRect& panelBounds)`

## 3. Frozen Rules (Do Not Break)
1. **Never alter visual theme, color schemes, font styling, or layout coordinates** of operational screens.
2. **Never execute synchronous screen deletions** inside UI callbacks or modal buttons; use deferred screen transitions (`m_pendingCallback`) via `LevelScreenBase`.
3. **No double-slashes (`//`) and no square brackets (`[]`)** in user-facing UI text and telemetry messages.
4. **Backend logic remains completely decoupled** from GUI rendering. All game state evaluations, validations, and step costs are performed by backend logic and communicated via `LevelBridge`.
5. **Campaign unlock state** starts from scratch on clean launch (first level unlocked), and supports session-only toggling via `Ctrl+Shift+U`.
