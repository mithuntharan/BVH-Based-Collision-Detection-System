# Bounding Volume Hierarchy (BVH) & Contact Physics Simulation System
### High-Performance Spatial Partitioning & Collision Dynamics Suite (DSA Case Study)

[![C++20](https://img.shields.io/badge/Language-C%2B%2B20-blue.svg)](https://en.cppreference.com/w/cpp/20)
[![Physics](https://img.shields.io/badge/Physics-Impulse%20%2B%20Coulomb%20Friction-emerald.svg)](#physics-engine)
[![Complexity](https://img.shields.io/badge/Broad%20Phase-O(N%20log%20N)%20vs%20O(N%C2%B2)-cyan.svg)](#complexity-analysis)
[![License](https://img.shields.io/badge/License-MIT-purple.svg)](LICENSE)

An industrial-grade, portfolio-ready 2D collision detection and impulse physics engine showcasing **Bounding Volume Hierarchy (BVH)** spatial partitioning. It delivers **real-time spatial subdivision**, **broad-phase subtree culling**, and **contact dynamics** in a native C++20 / SDL3 application.

---

## 🌟 Highlights & Key Features

- **Algorithmic Spatial Culling**: Cuts broad-phase collision tests from naive $O(N^2)$ down to $O(k + \log N)$ using hierarchical tree traversal.
- **Flat-Array Cache-Friendly Node Pool**: Nodes reside in a contiguous `std::vector<BVHNode>` indexed by 32-bit integer offsets, eliminating pointer chasing and allocation churn.
- **Multiple Partitioning Strategies**:
  - **Median-Split BVH**: Partitions object centroids along the longest bounding box axis ($O(N \log N)$ build).
  - **Surface Area Heuristic (SAH)**: 12-bin spatial heuristic minimizing expected query cost.
  - **Fat AABB Lazy Refit**: Inflated margins with bottom-up $O(N)$ tree refitting, rebuilding topology only when objects escape margins.
- **Contact Physics & Conservation Dynamics**:
  - Impulse-based momentum exchange with configurable restitution $e \in [0.0, 1.0]$ (Elastic, Inelastic, Partially Elastic).
  - Tangential Coulomb surface friction with static and dynamic impulse clamping.
  - Baumgarte positional de-penetration slop.
  - Quantitative conservation auditor computing Kinetic Energy ($\sum \frac{1}{2}mv^2$), Momentum ($|P|$), and dissipated energy.

---

## ⚡ Complexity & Benchmark Analysis

Let $N$ denote the number of active bodies in the simulation:

| Phase / Method | Theoretical Complexity | Empirical Pruning @ $N=500$ | Memory / Cache Locality |
| :--- | :--- | :--- | :--- |
| **Naive All-Pairs Broad Phase** | $O(N^2)$ | $0\%$ (124,750 pair checks) | Sequential loop |
| **Median-Split BVH** | $O(N \log N)$ build, $O(k + \log N)$ query | **$>88\%$ reduction** | Flat-array contiguous pool |
| **SAH BVH** | $O(N \log N \cdot B)$ build, $O(k + \log N)$ query | **$>92\%$ reduction** | Flat-array contiguous pool |
| **Fat AABB Refit** | $O(N)$ refit per frame | Same query quality as Median | Zero heap allocation churn |

### Quantitative Performance Measurements

*Empirical results measured over 300 frames per configuration ($dt = 1/60\text{ s}$):*

| Body Count ($N$) | Method | Build / Refit Time | Broad-Phase Query | Pairs Tested | Naive Pairs $O(N^2)$ | Pruning Efficiency |
| :---: | :--- | :---: | :---: | :---: | :---: | :---: |
| **100** | BVH-Median | $24.8\,\mu\text{s}$ | $18.2\,\mu\text{s}$ | 642 | 4,950 | **$87.0\%$** |
| **100** | BVH-SAH | $88.5\,\mu\text{s}$ | $15.4\,\mu\text{s}$ | 510 | 4,950 | **$89.7\%$** |
| **500** | BVH-Median | $152.0\,\mu\text{s}$ | $86.5\,\mu\text{s}$ | 4,210 | 124,750 | **$96.6\%$** |
| **500** | BVH-SAH | $490.2\,\mu\text{s}$ | $71.1\,\mu\text{s}$ | 3,380 | 124,750 | **$97.3\%$** |
| **500** | Fat Refit | $42.1\,\mu\text{s}$ | $94.0\,\mu\text{s}$ | 4,890 | 124,750 | **$96.1\%$** |
| **1,000** | Fat Refit | $88.4\,\mu\text{s}$ | $192.3\,\mu\text{s}$ | 11,240 | 499,500 | **$97.7\%$** |

---

## 🏗️ Project Structure

```
BVH-Based-Collision-Detection-System/
├── CMakeLists.txt          # Modern C++20 CMake build configuration
├── vcpkg.json              # SDL3 dependency pinning
├── BENCHMARKS.md           # Benchmark logs & empirical tables
├── EXPLANATION.md          # In-depth architectural DSA breakdown
├── include/
│   └── logger.h            # High-performance zero-allocation logging utility
├── src/
│   ├── main.cpp            # Application entry point, SDL3 window loop & CLI benchmarks
│   ├── collision.h         # BVHNode, AABB, Median/SAH/Refit interface
│   ├── collision.cpp       # Spatial partitioning algorithms & dual-tree traversal
│   ├── physics.h           # Contact dynamics, restitution & Coulomb friction interface
│   └── physics.cpp         # Impulse resolution, de-penetration & conservation auditor
└── tests/
    └── test_physics.cpp    # Unit & invariant verification test suite
```

---

## 🛠️ Building & Running the C++ Engine

### Prerequisites
- C++20 compliant compiler (MSVC v143+, Clang 16+, or GCC 12+)
- CMake 3.20+
- [vcpkg](https://github.com/microsoft/vcpkg) (configured with `VCPKG_ROOT`)

### Build Steps

```bash
# Clone the repository
git clone https://github.com/mithuntharan/BVH-Based-Collision-Detection-System.git
cd BVH-Based-Collision-Detection-System

# Configure via CMake with vcpkg toolchain
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"

# Build the release executable
cmake --build build --config Release
```

### Running the Native Application
```bash
# Launch interactive graphical simulation window (SDL3)
./build/Release/bvh_physics

# Run headless automated benchmark suite
./build/Release/bvh_physics --bench
```

---

## 🔬 Collision & Contact Physics Dynamics

### 1. Broad Phase: BVH Dual-Tree Traversal
The dual-tree traversal queries two bounding nodes simultaneously:
```
function queryTwoNodes(nodeA, nodeB):
  if not aabbOverlap(nodeA.bounds, nodeB.bounds):
    return // Subtree culled! Zero leaf tests needed

  if nodeA.isLeaf and nodeB.isLeaf:
    registerCandidatePair(nodeA.object, nodeB.object)
    return

  if nodeA.isLeaf:
    queryTwoNodes(nodeA, nodeB.left)
    queryTwoNodes(nodeA, nodeB.right)
  else:
    queryTwoNodes(nodeA.left, nodeB)
    queryTwoNodes(nodeA.right, nodeB)
```

### 2. Narrow Phase: Impulse & Friction Mechanics
Upon circle-circle overlap ($d < r_A + r_B$ with contact normal $\hat{n} = \frac{p_B - p_A}{d}$):
- **Normal Impulse ($j_n$)**:
  $$j_n = -\frac{(1 + e)(\vec{v}_{rel} \cdot \hat{n})}{m_A^{-1} + m_B^{-1}}$$
- **Tangential Coulomb Friction ($j_t$)**:
  $$\vec{t} = \frac{\vec{v}_{rel} - (\vec{v}_{rel} \cdot \hat{n})\hat{n}}{|\vec{v}_{rel} - (\vec{v}_{rel} \cdot \hat{n})\hat{n}|}, \quad j_t = \text{clamp}\left(-\frac{\vec{v}_{rel} \cdot \vec{t}}{m_A^{-1} + m_B^{-1}}, -\mu j_n, \mu j_n\right)$$
- **Baumgarte Positional De-penetration**:
  $$\Delta \vec{x} = \frac{\max(0, (r_A + r_B - d) - \text{slop})}{m_A^{-1} + m_B^{-1}} \cdot \beta \cdot \hat{n}$$

---

## 📜 License
This project is licensed under the [MIT License](LICENSE).
Distributed as an educational and technical case study in high-performance computer graphics and data structures.