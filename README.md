
## What is this?

This is a 2D physics simulation where circles bounce around, spin, and collide with each other. Think of it like those desktop physics toys, a newtons cradle

You'll learn:
- Why objects keep moving (Newton's First Law)
- Why heavier things are harder to push (Newton's Second Law)
- Why both objects move when they collide (Newton's Third Law)
- How collisions actually work in games
- Why your game might slow down with too many objects
- How to fix that slowdown (spoiler: it's really clever!)
- How games optimize rendering to stay smooth


## Getting started

### What you need

- Visual Studio 2019 or later (Community edition is free)
- SFML 3.x library - don't worry, we'll help you install it
- Basic understanding of C++ (loops, functions, classes)

### Installation

1. **Clone this repository**
   ```bash
   git clone https://github.com/KZelab/L5GAAM-RigidBody.git
   cd L5GAAM-RigidBody
   ```

2. **Install SFML using vcpkg** (easiest way)
   ```bash
   vcpkg install sfml:x64-windows
   ```

3. **Start at the beginning**
   ```bash
   git checkout 01-baseline
   ```

4. **Open the solution in Visual Studio**
   - Open `AdvancedRigidBodies.sln`
   - Hit F5 to build and run
   - Click around, play with it, break things!

## The learning path

### Stage 1: Baseline (Start here!)
**Branch:** `01-baseline`

This is where you begin. It's a simple simulation with circles falling and bouncing. You'll learn about game loops, deltaTime, and basic physics integration.

Run it, play with it, try changing the gravity slider. See what happens when you add lots of objects.

### Stage 2: Newton's Laws
**Branch:** `02-newtons-laws`

Now we focus on *why* things move the way they do. You'll see Newton's Three Laws in action, Notice anything interesting?

### Stage 3: Collision Basics
**Branch:** `03-collision-basics`

How do games know when things hit each other? How do they bounce apart? You'll learn about collision detection and response 

### Stage 4: Rotation and Torque
**Branch:** `04-rotation-and-torque`

Objects don't just move - they also spin! Hit a ball off-center and watch it rotate. You'll understand angular velocity, torque, and why a figure skater spins faster when they pull their arms in.

### Stage 5: Friction
**Branch:** `05-friction-and-materials`

Why do things slow down when they slide? How do different materials interact? This stage adds friction - the difference between ice and sandpaper.

### Stage 6: Performance Crisis (Important!)
**Branch:** `06-performance-crisis`

Here's where things get interesting. Keep adding objects and watch the simulation get slower... and slower... until it's basically a slideshow. **This is intentional!** 
You're experiencing the O(n²) problem that every game developer encounters. Understanding this problem is crucial before you can solve it.

### Stage 7: Spatial Partitioning
**Branch:** `07-spatial-partitioning`

The hero arrives! This stage shows you how professional games handle thousands of objects without melting your CPU.

### Stage 8: Batched Rendering
**Branch:** `08-batched-rendering`

Your physics are fast now, but the rendering is still slow. This stage teaches you how GPUs work and why reducing "draw calls" matters. It's like the difference between making 100 trips to carry groceries versus using a cart.

### Stage 9: Final Polish
**Branch:** `09-advanced-optimization`
 Sleep states, smart optimizations

## Playing with the simulation

Once you've built and run any stage, here's what you can do:

- **Left Click**: Add an object where you clicked
- **Right Click + Drag**: Grab and throw objects around
- **Space**: Add a bunch of random objects
- **C**: Clear everything (except the static obstacles)
- **G**: Turn gravity on/off (watch objects float!)
- **V**: Show velocity arrows (see which way things are moving)
- **T**: Show motion trails (pretty!)
- **D**: Debug mode (see collision points and normals)

Play around! The best way to learn is to experiment and break things.

## Tips for learning

1. **Don't rush.** Spend time on each stage. Run it, modify values, see what breaks.

2. **Read the comments.** The code is heavily commented with explanations. They're not just "this line does X" - they explain *why* and link to the physics/math.

3. **Experiment.** Change a number. See what happens. That's how you really learn.

4. **Use the visualizations.** Turn on velocity vectors (V key) and debug mode (D key). Actually *see* what the math is doing.

5. **Go backwards sometimes.** After you finish Stage 7 with spatial partitioning, go back to Stage 6. The slowness will make sense now. You'll appreciate the solution more.

6. **Ask questions.** If something doesn't make sense, that's normal! Try changing it and seeing what breaks.

## Going further

Once you've worked through all the stages, you might want to explore:

- Different shapes (boxes, polygons)
- Joints and constraints (ragdolls!)
- Soft body physics (jelly!)
- Fluid simulation
- 3D physics (same concepts, more math)

Check out these resources:
- [Game Programming Patterns](https://gameprogrammingpatterns.com/) - Free online book
- [Box2D](https://box2d.org/) - Professional 2D physics engine
- [Physics for Game Developers](https://www.amazon.com/Physics-Game-Developers-David-Bourg/dp/1449392512) - Great book


---

**Note:** Each branch has its own README with detailed explanations, exercises, and discussion questions. Don't skip those - they're where the real learning happens!
