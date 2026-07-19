// Stephi Lyou
// Module 1 Part 2: Camera Space Transformations
// ** camera.h **

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <cmath>
#include "math.h"
#include "config.h"


// Ray struct
struct Ray {
    Vec3 origin;
    Vec3 direction;
};

// Camera struct
struct Camera {
    string name;
    Vec3 location;
    Vec3 gazeVector;
    Vec3 upVector;
    double focalLength = 0;
    double sensorWidth = 0;
    double sensorHeight = 0;
    int resolutionX = 0;
    int resolutionY = 0;
};

// Parse the camera info from the text file
void parseCameraInfo(ifstream& file, Camera& camera);

// Convert pixel coordinates to a ray in world space
Ray generateRay(
    double px, double py,
    const Camera& camera,
    double jitterX, double jitterY);