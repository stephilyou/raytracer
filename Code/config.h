// Stephi Lyou
// Module 4 Part 1: System Integration
// ** config.h **

#pragma once
#include <string>
#include <iostream>

struct RenderConfig {
    std::string sceneFile = "scene.txt";
    std::string outputFile = "output.ppm";

    int samplesPerPixel = 16;
    int maxDepth        = 5;

    bool enableBVH          = true;
    bool enableAA           = true;
    bool enableDOF          = true;
    bool enableMotionBlur   = true;
    bool enableSoftShadows  = false; 
};

RenderConfig parseArgs(int argc, char** argv);
