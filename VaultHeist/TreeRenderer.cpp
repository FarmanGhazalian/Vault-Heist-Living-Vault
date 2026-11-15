#include "TreeRenderer.h"
#include "UIKit.h"
#include <cmath>
#include <algorithm>

void TreeRenderer::clear() {
    nodes.clear();
}

void TreeRenderer::buildFromFlat(const std::vector<RenderNode>& rawList, int rootId, float centerX, float topY, float xSpan) {
    std::vector<RenderNode> oldNodes = nodes;
    nodes = rawList;

    auto findIdx = [&](int id) {
        for (size_t i = 0; i < nodes.size(); ++i) {
            if (nodes[i].id == id) return static_cast<int>(i);
        }
        return -1;
    };

    auto place = [&](auto& self, int idx, float x, float y, float span) -> void {
        if (idx == -1) return;
        nodes[idx].targetPos = {x, y};

        // Preserve previous position for tweening if existed
        bool foundOld = false;
        for (const auto& on : oldNodes) {
            if (on.value == nodes[idx].value) {
                nodes[idx].currentPos = on.currentPos;
                nodes[idx].isVisited = on.isVisited;
                foundOld = true;
                break;
            }
        }
        if (!foundOld) {
            nodes[idx].currentPos = {x, y};
        }

        int l = findIdx(nodes[idx].leftId);
        int r = findIdx(nodes[idx].rightId);
        if (l != -1) self(self, l, x - span, y + 125.0f, span * 0.52f);
        if (r != -1) self(self, r, x + span, y + 125.0f, span * 0.52f);
    };

    int rootIdx = findIdx(rootId);
    if (rootIdx != -1) {
        place(place, rootIdx, centerX, topY, xSpan);
    }
}

void TreeRenderer::buildFromTree(TreeNode* root, float centerX, float topY, float xSpan) {
    if (!root) {
        nodes.clear();
        return;
    }
    std::vector<RenderNode> newNodes;
    int fakeId = 1000;

    auto collect = [&](auto& self, TreeNode* curr, float x, float y, float span) -> int {
        if (!curr) return -1;
        int myId = fakeId++;
        RenderNode rn;
        rn.id = myId;
        rn.value = curr->val;
        rn.targetPos = {x, y};

        // Keep old position if node already existed (for smooth tweening)
        bool foundOld = false;
        for (const auto& on : nodes) {
            if (on.value == curr->val) {
                rn.currentPos = on.currentPos;
                rn.isVisited = on.isVisited;
                foundOld = true;
                break;
            }
        }
        if (!foundOld) {
            rn.currentPos = {x, y};
        }

        int myIdx = static_cast<int>(newNodes.size());
        newNodes.push_back(rn);

        int lId = self(self, curr->left, x - span, y + 125.0f, span * 0.52f);
        int rId = self(self, curr->right, x + span, y + 125.0f, span * 0.52f);

        newNodes[myIdx].leftId = lId;
        newNodes[myIdx].rightId = rId;
        return myId;
    };

    collect(collect, root, centerX, topY, xSpan);
    nodes = newNodes;
}

void TreeRenderer::update(float dt, const sf::Vector2f& mousePos) {
    for (auto& n : nodes) {
        // Smooth position tweening
        sf::Vector2f diff = n.targetPos - n.currentPos;
        n.currentPos += diff * std::min(1.0f, dt * 10.0f);

        // Shake timer
        if (n.wrongTimer > 0.0f) {
            n.wrongTimer -= dt;
            if (n.wrongTimer <= 0.0f) {
                n.isWrong = false;
            }
        }

        // Pop scale decay
        if (n.popScale > 1.0f) {
            n.popScale -= dt * 2.5f;
            if (n.popScale < 1.0f) n.popScale = 1.0f;
        }

        // Hover check
        float dx = mousePos.x - n.currentPos.x;
        float dy = mousePos.y - n.currentPos.y;
        float distSq = dx * dx + dy * dy;
        float targetHover = (distSq <= 36.0f * 36.0f) ? 1.14f : 1.0f;
        n.hoverScale += (targetHover - n.hoverScale) * std::min(1.0f, dt * 12.0f);
    }
}

