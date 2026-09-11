// =============================================================================
// SMART GLASSES FOR VISUALLY IMPAIRED PEOPLE - 3D PRINTABLE PROTOTYPE
// Modeled to exact blueprint specs (Sheets 1–4):
// - Overall Frame Width: 145 mm | Rim Thickness: 6 mm
// - Lens Aperture: 50 mm x 38 mm | Bridge Width: 18 mm
// - Temple Arms: 140 mm length x 11 mm height x 6 mm depth
// - Camera aperture (8 mm dia), ToF sensor pocket (6x6 mm), Battery bay (34x9x4 mm)
// =============================================================================

$fn = 60; // Smooth curves for 3D printing

/* [Print Configuration] */
// Select what part to render for export:
// "assembly"  : Worn assembly preview
// "print_bed" : All parts laid flat for printing together
// "frame"     : Front frame only
// "left_arm"  : Left temple arm only
// "right_arm" : Right temple arm only
PART_TO_RENDER = "print_bed"; // ["assembly", "print_bed", "frame", "left_arm", "right_arm"]

/* [Dimensions from Blueprint] */
FRAME_WIDTH       = 145.0;
FRAME_HEIGHT      = 48.0;
FRAME_THICKNESS   = 6.0;

LENS_WIDTH        = 50.0;
LENS_HEIGHT       = 38.0;
BRIDGE_WIDTH      = 18.0;
CORNER_RADIUS     = 5.0;

CAM_HOLE_DIA      = 8.2;   // 8 mm + 0.2 mm clearance
TOF_SENSOR_SIZE   = 6.2;   // 6 mm + 0.2 mm clearance
LED_HOLE_DIA      = 3.0;   // Indicator LED hole
BTN_HOLE_DIA      = 4.0;   // Tactile button opening

TEMPLE_LEN        = 140.0;
TEMPLE_HEIGHT     = 11.0;
TEMPLE_THICKNESS  = 6.0;

HINGE_PIN_DIA     = 1.9;   // Sized for standard 1.75mm filament strand or M2 screw

// --- Helper: 2D Rounded Rectangle ---
module rounded_rect(w, h, r) {
    hull() {
        translate([-w/2 + r, -h/2 + r]) circle(r=r);
        translate([ w/2 - r, -h/2 + r]) circle(r=r);
        translate([-w/2 + r,  h/2 - r]) circle(r=r);
        translate([ w/2 - r,  h/2 - r]) circle(r=r);
    }
}

// =============================================================================
// 1. FRONT FRAME MODULE
// =============================================================================
module frame_front() {
    lens_offset_x = (BRIDGE_WIDTH + LENS_WIDTH) / 2;

    difference() {
        union() {
            // Main frame chassis
            linear_extrude(height = FRAME_THICKNESS) {
                rounded_rect(FRAME_WIDTH, FRAME_HEIGHT, 7);
            }
            // Hinge lugs on Left and Right sides
            for (side = [-1, 1]) {
                translate([side * (FRAME_WIDTH/2 - 2), 0, FRAME_THICKNESS/2])
                    cube([4, TEMPLE_HEIGHT, FRAME_THICKNESS], center=true);
            }
        }

        // Left Lens Aperture
        translate([-lens_offset_x, 0, -1])
            linear_extrude(height = FRAME_THICKNESS + 2)
                rounded_rect(LENS_WIDTH, LENS_HEIGHT, CORNER_RADIUS);

        // Right Lens Aperture
        translate([lens_offset_x, 0, -1])
            linear_extrude(height = FRAME_THICKNESS + 2)
                rounded_rect(LENS_WIDTH, LENS_HEIGHT, CORNER_RADIUS);

        // Feature 1: Forward Camera Module Aperture (8 mm dia) - Bridge Upper Area
        translate([0, 10, -1])
            cylinder(d=CAM_HOLE_DIA, h=FRAME_THICKNESS + 2);

        // Feature 2: Time-of-Flight (ToF) Distance Sensor (6x6 mm) - Bridge Center Area
        translate([-TOF_SENSOR_SIZE/2, -1 - TOF_SENSOR_SIZE/2, -1])
            cube([TOF_SENSOR_SIZE, TOF_SENSOR_SIZE, FRAME_THICKNESS + 2]);

        // Feature 3: Status Indicator LED (Top of Left Rim)
        translate([-lens_offset_x + 10, FRAME_HEIGHT/2 - 4, -1])
            cylinder(d=LED_HOLE_DIA, h=FRAME_THICKNESS + 2);

        // Feature 4: Tactile Control Button Opening (Bottom of Right Rim)
        translate([lens_offset_x + 18, -FRAME_HEIGHT/2 + 4, -1])
            cylinder(d=BTN_HOLE_DIA, h=FRAME_THICKNESS + 2);

        // Hinge Pin Holes (Left & Right)
        for (side = [-1, 1]) {
            translate([side * (FRAME_WIDTH/2 - 1.5), 0, -1])
                cylinder(d=HINGE_PIN_DIA, h=FRAME_THICKNESS + 2);
        }

        // Hinge clearance slot for temple knuckle
        for (side = [-1, 1]) {
            translate([side * (FRAME_WIDTH/2 - 1), 0, FRAME_THICKNESS/2])
                cube([6, TEMPLE_HEIGHT + 0.6, FRAME_THICKNESS/2 + 0.2], center=true);
        }
    }
}

