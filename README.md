# Ray Tracer
A simple C++ ray tracer that loads a text scene description, renders an image, and writes a PPM file. Rendering options (samples per pixel, recursion depth, BVH, anti‑aliasing) are controlled via command‑line flags.

## Build
From the project root:

- bash
- mkdir -p build
- cd build
- cmake ..
- make

Assuming the build produces an executable named raytracer in the build directory, you can then run it from there (or copy it to the project root).

## Basic usage
- bash
- ./raytracer --scene <scene_file> --out <output_ppm> [options]

## Required arguments

- --scene FILE
(Path to the text scene description (relative or absolute))

- --out FILE
(Path for the output image in PPM format (relative or absolute))

## Example

- bash
- ./raytracer --scene scenes/scene.txt --out renders/output.ppm

## Rendering options
These flags control quality and features:

- --spp N
  - Samples per pixel (integer). Higher values give smoother images but take longer to render.
  - Example: --spp 16

- --depth N
  - Maximum recursion depth for reflection/refraction rays. Larger values allow deeper bounces but increase render time.
  - Example: --depth 5

- --no-bvh
  - Disable the BVH acceleration structure. This is mainly for debugging; renders will be slower on complex scenes.

- --no-aa
  - Disable anti‑aliasing. The renderer will use a single sample per pixel regardless of --spp.

## Example commands
Render a scene with moderate quality:

bash ./raytracer --scene scenes/scene.txt --out renders/scene_spp16.ppm --spp 16 --depth 5

Render a quick, low‑quality preview (no BVH, no anti‑aliasing):

bash ./raytracer --scene scenes/scene.txt --out renders/preview.ppm --spp 1 --depth 2 --no-bvh --no-aa

## Help
You can print a short usage message from the executable itself:

bash ./raytracer --help

This will list the supported flags and their meanings.
