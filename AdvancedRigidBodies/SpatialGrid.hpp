#pragma once
#include <SFML/Graphics.hpp>
#include <vector>
#include <unordered_set>
#include "RigidBody.hpp"

/**
 * SPATIAL PARTITIONING - OPTIMIZATION TECHNIQUE
 * ==============================================
 *
 * PROBLEM: Naive collision detection is O(n²) - checking every object against every other object
 * - With 100 objects: 100 × 99 / 2 = 4,950 checks per frame
 * - With 200 objects: 200 × 199 / 2 = 19,900 checks per frame
 * - This becomes VERY slow as objects increase!
 *
 * SOLUTION: Spatial Grid (also called "Uniform Grid" or "Grid-Based Partitioning")
 * - Divide the world into a grid of cells
 * - Each object is placed into the cells it overlaps
 * - Only check collisions between objects in the same cell or neighboring cells
 *
 * COMPLEXITY IMPROVEMENT:
 * - Old way (brute force): O(n²) - every object vs every other object
 * - New way (spatial grid): O(n × k) where k = average objects per cell
 * - For well-distributed objects: k << n, so this is MUCH faster!
 *
 * REAL-WORLD ANALOGY:
 * Imagine finding two people in a city:
 * - Naive approach: Check if person A is near person B by checking every location in the city
 * - Grid approach: Both people tell you their neighborhood, only check if they're in the same area
 *
 * OTHER SPATIAL PARTITIONING METHODS:
 * - Quadtree: Hierarchical division (good for uneven distribution)
 * - KD-Tree: Binary space partitioning (good for 3D)
 * - Sweep and Prune: Sort objects along axes (good for few moving objects)
 * - Hash Grid: Similar to this but uses hash function for cell lookup
 */
class SpatialGrid {
public:
    /**
     * Constructor - Sets up the grid structure
     *
     * CHOOSING CELL SIZE:
     * - Too small: Objects span many cells, overhead increases
     * - Too large: Too many objects per cell, approaching O(n²) again
     * - Rule of thumb: Cell size ≈ 2-3× average object size
     * - Our choice: 100 pixels works well for bodies sized 10-40 pixels
     */
    SpatialGrid(float worldWidth, float worldHeight, float cellSize);

    /**
     * Clear all bodies from grid (called each frame before re-inserting)
     * Grid structure itself persists - we just empty the cells
     */
    void clear();

    /**
     * Insert a body into all grid cells it overlaps
     *
     * WHY MULTIPLE CELLS?
     * A body's bounding circle might overlap cell boundaries
     * We need to check it against bodies in ALL cells it touches
     */
    void insert(RigidBody* body);

    /**
     * Get all pairs of bodies that MIGHT be colliding
     * This is called "BROAD PHASE" collision detection
     *
     * BROAD PHASE vs NARROW PHASE:
     * - Broad Phase: Quick test to find potential collisions (this function)
     * - Narrow Phase: Precise test to confirm actual collision (distance check)
     *
     * Returns pairs of bodies in same cells - still need to check exact distance!
     */
    std::vector<std::pair<RigidBody*, RigidBody*>> getPotentialCollisions();

    // Helper functions to convert world coordinates to grid coordinates
    int getCellX(float x) const;  // Which column is this X position in?
    int getCellY(float y) const;  // Which row is this Y position in?
    int getCellIndex(int cellX, int cellY) const;  // Convert 2D grid coords to 1D array index

private:
    /**
     * A single cell in the grid
     * Contains pointers to all bodies currently overlapping this cell
     *
     * NOTE: Using raw pointers here is safe because:
     * - Bodies are owned by PhysicsEngine (unique_ptr)
     * - Grid is rebuilt every frame (no dangling pointer risk)
     * - Grid lifetime < Body lifetime
     */
    struct Cell {
        std::vector<RigidBody*> bodies;
    };

    // Grid dimensions and configuration
    float worldWidth;   // Total world width in pixels
    float worldHeight;  // Total world height in pixels
    float cellSize;     // Size of each cell (100 pixels in our case)
    int gridWidth;      // Number of cells horizontally
    int gridHeight;     // Number of cells vertically

    /**
     * 1D array storing all cells
     *
     * WHY 1D ARRAY instead of 2D?
     * - Better cache locality (cells stored contiguously in memory)
     * - Simpler memory allocation (one allocation vs many)
     * - Faster access (one lookup vs two)
     *
     * Conversion: index = y * gridWidth + x
     * This is called "row-major order" - common in graphics/game dev
     */
    std::vector<Cell> cells;

    // Helper methods
    void insertBodyIntoCell(RigidBody* body, int cellX, int cellY);
    std::vector<int> getCellsForBody(RigidBody* body) const;
};
