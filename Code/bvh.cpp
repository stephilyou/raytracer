// Stephi Lyou
// Module 2 Part 2: Acceleration Hierarchy
// ** bvh.cpp **

#include "bvh.h"
#include "shading.h"
#include <random>

// AABB constructor
AABB::AABB() {
    double inf = numeric_limits<double>::infinity();
    min = Vec3{inf, inf, inf};
    max = Vec3{-inf, -inf, -inf};
}

// Expand AABB to include point
void AABB::expand(const Vec3 &p) {
    min.x = std::min(min.x, p.x); min.y = std::min(min.y, p.y); min.z = std::min(min.z, p.z);
    max.x = std::max(max.x, p.x); max.y = std::max(max.y, p.y); max.z = std::max(max.z, p.z);
}

// Expand AABB to include another AABB
void AABB::expand(const AABB &b) {
    expand(b.min); expand(b.max);
}

// Check if AABB is valid
bool AABB::valid() const {
    return min.x <= max.x && min.y <= max.y && min.z <= max.z;
}

// AABB ray intersection test
bool AABB::intersect(const Ray &r, double tMin = 1e-6, double tMax = 1e9) const {
    // slab method with safe handling for near-zero directions
    for (int a = 0; a < 3; ++a) {
        double originComp = (a==0? r.origin.x : (a==1? r.origin.y : r.origin.z));
        double dirComp    = (a==0? r.direction.x : (a==1? r.direction.y : r.direction.z));
        double minComp    = (a==0? min.x : (a==1? min.y : min.z));
        double maxComp    = (a==0? max.x : (a==1? max.y : max.z));

        if (abs(dirComp) < 1e-12) {
            // Ray is parallel to slab; if origin not within slab, no hit
            if (originComp < minComp || originComp > maxComp) return false;
            else continue; // slab fully overlaps ray line
        }

        double invD = 1.0 / dirComp;
        double t0 = (minComp - originComp) * invD;
        double t1 = (maxComp - originComp) * invD;
        if (invD < 0.0) swap(t0, t1);
        tMin = std::max(t0, tMin);
        tMax = std::min(t1, tMax);
        if (tMax <= tMin) return false;
    }
    return true;
}

// Helper function to find longest axis of AABB
inline int longestAxis(const AABB &b) {
    Vec3 ext = { b.max.x - b.min.x, b.max.y - b.min.y, b.max.z - b.min.z };
    if (ext.x >= ext.y && ext.x >= ext.z) return 0;
    if (ext.y >= ext.x && ext.y >= ext.z) return 1;
    return 2;
}

// BVH recursive build function
unique_ptr<BVHNode> buildBVHRecursive(vector<Primitive> &prims,
                                      vector<int> &indices,
                                      int start, int end,
                                      const vector<Cube> &cubes,
                                      int maxLeafSize = 4)
{
    auto node = make_unique<BVHNode>();

    // compute node bounds
    AABB nodeBounds;
    for (int i = start; i < end; ++i) nodeBounds.expand(prims[indices[i]].bounds);
    node->bounds = nodeBounds;

    int n = end - start;
    if (n <= maxLeafSize) {
        node->primIndices.reserve(n);
        for (int i = start; i < end; ++i) node->primIndices.push_back(indices[i]);
        return node;
    }

    int axis = longestAxis(nodeBounds);
    
    // compute midpoint of centers along axis
    double mid = 0.0;
    for (int i = start; i < end; ++i) {
        const Vec3 &center = prims[indices[i]].center;
        mid += (axis==0 ? center.x : (axis==1 ? center.y : center.z));
    }
    mid /= double(n);

    // partition indices in-place
    int midIdx = partition(indices.begin()+start, indices.begin()+end,
        [&](int idx) {
            const Vec3 &center = prims[idx].center;
            double c = (axis==0 ? center.x : (axis==1 ? center.y : center.z));
            return c < mid;
        }) - indices.begin();

    // if partition degenerate, do median
    if (midIdx == start || midIdx == end) {
        midIdx = start + n/2;
        nth_element(indices.begin()+start, indices.begin()+midIdx, indices.begin()+end,
            [&](int a, int b) {
                const Vec3 &centerA = prims[a].center;
                const Vec3 &centerB = prims[b].center;
                double ca = (axis==0 ? centerA.x : (axis==1 ? centerA.y : centerA.z));
                double cb = (axis==0 ? centerB.x : (axis==1 ? centerB.y : centerB.z));
                return ca < cb;
            });
    }
    
    node->left  = buildBVHRecursive(prims, indices, start, midIdx, cubes, maxLeafSize);
    node->right = buildBVHRecursive(prims, indices, midIdx, end, cubes, maxLeafSize);

    return node;
}

