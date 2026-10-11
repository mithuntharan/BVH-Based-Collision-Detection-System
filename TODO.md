# BVH Physics Engine - Phase Plan

## Phase 0: CMake skeleton + SDL3 window with fixed-timestep loop
- [x] vcpkg.json pinning SDL3
- [x] CMakeLists.txt with vcpkg toolchain
- [x] main.cpp: SDL3 window, fixed timestep (dt=1/60), ESC to quit
- [x] Build and run verification

## Phase 1: Brute-force collision with ~500 bouncing circles
- [x] Circle struct with position, velocity, radius
- [x] O(n²) broad phase collision detection
- [x] Rendering circles with SDL3
- [x] Live on-screen counters: FPS, objects, pairs tested, collisions found
- [x] Console benchmark at n=100/500/1000 on quit

## Phase 2: BVH implementation
- [x] AABB struct
- [x] Flat-array BVH node pool (min[2], max[2], left, right, object)
- [x] Top-down median-split build
- [x] Iterative stack-based traversal
- [x] CRITICAL: Assert BVH produces IDENTICAL results to brute force every frame

## Phase 3: SAH split + lazy refit
- [x] SAH-based split option
- [x] Lazy refit with fat AABBs
- [x] Toggle for each feature (keys 1/2/3)
- [x] Extended benchmark table

## Phase 4: UI polish
- [x] Randomized object colors + collision flash
- [x] Key toggles: B (method), T (tree), UP/DOWN (depth), SPACE (pause), R (reset)
- [x] Tree bounds visualization with depth color coding
- [x] Left-click spawn, right-click drag force
- [x] BENCHMARKS.md auto-generation
- [x] EXPLANATION.md auto-generation

## Phase 5: Benchmarks documentation
- [x] BENCHMARKS.md with markdown table template