// Stephi Lyou
// Module 2 Part 1: Ray Intersection
// ** rayIntersection.cpp **

#include "rayIntersection.h"

// Helper function to parse Vec3 from a string
Vec3 parseVec3(const string &s) {
    size_t start = s.find('[');
    size_t end = s.find(']');
    string inside = s.substr(start + 1, end - start - 1);
    stringstream ss(inside);
    string num;
    vector<double> vals;
    while (getline(ss, num, ',')) vals.push_back(stod(num));
    return Vec3{vals[0], vals[1], vals[2]};
}

// Returns boolean indicating if ray intersects sphere and updates hit info
bool Sphere::intersect(const Ray &r, Hit &h) const {
    // Compute inverse transform
    Mat4 M_inv = inverseRTS(location, rotation, scale);

    // Transform ray into object space
    Ray r_local;
    r_local.origin = transformPoint(M_inv, r.origin);
    r_local.direction = normalize(transformDirection(M_inv, r.direction));

    // Local intersection test (unit sphere at origin, radius=1)
    Vec3 oc = r_local.origin;
    double a = dot(r_local.direction, r_local.direction);
    double b = 2.0 * dot(oc, r_local.direction);
    double c = dot(oc, oc) - 1.0;
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return false;
    }

    double t = (-b -  sqrt(discriminant)) / (2 * a);
    if (t < 0) {
        return false;
    }

    Vec3 p_local = r_local.origin + r_local.direction * t;
    Vec3 n_local = normalize(p_local);

    // Transform intersection back to world space
    Mat4 M = multiply(translateMatrix(location),
                      multiply(rotateZ(rotation.z),
                      multiply(rotateY(rotation.y),
                      multiply(rotateX(rotation.x), scaleMatrix(scale)))));
    Mat4 normalMatrix = transpose(M_inv);

    // Compute UV coordinates (spherical mapping)
    // p_local is on unit sphere surface
    double phi = atan2(n_local.z, n_local.x);       // azimuthal angle
    double theta = asin(n_local.y);                 // elevation angle
    
    h.u = 0.5 + phi / (2.0 * M_PI);                // map [-π, π] to [0, 1]
    h.v = 0.5 - theta / M_PI; 

    h.intersectionPoint = transformPoint(M, p_local);
    h.normal = normalize(transformDirection(normalMatrix, n_local));
    h.distance = length(h.intersectionPoint - r.origin);
    h.isHit = true;
    h.material.diffuseColor = diffuseColor;
    h.material.specularColor = specularColor;
    h.material.shininess = shininess;
    h.material.reflectivity = reflectivity;
    h.material.transparency = transparency;
    h.material.refractiveIndex = refractiveIndex;
    h.material.textureFile = textureFile;
    h.material.hasTexture = hasTexture;
    h.material.textureId = textureId;
    return true;
}

// Returns boolean indicating if ray intersects plane and updates hit info
bool Plane::intersect(const Ray &r, Hit &h) const {
    // Compute the plane's normal using the first 3 corners
    Vec3 edge1 = corner2 - corner1;
    Vec3 edge2 = corner3 - corner1;
    Vec3 normal = normalize(cross(edge1, edge2));

    // Check if ray is parallel to plane
    double denom = dot(normal, r.direction);
    if (fabs(denom) < 1e-6) {
        return false;
    }

    // Compute t (distance to plane)
    double t = dot(corner1 - r.origin, normal) / denom;
    if (t < 0) {
        return false;
    }

    // Compute intersection point
    Vec3 P = r.origin + r.direction * t;

    // Helper lambda for point-in-triangle test
    auto pointInTriangle = [&](const Vec3 &A, const Vec3 &B, const Vec3 &C, const Vec3 &P) {
        Vec3 v0 = C - A;
        Vec3 v1 = B - A;
        Vec3 v2 = P - A;

        double dot00 = dot(v0, v0);
        double dot01 = dot(v0, v1);
        double dot02 = dot(v0, v2);
        double dot11 = dot(v1, v1);
        double dot12 = dot(v1, v2);

        double invDenom = 1.0 / (dot00 * dot11 - dot01 * dot01);
        double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

        return (u >= 0) && (v >= 0) && (u + v <= 1);
    };

    // Check both triangles
    bool inside = pointInTriangle(corner1, corner2, corner3, P) || pointInTriangle(corner2, corner3, corner4, P);

    if (!inside) {
        return false;
    }

    edge1 = corner2 - corner1;
    edge2 = corner4 - corner1;  // perpendicular edge
    Vec3 localP = P - corner1;
    
    double len1 = length(edge1);
    double len2 = length(edge2);
    
    h.u = dot(localP, normalize(edge1)) / len1;
    h.v = dot(localP, normalize(edge2)) / len2;
    
    // Wrap to [0,1] if needed
    h.u = h.u - floor(h.u);
    h.v = h.v - floor(h.v);

    // Record hit
    h.isHit = true;
    h.distance = t;
    h.intersectionPoint = P;
    h.normal = normal;
    h.material.diffuseColor = diffuseColor;
    h.material.specularColor = specularColor;
    h.material.shininess = shininess;
    h.material.reflectivity = reflectivity;
    h.material.transparency = transparency;
    h.material.refractiveIndex = refractiveIndex;
    h.material.textureFile = textureFile;
    h.material.hasTexture = hasTexture;
    h.material.textureId = textureId;
    return true;
}

