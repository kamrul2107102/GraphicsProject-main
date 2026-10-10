import os
from pptx import Presentation
from pptx.util import Inches, Pt
from pptx.dml.color import RGBColor
from pptx.enum.text import PP_ALIGN
from pptx.enum.shapes import MSO_SHAPE

prs = Presentation()
prs.slide_width = Inches(13.333)
prs.slide_height = Inches(7.5)
blank_layout = prs.slide_layouts[6]

# Exact Sentinel PDF Color Palette
BG_COLOR = RGBColor(13, 23, 33)        # #0D1721 Deep Midnight Slate
CARD_BG = RGBColor(21, 34, 46)         # #15222E Card Surface
BOX_BG = RGBColor(17, 28, 40)          # #111C28 Formula / Highlight Box
BORDER_COL = RGBColor(38, 58, 78)      # #263A4E Subtle Card Border
GOLD = RGBColor(245, 166, 35)          # #F5A623 Sentinel Gold Bar & Accents
TEAL = RGBColor(46, 196, 182)          # #2EC4B6 Sentinel Mint / Teal Green
CYAN = RGBColor(56, 189, 248)          # #38BDF8 Highlight Cyan
WHITE = RGBColor(255, 255, 255)        # Pure White
TEXT_BODY = RGBColor(226, 232, 240)    # #E2E8F0 Soft White
MUTED = RGBColor(148, 163, 184)        # #94A3B8 Muted Steel
GREEN = RGBColor(74, 222, 128)         # #4ADE80 Badge Green

def apply_background(slide):
    bg = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, 0, 0, prs.slide_width, prs.slide_height)
    bg.fill.solid()
    bg.fill.fore_color.rgb = BG_COLOR
    bg.line.fill.background()
    return bg

def add_header(slide, title, slide_num, total_slides=14):
    # Top Gold Vertical Accent Bar
    bar = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.8), Inches(0.42), Inches(0.12), Inches(0.42))
    bar.fill.solid()
    bar.fill.fore_color.rgb = GOLD
    bar.line.fill.background()

    # Section Title
    tb = slide.shapes.add_textbox(Inches(1.05), Inches(0.38), Inches(9.5), Inches(0.5))
    tf = tb.text_frame
    tf.word_wrap = True
    p = tf.paragraphs[0]
    p.text = title
    p.font.size = Pt(22)
    p.font.bold = True
    p.font.color.rgb = WHITE
    p.font.name = "Arial"

    # Top-Right University Tag
    tb_u = slide.shapes.add_textbox(Inches(8.5), Inches(0.44), Inches(4.0), Inches(0.4))
    tf_u = tb_u.text_frame
    p_u = tf_u.paragraphs[0]
    p_u.alignment = PP_ALIGN.RIGHT
    p_u.text = f"SeaPort 3D · CSE 4208 · KUET   {slide_num} / {total_slides}"
    p_u.font.size = Pt(11)
    p_u.font.color.rgb = MUTED
    p_u.font.name = "Arial"

    # Subtle horizontal divider line
    div = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.8), Inches(0.95), Inches(11.733), Inches(0.015))
    div.fill.solid()
    div.fill.fore_color.rgb = BORDER_COL
    div.line.fill.background()

def add_card(slide, left, top, width, height, title=None, title_color=GOLD, border_color=BORDER_COL, bg_color=CARD_BG):
    card = slide.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, left, top, width, height)
    card.fill.solid()
    card.fill.fore_color.rgb = bg_color
    card.line.color.rgb = border_color
    card.line.width = Pt(1.0)

    if title:
        tb = slide.shapes.add_textbox(left + Inches(0.18), top + Inches(0.12), width - Inches(0.36), Inches(0.32))
        tf = tb.text_frame
        p = tf.paragraphs[0]
        p.text = title.upper()
        p.font.size = Pt(11)
        p.font.bold = True
        p.font.color.rgb = title_color
        p.font.name = "Arial"
    return card

def add_bullets(slide, left, top, width, height, bullets, pt_size=11, space_after=6):
    tb = slide.shapes.add_textbox(left, top, width, height)
    tf = tb.text_frame
    tf.word_wrap = True
    for i, b in enumerate(bullets):
        p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
        p.space_after = Pt(space_after)
        
        if ":" in b:
            prefix, rest = b.split(":", 1)
            r1 = p.add_run()
            r1.text = "▪ " + prefix + ":"
            r1.font.bold = True
            r1.font.color.rgb = WHITE
            r1.font.size = Pt(pt_size)
            r1.font.name = "Arial"

            r2 = p.add_run()
            r2.text = rest
            r2.font.bold = False
            r2.font.color.rgb = TEXT_BODY
            r2.font.size = Pt(pt_size)
            r2.font.name = "Arial"
        else:
            r = p.add_run()
            r.text = "▪ " + b
            r.font.color.rgb = TEXT_BODY
            r.font.size = Pt(pt_size)
            r.font.name = "Arial"

