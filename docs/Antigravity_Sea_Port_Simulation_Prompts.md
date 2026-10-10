# Antigravity AI — Interactive 3D Sea Port Simulation Development Prompts

> Recommended execution order: 01 → 02 → 03 → 04 → 05 → 06 → 07 → 08 → 09

---

<!-- ========================================================= -->
<!-- 01 — COMPLETE PROJECT AUDIT + IMPROVEMENT BLUEPRINT       -->
<!-- ========================================================= -->

# 01 — Complete Project Audit + Improvement Blueprint

Act as a senior C++ OpenGL engineer, Computer Graphics researcher, simulation designer, 3D interaction designer, and gameplay systems architect.

I have an existing C++ OpenGL Sea Port / Port Simulation project.

Your first task is to deeply analyze the entire project and determine how it can be transformed into a significantly better interactive 3D sea-port simulation.

You are NOT restricted to preserving the current architecture.

You may:
- refactor the code,
- reorganize modules,
- replace weak implementations,
- improve existing systems,
- redesign data structures,
- improve rendering,
- improve camera behavior,
- replace inefficient logic,
- introduce better abstractions,
- add new systems where useful.

However, changes should be technically justified and should result in a cleaner and more professional project rather than unnecessary complexity.

Analyze:

1. Project architecture
2. main rendering loop
3. OpenGL initialization
4. object management
5. transformations
6. camera system
7. ship system
8. crane system
9. container system
10. trucks
11. bridge
12. water
13. lighting
14. shaders
15. keyboard/mouse input
16. animation
17. collision/proximity logic
18. scene organization
19. performance bottlenecks
20. visual weaknesses
21. interaction limitations
22. opportunities for simulation/gameplay

Then design a better target architecture.

The final experience should become:

A visually attractive, wide-view, interactive 3D sea-port simulation where the player can observe the entire port, operate machinery, interact with cargo, manage ships, and perform realistic simplified port operations.

It should feel somewhat like a simulation game, but gameplay should remain lightweight and academically appropriate for a Computer Graphics project.

Identify improvements in:

A. Rendering
B. Camera
C. Environment
D. Crane interaction
E. Cargo interaction
F. Ship behavior
G. Docking
H. Loading/unloading
I. Port traffic
J. UI/HUD
K. Visual feedback
L. Animation
M. Water
N. Lighting
O. Performance
P. Code architecture

Then produce a detailed implementation roadmap divided into sequential development phases.

Do not implement the complete redesign yet.

At the end, clearly define the ideal final version of the project.

---

<!-- ========================================================= -->
<!-- 02 — SCENE + PORT ENVIRONMENT UPGRADE                    -->
<!-- ========================================================= -->

# 02 — Transform the Scene into a Professional 3D Sea Port

Now significantly improve the visual quality and composition of the sea-port scene.

The goal is to make the project look like a complete, believable 3D port environment rather than a collection of isolated OpenGL objects.

You may restructure or replace parts of the existing scene when that produces a better result.

Create a coherent port layout containing:

- large water area
- main dock
- container yard
- cargo ship terminal
- cranes
- warehouses
- trucks
- container stacks
- roads
- dock platforms
- loading areas
- navigation lights
- port infrastructure
- safety barriers
- bollards
- storage zones
- industrial structures

Improve spatial organization so the scene has clear depth and hierarchy.

Create a visually strong wide composition:

Foreground:
- crane
- containers
- truck activity

Middle ground:
- dock
- cargo handling
- ship

Background:
- additional port structures
- distant ships
- cranes
- industrial skyline

Add environmental details such as:

- animated water
- subtle water waves
- reflections/specular highlights
- blinking warning lights
- dock lights
- container color variation
- realistic container stacking
- distant moving vessels

Use procedural geometry where appropriate instead of requiring external assets.

Improve:
- lighting
- shadows
- material properties
- color balance
- depth perception
- object scale
- scene composition

The environment should clearly communicate:
"This is an active sea port."

Do not simply add random objects.

Every object should contribute to the port environment or visual storytelling.

Target presentation quality:
- attractive from a wide camera
- attractive from close camera
- visually readable during interaction
- suitable for Computer Graphics demonstration
- stable real-time performance

---

<!-- ========================================================= -->
<!-- 03 — ADVANCED CAMERA + GAME-LIKE INTERACTION             -->
<!-- ========================================================= -->

# 03 — Build a Professional Interactive Camera System

Now improve the camera system so the sea-port simulation feels like a modern 3D interactive simulation.

The camera should provide both cinematic observation and practical interaction.

Create multiple camera modes:

1. FREE CAMERA
2. PORT OVERVIEW
3. CRANE OPERATOR CAMERA
4. SHIP CAMERA
5. DOCK CAMERA
6. TOP/STRATEGIC CAMERA

FREE CAMERA:
- mouse look
- keyboard movement
- adjustable movement speed
- smooth rotation
- adjustable field of view

