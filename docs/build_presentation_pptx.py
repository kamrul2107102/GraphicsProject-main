import os
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN, MSO_ANCHOR
from pptx.enum.shapes import MSO_SHAPE

# 16:9 Widescreen Presentation
prs = Presentation()
prs.slide_width = Inches(13.333)
prs.slide_height = Inches(7.5)
blank_layout = prs.slide_layouts[6]

# Theme Colors
BG_COLOR = RGBColor(11, 19, 43)        # #0B132B Deep Midnight Navy
CARD_BG = RGBColor(24, 34, 58)         # #18223A Card Background
BORDER_COLOR = RGBColor(45, 62, 98)    # Border line
CYAN = RGBColor(0, 229, 255)           # #00E5FF High-tech Cyan
GOLD = RGBColor(255, 209, 102)         # #FFD166 Golden Accent
WHITE = RGBColor(240, 244, 248)        # Crisp White
MUTED = RGBColor(176, 190, 205)        # Muted Steel Grey
GREEN = RGBColor(100, 255, 150)        # Success / Key badge

def apply_background(slide):
    bg_shape = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, 0, 0, prs.slide_width, prs.slide_height)
    bg_shape.fill.solid()
    bg_shape.fill.fore_color.rgb = BG_COLOR
    bg_shape.line.fill.background()
    return bg_shape

def add_header(slide, slide_num, category, title, subtitle):
    # Top banner bar
    accent_bar = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.8), Inches(0.4), Inches(11.733), Inches(0.04))
    accent_bar.fill.solid()
    accent_bar.fill.fore_color.rgb = CYAN
    accent_bar.line.fill.background()

    # Category tag & slide number
    cat_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.5), Inches(9.0), Inches(0.35))
    tf_c = cat_box.text_frame
    tf_c.word_wrap = True
    p_c = tf_c.paragraphs[0]
    p_c.text = f"CSE 4208 • COMPUTER GRAPHICS & MULTIMEDIA LAB  |  {category.upper()}"
    p_c.font.size = Pt(11)
    p_c.font.bold = True
    p_c.font.color.rgb = CYAN
    p_c.font.name = "Arial"

    num_box = slide.shapes.add_textbox(Inches(10.5), Inches(0.5), Inches(2.0), Inches(0.35))
    tf_n = num_box.text_frame
    p_n = tf_n.paragraphs[0]
    p_n.alignment = PP_ALIGN.RIGHT
    p_n.text = f"SLIDE {slide_num:02d} / 12"
    p_n.font.size = Pt(11)
    p_n.font.bold = True
    p_n.font.color.rgb = GOLD
    p_n.font.name = "Arial"

    # Slide Title
    title_box = slide.shapes.add_textbox(Inches(0.8), Inches(0.8), Inches(11.733), Inches(0.6))
    tf_t = title_box.text_frame
    tf_t.word_wrap = True
    p_t = tf_t.paragraphs[0]
    p_t.text = title
    p_t.font.size = Pt(22)
    p_t.font.bold = True
    p_t.font.color.rgb = WHITE
    p_t.font.name = "Arial"

    # Slide Subtitle
    if subtitle:
        sub_box = slide.shapes.add_textbox(Inches(0.8), Inches(1.35), Inches(11.733), Inches(0.35))
        tf_s = sub_box.text_frame
        tf_s.word_wrap = True
        p_s = tf_s.paragraphs[0]
        p_s.text = subtitle
        p_s.font.size = Pt(12)
        p_s.font.color.rgb = MUTED
        p_s.font.name = "Arial"

def add_card(slide, left, top, width, height, title=None, border_color=BORDER_COLOR):
    card = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, left, top, width, height)
    card.fill.solid()
    card.fill.fore_color.rgb = CARD_BG
    card.line.color.rgb = border_color
    card.line.width = Pt(1.2)

    if title:
        header_box = slide.shapes.add_textbox(left + Inches(0.2), top + Inches(0.15), width - Inches(0.4), Inches(0.35))
        tf = header_box.text_frame
        p = tf.paragraphs[0]
        p.text = title.upper()
        p.font.size = Pt(12)
        p.font.bold = True
        p.font.color.rgb = GOLD
        p.font.name = "Arial"
    return card

