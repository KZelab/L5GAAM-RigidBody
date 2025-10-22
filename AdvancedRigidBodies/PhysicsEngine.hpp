#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <memory>
#include "RigidBody.hpp"
#include "ParticleSystem.hpp"
#include "SpatialGrid.hpp"

class PhysicsEngine {
public:
    PhysicsEngine(float width, float height);

    void addBody(std::unique_ptr<RigidBody> body);
    void clearDynamicBodies();
    void update(float deltaTime);
    void draw(sf::RenderWindow& window, bool showVelocity, bool showTrails, bool showDebug);

    void setGravity(const sf::Vector2f& g);
    sf::Vector2f getGravity() const { return gravity; }

    RigidBody* getBodyAt(const sf::Vector2f& point);
    size_t getBodyCount() const { return bodies.size(); }
    size_t getDynamicBodyCount() const;

private:
    void checkCollision(RigidBody& body1, RigidBody& body2);
    void updateSpatialGrid();
    void drawBatchedGlows(sf::RenderWindow& window);
    void drawBatchedTrails(sf::RenderWindow& window);

    std::vector<std::unique_ptr<RigidBody>> bodies;
    ParticleSystem particleSystem;
    SpatialGrid spatialGrid;
    sf::Vector2f gravity;
    float worldWidth;
    float worldHeight;

    // Vertex arrays for batched rendering
    sf::VertexArray glowVertices;
    sf::VertexArray trailVertices;
};