// Returns boolean indicating if ray intersects cube and updates hit info
bool Cube::intersect(const Ray &r, Hit &h) const {
    // Build object-to-world matrix (same order as Blender: Scale → RotX → RotY → RotZ → Translate)
    Mat4 S = scaleMatrix(scale);
    Mat4 Rx = rotateX(rotation.x);
    Mat4 Ry = rotateY(rotation.y);
    Mat4 Rz = rotateZ(rotation.z);
    Mat4 T = translateMatrix(location);
    Mat4 M = multiply(T, multiply(Rz, multiply(Ry, multiply(Rx, S))));
    Mat4 M_inv = inverseRTS(location, rotation, scale);

    // Transform ray into object (local) space
    Ray r_local;
    r_local.origin = transformPoint(M_inv, r.origin);
    r_local.direction = normalize(transformDirection(M_inv, r.direction));

    // Local cube from [-1,1] in each axis
    Vec3 min{-1, -1, -1};
    Vec3 max{1, 1, 1};

    double tmin = -1e9;
    double tmax =  1e9;

    // X slab
    if (std::fabs(r_local.direction.x) < 1e-12) {
        // Ray is parallel to X planes: must be inside slab
        if (r_local.origin.x < min.x || r_local.origin.x > max.x) return false;
    } else {
        double invDx = 1.0 / r_local.direction.x;
        double tx0 = (min.x - r_local.origin.x) * invDx;
        double tx1 = (max.x - r_local.origin.x) * invDx;
        if (invDx < 0.0) std::swap(tx0, tx1);
        tmin = std::max(tmin, tx0);
        tmax = std::min(tmax, tx1);
        if (tmax < tmin) return false;
    }

    // Y slab
    if (std::fabs(r_local.direction.y) < 1e-12) {
        if (r_local.origin.y < min.y || r_local.origin.y > max.y) return false;
    } else {
        double invDy = 1.0 / r_local.direction.y;
        double ty0 = (min.y - r_local.origin.y) * invDy;
        double ty1 = (max.y - r_local.origin.y) * invDy;
        if (invDy < 0.0) std::swap(ty0, ty1);
        tmin = std::max(tmin, ty0);
        tmax = std::min(tmax, ty1);
        if (tmax < tmin) return false;
    }

    // Z slab
    if (std::fabs(r_local.direction.z) < 1e-12) {
        if (r_local.origin.z < min.z || r_local.origin.z > max.z) return false;
    } else {
        double invDz = 1.0 / r_local.direction.z;
        double tz0 = (min.z - r_local.origin.z) * invDz;
        double tz1 = (max.z - r_local.origin.z) * invDz;
        if (invDz < 0.0) std::swap(tz0, tz1);
        tmin = std::max(tmin, tz0);
        tmax = std::min(tmax, tz1);
        if (tmax < tmin) return false;
    }

    if (tmax <= 0.0) return false;

    // Use entry point if in front, otherwise exit point
    double t_hit = (tmin > 0.0) ? tmin : tmax;
    Vec3 p_local = r_local.origin + r_local.direction * t_hit;


    // Local-space normal
    Vec3 n_local{0, 0, 0};
    double bias = 1e-4;
    if (fabs(p_local.x - 1.0) < bias) {          // +X face
        n_local = {1, 0, 0};
        h.u = (p_local.z + 1.0) * 0.5;
        h.v = (p_local.y + 1.0) * 0.5;
    } else if (fabs(p_local.x + 1.0) < bias) {   // -X face
        n_local = {-1, 0, 0};
        h.u = (-p_local.z + 1.0) * 0.5;
        h.v = (p_local.y + 1.0) * 0.5;
    } else if (fabs(p_local.y - 1.0) < bias) {   // +Y face
        n_local = {0, 1, 0};
        h.u = (p_local.x + 1.0) * 0.5;
        h.v = (p_local.z + 1.0) * 0.5;
    } else if (fabs(p_local.y + 1.0) < bias) {   // -Y face
        n_local = {0, -1, 0};
        h.u = (p_local.x + 1.0) * 0.5;
        h.v = (-p_local.z + 1.0) * 0.5;
    } else if (fabs(p_local.z - 1.0) < bias) {   // +Z face
        n_local = {0, 0, 1};
        h.u = (p_local.x + 1.0) * 0.5;
        h.v = (p_local.y + 1.0) * 0.5;
    } else {                                      // -Z face
        n_local = {0, 0, -1};
        h.u = (-p_local.x + 1.0) * 0.5;
        h.v = (p_local.y + 1.0) * 0.5;
    }

    S = scaleMatrix(scale);
    Rx = rotateX(rotation.x);
    Ry = rotateY(rotation.y);
    Rz = rotateZ(rotation.z);
    T = translateMatrix(location);
    M = multiply(T, multiply(Rz, multiply(Ry, multiply(Rx, S))));
    h.intersectionPoint = transformPoint(M, p_local);

    Mat4 M_model = multiply(Rz, multiply(Ry, multiply(Rx, S)));
    Mat4 M_model_inv_transpose = transpose(inverse(M_model));
    h.normal = normalize(transformDirection(M_model_inv_transpose, n_local));

    h.distance = length(h.intersectionPoint - r.origin);
    h.isHit = true;
    h.material.diffuseColor = diffuseColor;
    h.material.specularColor = specularColor;
    h.material.shininess = shininess;
    h.material.reflectivity = reflectivity;
    h.material.transparency = transparency;
    h.material.refractiveIndex = refractiveIndex;
    h.material.textureFile = textureFile;
    h.material.hasTexture = hasTexture;
    h.material.textureId = textureId;
    return true;
}

