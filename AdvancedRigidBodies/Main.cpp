#include <SFML/Graphics.hpp>
#include <random>
#include <iostream>
#include "PhysicsEngine.hpp"
#include "UIControls.hpp"

int main() {
    sf::VideoMode videoMode(sf::Vector2u(1200, 800));
    sf::RenderWindow window(videoMode, "Advanced Rigid Body Physics");
    window.setFramerateLimit(60);

    PhysicsEngine physics(1200.0f, 800.0f);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> posX(350.0f, 1100.0f);
    std::uniform_real_distribution<float> posY(100.0f, 300.0f);
    std::uniform_real_distribution<float> radiusDist(8.0f, 20.0f);
    std::uniform_real_distribution<float> massDist(1.0f, 5.0f);
    std::uniform_int_distribution<int> colourDist(120, 255);

    physics.addBody(std::make_unique<RigidBody>(
        sf::Vector2f(500.0f, 500.0f), 35.0f, 10.0f, sf::Color(80, 80, 90), true));
    physics.addBody(std::make_unique<RigidBody>(
        sf::Vector2f(700.0f, 400.0f), 30.0f, 10.0f, sf::Color(80, 80, 90), true));
    physics.addBody(std::make_unique<RigidBody>(
        sf::Vector2f(900.0f, 500.0f), 35.0f, 10.0f, sf::Color(80, 80, 90), true));

    for (int i = 0; i < 15; ++i) {
        float radius = radiusDist(gen);
        float mass = massDist(gen) * (radius / 20.0f);
        sf::Color colour(colourDist(gen), colourDist(gen), colourDist(gen));
        physics.addBody(std::make_unique<RigidBody>(
            sf::Vector2f(posX(gen), posY(gen)), radius, mass, colour));
    }

    sf::Font font;
    if (!font.openFromFile("C:/Windows/Fonts/arial.ttf")) {
        std::cerr << "Failed to load font" << std::endl;
        return 1;
    }

    UIControls ui(font);

    ui.onGravityChange = [&](float value) {
        physics.setGravity(sf::Vector2f(0.0f, value));
    };

    sf::Clock clock;
    sf::Clock fpsTimer;
    int frameCount = 0;
    float fps = 60.0f;
    RigidBody* draggedBody = nullptr;
    sf::Vector2f dragOffset;
    bool gravityOn = true;

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();

        frameCount++;
        if (fpsTimer.getElapsedTime().asSeconds() >= 1.0f) {
            fps = frameCount / fpsTimer.getElapsedTime().asSeconds();
            frameCount = 0;
            fpsTimer.restart();
        }

        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }

            sf::Vector2f mousePos(0.0f, 0.0f);
            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                mousePos = sf::Vector2f(static_cast<float>(mousePressed->position.x),
                                       static_cast<float>(mousePressed->position.y));
            }
            if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                mousePos = sf::Vector2f(static_cast<float>(mouseMoved->position.x),
                                       static_cast<float>(mouseMoved->position.y));
            }

            ui.handleEvent(event, mousePos);

            if (const auto* mousePressed = event->getIf<sf::Event::MouseButtonPressed>()) {
                if (!ui.isMouseOverUI(mousePos)) {
                    if (mousePressed->button == sf::Mouse::Button::Right) {
                        draggedBody = physics.getBodyAt(mousePos);
                        if (draggedBody) {
                            dragOffset = draggedBody->getPosition() - mousePos;
                        }
                    }
                    if (mousePressed->button == sf::Mouse::Button::Left) {
                        float radius = radiusDist(gen);
                        float mass = massDist(gen) * (radius / 20.0f);
                        sf::Color colour(colourDist(gen), colourDist(gen), colourDist(gen));
                        auto body = std::make_unique<RigidBody>(mousePos, radius, mass, colour);
                        body->setRestitution(ui.getRestitution());
                        body->setFriction(ui.getFriction());
                        physics.addBody(std::move(body));
                    }
                }
            }

            if (event->is<sf::Event::MouseButtonReleased>()) {
                draggedBody = nullptr;
            }

            if (const auto* mouseMoved = event->getIf<sf::Event::MouseMoved>()) {
                if (draggedBody && !draggedBody->getIsStatic()) {
                    sf::Vector2f targetPos = mousePos + dragOffset;
                    draggedBody->setVelocity((targetPos - draggedBody->getPosition()) * 10.0f);
                }
            }

            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Space) {
                    for (int i = 0; i < 8; ++i) {
                        float radius = radiusDist(gen);
                        float mass = massDist(gen) * (radius / 20.0f);
                        sf::Color colour(colourDist(gen), colourDist(gen), colourDist(gen));
                        auto body = std::make_unique<RigidBody>(
                            sf::Vector2f(posX(gen), posY(gen)), radius, mass, colour);
                        body->setRestitution(ui.getRestitution());
                        body->setFriction(ui.getFriction());
                        physics.addBody(std::move(body));
                    }
                } else if (keyPressed->scancode == sf::Keyboard::Scancode::C) {
                    physics.clearDynamicBodies();
                } else if (keyPressed->scancode == sf::Keyboard::Scancode::G) {
                    gravityOn = !gravityOn;
                    physics.setGravity(gravityOn ? sf::Vector2f(0.0f, ui.getGravity()) :
                                                   sf::Vector2f(0.0f, 0.0f));
                } else if (keyPressed->scancode == sf::Keyboard::Scancode::V) {
                    ui.showVelocityVectors = !ui.showVelocityVectors;
                } else if (keyPressed->scancode == sf::Keyboard::Scancode::T) {
                    ui.showMotionTrails = !ui.showMotionTrails;
                } else if (keyPressed->scancode == sf::Keyboard::Scancode::D) {
                    ui.showDebugVisualization = !ui.showDebugVisualization;
                }
            }
        }

        physics.update(deltaTime);
        ui.update(deltaTime);
        ui.updateStats(static_cast<int>(physics.getDynamicBodyCount()), fps);

        window.clear(sf::Color(10, 10, 15));
        physics.draw(window, ui.showVelocityVectors, ui.showMotionTrails, ui.showDebugVisualization);
        ui.draw(window);
        window.display();
    }

    return 0;
}
