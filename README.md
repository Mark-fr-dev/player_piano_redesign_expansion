
Player-Piano (FreeCAD Redesign and extension)

## Overview
This project is a **redesign of my earlier player-piano prototype**, moving from a wooden/acrylic build into a **parametric FreeCAD model** suitable for 3D printing.  
The aim is to create a **stable, modular frame** that can drive ~44 keys of a roll-up piano keyboard without “walking” across the surface. Later I plan to run it from a raspbery pi, which will ultimately be controlled via an AWS enabled server from an SAP HANA database.

- ✅ Open-source, parametric CAD design in **FreeCAD**  
- ✅ Modular approach: ~1 octave per frame section, bolted together  
- ✅ Arduino-driven servos (firmware in [separate repo](https://github.com/yourname/arduino-repo))  
- ✅ Documentation of design process via daily **lab notes**

---

## Project Goals
- Eliminate instability of the first prototype (prevent frame “walking”).  
- Create a **printable modular system** (1-octave units, chained to 44 keys).  
- Make the design **parameter-driven** (adjustable key pitch, frame spacing, servo layout).  
- Improve aesthetics and reproducibility (cleaner than the prototype).  

---

## Repository Structure
freecad/ # .FCStd parametric design files
stl/ # STL exports for 3D printing
notes/ # Daily lab notes in Markdown
images/ # Screenshots, diagrams, renders
README.md # Project overview (this file)

---

## Parameters (current baseline)
These are defined in a FreeCAD **Spreadsheet (Params)** and drive the master sketches:

| Parameter             | Value   | Notes |
|-----------------------|---------|--------------------------|
| `white_pitch`         | 23 mm   | White key spacing |
| `octave_width`        | 161 mm  | 7 × white_pitch |
| `piano_depth`         | 161 mm  | Depth of active key area |
| `black_pitch_2`       | 28 mm   | 2-black-key group width |
| `black_pitch_3`       | 27 mm   | 3-black-key group width |
| `inter_black_pitch`   | 39.5 mm | Gap between black groups |
| `full_frame_depth`    | 220 mm  | Total module depth |
| `full_frame_height`   | 180 mm  | Total module height |
| `servo_y_spacing`     | 76.4 mm | Servo row spacing |

(See [/notes](notes/) for full parameter tables and daily updates.)

---

## Build Concept
- **Frame**: Each octave = base plate (XY) + side cheeks (YZ), joined by bolts/dowels.  
- **Servo bar**: Spanning between cheeks, aligned to white/black key offsets.  
- **Modularity**: 4 modules chained → 44 notes total.  
- **Fasteners**:  
  - Z-direction bolts: clamp stacked frame layers.  
  - X-direction bolts: join octaves side-to-side.  

---

## Previous Prototype
The original player-piano prototype was built using **wood and perspex, with Fusion 360 CAD**.  
Documentation (sketches and videos) is preserved in the repo: [player-piano (prototype)](https://github.com/yourname/player-piano).  

---

## Firmware
Servo driving software is maintained separately:  
👉 [arduino-player-piano](https://github.com/yourname/arduino-repo)  

---

## Progress Tracking
- Daily lab notes: [/notes](notes/)  
- Screenshots/renders: [/images](images/)  

---

## License
[MIT](LICENSE) — free to use, modify, and build upon.

---
=======
# player_piano_redesign_expansion
Redesign of my piano-player for 3D printing, raspberry pie and online song selection

