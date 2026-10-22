# Contributing to the BVH Collision Detection & Benchmarking Suite

Welcome to the team project! This guide explains how teammates can extend the benchmarking suite, introduce new spatial distributions, add custom collision primitives, and contribute clean, mergeable code.

---

## 1. Project Philosophy & Division of Responsibilities

* **Core Algorithm Scope (`src/`)**: BVH construction, dual-tree traversal, refit mechanics, and impulse physics.
* **Benchmarking, Analytics & Docs Scope (`benchmarks/`, `analysis/`, `docs/`)**: Experimental harness, dataset generation, metric aggregation, statistical plotting, and technical reports.
* **Rule of Non-Invasive Extensions**: When implementing benchmark utilities, write adapters and wrappers in `/benchmarks/include` rather than mutating core headers, ensuring seamless team collaboration and zero merge conflicts.

---

## 2. Branching & Git Workflow

All benchmarking, analytics, and documentation contributions must be committed to the dedicated feature branch:
```bash
# Verify current branch
git branch --show-current
# Expected: benchmarking-docs

# Pull latest changes from remote
git pull origin benchmarking-docs
```

### Commit Message Guidelines
Use Conventional Commits formatting:
- `feat(benchmarks): add grid spatial distribution benchmark`
- `fix(analysis): correct R^2 curve fitting regression formula`
- `docs(architecture): add SIMD traversal flowchart`
- `perf(collision): optimize AABB overlap SIMD intrinsics`

---

## 3. How to Add a New Benchmark Scenario

### Step 1: Declare the Scenario in `benchmarks/include/benchmark_types.h`
Add the new distribution enum variant:
```cpp
enum class SceneDistribution {
    Uniform,
    Clustered,
    DenseOverlap,
    Sparse,
    Dynamic,
    Grid // <-- New scenario
};

inline const char* distributionToString(SceneDistribution d) {
    switch (d) {
        // ...
        case SceneDistribution::Grid: return "grid";
        default: return "unknown";
    }
}
```

### Step 2: Implement the Generator in `benchmarks/include/scene_generator.h`
Add deterministic object placement logic:
```cpp
static void generateGrid(std::vector<Circle>& circles, int n, float w, float h, std::mt19937_64& rng) {
    int cols = static_cast<int>(std::ceil(std::sqrt(n)));
    int rows = static_cast<int>(std::ceil(static_cast<float>(n) / cols));
    float stepX = (w - 80.0f) / cols;
    float stepY = (h - 80.0f) / rows;

    for (int i = 0; i < n; ++i) {
        int r = i / cols;
        int c = i % cols;
        float x = 40.0f + c * stepX;
        float y = 40.0f + r * stepY;
        circles.push_back(createBaseCircle(x, y, 0.0f, 0.0f, 10.0f));
    }
}
```
Connect it in `generateScene()` switch block.

### Step 3: Run the Benchmark Harness
```powershell
# Run a quick smoke test verifying the new scenario
powershell -ExecutionPolicy Bypass -File benchmarks/run_benchmarks.ps1 -Quick

# Run the complete test matrix
powershell -ExecutionPolicy Bypass -File benchmarks/run_benchmarks.ps1
```

### Step 4: Re-generate Analytics & Figures
```bash
python analysis/generate_analytics.py
```
This automatically updates all 8 figures in `analysis/figures/`, refreshes `analysis/key_metrics_summary.md`, and updates `analysis/findings.md`.

---

## 4. Verification & Testing Checklist

Before submitting a Pull Request, verify the following:
- [ ] **Correctness Invariant**: `BenchmarkUtils::verifyCorrectness()` passes with `[PASS]` for all configurations, confirming exact Bitwise identity with Brute Force.
- [ ] **Zero SDL Dependency in Benchmarks**: `benchmarks/` compiles purely with `-std=c++20` and standard libraries (no graphics card or display needed).
- [ ] **Clean Git Working Tree**: No generated binary executables (`.exe`), log dumps, or intermediate object files committed.
- [ ] **Minimal Python Dependencies**: All analysis scripts execute with standard packages pinned in `requirements.txt`.
