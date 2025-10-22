#include "SpatialGrid.hpp"
#include <algorithm>
#include <cmath>

/**
 * Initialize the spatial grid
 *
 * EXAMPLE with 1200×800 world and 100px cells:
 * - gridWidth = ceil(1200/100) = 12 cells wide
 * - gridHeight = ceil(800/100) = 8 cells tall
 * - Total cells = 12 × 8 = 96 cells
 *
 * MEMORY USAGE:
 * - Each cell is lightweight (just a vector)
 * - Empty cells take minimal memory
 * - Memory scales with world size, not object count
 */
SpatialGrid::SpatialGrid(float worldWidth, float worldHeight, float cellSize)
    : worldWidth(worldWidth), worldHeight(worldHeight), cellSize(cellSize) {
    // Calculate how many cells we need in each dimension
    // Use ceil() to ensure we cover the entire world even if it doesn't divide evenly
    gridWidth = static_cast<int>(std::ceil(worldWidth / cellSize));
    gridHeight = static_cast<int>(std::ceil(worldHeight / cellSize));

    // Pre-allocate all cells (they start empty)
    cells.resize(gridWidth * gridHeight);
}

/**
 * Clear all bodies from all cells
 *
 * PERFORMANCE NOTE:
 * - Called every frame before rebuilding
 * - O(number of cells) operation
 * - Very fast because cells vector stays allocated (no deallocation)
 *
 * WHY REBUILD EVERY FRAME?
 * - Objects are moving constantly
 * - Easier than tracking which cells objects enter/leave
 * - Fast enough that rebuilding is simpler than updating
 */
void SpatialGrid::clear() {
    for (auto& cell : cells) {
        cell.bodies.clear();  // Clear the bodies, keep the vector allocated
    }
}

/**
 * Convert world X coordinate to grid column
 *
 * EXAMPLE: x=250, cellSize=100
 * - cellX = 250/100 = 2 (third column, 0-indexed)
 *
 * CLAMPING:
 * - Ensures objects outside world boundaries don't cause crashes
 * - Objects are forced into edge cells if out of bounds
 */
int SpatialGrid::getCellX(float x) const {
    int cellX = static_cast<int>(x / cellSize);
    return std::clamp(cellX, 0, gridWidth - 1);  // Keep in valid range [0, gridWidth-1]
}

/**
 * Convert world Y coordinate to grid row
 * Same logic as getCellX but for Y axis
 */
int SpatialGrid::getCellY(float y) const {
    int cellY = static_cast<int>(y / cellSize);
    return std::clamp(cellY, 0, gridHeight - 1);  // Keep in valid range [0, gridHeight-1]
}

/**
 * Convert 2D grid coordinates to 1D array index
 *
 * ROW-MAJOR ORDER FORMULA: index = y * width + x
 *
 * VISUAL EXAMPLE (4×3 grid):
 * Grid layout:        Array indices:
 * [0,0][1,0][2,0][3,0]    [0][1][2][3]
 * [0,1][1,1][2,1][3,1]    [4][5][6][7]
 * [0,2][1,2][2,2][3,2]    [8][9][10][11]
 *
 * Cell [2,1] → index = 1 * 4 + 2 = 6
 */
int SpatialGrid::getCellIndex(int cellX, int cellY) const {
    return cellY * gridWidth + cellX;
}

/**
 * Determine which cells a body overlaps
 *
 * BOUNDING BOX APPROACH:
 * - Use AABB (Axis-Aligned Bounding Box) of the circular body
 * - Find min/max cells the box touches
 * - Add all cells in that rectangular region
 *
 * EXAMPLE: Body at (250, 250) with radius 30, cellSize 100
 * - Left edge: 250 - 30 = 220 → cell 2
 * - Right edge: 250 + 30 = 280 → cell 2
 * - Top edge: 250 - 30 = 220 → cell 2
 * - Bottom edge: 250 + 30 = 280 → cell 2
 * - Result: Body only in cell [2,2] (1 cell)
 *
 * EDGE CASE: Body at (295, 250) with radius 30
 * - Left: 265 → cell 2
 * - Right: 325 → cell 3
 * - Result: Body spans cells [2,2] and [3,2] (2 cells)
 *
 * WHY USE BOUNDING BOX?
 * - Simpler than precise circle-cell overlap test
 * - Fast to calculate
 * - Slightly conservative (might include extra cells) but safe
 */
