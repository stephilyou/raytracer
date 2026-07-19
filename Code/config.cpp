// Stephi Lyou
// Module 4 Part 1: System Integration
// ** config.cpp **

#include "config.h"

RenderConfig parseArgs(int argc, char** argv) {
    RenderConfig cfg;
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        auto next = [&](int& i) -> std::string {
            if (i + 1 < argc) return std::string(argv[++i]);
            throw std::runtime_error("Missing value after " + a);
        };

        if (a == "--scene")          cfg.sceneFile = next(i);
        else if (a == "--out")       cfg.outputFile = next(i);
        else if (a == "--spp")       cfg.samplesPerPixel = std::stoi(next(i));
        else if (a == "--depth")     cfg.maxDepth = std::stoi(next(i));

        else if (a == "--no-bvh")        cfg.enableBVH = false;
        else if (a == "--no-aa")         cfg.enableAA = false;

        else if (a == "--help") {
            std::cout
                << "Usage: renderer [options]\n"
                << "  --scene FILE       Scene description\n"
                << "  --out FILE         Output PPM\n"
                << "  --spp N            Samples per pixel\n"
                << "  --depth N          Max recursion depth\n"
                << "  --no-bvh           Disable BVH\n"
                << "  --no-aa            Disable anti-aliasing\n";
            std::exit(0);
        }
    }
    return cfg;
}