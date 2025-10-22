# Stage 5: Spatial Partitioning

## Learning Objectives

By the end of this stage, you will understand:
- ✅ Why naive collision detection becomes slow (O(n²) complexity)
- ✅ What spatial partitioning is and why it's essential for games
- ✅ How a uniform grid accelerates collision detection
- ✅ Trade-offs in choosing grid cell size
- ✅ The difference between O(n²) and O(n) performance

## What's New in This Stage

Stage 4 demonstrated the "performance crisis" - with 200+ objects, the simulation grinds to a halt. This is because we checked every object against every other object (O(n²) complexity).

**Key Additions**:
- **SpatialGrid class**: Divides world into cells
- Objects only check collisions within nearby cells
- Dramatic performance improvement (10-100× faster!)
- Can now handle 500+ objects smoothly
- Grid visualization (press G to see cells)

## The Problem: O(n²) Complexity

### Naive Collision Detection

```cpp
// Stage 1-4 approach: Check EVERY pair
for (i = 0; i < bodies.size(); i++) {
    for (j = i + 1; j < bodies.size(); j++) {
        checkCollision(bodies[i], bodies[j]);
    }
}
```

**Number of checks**:
- 10 objects: 45 checks
- 50 objects: 1,225 checks
- 100 objects: 4,950 checks
- 200 objects: 19,900 checks ← Performance crisis!
- 500 objects: 124,750 checks ← Impossible!

This is **O(n²)** - doubling objects quadruples the work!

### Why This Matters

At 60 FPS, each frame has only **16.67 milliseconds**:
- 100 objects × 4,950 checks = Fast enough
- 200 objects × 19,900 checks = Starts lagging
- 500 objects × 124,750 checks = Slideshow!

**Real-world games** have thousands of objects, so O(n²) is unacceptable.

## The Solution: Spatial Partitioning

### Uniform Grid Concept

Divide the world into a grid of cells:

```
+-----+-----+-----+-----+
|  O  |     |  O  |     |
|     |  O  |     |     |
+-----+-----+-----+-----+
|     |     | OO  |  O  |
|  O  |     |  O  |     |
+-----+-----+-----+-----+
```

**Key Insight**: Objects can only collide if they're in the same cell or neighboring cells!

### Algorithm Steps

1. **Clear grid** (each frame)
2. **Insert all objects** into their cell(s)
3. **For each object**, only check collisions with objects in:
   - Same cell
   - 8 neighboring cells (in 2D)
4. **Profit!** Far fewer checks needed

### Complexity Analysis

**Old way**:
- Check all pairs: O(n²)
- 200 objects = 19,900 checks

**New way**:
- Check n objects against k neighbors each: O(n × k)
- With good distribution: k ≈ 10-20 (constant!)
- 200 objects × 15 average = 3,000 checks
- **~6× fewer checks!**

### Real-World Analogy

**Finding two people in a city**:

**Naive approach** (O(n²)):
- Check if person A is near person B
- Must check every location × every location
- Extremely slow!

**Grid approach** (O(n)):
- A says "I'm in downtown"
- B says "I'm in suburbs"
- Different neighborhoods → no collision, skip!
- Same neighborhood → check only within that area

## Code Changes from Stage 4

### 1. New File: SpatialGrid.hpp/cpp

Entirely new class managing the grid structure:

```cpp
class SpatialGrid {
    SpatialGrid(float worldWidth, float worldHeight, float cellSize);

    void clear();  // Empty all cells
    void insert(RigidBody* body);  // Add object to relevant cells
    std::vector<std::pair<RigidBody*, RigidBody*>> getPotentialCollisions();
};
```

**Key parameters**:
- Cell size: 100 pixels (2-3× object size)
- Grid dimensions: Calculated from world size

### 2. PhysicsEngine Modifications

**Before (Stage 4)** - Naive O(n²):
```cpp
for (size_t i = 0; i < bodies.size(); ++i) {
    for (size_t j = i + 1; j < bodies.size(); ++j) {
        checkCollision(bodies[i], bodies[j]);  // EVERY pair!
    }
}
```

**After (Stage 5)** - Grid-optimized O(n):
```cpp
spatialGrid.clear();
for (auto& body : bodies) {
    spatialGrid.insert(&body);  // Insert into grid
}

auto pairs = spatialGrid.getPotentialCollisions();  // Only nearby pairs!
for (auto [a, b] : pairs) {
    checkCollision(*a, *b);
}
```

### 3. Grid Cell Size Selection

Choosing cell size is critical:

**Too small** (e.g., 10 pixels):
- Objects overlap many cells
- More insertion overhead
- More duplicate checks

**Too large** (e.g., 1000 pixels):
- All objects in same cell
- Back to O(n²)!
- No benefit

**Just right** (100 pixels):
- 2-3× average object size
- Objects mostly in 1-4 cells
- Good balance

### 4. Cell Coordinate Calculation (SpatialGrid.cpp)

