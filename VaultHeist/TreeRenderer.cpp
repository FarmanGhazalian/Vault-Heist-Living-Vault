#include "TreeRenderer.h"
#include <cmath>
#include <algorithm>

TreeRenderer::TreeRenderer()
    : m_nodeRadius(28.0f)
    , m_verticalSpacing(80.0f)
    , m_minHSpacing(64.0f)
    , m_hoveredNode(nullptr)
    , m_selectedNode(nullptr)
    , m_highlightedNode(nullptr)
    , m_highlightColor(Theme::BorderCyan)
    , m_animTimer(0.0f)
{
}

void TreeRenderer::setNodeRadius(float radius)
{
    m_nodeRadius = radius;
}

void TreeRenderer::setVerticalSpacing(float spacing)
{
    m_verticalSpacing = spacing;
}

void TreeRenderer::setMinHorizontalSpacing(float spacing)
{
    m_minHSpacing = spacing;
}

void TreeRenderer::update(float dt)
{
    m_animTimer += dt;
}

float TreeRenderer::computeSubtreeWidth(TreeNode* node)
{
    if (!node) return 0.0f;

    float leftW = computeSubtreeWidth(node->left);
    float rightW = computeSubtreeWidth(node->right);

    float childrenW = leftW + rightW + (node->left && node->right ? m_minHSpacing : 0.0f);
    return std::max(2.0f * m_nodeRadius, childrenW);
}

void TreeRenderer::assignPositions(TreeNode* node, float x, float y, float availableWidth)
{
    if (!node) return;

    node->position = sf::Vector2f(x, y);

    float leftW = computeSubtreeWidth(node->left);
    float rightW = computeSubtreeWidth(node->right);

    if (node->left && node->right)
    {
        float totalW = leftW + rightW + m_minHSpacing;
        float startX = x - (totalW / 2.0f);

        float leftCenterX = startX + (leftW / 2.0f);
        float rightCenterX = startX + leftW + m_minHSpacing + (rightW / 2.0f);

        assignPositions(node->left, leftCenterX, y + m_verticalSpacing, leftW);
        assignPositions(node->right, rightCenterX, y + m_verticalSpacing, rightW);
    }
    else if (node->left)
    {
        assignPositions(node->left, x - (m_minHSpacing * 0.75f), y + m_verticalSpacing, leftW);
    }
    else if (node->right)
    {
        assignPositions(node->right, x + (m_minHSpacing * 0.75f), y + m_verticalSpacing, rightW);
    }
}

void TreeRenderer::computeLayout(TreeNode* root, sf::FloatRect canvasBounds)
{
    if (!root) return;

    float rootX = canvasBounds.position.x + (canvasBounds.size.x * 0.5f);
    float rootY = canvasBounds.position.y + m_nodeRadius + 20.0f;

    assignPositions(root, rootX, rootY, canvasBounds.size.x);
}

TreeNode* TreeRenderer::getNodeAtPosition(TreeNode* root, sf::Vector2f mousePos)
{
    if (!root) return nullptr;

    float dx = mousePos.x - root->position.x;
    float dy = mousePos.y - root->position.y;
    float distSq = (dx * dx) + (dy * dy);

    if (distSq <= (m_nodeRadius * m_nodeRadius))
    {
        return root;
    }

    TreeNode* foundLeft = getNodeAtPosition(root->left, mousePos);
    if (foundLeft) return foundLeft;

    return getNodeAtPosition(root->right, mousePos);
}

void TreeRenderer::setHoveredNode(TreeNode* node)
{
    m_hoveredNode = node;
}

void TreeRenderer::setSelectedNode(TreeNode* node)
{
    m_selectedNode = node;
}

void TreeRenderer::setHighlightedNode(TreeNode* node, sf::Color color)
{
    m_highlightedNode = node;
    m_highlightColor = color;
}

void TreeRenderer::draw(sf::RenderTarget& target, const Theme& theme, TreeNode* root)
{
    if (!root) return;

    drawEdges(target, theme, root);
    drawNodes(target, theme, root);
}

