# Stephi Lyou
# Module 1 Part 1: Blender Export
# Export.py - Modified to export reflection and refraction properties

import bpy
from mathutils import *
import os

def get_material_properties(mat):
    """Extract Blinn-Phong properties from a Cycles material node tree."""
    if not mat or not mat.use_nodes:
        return None
    
    nt = mat.node_tree
    diffuse_node = None
    glossy_node = None
    mix_shader = None
    tex_image = None
    mix_rgb = None
    
    # Find nodes
    for node in nt.nodes:
        if node.type == 'BSDF_DIFFUSE':
            diffuse_node = node
        elif node.type == 'BSDF_GLOSSY':
            glossy_node = node
        elif node.type == 'MIX_SHADER':
            mix_shader = node
        elif node.type == 'TEX_IMAGE':
            tex_image = node
        elif node.type == 'MIX_RGB':
            mix_rgb = node
    
    props = {}
    
    # Get diffuse color (from MixRGB node input 2, which has your base color)
    if mix_rgb and len(mix_rgb.inputs[2].default_value) >= 3:
        props['diffuse'] = list(mix_rgb.inputs[2].default_value[:3])
    elif diffuse_node:
        props['diffuse'] = list(diffuse_node.inputs['Color'].default_value[:3])
    else:
        props['diffuse'] = [0.8, 0.8, 0.8]
    
    # Get specular color (from glossy)
    if glossy_node:
        props['specular'] = list(glossy_node.inputs['Color'].default_value[:3])
        roughness = glossy_node.inputs['Roughness'].default_value
        # Convert roughness back to shininess
        props['shininess'] = (1.0 / (roughness ** 2)) if roughness > 0.001 else 2048.0
    else:
        props['specular'] = [1.0, 1.0, 1.0]
        props['shininess'] = 80.0
    
    # Get specular strength (mix factor)
    if mix_shader:
        props['reflectivity'] = mix_shader.inputs['Fac'].default_value
    else:
        props['reflectivity'] = 0.0
    
    
    props['transparency'] = 0.0
    props['refractive_index'] = 1.0
    
    return props

def get_texture_path_from_material(mat):
    """Extract texture file path from material's Image Texture node."""
    if not mat or not mat.use_nodes:
        return ""
    
    nt = mat.node_tree
    for node in nt.nodes:
        if node.type == 'TEX_IMAGE' and node.image is not None:
            # Get absolute path
            abs_path = bpy.path.abspath(node.image.filepath)
            
            # Convert JPG to PPM if needed
            if abs_path.lower().endswith('.jpg') or abs_path.lower().endswith('.jpeg'):
                # Create PPM filename in same directory
                base_name = os.path.splitext(abs_path)[0]
                ppm_path = base_name + '.ppm'
                
                # Convert if PPM doesn't exist or is older than JPG
                if not os.path.exists(ppm_path) or \
                   os.path.getmtime(abs_path) > os.path.getmtime(ppm_path):
                    convert_jpg_to_ppm(abs_path, ppm_path, format='P6')
                
                return ppm_path
            else:
                # Already PPM or other format
                return abs_path
    
    return ""

def convert_jpg_to_ppm(jpg_path, ppm_path, format='P6'):
    """
    Convert JPG to PPM using Blender's image API.
    format: 'P6' (binary) or 'P3' (ASCII)
    """
    import bpy
    import struct
    
    # Load the image (or get existing)
    img = None
    if jpg_path in bpy.data.images:
        img = bpy.data.images[jpg_path]
    else:
        img = bpy.data.images.load(jpg_path)
    
    if img is None:
        print(f"❌ Failed to load {jpg_path}")
        return False
    
    width = img.size[0]
    height = img.size[1]
    
    # Get pixel data as flat list [R,G,B,A, R,G,B,A, ...]
    pixels = list(img.pixels)
    
    # Open output file
    mode = 'wb' if format == 'P6' else 'w'
    with open(ppm_path, mode) as f:
        # Write PPM header
        if format == 'P6':
            f.write(f"{format}\n{width} {height}\n255\n".encode('ascii'))
        else:
            f.write(f"{format}\n{width} {height}\n255\n")
        
        # Write pixel data (RGB only, skip alpha)
        if format == 'P6':
            # Binary format
            rgb_bytes = bytearray()
            for i in range(0, len(pixels), 4):
                r = int(pixels[i] * 255)
                g = int(pixels[i+1] * 255)
                b = int(pixels[i+2] * 255)
                rgb_bytes.extend([r, g, b])
            f.write(rgb_bytes)
        else:
            # ASCII format (P3)
            for i in range(0, len(pixels), 4):
                r = int(pixels[i] * 255)
                g = int(pixels[i+1] * 255)
                b = int(pixels[i+2] * 255)
                f.write(f"{r} {g} {b}\n")
    
    print(f"✅ Converted {jpg_path} → {ppm_path} ({format})")
    return True

