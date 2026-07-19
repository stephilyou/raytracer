// Stephi Lyou
// Module 1 Part 3: Image Read and Write
// ** image.cpp **

#include "image.h"

using namespace std;

// Constructors
Image::Image() : width(0), height(0), maxValue(255) {}

Image::Image(int w, int h)
    : width(w), height(h), maxValue(255) {
    pixels.resize(width * height * 3, 0);
}

// Set pixel color
void Image::setPixel(int x, int y, const Vec3& color) {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return;
    }
    int idx = (y * width + x) * 3;

    // Clamp to [0,1] to prevent underflow/overflow, then scale to [0,255]
    pixels[idx] = static_cast<unsigned char>(std::clamp(color.x, 0.0, 1.0) * 255);
    pixels[idx + 1] = static_cast<unsigned char>(std::clamp(color.y, 0.0, 1.0) * 255);
    pixels[idx + 2] = static_cast<unsigned char>(std::clamp(color.z, 0.0, 1.0) * 255);
}

// Save as PPM (supports P3 and P6)
void Image::save(const string& filename, const string& format) const {
    ofstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Could not open " << filename << " for writing.\n";
        return;
    }

    // Header
    file << format << "\n";
    file << width << " " << height << "\n";
    file << maxValue << "\n";

    if (format == "P6") {
        // Binary format
        file.write(reinterpret_cast<const char*>(pixels.data()), pixels.size());
    } else if (format == "P3") {
        // ASCII format
        for (int i = 0; i < width * height * 3; i += 3) {
            file << static_cast<int>(pixels[i]) << " "
                 << static_cast<int>(pixels[i + 1]) << " "
                 << static_cast<int>(pixels[i + 2]) << "\n";
        }
    } else {
        cerr << "Error: Unsupported PPM format. Use P3 or P6.\n";
    }

    file.close();
}

// Loads a PPM image (supports P3 or P6)
bool Image::load(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        cerr << "Error: Could not open " << filename << " for reading.\n";
        return false;
    }

    string format;
    file >> format;

    // Check format
    if (format != "P3" && format != "P6") {
        cerr << "Error: Unsupported PPM format. Only P3 and P6 are supported.\n";
        return false;
    }

    // Skip comments
    char c;
    file.get(c); // consume newline after format
    while (file.peek() == '#') {
        string comment;
        getline(file, comment);
    }

    // Read width, height, maxValue
    file >> width >> height >> maxValue;

    if (width <= 0 || height <= 0 || maxValue <= 0) {
        cerr << "Error: Invalid PPM dimensions or max value.\n";
        return false;
    }

    // Resize pixel buffer
    pixels.resize(width * height * 3);

    if (format == "P6") {
        // Binary format - read one more character (newline) before binary data
        file.get(c);
        
        // Read raw binary data
        file.read(reinterpret_cast<char*>(pixels.data()), pixels.size());
        
        if (!file) {
            cerr << "Error: Failed to read binary pixel data from " << filename << "\n";
            return false;
        }
    } else if (format == "P3") {
        // ASCII format
        for (size_t i = 0; i < pixels.size(); ++i) {
            int val;
            file >> val;
            if (!file) {
                cerr << "Error: Failed to read ASCII pixel data from " << filename << "\n";
                return false;
            }
            pixels[i] = static_cast<unsigned char>(std::clamp(val, 0, static_cast<int>(maxValue)));
        }
    }

    file.close();
    return true;
}

// Gets pixel color at (x, y)
Vec3 Image::getPixel(int x, int y) const {
    if (x < 0 || x >= width || y < 0 || y >= height) {
        return Vec3{0.0, 0.0, 0.0}; 
    }

    int idx = (y * width + x) * 3;
    double scale = 1.0 / maxValue;

    return Vec3{
        pixels[idx] * scale,
        pixels[idx + 1] * scale,
        pixels[idx + 2] * scale
    };
}

// Sample texture with bilinear filtering
Vec3 Image::sample(double u, double v) const {
    if (width == 0 || height == 0) {
        return Vec3{1.0, 1.0, 1.0}; // Default to white if empty
    }

    // Wrap UV coordinates
    u = u - floor(u);
    v = v - floor(v);

    // Flip V (standard texture convention)
    v = 1.0 - v;

    // Map to pixel coordinates
    double px = u * (width - 1);
    double py = v * (height - 1);

    int x0 = static_cast<int>(floor(px));
    int y0 = static_cast<int>(floor(py));
    int x1 = std::min(x0 + 1, width - 1);
    int y1 = std::min(y0 + 1, height - 1);

    // Bilinear interpolation weights
    double fx = px - x0;
    double fy = py - y0;

    Vec3 c00 = getPixel(x0, y0);
    Vec3 c10 = getPixel(x1, y0);
    Vec3 c01 = getPixel(x0, y1);
    Vec3 c11 = getPixel(x1, y1);

    // Interpolate
    Vec3 top = c00 * (1.0 - fx) + c10 * fx;
    Vec3 bot = c01 * (1.0 - fx) + c11 * fx;
    return top * (1.0 - fy) + bot * fy;
}