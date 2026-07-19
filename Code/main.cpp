// Stephi Lyou
// ** main.cpp **

#include <iostream>
#include <fstream>
#include "camera.h"
#include "image.h"
#include "rayIntersection.h"
#include "math.h"
#include "bvh.h"
#include "texture.h"
#include "config.h"
#include "shading.h"
#include "antialiasing.h"

using namespace std;

int main(int argc, char** argv)
{
    try {
        RenderConfig cfg = parseArgs(argc, argv);

        // Load camera
        Camera camera;
        {
            std::ifstream f(cfg.sceneFile);
            {
                std::ifstream f(cfg.sceneFile);
                if (!f) {
                    std::cerr << "Cannot open scene file: " << cfg.sceneFile << "\n";
                    return 1;
                }
            }
            parseCameraInfo(f, camera);
        }

        // Load geometry & lights
        std::vector<Sphere> spheres = loadSpheresFromScene(cfg.sceneFile);
        std::vector<Plane>  planes  = loadPlanesFromScene(cfg.sceneFile);
        std::vector<Cube>   cubes   = loadCubesFromScene(cfg.sceneFile);
        std::vector<Light>  lights  = loadLightsFromScene(cfg.sceneFile);

        // Load textures
        for (auto& cube : cubes) {
            if (cube.hasTexture) {
            cube.textureId = loadTexture(cube.textureFile);
            }
        }

        for (auto& sphere : spheres) {
            if (sphere.hasTexture) {
                sphere.textureId = loadTexture(sphere.textureFile);
            }
        }

        for (auto& plane : planes) {
            if (plane.hasTexture) {
                plane.textureId = loadTexture(plane.textureFile);
            }
        }

        // Build primitives and BVH
        std::vector<Primitive> prims = makePrimitives(spheres, planes, cubes);
        std::unique_ptr<BVHNode> bvhRoot;
        std::vector<int> primOrder;

        if (cfg.enableBVH) {
            bvhRoot = buildBVH(prims, cubes, 4);
            primOrder.resize(prims.size());
            for (size_t i = 0; i < primOrder.size(); ++i)
                primOrder[i] = int(i);
        }

        int W = camera.resolutionX;
        int H = camera.resolutionY;

        // Create framebuffer image
        Image image(W, H);

        for (int y = 0; y < H; ++y) {
            std::cerr << "\rScanline " << y + 1 << " / " << H << std::flush;
            for (int x = 0; x < W; ++x) {
                Vec3 color{0,0,0};

                if (cfg.enableAA && cfg.samplesPerPixel > 1) {
                    color = tracePixelAntiAliasing(
                        x, y, camera, cfg,
                        bvhRoot, prims, spheres, planes, cubes,
                        primOrder, lights
                    );
                } else {
                    Ray ray = generateRay(x, y, camera, 0.0, 0.0);
                    color = traceRay(
                        ray,
                        bvhRoot,
                        prims,
                        spheres,
                        planes,
                        cubes,
                        primOrder,
                        lights,
                        camera.location,
                        cfg.maxDepth,
                        cfg
                    );
                }

                // Store into image 
                image.setPixel(x, y, color);
            }
        }
        std::cerr << "\nDone tracing. Saving image...\n";

        // Save as PPM 
        image.save(cfg.outputFile, "P6");

        std::cerr << "Saved to " << cfg.outputFile << "\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
        return 1;
    }
    return 0;
}