```cpp
int getCellX(float x) {
    return static_cast<int>(x / cellSize);
}

int getCellY(float y) {
    return static_cast<int>(y / cellSize);
}
```

Maps world position to grid coordinates.

### 5. Neighbor Cell Checking

For each object, check collisions with 9 cells total:
```
+---+---+---+
| ↖ | ↑ | ↗ |
+---+---+---+
| ← | O | → |
+---+---+---+
| ↙ | ↓ | ↘ |
+---+---+---+
```

Only 9 cells instead of entire world!

## Observing Performance Improvement

**What to Try**:

1. **Press Space repeatedly** to add many objects:
   - Stage 4: Stutters with 150+ objects
   - Stage 5: Smooth with 500+ objects!

2. **Press G** to visualize grid:
   - See how objects distribute across cells
   - Notice cells with more objects (denser areas)

3. **Watch FPS counter**:
   - Stage 4: FPS drops as objects increase
   - Stage 5: FPS remains high even with many objects

4. **Create clusters**:
   - Click repeatedly in one area
   - Local density increases
   - Still handles well (local O(n²) but small n per cell)

## Discussion Questions

1. **Why is the grid cleared and rebuilt every frame?**
   - Could we update incrementally as objects move?
   - What would be the trade-offs?

2. **What happens if all objects cluster in one cell?**
   - Does spatial partitioning still help?
   - Hint: Consider worst-case vs average-case

3. **Why check 9 cells (including diagonals)?**
   - Could an object in cell (0,0) collide with one in cell (2,2)?
   - Why or why not?

4. **How would you choose cell size for different scenarios?**
   - Tiny bullets and huge planets?
   - All same-sized objects?

5. **What's the memory cost of spatial partitioning?**
   - How many grid cells are created?
   - Is this acceptable?

6. **Could we use a different data structure?**
   - Quadtree? Octree? Hash grid?
   - When would each be better?

7. **Why is this still O(n) not O(1)?**
   - We still process every object!
   - What makes it faster than O(n²)?

## Performance Comparison

| Metric | Stage 4 (Naive) | Stage 5 (Grid) | Improvement |
|--------|-----------------|----------------|-------------|
| 50 objects | 1,225 checks | ~750 checks | 1.6× |
| 100 objects | 4,950 checks | ~1,500 checks | 3.3× |
| 200 objects | 19,900 checks | ~3,000 checks | **6.6×** |
| 500 objects | 124,750 checks | ~7,500 checks | **16.6×** |

Numbers are approximate - actual performance depends on object distribution.

## Other Spatial Partitioning Methods

### 1. Quadtree
**How it works**: Recursively divide space into 4 quadrants
**Best for**: Uneven object distribution
**Used in**: Many 2D games, GIS systems

### 2. Octree (3D)
**How it works**: Like quadtree but 8 octants (3D)
**Best for**: 3D games, 3D modeling
**Used in**: Minecraft, collision detection in 3D engines

### 3. K-D Tree
**How it works**: Binary space partitioning along alternating axes
**Best for**: Static scenes, ray tracing
**Used in**: Photon mapping, nearest-neighbor searches

### 4. Sweep and Prune
**How it works**: Sort objects along axes, detect overlaps
**Best for**: Few moving objects, 1D or 2D
**Used in**: Some physics engines

### 5. Hash Grid
**How it works**: Like our grid but uses hash table
**Best for**: Sparse worlds (mostly empty)
**Used in**: Large open-world games

## Real-World Applications

**Video Games**:
- Minecraft: Chunks (spatial grid for blocks)
- GTA: Spatial hash for pedestrians/vehicles
- Starcraft: Grid for unit selection/collision

**Simulation**:
- Particle systems (thousands of particles)
- Crowd simulation (avoid O(n²) between people)
- Molecular dynamics (atoms/molecules)

**Graphics**:
- Ray tracing acceleration (BVH, k-d tree)
- Frustum culling (only render visible objects)
- Level of detail (LOD) systems

## Key Formulas Reference

```cpp
// Complexity
Naive: O(n²) - checks = n × (n-1) / 2
Grid:  O(n × k) - where k = objects per cell (usually constant)

// Cell coordinates
cell_x = floor(position.x / cell_size)
cell_y = floor(position.y / cell_size)

// Number of cells
cells_x = ceil(world_width / cell_size)
cells_y = ceil(world_height / cell_size)
total_cells = cells_x × cells_y

// Optimal cell size (rule of thumb)
cell_size ≈ 2-3 × average_object_radius
```

## Further Reading

- [Spatial Partitioning in Games](https://gameprogrammingpatterns.com/spatial-partition.html)
- [Quadtrees and Octrees](https://en.wikipedia.org/wiki/Quadtree)
- [Broad Phase Collision Detection](https://www.toptal.com/game/video-game-physics-part-ii-collision-detection-for-solid-objects)
- [Big O Notation Explained](https://www.bigocheatsheet.com/)

---
