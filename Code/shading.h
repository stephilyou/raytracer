// Stephi Lyou
// Module 3 Part 1: Whitted-style Raytracing
// ** shading.h **

#pragma once

#include <memory>
#include <vector>
#include <cmath>
#include <algorithm>
#include "rayIntersection.h"
#include "bvh.h"
#include "texture.h"
#include "math.h"
#include "image.h"

// Basic Blinn-Phong shader
Vec3 shadeBlinnPhong(const Hit &hit,
                    const std::vector<Light> &lights,
                    const Vec3 &cameraLocation,
                    const std::unique_ptr<BVHNode> &bvhRoot,
                    const std::vector<Primitive> &prims,
                    const std::vector<Sphere> &spheres,
                    const std::vector<Plane> &planes,
                    const std::vector<Cube> &cubes,
                    const RenderConfig &cfg);

// Shadow check
bool isInShadow(const Vec3 &point, const Vec3 &lightPos,
                const std::unique_ptr<BVHNode> &bvhRoot,
                const std::vector<Primitive> &prims,
                const std::vector<Sphere> &spheres,
                const std::vector<Plane> &planes,
                const std::vector<Cube> &cubes,
                const RenderConfig &cfg);
                

// Reflection calculation
Vec3 reflect(const Vec3 &incident, const Vec3 &normal);

// Refraction calculation
bool refract(const Vec3 &incident, const Vec3 &normal, double eta, Vec3 &refracted);

// Schlick's approximation for reflectance
double schlick(double cosine, double refractiveIndex);

// Recursive ray tracing function with reflection/refraction
Vec3 traceRayRecursive(
    const Ray& ray,
    const std::unique_ptr<BVHNode>& bvhRoot,
    const std::vector<Primitive>& prims,
    const std::vector<Sphere>& spheres,
    const std::vector<Plane>& planes,
    const std::vector<Cube>& cubes,
    const std::vector<int>& primOrder,
    const std::vector<int>& primIndexOrder,
    const std::vector<Light>& lights,
    const Vec3& cameraLocation,
    int depth,
    int maxDepth,
    const RenderConfig &cfg);

// Main ray tracing function
Vec3 traceRay(const Ray& ray,
              const std::unique_ptr<BVHNode>& bvhRoot,
              const std::vector<Primitive>& prims,
              const std::vector<Sphere>& spheres,
              const std::vector<Plane>& planes,
              const std::vector<Cube>& cubes,
              const std::vector<int>& primOrder,
              const std::vector<Light>& lights,
              const Vec3& cameraLocation,
              int maxDepth,
              const RenderConfig &cfg);