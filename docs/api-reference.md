# Public API Reference: BVH Collision & Benchmark Suite

This document provides complete technical specifications, function signatures, parameter descriptions, return values, algorithmic complexities, and usage examples for all public APIs in the BVH collision detection engine and benchmarking harness.

---

## 1. Core Collision & Spatial Partitioning API (`src/collision.h`)

### Data Structures

#### `struct AABB`
Axis-Aligned Bounding Box in 2D Euclidean space.
```cpp
struct AABB {
    float min[2]; // [min_x, min_y]
    float max[2]; // [max_x, max_y]
};
```
* **Memory Footprint**: 16 bytes.
* **Invariants**: `min[0] <= max[0]` and `min[1] <= max[1]`.

---

#### `struct BVHNode`
Contiguous flat-array tree node.
```cpp
struct BVHNode {
    AABB bounds;        // Bounding volume enclosing all descendants or the single leaf object
    int left = -1;      // Vector index of left child (-1 if leaf)
    int right = -1;     // Vector index of right child (-1 if leaf)
    int object_index = -1; // Index in circles vector if leaf (>= 0); -1 if interior node
};
```
* **Memory Footprint**: 28 bytes (aligned to 32 bytes in vector).
* **Cache Locality**: Stored in a single flat vector `std::vector<BVHNode>`, eliminating pointer overhead.

---

#### `struct Circle`
2D dynamic circular rigid body.
```cpp
struct Circle {
    float x, y;                     // Centroid position (pixels)
    float vx, vy;                   // Velocity vector (pixels/second)
    float r;                        // Radius (pixels)
    uint8_t r_col, g_col, b_col;    // Rendering base RGB color
    uint8_t flash_timer = 0;        // Collision highlight halo timer
    float mass = 1.0f;              // Inertial mass
    float invMass = 1.0f;           // Reciprocal mass (1/mass, 0 for static)
    float restitution = 1.0f;       // Elastic coefficient e in [0, 1]
    float staticFriction = 0.35f;   // Coulomb static friction coefficient
    float dynamicFriction = 0.25f;  // Coulomb dynamic friction coefficient
};
```

---

#### `struct Pair`
Candidate collision pair emitted by the broad phase.
```cpp
struct Pair {
    int a, b; // Indices of colliding circles (normalized so a < b)
    bool operator<(const Pair& other) const;
    bool operator==(const Pair& other) const;
};
```

---

#### `struct BVHStats`
Runtime instrumentation metrics populated by BVH passes.
```cpp
struct BVHStats {
    int nodes_used = 0;             // Total active nodes in the tree
    int pairs_tested = 0;           // Bounding volume overlap tests performed
    int collisions_found = 0;       // Number of overlapping candidate pairs emitted
    double build_time_us = 0.0;     // Tree construction duration in microseconds
    double query_time_us = 0.0;     // Traversal query duration in microseconds
    double refit_time_us = 0.0;     // Bottom-up refit duration in microseconds
    int rebuilds_triggered = 0;     // Number of full topology rebuilds triggered
};
```

---

### Functions

#### `buildBVH`
Constructs a complete Bounding Volume Hierarchy from scratch over a set of circles.
```cpp
void buildBVH(const std::vector<Circle>& circles,
              std::vector<BVHNode>& nodes,
              std::vector<int>& object_indices,
              BVHStats& stats,
              BuildMethod method = BuildMethod::Median,
              float fat_margin = 0.0f);
```
* **Parameters**:
  * `circles`: Input array of circular bodies.
  * `nodes`: Output flat vector storing all allocated tree nodes. Pre-reserved to $2N$ capacity.
  * `object_indices`: Scratch permutation index buffer tracking spatial partitioning order.
  * `stats`: Output statistics struct populated with `build_time_us` and `nodes_used`.
  * `method`: `BuildMethod::Median` (top-down centroid median split) or `BuildMethod::SAH` (12-bin Surface Area Heuristic).
  * `fat_margin`: Padding margin added to leaf AABBs (e.g., `3.0f` for lazy refitting; `0.0f` for tight bounds).
* **Time Complexity**:
  * `BuildMethod::Median`: $O(N \log N)$ average and worst case.
  * `BuildMethod::SAH`: $O(B \cdot N \log N)$ where $B = 12$ spatial bins.
* **Space Complexity**: $O(N)$ contiguous flat memory ($2N - 1$ nodes).

**Example**:
```cpp
std::vector<Circle> bodies = /* ... */;
std::vector<BVHNode> nodes;
std::vector<int> indices;
BVHStats stats;

buildBVH(bodies, nodes, indices, stats, BuildMethod::Median, 0.0f);
std::cout << "Built BVH with " << stats.nodes_used << " nodes in " 
          << stats.build_time_us << " us\n";
```

---

#### `refitBVH`
Recomputes bounding box bounds bottom-up preserving existing tree topology.
```cpp
void refitBVH(const std::vector<Circle>& circles,
              std::vector<BVHNode>& nodes,
              const std::vector<int>& object_indices,
              BVHStats& stats,
              float fat_margin = 0.0f);
```
* **Parameters**:
  * `circles`: Updated array of moving circles.
  * `nodes`: Existing flat vector of BVH nodes whose bounds will be updated in-place.
  * `object_indices`: Permutation buffer matching tree leaf mapping.
  * `stats`: Output statistics populated with `refit_time_us`.
  * `fat_margin`: Fat margin added to leaf bounding boxes.
