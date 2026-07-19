// Stephi Lyou
// Math Utilities
// ** math.cpp **

#include "math.h"

// Vec3 operators (add, subtract, scalar multiply)
Vec3 Vec3::operator+(const Vec3 &b) const { 
    return {x + b.x, y + b.y, z + b.z}; 
}
Vec3 Vec3::operator-(const Vec3 &b) const { 
    return {x - b.x, y - b.y, z - b.z}; 
}
Vec3 Vec3::operator*(double s) const { 
    return {x * s, y * s, z * s}; 
}
Vec3 Vec3::operator-() const {
    return Vec3{ -x, -y, -z };
}

// Cross product
Vec3 cross(const Vec3 &a, const Vec3 &b) {
    return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

// Dot product
double dot(const Vec3 &a, const Vec3 &b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

// Scalar multiply
Vec3 multiply(double s, const Vec3 &v) {
    return { s * v.x, s * v.y, s * v.z };
}

// Normalize vector
Vec3 normalize(const Vec3 &v) {
    double len = sqrt(dot(v, v));
    if (len == 0) {
        return {0, 0, 0};
    }
    return {v.x / len, v.y / len, v.z / len};
}

// Length of vector
double length(const Vec3 &v)  {
    return sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
}

// Helper function for printing Vec3
ostream& operator<<(ostream &os, const Vec3 &v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

// Helper function to parse a vector
vector<double> parseVector3(const string &line) { 
    size_t start = line.find('['); 
    size_t end = line.find(']'); 
    if (start == string::npos || end == string::npos || end <= start) 
        throw invalid_argument("Malformed vector line: " + line); 
        
    string inside = line.substr(start + 1, end - start - 1); 

    // Remove spaces 
    inside.erase(remove_if(inside.begin(), inside.end(), ::isspace), inside.end()); 

    vector<double> vec; 
    stringstream ss(inside); 
    string number; 

    while (getline(ss, number, ',')) { 
        if (!number.empty()) vec.push_back(stod(number)); 
    } 

    if (vec.size() != 3) 
        throw invalid_argument("Expected 3 elements, got " + to_string(vec.size())); 

    return vec; 
}

// Identity matrix
Mat4 identityMatrix() {
    Mat4 M{};
    for (int i = 0; i < 4; ++i) {
        M.m[i][i] = 1.0;
    }
    return M;
}

// Translate matrix
Mat4 translateMatrix(const Vec3 &t) {
    Mat4 M = identityMatrix();
    M.m[0][3] = t.x;
    M.m[1][3] = t.y;
    M.m[2][3] = t.z;
    return M;
}

// Scale matrix
Mat4 scaleMatrix(const Vec3 &s) {
    Mat4 M{};
    M.m[0][0] = s.x;
    M.m[1][1] = s.y;
    M.m[2][2] = s.z;
    M.m[3][3] = 1.0;
    return M;
}

// Rotation around x, y, z (in radians)
Mat4 rotateX(double angle) {
    Mat4 M = identityMatrix();
    double c = cos(angle), s = sin(angle);
    M.m[1][1] = c; 
    M.m[1][2] = -s;
    M.m[2][1] = s; 
    M.m[2][2] = c;
    return M;
}
Mat4 rotateY(double angle) {
    Mat4 M = identityMatrix();
    double c = cos(angle), s = sin(angle);
    M.m[0][0] = c; 
    M.m[0][2] = s;
    M.m[2][0] = -s; 
    M.m[2][2] = c;
    return M;
}
Mat4 rotateZ(double angle) {
    Mat4 M = identityMatrix();
    double c = cos(angle), s = sin(angle);
    M.m[0][0] = c; 
    M.m[0][1] = -s;
    M.m[1][0] = s; 
    M.m[1][1] = c;
    return M;
}

// Multiply matrices
Mat4 multiply(const Mat4 &A, const Mat4 &B) {
    Mat4 R{};
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            for (int k = 0; k < 4; ++k)
                R.m[i][j] += A.m[i][k] * B.m[k][j];
    return R;
}

// Transform Vec3 (point)
Vec3 transformPoint(const Mat4 &M, const Vec3 &p) {
    double x = M.m[0][0] * p.x + M.m[0][1] * p.y + M.m[0][2] * p.z + M.m[0][3];
    double y = M.m[1][0] * p.x + M.m[1][1] * p.y + M.m[1][2] * p.z + M.m[1][3];
    double z = M.m[2][0] * p.x + M.m[2][1] * p.y + M.m[2][2] * p.z + M.m[2][3];
    double w = M.m[3][0] * p.x + M.m[3][1] * p.y + M.m[3][2] * p.z + M.m[3][3];
    if (w != 0.0) { 
        x /= w; y /= w; z /= w; 
    }
    return Vec3{x, y, z};
}

// Transform direction
Vec3 transformDirection(const Mat4 &M, const Vec3 &v) {
    double x = M.m[0][0] * v.x + M.m[0][1] * v.y + M.m[0][2] * v.z;
    double y = M.m[1][0] * v.x + M.m[1][1] * v.y + M.m[1][2] * v.z;
    double z = M.m[2][0] * v.x + M.m[2][1] * v.y + M.m[2][2] * v.z;
    return Vec3{x, y, z};
}

// Invert transform
Mat4 inverseRTS(const Vec3 &translation, const Vec3 &rotation, const Vec3 &scale) {
    // Build inverse (reverse order, inverse of each)
    Vec3 invScale{ 1.0/scale.x, 1.0/scale.y, 1.0/scale.z };
    Mat4 Si = scaleMatrix(invScale);
    Mat4 Rxi = rotateX(-rotation.x);
    Mat4 Ryi = rotateY(-rotation.y);
    Mat4 Rzi = rotateZ(-rotation.z);
    Mat4 Ti = translateMatrix(Vec3{-translation.x, -translation.y, -translation.z});

    return multiply(Si, multiply(Rxi, multiply(Ryi, multiply(Rzi, Ti))));
}

// Helper function to transpose a 4x4 matrix
Mat4 transpose(const Mat4& m) {
    Mat4 t;
    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            t.m[i][j] = m.m[j][i];
    return t;
}

// Helper function to compute inverse rotation matrix from Euler angles
Mat4 inverseRotationMatrix(const Vec3 &rotation) {
    Mat4 Rx = rotateX(-rotation.x);
    Mat4 Ry = rotateY(-rotation.y);
    Mat4 Rz = rotateZ(-rotation.z);
    return multiply(Rz, multiply(Ry, Rx));
}

// General 4x4 matrix inverse
Mat4 inverse(const Mat4 &m) {
    Mat4 inv;
    double det;
    
    // Calculate cofactors
    inv.m[0][0] = m.m[1][1] * m.m[2][2] * m.m[3][3] - 
                  m.m[1][1] * m.m[2][3] * m.m[3][2] - 
                  m.m[2][1] * m.m[1][2] * m.m[3][3] + 
                  m.m[2][1] * m.m[1][3] * m.m[3][2] +
                  m.m[3][1] * m.m[1][2] * m.m[2][3] - 
                  m.m[3][1] * m.m[1][3] * m.m[2][2];

    inv.m[1][0] = -m.m[1][0] * m.m[2][2] * m.m[3][3] + 
                   m.m[1][0] * m.m[2][3] * m.m[3][2] + 
                   m.m[2][0] * m.m[1][2] * m.m[3][3] - 
                   m.m[2][0] * m.m[1][3] * m.m[3][2] - 
                   m.m[3][0] * m.m[1][2] * m.m[2][3] + 
                   m.m[3][0] * m.m[1][3] * m.m[2][2];

    inv.m[2][0] = m.m[1][0] * m.m[2][1] * m.m[3][3] - 
                  m.m[1][0] * m.m[2][3] * m.m[3][1] - 
                  m.m[2][0] * m.m[1][1] * m.m[3][3] + 
                  m.m[2][0] * m.m[1][3] * m.m[3][1] + 
                  m.m[3][0] * m.m[1][1] * m.m[2][3] - 
                  m.m[3][0] * m.m[1][3] * m.m[2][1];

    inv.m[3][0] = -m.m[1][0] * m.m[2][1] * m.m[3][2] + 
                   m.m[1][0] * m.m[2][2] * m.m[3][1] +
                   m.m[2][0] * m.m[1][1] * m.m[3][2] - 
                   m.m[2][0] * m.m[1][2] * m.m[3][1] - 
                   m.m[3][0] * m.m[1][1] * m.m[2][2] + 
                   m.m[3][0] * m.m[1][2] * m.m[2][1];

    inv.m[0][1] = -m.m[0][1] * m.m[2][2] * m.m[3][3] + 
                   m.m[0][1] * m.m[2][3] * m.m[3][2] + 
                   m.m[2][1] * m.m[0][2] * m.m[3][3] - 
                   m.m[2][1] * m.m[0][3] * m.m[3][2] - 
                   m.m[3][1] * m.m[0][2] * m.m[2][3] + 
                   m.m[3][1] * m.m[0][3] * m.m[2][2];

    inv.m[1][1] = m.m[0][0] * m.m[2][2] * m.m[3][3] - 
                  m.m[0][0] * m.m[2][3] * m.m[3][2] - 
                  m.m[2][0] * m.m[0][2] * m.m[3][3] + 
                  m.m[2][0] * m.m[0][3] * m.m[3][2] + 
                  m.m[3][0] * m.m[0][2] * m.m[2][3] - 
                  m.m[3][0] * m.m[0][3] * m.m[2][2];

    inv.m[2][1] = -m.m[0][0] * m.m[2][1] * m.m[3][3] + 
                   m.m[0][0] * m.m[2][3] * m.m[3][1] + 
                   m.m[2][0] * m.m[0][1] * m.m[3][3] - 
                   m.m[2][0] * m.m[0][3] * m.m[3][1] - 
                   m.m[3][0] * m.m[0][1] * m.m[2][3] + 
                   m.m[3][0] * m.m[0][3] * m.m[2][1];

    inv.m[3][1] = m.m[0][0] * m.m[2][1] * m.m[3][2] - 
                  m.m[0][0] * m.m[2][2] * m.m[3][1] - 
                  m.m[2][0] * m.m[0][1] * m.m[3][2] + 
                  m.m[2][0] * m.m[0][2] * m.m[3][1] + 
                  m.m[3][0] * m.m[0][1] * m.m[2][2] - 
                  m.m[3][0] * m.m[0][2] * m.m[2][1];

    inv.m[0][2] = m.m[0][1] * m.m[1][2] * m.m[3][3] - 
                  m.m[0][1] * m.m[1][3] * m.m[3][2] - 
                  m.m[1][1] * m.m[0][2] * m.m[3][3] + 
                  m.m[1][1] * m.m[0][3] * m.m[3][2] + 
                  m.m[3][1] * m.m[0][2] * m.m[1][3] - 
                  m.m[3][1] * m.m[0][3] * m.m[1][2];

    inv.m[1][2] = -m.m[0][0] * m.m[1][2] * m.m[3][3] + 
                   m.m[0][0] * m.m[1][3] * m.m[3][2] + 
                   m.m[1][0] * m.m[0][2] * m.m[3][3] - 
                   m.m[1][0] * m.m[0][3] * m.m[3][2] - 
                   m.m[3][0] * m.m[0][2] * m.m[1][3] + 
                   m.m[3][0] * m.m[0][3] * m.m[1][2];

    inv.m[2][2] = m.m[0][0] * m.m[1][1] * m.m[3][3] - 
                  m.m[0][0] * m.m[1][3] * m.m[3][1] - 
                  m.m[1][0] * m.m[0][1] * m.m[3][3] + 
                  m.m[1][0] * m.m[0][3] * m.m[3][1] + 
                  m.m[3][0] * m.m[0][1] * m.m[1][3] - 
                  m.m[3][0] * m.m[0][3] * m.m[1][1];

    inv.m[3][2] = -m.m[0][0] * m.m[1][1] * m.m[3][2] + 
                   m.m[0][0] * m.m[1][2] * m.m[3][1] + 
                   m.m[1][0] * m.m[0][1] * m.m[3][2] - 
                   m.m[1][0] * m.m[0][2] * m.m[3][1] - 
                   m.m[3][0] * m.m[0][1] * m.m[1][2] + 
                   m.m[3][0] * m.m[0][2] * m.m[1][1];

    inv.m[0][3] = -m.m[0][1] * m.m[1][2] * m.m[2][3] + 
                   m.m[0][1] * m.m[1][3] * m.m[2][2] + 
                   m.m[1][1] * m.m[0][2] * m.m[2][3] - 
                   m.m[1][1] * m.m[0][3] * m.m[2][2] - 
                   m.m[2][1] * m.m[0][2] * m.m[1][3] + 
                   m.m[2][1] * m.m[0][3] * m.m[1][2];

    inv.m[1][3] = m.m[0][0] * m.m[1][2] * m.m[2][3] - 
                  m.m[0][0] * m.m[1][3] * m.m[2][2] - 
                  m.m[1][0] * m.m[0][2] * m.m[2][3] + 
                  m.m[1][0] * m.m[0][3] * m.m[2][2] + 
                  m.m[2][0] * m.m[0][2] * m.m[1][3] - 
                  m.m[2][0] * m.m[0][3] * m.m[1][2];

    inv.m[2][3] = -m.m[0][0] * m.m[1][1] * m.m[2][3] + 
                   m.m[0][0] * m.m[1][3] * m.m[2][1] + 
                   m.m[1][0] * m.m[0][1] * m.m[2][3] - 
                   m.m[1][0] * m.m[0][3] * m.m[2][1] - 
                   m.m[2][0] * m.m[0][1] * m.m[1][3] + 
                   m.m[2][0] * m.m[0][3] * m.m[1][1];

    inv.m[3][3] = m.m[0][0] * m.m[1][1] * m.m[2][2] - 
                  m.m[0][0] * m.m[1][2] * m.m[2][1] - 
                  m.m[1][0] * m.m[0][1] * m.m[2][2] + 
                  m.m[1][0] * m.m[0][2] * m.m[2][1] + 
                  m.m[2][0] * m.m[0][1] * m.m[1][2] - 
                  m.m[2][0] * m.m[0][2] * m.m[1][1];

    // Calculate determinant
    det = m.m[0][0] * inv.m[0][0] + m.m[0][1] * inv.m[1][0] + 
          m.m[0][2] * inv.m[2][0] + m.m[0][3] * inv.m[3][0];

    if (fabs(det) < 1e-12) {
        // Singular matrix, return identity
        return Mat4();
    }

    det = 1.0 / det;

    // Scale by determinant
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            inv.m[i][j] *= det;
        }
    }

    return inv;
}