def add_formula_card(slide, left, top, width, height, header, formulas, caption=None):
    add_card(slide, left, top, width, height, title=header, title_color=TEAL, border_color=TEAL, bg_color=BOX_BG)
    tb = slide.shapes.add_textbox(left + Inches(0.18), top + Inches(0.42), width - Inches(0.36), height - Inches(0.5))
    tf = tb.text_frame
    tf.word_wrap = True
    
    for i, f in enumerate(formulas):
        p = tf.paragraphs[0] if i == 0 else tf.add_paragraph()
        p.space_after = Pt(3)
        r = p.add_run()
        r.text = f
        r.font.color.rgb = WHITE
        r.font.size = Pt(10.5)
        r.font.name = "Consolas"
    
    if caption:
        p_c = tf.add_paragraph()
        p_c.space_before = Pt(4)
        r_c = p_c.add_run()
        r_c.text = caption
        r_c.font.italic = True
        r_c.font.size = Pt(9.5)
        r_c.font.color.rgb = MUTED
        r_c.font.name = "Arial"

def add_image_panel(slide, left, top, width, height, img_path, caption=None):
    if img_path and os.path.exists(img_path):
        border = slide.shapes.add_shape(MSO_SHAPE.RECTANGLE, left - Inches(0.02), top - Inches(0.02), width + Inches(0.04), height + Inches(0.04))
        border.fill.solid()
        border.fill.fore_color.rgb = BORDER_COL
        border.line.fill.background()
        
        slide.shapes.add_picture(img_path, left, top, width, height)
        
        if caption:
            tb = slide.shapes.add_textbox(left, top + height + Inches(0.04), width, Inches(0.3))
            tf = tb.text_frame
            p = tf.paragraphs[0]
            p.alignment = PP_ALIGN.CENTER
            p.text = caption
            p.font.size = Pt(9.5)
            p.font.italic = True
            p.font.color.rgb = MUTED
            p.font.name = "Arial"

# ==================== SLIDE 1: TITLE SLIDE ====================
s1 = prs.slides.add_slide(blank_layout)
apply_background(s1)
add_image_panel(s1, Inches(0.8), Inches(1.1), Inches(11.733), Inches(4.5), "report_assets/screenshot_overview.png")

tb1 = s1.shapes.add_textbox(Inches(1.2), Inches(1.3), Inches(10.933), Inches(4.1))
tf1 = tb1.text_frame
tf1.word_wrap = True

p = tf1.paragraphs[0]
p.text = "KHULNA UNIVERSITY OF ENGINEERING & TECHNOLOGY  ·  CSE 4208"
p.font.size = Pt(13)
p.font.bold = True
p.font.color.rgb = TEAL

line = s1.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(1.2), Inches(1.8), Inches(1.5), Inches(0.05))
line.fill.solid()
line.fill.fore_color.rgb = GOLD
line.line.fill.background()

p = tf1.add_paragraph()
p.space_before = Pt(20)
p.text = "SeaPort 3D"
p.font.size = Pt(46)
p.font.bold = True
p.font.color.rgb = WHITE

p = tf1.add_paragraph()
p.text = "A Fully Procedural Simulation and Interactive Logistics Engine"
p.font.size = Pt(20)
p.font.color.rgb = TEXT_BODY

p = tf1.add_paragraph()
p.space_before = Pt(14)
p.text = "Nothing is loaded from a 3D model, image or asset file: everything is generated procedurally from C++ code."
p.font.size = Pt(13)
p.font.italic = True
p.font.color.rgb = TEAL
p.font.name = "Georgia"

tb_b1 = s1.shapes.add_textbox(Inches(0.8), Inches(6.0), Inches(5.5), Inches(1.0))
tf_b1 = tb_b1.text_frame
p1 = tf_b1.paragraphs[0]
p1.text = "Md. Kamrul Hasan\nRoll 2107102 · 4th Year 1st Term"
p1.font.size = Pt(12)
p1.font.bold = True
p1.font.color.rgb = WHITE

tb_b2 = s1.shapes.add_textbox(Inches(6.8), Inches(6.0), Inches(5.733), Inches(1.0))
tf_b2 = tb_b2.text_frame
p2 = tf_b2.paragraphs[0]
p2.alignment = PP_ALIGN.RIGHT
p2.text = "Instructors: Computer Graphics & Image Processing Laboratory\nDepartment of Computer Science and Engineering · October 2026"
p2.font.size = Pt(11)
p2.font.color.rgb = MUTED

# ==================== SLIDE 2: OUTLINE ====================
s2 = prs.slides.add_slide(blank_layout)
apply_background(s2)
add_header(s2, "Outline", 2, 14)