def add_bullet_points(slide, left, top, width, height, bullets):
    tb = slide.shapes.add_textbox(left, top, width, height)
    tf = tb.text_frame
    tf.word_wrap = True
    for i, b in enumerate(bullets):
        p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
        p.space_after = Pt(8)
        
        # Split prefix if bold
        if ":" in b:
            prefix, rest = b.split(":", 1)
            run1 = p.add_run()
            run1.text = "• " + prefix + ":"
            run1.font.bold = True
            run1.font.color.rgb = WHITE
            run1.font.size = Pt(12)
            run1.font.name = "Arial"

            run2 = p.add_run()
            run2.text = rest
            run2.font.bold = False
            run2.font.color.rgb = MUTED
            run2.font.size = Pt(12)
            run2.font.name = "Arial"
        else:
            run = p.add_run()
            run.text = "• " + b
            run.font.color.rgb = WHITE
            run.font.size = Pt(12)
            run.font.name = "Arial"

def add_viva_note(slide, left, top, width, height, text):
    card = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, left, top, width, height)
    card.fill.solid()
    card.fill.fore_color.rgb = RGBColor(16, 26, 48)
    card.line.color.rgb = CYAN
    card.line.width = Pt(1)

    tb = slide.shapes.add_textbox(left + Inches(0.15), top + Inches(0.1), width - Inches(0.3), height - Inches(0.2))
    tf = tb.text_frame
    tf.word_wrap = True

    p0 = tf.paragraphs[0]
    p0.text = "🎤 VIVA DEFENSE SCRIPT (WHAT TO SAY TO TEACHER):"
    p0.font.bold = True
    p0.font.size = Pt(10)
    p0.font.color.rgb = CYAN
    p0.font.name = "Arial"

    p1 = tf.add_paragraph()
    p1.text = f'"{text}"'
    p1.font.italic = True
    p1.font.size = Pt(11)
    p1.font.color.rgb = WHITE
    p1.font.name = "Georgia"

def add_image_or_placeholder(slide, left, top, width, height, img_path, placeholder_text, key_action=None):
    if img_path and os.path.exists(img_path):
        slide.shapes.add_picture(img_path, left, top, width, height)
    else:
        # Placeholder box
        box = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, left, top, width, height)
        box.fill.solid()
        box.fill.fore_color.rgb = RGBColor(18, 28, 50)
        box.line.color.rgb = GOLD
        box.line.width = Pt(1.5)

        tb = slide.shapes.add_textbox(left + Inches(0.2), top + Inches(0.4), width - Inches(0.4), height - Inches(0.8))
        tf = tb.text_frame
        tf.word_wrap = True
        
        p0 = tf.paragraphs[0]
        p0.alignment = PP_ALIGN.CENTER
        p0.text = "📷 SCREENSHOT PLACEHOLDER"
        p0.font.bold = True
        p0.font.size = Pt(13)
        p0.font.color.rgb = GOLD

        p1 = tf.add_paragraph()
        p1.alignment = PP_ALIGN.CENTER
        p1.space_before = Pt(6)
        p1.text = placeholder_text
        p1.font.size = Pt(11)
        p1.font.color.rgb = WHITE

        if key_action:
            p2 = tf.add_paragraph()
            p2.alignment = PP_ALIGN.CENTER
            p2.space_before = Pt(8)
            p2.text = f"KEY PRESS: {key_action}"
            p2.font.bold = True
            p2.font.size = Pt(11)
            p2.font.color.rgb = GREEN

# ==================== SLIDE 1: TITLE SLIDE ====================
s1 = prs.slides.add_slide(blank_layout)
apply_background(s1)

# Large Center Decorative Box
title_card = s1.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(1.2), Inches(1.0), Inches(10.933), Inches(5.5))
title_card.fill.solid()
title_card.fill.fore_color.rgb = CARD_BG
title_card.line.color.rgb = CYAN
title_card.line.width = Pt(2.0)

tb1 = s1.shapes.add_textbox(Inches(1.6), Inches(1.3), Inches(10.133), Inches(4.9))
tf1 = tb1.text_frame
tf1.word_wrap = True

p = tf1.paragraphs[0]
p.alignment = PP_ALIGN.CENTER
p.text = "KHULNA UNIVERSITY OF ENGINEERING & TECHNOLOGY (KUET)"
p.font.size = Pt(14)
p.font.bold = True
p.font.color.rgb = CYAN

p = tf1.add_paragraph()
p.alignment = PP_ALIGN.CENTER
p.text = "DEPARTMENT OF COMPUTER SCIENCE AND ENGINEERING"
p.font.size = Pt(12)
p.font.color.rgb = MUTED
p.space_after = Pt(20)

