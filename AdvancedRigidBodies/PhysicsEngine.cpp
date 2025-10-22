#include "PhysicsEngine.hpp"
#include "Vector2Utils.hpp"
#include <algorithm>

using namespace PhysicsUtils;

PhysicsEngine::PhysicsEngine(float width, float height)
    : worldWidth(width), worldHeight(height), gravity(0.f, 500.f),
      spatialGrid(width, height, 100.0f) {
}

void PhysicsEngine::addBody(std::unique_ptr<RigidBody> body) {
    bodies.push_back(std::move(body));
}

void PhysicsEngine::clearDynamicBodies() {
    bodies.erase(
        std::remove_if(bodies.begin(), bodies.end(),
            [](const std::unique_ptr<RigidBody>& body) { return !body->getIsStatic(); }),
        bodies.end()
    );
}

size_t PhysicsEngine::getDynamicBodyCount() const {
    return std::count_if(bodies.begin(), bodies.end(),
        [](const std::unique_ptr<RigidBody>& body) { return !body->getIsStatic(); });
}

void PhysicsEngine::updateSpatialGrid() {
    spatialGrid.clear();
    for (auto& body : bodies) {
        spatialGrid.insert(body.get());
    }
}

void PhysicsEngine::update(float deltaTime) {
    for (auto& body : bodies) {
        if (!body->getIsStatic()) {
            body->applyForce(gravity * body->getMass());
        }
        body->updateCollisionInfo(deltaTime);
    }

    for (auto& body : bodies) {
        body->update(deltaTime);
        body->checkBoundaryCollision(worldWidth, worldHeight);
    }

    // Use spatial grid for collision detection
    updateSpatialGrid();
    auto potentialCollisions = spatialGrid.getPotentialCollisions();

    for (const auto& pair : potentialCollisions) {
        // Skip if both bodies are static
        if (pair.first->getIsStatic() && pair.second->getIsStatic()) {
            continue;
        }
        checkCollision(*pair.first, *pair.second);
    }

    particleSystem.update(deltaTime);
}

/**
 * COLLISION DETECTION & RESPONSE - NARROW PHASE
 * ==============================================
 *
 * This function implements:
 * 1. Circle-circle collision detection (geometric test)
 * 2. Position correction (separate overlapping objects)
 * 3. IMPULSE-BASED collision response (velocity change)
 * 4. Friction calculation
 *
 * IMPULSE-BASED PHYSICS:
 * Instead of applying forces over time (F = ma), we apply instant velocity changes
 * - Impulse J = change in momentum = Δ(mv)
 * - For instantaneous collisions, this is more stable than force-based
 * - Used in most modern game physics engines (Box2D, Bullet, PhysX)
 *
 * NEWTON'S THIRD LAW IN ACTION:
 * "For every action, there is an equal and opposite reaction"
 * - Body 1 receives impulse J
 * - Body 2 receives impulse -J (same magnitude, opposite direction)
 * - Total momentum is conserved!
 */