PORT OVERVIEW:
- wide field of view
- entire port visible
- crane, ship, dock, yard and water visible simultaneously
- ideal for presentation

CRANE OPERATOR:
- follows crane operation area
- provides better visibility when controlling the crane
- automatically focuses on the relevant cargo area

SHIP CAMERA:
- follows or observes the active ship

DOCK CAMERA:
- focused on docking/loading operations

TOP CAMERA:
- strategic overview of the entire port

Add smooth transitions between camera modes.

Improve:
- perspective
- FOV
- camera clipping
- camera smoothing
- camera collision handling
- camera follow behavior
- zoom
- orbiting
- look-at functionality

Add cinematic camera behavior for important events:

- ship arriving
- ship docking
- crane picking cargo
- crane placing cargo
- ship leaving

These should remain subtle and should not interrupt user control unnecessarily.

The user should always understand:

Where am I?
Where is the ship?
Where is the crane?
Where is the active cargo?
Where is the destination?

Use an interaction-friendly visual language similar to simulation games, but keep the project academic and professional.

---

<!-- ========================================================= -->
<!-- 04 — SMART CRANE + REAL CARGO INTERACTION                -->
<!-- ========================================================= -->

# 04 — Advanced Interactive Crane and Cargo System

Turn the crane into one of the central interactive systems of the port simulation.

The crane should behave like a simplified real container crane.

Create a proper crane hierarchy:

CRANE BASE
→ ROTATING STRUCTURE
→ BOOM
→ TROLLEY
→ HOIST
→ CABLE
→ HOOK
→ CARGO

Allow the player to control:

- crane rotation
- trolley movement
- hook vertical movement
- optionally boom angle if appropriate
- pickup
- release

Movement should be:
- smooth
- frame-rate independent
- physically constrained
- visually connected

Implement cargo interaction using actual world-space positions.

Each container should have:

- unique ID
- position
- dimensions
- state
- source zone
- destination zone
- carried status
- cargo type if useful

Possible states:

YARD
SHIP
TARGETED
CARRIED
DELIVERED
MISPLACED

Pickup should depend on spatial proximity.

The hook must actually reach the cargo.

When picked up:

- cargo attaches to hook
- cargo follows hook
- cable/hook visually remains connected
- cargo orientation follows the crane system appropriately

When released:

- cargo is placed at the current world position
- determine whether placement is valid
- snap to a valid container slot where appropriate

Add intelligent placement behavior.

For example:

If placing a container on another container:
- detect correct stacking height
- align with storage grid
- snap into position

If placing onto ship:
- align with available ship cargo slots

This should make cargo handling feel much more realistic than simply moving cubes around.

Add visual feedback:
- highlight selected cargo
- highlight pickup range
- highlight destination
- show invalid areas
- show carried container
- show active target

---

<!-- ========================================================= -->
<!-- 05 — SHIP + DOCKING + PORT TRAFFIC SIMULATION            -->
<!-- ========================================================= -->

# 05 — Intelligent Ship Docking and Port Traffic

Transform the ship system into a dynamic port-operation simulation.

Create a ship lifecycle:

SPAWNING
→ APPROACHING
→ MANEUVERING
→ DOCKING
→ DOCKED
→ CARGO OPERATION
→ UNDOCKING
→ DEPARTING

Ships should not simply teleport between states.

Implement smooth movement and basic steering.

Add:
- approach path
- docking point
- docking alignment
- stopping behavior
- departure path

Create multiple ship types if practical:
- small cargo ship
- medium container ship
- large container ship

Each ship can have:
- size
- speed
- cargo capacity
- loading requirement
- unloading requirement
- docking position

Add a simple port traffic system.

Background ships may:
- cross the water
- enter/leave the port
- move along predefined paths

Active ship:
- receives the main player task
- docks at the main terminal
- becomes the focus of cargo operations

The system should avoid:
- ship overlap
- unrealistic movement
- sudden teleportation
- simultaneous conflicting docking

Use lightweight AI/path logic rather than a complicated navigation framework.

---

<!-- ========================================================= -->
<!-- 06 — FULL PORT OPERATION GAMEPLAY                        -->
<!-- ========================================================= -->

# 06 — Build the Complete Port Operation System

Build a complete interactive port-operation system around the ship, crane, cargo and dock systems.

The player should perform realistic simplified port tasks.

Create missions such as:

TASK A:
Unload containers from ship → yard

TASK B:
Load containers from yard → ship

TASK C:
Move containers between storage areas

TASK D:
Prepare a docked ship for departure

TASK E:
Complete a mixed loading/unloading operation

Each task should have:
- objective
- source
- destination
- required number of containers
- time/status information
- completion condition

Example:

SHIP 07
TASK: UNLOAD
TARGET: 3 CONTAINERS

The player must:

1. identify correct cargo
2. operate crane
3. pick cargo
4. transport cargo
5. place cargo correctly
6. repeat until task completion