std::vector<int> SpatialGrid::getCellsForBody(RigidBody* body) const {
    std::vector<int> cellIndices;

    sf::Vector2f pos = body->getPosition();
    float radius = body->getRadius();

    // Calculate the bounding box of the body's circle
    // This gives us the rectangular region the body occupies
    int minCellX = getCellX(pos.x - radius);  // Leftmost cell
    int maxCellX = getCellX(pos.x + radius);  // Rightmost cell
    int minCellY = getCellY(pos.y - radius);  // Topmost cell
    int maxCellY = getCellY(pos.y + radius);  // Bottommost cell

    // Insert body into all cells within the bounding box
    // Usually 1-4 cells for typical body sizes
    for (int y = minCellY; y <= maxCellY; ++y) {
        for (int x = minCellX; x <= maxCellX; ++x) {
            cellIndices.push_back(getCellIndex(x, y));
        }
    }

    return cellIndices;
}

/**
 * Insert a body into a specific cell
 * Includes bounds checking to prevent crashes
 */
void SpatialGrid::insertBodyIntoCell(RigidBody* body, int cellX, int cellY) {
    // Bounds check - make sure cell coordinates are valid
    if (cellX >= 0 && cellX < gridWidth && cellY >= 0 && cellY < gridHeight) {
        int index = getCellIndex(cellX, cellY);
        cells[index].bodies.push_back(body);
    }
}

/**
 * Insert a body into all cells it overlaps
 *
 * PERFORMANCE:
 * - O(1) to O(4) typically (most bodies fit in 1-4 cells)
 * - Called once per body per frame
 * - Total: O(n) for n bodies
 */
void SpatialGrid::insert(RigidBody* body) {
    auto cellIndices = getCellsForBody(body);
    for (int index : cellIndices) {
        cells[index].bodies.push_back(body);
    }
}

/**
 * Get all potential collision pairs from the grid
 * This is the BROAD PHASE collision detection
 *
 * ALGORITHM:
 * 1. For each cell, check all pairs of bodies in that cell
 * 2. Use hash set to avoid duplicate pairs (bodies in multiple cells)
 * 3. Return list of unique pairs for narrow-phase testing
 *
 * DUPLICATE PROBLEM:
 * - Body A might be in cells [1,1] and [2,1]
 * - Body B might be in cells [2,1] and [2,2]
 * - Both appear in cell [2,1] → we'd check them
 * - Without deduplication, we'd check the same pair multiple times!
 *
 * HASH FUNCTION:
 * - Combine two pointer addresses into single hash
 * - XOR (^) and bit shift (<<) mix the bits
 * - Order-independent: hash(A,B) == hash(B,A)
 *
 * COMPLEXITY ANALYSIS:
 * - Best case: O(n) when bodies evenly distributed
 * - Worst case: O(n²) if all bodies in one cell (but this is rare!)
 * - Typical case: O(n × k) where k = average bodies per cell (small!)
 */
std::vector<std::pair<RigidBody*, RigidBody*>> SpatialGrid::getPotentialCollisions() {
    std::vector<std::pair<RigidBody*, RigidBody*>> pairs;
    std::unordered_set<size_t> processedPairs;  // Track which pairs we've already added

    // For each cell, check collisions between bodies in the cell
    for (const auto& cell : cells) {
        const auto& bodies = cell.bodies;

        // Check all pairs within this cell using nested loop
        // This is O(k²) where k = bodies in this cell
        for (size_t i = 0; i < bodies.size(); ++i) {
            for (size_t j = i + 1; j < bodies.size(); ++j) {  // j starts at i+1 to avoid checking (A,B) and (B,A)
                RigidBody* body1 = bodies[i];
                RigidBody* body2 = bodies[j];

                // Create a unique hash for this pair
                // Use pointer addresses to create unique identifier
                // Order the pointers to ensure consistent hashing
                size_t hash = 0;
                if (body1 < body2) {
                    hash = reinterpret_cast<size_t>(body1) ^ (reinterpret_cast<size_t>(body2) << 1);
                } else {
                    hash = reinterpret_cast<size_t>(body2) ^ (reinterpret_cast<size_t>(body1) << 1);
                }

                // Only add if we haven't processed this pair yet
                // This prevents duplicate collision checks when bodies share multiple cells
                if (processedPairs.find(hash) == processedPairs.end()) {
                    processedPairs.insert(hash);
                    pairs.emplace_back(body1, body2);
                }
            }
        }
    }

    return pairs;
}