# Function to export to ascii
def export_to_ascii(filepath):
    
    # Open the file to write
    with open(filepath, 'w') as f:
        
        # Go through all the objects in the scene
        for obj in bpy.context.scene.objects:
            
            # Write object type and name
            f.write(f"Type: {obj.type}\n")
            f.write(f"Name: {obj.name}\n")
            
            # Camera information
            if obj.type == 'CAMERA':
                f.write(f"Location: {list(obj.location)}\n")
                gaze = -(obj.matrix_world.to_3x3() @ Vector((0.0, 0.0, 1.0)))
                up = obj.matrix_world.to_3x3() @ Vector((0.0, 1.0, 0.0))
                f.write(f"Gaze Vector: {list(gaze)}\n")
                f.write(f"Up Vector: {list(up)}\n")
                f.write(f"Focal Length: {obj.data.lens}\n")
                f.write(f"Sensor Width: {obj.data.sensor_width}\n")
                f.write(f"Sensor Height: {obj.data.sensor_height}\n")
                res_x = bpy.context.scene.render.resolution_x
                res_y = bpy.context.scene.render.resolution_y
                f.write(f"Resolution X: {res_x}\n")
                f.write(f"Resolution Y: {res_y}\n")
                f.write(f"\n")
                
            # Point light information
            if obj.type == 'LIGHT' and obj.data.type == 'POINT':
                f.write(f"Location: {list(obj.location)}\n") 
                f.write(f"Color: {list(obj.data.color)}\n")               
                f.write(f"Radiant Intensity: {obj.data.energy}\n")
                f.write(f"\n")
                
            # Mesh objects (including spheres, cubes, and planes)
            if obj.type == 'MESH':
                f.write(f"Location: {list(obj.location)}\n")
                f.write(f"Rotation: {list(obj.rotation_euler)}\n")
                f.write(f"Scale: {list(obj.scale)}\n")
                
                mat = obj.data.materials[0] if obj.data.materials else None
                props = get_material_properties(mat)
            
                if props:
                    f.write(f"Diffuse: [{props['diffuse'][0]}, {props['diffuse'][1]}, {props['diffuse'][2]}]\n")
                    f.write(f"Specular: [{props['specular'][0]}, {props['specular'][1]}, {props['specular'][2]}]\n")
                    f.write(f"Shininess: {props['shininess']}\n")
                    f.write(f"Reflectivity: {props['reflectivity']}\n")
                    f.write(f"Transparency: {props['transparency']}\n")
                    f.write(f"Refractive Index: {props['refractive_index']}\n")
            
                    
                texture_path = get_texture_path_from_material(mat)
                if texture_path:
                    f.write(f"Texture: {texture_path}\n")

                    
                # Export the plane corners if it's a plane
                if 'Plane' in obj.name:
                    vertices = [obj.matrix_world @ v.co for v in obj.data.vertices]
                    for i in range(len(vertices)):
                        f.write(f"Corner{i+1}: {list(vertices[i])}\n")
                    
                f.write(f"\n")
                
                
# Main function
def main():
    filename = bpy.path.abspath('//scene.txt')
    export_to_ascii(filename)
    print(f"Scene exported to {filename}")
    
if __name__ == "__main__":
    main()        