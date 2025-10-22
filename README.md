# Stage 1: Baseline - Simple Physics Simulation

## Learning Objectives

By the end of this stage, you will understand:
- ✅ The game loop pattern and why it's fundamental to game development
- ✅ What `deltaTime` is and why frame-independent physics matters
- ✅ Basic 2D vector mathematics (position, velocity)
- ✅ Euler integration for physics simulation
- ✅ Circle collision detection fundamentals

## Theoretical Background

### The Game Loop

Every game and physics simulation runs on a **game loop** - a continuous cycle that updates the simulation and renders it to the screen:

```cpp
while (game is running) {
    1. Handle user input (mouse clicks, keyboard)
    2. Update physics (move objects, apply forces)
    3. Render (draw everything to screen)
}
```

**Why do we need a loop?**
- Games are interactive and continuous (unlike a static program)
- Physics must be updated many times per second to appear smooth
- Target: 60 times per second (60 FPS) for smooth animation

### DeltaTime: Frame-Independent Physics

**The Problem**: Different computers run at different speeds!
- Fast computer: 100 FPS → update every 0.01 seconds
- Slow computer: 30 FPS → update every 0.033 seconds

**The Solution**: Use `deltaTime` (time elapsed since last frame)

```cpp
// BAD - frame-dependent
position += velocity;  // Moves faster on faster computers!

// GOOD - frame-independent
position += velocity * deltaTime;  // Same speed on all computers!
```

**Example**:
- Velocity = 100 pixels/second
- Fast PC (60 FPS, dt=0.0167s): moves 100 × 0.0167 = 1.67 pixels/frame
- Slow PC (30 FPS, dt=0.033s): moves 100 × 0.033 = 3.33 pixels/frame
- **Result**: Both move 100 pixels in 1 second!

### Euler Integration

**Euler integration** is the simplest method to simulate physics over time:

```cpp
velocity += acceleration * deltaTime;  // Update velocity
position += velocity * deltaTime;      // Update position
```

**Why this order matters**:
- We use **semi-implicit Euler** (update velocity first)
- More stable than explicit Euler (update position first)
- Good balance of simplicity and accuracy

