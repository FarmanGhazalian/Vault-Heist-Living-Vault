#include <SFML/Graphics.hpp>

int main() {
    // 800x600 size ki test window (Vault Heist Theme)
    sf::RenderWindow window(sf::VideoMode({ 800, 600 }), "Vault Heist - SFML Window Test");

    while (window.isOpen()) {
        while (const auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>())
                window.close();
        }

        // Vault Dark Blue Background
        window.clear(sf::Color(20, 24, 33));
        window.display();
    }

    return 0;
}