void TreeRenderer::draw(sf::RenderWindow& window, const sf::Font& font) {
    auto findPos = [&](int id) {
        for (const auto& n : nodes) {
            if (n.id == id) return n.currentPos;
        }
        return sf::Vector2f{0.0f, 0.0f};
    };

    // 1. Draw Pipes / Connectors
    for (const auto& n : nodes) {
        auto drawPipe = [&](int childId) {
            if (childId == -1) return;
            sf::Vector2f cp = findPos(childId);
            sf::Vector2f diff = cp - n.currentPos;
            float len = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            if (len < 1.0f) return;

            sf::RectangleShape p({len, 6.0f});
            p.setOrigin({0.0f, 3.0f});
            p.setPosition(n.currentPos);
            p.setRotation(sf::degrees(std::atan2(diff.y, diff.x) * 180.0f / 3.14159265f));
            p.setFillColor(sf::Color(45, 60, 90));
            p.setOutlineThickness(1.0f);
            p.setOutlineColor(Theme::PanelBorder);
            window.draw(p);
        };
        drawPipe(n.leftId);
        drawPipe(n.rightId);
    }

    // 2. Draw Nodes
    for (const auto& n : nodes) {
        float effectiveScale = n.hoverScale * n.popScale;
        float radius = 34.0f * effectiveScale;
        sf::Vector2f pos = n.currentPos;

        // Shake displacement on wrong move
        if (n.isWrong) {
            pos.x += std::sin(n.wrongTimer * 50.0f) * 6.0f;
        }

        // Outer glow on hover
        if (n.hoverScale > 1.02f) {
            sf::CircleShape glow(radius + 4.0f);
            glow.setOrigin({radius + 4.0f, radius + 4.0f});
            glow.setPosition(pos);
            glow.setFillColor(sf::Color(56, 189, 248, 50));
            window.draw(glow);
        }

        // Main node body
        sf::CircleShape circle(radius);
        circle.setOrigin({radius, radius});
        circle.setPosition(pos);

        sf::Color fillCol = sf::Color(22, 32, 54);
        sf::Color borderCol = Theme::PanelBorder;

        if (n.isWrong) {
            fillCol = sf::Color(90, 20, 20);
            borderCol = Theme::Red;
        } else if (n.isVisited) {
            fillCol = sf::Color(16, 75, 45);
            borderCol = Theme::Green;
        } else if (n.isTarget || n.isLCA) {
            fillCol = sf::Color(85, 65, 15);
            borderCol = Theme::Gold;
        } else if (n.isSelected) {
            fillCol = sf::Color(20, 60, 95);
            borderCol = Theme::AccentCyan;
        } else if (n.hoverScale > 1.02f) {
            fillCol = sf::Color(30, 45, 75);
            borderCol = Theme::AccentCyan;
        }

        circle.setFillColor(fillCol);
        circle.setOutlineThickness(1.5f * effectiveScale);
        circle.setOutlineColor(borderCol);
        window.draw(circle);

        // Value text
        sf::Text valText(font, std::to_string(n.value), static_cast<unsigned int>(20 * effectiveScale));
        valText.setStyle(sf::Text::Bold);
        valText.setFillColor(Theme::TextWhite);
        auto vb = valText.getLocalBounds();
        valText.setOrigin({vb.position.x + vb.size.x * 0.5f, vb.position.y + vb.size.y * 0.5f});
        valText.setPosition(pos);
        window.draw(valText);

        // Short state label below node
        std::string label = n.stateLabel;
        if (label.empty()) {
            if (n.isVisited) label = "VISITED";
            else if (n.isLCA) label = "LCA";
            else if (n.isTarget) label = "TARGET";
        }

        if (!label.empty()) {
            sf::Text lbl(font, label, 10);
            lbl.setStyle(sf::Text::Bold);
            lbl.setFillColor(n.isVisited ? Theme::Green : (n.isTarget || n.isLCA ? Theme::Gold : Theme::TextMuted));
            auto lb = lbl.getLocalBounds();
            lbl.setOrigin({lb.position.x + lb.size.x * 0.5f, 0.0f});
            lbl.setPosition({pos.x, pos.y + radius + 4.0f});
            window.draw(lbl);
        }
    }
}

int TreeRenderer::getNodeValueAt(const sf::Vector2f& mousePos) const {
    for (const auto& n : nodes) {
        float dx = mousePos.x - n.currentPos.x;
        float dy = mousePos.y - n.currentPos.y;
        if (dx * dx + dy * dy <= 36.0f * 36.0f) {
            return n.value;
        }
    }
    return -1;
}

int TreeRenderer::getNodeIdAt(const sf::Vector2f& mousePos) const {
    for (const auto& n : nodes) {
        float dx = mousePos.x - n.currentPos.x;
        float dy = mousePos.y - n.currentPos.y;
        if (dx * dx + dy * dy <= 36.0f * 36.0f) {
            return n.id;
        }
    }
    return -1;
}

RenderNode* TreeRenderer::findByValue(int val) {
    for (auto& n : nodes) {
        if (n.value == val) return &n;
    }
    return nullptr;
}

RenderNode* TreeRenderer::findById(int id) {
    for (auto& n : nodes) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

void TreeRenderer::triggerWrong(int val) {
    auto* n = findByValue(val);
    if (n) {
        n->isWrong = true;
        n->wrongTimer = 0.6f;
    }
}

void TreeRenderer::markVisited(int val) {
    auto* n = findByValue(val);
    if (n) {
        n->isVisited = true;
        n->popScale = 1.25f;
    }
}