p = tf1.add_paragraph()
p.alignment = PP_ALIGN.CENTER
p.text = "Interactive 3D Sea Port Simulation with\nHierarchical Logistics Mechanics & Screen-Space Ray Tracing"
p.font.size = Pt(25)
p.font.bold = True
p.font.color.rgb = WHITE
p.space_after = Pt(18)

p = tf1.add_paragraph()
p.alignment = PP_ALIGN.CENTER
p.text = "Course: CSE 4208 (Computer Graphics and Multimedia Laboratory)"
p.font.size = Pt(14)
p.font.bold = True
p.font.color.rgb = GOLD
p.space_after = Pt(24)

p = tf1.add_paragraph()
p.alignment = PP_ALIGN.CENTER
p.text = "Core Features: Blinn-Phong Shading | Gouraud vs. Phong | Procedural Quadrics | 6-Level STS Crane | Real-Time Whitted Ray Tracer"
p.font.size = Pt(12)
p.font.color.rgb = WHITE

# ==================== SLIDE 2: ARCHITECTURE ====================
s2 = prs.slides.add_slide(blank_layout)
apply_background(s2)
add_header(s2, 2, "System Architecture", "Custom 3D Rendering Pipeline & Scene Synthesis", "Core OpenGL Architecture, Perspective Viewing Matrix, and Coordinate Transformations")
add_card(s2, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Pipeline Specifications")
add_bullet_points(s2, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Modern Core Profile: Pure OpenGL 3.3 Core with GLSL 330 Shaders (Zero legacy fixed-function pipeline).",
    "Perspective Projection: glm::perspective with 52° vertical FOV, aspect ratio 16:9, near 0.1m, far 320m.",
    "Dual Transformation Pipeline: World-to-Camera View Matrix and Model Space Local Hierarchies.",
    "Multi-Primitive Synthesis: Procedural VAO/VBO creation for cubes, cylinders, quadric cones, rings, sky dome, and water grids.",
    "Hardware Resource Management: Strict zero-leak VBO binding with depth testing enabled."
])
add_viva_note(s2, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, our rendering engine is written in C++17 and OpenGL 3.3 Core using GLAD and GLFW. Every frame executes modern vertex and fragment shaders with a 52-degree perspective projection matrix spanning 320 meters of harbor terrain without clipping.")
add_image_or_placeholder(s2, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), "report_assets/pipeline_flow.png", "System Architecture Flowchart")

# ==================== SLIDE 3: DIRECTIONAL LIGHT ====================
s3 = prs.slides.add_slide(blank_layout)
apply_background(s3)
add_header(s3, 3, "Illumination Model", "Physical Directional Lighting & Visible 3D Celestial Sun", "Mathematical Sunlight Vector Anchored by an Emissive 3D Sun Primitive")
add_card(s3, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Mathematical Formulation")
add_bullet_points(s3, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Physical Source Anchor: Visible 5.2m radiant sun core at (24.0, 275.0, 28.0) with golden corona halos.",
    "Directional Light Vector: L_sun = normalize(vec3(0.35, 0.88, 0.32)), illuminating all horizontal surfaces.",
    "Blinn-Phong Shading: Ambient (I_a * k_a) + Diffuse (I_d * k_d * max(N · L, 0)) + Specular (I_s * k_s * (N · H)^alpha).",
    "Atmospheric Distance Fog: Exponential depth fog blends far water horizon into sky dome color.",
    "Surface Radiance: Diffuse coefficient ensures clear contrast between vertical quay walls and asphalt decks."
])
add_viva_note(s3, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, our directional sunlight is physically anchored by an emissive 3D sun primitive at 275m altitude. This provides verifiable mathematical lighting with optimal diffuse shading across all horizontal surfaces.")
add_image_or_placeholder(s3, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), None, "Look UP at the 3D Sun in Daytime\nShow Golden Sun Core & Solar Corona Rings", "Mouse Look UP (Skyward)")

