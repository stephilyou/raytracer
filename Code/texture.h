// Stephi Lyou
// Module 3 Part 3: Textures
// ** texture.h **

#pragma once

#include <unordered_map>
#include <vector>
#include "math.h"
#include "image.h"

// Texture struct
struct Texture {
    int width = 0, height = 0;
    std::vector<Vec3> pixels;
};

// Sample a texture at (u,v) coordinates
Vec3 sampleTexture(const Texture &tex, double u, double v);

// Load a texture and return its ID
int loadTexture(const std::string& filepath);


