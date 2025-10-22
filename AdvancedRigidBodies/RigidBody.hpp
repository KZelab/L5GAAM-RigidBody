#pragma once
#include <SFML/Graphics.hpp>
#include <deque>

/**
 * RIGID BODY PHYSICS - NEWTON'S LAWS IN ACTION
 * ============================================
 *
 * This class simulates a 2D rigid body (a circle) with realistic physics
 * based on Newton's Three Laws of Motion:
 *
 * NEWTON'S FIRST LAW (Law of Inertia):
 * "An object at rest stays at rest, and an object in motion stays in motion
 *  with the same speed and direction unless acted upon by a force"
 * - Implementation: Velocity persists until forces change it
 * - See: update() method - velocity only changes when acceleration is applied
 *
 * NEWTON'S SECOND LAW (F = ma):
 * "Force equals mass times acceleration"
 * - Implementation: acceleration = force / mass
 * - See: applyForce() method
 * - Heavier objects (higher mass) accelerate less from same force
 *
 * NEWTON'S THIRD LAW (Action-Reaction):
 * "For every action, there is an equal and opposite reaction"
 * - Implementation: In collision response, impulses are equal but opposite
 * - See: PhysicsEngine::checkCollision() - impulse applied to both bodies
 *
 * KEY PHYSICS CONCEPTS DEMONSTRATED:
 * - Linear motion: position, velocity, acceleration (Newton's laws)
 * - Angular motion: rotation, angular velocity, angular acceleration (rotational analog)
 * - Impulse-based collision response (change in momentum)
 * - Coefficient of restitution (bounciness)
 * - Friction (energy dissipation)
 * - Moment of inertia (resistance to rotation)
 */
class RigidBody {
public:
    /**
     * Visual trail showing motion path
     * Used for visualization, not physics
     */
    struct MotionTrail {
        sf::Vector2f position;  // Historical position
        float alpha;            // Transparency (fades over time)
    };

    /**
     * Debug information about collisions
     * Helps visualize collision physics
     */
    struct CollisionInfo {
        sf::Vector2f contactPoint;  // Where objects touched
        sf::Vector2f normal;         // Direction of collision force
        float penetration;           // How deep objects overlapped
        float lifetime;              // How long to display (for visualization)
    };

    /**
     * Constructor
     * @param pos   - Initial position (pixels)
     * @param r     - Radius (pixels)
     * @param m     - Mass (arbitrary units, affects inertia)
     * @param col   - Display color
     * @param stat  - Is this a static (immovable) object?
     */
    RigidBody(sf::Vector2f pos, float r, float m, sf::Color col, bool stat = false);

    /**
     * NEWTON'S SECOND LAW: F = ma, therefore a = F/m
     *
     * Apply a force to the body's center of mass
     * Force causes acceleration, which changes velocity over time
     *
     * @param force - Force vector in Newtons (direction and magnitude)
     *
     * UNITS:
     * - In real physics: Force (Newtons) = mass (kg) × acceleration (m/s²)
     * - In our simulation: Force (arbitrary) = mass × acceleration (pixels/s²)
     */
    void applyForce(const sf::Vector2f& force);

    /**
     * Apply force at a specific point (not center of mass)
     * Creates both LINEAR and ANGULAR acceleration
     *
     * TORQUE GENERATION:
     * - If force is applied off-center, it creates rotation
     * - Torque = r × F (cross product of radius vector and force)
     * - Example: Pushing the edge of a door makes it spin
     *
     * @param force - Force vector
     * @param point - World position where force is applied
     */
    void applyForceAtPoint(const sf::Vector2f& force, const sf::Vector2f& point);

    /**
     * Apply rotational force (torque)
     * Angular analog of linear force
     *
     * TORQUE: Causes angular acceleration (rotational equivalent of F=ma)
     * - τ = I × α (torque = moment of inertia × angular acceleration)
     * - Therefore: α = τ / I
     *
     * @param torque - Rotational force (positive = counter-clockwise)
     */
    void applyTorque(float torque);

    /**
     * NUMERICAL INTEGRATION - Simulating Motion Over Time
     *
     * Update body's state for one timestep using SEMI-IMPLICIT EULER method
     *
     * INTEGRATION METHODS (from simple to complex):
     * 1. Explicit Euler: v += a*dt; p += v*dt (simple but unstable)
     * 2. Semi-Implicit Euler: v += a*dt; p += v*dt (better stability) ← We use this!
     * 3. Verlet: More accurate, used in many physics engines
     * 4. Runge-Kutta (RK4): Very accurate, expensive
     *
     * WHY SEMI-IMPLICIT EULER?
     * - Good balance of accuracy and speed
     * - Better energy conservation than explicit Euler
     * - Simpler than Verlet or RK4
     *
     * @param deltaTime - Time elapsed since last frame (seconds)
     */
    void update(float deltaTime);

    /**
     * COLLISION DETECTION & RESPONSE: Walls
     *
     * Check and resolve collisions with world boundaries
     * Applies coefficient of restitution (bounciness) and friction
     */
    void checkBoundaryCollision(float width, float height);