outline_items = [
    ("1", "Problem, objectives and architecture"),
    ("2", "Procedural shapes and mathematical quadrics"),
    ("3", "Transformations: creating and combining objects"),
    ("4", "Illumination models and light sources"),
    ("5", "Night cycle & multi-point light attenuation"),
    ("6", "Focused crane spotlight & atmospheric fog"),
    ("7", "Shading comparison: Gouraud vs. Phong fidelity"),
    ("8", "Interactive crane kinematics & 1st-person cockpit"),
    ("9", "3-perspective cameras & intermodal logistics"),
    ("10", "Advanced bonus: real-time screen-space ray tracing"),
    ("11", "Results, verification, bug fixes and performance budget")
]

tb_o = s2.shapes.add_textbox(Inches(0.9), Inches(1.15), Inches(6.8), Inches(5.9))
tf_o = tb_o.text_frame
tf_o.word_wrap = True

for i, (num, item) in enumerate(outline_items):
    p = tf_o.paragraphs[0] if i == 0 else tf_o.add_paragraph()
    p.space_after = Pt(8)
    
    r_num = p.add_run()
    r_num.text = f"{num.rjust(2)}   "
    r_num.font.size = Pt(14)
    r_num.font.bold = True
    r_num.font.color.rgb = GOLD
    r_num.font.name = "Consolas"

    r_txt = p.add_run()
    r_txt.text = item
    r_txt.font.size = Pt(12.5)
    r_txt.font.color.rgb = WHITE

add_image_panel(s2, Inches(7.8), Inches(1.35), Inches(4.7), Inches(2.55), "report_assets/screenshot_quayside_sts.png", "STS Gantry Crane & Dockside Infrastructure")
add_image_panel(s2, Inches(7.8), Inches(4.2), Inches(4.7), Inches(2.55), "report_assets/screenshot_night_mode.png", "Night Illumination, Streetlamps & Crane Spotlight")

# ==================== SLIDE 3: 1. PROBLEM & ARCHITECTURE ====================
s3 = prs.slides.add_slide(blank_layout)
apply_background(s3)
add_header(s3, "1. Problem, objectives and architecture", 3, 14)

tb_q = s3.shapes.add_textbox(Inches(0.8), Inches(1.15), Inches(11.733), Inches(0.55))
tf_q = tb_q.text_frame
p_q = tf_q.paragraphs[0]
p_q.text = "Can a complete, playable 3D sea port engine be made by code alone on a student laptop?"
p_q.font.size = Pt(17)
p_q.font.bold = True
p_q.font.color.rgb = WHITE

add_card(s3, Inches(0.8), Inches(1.85), Inches(5.5), Inches(2.1), "TWO HARD LIMITS", TEAL)
add_bullets(s3, Inches(1.0), Inches(2.3), Inches(5.1), Inches(1.5), [
    "No external 3D model, image texture or precomputed asset file.",
    "Standard OpenGL 3.3 Core Profile: zero game engines, zero compute shaders."
], pt_size=11, space_after=8)

add_card(s3, Inches(0.8), Inches(4.1), Inches(5.5), Inches(2.1), "OBJECTIVES", TEAL)
add_bullets(s3, Inches(1.0), Inches(4.55), Inches(5.1), Inches(1.5), [
    "Layered engine architecture with strict downward dependencies.",
    "100% of visual content generated from code and trigonometric math.",
    "Modern forward shader renderer with full day/night cycle and shadow logic.",
    "Verified by objective benchmarks and a 16.7 ms (60 FPS) frame budget."
], pt_size=10.5, space_after=6)

add_card(s3, Inches(6.6), Inches(1.85), Inches(5.9), Inches(4.35), "LAYERED ENGINE ARCHITECTURE", GOLD)
stack_layers = [
    "Game mission, AI vessels, logistics scoring, sound",
    "Render: forward Phong/Gouraud, multi-lights, sky dome",
    "World collision, camera systems (1st-person & 3rd-person)",
    "Procedural quadrics, parametric buoys, cones, water grid",
    "GPU GLSL shaders, VAO / VBO vertex buffers",
    "Platform GLFW window, input handling, OpenGL context"
]
tb_st = s3.shapes.add_textbox(Inches(6.8), Inches(2.3), Inches(5.5), Inches(3.5))
tf_st = tb_st.text_frame
for i, layer in enumerate(stack_layers):
    p = tf_st.paragraphs[0] if i == 0 else tf_st.add_paragraph()
    p.space_after = Pt(10)
    p.text = f"  [{6-i}]  {layer}"
    p.font.size = Pt(11)
    p.font.bold = True
    p.font.color.rgb = CYAN if i < 3 else TEAL
    p.font.name = "Consolas"

c_bar = s3.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.8), Inches(6.4), Inches(11.733), Inches(0.65))
c_bar.fill.solid()
c_bar.fill.fore_color.rgb = BOX_BG
c_bar.line.color.rgb = GOLD
c_bar.line.width = Pt(1.0)
tb_cb = s3.shapes.add_textbox(Inches(0.9), Inches(6.45), Inches(11.5), Inches(0.55))
tf_cb = tb_cb.text_frame
p_cb = tf_cb.paragraphs[0]
p_cb.text = "The simulation: STS gantry crane, drivable cargo lorry, bascule drawbridge, container ships, lighthouse; 5-objective port management mission; all visual geometry synthesised."
p_cb.font.size = Pt(11)
p_cb.font.bold = True
p_cb.font.color.rgb = WHITE

