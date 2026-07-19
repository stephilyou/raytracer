// Stephi Lyou
// Module 3 Part 1: Whitted-style Raytracing
// ** shading.cpp **

#include "shading.h"

extern std::vector<Image> g_textures;

// Safe helper functions
static inline double safeClampl(double v, double minVal, double maxVal) {
    if (std::isnan(v) || std::isinf(v)) return minVal;
    return std::max(minVal, std::min(maxVal, v));
}

static inline double clamp01(double v) {
    return safeClampl(v, 0.0, 1.0);
}

static inline Vec3 clampVec3(const Vec3 &v) {
    return Vec3{ 
        clamp01(v.x), 
        clamp01(v.y), 
        clamp01(v.z) 
    };
}

// Component-wise multiplication
static inline Vec3 multiplyVec3(const Vec3 &a, const Vec3 &b) {
    return Vec3{ 
        a.x * b.x, 
        a.y * b.y, 
        a.z * b.z 
    };
}

// Vector addition
static inline Vec3 addVec3(const Vec3 &a, const Vec3 &b) {
    return Vec3{
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

// Scalar multiply
static inline Vec3 scaleVec3(const Vec3 &v, double s) {
    return Vec3{
        v.x * s,
        v.y * s,
        v.z * s
    };
}

// Shadow check
bool isInShadow(const Vec3 &point, 
                const Vec3 &lightPos,
                const std::unique_ptr<BVHNode> &bvhRoot,
                const std::vector<Primitive> &prims,
                const std::vector<Sphere> &spheres,
                const std::vector<Plane> &planes,
                const std::vector<Cube> &cubes,
                const RenderConfig &cfg) 
{
    Vec3 toLight = lightPos - point;
    double distToLight = length(toLight);
    if (distToLight < 1e-6) return false;

    Ray shadowRay;
    Vec3 dir = normalize(toLight);
    shadowRay.origin = point + dir * 1e-4;
    shadowRay.direction = dir;

    Hit shadowHit;
    shadowHit.distance = distToLight;
    shadowHit.isHit = false;

    bool hitAnything;
    
    if (cfg.enableBVH && bvhRoot) {
        hitAnything = bvhIntersect(bvhRoot, shadowRay, shadowHit, 
                                   prims, spheres, planes, cubes);
    } else {
        hitAnything = sceneIntersectNoBVH(shadowRay, shadowHit, 
                                          prims, spheres, planes, cubes);
    }

    return hitAnything && shadowHit.distance < distToLight;
}

// Blinn-Phong shading
Vec3 shadeBlinnPhong(const Hit &hit,
                    const std::vector<Light> &lights,
                    const Vec3 &cameraLocation,
                    const std::unique_ptr<BVHNode> &bvhRoot,
                    const std::vector<Primitive> &prims,
                    const std::vector<Sphere> &spheres,
                    const std::vector<Plane> &planes,
                    const std::vector<Cube> &cubes, 
                    const RenderConfig &cfg)            
{
    if (!hit.isHit) {
        return Vec3{0.0, 0.0, 0.0};
    }

    // Base diffuse color
    Vec3 diffuseColor = hit.material.diffuseColor;
    if (hit.material.hasTexture) {
        const Image &tex = g_textures[hit.material.textureId];
        Vec3 texColor = tex.sample(hit.u, hit.v);
        diffuseColor = multiplyVec3(diffuseColor, texColor);
    }
    diffuseColor = clampVec3(diffuseColor);
    double safeShininess = safeClampl(hit.material.shininess, 1.0, 256.0);

    // Small ambient
    Vec3 ambient = scaleVec3(diffuseColor, 0.3);
    Vec3 finalColor = ambient;

    for (const auto &light : lights) {
        double safeIntensity = safeClampl(light.intensity, 0.0, 1.0);
        Vec3 safeLightColor = clampVec3(light.color);

        Vec3 toLight = light.location - hit.intersectionPoint;
        double distToLight = length(toLight);
        if (distToLight < 1e-6) continue;

        double attenuation = 1.0 / (1.0 + distToLight * distToLight);
        double lightFactor = safeIntensity * attenuation;

        Vec3 L = normalize(toLight);
        Vec3 N = normalize(hit.normal);
        Vec3 V = normalize(cameraLocation - hit.intersectionPoint);
        Vec3 H = normalize(addVec3(L, V));

        double NdotL = dot(N, L);
        double NdotH = dot(N, H);

        // Shadow test with cfg
        if (isInShadow(hit.intersectionPoint, light.location, 
                       bvhRoot, prims, spheres, planes, cubes, cfg)) {
            continue;
        }

        // Diffuse
        if (NdotL > 0.0) {
            Vec3 diffuseContrib = multiplyVec3(diffuseColor, safeLightColor);
            diffuseContrib = scaleVec3(diffuseContrib, NdotL * lightFactor);
            finalColor = addVec3(finalColor, diffuseContrib);
        }

        // Specular
        if (NdotL > 0.0 && NdotH > 0.0) {
            double normFactor = (safeShininess + 8.0) / (8.0 * 3.14159265359);
            double specularFactor = pow(NdotH, safeShininess) * normFactor;

            Vec3 specColor = clampVec3(hit.material.specularColor);
            Vec3 specularContrib = multiplyVec3(specColor, safeLightColor);
            specularContrib = scaleVec3(specularContrib, specularFactor * lightFactor);

            finalColor = addVec3(finalColor, specularContrib);
        }
    }

    return clampVec3(finalColor);
}

// Reflect a vector about a normal
Vec3 reflect(const Vec3 &incident, const Vec3 &normal) {
    return incident - normal * (2.0 * dot(incident, normal));
}

// Refract a vector through a surface using Snell's Law
bool refract(const Vec3 &incident, const Vec3 &normal, double eta, Vec3 &refracted) {
    Vec3 uv = normalize(incident);
    double dt = dot(uv, normal);
    double discriminant = 1.0 - eta * eta * (1.0 - dt * dt);
    
    if (discriminant > 0.0) {
        refracted = (uv - normal * dt) * eta - normal * sqrt(discriminant);
        return true;
    }
    return false;
}

// Schlick's approximation for Fresnel reflectance
double schlick(double cosine, double refractiveIndex) {
    double r0 = (1.0 - refractiveIndex) / (1.0 + refractiveIndex);
    r0 = r0 * r0;
    return r0 + (1.0 - r0) * pow(1.0 - cosine, 5.0);
}

// Recursive ray tracing function with reflection/refraction
Vec3 traceRayRecursive(
    const Ray& ray,
    const std::unique_ptr<BVHNode>& bvhRoot,
    const std::vector<Primitive>& prims,
    const std::vector<Sphere>& spheres,
    const std::vector<Plane>& planes,
    const std::vector<Cube>& cubes,
    const std::vector<int>& primOrder,
    const std::vector<Light>& lights,
    const Vec3& cameraLocation,
    int depth,
    int maxDepth,
    const RenderConfig &cfg
) {
    // Base case
    if (depth >= maxDepth) {
        return Vec3{0.0, 0.0, 0.0};
    }

    // Find closest intersection
    Hit hit;
    bool hitAny;

    if (cfg.enableBVH && bvhRoot) {
        hitAny = bvhIntersect(bvhRoot, ray, hit, prims, spheres, planes, cubes);
    } else {
        hitAny = sceneIntersectNoBVH(ray, hit, prims, spheres, planes, cubes);
    }

    if (!hitAny) {
        return Vec3{0.3, 0.3, 0.3};
    }

    // Material properties from Blender
    double reflectivity     = hit.material.reflectivity;
    double transparency     = hit.material.transparency;
    double refractiveIndex  = hit.material.refractiveIndex;
    reflectivity = std::max(0.0, std::min(1.0, reflectivity));
    transparency = std::max(0.0, std::min(1.0, transparency));

    // Blinn-Phong shading
    Vec3 localColor = shadeBlinnPhong(hit, lights, cameraLocation, bvhRoot, 
                                      prims, spheres, planes, cubes, cfg);

    // If purely diffuse/opaque, no recursion
    if (reflectivity < 0.01 && (transparency < 0.01 || refractiveIndex <= 1.0)) {
        return localColor;
    }

    Vec3 N = normalize(hit.normal);
    Vec3 incident = normalize(ray.direction);

    // Reflection
    Vec3 reflectedColor{0.0, 0.0, 0.0};
    if (reflectivity > 0.01 && depth < maxDepth) {
        Vec3 R = reflect(incident, N);
        Ray reflRay;
        reflRay.origin    = hit.intersectionPoint + R * 1e-4;
        reflRay.direction = R;
        
        reflectedColor = traceRayRecursive(
            reflRay, bvhRoot, prims, spheres, planes, cubes, primOrder,
            lights, cameraLocation, depth + 1, maxDepth, cfg
        );
    }

    // Refraction
    Vec3 refractedColor{0.0, 0.0, 0.0};
    if (transparency > 0.01 && refractiveIndex > 1.0) {
        bool entering = dot(incident, N) < 0.0;
        Vec3 outwardNormal;
        double eta;

        if (entering) {
            outwardNormal = N;
            eta = 1.0 / refractiveIndex;
        } else {
            outwardNormal = N * -1.0;
            eta = refractiveIndex;
        }

        Vec3 refractDir;
        if (refract(incident, outwardNormal, eta, refractDir)) {
            Ray refractRay;
            refractRay.origin    = hit.intersectionPoint - outwardNormal * 1e-4;
            refractRay.direction = normalize(refractDir);

            refractedColor = traceRayRecursive(
                refractRay, bvhRoot, prims, spheres, planes, cubes, primOrder,
                lights, cameraLocation, depth + 1, maxDepth, cfg
            );

            // Fresnel mixing
            if (reflectivity > 0.01) {
                double cosine = entering ? -dot(incident, N)
                                         :  dot(refractDir, N);
                double F = schlick(cosine, refractiveIndex);
                
                double reflWeight = reflectivity * F;
                double refrWeight = transparency * (1.0 - F);
                double localWeight = std::max(0.0, 1.0 - reflWeight - refrWeight);

                Vec3 finalColor =
                    localColor      * localWeight +
                    reflectedColor  * reflWeight +
                    refractedColor  * refrWeight;

                return clampVec3(finalColor);
            }
        } else {
            // Total internal reflection
            Vec3 reflectDir = reflect(incident, outwardNormal);
            Ray reflectRay;
            reflectRay.origin    = hit.intersectionPoint + outwardNormal * 1e-4;
            reflectRay.direction = reflectDir;

            reflectedColor = traceRayRecursive(
                reflectRay, bvhRoot, prims, spheres, planes, cubes, primOrder,
                lights, cameraLocation, depth + 1, maxDepth, cfg
            );
        }
    }

    // Simple mix using Blender reflectivity/transparency
    double localWeight = std::max(0.0, 1.0 - reflectivity - transparency);

    Vec3 finalColor =
        localColor     * localWeight +
        reflectedColor * reflectivity +
        refractedColor * transparency;

    return clampVec3(finalColor);
}

// Main ray tracing function
Vec3 traceRay(const Ray& ray,
              const std::unique_ptr<BVHNode>& bvhRoot,
              const std::vector<Primitive>& prims,
              const std::vector<Sphere>& spheres,
              const std::vector<Plane>& planes,
              const std::vector<Cube>& cubes,
              const std::vector<int>& primOrder,
              const std::vector<Light>& lights,
              const Vec3& cameraLocation,
              int maxDepth,
              const RenderConfig &cfg)
{
    return traceRayRecursive(ray, bvhRoot, prims, spheres, planes, cubes,
                             primOrder, lights, cameraLocation, 0, maxDepth, cfg);
}