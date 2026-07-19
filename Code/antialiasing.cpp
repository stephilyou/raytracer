// Stephi Lyou
// Module 3 Part 2: Anti-Aliasing
// ** antialiasing.cpp **

#include "antialiasing.h"

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
    const RenderConfig &cfg)
{
    double jitterX = 0.0;
    double jitterY = 0.0;

    Ray ray = generateRay(px, py, camera, jitterX, jitterY);

    return traceRay(
        ray,
        bvhRoot,
        prims,
        spheres,
        planes,
        cubes,
        primIndexOrder,
        lights,
        camera.location,
        cfg.maxDepth,
        cfg
    );
}


// Recursively sample a rectangular region in pixel space.
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
    const RenderConfig &cfg)
{
    const int   MAX_DEPTH   = 1; 
    const double THRESHOLD  = 0.08; 

    double mx = 0.5 * (x0 + x1);
    double my = 0.5 * (y0 + y1);

    // 4 samples at centers of the 4 quadrants of this region
    Vec3 c00 = sampleSubpixel(0.5 * (x0 + mx), 0.5 * (y0 + my),
                              camera, bvhRoot, prims, spheres, planes, cubes, primIndexOrder, lights, cfg);
    Vec3 c10 = sampleSubpixel(0.5 * (mx + x1), 0.5 * (y0 + my),
                              camera, bvhRoot, prims, spheres, planes, cubes, primIndexOrder, lights, cfg);
    Vec3 c01 = sampleSubpixel(0.5 * (x0 + mx), 0.5 * (my + y1),
                              camera, bvhRoot, prims, spheres, planes, cubes, primIndexOrder, lights, cfg);
    Vec3 c11 = sampleSubpixel(0.5 * (mx + x1), 0.5 * (my + y1),
                              camera, bvhRoot, prims, spheres, planes, cubes, primIndexOrder, lights, cfg);

    Vec3 avg = (c00 + c10 + c01 + c11) * 0.25;

    // Maximum color difference from the average (simple contrast metric)
    auto diffLen = [&](const Vec3 &c) { return length(c - avg); };
    double maxDiff = std::max(std::max(diffLen(c00), diffLen(c10)),
                              std::max(diffLen(c01), diffLen(c11)));

    // If low contrast or max depth reached, just return the average
    if (maxDiff < THRESHOLD || depth >= MAX_DEPTH) {
        return avg;
    }

    // Otherwise, subdivide into 4 subregions and recurse
    Vec3 q0 = adaptiveSampleRegion(x0, y0, mx, my, depth + 1,
                                   camera, bvhRoot, prims, spheres, planes, cubes, primIndexOrder, lights, cfg);
    Vec3 q1 = adaptiveSampleRegion(mx, y0, x1, my, depth + 1,
                                   camera, bvhRoot, prims, spheres, planes, cubes, primIndexOrder, lights, cfg);
    Vec3 q2 = adaptiveSampleRegion(x0, my, mx, y1, depth + 1,
                                   camera, bvhRoot, prims, spheres, planes, cubes, primIndexOrder, lights, cfg);
    Vec3 q3 = adaptiveSampleRegion(mx, my, x1, y1, depth + 1,
                                   camera, bvhRoot, prims, spheres, planes, cubes, primIndexOrder, lights, cfg);

    return (q0 + q1 + q2 + q3) * 0.25;
}

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
                  const std::vector<Light>& lights)
{
    int spp = std::max(1, cfg.samplesPerPixel);
    int samplesPerAxis = int(std::sqrt(double(spp)));
    if (samplesPerAxis < 1) samplesPerAxis = 1;
    double subPixelSize = 1.0 / samplesPerAxis;

    Vec3 colorSum{0.0, 0.0, 0.0};

    for (int sy = 0; sy < samplesPerAxis; ++sy) {
        for (int sx = 0; sx < samplesPerAxis; ++sx) {
            double jitterX = (sx + ([](){ static thread_local std::mt19937 gen(std::random_device{}()); static std::uniform_real_distribution<double> dist(0.0, 1.0); return dist(gen); })()) * subPixelSize;
            double jitterY = (sy + ([](){ static thread_local std::mt19937 gen(std::random_device{}()); static std::uniform_real_distribution<double> dist(0.0, 1.0); return dist(gen); })()) * subPixelSize;

            Ray ray = generateRay(pixelX, pixelY, camera, jitterX, jitterY);

            Vec3 c = traceRay(ray, bvhRoot, prims, spheres, planes, cubes,
                              primOrder, lights, camera.location, cfg.maxDepth, cfg);

            colorSum.x += c.x;
            colorSum.y += c.y;
            colorSum.z += c.z;
        }
    }

    double invSamples = 1.0 / double(samplesPerAxis * samplesPerAxis);
    return Vec3{colorSum.x * invSamples,
                colorSum.y * invSamples,
                colorSum.z * invSamples};
}