# ==================== SLIDE 4: 2. PROCEDURAL SHAPES ====================
s4 = prs.slides.add_slide(blank_layout)
apply_background(s4)
add_header(s4, "2. Procedural shapes and mathematical quadrics", 4, 14)

add_formula_card(s4, Inches(0.8), Inches(1.3), Inches(5.8), Inches(2.6),
    "ANALYTICAL CONE QUADRIC (PARAMETRIC RINGS)",
    [
        "x(u, v) = r * (1 - v/h) * cos(u)",
        "y(u, v) = v,   v in [0, h]",
        "z(u, v) = r * (1 - v/h) * sin(u)",
        "n_hat = normalize(cos(u), r/h, sin(u))"
    ],
    "Trigonometric vertex rings generated on CPU with analytical outward normals."
)

add_formula_card(s4, Inches(0.8), Inches(4.15), Inches(5.8), Inches(2.7),
    "SINUSOIDAL WATER WAVE SYNTHESIZER",
    [
        "y(x, z, t) = Sum [ A_i * sin(k_i * (x, z) - w_i * t) ]",
        "n_x = - Sum [ A_i * k_ix * cos(k_i * (x, z) - w_i * t) ]",
        "n_z = - Sum [ A_i * k_iz * cos(k_i * (x, z) - w_i * t) ]",
        "n_hat = normalize(vec3(n_x, 1.0, n_z))"
    ],
    "Dynamic surface displacement and normal recomputation per frame."
)

add_card(s4, Inches(6.9), Inches(1.3), Inches(5.633), Inches(2.6), "GEOMETRIC ASSET SYNTHESIS", TEAL)
add_bullets(s4, Inches(7.1), Inches(1.75), Inches(5.2), Inches(2.0), [
    "7 Procedural Primitives: Cube, Cylinder, Sphere, Quadric Cone, Ring Quadric, Sky Dome, Water Grid.",
    "Maritime Channel Buoys: Starboard Green and Port Red conical floating navigation markers.",
    "Roadway Traffic Cones: High-visibility orange cones with reflective white safety bands.",
    "Zero File I/O: Instantaneous engine boot time with zero runtime disk reads."
], pt_size=11, space_after=6)

add_image_panel(s4, Inches(6.9), Inches(4.15), Inches(5.633), Inches(2.7), "report_assets/screenshot_day_mode.png", "Daytime Port Scene: Water Wave Grid, Navigation Buoys & Quay Structures")

# ==================== SLIDE 5: 3. TRANSFORMATIONS ====================
s5 = prs.slides.add_slide(blank_layout)
apply_background(s5)
add_header(s5, "3. Transformations: creating and combining objects", 5, 14)

pipe_box = s5.shapes.add_shape(MSO_SHAPE.RECTANGLE, Inches(0.8), Inches(1.2), Inches(11.733), Inches(0.55))
pipe_box.fill.solid()
pipe_box.fill.fore_color.rgb = BOX_BG
pipe_box.line.color.rgb = TEAL
tb_p = s5.shapes.add_textbox(Inches(0.9), Inches(1.23), Inches(11.5), Inches(0.5))
tf_p = tb_p.text_frame
p = tf_p.paragraphs[0]
p.text = "Primitive Mesh  -->  Local Transform (M_part)  -->  Hierarchy Tree  -->  M_world  -->  View (V)  -->  Projection (P)  -->  Screen"
p.font.size = Pt(11)
p.font.bold = True
p.font.color.rgb = GOLD
p.font.name = "Consolas"

add_formula_card(s5, Inches(0.8), Inches(1.95), Inches(5.8), Inches(2.35),
    "ONE PART: SCALE, THEN ROTATE, THEN TRANSLATE",
    [
        "M_part = T(t) * R_z(gamma) * R_y(beta) * R_x(alpha) * S(s)",
        "",
        "Example: (1,0,0) -> S(2,1,1) -> R_y(90 deg) -> T(8,0,13.5)"
    ],
    "Applied right-to-left: stretched locally, rotated, then placed in world."
)

add_formula_card(s5, Inches(0.8), Inches(4.5), Inches(5.8), Inches(2.4),
    "NORMALS USE THE INVERSE TRANSPOSE",
    [
        "n_hat' = normalize( (L^-1)^T * n_hat )",
        "where L = upper-left 3x3 of Model Matrix M",
        "",
        "Corrects non-uniform scaling distortion across scaled cylinders/quadrics."
    ],
    "Essential for Blinn-Phong specular glints under asymmetric scaling."
)

add_image_panel(s5, Inches(6.9), Inches(1.95), Inches(5.633), Inches(4.95), "report_assets/crane_hierarchy_tree.png", "STS Crane 6-Level Kinematic Transformation Hierarchy Tree")