void PhysicsEngine::checkCollision(RigidBody& body1, RigidBody& body2) {
    // STEP 1: COLLISION DETECTION (Narrow Phase)
    // Check if two circles are overlapping

    sf::Vector2f diff = body2.getPosition() - body1.getPosition();
    float distance = length(diff);  // Distance between centers
    float minDistance = body1.getRadius() + body2.getRadius();  // Sum of radii

    // Are the circles overlapping?
    if (distance < minDistance) {
        // EDGE CASE: Bodies exactly on top of each other
        // Prevent division by zero when normalizing
        if (distance < 0.001f) {
            distance = 0.001f;
            diff = sf::Vector2f(1.0f, 0.0f) * minDistance;  // Push apart horizontally
        }

        // STEP 2: CALCULATE COLLISION GEOMETRY
        /**
         * COLLISION NORMAL: Direction to push bodies apart
         * Points from body1 toward body2
         *
         * VISUAL:
         *     body1  →  normal  →  body2
         *       O    ----------->    O
         */
        sf::Vector2f normal = normalise(diff);

        /**
         * OVERLAP/PENETRATION: How much circles are intersecting
         * This shouldn't happen in reality, but due to discrete timesteps, it does
         *
         * Example: radii are 20 and 30, distance is 45
         * - minDistance = 50 (they should be 50 apart)
         * - overlap = 50 - 45 = 5 pixels of penetration
         */
        float overlap = minDistance - distance;

        /**
         * SEPARATION FACTOR: Slightly over-separate to prevent jitter
         * 1.01 means separate 1% more than needed
         * Prevents floating-point errors from causing repeated collisions
         */
        float separationFactor = 1.01f;

        /**
         * CONTACT POINT: Where the collision occurred
         * Located on body1's surface, along the collision normal
         */
        sf::Vector2f contactPoint = body1.getPosition() + normal * body1.getRadius();

        // Store collision data for debug visualization
        body1.addCollisionInfo(contactPoint, -normal, overlap);
        body2.addCollisionInfo(contactPoint, normal, overlap);

        // STEP 3: POSITION CORRECTION
        /**
         * Separate the overlapping bodies immediately
         * This prevents objects from getting "stuck" inside each other
         *
         * THREE CASES:
         * 1. Both dynamic: Split the separation (each moves half)
         * 2. One static: Only move the dynamic one (static = infinite mass)
         * 3. Both static: No separation needed
         */
        if (!body1.getIsStatic() && !body2.getIsStatic()) {
            // Both dynamic: each moves half the overlap
            body1.setPosition(body1.getPosition() - normal * (overlap * 0.5f * separationFactor));
            body2.setPosition(body2.getPosition() + normal * (overlap * 0.5f * separationFactor));
        } else if (!body1.getIsStatic()) {
            // Only body1 is dynamic: it moves the full overlap
            body1.setPosition(body1.getPosition() - normal * (overlap * separationFactor));
        } else if (!body2.getIsStatic()) {
            // Only body2 is dynamic: it moves the full overlap
            body2.setPosition(body2.getPosition() + normal * (overlap * separationFactor));
        }

        // STEP 4: CALCULATE VELOCITY AT CONTACT POINT
        /**
         * For rotating objects, velocity at contact point ≠ velocity at center!
         *
         * FORMULA: v_contact = v_center + ω × r
         * Where:
         * - ω (omega) = angular velocity
         * - r = vector from center to contact point
         * - × = cross product (in 2D: produces perpendicular velocity)
         *
         * 2D CROSS PRODUCT: ω × r = (-ω * r.y, ω * r.x)
         * This gives the linear velocity caused by rotation
         *
         * EXAMPLE: Spinning basketball hits floor
         * - Center might be moving horizontally
         * - But contact point also has velocity from spin
         * - Total velocity = linear + rotational components
         */
        sf::Vector2f r1 = contactPoint - body1.getPosition();  // Radius vector to contact
        sf::Vector2f r2 = contactPoint - body2.getPosition();

        // Calculate velocity at contact point (linear + rotational)
        sf::Vector2f v1 = body1.getVelocity() + sf::Vector2f(-body1.getAngularVelocity() * r1.y, body1.getAngularVelocity() * r1.x);
        sf::Vector2f v2 = body2.getVelocity() + sf::Vector2f(-body2.getAngularVelocity() * r2.y, body2.getAngularVelocity() * r2.x);

        /**
         * RELATIVE VELOCITY: How fast objects are approaching/separating
         * Positive along normal = separating (no collision response needed)
         * Negative along normal = approaching (need to bounce apart)
         */
        sf::Vector2f relativeVelocity = v2 - v1;
        float velocityAlongNormal = dot(relativeVelocity, normal);

        // EARLY EXIT: Objects already separating? Don't apply impulse!
        if (velocityAlongNormal > 0) {
            return;  // Moving apart, no bounce needed
        }

        // STEP 5: CALCULATE IMPULSE MAGNITUDE
        /**
         * COEFFICIENT OF RESTITUTION (e)
         * Determines how "bouncy" the collision is
         * - e = 0: Perfectly inelastic (objects stick together)
         * - e = 1: Perfectly elastic (100% energy conserved)
         *
         * We use the MINIMUM of both bodies (less bouncy wins)
         * Example: Rubber ball (e=0.8) hits concrete (e=0.3) → use 0.3
         */
        float e = std::min(body1.restitution, body2.restitution);

        /**
         * CROSS PRODUCT: r × n
         * In 2D, cross product gives a SCALAR (in 3D it's a vector)
         * Used to calculate rotational effect of impulse
         *
         * Physics meaning: How much torque will the impulse create?
         * - Large r × n: Force applied far from center → lots of spin
         * - Small r × n: Force near center → little spin
         */
        float r1CrossN = cross(r1, normal);
        float r2CrossN = cross(r2, normal);

        /**
         * INVERSE MASS SUM
         * For collision resolution, we work with inverse mass (1/m)
         * Why? Static objects have infinite mass, so 1/∞ = 0 (easy to handle!)
         *
         * This sum determines how much the impulse affects linear motion
         */
        float invMassSum = 0.0f;
        if (!body1.getIsStatic()) invMassSum += 1.0f / body1.getMass();
        if (!body2.getIsStatic()) invMassSum += 1.0f / body2.getMass();

        /**
         * INVERSE INERTIA SUM (rotational component)
         * Similar to inverse mass, but for rotation
         * Weighted by (r × n)² because off-center impacts create more rotation
         *
         * Physics: How much will the impulse cause spinning?
         */
        float invInertiaSum = 0.0f;
        if (!body1.getIsStatic()) invInertiaSum += (r1CrossN * r1CrossN) / body1.getInertia();
        if (!body2.getIsStatic()) invInertiaSum += (r2CrossN * r2CrossN) / body2.getInertia();

        /**
         * IMPULSE FORMULA (derived from conservation of momentum and energy)
         *
         * j = -(1 + e) × v_rel_n / (1/m1 + 1/m2 + rotational_terms)
         *
         * Where:
         * - j = impulse magnitude (scalar)
         * - e = coefficient of restitution
         * - v_rel_n = relative velocity along normal
         * - rotational_terms = angular effects (from cross products)
         *
         * DERIVATION (simplified):
         * 1. Conservation of momentum: m1*v1' + m2*v2' = m1*v1 + m2*v2
         * 2. Coefficient of restitution: e = (v2' - v1') / (v1 - v2)
         * 3. Solve for impulse J that satisfies both equations
         * 4. Include rotational inertia for realistic spinning
         *
         * The negative sign is because we're calculating the approach velocity
         */
        float j = -(1.0f + e) * velocityAlongNormal;
        j /= (invMassSum + invInertiaSum);

        /**
         * IMPULSE VECTOR: Direction and magnitude
         * Impulse points along the collision normal
         * Magnitude j was calculated above
         */
        sf::Vector2f impulse = j * normal;

        // Visual effect: Create particle burst at impact
        float impactIntensity = std::abs(j) / 100.0f;  // Normalize for visuals
        sf::Color averageColour(
            (body1.getColour().r + body2.getColour().r) / 2,
            (body1.getColour().g + body2.getColour().g) / 2,
            (body1.getColour().b + body2.getColour().b) / 2
        );
        particleSystem.createImpactBurst(contactPoint, normal, averageColour, std::min(1.0f, impactIntensity));

        // STEP 6: APPLY IMPULSE - NEWTON'S THIRD LAW!
        /**
         * "For every action, there is an equal and opposite reaction"
         *
         * VELOCITY CHANGE from impulse:
         * Δv = J / m  (change in velocity = impulse / mass)
         *
         * ANGULAR VELOCITY CHANGE from impulse:
         * Δω = (r × J) / I  (change in angular velocity = torque / inertia)
         *
         * KEY OBSERVATIONS:
         * - Body 1 gets -impulse (negative direction)
         * - Body 2 gets +impulse (positive direction)
         * - Equal magnitude, opposite directions → Newton's 3rd Law!
         * - Heavier objects change velocity less (larger m in denominator)
         * - Off-center hits create more spin (larger r × J)
         */
        if (!body1.getIsStatic()) {
            // Linear velocity change
            body1.setVelocity(body1.getVelocity() - impulse / body1.getMass());

            // Angular velocity change (torque from impulse)
            body1.setAngularVelocity(body1.getAngularVelocity() - cross(r1, impulse) / body1.getInertia());
        }
        if (!body2.getIsStatic()) {
            // Note the OPPOSITE sign on impulse (+) - Newton's 3rd Law!
            body2.setVelocity(body2.getVelocity() + impulse / body2.getMass());
            body2.setAngularVelocity(body2.getAngularVelocity() + cross(r2, impulse) / body2.getInertia());
        }

        // STEP 7: FRICTION (Tangential Impulse)
        /**
         * FRICTION opposes sliding motion
         * Acts perpendicular (tangent) to the collision normal
         *
         * TANGENT DIRECTION:
         * - Normal = perpendicular to surface (bounce direction)
         * - Tangent = parallel to surface (sliding direction)
         *
         * VISUAL:
         *        normal ↑
         *               |
         *    ← tangent--O-→ tangent (sliding motion)
         *               |
         *               ↓
         *
         * We remove the normal component from relative velocity to get tangent
         */
        sf::Vector2f tangent = relativeVelocity - normal * velocityAlongNormal;

        // Only apply friction if there's tangential motion
        if (length(tangent) > 0.001f) {
            tangent = normalise(tangent);

            /**
             * COEFFICIENT OF FRICTION
             * Average the friction coefficients of both materials
             * Example: Rubber (0.8) on ice (0.1) → average = 0.45
             */
            float frictionCoeff = (body1.friction + body2.friction) * 0.5f;

            /**
             * FRICTION IMPULSE MAGNITUDE
             * Similar formula to normal impulse, but along tangent direction
             */
            float jt = -dot(relativeVelocity, tangent);
            jt /= (invMassSum + invInertiaSum);

            /**
             * COULOMB FRICTION LAW
             * Friction force is limited by normal force
             * - μ (mu) = coefficient of friction
             * - F_friction ≤ μ × F_normal
             *
             * In impulse terms: J_friction ≤ μ × J_normal
             *
             * PHYSICAL MEANING:
             * - Can't have more friction than the normal force allows
             * - Heavy normal force → more friction possible
             * - Light normal force → less friction possible
             * Example: Pushing down on object makes it harder to slide
             */
            float frictionLimit = std::abs(j * frictionCoeff);
            jt = std::clamp(jt, -frictionLimit, frictionLimit);

            /**
             * APPLY FRICTION IMPULSE
             * Same process as normal impulse, but along tangent
             * This slows down sliding and creates realistic rolling/skidding
             */
            sf::Vector2f frictionImpulse = jt * tangent;
            if (!body1.getIsStatic()) {
                body1.setVelocity(body1.getVelocity() - frictionImpulse / body1.getMass());
                body1.setAngularVelocity(body1.getAngularVelocity() - cross(r1, frictionImpulse) / body1.getInertia());
            }
            if (!body2.getIsStatic()) {
                body2.setVelocity(body2.getVelocity() + frictionImpulse / body2.getMass());
                body2.setAngularVelocity(body2.getAngularVelocity() + cross(r2, frictionImpulse) / body2.getInertia());
            }
        }
    }
}