// =============================================================================
// 2. TEMPLE ARM MODULE (Left / Right mirrored)
// =============================================================================
module temple_arm(is_right = false) {
    mirror([0, is_right ? 1 : 0, 0]) {
        difference() {
            union() {
                // Main arm body
                hull() {
                    // Front hinge section
                    translate([0, 0, 0])
                        cube([8, TEMPLE_THICKNESS, TEMPLE_HEIGHT]);
                    
                    // Main straight body
                    translate([95, 0, 0])
                        cube([1, TEMPLE_THICKNESS, TEMPLE_HEIGHT]);
                    
                    // Ergonomic ear contour
                    translate([TEMPLE_LEN - 10, -6, 0])
                        cube([10, TEMPLE_THICKNESS * 0.75, TEMPLE_HEIGHT * 0.7]);
                }
                
                // Mating Hinge Knuckle
                translate([0, TEMPLE_THICKNESS/2, TEMPLE_HEIGHT/4])
                    cylinder(d=TEMPLE_THICKNESS, h=TEMPLE_HEIGHT/2);
            }

            // Hinge Pin Hole
            translate([0, TEMPLE_THICKNESS/2, -1])
                cylinder(d=HINGE_PIN_DIA, h=TEMPLE_HEIGHT + 2);

            // Sheet 4 Internal Cavity: Battery Cell (34 mm x 9 mm x 4 mm deep)
            translate([75, 1.5, (TEMPLE_HEIGHT - 9)/2])
                cube([34, 4.2, 9.2]);

            // Sheet 3 Internal Cavity: AI SoC / GNSS / Wiring channel
            translate([15, 1.5, 2])
                cube([55, 3.5, TEMPLE_HEIGHT - 4]);

            // Sheet 4 Feature 5: Bone Conduction Transducer Pocket near ear
            translate([TEMPLE_LEN - 22, 1, 2])
                cube([10, 4.5, TEMPLE_HEIGHT - 4]);
        }
    }
}

// =============================================================================
// SCENE RENDER LOGIC
// =============================================================================
if (PART_TO_RENDER == "frame") {
    frame_front();
}
else if (PART_TO_RENDER == "left_arm") {
    temple_arm(false);
}
else if (PART_TO_RENDER == "right_arm") {
    temple_arm(true);
}
else if (PART_TO_RENDER == "assembly") {
    // 3D Assembled preview
    color([0.2, 0.25, 0.3]) frame_front();
    
    // Left Temple (rotated into open position)
    translate([-FRAME_WIDTH/2 + 2, 0, 0])
        rotate([90, 0, -90])
            color([0.28, 0.35, 0.45]) temple_arm(false);

    // Right Temple
    translate([FRAME_WIDTH/2 - 2, 0, 0])
        rotate([90, 0, 90])
            color([0.28, 0.35, 0.45]) temple_arm(true);
}
else if (PART_TO_RENDER == "print_bed") {
    // Laid flat for rapid 3D printing without supports
    translate([0, 0, 0]) frame_front();
    translate([0, 45, 0]) rotate([90, 0, 0]) temple_arm(false);
    translate([0, -45, 0]) rotate([-90, 0, 0]) temple_arm(true);
}