# ==================== SLIDE 6: 4. ILLUMINATION MODELS ====================
s6 = prs.slides.add_slide(blank_layout)
apply_background(s6)
add_header(s6, "4. Illumination models and light sources", 6, 14)

add_formula_card(s6, Inches(0.8), Inches(1.2), Inches(11.733), Inches(1.25),
    "DIRECT LIGHT + AMBIENT + EMISSION MASTER EQUATION",
    [
        "L_o = Sum_{j=1..N} [ f(l_j, v) * L_j * att_j * sh_j * max(0, n · l_j) ] + I_amb * k_amb + e",
        "where att_j is distance falloff, sh_j is shadow occlusion factor, and f is the surface BRDF."
    ]
)

add_formula_card(s6, Inches(0.8), Inches(2.6), Inches(5.8), Inches(4.3),
    "SURFACE BRDF FORMULATIONS EVALUATED IN SHADER",
    [
        "[Lambert Diffuse]",
        "  f_lamb = c_d / pi",
        "",
        "[Classic Phong]",
        "  f_phong = c_d/pi + (m+2)/(2*pi) * max(0, r · v)^m",
        "",
        "[Blinn-Phong Optimization]",
        "  h = normalize(l + v)",
        "  f_blinn = c_d/pi + (m+8)/(8*pi) * max(0, n · h)^m",
        "",
        "Replaces reflection vector r with halfway vector h for real-time speed."
    ]
)

add_card(s6, Inches(6.9), Inches(2.6), Inches(5.633), Inches(4.3), "LIGHT SOURCES IN SCENE", GOLD)
add_bullets(s6, Inches(7.1), Inches(3.05), Inches(5.2), Inches(3.6), [
    "Directional Sunlight: Physical 3D Sun at 275m altitude, L_sun = normalize(0.35, 0.88, 0.32).",
    "7 Quayside Point Lights: Sodium street lamps with quadratic attenuation att(d) = 1 / (1 + 0.14d + 0.07d^2).",
    "Crane Trolley Spotlight: Directional cone pointing (0, -1, 0) with smooth cosine cutoff angles (18 deg inner, 25 deg outer).",
    "Lighthouse Rotating Beam: Emissive searchlight sweeping 360 deg across harbor fairway."
], pt_size=11, space_after=10)

# ==================== SLIDE 7: 5. NIGHT CYCLE & ATTENUATION ====================
s7 = prs.slides.add_slide(blank_layout)
apply_background(s7)
add_header(s7, "5. Night cycle & multi-point light attenuation", 7, 14)

add_image_panel(s7, Inches(0.8), Inches(1.3), Inches(6.2), Inches(3.5), "report_assets/screenshot_night_mode.png", "Night Scene (Key N): Sodium Streetlamps, Light Pools & 3D Celestial Moon")

add_formula_card(s7, Inches(7.2), Inches(1.3), Inches(5.333), Inches(3.5),
    "QUADRATIC DISTANCE ATTENUATION FORMULA",
    [
        "att(d) = 1.0 / (k_c + k_l * d + k_q * d^2)",
        "",
        "Parameters:",
        "  k_c = 1.0   (Constant base factor)",
        "  k_l = 0.14  (Linear distance factor)",
        "  k_q = 0.07  (Quadratic distance factor)",
        "",
        "Result: Produces natural localized light pools",
        "on the dock road with zero distant light bleed."
    ]
)

add_image_panel(s7, Inches(0.8), Inches(5.0), Inches(5.8), Inches(2.0), "report_assets/light_attenuation.png", "Quadratic Attenuation Curve (1 / (1 + 0.14d + 0.07d^2))")

add_card(s7, Inches(6.8), Inches(5.0), Inches(5.733), Inches(2.0), "NOCTURNAL SYSTEM SPECIFICATIONS", TEAL)
add_bullets(s7, Inches(7.0), Inches(5.35), Inches(5.3), Inches(1.5), [
    "Dynamic Day/Night Cycle (Key N): Toggles daytime sun to midnight sky dome with procedural stars.",
    "Luminous 3D Moon: Emissive celestial body acting as secondary night directional light source.",
    "7 Quayside Street Lamps: Multi-source light accumulation evaluated concurrently in phong.frag."
], pt_size=10.5, space_after=4)

# ==================== SLIDE 8: 6. SPOTLIGHT & FOG ====================
s8 = prs.slides.add_slide(blank_layout)
apply_background(s8)
add_header(s8, "6. Focused crane spotlight & atmospheric fog", 8, 14)

add_formula_card(s8, Inches(0.8), Inches(1.3), Inches(5.8), Inches(2.7),
    "CRANE TROLLEY SPOTLIGHT (COSINE CUTOFF)",
    [
        "I_spot = clamp( (cos_theta - cos_outer) /",
        "                (cos_inner - cos_outer), 0.0, 1.0 )",
        "",
        "Cutoff Angles:",
        "  theta_inner = 18.0 deg  -->  cos_inner = 0.951",
        "  theta_outer = 25.0 deg  -->  cos_outer = 0.906",
        "Direction: Fixed unit vector (0.0, -1.0, 0.0)"
    ],
    "Mounted directly under trolley; tracks downward onto cargo containers."
)