Add task validation.

A task should only count when the actual cargo position and state satisfy the objective.

Add:
- task progress
- completion feedback
- penalties for incorrect actions
- optional time bonus
- completion bonus

Use a lightweight score system.

Do not turn the project into a complicated arcade game.

The primary focus remains:

simulation,
interaction,
visualization,
and Computer Graphics concepts.

Make task generation modular so additional mission types can be added later.

---

<!-- ========================================================= -->
<!-- 07 — HUD + VISUAL FEEDBACK + PROFESSIONAL UX             -->
<!-- ========================================================= -->

# 07 — Build a Professional Simulation HUD

Add a clean, modern HUD suitable for a 3D simulation.

The HUD should not dominate the scene.

Display:

PORT STATUS
SHIP STATUS
CURRENT TASK
CRANE MODE
TARGET CARGO
TASK PROGRESS
SCORE
OPTIONAL TIMER

Example:

────────────────────────────
SEA PORT OPERATIONS

SHIP: MV OCEAN STAR
STATUS: DOCKED

TASK: UNLOAD
PROGRESS: 2 / 4

CRANE: MANUAL
TARGET: CONTAINER C07

SCORE: 650
────────────────────────────

Add contextual prompts:

"PICK UP"
"DROP"
"TARGET IN RANGE"
"INVALID DROP"
"DOCKED"
"TASK COMPLETE"

Use colors and simple icons to communicate state.

Example concept:

Green  = valid
Yellow = active
Red    = invalid
Blue   = informational

Add a compact controls panel.

Controls should be discoverable without making the interface cluttered.

Provide visual feedback when:
- cargo is selected
- cargo is in pickup range
- destination is valid
- task is complete
- ship is docking
- crane is operating

Make the UX feel like a professional simulation rather than a debug application.

---

<!-- ========================================================= -->
<!-- 08 — ADVANCED VISUAL EFFECTS + GRAPHICS SHOWCASE         -->
<!-- ========================================================= -->

# 08 — Upgrade Graphics Quality and Effects

Upgrade the rendering quality so the project demonstrates strong Computer Graphics techniques.

Improve:

1. Lighting
2. Materials
3. Shadows
4. Water
5. Reflections
6. Fog/depth
7. Sky/environment
8. Object highlighting
9. Animated lights
10. Depth and atmosphere

Create day/evening atmosphere if practical.

Use the existing shader system or redesign it if necessary.

Add subtle effects:

- water movement
- specular reflections
- fog over distant water
- dock light glow
- warning lights
- crane indicator lights
- ship navigation lights
- container material variation

Do not overuse post-processing.

The visual goal is:

clean,
real-time,
3D,
industrial,
immersive,
professional.

Make sure the scene still runs smoothly on modest hardware.

Optimize:
- draw calls where practical
- object reuse
- unnecessary transformations
- repeated geometry
- shader overhead
- update calculations

Do not sacrifice interaction responsiveness for visual effects.

---

<!-- ========================================================= -->
<!-- 09 — FINAL PROFESSIONALIZATION + QA                      -->
<!-- ========================================================= -->

# 09 — Final Professionalization Pass

Perform a final professionalization pass on the entire Sea Port Simulation project.

Evaluate the project as if it were being submitted for a university Computer Graphics project and demonstrated live to an examiner.

The final system should communicate all of the following within the first few seconds:

- this is a 3D sea port,
- ships can arrive and dock,
- cranes handle cargo,
- containers move physically,
- the user can interact with the environment,
- the port is alive and animated,
- the simulation has clear operational objectives.

Review the entire project and improve anything that feels:

- unfinished
- visually weak
- confusing
- repetitive
- unrealistic
- mechanically awkward
- difficult to control
- poorly organized
- unnecessarily slow

Improve the project wherever necessary.

You may refactor or replace existing implementations when doing so produces a meaningful improvement.

Prioritize:

1. visual quality
2. interaction quality
3. simulation realism
4. camera experience
5. animation quality
6. code quality
7. performance
8. presentation quality

The final demo should support a smooth presentation sequence:

START
→ wide port overview
→ ship approaches
→ ship docks
→ player switches to crane operation
→ selects cargo
→ operates crane
→ picks container
→ container follows crane
→ cargo is transported
→ container is placed correctly
→ task progress updates
→ remaining cargo is handled
→ ship task completes
→ ship departs
→ next ship arrives

Add polish where it has high visual or interactive impact.

Remove bugs, broken interactions, awkward transitions and unnecessary debug behavior.

Perform a clean build and verify that the final project compiles and runs correctly.

Finally provide:

1. Final feature list
2. Final controls
3. Main systems/architecture
4. Graphics techniques demonstrated
5. Interaction features
6. Performance considerations
7. Recommended 2–3 minute live demo sequence

The result should feel like a polished interactive 3D sea-port simulation, not merely a collection of OpenGL primitives.
