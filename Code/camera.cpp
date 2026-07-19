// Stephi Lyou
// Module 1 Part 2: Camera Space Transformations
// ** camera.cpp **

#include "camera.h"

// Parse the camera info from the text file
void parseCameraInfo(ifstream& file, Camera& camera) {
    string line;
    bool cameraFound = false;

    while (getline(file, line)) {
        if (line.find("Type: CAMERA") != string::npos) {
            cameraFound = true;
        }

        if (cameraFound) {
            if (line.find("Name:") != string::npos) {
                size_t colonPos = line.find(":");
                if (colonPos != string::npos) {
                    camera.name = line.substr(colonPos + 1);
                    camera.name.erase(0, camera.name.find_first_not_of(" \t"));
                }
            } else if (line.find("Location:") != string::npos) {
                auto v = parseVector3(line);
                camera.location = {v[0], v[1], v[2]};
            } else if (line.find("Gaze Vector:") != string::npos) {
                auto v = parseVector3(line);
                camera.gazeVector = {v[0], v[1], v[2]};
            } else if (line.find("Up Vector:") != string::npos) {
                auto v = parseVector3(line);
                camera.upVector = {v[0], v[1], v[2]};
            } else if (line.find("Focal Length:") != string::npos) {
                camera.focalLength = stod(line.substr(line.find(":") + 1));
            } else if (line.find("Sensor Width:") != string::npos) {
                camera.sensorWidth = stod(line.substr(line.find(":") + 1));
            } else if (line.find("Sensor Height:") != string::npos) {
                camera.sensorHeight = stod(line.substr(line.find(":") + 1));
            } else if (line.find("Resolution X:") != string::npos) {
                camera.resolutionX = stoi(line.substr(line.find(":") + 1));
            } else if (line.find("Resolution Y:") != string::npos) {
                camera.resolutionY = stoi(line.substr(line.find(":") + 1));
            } else if (line.empty()) {
                break;
            }
        }
    }
}

// Convert pixel coordinates to a ray in world space
Ray generateRay(
    double px, double py,
    const Camera& camera,
    double jitterX, double jitterY)
{
    Vec3 w = normalize(Vec3{-camera.gazeVector.x,
                            -camera.gazeVector.y,
                            -camera.gazeVector.z});
    Vec3 u = normalize(cross(camera.upVector, w));
    Vec3 v = cross(w, u);

    double x = px + jitterX;
    double y = py + jitterY;

    double ndcX = (x + 0.5) / double(camera.resolutionX);
    double ndcY = (y + 0.5) / double(camera.resolutionY);

    double sensorX = (ndcX - 0.5) * camera.sensorWidth;
    double sensorY = (0.5 - ndcY) * camera.sensorHeight;

    Vec3 forward = normalize(camera.gazeVector);
    Vec3 Pc = camera.location + forward * camera.focalLength;
    Vec3 Pimage = Pc + u * sensorX + v * sensorY;

    Ray ray;
    ray.origin    = camera.location;
    ray.direction = normalize(Pimage - camera.location);
    return ray;
}