#pragma once

#include "collision.h"
#include "physics.h"
#include "benchmark_types.h"
#include <vector>
#include <random>
#include <cmath>
#include <algorithm>

/**
 * @file scene_generator.h
 * @brief Deterministic, reproducible synthetic scene generator for BVH benchmarking.
 */
class SceneGenerator {
public:
    static std::vector<Circle> generateScene(const BenchmarkConfig& config, uint32_t run_seed) {
        std::mt19937_64 rng(run_seed);
        std::vector<Circle> circles;
        circles.reserve(config.n);

        switch (config.distribution) {
            case SceneDistribution::Uniform:
                generateUniform(circles, config.n, config.arena_width, config.arena_height, rng);
                break;
            case SceneDistribution::Clustered:
                generateClustered(circles, config.n, config.arena_width, config.arena_height, rng);
                break;
            case SceneDistribution::DenseOverlap:
                generateDenseOverlap(circles, config.n, config.arena_width, config.arena_height, rng);
                break;
            case SceneDistribution::Sparse:
                generateSparse(circles, config.n, config.arena_width * 3.0f, config.arena_height * 3.0f, rng);
                break;
            case SceneDistribution::Dynamic:
                // Start with uniform distribution, velocities already initialized
                generateUniform(circles, config.n, config.arena_width, config.arena_height, rng);
                break;
        }

        return circles;
    }

private:
    static Circle createBaseCircle(float x, float y, float vx, float vy, float r) {
        Circle c{};
        c.x = x;
        c.y = y;
        c.vx = vx;
        c.vy = vy;
        c.r = r;
        c.r_col = 180;
        c.g_col = 180;
        c.b_col = 220;
        c.flash_timer = 0;
        c.mass = c.r * 0.2f;
        c.invMass = (c.mass > 0.0f) ? (1.0f / c.mass) : 0.0f;
        c.restitution = 1.0f;
        c.staticFriction = 0.35f;
        c.dynamicFriction = 0.25f;
        return c;
    }

    static void generateUniform(std::vector<Circle>& circles, int n, float w, float h, std::mt19937_64& rng) {
        std::uniform_real_distribution<float> distX(40.0f, w - 40.0f);
        std::uniform_real_distribution<float> distY(40.0f, h - 40.0f);
        std::uniform_real_distribution<float> distR(6.0f, 16.0f);
        std::uniform_real_distribution<float> distV(-120.0f, 120.0f);

        for (int i = 0; i < n; ++i) {
            circles.push_back(createBaseCircle(
                distX(rng), distY(rng),
                distV(rng), distV(rng),
                distR(rng)
            ));
        }
    }

    static void generateClustered(std::vector<Circle>& circles, int n, float w, float h, std::mt19937_64& rng) {
        // Create 6 cluster centroids across the canvas
        const int num_clusters = 6;
        struct ClusterCenter { float cx, cy; };
        std::vector<ClusterCenter> centers;
        std::uniform_real_distribution<float> cX(w * 0.15f, w * 0.85f);
        std::uniform_real_distribution<float> cY(h * 0.15f, h * 0.85f);
        for (int i = 0; i < num_clusters; ++i) {
            centers.push_back({ cX(rng), cY(rng) });
        }

        std::normal_distribution<float> normDist(0.0f, std::min(w, h) * 0.06f);
        std::uniform_real_distribution<float> distR(5.0f, 14.0f);
        std::uniform_real_distribution<float> distV(-100.0f, 100.0f);
        std::uniform_int_distribution<int> pickCenter(0, num_clusters - 1);

        for (int i = 0; i < n; ++i) {
            const auto& center = centers[pickCenter(rng)];
            float x = std::clamp(center.cx + normDist(rng), 20.0f, w - 20.0f);
            float y = std::clamp(center.cy + normDist(rng), 20.0f, h - 20.0f);
            circles.push_back(createBaseCircle(x, y, distV(rng), distV(rng), distR(rng)));
        }
    }

    static void generateDenseOverlap(std::vector<Circle>& circles, int n, float w, float h, std::mt19937_64& rng) {
        // Confine all objects within a tight central bounding box (e.g., 30% of arena)
        float minX = w * 0.35f;
        float maxX = w * 0.65f;
        float minY = h * 0.35f;
        float maxY = h * 0.65f;

        std::uniform_real_distribution<float> distX(minX, maxX);
        std::uniform_real_distribution<float> distY(minY, maxY);
        std::uniform_real_distribution<float> distR(8.0f, 18.0f);
        std::uniform_real_distribution<float> distV(-150.0f, 150.0f);

        for (int i = 0; i < n; ++i) {
            circles.push_back(createBaseCircle(
                distX(rng), distY(rng),
                distV(rng), distV(rng),
                distR(rng)
            ));
        }
    }

    static void generateSparse(std::vector<Circle>& circles, int n, float w, float h, std::mt19937_64& rng) {
        // Disperse over a substantially larger coordinate range with small radii
        std::uniform_real_distribution<float> distX(20.0f, w - 20.0f);
        std::uniform_real_distribution<float> distY(20.0f, h - 20.0f);
        std::uniform_real_distribution<float> distR(3.0f, 7.0f);
        std::uniform_real_distribution<float> distV(-50.0f, 50.0f);

        for (int i = 0; i < n; ++i) {
            circles.push_back(createBaseCircle(
                distX(rng), distY(rng),
                distV(rng), distV(rng),
                distR(rng)
            ));
        }
    }
};