// Load spheres from ASCII txt file
vector<Sphere> loadSpheresFromScene(const string &filename) {
    vector<Sphere> spheres;
    ifstream scene(filename);
    string line;

    while (getline(scene, line)) {
        if (line.find("Sphere") != string::npos) {
            Sphere s;
            while (getline(scene, line)) {
                if (line.find("Location:") != string::npos)
                    s.location = parseVec3(line);
                else if (line.find("Rotation:") != string::npos)
                    s.rotation = parseVec3(line);
                else if (line.find("Scale:") != string::npos)
                    s.scale = parseVec3(line);
                else if (line.find("Diffuse:") != string::npos)
                    s.diffuseColor = parseVec3(line);
                else if (line.find("Specular:") != string::npos)
                    s.specularColor = parseVec3(line);
                else if (line.find("Shininess:") != string::npos)
                    s.shininess = stod(line.substr(line.find(":") + 1));
                else if (line.find("Reflectivity:") != string::npos)
                    s.reflectivity = stod(line.substr(line.find(":") + 1));
                else if (line.find("Transparency:") != string::npos)
                    s.transparency = stod(line.substr(line.find(":") + 1));
                else if (line.find("Refractive Index:") != string::npos)
                    s.refractiveIndex = stod(line.substr(line.find(":") + 1));
                else if (line.find("Texture:") != string::npos) {
                    size_t colonPos = line.find(":");
                    string texPath = line.substr(colonPos + 1);
                    // Trim whitespace
                    texPath.erase(0, texPath.find_first_not_of(" \t\r\n"));
                    texPath.erase(texPath.find_last_not_of(" \t\r\n") + 1);
                    
                    if (!texPath.empty()) {
                        s.textureFile = texPath;
                        s.hasTexture = true;
                    }
                }
                else if (line.empty()) break;
            }
            spheres.push_back(s);
        }
    }
    return spheres;
}

