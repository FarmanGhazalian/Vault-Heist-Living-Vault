<div align="center">

# 🏛️ Vault Heist: The Living Vault

**A data-structures puzzle game where the vault is a tree and the alarm is its height.**

![C++](https://img.shields.io/badge/C%2B%2B-17-00599C?style=flat-square&logo=cplusplus&logoColor=white)
![SFML](https://img.shields.io/badge/SFML-3.0-8CC445?style=flat-square)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?style=flat-square&logo=windows&logoColor=white)
![Campaign](https://img.shields.io/badge/Campaign-4%20Acts%20%7C%2020%20Levels-00B8D4?style=flat-square)
![Structures](https://img.shields.io/badge/Data%20Structures-From%20Scratch-FF2A55?style=flat-square)
![Theme](https://img.shields.io/badge/Theme-Cyber--Vault%20Neon-7C4DFF?style=flat-square)

*Read the tree. Break the vault. Escape before the alarm fills.*

</div>

| | |
|---|---|
| **Student** | Farman Ullah |
| **Roll Number** | 25I-3096 |
| **Section** | SE (3A) |
| **Course** | Data Structures |
| **Project** | Vault Heist: The Living Vault |

---

## 📑 Table of Contents

1. [Overview](#1--overview)
2. [Key Features](#2--key-features)
3. [Quick Start](#3--quick-start)
4. [System Requirements](#4--system-requirements)
5. [Controls](#5--controls)
6. [Game Modes](#6--game-modes)
7. [Campaign Structure](#7--campaign-structure)
8. [Scoring](#8--scoring)
9. [Data Structures and Complexity](#9--data-structures-and-complexity)
10. [Architecture](#10--architecture)
11. [Project Structure](#11--project-structure)
12. [Unit Tests and Benchmarks](#12--unit-tests-and-benchmarks)
13. [Adding a New Level](#13--adding-a-new-level)
14. [Third-Party Components](#14--third-party-components)
15. [Submission Contents](#15--submission-contents)
16. [Future Work](#16--future-work)

---

## 1. 🎯 Overview

The player is a thief breaking into a vault whose rooms form a **tree**. Each room holds a treasure with an integer value, and the player always starts at the root, the entrance. Entering a room, or comparing against a room's key, costs one **alarm step**. A tall, lopsided tree fills the alarm meter quickly, while a balanced tree keeps every path short. Tree height, balance and ordering therefore decide whether the player escapes.

The player learns by using the structures: reading traversals, reconstructing trees, searching and deleting in a BST, repairing imbalance with AVL rotations, and finally organising and sorting treasures with a Max Heap.

> [!IMPORTANT]
> All major data structures and algorithms (binary tree, BST, AVL tree, Max Heap and Heap Sort) are implemented from scratch. No library trees, BSTs, AVL trees, heaps or priority queues are used.

## 2. ✨ Key Features

- 🗺️ **Campaign:** four acts and twenty levels, with stars, scores and locked levels.
- ♾️ **Endless Mode:** procedurally generated waves of rising difficulty, with a local top-ten leaderboard.
- 🧪 **Sandbox:** a free-play editor for Binary Tree, BST, AVL Tree and Max Heap that supports every operation.
- 🎬 **Event-driven animation:** every model operation returns an event log, and the GUI plays it back (rotations, swaps, sift-up, sift-down, extraction).
- ⏪ **Undo, Replay, Reset and Hint** on every level.
- 🔀 **Synchronised tree and array views** for the Max Heap and Heap Sort.
- 🌱 **Seeded levels:** each level is generated from a seed, so results are reproducible and gradable.
- 📄 **JSON level files:** new levels can be added without changing code.
- ✅ **Validator:** checks BST ordering, stored heights and sizes, balance factors, parent links and the heap property.
- 📊 **Built-in unit tests and benchmarks**, started from the Settings screen.
- 🎨 **Cyber-Vault Neon theme** with colour-independent cues (labels, outlines and icons).

## 3. ⚡ Quick Start

1. Start the game and choose **Campaign** from the main menu.
2. Begin with **Level 1.1**: click the rooms in the order the guards will visit them.
3. Watch the **alarm meter**. Every wrong move raises it, and a full meter fails the level.
4. Finish a level to earn up to three stars and unlock the next one.

> [!TIP]
> Press `Ctrl + Shift + U` on the campaign map to unlock every level for testing or a demonstration. Press it again to lock them.

## 4. 💻 System Requirements

| Component | Requirement |
|---|---|
| Operating system | Windows 10 or 11 (64-bit) |
| Compiler and IDE | Visual Studio with the Desktop development with C++ workload (C++17) |
| Library | SFML 3.0 (64-bit, matching the Visual Studio toolset) |
| Bundled header | nlohmann JSON (included in `include/nlohmann`) |

## 5. 🎮 Controls

The game is played with the mouse.

| Action | Control |
|---|---|
| Select a room, answer a question, press a button | Left click |
| View key, height, balance factor, size and depth | Hover over a room |
| Zoom toward the cursor | Mouse wheel |
| Pan the view | Drag the background |
| Place a room token (reconstruction level) | Drag and drop |
| Type a value (for example in Sandbox or Level 1.3) | Number keys |

### 🧰 Toolbar

Each level provides **Undo**, **Replay**, **Reset**, **Hint** and an animation speed control. Using a hint a second time only shows a message and adds to the alarm.

### 🛠️ Developer Shortcut

`Ctrl + Shift + U` unlocks all campaign levels. Pressing it again locks them.

## 6. 🕹️ Game Modes

| Mode | Description |
|---|---|
| 🗺️ **Campaign** | Four acts and twenty levels. Stars and unlocks are kept for the current session. |
| ♾️ **Endless** | Waves of Binary Tree, BST, AVL and Max Heap challenges with rising difficulty. The run ends when the alarm meter is full; qualifying scores are saved to `leaderboard.json`. |
| 🧪 **Sandbox** | Free play with no alarm and no penalties. Switch between Binary Tree, BST, AVL and Max Heap, insert, delete, search, run traversals and range queries, rotate nodes, toggle AVL auto-balance, and run Extract-Max, Build-Heap and Heap Sort. |
| 🏅 **Leaderboard** | Top ten scores with player name, mode, waves or stars, and score. |
| ⚙️ **Settings** | Audio on or off, animation speed (0.5x to 2.0x), reset progress, and buttons to run the unit tests and the benchmark. |

## 7. 🗺️ Campaign Structure

### 🔵 Act 1: The Blueprint Wing (General Binary Tree)

| Level | Mission | Concept |
|---|---|---|
| 1.1 | Predict the guard patrol route | Preorder, inorder, postorder and level-order traversals |
| 1.2 | Decode the blueprint | Tree reconstruction from traversals |
| 1.3 | The alarm spreads from the leaves | Height and diameter |
| 1.4 | Where two escape routes meet | Lowest common ancestor and path |
| 1.5 | Mirror the vault | Tree inversion and recursion |

### 🟢 Act 2: The Sorted Wing (BST)

| Level | Mission | Concept |
|---|---|---|
| 2.1 | Search heist | BST search |
| 2.2 | Stash loot | BST insert |
| 2.3 | Steal and collapse | Delete: leaf, one child, two children |
| 2.4 | Range raid | Range query with pruning |
| 2.5 | Boss: The Lazy Architect | Degeneration into a corridor |

### 🟡 Act 3: The Architect's Wing (AVL Tree)

| Level | Mission | Concept |
|---|---|---|
| 3.1 | Read the badges | Balance factor |
| 3.2 | Manual repair, single rotations | LL and RR cases |
| 3.3 | Manual repair, double rotations | LR and RL cases |
| 3.4 | Auto mode and prediction | Insertion with rebalancing |
| 3.5 | Boss: Order-statistic heist | subtreeSize, kth and rank |

### 🔴 Act 4: The Heap Fortress (Max Heap and Heap Sort)

| Level | Mission | Concept |
|---|---|---|
| 4.1 | Activate the Vault | Max Heap property |
| 4.2 | Repair the Security Locks | Sift-up and sift-down |
| 4.3 | Extract the Greatest Treasure | Extract-max |
| 4.4 | Race Against the Guards | Heap Sort steps under an alarm limit |
| 4.5 | Final Vault Boss | Complete Heap Sort |

Act 4 shows the heap as a tree and as an array at the same time. A swap is highlighted in both views, and the active heap, extracted maxima and sorted region are labelled.

## 8. 🏆 Scoring

The alarm capacity of each level is `ceil(1.5 x optimalSteps) + 2`. When the meter is full, the level is failed. Optimal steps are computed by the game from the level data and are never typed in by hand.

| Result | Stars |
|---|---|
| Steps at most optimal, no penalties | ⭐⭐⭐ |
| Steps at most 1.25 x optimal, at most 1 penalty | ⭐⭐ |
| Completed within alarm capacity | ⭐ |
| Alarm full | 0 (retry) |

> [!WARNING]
> Penalties: a wrong traversal click adds 1 alarm step; a wrong rotation adds 2 alarm steps and subtracts 50 points; using a hint caps the star rating; an invalid swap in Act 4 adds 1 alarm step.

Score = `1000 - 10 x alarmSteps - penalties + bonuses`, with a minimum of 0.

## 9. 📐 Data Structures and Complexity

| Structure | Operation | Time complexity |
|---|---|---|
| Binary Tree | Traversals, height, diameter, LCA, mirror | O(n) |
| BST | Search, insert, delete | O(h), which is O(n) when degenerate |
| BST | Range query | O(h + k) with pruning |
| AVL Tree | Search, insert, delete (with rotations) | O(log n) |
| AVL Tree | kth, rank (using subtree size) | O(log n) |
| Max Heap | Insert (sift-up), extract-max (sift-down) | O(log n) |
| Max Heap | Build heap (bottom-up) | O(n) |
| Heap Sort | Sort n elements, in place | O(n log n) |

Here `h` is the tree height and `k` is the number of reported keys. A BST built from sorted input degenerates into a chain of height n, while an AVL tree keeps its height at O(log n). This difference is what the alarm meter makes visible in the game.

## 10. 🧱 Architecture

```
Model (data structures) -> Event Log -> Animation Controller -> Renderer
Input -> Level Logic -> Model operation -> new Event Log
```

The data structures have no dependency on the GUI. Every operation returns a list of events describing what happened, and the GUI plays these events back as animation. This keeps the model fully unit-testable and provides undo, replay and hints with little additional code.

| Module | Responsibility |
|---|---|
| BinaryTree | Traversals, reconstruction, LCA, diameter, mirror |
| BST | Ordered insert, search, delete, successor, predecessor, range query |
| AVLTree | Height, balance factor, rotations, subtree size, kth, rank |
| MaxHeap | Array-based heap, build, sift-up, sift-down, extract-max, Heap Sort |
| Validator | Checks structural invariants after operations |
| EventLog | Records and stores the events of each operation |
| LayoutEngine and Renderer | Compute node and array positions and draw them |
| AnimationController | Plays events with easing, pause, step and speed control |
| LevelManager and ScoreSystem | Level data, win and lose checks, alarm, penalties and stars |

## 11. 📁 Project Structure

The backend (data structures and game logic) and the frontend (GUI) are kept in separate folders, for both headers and source files.

```
VaultHeist/
|-- VaultHeist.sln                solution file
|-- VaultHeist/                   project folder
|   |-- include/
|   |   |-- backend/              headers: data structures, validator, events, levels, scoring
|   |   |-- gui/                  headers: screens, renderer, layout, animation, UI
|   |   `-- nlohmann/             bundled JSON header library
|   |-- src/
|   |   |-- backend/              implementation of the backend modules
|   |   `-- gui/                  implementation of the GUI modules and the entry point
|   |-- levels/                   level definition files (JSON)
|   |-- assets/                   fonts, sounds and images
|   `-- docs/                     project report, test report and level guide
`-- x64/                          build output (Debug and Release)
```

## 12. 🧪 Unit Tests and Benchmarks

Both are started from the Settings screen with the **RUN TEST CASES** and **RUN BENCHMARK** buttons.

| Output file | Contents |
|---|---|
| `test_report.txt` | PASS or FAIL for each test: traversals, reconstruction, BST operations, AVL rotations and balance, order statistics, Max Heap, Heap Sort and edge cases |
| `benchmark_results.csv` | BST versus AVL height and speed, search cost, rotation counts, order statistics, heap construction, Heap Sort and sorting comparison |

Benchmarks run without the GUI and with event recording disabled, using a fixed random seed so results can be repeated. For meaningful timings, run the project in the **Release** configuration.

> [!TIP]
> The benchmark file can be opened directly in a spreadsheet application to produce tables and charts.

## 13. ➕ Adding a New Level

Levels are described in JSON files in the `levels` folder. A new level requires only a new JSON file with its seed and parameters; no code change is needed. Changing the seed of a level produces a different tree or array. See `docs/add_new_level.md` for the step-by-step guide.

## 14. 🧩 Third-Party Components

| Component | Purpose |
|---|---|
| SFML 3.0 | Window, graphics, input and audio |
| nlohmann JSON | Reading and writing the JSON level and settings files |

No third-party component implements any tree, BST, AVL tree, heap or priority queue.

## 15. 📦 Submission Contents

- Complete source code and this README
- Unit test suite and test report (`test_report.txt`)
- Written project report
- Level data files and the guide `docs/add_new_level.md`

## 16. 🔭 Future Work

- Additional levels and Sandbox operations
- Sound and visual polish
- Saving and loading campaign progress between sessions

---

<div align="center">

**Farman Ullah** · 25I-3096 · SE (3A) · Data Structures Project

</div>
