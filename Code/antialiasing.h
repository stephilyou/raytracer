// Stephi Lyou
// Module 3 Part 2: Anti-Aliasing
// ** antialiasing.h **

#pragma once
#include <random>
#include "math.h"
#include "camera.h"
#include "rayIntersection.h"
#include "shading.h"
#include "bvh.h"

// Sample a ray at arbitrary sub-pixel coordinates
Vec3 sampleSubpixel(
    double px, double py,
    const Camera &camera,
    const std::unique_ptr<BVHNode> &bvhRoot,
    const std::vector<Primitive> &prims,
    const std::vector<Sphere> &spheres,
    const std::vector<Plane> &planes,
    const std::vector<Cube> &cubes,
    const std::vector<int> &primIndexOrder,
    const std::vector<Light> &lights,
    const RenderConfig &cfg);

// Recursively sample a rectangular region in pixel space
Vec3 adaptiveSampleRegion(
    double x0, double y0,
    double x1, double y1,
    int depth,
    const Camera &camera,
    const std::unique_ptr<BVHNode> &bvhRoot,
    const std::vector<Primitive> &prims,
    const std::vector<Sphere> &spheres,
    const std::vector<Plane> &planes,
    const std::vector<Cube> &cubes,
    const std::vector<int> &primIndexOrder,
    const std::vector<Light> &lights,
    const RenderConfig &cfg);

// Anti-aliasing ray tracing function
Vec3 tracePixelAntiAliasing(int pixelX, int pixelY,
                  const Camera& camera,
                  const RenderConfig& cfg,
                  const std::unique_ptr<BVHNode>& bvhRoot,
                  const std::vector<Primitive>& prims,
                  const std::vector<Sphere>& spheres,
                  const std::vector<Plane>& planes,
                  const std::vector<Cube>& cubes,
                  const std::vector<int>& primOrder,
                  const std::vector<Light>& lights);