    // Rendering methods (visual only, not physics)
    void draw(sf::RenderWindow& window, bool showVelocity);
    void drawMotionTrail(sf::RenderWindow& window);
    void drawDebug(sf::RenderWindow& window);


    // Debug/visualization helpers
    void addCollisionInfo(const sf::Vector2f& point, const sf::Vector2f& normal, float penetration);
    void updateCollisionInfo(float deltaTime);

    // Getters - Access physics state
    sf::Vector2f getPosition() const { return position; }
    sf::Vector2f getVelocity() const { return velocity; }
    float getRadius() const { return radius; }
    float getMass() const { return mass; }
    float getRotation() const { return rotation; }
    float getAngularVelocity() const { return angularVelocity; }
    float getInertia() const { return inertia; }
    bool getIsStatic() const { return isStatic; }
    sf::Color getColour() const { return colour; }
    const std::deque<MotionTrail>& getMotionTrail() const { return motionTrail; }

    // Setters - Modify physics state
    void setPosition(const sf::Vector2f& pos) { position = pos; }
    void setVelocity(const sf::Vector2f& vel) { velocity = vel; }
    void setAngularVelocity(float av) { angularVelocity = av; }
    void setRestitution(float r) { restitution = r; }
    void setFriction(float f) { friction = f; }

    /**
     * COEFFICIENT OF RESTITUTION (Bounciness)
     * Range: 0.0 to 1.0
     * - 0.0 = perfectly inelastic (no bounce, like clay)
     * - 0.6 = typical bouncy ball
     * - 1.0 = perfectly elastic (100% energy conserved, like ideal billiard ball)
     *
     * Physics: e = relative velocity after / relative velocity before
     */
    float restitution;

    /**
     * COEFFICIENT OF FRICTION
     * Range: typically 0.0 to 1.0 (can be higher for very rough surfaces)
     * - 0.0 = frictionless (like ice)
     * - 0.3 = typical value (moderate friction)
     * - 1.0 = high friction (rubber on concrete)
     *
     * Applied to both sliding AND rolling motion
     */
    float friction;

    /**
     * Impact intensity (0-1) for visual effects
     * Not part of physics simulation, just for rendering squash/glow
     */
    float impactIntensity;

    // Debug/visualization data
    sf::Vector2f lastImpulse;   // Last collision impulse (for visualization)
    sf::Vector2f appliedForce;  // Current applied force (for visualization)

private:
    // LINEAR MOTION STATE (Newton's Laws)
    sf::Vector2f position;      // Current position (pixels)
    sf::Vector2f velocity;      // Current velocity (pixels/second) - First Law
    sf::Vector2f acceleration;  // Current acceleration (pixels/second²) - Second Law

    // PHYSICAL PROPERTIES
    float mass;    // Mass affects inertia (resistance to acceleration) - Second Law
    float radius;  // Size of the circle (pixels)

    /**
     * ANGULAR MOTION STATE (Rotational analog of linear motion)
     *
     * Just as linear motion has position→velocity→acceleration,
     * angular motion has rotation→angular velocity→angular acceleration
     *
     * Linear    | Angular
     * ----------|-------------------
     * position  | rotation (angle)
     * velocity  | angular velocity (ω)
     * accel     | angular accel (α)
     * force     | torque (τ)
     * mass      | moment of inertia (I)
     * F = ma    | τ = I × α
     */
    float rotation;              // Current angle (radians)
    float angularVelocity;       // Rotation speed (radians/second)
    float angularAcceleration;   // Rotation acceleration (radians/second²)

    /**
     * MOMENT OF INERTIA (Rotational mass)
     *
     * Resists angular acceleration, like mass resists linear acceleration
     *
     * For a solid disk/circle: I = (1/2) × mass × radius²
     * - Larger radius → harder to spin
     * - More mass → harder to spin
     *
     * REAL-WORLD EXAMPLES:
     * - Figure skater pulls arms in → smaller radius → smaller I → spins faster!
     * - Heavy door (high I) is hard to push open
     * - Bicycle wheel (mass at rim) has high I, resists changing rotation
     */
    float inertia;

    // Visual and state properties
    sf::Color colour;     // Display color
    bool isStatic;        // Immovable object (infinite mass)?

    // Visual effects
    float squashStretch;  // Deformation factor for impact animation

    /**
     * Motion trail - historical positions for visualization
     * Using deque because we add to front and remove from back efficiently
     */
    std::deque<MotionTrail> motionTrail;

    /**
     * Collision debug info - temporary data for visualization
     * Shows contact points and normals
     */
    std::vector<CollisionInfo> collisionInfos;

    // Trail configuration
    static constexpr size_t MAX_TRAIL_LENGTH = 30;       // Max trail points
    static constexpr float TRAIL_UPDATE_INTERVAL = 0.05f; // Add point every 0.05s
    float trailTimer;  // Time until next trail point
};
