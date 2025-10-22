# Stage 3: Friction and Material Properties

## Learning Objectives

By the end of this stage, you will understand:
- ✅ What friction is and how it dissipates energy
- ✅ The difference between static and kinetic friction
- ✅ What coefficient of restitution (bounciness) represents
- ✅ How friction affects both sliding and rolling motion
- ✅ Why objects eventually come to rest

## What's New in This Stage

Objects in Stage 2 bounced forever because we had perfectly elastic collisions with no friction. Real surfaces have friction and lose energy during collisions.

**Key Additions**:
- Friction coefficient enabled (was 0.0, now 0.3)
- Objects slow down when sliding along surfaces
- Rotational friction (angular damping)
- More realistic material behavior
- Objects eventually come to rest

## Theoretical Background

### What is Friction?

**Friction** is a force that opposes relative motion between surfaces in contact. It converts kinetic energy into heat.

**Key Properties**:
- Always opposes motion (never helps it)
- Depends on the surfaces in contact (rubber vs ice)
- Independent of contact area (surprisingly!)
- Depends on normal force (how hard surfaces press together)

### Coefficient of Friction

The **coefficient of friction** (μ) describes how "sticky" a surface is:

| Material Pair | Coefficient (μ) |
|--------------|-----------------|
| Ice on ice | 0.02 - 0.05 |
| Wood on wood | 0.25 - 0.5 |
| Rubber on concrete | 0.6 - 0.85 |
| Steel on steel | 0.5 - 0.8 |
| Our simulation | 0.3 (moderate) |

**Formula**:
```cpp
F_friction = μ × F_normal
```

Where:
- `F_normal` = force pressing surfaces together
- `μ` = coefficient of friction (0 = frictionless, 1 = very rough)

### Types of Friction

1. **Static Friction**: Prevents motion from starting
   - Acts when objects are at rest relative to each other
   - Must be overcome to start sliding
   - Generally higher than kinetic friction

2. **Kinetic Friction**: Opposes ongoing motion
   - Acts when objects are sliding
   - What we simulate in this stage
   - Lower than static friction

3. **Rolling Friction**: Acts on rotating objects
   - Much lower than sliding friction
   - Why wheels are efficient

### Coefficient of Restitution (Bounciness)

**Restitution** (e) determines how much energy is preserved in a bounce:

```cpp
e = relative velocity after / relative velocity before
```

| Value | Meaning | Example |
|-------|---------|---------|
| e = 1.0 | Perfectly elastic | Ideal billiard ball |
| e = 0.6 | Moderate bounce | Our simulation (bouncy ball) |
| e = 0.3 | Low bounce | Basketball |
| e = 0.0 | No bounce | Clay ball |

**Energy Loss**:
- Each bounce loses energy proportional to (1 - e²)
- With e = 0.6, about 64% of energy is retained
- Object eventually stops bouncing

### How Friction Works in Collisions

During a collision with a wall or another object:

1. **Normal Component** (perpendicular to surface):
   - Velocity reversed and scaled by restitution
   - `v_normal' = -v_normal × e`

2. **Tangential Component** (parallel to surface):
   - Velocity reduced by friction
   - `v_tangent' = v_tangent × (1 - μ)`

3. **Angular Component** (rotation):
   - Angular velocity reduced by friction
   - `ω' = ω × (1 - μ)`

## Code Changes from Stage 2

### 1. RigidBody Constructor (RigidBody.cpp:11)

Friction coefficient changed from 0.0 to 0.3:

**Before (Stage 2)**:
```cpp
restitution(0.6f), friction(0.0f), trailTimer(0.0f),
```

**After (Stage 3)**:
```cpp
restitution(0.6f), friction(0.3f), trailTimer(0.0f),
```

### 2. Boundary Collision - Horizontal Walls (RigidBody.cpp:79-91)

Friction now applied to tangential motion:

```cpp
if (position.x - radius < 0) {
    position.x = radius;
    velocity.x = -velocity.x * restitution;    // Bounce (normal)
    velocity.y *= (1.0f - friction);           // Friction (tangential) ← NEW
    angularVelocity *= (1.0f - friction);      // Rotational friction ← NEW
}
```