void PhysicsEngine::drawBatchedGlows(sf::RenderWindow& window) {
    glowVertices.clear();
    glowVertices.setPrimitiveType(sf::PrimitiveType::Triangles);

    const int glowLayers = 3;
    const int segments = 16; // Reduced from default circle resolution for performance

    for (const auto& body : bodies) {
        if (body->getIsStatic()) continue;

        sf::Vector2f pos = body->getPosition();
        float radius = body->getRadius();
        sf::Color colour = body->getColour();
        float impactIntensity = body->impactIntensity;

        // Calculate display color
        sf::Color displayColour = colour;

        float flashIntensity = impactIntensity * 100.0f;
        displayColour.r = std::min(255, static_cast<int>(displayColour.r + flashIntensity));
        displayColour.g = std::min(255, static_cast<int>(displayColour.g + flashIntensity));
        displayColour.b = std::min(255, static_cast<int>(displayColour.b + flashIntensity));

        // Draw glow layers as triangle fans
        for (int layer = glowLayers; layer > 0; --layer) {
            float glowRadius = radius + (layer * 4.0f) + (impactIntensity * 5.0f);
            float alpha = 20.0f;
            alpha = alpha / (layer + 1) + (impactIntensity * 30.0f);

            sf::Color glowColor(displayColour.r, displayColour.g, displayColour.b,
                               static_cast<uint8_t>(alpha));

            // Create triangle fan for circle
            for (int i = 0; i < segments; ++i) {
                float angle1 = (i * 2.0f * 3.14159f) / segments;
                float angle2 = ((i + 1) * 2.0f * 3.14159f) / segments;

                sf::Vector2f p1 = pos + sf::Vector2f(std::cos(angle1) * glowRadius,
                                                      std::sin(angle1) * glowRadius);
                sf::Vector2f p2 = pos + sf::Vector2f(std::cos(angle2) * glowRadius,
                                                      std::sin(angle2) * glowRadius);

                glowVertices.append(sf::Vertex(pos, glowColor));
                glowVertices.append(sf::Vertex(p1, glowColor));
                glowVertices.append(sf::Vertex(p2, glowColor));
            }
        }
    }

    window.draw(glowVertices);
}

