// Stephi Lyou
// Module 3 Part 3: Textures
// ** texture.cpp **

#include "texture.h"

// Global texture storage
std::vector<Image> g_textures;
std::unordered_map<std::string, int> g_textureCache;

// Sample a texture at (u,v) coordinates
Vec3 sampleTexture(const Texture &tex, double u, double v)
{
    if (tex.width == 0 || tex.height == 0) return Vec3{1,1,1};

    u = u - floor(u); 
    v = v - floor(v);

    int x = int(u * (tex.width  - 1));
    int y = int((1.0 - v) * (tex.height - 1)); 

    int idx = y * tex.width + x;
    return tex.pixels[idx];
}

// Load a texture and return its ID
int loadTexture(const std::string& filepath) {
    // Check if already loaded
    auto it = g_textureCache.find(filepath);
    if (it != g_textureCache.end()) {
        return it->second;
    }
    
    // Load new texture
    Image tex;
    if (!tex.load(filepath)) {
        std::cerr << "Warning: Failed to load texture: " << filepath << std::endl;
        return -1;
    }
    
    int id = static_cast<int>(g_textures.size());
    g_textures.push_back(tex);
    g_textureCache[filepath] = id;
    
    std::cout << "Loaded texture: " << filepath << " (ID: " << id << ")\n";
    return id;
}
