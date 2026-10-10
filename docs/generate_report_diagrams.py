import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches
from PIL import Image, ImageDraw, ImageFont
import numpy as np

os.makedirs('report_assets', exist_ok=True)

# -------------------------------------------------------------------------
# Figure 3.1: The six layers (dependencies point downward) and the main loop
# (Matching user's phone screenshot 1 but 100% customized for Sea Port 3D)
# -------------------------------------------------------------------------
fig, ax = plt.subplots(figsize=(10, 3.2), dpi=300)
ax.set_xlim(0, 10)
ax.set_ylim(0, 3.2)
ax.axis('off')

# Layer boxes (stacked on left)
layers = [
    ("Port: mission, STS crane mechanics, lorry transport, AI vessels, HUD, sound", "#d4ddf7"),
    ("Engine/Render: forward Blinn-Phong & Gouraud, multi-lights, sky dome", "#dce5f9"),
    ("Engine/World: quay collision, 1st-person FPS, crane cabin, 3rd-person orbit", "#e4ecfb"),
    ("Engine/Procedural: 7 analytical quadrics, buoys, traffic cones, water wave grid", "#ebf2fd"),
    ("Engine/GPU: GLSL 330 shaders, VAO/VBO/EBO buffers, in-memory font atlas", "#f2f7fe"),
    ("Engine/Platform: GLFW window, OpenGL 3.3 Core, input, high-res clock", "#f7faff")
]

box_x = 0.5
box_w = 5.6
box_h = 0.42
start_y = 2.65

for i, (text, col) in enumerate(layers):
    y = start_y - i * box_h
    rect = patches.Rectangle((box_x, y), box_w, box_h, linewidth=0.8, edgecolor='#333333', facecolor=col)
    ax.add_patch(rect)
    ax.text(box_x + box_w/2, y + box_h/2, text, ha='center', va='center', fontsize=7.2, color='#111111', weight='medium')

# Arrow pointing downward on the right of the stack
arr_x = box_x + box_w + 0.15
ax.annotate('', xy=(arr_x, start_y - 5.5 * box_h), xytext=(arr_x, start_y + box_h),
            arrowprops=dict(arrowstyle="->", lw=1.2, color='#222222'))
ax.text(arr_x + 0.1, start_y - 2.5 * box_h, "depends\non", ha='left', va='center', fontsize=7.5, color='#222222')

# Main loop on the right
loop_steps = ["init", "input", "update", "render", "end"]
step_x = 7.1
step_y = 1.6
step_w = 0.52
step_h = 0.32

for j, step in enumerate(loop_steps):
    x = step_x + j * (step_w + 0.12)
    # rounded box
    rect = patches.FancyBboxPatch((x, step_y - step_h/2), step_w, step_h, boxstyle="round,pad=0.04,rounding_size=0.08",
                                  linewidth=0.8, edgecolor='#333333', facecolor='#ffffff')
    ax.add_patch(rect)
    ax.text(x + step_w/2, step_y, step, ha='center', va='center', fontsize=7.5, color='#111111')
    
    if j < len(loop_steps) - 1:
        ax.annotate('', xy=(x + step_w + 0.12, step_y), xytext=(x + step_w, step_y),
                    arrowprops=dict(arrowstyle="->", lw=1.0, color='#333333'))

plt.tight_layout()
plt.savefig('report_assets/figure_six_layers_mainloop.png', bbox_inches='tight', dpi=300)
plt.close()
print("Generated figure_six_layers_mainloop.png")

# -------------------------------------------------------------------------
# Figure 3.6: The geometry of lighting: vectors and the four light types
# (Matching user's phone screenshot 2)
# -------------------------------------------------------------------------
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(9.5, 3.2), dpi=300, gridspec_kw={'width_ratios': [1, 1.3]})

# (a) Vectors of a model
ax1.set_xlim(-1.8, 1.8)
ax1.set_ylim(-0.4, 2.2)
ax1.axis('off')

# Surface
ax1.plot([-1.5, 1.5], [0, 0], color='#222222', lw=2)

# Normal n
ax1.annotate('', xy=(0, 1.8), xytext=(0, 0), arrowprops=dict(arrowstyle="->", lw=2, color='#111111'))
ax1.text(0, 1.95, "n", ha='center', va='bottom', fontsize=10, weight='bold')

# Light l
ax1.annotate('', xy=(-1.2, 1.2), xytext=(0, 0), arrowprops=dict(arrowstyle="->", lw=1.8, color='#b45309'))
ax1.text(-1.35, 1.3, "l", ha='center', va='bottom', fontsize=10, color='#b45309', weight='bold')

# View v
ax1.annotate('', xy=(1.3, 1.1), xytext=(0, 0), arrowprops=dict(arrowstyle="->", lw=1.8, color='#1e40af'))
ax1.text(1.42, 1.15, "v", ha='center', va='bottom', fontsize=10, color='#1e40af', weight='bold')

# Half vector h
H_dir = np.array([-1.2, 1.2]) / np.linalg.norm([-1.2, 1.2]) + np.array([1.3, 1.1]) / np.linalg.norm([1.3, 1.1])
H = 1.65 * (H_dir / np.linalg.norm(H_dir))
ax1.annotate('', xy=(H[0], H[1]), xytext=(0, 0), arrowprops=dict(arrowstyle="->", lw=1.8, color='#047857'))
ax1.text(H[0]+0.15, H[1]+0.05, "h", ha='center', va='bottom', fontsize=10, color='#047857', weight='bold')

# Arc for theta_i
arc = patches.Arc((0, 0), 1.0, 1.0, angle=0, theta1=90, theta2=135, color='#b45309', lw=1.2)
ax1.add_patch(arc)
ax1.text(-0.25, 0.65, r"$\theta_i$", fontsize=8.5, color='#b45309')

