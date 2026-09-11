import struct
import math

def write_binary_stl(filename, triangles):
    """Writes a standard binary STL file accepted by all 3D printer slicers."""
    with open(filename, 'wb') as f:
        # 80-byte header
        header = b'Smart Glasses Prototype STL - Generated for 3D Printing'
        header = header.ljust(80, b' ')
        f.write(header)
        
        # Total facet count
        f.write(struct.pack('<I', len(triangles)))
        
        # Facets
        for normal, v1, v2, v3 in triangles:
            f.write(struct.pack('<3f', *normal))
            f.write(struct.pack('<3f', *v1))
            f.write(struct.pack('<3f', *v2))
            f.write(struct.pack('<3f', *v3))
            f.write(struct.pack('<H', 0)) # Attribute byte count

def add_box(triangles, x0, x1, y0, y1, z0, z1):
    """Generates 12 triangles representing an outward-facing rectangular solid."""
    p = [
        (x0, y0, z0), (x1, y0, z0), (x1, y1, z0), (x0, y1, z0),
        (x0, y0, z1), (x1, y0, z1), (x1, y1, z1), (x0, y1, z1)
    ]
    # (v1, v2, v3, v4, normal)
    faces = [
        (p[0], p[3], p[2], p[1], (0, 0, -1)), # Bottom
        (p[4], p[5], p[6], p[7], (0, 0, 1)),  # Top
        (p[0], p[1], p[5], p[4], (0, -1, 0)), # Front
        (p[2], p[3], p[7], p[6], (0, 1, 0)),  # Back
        (p[0], p[4], p[7], p[3], (-1, 0, 0)), # Left
        (p[1], p[2], p[6], p[5], (1, 0, 0))   # Right
    ]
    for v1, v2, v3, v4, norm in faces:
        triangles.append((norm, v1, v2, v3))
        triangles.append((norm, v1, v3, v4))

def build_front_frame():
    """Builds front frame according to Sheets 2 & 4 dimensions (145x48x6 mm)."""
    triangles = []
    # Outer End Blocks (Left & Right Hinge Wings)
    add_box(triangles, -72.5, -59.0, -24.0, 24.0, 0.0, 6.0)
    add_box(triangles,  59.0,  72.5, -24.0, 24.0, 0.0, 6.0)

    # Left Lens Rim (Top & Bottom bars around 50x38 mm cutout)
    add_box(triangles, -59.0,  -9.0,  19.0, 24.0, 0.0, 6.0) # Top
    add_box(triangles, -59.0,  -9.0, -24.0, -19.0, 0.0, 6.0) # Bottom

    # Right Lens Rim (Top & Bottom bars around 50x38 mm cutout)
    add_box(triangles,   9.0,  59.0,  19.0, 24.0, 0.0, 6.0) # Top
    add_box(triangles,   9.0,  59.0, -24.0, -19.0, 0.0, 6.0) # Bottom

    # Bridge Structure (18 mm wide: X from -9 to +9)
    add_box(triangles,  -9.0,   9.0,  16.0, 24.0, 0.0, 6.0) # Top bridge arch
    add_box(triangles,  -9.0,   9.0, -24.0, -10.0, 0.0, 6.0) # Bottom nose arch
    # Left & Right bridge pillars flanking sensor ports
    add_box(triangles,  -9.0,  -4.5, -10.0, 16.0, 0.0, 6.0)
    add_box(triangles,   4.5,   9.0, -10.0, 16.0, 0.0, 6.0)
    # Bridge middle separator (Leaves 8mm camera hole above, 6x6mm ToF port below)
    add_box(triangles,  -4.5,   4.5,   3.0,  6.0, 0.0, 6.0)

    # Left & Right Hinge Lugs (for M2 screw / 1.75mm filament pin)
    add_box(triangles, -75.5, -72.5,  -5.5,  5.5, 0.0, 6.0)
    add_box(triangles,  72.5,  75.5,  -5.5,  5.5, 0.0, 6.0)
    return triangles

def build_temple_arm():
    """Builds 140 mm temple arm with ergonomic ear hook (Sheet 3 & 4)."""
    triangles = []
    # Main straight temple body (11 mm height x 6 mm depth x 100 mm length)
    add_box(triangles, 0.0, 100.0, 0.0, 11.0, 0.0, 6.0)

    # Ergonomic ear taper (angled transition)
    add_box(triangles, 100.0, 125.0, -5.0,  9.0, 0.0, 5.0)
    add_box(triangles, 125.0, 140.0, -14.0,  4.0, 0.0, 4.5)

    # Hinge knuckle tab to interlock with front frame
    add_box(triangles, -4.0, 0.0, 2.75, 8.25, 0.0, 6.0)
    return triangles

if __name__ == '__main__':
    frame_tris = build_front_frame()
    write_binary_stl("smart_glasses_frame.stl", frame_tris)
    print(" Created: smart_glasses_frame.stl (145 mm Front Frame)")

    temple_tris = build_temple_arm()
    write_binary_stl("smart_glasses_temples.stl", temple_tris)
    print(" Created: smart_glasses_temples.stl (140 mm Temple Arms)")
    print("\nFiles are ready. Drag and drop them directly into your slicer!")