// Stephi Lyou
// Math Utilities
// ** math.h **

#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <array>
#include <cmath>

using namespace std;

// Struct for 3D vector
struct Vec3 {
    double x = 0.0, y = 0.0, z = 0.0;

    Vec3 operator+(const Vec3 &b) const;
    Vec3 operator-(const Vec3 &b) const;
    Vec3 operator*(double s) const;
    Vec3 operator-() const;
};

// Cross product
Vec3 cross(const Vec3 &a, const Vec3 &b);

// Dot product
double dot(const Vec3 &a, const Vec3 &b);

// Scalar multiply
Vec3 multiply(double s, const Vec3 &v);

// Normalize vector
Vec3 normalize(const Vec3 &v);

// Length of vector
double length(const Vec3 &v);

// Helper function for printing Vec3
ostream& operator<<(ostream &os, const Vec3 &v);

// Helper function to parse a vector
vector<double> parseVector3(const string &line);

// Struct for 4x4 matrix
struct Mat4 {
    double m[4][4];
};

// Identity
Mat4 identityMatrix();

// Translate matrix
Mat4 translateMatrix(const Vec3 &t);

// Scale matrix
Mat4 scaleMatrix(const Vec3 &s);

// Rotation around x, y, z (in radians)
Mat4 rotateX(double angle);
Mat4 rotateY(double angle);
Mat4 rotateZ(double angle);

// Multiply matrices
Mat4 multiply(const Mat4 &A, const Mat4 &B);

// Transform Vec3 (point)
Vec3 transformPoint(const Mat4 &M, const Vec3 &p);

// Transform direction
Vec3 transformDirection(const Mat4 &M, const Vec3 &v);

// Invert transform
Mat4 inverseRTS(const Vec3 &translation, const Vec3 &rotation, const Vec3 &scale);

// Transpose matrix
Mat4 transpose(const Mat4& m);

// Inverse rotation matrix from Euler angles
Mat4 inverseRotationMatrix(const Vec3 &rotation);

// General inverse
Mat4 inverse(const Mat4 &m);