# ==================== SLIDE 4: GOURAUD VS PHONG ====================
s4 = prs.slides.add_slide(blank_layout)
apply_background(s4)
add_header(s4, 4, "Shading Models", "Gouraud vs. Phong Shading: Vertex vs. Fragment Evaluation", "Comparative Implementation Demonstrating Specular Fidelity and Mach Banding")
add_card(s4, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Technical Comparison")
add_bullet_points(s4, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Phong Shading (Key F2): Evaluated in phong.frag per-fragment. Interpolates surface normals for pinpoint Blinn-Phong specular highlights.",
    "Gouraud Shading (Key F1): Evaluated in gouraud.vert per-vertex. Linearly interpolates resulting vertex colors across rasterized triangles.",
    "Halfway Vector H: H = (L + V) / ||L + V|| replaces reflection vector R, optimizing real-time calculation.",
    "Specular Exponent (alpha = 32-128): Highlights metallic containers, water ripples, and crane structure.",
    "Visual Artifacts: Gouraud exhibits noticeable Mach banding and blurred highlights across low-poly surfaces."
])
add_viva_note(s4, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, pressing F1 and F2 dynamically switches between vertex-based Gouraud and fragment-based Phong shading. In Phong mode, normal vectors are interpolated per-fragment, eliminating Mach banding and producing realistic specular glints.")
add_image_or_placeholder(s4, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), "report_assets/gouraud_vs_phong.png", "Gouraud vs Phong Shading Diagram")

# ==================== SLIDE 5: MULTI-POINT LIGHTS ====================
s5 = prs.slides.add_slide(blank_layout)
apply_background(s5)
add_header(s5, 5, "Illumination Model", "Dynamic Day/Night Cycle & 7-Point Light Attenuation", "Nocturnal Lunar Source with Quadratic Distance Attenuation on Street Lamps")
add_card(s5, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Night Lighting Architecture")
add_bullet_points(s5, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Day/Night Cycle (Key N): Toggles sunlight to nocturnal moonlight with deep navy ambient sky dome.",
    "Luminous 3D Moon: Emissive sphere with craters acting as secondary night directional light source.",
    "7 Quayside Street Lamps: Sodium vapor point lights positioned along the 640m expressway.",
    "Quadratic Distance Attenuation: Atten = 1.0 / (k_c + k_l * d + k_q * d^2) where k_c=1.0, k_l=0.14, k_q=0.07.",
    "Light Pools: Illuminates asphalt roadways with natural circular falloff and specular roadway shine."
])
add_viva_note(s5, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, pressing N toggles our nocturnal lighting. Seven quayside street lamps cast sodium light pools using quadratic distance attenuation, while the visible 3D moon provides secondary directional radiance.")
add_image_or_placeholder(s5, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), "report_assets/light_attenuation.png", "Point Light Attenuation Curve")

# ==================== SLIDE 6: SPOTLIGHT FALLOFF ====================
s6 = prs.slides.add_slide(blank_layout)
apply_background(s6)
add_header(s6, 6, "Illumination Model", "Focused Crane Spotlight with Smooth Cosine Cutoff", "Trolley-Mounted Directional Cone Light for Low-Light Container Handling")
add_card(s6, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Spotlight Mathematics")
add_bullet_points(s6, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Dynamic Spatial Anchor: Spotlight origin pinned to crane trolley (8.0, 7.0, trolleyZ), tracking in real time.",
    "Beam Direction: Unit vector (0.0, -1.0, 0.0) projecting directly downward onto vessel cargo and dock bay.",
    "Cutoff Angles: Inner cutoff theta_inner = 18.0°, Outer cutoff theta_outer = 25.0° for smooth edge fade.",
    "Cosine Intensity Factor: I = clamp((cos(theta) - cos(theta_outer)) / (cos(theta_inner) - cos(theta_outer)), 0.0, 1.0).",
    "Operational Benefit: High-contrast illumination for precision alignment during nocturnal loading."
])
add_viva_note(s6, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, the crane trolley houses a downward spotlight with smooth cosine cutoff angles. As the operator drives the trolley, the spotlight tracks dynamically, projecting a sharp worklight beam over the active cargo." )
add_image_or_placeholder(s6, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), None, "Night Crane Spotlight Projection\nDownward Conical Light Beam on Vessel & Lorry", "Press N -> Look under Crane Trolley")

