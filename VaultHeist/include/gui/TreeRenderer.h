#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "backend/TreeNode.h"
#include "gui/Theme.h"

// TreeRenderer: Renders generic binary tree (TreeNode* root) in Cyber-Vault Neon style.
// Handles automatic node placement, edge drawing, state glow, and mouse interaction.
class TreeRenderer
{
public:
    TreeRenderer();

    // Configuration
    void setNodeRadius(float radius);
    void setVerticalSpacing(float spacing);
    void setMinHorizontalSpacing(float spacing);

    // Compute node coordinates recursively based on canvas bounding box
    void computeLayout(TreeNode* root, sf::FloatRect canvasBounds);

    // Compute heights and subtree sizes (leaf height = 1)
    static int updateNodeMetrics(TreeNode* node);

    // Shared tooltip height conversion (leaf = 0 in tooltip only)
    static int getTooltipHeight(const TreeNode* node);

    // Shared tooltip renderer according to Rule 7
    static void drawNodeTooltip(sf::RenderTarget& target, const Theme& theme, TreeNode* root, TreeNode* hoverNode, bool hideHeight = false);

    // Drawing
    void draw(sf::RenderTarget& target, const Theme& theme, TreeNode* root);

    // Interaction
    TreeNode* getNodeAtPosition(TreeNode* root, sf::Vector2f mousePos);
    void setHoveredNode(TreeNode* node);
    void setSelectedNode(TreeNode* node);
    void setHighlightedNode(TreeNode* node, sf::Color color);

    // Visual helpers
    void update(float dt);

private:
    float m_nodeRadius;
    float m_verticalSpacing;
    float m_minHSpacing;

    TreeNode* m_hoveredNode;
    TreeNode* m_selectedNode;
    TreeNode* m_highlightedNode;
    sf::Color m_highlightColor;
    float m_animTimer;

    // Layout helpers
    float computeSubtreeWidth(TreeNode* node);
    void assignPositions(TreeNode* node, float x, float y, float availableWidth);

    // Draw helpers
    void drawEdges(sf::RenderTarget& target, const Theme& theme, TreeNode* node);
    void drawNodes(sf::RenderTarget& target, const Theme& theme, TreeNode* node);
    void drawSingleNode(sf::RenderTarget& target, const Theme& theme, TreeNode* node);
};
