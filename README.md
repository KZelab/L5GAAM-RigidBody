# Stage 6: Batched Rendering

## Learning Objectives

By the end of this stage, you will understand:
- ✅ Why individual draw calls are expensive
- ✅ What GPU batching is and why it matters
- ✅ How vertex arrays reduce CPU-GPU communication
- ✅ The difference between immediate and batched rendering
- ✅ When to optimize rendering vs physics

## What's New in This Stage

Stage 5 fixed physics performance with spatial partitioning. But with many objects, **rendering** becomes the bottleneck! Each circle, glow, and trail was drawn separately - hundreds of GPU commands per frame.

**Key Additions**:
- **Batched glow rendering**: All glows in one draw call
- **Batched trail rendering**: All trails in one draw call
- Vertex arrays for efficient GPU communication
- 50-100× fewer draw calls!
- Smoother rendering with 500+ objects

## The Problem: Draw Call Overhead

### What is a Draw Call?

Every time you tell the GPU to draw something:
```cpp
window.draw(shape);  // One draw call
```

**What happens**:
1. CPU packages rendering data
2. Sends data to GPU (slow!)
3. GPU processes and draws
4. **Repeat for every object...**

### Why Many Draw Calls Are Slow

**Stage 5 rendering**:
```cpp
for (auto& body : bodies) {
    body.draw(window);  // Draw main circle
    // Inside RigidBody::draw():
    window.draw(glowShape);      // Draw call #1
    window.draw(mainShape);      // Draw call #2
    window.draw(coreShape);      // Draw call #3
    window.draw(rotationLine);   // Draw call #4
}
```

With 200 objects:
- 200 × 4 = **800 draw calls** just for circles!
- Plus trails, particles, UI...
- Total: **1000+ draw calls per frame**

**The bottleneck**:
- Each draw call takes ~0.01ms
- 1000 calls × 0.01ms = **10ms**
- Only 16.67ms available per frame at 60 FPS!
- **60% of frame time wasted on draw calls!**

## The Solution: Batched Rendering

### Vertex Arrays

**sf::VertexArray** lets you draw many shapes in one call:

```cpp
sf::VertexArray vertices(sf::Quads, 800);  // 200 quads × 4 vertices

// Fill vertex data for all objects
for (int i = 0; i < 200; i++) {
    vertices[i*4 + 0] = /* top-left */;
    vertices[i*4 + 1] = /* top-right */;
    vertices[i*4 + 2] = /* bottom-right */;
    vertices[i*4 + 3] = /* bottom-left */;
}

window.draw(vertices);  // ONE draw call for all 200 objects!
```

### Performance Comparison

| Approach | Draw Calls | Time |
|----------|-----------|------|
| Individual (Stage 5) | 1000+ | 10ms |
| Batched (Stage 6) | ~10 | 0.1ms |
| **Speedup** | **100×** | **100×** |

## How Batching Works

### 1. Collect Geometry

Instead of drawing immediately, **collect vertices**:

```cpp
// Old way - immediate
window.draw(glowCircle);  // Draw call!

// New way - batched
glowVertices.append(Vertex(pos1, color1));
glowVertices.append(Vertex(pos2, color2));
glowVertices.append(Vertex(pos3, color3));
glowVertices.append(Vertex(pos4, color4));
// ... collect all vertices first
```

### 2. Draw Once

After collecting all vertices:
```cpp
window.draw(glowVertices);  // ONE draw call for ALL glows!
```

### 3. GPU Processes Batch

GPU receives all data at once:
- More efficient data transfer
- Better GPU parallelization
- Less CPU-GPU synchronization

## Code Changes from Stage 5

### 1. PhysicsEngine - Vertex Arrays (PhysicsEngine.hpp:38-39)

New member variables for batching:
```cpp
// Vertex arrays for batched rendering
sf::VertexArray glowVertices;
sf::VertexArray trailVertices;
```

### 2. Batched Glow Rendering (PhysicsEngine.cpp)

**Before (Stage 5)** - Individual draws:
```cpp
// Inside RigidBody::draw()
sf::CircleShape glow(radius * 1.5f);
glow.setFillColor(glowColor);
window.draw(glow);  // Draw call per object!
```

**After (Stage 6)** - Batched:
```cpp
void PhysicsEngine::drawBatchedGlows(sf::RenderWindow& window) {
    glowVertices.clear();
    glowVertices.setPrimitiveType(sf::Quads);

    for (auto& body : bodies) {
        // Create quad (4 vertices) for glow
        sf::Vector2f pos = body->getPosition();
        float r = body->getRadius() * 1.5f;

        glowVertices.append(Vertex(pos + Vector2f(-r, -r), glowColor));
        glowVertices.append(Vertex(pos + Vector2f(+r, -r), glowColor));
        glowVertices.append(Vertex(pos + Vector2f(+r, +r), glowColor));
        glowVertices.append(Vertex(pos + Vector2f(-r, +r), glowColor));
    }

    window.draw(glowVertices);  // ONE draw call!
}
```

### 3. Batched Trail Rendering (PhysicsEngine.cpp)

**Before (Stage 5)** - Individual lines:
```cpp
// Inside RigidBody::drawMotionTrail()
for (size_t i = 1; i < trail.size(); i++) {
    sf::Vertex line[2] = {
        Vertex(trail[i-1].pos, color),
        Vertex(trail[i].pos, color)
    };
    window.draw(line, 2, sf::Lines);  // Draw call per trail segment!
}
```