add_formula_card(s8, Inches(0.8), Inches(4.2), Inches(5.8), Inches(2.7),
    "ATMOSPHERIC DISTANCE FOG (EXPONENTIAL)",
    [
        "C_final = T * C_surface + (1.0 - T) * C_fog",
        "Transmission: T = exp( - k_fog * distance )",
        "",
        "k_fog = 0.0035 m^-1",
        "Distance = length(uViewPos - FragPos)",
        "",
        "Blends distant water horizon smoothly into sky dome."
    ],
    "Prevents sharp polygon cutoff at the 320m far clipping boundary."
)

add_image_panel(s8, Inches(6.9), Inches(1.3), Inches(5.633), Inches(5.6), "report_assets/screenshot_quayside_sts.png", "Quayside STS Crane: Downward Illumination, Fairway Fog & Canal Depth")

# ==================== SLIDE 9: 7. SHADING COMPARISON ====================
s9 = prs.slides.add_slide(blank_layout)
apply_background(s9)
add_header(s9, "7. Shading comparison: Gouraud vs. Phong fidelity", 9, 14)

add_image_panel(s9, Inches(0.8), Inches(1.3), Inches(5.8), Inches(5.6), "report_assets/gouraud_vs_phong.png", "Gouraud vs Phong Shading: Vertex vs Fragment Pipeline Comparison")

add_card(s9, Inches(6.9), Inches(1.3), Inches(5.633), Inches(2.7), "PHONG SHADING (KEY F2) -- PER-FRAGMENT", TEAL)
add_bullets(s9, Inches(7.1), Inches(1.75), Inches(5.2), Inches(2.1), [
    "Evaluated per-fragment inside phong.frag.",
    "Surface normal vector n_hat is interpolated across rasterized pixels.",
    "Halfway vector h calculated per-pixel for Blinn-Phong specular glints.",
    "Pinpoint specular reflections on metallic containers and water ripples."
], pt_size=10.5, space_after=5)

add_card(s9, Inches(6.9), Inches(4.2), Inches(5.633), Inches(2.7), "GOURAUD SHADING (KEY F1) -- PER-VERTEX", GOLD)
add_bullets(s9, Inches(7.1), Inches(4.65), Inches(5.2), Inches(2.1), [
    "Evaluated per-vertex inside gouraud.vert.",
    "Lighting equations evaluated strictly at triangle vertex points.",
    "Resulting vertex color is linearly interpolated across triangles.",
    "Demonstrates classic Mach banding artifacts across cylindrical masts."
], pt_size=10.5, space_after=5)

# ==================== SLIDE 10: 8. CRANE KINEMATICS & 1ST-PERSON ====================
s10 = prs.slides.add_slide(blank_layout)
apply_background(s10)
add_header(s10, "8. Interactive crane kinematics & 1st-person cockpit", 10, 14)

add_image_panel(s10, Inches(0.8), Inches(1.3), Inches(6.0), Inches(3.4), "report_assets/screenshot_crane_loading_cab.png", "1st-Person Cabin POV (Key C): Targeting & Locking Cargo from Vessel")

add_card(s10, Inches(7.1), Inches(1.3), Inches(5.433), Inches(3.4), "COCKPIT OPTIMIZATION & ZOOM", TEAL)
add_bullets(s10, Inches(7.3), Inches(1.75), Inches(5.0), Inches(2.8), [
    "Repositioned Fairway (Z=0.5m): Moving container ship 5m out into canal provided an unobstructed 60° downward sightline.",
    "Dynamic Zoom (Scroll / Z, X): FOV zooms smoothly between 32° (close-up twistlocks) and 85° (wide harbor panorama).",
    "Active Target Beacon: Pulsing gold-to-green vertical laser beacon guides operator to the designated cargo box.",
    "HUD Telemetry: Real-time digital readouts for trolley position (m), hook height (m), and payload attachment."
], pt_size=10.5, space_after=5)

add_card(s10, Inches(0.8), Inches(4.9), Inches(11.733), Inches(2.0), "6-LEVEL PARENT-CHILD TRANSFORMATION HIERARCHY", GOLD)
add_bullets(s10, Inches(1.0), Inches(5.3), Inches(11.3), Inches(1.4), [
    "Gantry Portal Base --> Slewing Turret (R_y) --> Boom Arm (22m) --> Trolley (Z=-1.5m to 15.5m) --> Hoist Cables (scaled Y) --> Spreader Hook --> Secured Cargo.",
    "Matrix Stacks: When the operator slews the cab or moves the trolley, all descendant child nodes transform synchronously in relative coordinate space."
], pt_size=10.5, space_after=4)

