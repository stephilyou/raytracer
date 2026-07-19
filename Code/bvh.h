// Stephi Lyou
// Module 2 Part 2: Acceleration Hierarchy
// ** bvh.h **

#pragma once
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include <cmath>
#include "rayIntersection.h"

using namespace std;

// Define primitive types
#ifndef PRIMTYPE_DEFINED
enum class PrimType { SPHERE, PLANE, CUBE };
#define PRIMTYPE_DEFINED
#endif

// Axis Aligned Bounding Box
struct AABB {
    Vec3 min;
    Vec3 max;
    AABB();

    // Expand the AABB to include point
    void expand(const Vec3 &p);

    // Expand the AABB to include another AABB
    void expand(const AABB &b);

    // Check if AABB is valid
    bool valid() const;

    // AABB ray intersection test
    bool intersect(const Ray &r, double tMin, double tMax) const;
};

// Primitive struct
struct Primitive {
    PrimType type;
    size_t index;
    AABB bounds;
    Vec3 center;
};

// BVH node
struct BVHNode {
    AABB bounds;
    unique_ptr<BVHNode> left;
    unique_ptr<BVHNode> right;
    vector<int> primIndices;
};

// Helper function to find longest axis of AABB
inline int longestAxis(const AABB &b);

// BVH recursive build function
unique_ptr<BVHNode> buildBVHRecursive(vector<Primitive> &prims,
                                      vector<int> &indices,
                                      int start, int end,
                                      const vector<Cube> &cubes,
                                      int maxLeafSize);

// BVH build function
unique_ptr<BVHNode> buildBVH(vector<Primitive> &prims, const vector<Cube> &cubes, int maxLeafSize);

// BVH intersection
bool bvhIntersectNode(const BVHNode *node,
                      const Ray &r,
                      Hit &outHit,
                      const vector<Primitive> &prims,
                      const vector<Sphere> &spheres,
                      const vector<Plane> &planes,
                      const vector<Cube> &cubes);

// Lightweight wrapper that builds primIndices array from primitives order used in BVH
bool bvhIntersect(const unique_ptr<BVHNode> &root,
                  const Ray &r,
                  Hit &outHit,
                  const vector<Primitive> &prims,
                  const vector<Sphere> &spheres,
                  const vector<Plane> &planes,
                  const vector<Cube> &cubes);

// Primitive creation from scene objects
vector<Primitive> makePrimitives(const vector<Sphere> &spheres, 
                                 const vector<Plane> &planes, 
                                 const vector<Cube> &cubes);

// Scene intersection without BVH
bool sceneIntersectNoBVH(const Ray &ray,
                         Hit &hit,
                         const std::vector<Primitive> &prims,
                         const std::vector<Sphere> &spheres,
                         const std::vector<Plane> &planes,
                         const std::vector<Cube> &cubes);