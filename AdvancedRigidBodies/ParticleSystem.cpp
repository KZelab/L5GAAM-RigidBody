#include "ParticleSystem.hpp"
#include "Vector2Utils.hpp"

using namespace PhysicsUtils;

void ParticleSystem::createImpactBurst(sf::Vector2f position, sf::Vector2f normal, sf::Color colour, float intensity) {
    // Limit particle creation if we're near the max
    if (particles.size() >= MAX_PARTICLES) {
        return; // Skip creating new particles if at limit
    }

    std::uniform_real_distribution<float> angleDist(-0.5f, 0.5f);
    std::uniform_real_distribution<float> speedDist(50.0f, 150.0f);
    std::uniform_real_distribution<float> sizeDist(1.0f, 3.0f);

    int particleCount = static_cast<int>(intensity * 20.0f);
    particleCount = std::clamp(particleCount, 5, 30);

    // Further reduce if we're getting close to limit
    size_t remainingCapacity = MAX_PARTICLES - particles.size();
    if (remainingCapacity < static_cast<size_t>(particleCount)) {
        particleCount = static_cast<int>(remainingCapacity);
    }

    for (int i = 0; i < particleCount; ++i) {
        float angle = std::atan2(normal.y, normal.x) + angleDist(gen);
        float speed = speedDist(gen) * intensity;
        sf::Vector2f velocity(std::cos(angle) * speed, std::sin(angle) * speed);

        particles.emplace_back(
            position,
            velocity,
            colour,
            0.3f + intensity * 0.2f,
            sizeDist(gen)
        );
    }
}

void ParticleSystem::update(float deltaTime) {
    for (auto& particle : particles) {
        particle.update(deltaTime);
    }

    particles.erase(
        std::remove_if(particles.begin(), particles.end(),
            [](const Particle& p) { return p.isDead(); }),
        particles.end()
    );
}

void ParticleSystem::draw(sf::RenderWindow& window) {
    for (const auto& particle : particles) {
        sf::Color col = particle.colour;
        uint8_t alpha = static_cast<uint8_t>(particle.getAlpha());
        col.a = alpha;

        // Draw glow (larger circle with lower alpha)
        sf::CircleShape glow(particle.size * 2.0f);
        glow.setPosition(particle.position - sf::Vector2f(particle.size * 2.0f, particle.size * 2.0f));
        sf::Color glowCol = col;
        glowCol.a = static_cast<uint8_t>(alpha * 0.3f);
        glow.setFillColor(glowCol);
        window.draw(glow);

        // Draw main particle
        sf::CircleShape shape(particle.size);
        shape.setPosition(particle.position - sf::Vector2f(particle.size, particle.size));
        shape.setFillColor(col);
        window.draw(shape);
    }
}