// BVH build function
unique_ptr<BVHNode> buildBVH(vector<Primitive> &prims, const vector<Cube> &cubes, int maxLeafSize = 4) {
    vector<int> indices(prims.size());
    for (size_t i = 0; i < indices.size(); ++i) indices[i] = (int)i;
    return buildBVHRecursive(prims, indices, 0, (int)indices.size(), cubes, maxLeafSize);
}

// BVH intersection
bool bvhIntersectNode(const BVHNode *node,
                               const Ray &r,
                               Hit &outHit,
                               const vector<Primitive> &prims,
                               const vector<Sphere> &spheres,
                               const vector<Plane> &planes,
                               const vector<Cube> &cubes)
{
    if (!node) return false;
    
    // AABB test with current closest distance
    if (!node->bounds.intersect(r, 1e-6, outHit.distance)) {
        return false;
    }

    bool hitAnything = false;

    // LEAF NODE: test primitives
    if (!node->primIndices.empty()) {
        for (int pid : node->primIndices) {
            const Primitive &p = prims[pid];
            Hit temp;
            temp.distance = outHit.distance;
            bool hit = false;
            
            if (p.type == PrimType::SPHERE) {
                hit = spheres[p.index].intersect(r, temp);
            }
            else if (p.type == PrimType::PLANE) {
                hit = planes[p.index].intersect(r, temp);
            }
            else if (p.type == PrimType::CUBE) {
                hit = cubes[p.index].intersect(r, temp);
            }

            if (hit && temp.distance < outHit.distance) {
                outHit = temp;
                hitAnything = true;
            }
        }
        return hitAnything;
    }

    // Calculate centroids of child bounding boxes
    Vec3 leftCenter = (node->left->bounds.min + node->left->bounds.max) * 0.5;
    Vec3 rightCenter = (node->right->bounds.min + node->right->bounds.max) * 0.5;
    
    double distLeft = length(leftCenter - r.origin);
    double distRight = length(rightCenter - r.origin);
    
    // Visit closer child first for better early pruning
    const BVHNode *firstChild = (distLeft < distRight) ? node->left.get() : node->right.get();
    const BVHNode *secondChild = (distLeft < distRight) ? node->right.get() : node->left.get();
    
    bool hitFirst = bvhIntersectNode(firstChild, r, outHit, prims, spheres, planes, cubes);
    bool hitSecond = bvhIntersectNode(secondChild, r, outHit, prims, spheres, planes, cubes);
    
    return hitFirst || hitSecond;
}


// Lightweight wrapper that builds primIndices array from primitives order used in BVH
bool bvhIntersect(const unique_ptr<BVHNode> &root,
                  const Ray &r,
                  Hit &outHit,
                  const vector<Primitive> &prims,
                  const vector<Sphere> &spheres,
                  const vector<Plane> &planes,
                  const vector<Cube> &cubes) {
    // Initialize hit distance to far plane
    outHit.distance = 1e9;
    outHit.isHit = false;
    
    bool hit = bvhIntersectNode(root.get(), r, outHit, prims, spheres, planes, cubes);
    
    return hit;
}

