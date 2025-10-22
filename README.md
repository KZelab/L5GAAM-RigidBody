# Stage 2: Rotation and Torque

## Learning Objectives

By the end of this stage, you will understand:
- ✅ The rotational analog of linear motion
- ✅ What angular velocity and angular acceleration are
- ✅ How torque causes rotation (like force causes acceleration)
- ✅ What moment of inertia represents
- ✅ How to apply forces off-center to create spin

## What's New in This Stage

In Stage 1, our circles could move around but couldn't spin. Real objects rotate when forces are applied off-center - think of pushing a door at the edge versus the middle.

**Key Additions**:
- Circles now spin (rotate)
- Visual indicator showing rotation direction
- Forces applied off-center create torque
- Moment of inertia determines resistance to spinning

## Theoretical Background

### Linear vs Angular Motion

Every concept in linear (straight-line) motion has a rotational equivalent:

| Linear | Angular | Relationship |
|--------|---------|--------------|
| Position (m) | Rotation (radians) | How far/much rotated |
| Velocity (m/s) | Angular Velocity (rad/s) | How fast moving/spinning |
| Acceleration (m/s²) | Angular Acceleration (rad/s²) | Rate of change |
| Force (N) | Torque (N⋅m) | What causes change |
| Mass (kg) | Moment of Inertia (kg⋅m²) | Resistance to change |
| F = ma | τ = Iα | The fundamental equation |

### Understanding Rotation

**Rotation** is measured in radians (not degrees):
- 2π radians = 360 degrees = one full rotation
- π radians = 180 degrees = half rotation

**Angular Velocity** (ω) tells us how fast something spins:
- Positive = counter-clockwise
- Negative = clockwise
- Measured in radians/second

**Angular Acceleration** (α) is the rate of change of angular velocity:
- Like linear acceleration but for rotation
- Caused by torque (rotational force)

### Torque: The Rotational Force

**Torque** (τ) is what makes things rotate. It depends on:
1. **How much force** you apply
2. **Where** you apply it (distance from center)

```cpp
τ = r × F  // Cross product
τ = r * F * sin(θ)  // In 2D, becomes simple multiplication
```

**Real-World Examples**:
- Door: Pushing far from hinges = easy to open (large torque)
- Door: Pushing near hinges = hard to open (small torque)
- Wrench: Longer wrench = more torque with same force
- Bicycle pedal: Force at end of pedal creates rotation

### Moment of Inertia: Rotational Mass

**Moment of Inertia** (I) resists rotation like mass resists linear motion:

```cpp
// For a solid disk/circle:
I = 0.5 × mass × radius²
```

**Key Insights**:
- Larger radius → harder to spin (more inertia)
- More mass → harder to spin
- Distribution of mass matters!

**Real-World Examples**:
- Figure skater pulling arms in → smaller radius → smaller I → spins faster!
- Flywheel: Heavy rim (high I) stores rotational energy
- Ice dancer: Extended arms = slow spin, pulled in = fast spin

### The Rotational Equation of Motion

Just like Newton's Second Law (F = ma), we have:

```cpp
τ = I × α
α = τ / I  // Angular acceleration = torque / moment of inertia
```

This is exactly parallel to:
```cpp
F = m × a
a = F / m  // Acceleration = force / mass
```

## Code Changes from Stage 1

### 1. RigidBody Constructor (RigidBody.cpp:14)

Moment of inertia is now calculated:
```cpp
inertia = 0.5f * mass * radius * radius;  // For solid disk
```

### 2. Update Function (RigidBody.cpp:46-48)

Rotation is now updated every frame (previously disabled):

**Before (Stage 1)**:
```cpp
// Rotation will be added in Stage 4
rotation = 0.0f;
angularVelocity = 0.0f;
angularAcceleration = 0.0f;
```

**After (Stage 2)**:
```cpp
angularVelocity += angularAcceleration * deltaTime;
rotation += angularVelocity * deltaTime;
angularVelocity *= 0.98f;  // Angular damping (air resistance)
```

This is **semi-implicit Euler** for rotation - same pattern as linear motion!

### 3. Apply Torque Function (RigidBody.cpp:34-38)

New function to apply rotational force:
```cpp
void RigidBody::applyTorque(float torque) {
    if (!isStatic) {
        angularAcceleration += torque / inertia;  // α = τ / I
    }
}
```

### 4. Apply Force at Point (RigidBody.cpp:24-32)

Forces applied off-center now create both linear and angular motion:
```cpp
void RigidBody::applyForceAtPoint(const sf::Vector2f& force, const sf::Vector2f& point) {
    if (!isStatic) {
        acceleration += force / mass;  // Linear component (F = ma)

        sf::Vector2f r = point - position;  // Vector from center to point
        float torque = cross(r, force);      // τ = r × F
        applyTorque(torque);                 // Apply rotational component
    }
}
```

### 5. Visual Rotation Indicator (RigidBody.cpp:171-178)

White line shows rotation:
```cpp
if (!isStatic && std::abs(angularVelocity) > 0.1f) {
    sf::Vector2f lineEnd = position + rotate(sf::Vector2f(radius, 0.f), rotation);
    // Draw line from center to edge
}
```

## Observing Rotation

**What to Try**:
1. Right-click and drag objects - they spin!
2. Drag near the edge → more spin (larger torque)
3. Drag near center → less spin (smaller torque)
4. Watch the white line rotate with each circle
5. Notice heavier objects spin slower (higher inertia)

**Why Objects Spin**:
- When you drag with the mouse, force isn't perfectly through center
- Off-center force creates torque
- Torque causes angular acceleration
- Angular acceleration changes spin speed

## Discussion Questions

1. **Why does a door handle go on the edge, not the middle?**
   - Think about torque and radius

2. **What happens if you apply force exactly through the center of mass?**
   - Will it rotate? Why or why not?

3. **Why do larger circles spin slower than smaller ones (same force)?**
   - Consider the moment of inertia formula

4. **What's the difference between rotation and angular velocity?**
   - Hint: Same as difference between position and velocity

5. **Why multiply angular velocity by 0.98 each frame?**
   - What physical phenomenon does this simulate?

6. **If two circles have same mass but different radii, which spins easier?**
   - Calculate their moments of inertia to find out

7. **What would happen if we used explicit Euler for rotation?**
   - Compare to semi-implicit Euler stability

## Key Formulas Reference

```cpp
// Moment of inertia (solid disk)
I = 0.5 × m × r²

// Angular motion (rotational analog of F=ma)
τ = I × α
α = τ / I

// Torque from force and radius
τ = r × F  (cross product in 2D)

// Integration (semi-implicit Euler)
ω' = ω + α × dt
θ' = θ + ω × dt
```

## Further Reading

- [Moment of Inertia Explained](https://en.wikipedia.org/wiki/Moment_of_inertia)
- [Torque and Rotation](https://www.khanacademy.org/science/physics/torque-angular-momentum)
- [Angular Motion in Games](https://gafferongames.com/post/physics_in_3d/)
- [Cross Product in 2D](https://en.wikipedia.org/wiki/Cross_product)

---