# ==================== SLIDE 7: PROCEDURAL PRIMITIVES ====================
s7 = prs.slides.add_slide(blank_layout)
apply_background(s7)
add_header(s7, 7, "Geometric Modeling", "Procedural 3D Geometric Primitives & Quadric Cones", "Mathematical Parametric Geometry Synthesis without External 3D Asset Files")
add_card(s7, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Primitive Algorithms")
add_bullet_points(s7, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Zero External Files: All meshes procedurally generated in C++ code using analytical math.",
    "Analytical Cone Quadric: Parametric vertex rings from apex (y=h) to base (y=0) with outward normals.",
    "Marine Channel Buoys: Starboard Green and Port Red conical floating buoys in the water canal.",
    "Roadway Traffic Cones: High-visibility orange cones with reflective white safety bands.",
    "Complete Suite: Cube (dock, warehouse), Cylinder (masts, legs), Sphere (buoys, sun, moon), Ring (lifebuoys)."
])
add_viva_note(s7, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, every object in this simulation is procedurally generated using analytical geometry formulas. You can see our Cone and Ring primitives functioning as maritime navigational buoys and roadway safety markers.")
add_image_or_placeholder(s7, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), None, "Conical Buoys on Water & Traffic Cones on Road\nShow analytical cone quadrics and circular bases", "Walk near Water Canal / Dock Road")

# ==================== SLIDE 8: HIERARCHICAL MODELING ====================
s8 = prs.slides.add_slide(blank_layout)
apply_background(s8)
add_header(s8, 8, "Kinematic Modeling", "6-Level Hierarchical Kinematics: Ship-to-Shore Crane", "Parent-Child Transformation Scene Graph with Coordinated Matrix Stacks")
add_card(s8, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Kinematic Hierarchy Tree")
add_bullet_points(s8, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Level 1 - Gantry Portal Base: T(8.0, 0.0, 8.35) portal legs spanning quayside roadway.",
    "Level 2 - Slewing Machinery Turret: R_y(slewingYaw) vertical rotation around Y-axis.",
    "Level 3 - Horizontal Boom Arm: 22-meter truss boom extending from canal fairway to dock apron.",
    "Level 4 - Mobile Trolley: T(0, -0.65, trolleyRelZ) traversing from Z=-1.5m to Z=15.5m.",
    "Level 5 - Hoist Wire Cables & Spreader: Dual cables dynamically scaled by cableLength with twistlock frame.",
    "Level 6 - Intermodal Cargo Container: Secured container transforms synchronously with spreader hook."
])
add_viva_note(s8, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, the STS crane represents a 6-level kinematic transformation tree. When the operator slews the cab or moves the trolley, all child nodes—cables, spreader, and cargo—inherit parent matrices synchronously.")
add_image_or_placeholder(s8, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), "report_assets/crane_hierarchy_tree.png", "Crane 6-Level Hierarchy Tree Diagram")

# ==================== SLIDE 9: 1ST-PERSON CABIN POV ====================
s9 = prs.slides.add_slide(blank_layout)
apply_background(s9)
add_header(s9, 9, "Interactive Mechanics", "1st-Person Operator Cockpit & Dynamic Sightline Zoom", "Repositioned Fairway Geometry (Z=0.5m) and Real-Time FOV Zoom (32° to 85°)")
add_card(s9, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Operator Interface & Geometry")
add_bullet_points(s9, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Vessel Repositioned to Z=0.5m: Solved windshield obstruction by providing an unobstructed 60° downward sightline.",
    "Dynamic Zoom System: Scroll wheel or Z/X keys smoothly adjust cabin FOV between 32° (close-up) and 85° (wide panoramic).",
    "Active Target Cargo System: Pulsing gold-to-green vertical laser beacon and bounding box highlights selected container.",
    "Panoramic Cabin POV: 360° mouse-look inside glass operator cockpit with dual illuminated joysticks and telemetry screens.",
    "HUD Telemetry: Real-time digital readouts for trolley travel (m), hoist altitude (m), payload status, and score."
])
add_viva_note(s9, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, pressing C places the user inside the 1st-person crane cockpit. We solved close-quarter camera obstruction by moving the vessel to Z=0.5m and adding dynamic FOV zoom via Z/X keys and mouse scroll.")
add_image_or_placeholder(s9, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), None, "1st-Person Cabin View looking down at Vessel\nShow Target Beacon, Cargo Stacks & Console", "Press C (Enter Cab) -> Zoom via Scroll/X")

