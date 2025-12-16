// ===== Parameters =====
magnet_d = 35;
magnet_h = 25;
diameter_tolerance = 2; // extra safety tolerance

cyl_d = magnet_d + diameter_tolerance; // 37 mm
wall = 3;
slot_w = 7;

plate_w = 40;
plate_h = 45;           // Z-height of the top of the hook geometry
plate_t = 4;

hook_depth = 10;        // wood thickness
hook_drop = 45;         // drop length of the outer plate

screw_d = 3.5;          // wood screw
screw_spacing = 20;

overlap = 2;            // overlap between cylinder and plate

// ===== Derived =====
outer_d = cyl_d + wall * 2;
outer_h = magnet_h + 3; // Magnet height + floor thickness

// Z-alignment
// User Refinement 1: "Outside plate should be brought up to be flush with the top of the hook bridge."
// Let's define the absolute top of the model as `plate_h`.
// The hook bridge sits at the very top.
total_top_z = plate_h; 

// The magnet cup hangs from the "inner" side.
// Previous logic: Cup was aligned to the top.
// Let's keep the cup top flush with the bridge top, or slightly below? 
// "Move cylinder up so the top is flush with the top of the hook" (from prev request).
// So Cup Top = total_top_z.
cyl_z_floor = total_top_z - outer_h;

// Y-alignment
// Inner Plate Front Face at Y=0.
// Cylinder Center Y is offset to create overlap.
cyl_center_y = (outer_d / 2) - overlap;

difference() {
    union() {
        // --- Magnet Assembly ---
        
        // Magnet cup
        translate([0, cyl_center_y, cyl_z_floor])
            cylinder(d=outer_d, h=outer_h, $fn=64);

        // Mounting plate (Inner)
        // Extends from cup bottom to top.
        translate([-plate_w/2, -plate_t, cyl_z_floor])
            cube([plate_w, plate_t, outer_h]);

        // Refinement 3: Reinforcement
        // "Space between inside plate and cylinder... filled up to the beginning of the notch"
        // Fill the corner between the flat plate (y=0) and the cylinder.
        // We'll use a hull of the plate's back face and a slice of the cylinder, or just a block intersecting.
        // Simplest robust way: Hull the plate rectangle with the cylinder at the same Z, but restricted width?
        // No, that changes the shape too much.
        // Let's just add a block that connects them and let the cylinder shape dominate?
        // Actually, a simple cube bridging -plate_t to cyl_center_y, but restricted in X to plate_w.
        intersection() {
             translate([-plate_w/2, -plate_t, cyl_z_floor])
                cube([plate_w, cyl_center_y + outer_d/2, outer_h]); // Big block
             
             // Intersect with a shape that fills the gap but respects the cylinder curve?
             // Actually, just Hulling the plate and the Cylinder is the cleanest "fill".
             // But we want to preserve the cylinder's "roundness" on the far side?
             // The user specifically talked about the space "between inside plate and cylinder".
             // This implies the "air gap" near the corners of the plate.
             // Let's hull the plate with a rectangle inside the cylinder.
             hull() {
                 translate([-plate_w/2, -plate_t, cyl_z_floor])
                    cube([plate_w, plate_t/2, outer_h]); // The plate interface
                 translate([-plate_w/2, cyl_center_y, cyl_z_floor])
                    cube([plate_w, 0.1, outer_h]); // A plane at the center
             }
        }

        // --- Hook Assembly ---
        
        // Hook Bridge (Top lip)
        // Spans the gap (hook_depth).
        // Y: From -plate_t (Inner Back) to -plate_t - hook_depth (Outer Front).
        // Z: Top aligned with total_top_z. Thickness plate_t.
        translate([-plate_w/2, -plate_t - hook_depth, total_top_z - plate_t])
            cube([plate_w, hook_depth, plate_t]);

        // Hook Vertical Drop (Outer Plate)
        // Refinement 1: "Outside plate should be brought up to be flush with the top of the hook bridge."
        // Position X: -plate_w/2
        // Position Y: -plate_t - hook_depth - plate_t
        // Position Z: Top at total_top_z. Height = hook_drop.
        // Wait, if height is hook_drop, does it extend low enough?
        // User wants "flush with top".
        // Let's assume hook_drop defines the total length of this plate.
        translate([-plate_w/2, -plate_t - hook_depth - plate_t, total_top_z - hook_drop])
            cube([plate_w, plate_t, hook_drop]);
    }

    // --- Subtractions ---

    // Magnet cavity
    translate([0, cyl_center_y, cyl_z_floor + 3])
        cylinder(d=cyl_d, h=magnet_h + 10, $fn=64);

    // Wire slot
    // Cut through the reinforcement we just added.
    translate([-outer_d, cyl_center_y - slot_w/2, cyl_z_floor + 3])
        cube([outer_d, slot_w, magnet_h + 20]);

    // Screw holes (Outer Plate)
    // Refinement 2: "moved down, at least below the level of the bottom of the inside plate"
    // Bottom of inside plate is `cyl_z_floor`.
    // So Hole Z < cyl_z_floor.
    // cyl_z_floor = 45 - 28 = 17.
    // Holes at Z=10 would be good.
    
    hole_y_start = -plate_t - hook_depth - plate_t - 5;
    hole_len = plate_t + 10;
    
    hole_z = 8; // Safely below 17
    
    for (x = [-screw_spacing/2, screw_spacing/2]) {
        translate([x, hole_y_start, hole_z])
            rotate([-90,0,0])
                cylinder(d=screw_d, h=hole_len, $fn=24);
    }
}
