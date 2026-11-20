Vault Heist: The Living Vault — A 2D SFML-based data structures puzzle game featuring dynamic 20-level binary tree mechanics, model-event-renderer architecture, and interactive animation feedback.

## 10. Project Structure

```
VaultHeist/
├── assets/                  # Fonts, sounds, and media
├── docs/                    # Design specifications and documentation
├── include/
│   ├── backend/             # Data structures and level logic headers
│   │   ├── AVL.h
│   │   ├── BinaryTree.h
│   │   ├── BST.h
│   │   ├── MaxHeap.h
│   │   ├── Validator.h
│   │   ├── EventLog.h
│   │   ├── LevelBridge.h
│   │   ├── ScoreSystem.h
│   │   ├── SaveSystem.h
│   │   ├── SettingsManager.h
│   │   ├── LeaderboardManager.h
│   │   ├── LevelGenerator.h
│   │   ├── Benchmark.h
│   │   ├── TestSuite.h
│   │   └── Act*Level*.h
│   ├── gui/                 # Screens, renderer, and UI headers
│   │   ├── GameEngine.h
│   │   ├── Screen.h
│   │   ├── LevelScreenBase.h
│   │   ├── TreeRenderer.h
│   │   ├── UIKit.h
│   │   ├── Theme.h
│   │   ├── AudioManager.h
│   │   ├── AnimationController.h
│   │   └── *Screen.h
│   └── nlohmann/            # JSON library (json.hpp)
├── levels/                  # Level JSON files (Act*.json, endless_config.json)
├── src/
│   ├── backend/             # Model, data structure, and game logic implementations
│   │   ├── AVL.cpp
│   │   ├── BinaryTree.cpp
│   │   ├── BST.cpp
│   │   ├── MaxHeap.cpp
│   │   ├── Validator.cpp
│   │   ├── EventLog.cpp
│   │   ├── LevelBridge.cpp
│   │   ├── ScoreSystem.cpp
│   │   ├── SaveSystem.cpp
│   │   ├── SettingsManager.cpp
│   │   ├── LeaderboardManager.cpp
│   │   ├── LevelGenerator.cpp
│   │   ├── Benchmark.cpp
│   │   ├── TestSuite.cpp
│   │   └── Act*Level*.cpp
│   ├── gui/                 # UI screens and renderer implementations
│   │   ├── GameEngine.cpp
│   │   ├── LevelScreenBase.cpp
│   │   ├── TreeRenderer.cpp
│   │   ├── UIKit.cpp
│   │   ├── Theme.cpp
│   │   ├── AudioManager.cpp
│   │   └── *Screen.cpp
│   └── main.cpp             # Application entry point
├── VaultHeist.slnx          # Solution file
├── VaultHeist.vcxproj       # Project build file
├── VaultHeist.vcxproj.filters # Project filter structure
├── .gitignore               # Build output exclusions
└── README.md                # Project documentation
```
