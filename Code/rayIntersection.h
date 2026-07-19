// Stephi Lyou
// Module 2 Part 1: Ray Intersection
// ** rayIntersection.h **

#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <cmath>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <string>
#include "math.h"
#include "camera.h"

// Struct for material properties
struct Material {
    Vec3 diffuseColor;
    Vec3 specularColor;
    double shininess;
    double reflectivity;
    double transparency;
    double refractiveIndex;
    string textureFile;
    bool hasTexture = false;
    int textureId = -1; 
};

// Struct to hold hit information
struct Hit {
    bool isHit = false;
    double distance = 1e9;
    Vec3 intersectionPoint;
    Vec3 normal;
    Material material;
    double u, v;
};

// Sphere struct
struct Sphere {
    Vec3 location;
    Vec3 rotation;
    Vec3 scale;
    Vec3 diffuseColor;
    Vec3 specularColor;
    double shininess;
    double reflectivity;
    double transparency;
    double refractiveIndex;
    string textureFile;
    bool hasTexture = false;
    int textureId = -1; 
    bool intersect(const Ray &r, Hit &h) const;
};

// Plane struct
struct Plane {
    Vec3 corner1;
    Vec3 corner2;
    Vec3 corner3;
    Vec3 corner4;
    Vec3 rotation;
    Vec3 scale;
    Vec3 diffuseColor;
    Vec3 specularColor;
    double shininess;
    double reflectivity;
    double transparency;
    double refractiveIndex;
    string textureFile;
    bool hasTexture = false;
    int textureId = -1; 
    bool intersect(const Ray &r, Hit &h) const;
};

// Cube struct
struct Cube {
    Vec3 location;
    Vec3 rotation;
    Vec3 scale;
    Vec3 diffuseColor;
    Vec3 specularColor;
    double shininess;
    double reflectivity;
    double transparency;
    double refractiveIndex;
    string textureFile;
    bool hasTexture = false;
    int textureId = -1; 
    bool intersect(const Ray &r, Hit &h) const;
};

// Light struct
struct Light {
    Vec3 location;
    Vec3 color;
    double intensity;
};

// Helper function to parse Vec3 from a string
Vec3 parseVec3(const std::string &s);

// Loader functions for scene objects
vector<Sphere> loadSpheresFromScene(const string &filename);
vector<Plane> loadPlanesFromScene(const string &filename);
vector<Cube> loadCubesFromScene(const string &filename);
vector<Light> loadLightsFromScene(const string &filename);