ax1.text(0, -0.3, "(a) Vectors of a model", ha='center', fontsize=9.5, weight='bold')

# (b) The four light types
ax2.set_xlim(0, 8.5)
ax2.set_ylim(-0.4, 2.2)
ax2.axis('off')

# 1. Directional
for dy in [0.7, 1.2, 1.7]:
    ax2.annotate('', xy=(1.4, dy - 0.45), xytext=(0.4, dy),
                 arrowprops=dict(arrowstyle="->", lw=1.5, color='#b45309'))
ax2.text(0.9, -0.1, "Directional\n(Sun)", ha='center', fontsize=8)

# 2. Point (Sodium lamp)
px, py = 3.1, 1.2
for ang in np.linspace(0, 2*np.pi, 8, endpoint=False):
    dx, dy = 0.5 * np.cos(ang), 0.5 * np.sin(ang)
    ax2.annotate('', xy=(px + dx, py + dy), xytext=(px, py),
                 arrowprops=dict(arrowstyle="->", lw=1.3, color='#b45309'))
ax2.scatter([px], [py], color='#b45309', s=40, zorder=5)
ax2.text(px, -0.1, "Point\n(Lamp)", ha='center', fontsize=8)

# 3. Spot (Crane worklight)
sx, sy = 5.4, 1.7
ax2.plot([sx - 0.55, sx, sx + 0.55, sx - 0.55], [0.55, sy, 0.55, 0.55], color='#b45309', lw=1.5)
ax2.annotate('', xy=(sx, 0.6), xytext=(sx, sy), arrowprops=dict(arrowstyle="->", lw=1.5, color='#b45309'))
ax2.text(sx, -0.1, "Spot\n(Worklight)", ha='center', fontsize=8)

# 4. Sweeping Lighthouse Beam
bx, by = 7.5, 1.7
for offset in [-0.3, -0.1, 0.1, 0.3]:
    ax2.annotate('', xy=(bx + offset, 0.5), xytext=(bx + offset, by),
                 arrowprops=dict(arrowstyle="->", lw=1.3, color='#b45309'))
ax2.plot([bx - 0.35, bx + 0.35], [by, by], color='#b45309', lw=2)
ax2.text(bx, -0.1, "Beam\n(Lighthouse)", ha='center', fontsize=8)

ax2.text(4.25, -0.3, "(b) The four simulation light types", ha='center', fontsize=9.5, weight='bold')

plt.tight_layout()
plt.savefig('report_assets/figure_lighting_geometry_four_types.png', bbox_inches='tight', dpi=300)
plt.close()
print("Generated figure_lighting_geometry_four_types.png")

# -------------------------------------------------------------------------
# Figure 4.2: 9-Panel Engine and Simulation in Action Collage
# (Matching user's phone screenshot 3 with REAL in-game screenshots)
# -------------------------------------------------------------------------
grid_items = [
    ("report_assets/screenshot_overview.png", "(a) Harbor Overview"),
    ("report_assets/screenshot_quayside_sts.png", "(b) Quayside STS Crane"),
    ("report_assets/screenshot_crane_loading_cab.png", "(c) 1st-Person Cabin Loading"),
    ("report_assets/screenshot_crane_tactical.png", "(d) 3rd-Person Tactical Orbit"),
    ("report_assets/screenshot_lorry_bay.png", "(e) Unload Bay & Lorry"),
    ("report_assets/screenshot_day_mode.png", "(f) Daytime Illumination"),
    ("report_assets/screenshot_night_mode.png", "(g) Nocturnal Light Pools"),
    ("C:/Users/CZ/.gemini/antigravity-ide/brain/27b72aa2-8c74-4ab8-aeeb-733b9d5643de/.user_uploaded/media_1791613879278.png", "(h) Sightline Bug (Before)"),
    ("report_assets/screenshot_crane_tactical.png", "(i) Sightline Fixed (After)")
]

fig, axes = plt.subplots(3, 3, figsize=(10, 7.5), dpi=300)
axes = axes.flatten()

for idx, (path, label) in enumerate(grid_items):
    ax = axes[idx]
    if os.path.exists(path):
        img = Image.open(path)
        ax.imshow(img)
    else:
        ax.fill([0, 1, 1, 0], [0, 0, 1, 1], '#e2e8f0')
        ax.text(0.5, 0.5, label, ha='center', va='center', fontsize=8)
    ax.axis('off')
    ax.set_title(label, fontsize=8.5, weight='bold', pad=4, color='#1e293b')

plt.tight_layout()
plt.savefig('report_assets/figure_simulation_action_grid.png', bbox_inches='tight', dpi=300)
plt.close()
print("Generated figure_simulation_action_grid.png")

# -------------------------------------------------------------------------
# Figure 3.7: Painted Spheres & Ray Tracing Reflection Showcase
# (Matching user's phone screenshot 4)
# -------------------------------------------------------------------------
fig, ax = plt.subplots(figsize=(9, 4), dpi=300)
ax.axis('off')
rt_img_path = 'report_assets/screenshot_raytracing_bonus.png'
if os.path.exists(rt_img_path):
    rt_img = Image.open(rt_img_path)
    ax.imshow(rt_img)
    ax.set_title("Figure 3.7: Real-Time Screen-Space Whitted Ray Tracing Simulation Showcase\n(Red Specular, Center Chrome Mirror Reflection, Green Glossy, and Secondary Reflected Floor Bounces)",
                 fontsize=9.5, weight='bold', pad=8, color='#0f172a')
plt.tight_layout()
plt.savefig('report_assets/figure_raytracing_spheres_showcase.png', bbox_inches='tight', dpi=300)
plt.close()
print("Generated figure_raytracing_spheres_showcase.png")
