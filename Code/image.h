// Stephi Lyou
// Module 1 Part 3: Image Read and Write
// ** image.h **

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include "math.h"

// Image class for handling PPM images
class Image {
public:
    int width, height, maxValue;
    std::vector<unsigned char> pixels;

    // Constructors
    Image();
    Image(int w, int h);

    // Set pixel color
    void setPixel(int x, int y, const Vec3& color);

    // Save as PPM
    void save(const std::string& filename, const std::string& format = "P6") const;

    // Load a PPM image
    bool load(const string& filename);

    // Get pixel color
    Vec3 getPixel(int x, int y) const;

    // Bilinear sample
    Vec3 sample(double u, double v) const;
};