// Load plane from ASCII txt file
vector<Plane> loadPlanesFromScene(const string &filename) {
    vector<Plane> planes;
    ifstream scene(filename);
    string line;

    while (getline(scene, line)) {
        if (line.find("Plane") != string::npos) {
            Plane p;
            while (getline(scene, line)) {
                if (line.find("Rotation:") != string::npos)
                    p.rotation = parseVec3(line);
                else if (line.find("Scale:") != string::npos)
                    p.scale = parseVec3(line);
                else if (line.find("Diffuse:") != string::npos)
                    p.diffuseColor = parseVec3(line);
                else if (line.find("Specular:") != string::npos)
                    p.specularColor = parseVec3(line);
                else if (line.find("Shininess:") != string::npos)
                    p.shininess = stod(line.substr(line.find(":") + 1));
                else if (line.find("Reflectivity:") != string::npos)
                    p.reflectivity = stod(line.substr(line.find(":") + 1));
                else if (line.find("Transparency:") != string::npos)
                    p.transparency = stod(line.substr(line.find(":") + 1));
                else if (line.find("Refractive Index:") != string::npos)
                    p.refractiveIndex = stod(line.substr(line.find(":") + 1));
                else if (line.find("Texture:") != string::npos) {
                    size_t colonPos = line.find(":");
                    string texPath = line.substr(colonPos + 1);
                    // Trim whitespace
                    texPath.erase(0, texPath.find_first_not_of(" \t\r\n"));
                    texPath.erase(texPath.find_last_not_of(" \t\r\n") + 1);
                    
                    if (!texPath.empty()) {
                        p.textureFile = texPath;
                        p.hasTexture = true;
                    }
                }
                else if (line.find("Corner1:") != string::npos)
                    p.corner1 = parseVec3(line);
                else if (line.find("Corner2:") != string::npos)
                    p.corner2 = parseVec3(line);
                else if (line.find("Corner3:") != string::npos)
                    p.corner3 = parseVec3(line);
                else if (line.find("Corner4:") != string::npos)
                    p.corner4 = parseVec3(line);
                else if (line.empty()) break;
            }
            planes.push_back(p);
        }
    }
    return planes;
}

// Load cubes from ASCII txt file
vector<Cube> loadCubesFromScene(const string &filename) {
    vector<Cube> cubes;
    ifstream scene(filename);
    string line;

    while (getline(scene, line)) {
        if (line.find("Cube") != string::npos) {
            Cube c;
            while (getline(scene, line)) {
                if (line.find("Location:") != string::npos)
                    c.location = parseVec3(line);
                else if (line.find("Rotation:") != string::npos)
                    c.rotation = parseVec3(line);
                else if (line.find("Scale:") != string::npos)
                    c.scale = parseVec3(line);
                else if (line.find("Diffuse:") != string::npos)
                    c.diffuseColor = parseVec3(line);
                else if (line.find("Specular:") != string::npos)
                    c.specularColor = parseVec3(line);
                else if (line.find("Shininess:") != string::npos)
                    c.shininess = stod(line.substr(line.find(":") + 1));
                else if (line.find("Reflectivity:") != string::npos)
                    c.reflectivity = stod(line.substr(line.find(":") + 1));
                else if (line.find("Transparency:") != string::npos)
                    c.transparency = stod(line.substr(line.find(":") + 1));
                else if (line.find("Refractive Index:") != string::npos)
                    c.refractiveIndex = stod(line.substr(line.find(":") + 1));
                else if (line.find("Texture:") != string::npos) {
                    size_t colonPos = line.find(":");
                    string texPath = line.substr(colonPos + 1);
                    // Trim whitespace
                    texPath.erase(0, texPath.find_first_not_of(" \t\r\n"));
                    texPath.erase(texPath.find_last_not_of(" \t\r\n") + 1);
                    
                    if (!texPath.empty()) {
                        c.textureFile = texPath;
                        c.hasTexture = true;
                    }
                }
                else if (line.empty()) break;
            }
            cubes.push_back(c);
        }
    }
    return cubes;
}

// Load light sources from ASCII txt file
vector<Light> loadLightsFromScene(const string &filename) {
    vector<Light> lights;
    ifstream scene(filename);
    string line;

    while (getline(scene, line)) {
        if (line.find("Type: LIGHT") != string::npos) {
            Light light;
            while (getline(scene, line)) {
                if (line.find("Location:") != string::npos)
                    light.location = parseVec3(line);
                else if (line.find("Color:") != string::npos)
                    light.color = parseVec3(line);
                else if (line.find("Radiant Intensity:") != string::npos)
                    light.intensity = stod(line.substr(line.find(":") + 1));
                else if (line.empty()) break;
            }
            lights.push_back(light);
        }
    }
    return lights;
}