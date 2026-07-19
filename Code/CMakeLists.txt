cmake_minimum_required(VERSION 3.16)
project(RayTracer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Get every .cpp file in this directory
file(GLOB SRC_FILES
    "*.cpp"
)

add_executable(raytracer ${SRC_FILES})