**What's happening**:
- `velocity.x`: Normal component bounces with restitution
- `velocity.y`: Tangential component reduced by friction
- `angularVelocity`: Spin slows down due to friction

### 3. Boundary Collision - Vertical Walls (RigidBody.cpp:103-106)

Same friction logic for vertical walls:

```cpp
if (position.y + radius > height) {
    position.y = height - radius;
    velocity.y = -velocity.y * restitution;    // Bounce (normal)
    velocity.x *= (1.0f - friction);           // Friction (tangential) ← NEW
    angularVelocity *= (1.0f - friction);      // Rotational friction ← NEW
}
```

### 4. Object-Object Collisions (PhysicsEngine.cpp)

Similar friction applied in circle-circle collisions:
- Normal impulse scaled by restitution
- Tangential velocity reduced by friction
- Both objects affected

## Observing Friction

**What to Try**:

1. **Drop objects straight down**:
   - They bounce but eventually stop (energy lost to friction)
   - Compare to Stage 2 where they bounced forever

2. **Slide objects along bottom wall**:
   - Notice how they slow down horizontally (tangential friction)
   - Watch rotation slow down too

3. **Spin an object and let it hit a wall**:
   - Angular velocity decreases on each bounce
   - Eventually stops spinning

4. **High-speed collision**:
   - More energy lost (friction proportional to velocity)
   - Fewer bounces before stopping

**Energy Dissipation**:
- Restitution: ~36% energy lost per bounce (e = 0.6)
- Friction: 30% of tangential velocity lost per wall hit (μ = 0.3)
- Objects come to rest after a few seconds

## Discussion Questions

1. **Why do objects eventually stop moving?**
   - Where does the kinetic energy go?

2. **What would happen with friction = 1.0?**
   - Try modifying the code!

3. **Why is friction applied to both linear and angular velocity?**
   - What physical phenomenon does this represent?

4. **How is friction different from air resistance (damping)?**
   - Hint: Look at when each is applied

5. **What happens if restitution = 1.0 and friction = 0.0?**
   - Would objects ever stop? Why or why not?

6. **Why multiply velocity by (1 - friction) instead of subtracting?**
   - Consider what happens with different velocities

7. **In real life, why doesn't friction depend on contact area?**
   - Research the Coulomb friction model

## Real-World Applications

**Video Games**:
- Platformers: Ground friction affects acceleration/deceleration
- Racing games: Different surfaces (asphalt, grass, ice) have different μ
- Pool/billiards: Friction makes balls stop, restitution controls bounce

**Sports**:
- Basketball: Low restitution (0.3) prevents excessive bouncing
- Tennis: Fuzzy ball has higher friction with court
- Curling: Ice surface has very low friction (μ ≈ 0.02)

**Engineering**:
- Brake pads: High friction materials (μ > 0.6)
- Bearings: Low friction to reduce energy loss
- Tires: Balance between grip (high μ) and wear

## Key Formulas Reference

```cpp
// Friction force
F_friction = μ × F_normal

// Velocity after friction (multiplicative model)
v' = v × (1 - μ)

// Coefficient of restitution
e = |v_after| / |v_before|
e = √(kinetic energy after / kinetic energy before)

// Tangential and normal components
v_normal = (v · n) × n  // Parallel to surface normal
v_tangent = v - v_normal // Perpendicular to normal
```

## Comparing the Stages

| Property | Stage 1 | Stage 2 | Stage 3 |
|----------|---------|---------|---------|
| Rotation | ❌ No | ✅ Yes | ✅ Yes |
| Friction | ❌ None (μ=0) | ❌ None (μ=0) | ✅ Moderate (μ=0.3) |
| Bounciness | ✅ e=0.6 | ✅ e=0.6 | ✅ e=0.6 |
| Objects stop? | ❌ No | ❌ No | ✅ Yes |
| Realistic? | Low | Medium | **High** |

## Further Reading

- [Coulomb Friction Model](https://en.wikipedia.org/wiki/Friction)
- [Coefficient of Restitution](https://en.wikipedia.org/wiki/Coefficient_of_restitution)
- [Physics of Friction in Games](https://www.gamasutra.com/view/feature/131424/pool_hall_lessons_fast_accurate_.php)
- [Material Properties Table](https://www.engineeringtoolbox.com/friction-coefficients-d_778.html)

---
