#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "TreeNode.h"

struct RenderNode {
    int id = -1;
    int value = 0;
    int leftId = -1;
    int rightId = -1;
    sf::Vector2f currentPos{0.0f, 0.0f};
    sf::Vector2f targetPos{0.0f, 0.0f};
    bool isVisited = false;
    bool isWrong = false;
    float wrongTimer = 0.0f;
    bool isTarget = false;
    bool isLCA = false;
    bool isSelected = false;
    std::string stateLabel = "";
    float hoverScale = 1.0f;
    float popScale = 1.0f;
};

class TreeRenderer {
public:
    std::vector<RenderNode> nodes;

    void clear();
    void buildFromFlat(const std::vector<RenderNode>& rawList, int rootId, float centerX, float topY, float xSpan);
    void buildFromTree(TreeNode* root, float centerX, float topY, float xSpan);
    void update(float dt, const sf::Vector2f& mousePos);
    void draw(sf::RenderWindow& window, const sf::Font& font);

    int getNodeValueAt(const sf::Vector2f& mousePos) const;
    int getNodeIdAt(const sf::Vector2f& mousePos) const;
    RenderNode* findByValue(int val);
    RenderNode* findById(int id);
    void triggerWrong(int val);
    void markVisited(int val);
};