void PhysicsEngine::drawBatchedTrails(sf::RenderWindow& window) {
    trailVertices.clear();
    trailVertices.setPrimitiveType(sf::PrimitiveType::Lines);

    for (const auto& body : bodies) {
        const auto& trail = body->getMotionTrail();
        sf::Color colour = body->getColour();

        if (trail.size() < 2) continue;

        for (size_t i = 1; i < trail.size(); ++i) {
            uint8_t alpha = static_cast<uint8_t>(trail[i].alpha * 150);
            sf::Color trailColor(colour.r, colour.g, colour.b, alpha);

            trailVertices.append(sf::Vertex(trail[i - 1].position, trailColor));
            trailVertices.append(sf::Vertex(trail[i].position, trailColor));
        }
    }

    window.draw(trailVertices);
}

void PhysicsEngine::draw(sf::RenderWindow& window, bool showVelocity, bool showTrails, bool showDebug) {
    // Draw batched glows first (background layer)
    drawBatchedGlows(window);

    // Draw batched trails
    if (showTrails) {
        drawBatchedTrails(window);
    }

    // Draw particles (will be optimized separately)
    particleSystem.draw(window);

    // Draw bodies (main shapes, cores, rotation indicators)
    for (auto& body : bodies) {
        body->draw(window, showVelocity);
    }

    // Draw debug visualizations
    if (showDebug) {
        for (auto& body : bodies) {
            body->drawDebug(window);
        }
    }
}

void PhysicsEngine::setGravity(const sf::Vector2f& g) {
    gravity = g;
}

RigidBody* PhysicsEngine::getBodyAt(const sf::Vector2f& point) {
    for (auto& body : bodies) {
        sf::Vector2f diff = body->getPosition() - point;
        if (length(diff) <= body->getRadius()) {
            return body.get();
        }
    }
    return nullptr;
}