// Primitive creation from scene objects
vector<Primitive> makePrimitives(
    const vector<Sphere> &spheres,
    const vector<Plane> &planes,
    const vector<Cube> &cubes)
{
    vector<Primitive> prims;

    // ---- Spheres ----
    for (size_t i = 0; i < spheres.size(); ++i) {
        Primitive p;
        p.type = PrimType::SPHERE;
        p.index = (int)i;
        p.center = spheres[i].location;

        // Get the scale
        Vec3 radi = spheres[i].scale; 

        // Build rotation matrix 
        double rx = spheres[i].rotation.x; 
        double ry = spheres[i].rotation.y; 
        double rz = spheres[i].rotation.z;
        Mat4 Rx = rotateX(rx);
        Mat4 Ry = rotateY(ry);
        Mat4 Rz = rotateZ(rz);
        Mat4 R = multiply(Rz, multiply(Ry, Rx));

        // Transform the corners of the ellipsoid's bounding box
        AABB bbox;
        for (int dx = -1; dx <= 1; dx += 2) {
            for (int dy = -1; dy <= 1; dy += 2) {
                for (int dz = -1; dz <= 1; dz += 2) {
                    // Corner of the unrotated local bounding box
                    Vec3 localCorner = {dx * radi.x, dy * radi.y, dz * radi.z};
                
                    // Rotate and translate
                    Vec3 worldCorner = p.center + transformPoint(R, localCorner);
                    bbox.expand(worldCorner);
                }
            }
        }
    
        // Add a small epsilon padding to be safe
        double epsilon = 1e-3;
        bbox.min = bbox.min - Vec3{epsilon, epsilon, epsilon};
        bbox.max = bbox.max + Vec3{epsilon, epsilon, epsilon};

        p.bounds = bbox;
        prims.push_back(p);
    }


    // Cubes
    for (size_t i = 0; i < cubes.size(); ++i) {
        Primitive p;
        p.type = PrimType::CUBE;
        p.index = (int)i;

        const Cube &c = cubes[i];
        Vec3 center = c.location;
        Vec3 half = c.scale * 0.5;

        p.center = center;

        // Build rotation matrix from Euler angles
        double rx = c.rotation.x, ry = c.rotation.y, rz = c.rotation.z;

        Mat4 Rx = rotateX(rx);
        Mat4 Ry = rotateY(ry);
        Mat4 Rz = rotateZ(rz);
        Mat4 R = multiply(Rz, multiply(Ry, Rx));

        // Compute all 8 local corners
        vector<Vec3> corners;
        corners.reserve(8);
        for (int dx = -1; dx <= 1; dx += 2)
            for (int dy = -1; dy <= 1; dy += 2)
                for (int dz = -1; dz <= 1; dz += 2)
                    corners.push_back(Vec3{dx * half.x, dy * half.y, dz * half.z});

        // Transform corners to world space
        AABB bbox;
        for (auto &corner : corners) {
            Vec3 worldCorner = center + transformPoint(R, corner);
            bbox.expand(worldCorner);
        }

        const double epsilon = 1e-1;
        bbox.min = bbox.min - Vec3{epsilon, epsilon, epsilon};
        bbox.max = bbox.max + Vec3{epsilon, epsilon, epsilon};
        p.bounds = bbox;
        prims.push_back(p);
    }


    // Planes
    for (size_t i = 0; i < planes.size(); ++i) {
        Primitive p;
        p.type = PrimType::PLANE;
        p.index = (int)i;

        p.bounds = AABB();
        p.bounds.expand(planes[i].corner1);
        p.bounds.expand(planes[i].corner2);
        p.bounds.expand(planes[i].corner3);
        p.bounds.expand(planes[i].corner4);

        // Compute approximate center
        p.center = (planes[i].corner1 + planes[i].corner2 +
                    planes[i].corner3 + planes[i].corner4) * 0.25;
        prims.push_back(p);
    }

    return prims;
}

// Scene intersection without BVH
bool sceneIntersectNoBVH(const Ray &ray,
                         Hit &hit,
                         const std::vector<Primitive> &prims,
                         const std::vector<Sphere> &spheres,
                         const std::vector<Plane> &planes,
                         const std::vector<Cube> &cubes)
{
    hit.distance = 1e9;
    hit.isHit = false;

    for (size_t i = 0; i < prims.size(); ++i) {
        const Primitive &p = prims[i];
        Hit temp;
        temp.distance = hit.distance;
        bool ok = false;

        if (p.type == PrimType::SPHERE)      ok = spheres[p.index].intersect(ray, temp);
        else if (p.type == PrimType::PLANE)  ok = planes[p.index].intersect(ray, temp);
        else if (p.type == PrimType::CUBE)   ok = cubes[p.index].intersect(ray, temp);

        if (ok && temp.distance < hit.distance) {
            hit = temp;
        }
    }
    return hit.isHit;
}