**After (Stage 6)** - Batched:
```cpp
void PhysicsEngine::drawBatchedTrails(sf::RenderWindow& window) {
    trailVertices.clear();
    trailVertices.setPrimitiveType(sf::Lines);

    for (auto& body : bodies) {
        auto& trail = body->getMotionTrail();
        for (size_t i = 1; i < trail.size(); i++) {
            trailVertices.append(Vertex(trail[i-1].pos, color));
            trailVertices.append(Vertex(trail[i].pos, color));
        }
    }

    window.draw(trailVertices);  // ONE draw call for all trails!
}
```

### 4. RigidBody::draw() Simplified (RigidBody.cpp:125)

Comment added explaining the change:
```cpp
// Glows are now drawn in batched mode by PhysicsEngine
```

Main circles still drawn individually (complex shapes harder to batch).

## What Gets Batched?

### ✅ Batched in This Stage
- **Glows**: Simple quads, easy to batch
- **Motion trails**: Lines, perfect for batching
- **Particles**: Already batched in ParticleSystem

### ❌ Not Batched
- **Main circles**: Complex (need textures, outlines)
- **UI elements**: Different rendering state
- **Debug visualizations**: Rare, not worth optimizing

## Observing Rendering Performance

**What to Try**:

1. **Press T to enable trails**:
   - Stage 5: Significant FPS drop with many objects
   - Stage 6: Minimal FPS impact!

2. **Add 500+ objects**:
   - Stage 5: Rendering becomes slow
   - Stage 6: Smooth rendering

3. **Profile with debug tools**:
   - Measure draw calls (GPU profiler)
   - See dramatic reduction

## When to Batch?

### Good Candidates for Batching

✅ **Many similar objects**:
- Particles (thousands of dots)
- Bullets/projectiles
- Grass blades/trees
- UI icons

✅ **Simple geometry**:
- Lines, points, quads
- Same texture/material
- No complex state changes

### Poor Candidates

❌ **Few objects**: Overhead not worth it
❌ **Complex state**: Different textures, shaders
❌ **Dynamic topology**: Geometry changes every frame

## Discussion Questions

1. **Why aren't main circles batched?**
   - What makes them harder to batch than glows?

2. **What's the trade-off with batching?**
   - More complex code, memory usage
   - When is it worth it?

3. **Could we batch trails differently?**
   - Triangle strips vs individual lines?
   - What about thick lines?

4. **Why clear vertex arrays each frame?**
   - Why not reuse the same vertices?

5. **What happens if objects have different textures?**
   - Can they be batched together?
   - How do texture atlases help?

6. **How does batching relate to spatial partitioning?**
   - Both are optimizations - but for what?

7. **What's the memory cost of vertex arrays?**
   - 200 objects × 4 vertices × 20 bytes = ?
   - Is this acceptable?

## Rendering Pipeline Overview

### CPU Side
```
1. Physics Update (Stage 5 optimized this!)
2. Collect Vertices (batching)
3. Send to GPU
```

### GPU Side
```
4. Vertex Processing
5. Rasterization
6. Fragment Shading
7. Display
```

**Where batching helps**: Step 3 (CPU-GPU communication)

## Advanced Batching Techniques

### 1. Instanced Rendering
Draw same mesh multiple times with different positions:
```cpp
// One draw call for 1000 identical trees!
glDrawInstanced(treeMesh, 1000);
```

### 2. Texture Atlases
Combine textures to avoid state changes:
```
+--------+--------+
| Tree   | Rock   |
+--------+--------+
| Grass  | Flower |
+--------+--------+
```
All sprites use same texture → can batch!

### 3. Geometry Instancing
Store transforms in buffer, not vertex data:
- Less data transferred
- GPU applies transforms
- Used in modern engines

### 4. Indirect Drawing
GPU decides what to draw:
- CPU builds command buffer
- GPU executes without CPU sync
- Ultimate batching!

## Real-World Applications

**Game Engines**:
- Unity: Dynamic batching, static batching
- Unreal: Instanced static meshes
- Godot: MultiMesh nodes

**Particle Systems**:
- Thousands of particles in one draw call
- Critical for explosions, rain, snow

**UI Rendering**:
- ImGui: Batches all UI in vertex buffers
- Web browsers: Batch text, rectangles

**Terrain Rendering**:
- Thousands of grass instances
- Tree billboards
- Rock meshes

## Performance Metrics

**Before (Stage 5)**:
```
Physics: 2ms (optimized)
Rendering: 10ms (bottleneck!)
Total: 12ms (83 FPS)
```

**After (Stage 6)**:
```
Physics: 2ms (same)
Rendering: 0.5ms (optimized!)
Total: 2.5ms (400 FPS)
```

**Result**: Rendering no longer bottleneck!

## Key Concepts Reference

```cpp
// Draw call - CPU to GPU command
window.draw(shape);  // One draw call

// Vertex array - batch many shapes
sf::VertexArray vertices(sf::Quads, count * 4);
window.draw(vertices);  // Still one draw call!

// Primitive types
sf::Points       // Individual dots
sf::Lines        // Pairs of vertices (trails)
sf::LineStrip    // Connected lines
sf::Triangles    // 3 vertices per shape
sf::Quads        // 4 vertices per shape (glows)

// Vertex structure
sf::Vertex(position, color, texCoords);
```

## Further Reading

- [Batching Explained](https://docs.unity3d.com/Manual/DrawCallBatching.html)
- [GPU Performance Best Practices](https://developer.nvidia.com/gpugems/gpugems/part-v-performance-and-practicalities)
- [Vertex Arrays in SFML](https://www.sfml-dev.org/tutorials/2.5/graphics-vertex-array.php)
- [Reducing Draw Calls](https://learn.microsoft.com/en-us/windows/win32/direct3d11/overviews-direct3d-11-render-multi-thread-render)

---