# ==================== SLIDE 11: 9. 3-PERSPECTIVE & LORRY ====================
s11 = prs.slides.add_slide(blank_layout)
apply_background(s11)
add_header(s11, "9. 3-perspective cameras & intermodal logistics", 11, 14)

add_image_panel(s11, Inches(0.8), Inches(1.3), Inches(5.7), Inches(3.2), "report_assets/screenshot_crane_tactical.png", "3rd-Person Tactical Orbit (Key V): Mouse Orbit & Hoisted Container on Boom")
add_image_panel(s11, Inches(6.8), Inches(1.3), Inches(5.7), Inches(3.2), "report_assets/screenshot_lorry_bay.png", "Dock Unloading Bay (Z=13.5m) & Cargo Lorry on Roadway")

add_card(s11, Inches(0.8), Inches(4.8), Inches(11.733), Inches(2.1), "3-CAMERA PERSPECTIVE SYSTEM & QUAYSIDE TRANSFER", TEAL)
add_bullets(s11, Inches(1.0), Inches(5.2), Inches(11.3), Inches(1.5), [
    "Camera Cycling (Key V): 1st-Person Cabin (Zoomable FOV 32°-85°) <--> 3rd-Person Tactical Orbit (Mouse Look, Distance 10m-65m) <--> Precision Spreader Drop Cam.",
    "Repositioned Unload Bay (Z=13.5m): Moved clear of gantry legs onto dock road, delivering 100% rear window visibility.",
    "Cargo Deposit & Transfer (Key Space): Lowers container onto flatbed trailer (+500 points).",
    "Drivable Cargo Lorry (Key L): Operator jumps into truck cab with Ackerman steering and delivers to Freight Depot (+1000 pts)."
], pt_size=11, space_after=4)

# ==================== SLIDE 12: 10. ADVANCED BONUS RAY TRACING ====================
s12 = prs.slides.add_slide(blank_layout)
apply_background(s12)
add_header(s12, "10. Advanced bonus: real-time screen-space ray tracing", 12, 14)

add_formula_card(s12, Inches(0.8), Inches(1.3), Inches(5.8), Inches(2.7),
    "RAY EQUATION & ANALYTICAL QUADRIC INTERSECTION",
    [
        "P(t) = O + t * D,   t > 0",
        "Sphere: ||P - C||^2 = R^2",
        "Quadratic: a*t^2 + b*t + c = 0",
        "  Delta = b^2 - 4ac",
        "  t_hit = (-b - sqrt(Delta)) / (2a)"
    ],
    "Exact analytical surface intersection evaluated in GLSL fragment shader."
)

add_formula_card(s12, Inches(0.8), Inches(4.2), Inches(5.8), Inches(2.7),
    "HARD SHADOWS & RECURSIVE SPECULAR BOUNCES",
    [
        "Shadow Ray: S(s) = P_hit + s * L_dir",
        "  If blocked by scene object --> zero direct light.",
        "",
        "Reflection: R_refl = D - 2 * (D · N) * N",
        "  Traces secondary rays for chrome sphere & ocean."
    ],
    "Real-time Whitted ray tracer running at stable 60 FPS in screen-space quad."
)

add_image_panel(s12, Inches(6.9), Inches(1.3), Inches(5.633), Inches(5.6), "report_assets/raytracing_diagram.png", "Whitted Ray Tracing Architecture: Primary, Shadow & Reflected Bounces")

# ==================== SLIDE 13: 11. RESULTS & VERIFICATION ====================
s13 = prs.slides.add_slide(blank_layout)
apply_background(s13)
add_header(s13, "11. Results, verification, bug fixes and performance budget", 13, 14)

add_card(s13, Inches(0.8), Inches(1.3), Inches(5.8), Inches(3.3), "GPU TIME PER PASS (MS)  --  11.2 MS OF 16.7 MS BUDGET (60 FPS)", TEAL)
passes = [
    ("Terrain & Quay Road", "3.80 ms", 0.76),
    ("STS Crane & Vessels", "4.20 ms", 0.84),
    ("Water Wave & Specular", "1.90 ms", 0.38),
    ("Post, Text & HUD", "1.25 ms", 0.25),
    ("Sky Dome & Atmosphere", "0.05 ms", 0.02)
]
tb_gpu = s13.shapes.add_textbox(Inches(1.0), Inches(1.8), Inches(5.4), Inches(2.6))
tf_gpu = tb_gpu.text_frame
for i, (name, val, frac) in enumerate(passes):
    p = tf_gpu.paragraphs[0] if i == 0 else tf_gpu.add_paragraph()
    p.space_after = Pt(6)
    p.text = f"{name.ljust(22)} {val.rjust(8)}  {'█' * int(frac * 18)}"
    p.font.size = Pt(10.5)
    p.font.color.rgb = CYAN if i < 2 else TEAL
    p.font.name = "Consolas"