* **Time Complexity**: Strict $O(N)$ (single post-order bottom-up pass).
* **Precondition**: `nodes` must contain a valid previously constructed tree.

---

#### `needsRebuild`
Evaluates whether any moving circle has escaped its leaf's fat bounding box.
```cpp
bool needsRebuild(const std::vector<Circle>& circles,
                  const std::vector<BVHNode>& nodes,
                  const std::vector<int>& object_indices,
                  float fat_margin);
```
* **Returns**: `true` if any circle's tight AABB breaches the bounds stored in its leaf node; `false` if bounds remain valid for refitting.
* **Time Complexity**: $O(N)$ scalar scan.

---

#### `queryBVH`
Executes broad-phase dual-tree self-traversal to find all overlapping candidate pairs.
```cpp
void queryBVH(const std::vector<BVHNode>& nodes,
              const std::vector<Circle>& circles,
              std::vector<Pair>& out_pairs,
              BVHStats& stats);
```
* **Parameters**:
  * `nodes`: Flat vector of BVH nodes.
  * `circles`: Array of circular bodies.
  * `out_pairs`: Output vector populated with unique candidate colliding index pairs `(a, b)` with `a < b`.
  * `stats`: Output statistics populated with `query_time_us`, `pairs_tested`, and `collisions_found`.
* **Time Complexity**: $O(k + \log N)$ average case, where $k = |\text{out\_pairs}|$. Worst case $O(N^2)$ if all objects completely overlap in the same point.
* **Space Complexity**: $O(\text{Tree Depth})$ stack auxiliary space (typically $< 32$ entries).

---

## 2. Physics & Contact Dynamics API (`src/physics.h`)

#### `resolvePhysicsCollision`
Narrow-phase impulse resolution with Newtonian restitution and tangential Coulomb friction.
```cpp
void resolvePhysicsCollision(Circle& a,
                             Circle& b,
                             const PhysicsConfig& config,
                             PhysicsAudit& audit);
```
* **Parameters**:
  * `a`, `b`: The two intersecting circular bodies modified in-place (velocities and positions updated).
  * `config`: Physics configuration controlling restitution coefficient $e$, friction toggle, and gravity.
  * `audit`: Quantitative auditor tracking kinetic energy dissipation and collision count.
* **Time Complexity**: $O(1)$.
* **Mechanics**:
  1. Circle distance check: $\Delta d < r_a + r_b$.
  2. Normal impulse: $j_n = -(1 + e)(\vec{v}_{rel} \cdot \hat{n}) / (m_a^{-1} + m_b^{-1})$.
  3. Coulomb friction: $|j_t| \le \mu_s j_n$ (static) or $-\mu_k j_n$ (dynamic slip).
  4. Baumgarte linear projection: position correction to eliminate overlap penetration.

---

#### `updatePhysicsKinematics`
Integrates equations of motion using Symplectic Euler integration and evaluates boundary walls.
```cpp
void updatePhysicsKinematics(std::vector<Circle>& circles,
                             float dt,
                             float windowW,
                             float windowH,
                             const PhysicsConfig& config);
```
* **Time Complexity**: $O(N)$ sequential pass.

---

## 3. Benchmarking & Analytics API (`benchmarks/include/`)

#### `BenchmarkUtils::queryBruteForce`
Canonical $O(N^2)$ all-pairs broad-phase baseline.
```cpp
void BenchmarkUtils::queryBruteForce(const std::vector<Circle>& circles,
                                     std::vector<Pair>& out_pairs,
                                     int& pairs_tested);
```
* **Guarantees**: Tests all $\frac{N(N-1)}{2}$ pairs using tight circle AABB overlap, outputting candidate pairs with $i < j$.

---

#### `BenchmarkUtils::verifyCorrectness`
Asserts bitwise pair-for-pair identity between BVH broad phase and brute-force broad phase.
```cpp
bool BenchmarkUtils::verifyCorrectness(const std::vector<Pair>& bvh_pairs,
                                       const std::vector<Pair>& brute_pairs,
                                       std::string& err_msg);
```
* **Returns**: `true` if sorted candidate sets match 100%; `false` with detailed mismatch diagnostic if any pair differs.

---

#### `SceneGenerator::generateScene`
Generates deterministic synthetic test configurations with fixed random seeds.
```cpp
std::vector<Circle> SceneGenerator::generateScene(const BenchmarkConfig& config,
                                                  uint32_t run_seed);
```
* **Supported Distributions**:
  * `SceneDistribution::Uniform`: Uniform random distribution across arena.
  * `SceneDistribution::Clustered`: 6 Gaussian cluster centroids.
  * `SceneDistribution::DenseOverlap`: Objects tightly confined to central 30% canvas region.
  * `SceneDistribution::Sparse`: Dispersed over a 9x expanded coordinate arena.
  * `SceneDistribution::Dynamic`: Initialized with randomized physical velocities.