# ==================== SLIDE 10: MULTI-PERSPECTIVE & LORRY ====================
s10 = prs.slides.add_slide(blank_layout)
apply_background(s10)
add_header(s10, 10, "Interactive Gameplay", "3-Perspective Multi-Camera & Cargo Lorry Delivery", "Tactical Orbit Overview, Precision Drop Cam, and Drivable Roadway Transport")
add_card(s10, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Camera Modes & Logistics Flow")
add_bullet_points(s10, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "3-Camera Cycling (Key V): Seamlessly toggles 1st-Person Cabin -> 3rd-Person Orbit Overview -> Precision Drop Cam.",
    "Tactical Orbit Overview: 360° orbital rotation with adjustable camera distance (10m to 65m) for presentation demos.",
    "Repositioned Unload Bay (Z=13.5m): Moved clear of gantry legs onto dock road, providing 100% rear window visibility.",
    "Cargo Deposit & Lorry Transfer (Key Space): Lowers container onto flatbed trailer (+500 points).",
    "Drivable Cargo Lorry (Key L): Operator jumps into truck cab with Ackerman steering, acceleration, braking, and depot delivery (+1000 pts)."
])
add_viva_note(s10, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, the V key switches dynamically between cabin, 3rd-person tactical orbit, and downward drop cam. After loading the container, pressing L allows the player to drive the transport lorry across the quayside.")
add_image_or_placeholder(s10, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), None, "3rd-Person Tactical Orbit Overview\nShow Crane, Vessel, Hoisted Cargo & Lorry", "Press C -> Press V (Orbit View)")

# ==================== SLIDE 11: BONUS RAY TRACING ====================
s11 = prs.slides.add_slide(blank_layout)
apply_background(s11)
add_header(s11, 11, "Advanced Bonus (+10 Marks)", "Real-Time Whitted Ray Tracing Shader Architecture", "Full-Screen GLSL Ray Tracer: Analytical Spheres, Hard Shadows & Specular Bounces")
add_card(s11, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Ray Tracing Implementation")
add_bullet_points(s11, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Bonus Mode Activation (Key R / F9): Switches pipeline to full-screen Whitted ray tracing quad.",
    "Primary Ray Generation: P(t) = O + t * D generated from normalized screen coordinates and camera orientation.",
    "Analytical Sphere Intersection: Solves quadratic equation a*t^2 + b*t + c = 0 via discriminant check.",
    "Hard Shadow Rays: Casts occlusion ray from hit point toward light source, delivering photorealistic shadows.",
    "Recursive Specular Reflection: R = D - 2*(D · N)*N creates mirror chrome sphere reflections and reflective water floor.",
    "Real-Time Performance: Highly optimized GLSL fragment shader runs at stable 60 FPS."
])
add_viva_note(s11, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, as our advanced bonus feature, pressing R/F9 activates a pure GLSL screen-space Whitted ray tracing shader. It calculates primary rays, analytical sphere intersections, hard shadow rays, and recursive mirror reflection bounces at 60 FPS.")
add_image_or_placeholder(s11, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), "report_assets/raytracing_diagram.png", "Whitted Ray Tracing Diagram")

# ==================== SLIDE 12: CONCLUSION ====================
s12 = prs.slides.add_slide(blank_layout)
apply_background(s12)
add_header(s12, 12, "Summary & Benchmarks", "Performance Benchmarks, Technical Challenges & Conclusion", "Summary of Achievements, Verified Rubric Mapping, and Viva Defense Q/A")
add_card(s12, Inches(0.8), Inches(1.8), Inches(6.0), Inches(3.8), "Performance & Verification Summary")
add_bullet_points(s12, Inches(1.0), Inches(2.3), Inches(5.6), Inches(3.2), [
    "Rock-Solid 60 FPS: Smooth execution under V-Sync on standard GPU hardware with zero frame stuttering.",
    "Zero Polygon Clipping: Dynamic near-plane tuning (0.1m) and repositioned canal spacing prevent clipping.",
    "100% Evaluation Rubric Compliance: All required light types, shading comparisons, quadrics, and hierarchies implemented.",
    "Gamified Logistics Flow: Interactive crane operator cabin, drivable transport truck, automated cycle, and scoring system.",
    "Advanced Extension Delivered: Real-time screen-space Whitted ray tracer earning top-bracket bonus marks."
])
add_viva_note(s12, Inches(0.8), Inches(5.8), Inches(11.733), Inches(1.3),
    "Sir, to conclude, our 3D Sea Port simulation fulfills every teacher grading rubric requirement. We achieved high visual realism, zero clipping, smooth 60 FPS performance, and delivered full real-time ray tracing. Thank you, and we welcome your questions!")
add_image_or_placeholder(s12, Inches(7.1), Inches(1.8), Inches(5.433), Inches(3.8), "report_assets/coord_transforms.png", "Coordinate Transformation Pipeline")

# Save Presentation
out_pptx = "CSE4208_3D_Sea_Port_Simulation_Presentation.pptx"
prs.save(out_pptx)
print(f"SUCCESS: Generated {out_pptx} with 12 widescreen slides!")