add_card(s13, Inches(0.8), Inches(4.75), Inches(5.8), Inches(2.15), "VERIFICATION METRICS", GOLD)
add_bullets(s13, Inches(1.0), Inches(5.15), Inches(5.4), Inches(1.6), [
    "Procedural VAO/VBO Primitives Verified: 7 geometries.",
    "Matrix Transforms per Frame: 320+ nodes evaluated.",
    "Fragment Lighting Evaluations: 48,000+ per frame.",
    "Logistics Playthroughs: 5/5 objectives verified."
], pt_size=10.5, space_after=4)

add_card(s13, Inches(6.9), Inches(1.3), Inches(5.633), Inches(3.3), "BUG FIXED: CABIN LINE-OF-SIGHT & FRUSTUM CLIPPING", GOLD)
add_bullets(s13, Inches(7.1), Inches(1.75), Inches(5.2), Inches(2.6), [
    "Problem: Vessel at Z=2.4m filled windshield; bay at Z=9.8m was hidden under gantry tracks.",
    "Root Cause: Insufficient horizontal sightline distance from elevated cab (Y=12m).",
    "Solution: Repositioned vessel to canal center (Z=0.5m) and unload bay to open road (Z=13.5m).",
    "Result: Unobstructed 60° forward and rear sightlines with dynamic zoom."
], pt_size=10.5, space_after=6)

add_card(s13, Inches(6.9), Inches(4.75), Inches(5.633), Inches(2.15), "LIMITS AND FUTURE WORK", TEAL)
add_bullets(s13, Inches(7.1), Inches(5.15), Inches(5.2), Inches(1.6), [
    "Cascaded point-light shadow maps for nocturnal quayside.",
    "Dynamic volumetric water foam and vessel wake simulation.",
    "Takeaway: Most computer graphics art is a few simple mathematical ideas, applied consistently and rigorously tested."
], pt_size=10, space_after=4)

# ==================== SLIDE 14: 12. REFERENCES & CONCLUSION ====================
s14 = prs.slides.add_slide(blank_layout)
apply_background(s14)
add_header(s14, "12. References & Conclusion", 14, 14)

add_card(s14, Inches(0.8), Inches(1.3), Inches(11.733), Inches(3.5), "ACADEMIC & INDUSTRY REFERENCES", TEAL)
refs = [
    "[1] J. F. Blinn, 'Models of light reflected surfaces for computer synthesized pictures,' ACM SIGGRAPH, 1977.",
    "[2] H. Gouraud, 'Continuous shading of curved surfaces,' IEEE Transactions on Computers, 1971.",
    "[3] B. T. Phong, 'Illumination for computer generated pictures,' Communications of the ACM, 1975.",
    "[4] T. Whitted, 'An improved illumination model for shaded display,' Communications of the ACM, 1980.",
    "[5] T. Akenine-Möller, E. Haines, and N. Hoffman, Real-Time Rendering, 4th ed., CRC Press, 2018.",
    "[6] Khronos Group, 'The OpenGL Graphics System: A Specification (Version 3.3 Core Profile),' 2010.",
    "[7] B. Karis, 'Real Shading in Unreal Engine 4,' Epic Games, SIGGRAPH Course Notes, 2013.",
    "[8] I. Quilez, 'Raymarching and analytical primitives,' iquilezles.org, 2020."
]
tb_ref = s14.shapes.add_textbox(Inches(1.0), Inches(1.75), Inches(11.3), Inches(2.9))
tf_ref = tb_ref.text_frame
for i, r in enumerate(refs):
    p = tf_ref.paragraphs[0] if i == 0 else tf_ref.add_paragraph()
    p.space_after = Pt(4)
    p.text = r
    p.font.size = Pt(10)
    p.font.color.rgb = TEXT_BODY

t_bar = s14.shapes.add_shape(MSO_SHAPE.ROUNDED_RECTANGLE, Inches(0.8), Inches(5.1), Inches(11.733), Inches(1.8))
t_bar.fill.solid()
t_bar.fill.fore_color.rgb = CARD_BG
t_bar.line.color.rgb = GOLD
t_bar.line.width = Pt(1.5)

tb_ty = s14.shapes.add_textbox(Inches(1.2), Inches(5.25), Inches(10.933), Inches(1.5))
tf_ty = tb_ty.text_frame
p_ty1 = tf_ty.paragraphs[0]
p_ty1.text = "Thank You"
p_ty1.font.size = Pt(36)
p_ty1.font.bold = True
p_ty1.font.color.rgb = WHITE

p_ty2 = tf_ty.add_paragraph()
p_ty2.text = "Questions & Technical Discussion are Welcome  ·  Md. Kamrul Hasan (Roll 2107102)"
p_ty2.font.size = Pt(14)
p_ty2.font.color.rgb = TEAL

out_file = "CSE4208_3D_Sea_Port_Simulation_Sentinel_Presentation.pptx"
prs.save(out_file)
print(f"SUCCESS: Generated {out_file} with 14 widescreen slides in exact Sentinel PDF style!")