void TreeRenderer::drawEdges(sf::RenderTarget& target, const Theme& theme, TreeNode* node)
{
    if (!node) return;

    auto drawSingleEdge = [&](sf::Vector2f start, sf::Vector2f end, bool isTarget)
    {
        sf::Vector2f dir = end - start;
        float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
        if (len < 0.001f) return;

        sf::Vector2f unitDir = dir / len;
        sf::Vector2f p1 = start + (unitDir * m_nodeRadius);
        sf::Vector2f p2 = end - (unitDir * m_nodeRadius);

        sf::Vector2f perp(-unitDir.y, unitDir.x);

        sf::Color edgeCol = isTarget ? Theme::AccentYellow : sf::Color(0, 240, 255, 140);
        sf::Color glowCol = isTarget ? sf::Color(255, 214, 10, 40) : sf::Color(0, 240, 255, 35);

        sf::Vertex glowQuad[4] = {
            sf::Vertex(p1 - perp * 3.5f, glowCol),
            sf::Vertex(p2 - perp * 3.5f, glowCol),
            sf::Vertex(p2 + perp * 3.5f, glowCol),
            sf::Vertex(p1 + perp * 3.5f, glowCol)
        };
        target.draw(glowQuad, 4, sf::PrimitiveType::TriangleStrip);

        sf::Vertex coreLine[4] = {
            sf::Vertex(p1 - perp * 1.0f, edgeCol),
            sf::Vertex(p2 - perp * 1.0f, edgeCol),
            sf::Vertex(p2 + perp * 1.0f, edgeCol),
            sf::Vertex(p1 + perp * 1.0f, edgeCol)
        };
        target.draw(coreLine, 4, sf::PrimitiveType::TriangleStrip);
    };

    if (node->left)
    {
        bool targetEdge = (node->state == TreeNode::TARGET && node->left->state == TreeNode::TARGET);
        drawSingleEdge(node->position, node->left->position, targetEdge);
        drawEdges(target, theme, node->left);
    }
    if (node->right)
    {
        bool targetEdge = (node->state == TreeNode::TARGET && node->right->state == TreeNode::TARGET);
        drawSingleEdge(node->position, node->right->position, targetEdge);
        drawEdges(target, theme, node->right);
    }
}

void TreeRenderer::drawNodes(sf::RenderTarget& target, const Theme& theme, TreeNode* node)
{
    if (!node) return;

    if (node->left) drawNodes(target, theme, node->left);
    if (node->right) drawNodes(target, theme, node->right);

    drawSingleNode(target, theme, node);
}

void TreeRenderer::drawSingleNode(sf::RenderTarget& target, const Theme& theme, TreeNode* node)
{
    if (!node) return;

    bool isHovered = (node == m_hoveredNode);
    bool isSelected = (node == m_selectedNode);
    bool isHighlighted = (node == m_highlightedNode);

    sf::Color ringColor = Theme::BorderCyan;
    if (node->state == TreeNode::LOCKED)
    {
        ringColor = sf::Color(80, 95, 120);
    }
    else if (node->state == TreeNode::TARGET)
    {
        ringColor = Theme::AccentYellow;
    }
    else if (node->state == TreeNode::INVALID)
    {
        ringColor = Theme::CrimsonAlarm;
    }
    else if (node->isVisited)
    {
        ringColor = Theme::SuccessEmerald;
    }

    if (isHighlighted) ringColor = m_highlightColor;
    if (isSelected) ringColor = Theme::AccentYellow;

    float r = m_nodeRadius;
    if (isHovered) r += 2.0f;

    // 1. Soft Outer Glow Ring
    sf::CircleShape glowRing(r + 6.0f);
    glowRing.setOrigin(sf::Vector2f(r + 6.0f, r + 6.0f));
    glowRing.setPosition(node->position);
    glowRing.setFillColor(sf::Color::Transparent);
    glowRing.setOutlineThickness(3.0f);
    sf::Color softGlow = ringColor;
    softGlow.a = isHovered ? 90 : 45;
    glowRing.setOutlineColor(softGlow);
    target.draw(glowRing);

    // 2. Node Background (Dark Glass)
    sf::CircleShape baseCircle(r);
    baseCircle.setOrigin(sf::Vector2f(r, r));
    baseCircle.setPosition(node->position);
    baseCircle.setFillColor(Theme::NodeDarkFill);
    baseCircle.setOutlineThickness(isHovered ? 2.5f : 1.8f);
    baseCircle.setOutlineColor(ringColor);
    target.draw(baseCircle);

    // 3. Node Value Text (Centered)
    std::string valStr = std::to_string(node->val);
    sf::Text valText(theme.fontMono, valStr, static_cast<unsigned int>(r * 0.72f));
    valText.setFillColor(isHovered ? sf::Color::White : Theme::TextPrimary);
    sf::FloatRect bounds = valText.getLocalBounds();
    valText.setOrigin(sf::Vector2f(bounds.position.x + bounds.size.x * 0.5f,
                                 bounds.position.y + bounds.size.y * 0.5f));
    valText.setPosition(node->position);
    target.draw(valText);

    // 4. Height / Size badge
    if (node->height > 1)
    {
        sf::Vector2f badgePos = node->position + sf::Vector2f(r * 0.65f, -r * 0.65f);
        sf::CircleShape badge(7.0f);
        badge.setOrigin(sf::Vector2f(7.0f, 7.0f));
        badge.setPosition(badgePos);
        badge.setFillColor(sf::Color(10, 14, 23, 220));
        badge.setOutlineThickness(1.0f);
        badge.setOutlineColor(Theme::BorderCyan);
        target.draw(badge);

        sf::Text hText(theme.fontMono, std::to_string(node->height), 9);
        hText.setFillColor(Theme::TextMuted);
        sf::FloatRect hBounds = hText.getLocalBounds();
        hText.setOrigin(sf::Vector2f(hBounds.position.x + hBounds.size.x * 0.5f,
                                    hBounds.position.y + hBounds.size.y * 0.5f));
        hText.setPosition(badgePos);
        target.draw(hText);
    }
}
