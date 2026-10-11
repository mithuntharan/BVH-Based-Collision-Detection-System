# BVH for Collision Detection — Architecture & DSA Analysis

## 1. High-Level Architecture

The engine uses a **flat-array node pool** instead of a raw/smart pointer tree:

    struct BVHNode { AABB bounds; int left, right, object_index; };

- All nodes live in one contiguous `std::vector<BVHNode>`.
- `left` / `right` are **indices** (pool offsets), never pointers.
- `object_index >= 0` marks a leaf; interior nodes store child indices.
- **Why:** cache-friendly traversal, no allocation churn, no ownership
  semantics needed, trivially serializable, zero dangling references.

Compare with pointer trees: every node is a separate heap allocation,
pointer-chasing kills cache locality, and memory must be reclaimed
manually or via smart-pointer refcounting ceremony.

## 2. Complexity Analysis

Let N = number of objects.

| Algorithm | Build | Query (broad phase) | Notes |
|-----------|-------|---------------------|-------|
| Median-split BVH | O(N log N) | O(N log N) worst, O(k + log N) typical | k = reported pairs |
| SAH BVH | O(N log N * B) | same as median | B = bins (12) per node |
| Fat AABB refit | O(N) per frame | same as median | rebuild only when escaped |

- **BVH build** partitions space recursively; each level of recursion
  processes each object once → O(N log N) depth is log2(N).
- **BVH query** is an iterative stack traversal: two nodes are tested only
  if their AABBs overlap. Non-overlapping subtrees are skipped entirely,
  which is why candidate pairs collapse from O(N^2) to near-linear in k.
- **Refit** recomputes bounds bottom-up from leaves using the existing
  topology: O(N), no re-sorting, no re-partitioning.

## 3. Trade-offs: Median vs SAH vs Fat AABB Refit

### Median split
Sorts object centroids along the longest axis, splits at the midpoint.
- Cheap: O(N log N) but simple.
- Weakness: equal object counts != equal surface areas; can produce
  long thin boxes with poor culling.

### SAH (Surface Area Heuristic)
Cost = C_trav + (SA_left / SA_parent)*N_left + (SA_right / SA_parent)*N_right
- Uses 12 spatial bins per axis; evaluates split planes between bins.
- Selects the split minimizing *expected query cost*.
- Better tree quality (fewer pairs tested) but ~2-4x slower to build.

### Fat AABBs + lazy refit
- Each object's box is inflated by a margin (+3 px).
- The tree is **refit** (bounds recomputed, topology preserved) each frame.
- A **full rebuild** is triggered only when an object escapes its fat box.
- Wins when objects move slowly relative to their margins: O(N) refit
  beats O(N log N) rebuild almost always.
- Loses (slightly) in query quality: fatter boxes → more false positives.

## 4. Controls

| Input | Action |
|-------|--------|
| B | Cycle collision mode: Median -> SAH -> Refit |
| 1 / 2 / 3 | Directly switch mode: 1=Median, 2=SAH, 3=Refit |
| T | Toggle tree AABB visualization (visual debug layer) |
| Up / Down | Increase / decrease max tree depth drawn |
| SPACE | Pause / resume simulation |
| R | Reset scene (500 new random circles) |
| Left click | Spawn a circle at cursor |
| Right click + drag | Pull nearby circles toward cursor (white guide lines show affected objects) |
| ESC | Quit, print benchmarks to console and write BENCHMARKS.md / EXPLANATION.md |

### Visual debug layer & rendering notes

- **Persistent color assignment:** each circle's RGB is randomized exactly once
  at spawn time (in `makeCircle`) and stored in `Circle::color_r/g/b`. The
  render and update loops never regenerate colors, so base colors are stable.
- **Collision highlight:** on impact, a translucent white *outer halo* is drawn
  around the object for ~2 frames. The base fill is redrawn on top, so the
  highlight never overwrites or replaces the assigned color.
- **Tree overlay (key T):** AABBs are drawn as thin 1px outlines, color-coded
  by depth (L0 green, L1 cyan, L2 yellow, L3 magenta, L4 orange, ...) with
  semi-transparent alpha. When the depth slider (Up/Down) exceeds level 2, leaf
  boxes are skipped so only interior nodes are shown — this keeps the
  space-partition picture readable instead of cluttering the screen.
- **Drag force visualization:** while right-click dragging, faint lines connect
  the cursor to every affected object, with a small crosshair at the cursor.

## 5. Collision Pipeline & Tree Traversal

Every frame executes the BVH broad-phase query to find overlapping AABB pairs.
The iterative dual-tree traversal pushes candidate node pairs onto an internal
stack, culling non-overlapping subtrees early. The detected candidate pairs are
then directly resolved using standard impulse physics.