**Physical Meaning**:
1. Acceleration changes velocity (Newton's 2nd Law)
2. Velocity changes position (definition of velocity)

### 2D Vectors

Vectors represent quantities with **magnitude** and **direction**:

```cpp
sf::Vector2f position(100.0f, 200.0f);  // x=100, y=200
sf::Vector2f velocity(50.0f, -30.0f);   // moving right and up
```

**Common Vector Operations**:
- Addition: `a + b` (combine movements)
- Subtraction: `a - b` (find difference/direction)
- Scalar multiplication: `v * 2.0f` (double the magnitude)
- Length: `√(x² + y²)` (how far from origin)

## The Problem We're Solving

**Goal**: Create a simple physics simulation where:
- Circles fall due to gravity
- Circles bounce off walls and each other
- User can add more circles with mouse clicks
- Adjustable parameters (gravity, bounciness)

**Starting Simple**:
- No rotation (circles don't spin)
- No friction (surfaces are frictionless)
- Basic collision (will improve later)

## Implementation Overview

### Key Files

- **`Main.cpp`** - Game loop, window management, user input
- **`RigidBody.hpp/cpp`** - Physics object (circle with position, velocity)
- **`PhysicsEngine.hpp/cpp`** - Manages all objects, handles collisions
- **`UIControls.hpp/cpp`** - Sliders for adjusting parameters
- **`ParticleSystem.hpp/cpp`** - Visual effects for collisions
- **`Vector2Utils.hpp`** - Helper functions for vector math

### The Game Loop (Main.cpp)

```cpp
while (window.isOpen()) {
    float deltaTime = clock.restart().asSeconds();  // Time since last frame

    // 1. Handle Input
    while (const std::optional event = window.pollEvent()) {
        // Mouse clicks, keyboard, window close
    }

    // 2. Update Physics
    physics.update(deltaTime);

    // 3. Render
    window.clear();
    physics.draw(window);
    window.display();
}
```

### Physics Update (RigidBody.cpp)

```cpp
void RigidBody::update(float deltaTime) {
    // Semi-implicit Euler integration
    velocity += acceleration * deltaTime;  // v' = v + a×dt
    position += velocity * deltaTime;      // p' = p + v×dt

    velocity *= 0.99f;  // Air resistance (damping)
    acceleration = {0, 0};  // Clear acceleration for next frame
}
```

### Collision Detection (PhysicsEngine.cpp)

```cpp
// Check every pair of objects (O(n²) - we'll optimize later!)
for (size_t i = 0; i < bodies.size(); ++i) {
    for (size_t j = i + 1; j < bodies.size(); ++j) {
        float distance = length(pos2 - pos1);
        float minDistance = radius1 + radius2;

        if (distance < minDistance) {
            // Collision! Separate and bounce
        }
    }
}
```

## Building and Running

### Prerequisites
- Visual Studio 2019 or later
- SFML 3.x library (install via vcpkg)

### Compilation
```bash
# Using vcpkg
vcpkg install sfml:x64-windows

# Open AdvancedRigidBodies.sln in Visual Studio
# Build → Build Solution (F7)
# Run (F5)
```

### Controls
- **Left Click**: Add a new object at cursor
- **Right Click + Drag**: Grab and move objects
- **Space**: Add 8 random objects
- **C**: Clear all dynamic objects
- **G**: Toggle gravity on/off
- **V**: Toggle velocity vectors
- **T**: Toggle motion trails
- **D**: Toggle debug visualization

## Exercises

### Exercise 1: Modify Gravity
1. Run the simulation and observe the default behavior
2. Use the gravity slider to set gravity to 0
   - **Question**: What happens? Why?
3. Set gravity to 1000
   - **Question**: Do all objects fall at the same rate? Why?
4. Set gravity to -500 (negative = upward)
   - **Question**: What happens now?

### Exercise 2: Understand DeltaTime
1. Add a print statement in the game loop:
   ```cpp
   std::cout << "DeltaTime: " << deltaTime << " FPS: " << 1.0f/deltaTime << std::endl;
   ```
2. Run and observe the values
   - **Question**: Does deltaTime stay constant? Why not?
3. What happens to FPS when you add 100 objects?

### Exercise 3: Experiment with Integration
1. Find the `update()` method in `RigidBody.cpp`
2. Try explicit Euler (swap the order):
   ```cpp
   position += velocity * deltaTime;      // Position first
   velocity += acceleration * deltaTime;  // Then velocity
   ```
3. **Question**: Do you notice any difference in behavior?

### Exercise 4: Vector Math
1. In `RigidBody.cpp`, add a constant velocity:
   ```cpp
   velocity += sf::Vector2f(50.0f, 0.0f) * deltaTime;  // Push right
   ```
2. **Question**: What happens to the objects?
3. Try different vectors:
   - `(0.0f, -100.0f)` - push up
   - `(100.0f, 100.0f)` - push diagonally

## Performance Metrics

| Bodies | FPS | Collision Checks | Draw Calls |
|--------|-----|------------------|------------|
| 10     | 60  | 45               | ~60        |
| 30     | 60  | 435              | ~180       |
| 50     | 45  | 1,225            | ~300       |
| 100    | 25  | 4,950            | ~600       |

**Observation**: Performance degrades significantly with more objects!
- **Why?** O(n²) collision detection (we'll fix in Stage 7)
- **Why?** Lots of draw calls (we'll fix in Stage 8)

## Discussion Questions

1. **Why is the game loop infinite?** What would happen without a loop?

2. **What is the relationship between FPS and deltaTime?**
   - Hint: FPS = 1 / deltaTime

3. **Why do we multiply by deltaTime in physics?**
   - What would happen without it?

4. **What does "frame-independent" mean?**
   - Why is it important for games?

5. **What is the difference between position and velocity?**
   - How are they related mathematically?

6. **Why do circles start to overlap/penetrate each other?**
   - Hint: Think about the order of operations in collision

7. **What happens when you click very fast to add many objects?**
   - Why does the simulation slow down?

## Common Mistakes to Avoid

❌ **Forgetting to multiply by deltaTime**
```cpp
position += velocity;  // WRONG - frame-dependent!
```
✅ **Correct approach**
```cpp
position += velocity * deltaTime;  // RIGHT - frame-independent
```

❌ **Clearing velocity instead of acceleration**
```cpp
velocity = {0, 0};  // WRONG - objects would stop immediately!
```
✅ **Correct approach**
```cpp
acceleration = {0, 0};  // RIGHT - clear forces, keep velocity
```

❌ **Ignoring static objects in collision**
```cpp
// WRONG - tries to move immovable object!
body1.setPosition(...);
body2.setPosition(...);
```
✅ **Correct approach**
```cpp
if (!body1.getIsStatic()) {
    body1.setPosition(...);
}
```

## Next Steps

In **Stage 2 (Newton's Laws)**, we'll explore:
- Newton's First Law (Inertia)
- Newton's Second Law (F = ma)
- Newton's Third Law (Action-Reaction)
- Force accumulation and application
- Why mass matters in collisions

**Preview Question**: If a heavy ball hits a light ball, which one moves more? Why?

## Further Reading

- [Game Loop Pattern](https://gameprogrammingpatterns.com/game-loop.html)
- [Fix Your Timestep](https://gafferongames.com/post/fix_your_timestep/)
- [Euler Integration Explained](https://en.wikipedia.org/wiki/Euler_method)
- [SFML Documentation](https://www.sfml-dev.org/documentation/3.0.0/)

---

**Time Estimate**: 2-3 hours to complete exercises and understand concepts
**Difficulty**: ⭐☆☆☆☆ (Beginner)
