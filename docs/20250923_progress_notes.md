# 🎹 Piano Player Project — Lab Notes
**Date:** 2025-09-23  
**Author:** Mark de Villiers  

---

## Objective
Redesign the piano player mechanism to be **more stable** (prevent “walking” on the rubber keyboard), improve appearance, and prepare for a **3D-printed modular version**. The long-term aim is to achieve a rigid, reliable structure that can be printed at low cost and assembled in modules (≈1 octave each).

---

## Activities Today
- **Transitioned CAD tools**  
  - Began learning **FreeCAD** as an alternative to Fusion 360 (too expensive).  
  - Learned FreeCAD basics: creating sketches, pads, referencing planes, and capturing geometry.  

- **Captured baseline geometry**  
  - Modeled the prototype piano player frame (current dimensions).  
  - Confirmed the approximate overall size; unlikely to reduce footprint significantly, but improvements in **rigidity and aesthetics** are achievable.  

- **Parameterization**  
  - Created initial design parameters for keys, frame, and servos.  
  - Verified octave width, white/black key spacing, and depth relations.  

- **Next steps**  
  - Integrate these parameters into a FreeCAD **Spreadsheet** for parametric design.  
  - Begin modular design (1-octave frame with side cheeks, servo bar, and base plate).  
  - Test ShapeBinders to propagate master sketches into Bodies.  

---

## Parameters Defined
| Parameter | Value | Notes |
|-----------|-------|-------|
| `white_pitch` | 23 mm | White key spacing |
| `octave_width` | 161 mm | 7 × white_pitch |
| `piano_depth` | 161 mm | Depth of piano key area |
| `key_depth` | 131 mm | Active depth of white keys |
| `black_pitch_2` | 28 mm | Spacing for 2-black-key group |
| `black_pitch_3` | 27 mm | Spacing for 3-black-key group |
| `inter_black_pitch` | 39.5 mm | Gap between black key groups |
| `black_key_width` | 12 mm | Nominal width of black key |
| `white_key_width` | 20.5 mm | Nominal width of white key |
| `black_key_depth` | 80 mm | Depth of black key |
| `full_frame_depth` | 220 mm | Total depth of module |
| `full_frame_height` | 180 mm | Total height of module |
| `edge_offset` | 15 mm | Bolt hole offset |
| `rubber_height_clearance` | 15 mm | Rubber key + 5 mm clearance |
| `base_thickness` | 20 mm | Printed base plate |
| `shaft_length` | 125 mm | Servo shaft length requirement |
| `servo_travel` | 10 mm | Actuator stroke |
| `top_thickness` | 5 mm | Printed top plate |
| `servo_y_spacing` | 76.4 mm | Spacing between servo rows |

---

## Reflections
- FreeCAD is different from Fusion 360 but already feels workable for this project.  
- The prototype size may stay similar, but **parametric modeling** will let me refine structure, tolerances, and servo alignment.  
- The focus now is **rigidity + modularity** (1-octave units that bolt together cleanly).  

---

## Next Steps
- Build **Spreadsheet (Params)** in FreeCAD with the values above.  
- Create **master sketches** (XY for footprint, ZY for cheeks, bolt hole references).  
- Experiment with **ShapeBinders** to reuse sketches across Bodies.  
- Print a **test plate** with bolt holes to check tolerances (M3/M5, dowels).  
- Draft servo bar design: use `servo_y_spacing` + `white_pitch` to test array of cutouts.  

---
