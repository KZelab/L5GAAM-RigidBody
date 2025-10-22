/**
 * PARTICLE SYSTEMS - VISUAL EFFECTS
 * ==================================
 *
 * Particle systems create effects using many small, short-lived objects
 * Common uses: explosions, smoke, fire, sparks, rain, magic effects
 *
 * KEY CHARACTERISTICS:
 * - Large number of simple objects (hundreds or thousands)
 * - Short lifespan (fade out quickly)
 * - Randomized properties (position, velocity, color, size)
 * - Efficient rendering (batching, no physics interactions)
 *
 * PERFORMANCE CONSIDERATIONS:
 * - Particles don't collide with each other (too expensive!)
 * - Use simple physics (linear motion, fade out)
 * - Batch render many particles in few draw calls
 * - Limit maximum particle count to prevent lag
 */
#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <random>

/**
 * Individual particle - lightweight, simple physics
 */
class Particle {
public:
    sf::Vector2f position;   // Current position
    sf::Vector2f velocity;   // Movement direction and speed
    sf::Color colour;        // Particle color
    float lifetime;          // Time remaining (seconds)
    float maxLifetime;       // Initial lifetime (for fade calculation)
    float size;              // Particle radius

    Particle(sf::Vector2f pos, sf::Vector2f vel, sf::Color col, float life, float sz)
        : position(pos), velocity(vel), colour(col), lifetime(life), maxLifetime(life), size(sz) {}

    /**
     * Simple particle physics:
     * - Move in straight line (no forces except drag)
     * - Apply drag/air resistance (velocity *= 0.98)
     * - Count down lifetime
     *
     * No complex physics needed - particles are just visual!
     */
    void update(float deltaTime) {
        position += velocity * deltaTime;  // Euler integration (simple!)
        lifetime -= deltaTime;              // Countdown to death
        velocity *= 0.98f;                  // Air resistance / drag
    }

    /**
     * Should this particle be removed?
     * Dead particles are erased from the system
     */
    bool isDead() const { return lifetime <= 0.0f; }

    /**
     * Calculate fade-out transparency
     * Alpha goes from 255 (opaque) to 0 (transparent) as lifetime decreases
     *
     * VISUAL EFFECT:
     * - Fresh particle: lifetime = maxLifetime → alpha = 255 (solid)
     * - Half-life: lifetime = maxLifetime/2 → alpha = 127 (translucent)
     * - Nearly dead: lifetime → 0 → alpha → 0 (invisible)
     */
    float getAlpha() const {
        return (lifetime / maxLifetime) * 255.0f;
    }
};

/**
 * ParticleSystem - Manages collision impact effects
 */
class ParticleSystem {
public:
    ParticleSystem() = default;

    /**
     * Create particle burst at collision point
     *
     * RANDOMIZATION:
     * - Uses random distributions for natural variation
     * - Each particle has random angle, speed, and size
     * - Creates realistic spray effect
     *
     * @param position  - Where the collision occurred
     * @param normal    - Direction to spray particles (away from surface)
     * @param colour    - Base color for particles
     * @param intensity - How powerful the collision was (0-1)
     *                    Affects particle count and speed
     */
    void createImpactBurst(sf::Vector2f position, sf::Vector2f normal, sf::Color colour, float intensity);

    /**
     * Update all particles
     * - Move particles
     * - Decrease lifetimes
     * - Remove dead particles (erase-remove idiom)
     */
    void update(float deltaTime);

    /**
     * BATCHED RENDERING OPTIMIZATION
     *
     * OLD APPROACH (slow):
     * - For each particle: create CircleShape, draw it
     * - 100 particles = 200 draw calls (particle + glow)
     *
     * NEW APPROACH (fast):
     * - Build vertex arrays for all particles
     * - Draw all particles in 2 calls (glows, then cores)
     * - 100 particles = 2 draw calls total!
     *
     * PERFORMANCE GAIN: 100x reduction in draw calls!
     *
     * HOW IT WORKS:
     * - Create triangles to form circles (approximation)
     * - Store all triangles in one VertexArray
     * - GPU draws them all at once
     */
    void draw(sf::RenderWindow& window);

    const std::vector<Particle>& getParticles() const { return particles; }

private:
    std::vector<Particle> particles;

    /**
     * Random number generator
     * std::mt19937 = Mersenne Twister (high-quality RNG)
     * Used to randomize particle properties
     */
    std::mt19937 gen{std::random_device{}()};

    /**
     * PERFORMANCE LIMIT
     * Cap particle count to prevent frame rate drops
     * During intense collisions, skip creating new particles if at limit
     *
     * Why 500?
     * - Enough for good visual effect
     * - Low enough to maintain 60 FPS
     * - Can be tuned based on target hardware
     */
    static constexpr size_t MAX_PARTICLES = 500;
};
