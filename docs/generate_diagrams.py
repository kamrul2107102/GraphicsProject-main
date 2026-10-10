import os
import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import matplotlib.patches as patches
import numpy as np

os.makedirs('report_assets', exist_ok=True)

# Set high DPI and professional aesthetic
plt.rcParams['font.sans-serif'] = 'DejaVu Sans'
plt.rcParams['axes.edgecolor'] = '#2c3e50'
plt.rcParams['axes.linewidth'] = 1.2

# -------------------------------------------------------------
# 1. System Architecture & Graphics Pipeline Flowchart
# -------------------------------------------------------------
fig, ax = plt.subplots(figsize=(10, 4.5), dpi=300)
ax.set_xlim(0, 10)
ax.set_ylim(0, 5)
ax.axis('off')

boxes = [
    ("Vertex Data\n(Cube, Cyl, Sph, Cone)", 0.6, 2.5, "#34495e"),
    ("Vertex Array Object (VAO)\n& Buffers (VBO/EBO)", 2.4, 2.5, "#2980b9"),
    ("Vertex Shader\n(Transforms & Wave)", 4.2, 2.5, "#16a085"),
    ("Primitive Assembly\n& Rasterization", 6.0, 2.5, "#d35400"),
    ("Fragment Shader\n(Phong/Gouraud/Sky)", 7.8, 2.5, "#8e44ad"),
    ("Framebuffer\n(Color/Depth)", 9.4, 2.5, "#27ae60")
]

for label, cx, cy, col in boxes:
    rect = patches.FancyBboxPatch((cx-0.75, cy-0.7), 1.5, 1.4, boxstyle="round,pad=0.1,rounding_size=0.15",
                                  facecolor=col, edgecolor='#1a252f', linewidth=1.5, alpha=0.9)
    ax.add_patch(rect)
    ax.text(cx, cy, label, ha='center', va='center', color='white', fontsize=8.5, weight='bold')

# Draw arrows
for i in range(len(boxes)-1):
    x1 = boxes[i][1] + 0.75
    x2 = boxes[i+1][1] - 0.75
    ax.annotate('', xy=(x2, 2.5), xytext=(x1, 2.5),
                arrowprops=dict(arrowstyle="->", lw=2.5, color='#2c3e50'))

# Top Uniform / State Block
rect_uni = patches.FancyBboxPatch((3.0, 4.1), 4.0, 0.7, boxstyle="round,pad=0.08,rounding_size=0.1",
                                  facecolor='#f39c12', edgecolor='#b9770e', linewidth=1.2)
ax.add_patch(rect_uni)
ax.text(5.0, 4.45, "Uniforms: Model, View, Projection, Light Specs, Time", ha='center', va='center', color='white', fontsize=9, weight='bold')
ax.annotate('', xy=(4.2, 3.2), xytext=(4.2, 4.1), arrowprops=dict(arrowstyle="->", lw=1.8, color='#d35400', ls='--'))
ax.annotate('', xy=(7.8, 3.2), xytext=(6.5, 4.1), arrowprops=dict(arrowstyle="->", lw=1.8, color='#d35400', ls='--'))

plt.title("Figure 1.1: OpenGL 3.3 Core Profile Graphics Pipeline & Data Flow in 3D Sea Port Simulation",
          fontsize=10.5, weight='bold', pad=15, color='#1a252f')
plt.tight_layout()
plt.savefig('report_assets/pipeline_flow.png', bbox_inches='tight')
plt.close()

# -------------------------------------------------------------
# 2. Coordinate Transformation Chain
# -------------------------------------------------------------
fig, ax = plt.subplots(figsize=(10, 3.6), dpi=300)
ax.set_xlim(0, 10)
ax.set_ylim(0, 3.5)
ax.axis('off')

stages = [
    ("Local / Object\nSpace ($v_{obj}$)", 0.8, 1.6, "#2c3e50"),
    ("World Space\n($v_{world}$)", 2.8, 1.6, "#2980b9"),
    ("View / Eye\nSpace ($v_{view}$)", 4.8, 1.6, "#16a085"),
    ("Clip Space\n($v_{clip}$)", 6.8, 1.6, "#e67e22"),
    ("Normalized Device\nCoord (NDC)", 8.8, 1.6, "#8e44ad")
]

for label, cx, cy, col in stages:
    rect = patches.FancyBboxPatch((cx-0.75, cy-0.6), 1.5, 1.2, boxstyle="round,pad=0.1,rounding_size=0.12",
                                  facecolor=col, edgecolor='#1a252f', linewidth=1.5)
    ax.add_patch(rect)
    ax.text(cx, cy, label, ha='center', va='center', color='white', fontsize=8.5, weight='bold')

matrices = [
    ("Model Matrix (M)\nT * R * S", 1.8),
    ("View Matrix (V)\nglm::lookAt()", 3.8),
    ("Projection (P)\nglm::perspective()", 5.8),
    ("Perspective\nDivision (/w)", 7.8)
]

for mat_text, mx in matrices:
    ax.annotate('', xy=(mx+0.25, 1.6), xytext=(mx-0.25, 1.6),
                arrowprops=dict(arrowstyle="->", lw=2.2, color='#c0392b'))
    ax.text(mx, 2.5, mat_text, ha='center', va='center', color='#c0392b', fontsize=8, weight='bold')

plt.title("Figure 1.2: Complete Coordinate Transformation Pipeline from Local Coordinates to Screen NDC",
          fontsize=10.5, weight='bold', pad=12, color='#1a252f')
plt.tight_layout()
plt.savefig('report_assets/coord_transforms.png', bbox_inches='tight')
plt.close()

# -------------------------------------------------------------
# 3. Blinn-Phong Lighting Vectors Diagram
# -------------------------------------------------------------
fig, ax = plt.subplots(figsize=(6.5, 4.5), dpi=300)
ax.set_xlim(-2.5, 2.5)
ax.set_ylim(-0.8, 3.2)
ax.axis('off')

# Surface line
ax.plot([-2.2, 2.2], [0, 0], color='#34495e', lw=3.5)
ax.fill_between([-2.2, 2.2], [-0.5, -0.5], [0, 0], color='#bdc3c7', alpha=0.5)
ax.text(0, -0.25, "Surface Plane ($P_{frag}$)", ha='center', va='center', fontsize=9, weight='bold', color='#2c3e50')

# Point of intersection
p = np.array([0, 0])

# Normal N
N = np.array([0, 2.2])
ax.annotate('', xy=(N[0], N[1]), xytext=(0, 0), arrowprops=dict(arrowstyle="->", lw=2.5, color='#27ae60'))
ax.text(N[0]+0.12, N[1]-0.1, r"$\vec{N}$ (Normal)", color='#27ae60', fontsize=10, weight='bold')

# Light Vector L
L = np.array([-1.5, 2.0])
ax.annotate('', xy=(L[0], L[1]), xytext=(0, 0), arrowprops=dict(arrowstyle="->", lw=2.5, color='#e67e22'))
ax.text(L[0]-0.2, L[1]+0.1, r"$\vec{L}$ (To Light)", color='#e67e22', fontsize=10, weight='bold')

# View Vector V
V = np.array([1.6, 1.8])
ax.annotate('', xy=(V[0], V[1]), xytext=(0, 0), arrowprops=dict(arrowstyle="->", lw=2.5, color='#2980b9'))
ax.text(V[0]+0.1, V[1]+0.05, r"$\vec{V}$ (To Viewer)", color='#2980b9', fontsize=10, weight='bold')

# Halfway Vector H
H_dir = (L/np.linalg.norm(L) + V/np.linalg.norm(V))
H = 2.1 * (H_dir / np.linalg.norm(H_dir))
ax.annotate('', xy=(H[0], H[1]), xytext=(0, 0), arrowprops=dict(arrowstyle="->", lw=2.5, color='#8e44ad', ls='--'))
ax.text(H[0]+0.12, H[1]+0.05, r"$\vec{H} = \frac{\vec{L}+\vec{V}}{||\vec{L}+\vec{V}||}$", color='#8e44ad', fontsize=10, weight='bold')

# Angles
arc_theta = patches.Arc((0, 0), 1.2, 1.2, angle=0, theta1=90, theta2=127, color='#e67e22', lw=1.5)
ax.add_patch(arc_theta)
ax.text(-0.25, 0.7, r"$\theta$", color='#e67e22', fontsize=9.5, weight='bold')

arc_psi = patches.Arc((0, 0), 1.6, 1.6, angle=0, theta1=86, theta2=90, color='#8e44ad', lw=1.5)
ax.add_patch(arc_psi)

plt.title("Figure 1.3: Geometric Vectors in the Blinn-Phong Illumination Model",
          fontsize=10.5, weight='bold', pad=15, color='#1a252f')
plt.tight_layout()
plt.savefig('report_assets/blinn_phong_vectors.png', bbox_inches='tight')
plt.close()

# -------------------------------------------------------------
# 4. Light Attenuation Curves
# -------------------------------------------------------------
fig, ax = plt.subplots(figsize=(7, 4), dpi=300)
d = np.linspace(0.1, 35, 300)

# Formula: 1.0 / (kc + kl*d + kq*d^2)
# Dock Lamp: kc=1.0, kl=0.09, kq=0.02
atten_dock = 1.0 / (1.0 + 0.09 * d + 0.02 * d**2)
# Crane Worklight: kc=1.0, kl=0.04, kq=0.008
atten_crane = 1.0 / (1.0 + 0.04 * d + 0.008 * d**2)
# Main Point Light: kc=1.0, kl=0.06, kq=0.012
atten_main = 1.0 / (1.0 + 0.06 * d + 0.012 * d**2)

ax.plot(d, atten_crane, label=r'Crane Spotlight ($k_c=1.0, k_l=0.04, k_q=0.008$)', color='#e67e22', lw=2.2)
ax.plot(d, atten_main, label=r'Main Point Light ($k_c=1.0, k_l=0.06, k_q=0.012$)', color='#2980b9', lw=2.2)
ax.plot(d, atten_dock, label=r'Dock Sodium Lamps ($k_c=1.0, k_l=0.09, k_q=0.020$)', color='#27ae60', lw=2.2)

ax.set_xlabel('Distance from Light Source $d$ (meters)', fontsize=9.5, weight='bold', color='#2c3e50')
ax.set_ylabel('Attenuation Factor $Atten(d)$', fontsize=9.5, weight='bold', color='#2c3e50')
ax.set_title('Figure 1.4: Distance Attenuation Falloff Characteristics for Simulation Light Sources', fontsize=10.5, weight='bold', color='#1a252f', pad=12)
ax.grid(True, linestyle='--', alpha=0.6)
ax.legend(fontsize=8.5, loc='upper right')
ax.set_ylim(0, 1.05)
plt.tight_layout()
plt.savefig('report_assets/light_attenuation.png', bbox_inches='tight')
plt.close()

# -------------------------------------------------------------
# 5. Gouraud vs Phong Shading Comparison Diagram
# -------------------------------------------------------------
fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(9.5, 4.2), dpi=300)

for ax in (ax1, ax2):
    ax.set_xlim(-0.2, 3.2)
    ax.set_ylim(-0.3, 2.5)
    ax.axis('off')

# Gouraud (Per-Vertex)
ax1.plot([0, 3], [0.3, 0.3], color='#34495e', lw=3)
ax1.plot([0, 0], [0.3, 1.8], color='#27ae60', lw=2, ls='--')
ax1.plot([3, 3], [0.3, 1.8], color='#27ae60', lw=2, ls='--')
ax1.scatter([0, 3], [0.3, 0.3], color='#c0392b', s=100, zorder=5)
ax1.text(0, 0.05, "$V_1$ (Vertex 1)", ha='center', weight='bold', fontsize=8.5)
ax1.text(3, 0.05, "$V_2$ (Vertex 2)", ha='center', weight='bold', fontsize=8.5)

# Calculate lighting at vertices
ax1.scatter([0, 3], [1.8, 0.7], color='#f39c12', s=90, zorder=5)
ax1.plot([0, 3], [1.8, 0.7], color='#e67e22', lw=2.5)
ax1.text(0, 2.05, "$I(V_1)$ computed", ha='center', color='#d35400', weight='bold', fontsize=8)
ax1.text(3, 0.95, "$I(V_2)$ computed", ha='center', color='#d35400', weight='bold', fontsize=8)
ax1.text(1.5, 1.45, "Linear Interpolation of\nLight Intensity $I(s)$", ha='center', color='#2980b9', fontsize=8.5, weight='bold')
ax1.set_title("Gouraud Shading (Per-Vertex)\n[Evaluated in Vertex Shader]", fontsize=9.5, weight='bold', color='#1a252f')

# Phong (Per-Fragment)
ax2.plot([0, 3], [0.3, 0.3], color='#34495e', lw=3)
ax2.scatter([0, 3], [0.3, 0.3], color='#c0392b', s=100, zorder=5)
ax2.text(0, 0.05, "$V_1$", ha='center', weight='bold', fontsize=8.5)
ax2.text(3, 0.05, "$V_2$", ha='center', weight='bold', fontsize=8.5)

# Show interpolated normals
xs = np.linspace(0.3, 2.7, 7)
for x in xs:
    norm_angle = 90 - (x - 1.5) * 20
    rad = np.radians(norm_angle)
    dx = 0.5 * np.cos(rad)
    dy = 0.5 * np.sin(rad)
    ax2.annotate('', xy=(x+dx, 0.3+dy), xytext=(x, 0.3),
                 arrowprops=dict(arrowstyle="->", lw=1.5, color='#27ae60'))
ax2.text(1.5, 1.1, "Interpolated Normals $\\vec{N}(s)$\nFull Illumination per Fragment!",
         ha='center', color='#8e44ad', fontsize=8.5, weight='bold')

# Specular peak curve
curve_x = np.linspace(0, 3, 100)
curve_y = 0.3 + 1.6 * np.exp(-((curve_x - 1.5)**2) / 0.18)
ax2.plot(curve_x, curve_y, color='#9b59b6', lw=2.5)
ax2.text(1.5, 2.1, "Sharp Specular Peak\nAccurately Preserved!", ha='center', color='#8e44ad', weight='bold', fontsize=8)
ax2.set_title("Phong Shading (Per-Fragment)\n[Evaluated in Fragment Shader]", fontsize=9.5, weight='bold', color='#1a252f')

fig.suptitle("Figure 1.5: Gouraud (Per-Vertex) vs. Phong (Per-Fragment) Shading Interpolation Mechanism",
             fontsize=10.5, weight='bold', y=1.02, color='#1a252f')
plt.tight_layout()
plt.savefig('report_assets/gouraud_vs_phong.png', bbox_inches='tight')
plt.close()

# -------------------------------------------------------------
# 6. Crane Hierarchical Kinematic Tree
# -------------------------------------------------------------
fig, ax = plt.subplots(figsize=(9, 4.5), dpi=300)
ax.set_xlim(0, 10)
ax.set_ylim(0, 5)
ax.axis('off')

tree_nodes = [
    ("Gantry Base\n(Fixed World Pos $T(8,0,8.35)$)", 5.0, 4.2, "#2c3e50"),
    ("Rotating Turret / Cab\n(Slewing Yaw $R_y(\\theta_{slew})$)", 5.0, 3.0, "#2980b9"),
    ("Jib Boom Arm\n(Horizontal Cantilever)", 3.0, 1.8, "#16a085"),
    ("Operator Cab\n(360° Glass Interior)", 7.0, 1.8, "#8e44ad"),
    ("Trolley Carriage\n($T_z(\\Delta z_{trolley})$)", 3.0, 0.6, "#d35400"),
    ("Spreader Hook & Cables\n($T_y(-\\Delta y_{hook})$)", 5.0, 0.6, "#c0392b"),
    ("Attached Cargo Container\n(Intermodal Box)", 7.2, 0.6, "#f39c12")
]

for label, cx, cy, col in tree_nodes:
    rect = patches.FancyBboxPatch((cx-1.0, cy-0.45), 2.0, 0.9, boxstyle="round,pad=0.08,rounding_size=0.12",
                                  facecolor=col, edgecolor='#1a252f', linewidth=1.3)
    ax.add_patch(rect)
    ax.text(cx, cy, label, ha='center', va='center', color='white', fontsize=7.5, weight='bold')

# Tree arrows
ax.annotate('', xy=(5.0, 3.45), xytext=(5.0, 3.75), arrowprops=dict(arrowstyle="->", lw=2, color='#34495e'))
ax.annotate('', xy=(3.0, 2.25), xytext=(4.5, 2.55), arrowprops=dict(arrowstyle="->", lw=2, color='#34495e'))
ax.annotate('', xy=(7.0, 2.25), xytext=(5.5, 2.55), arrowprops=dict(arrowstyle="->", lw=2, color='#34495e'))
ax.annotate('', xy=(3.0, 1.05), xytext=(3.0, 1.35), arrowprops=dict(arrowstyle="->", lw=2, color='#34495e'))
ax.annotate('', xy=(4.5, 0.6), xytext=(4.0, 0.6), arrowprops=dict(arrowstyle="->", lw=2, color='#34495e'))
ax.annotate('', xy=(6.2, 0.6), xytext=(6.0, 0.6), arrowprops=dict(arrowstyle="->", lw=2, color='#34495e'))

plt.title("Figure 1.6: Multi-Level Hierarchical Kinematic Transformation Tree for Ship-to-Shore Container Crane",
          fontsize=10.5, weight='bold', pad=15, color='#1a252f')
plt.tight_layout()
plt.savefig('report_assets/crane_hierarchy_tree.png', bbox_inches='tight')
plt.close()

# -------------------------------------------------------------
# 7. Whitted Ray Tracing Principles Diagram
# -------------------------------------------------------------
fig, ax = plt.subplots(figsize=(8, 4.5), dpi=300)
ax.set_xlim(-1, 9)
ax.set_ylim(-1, 5)
ax.axis('off')

# Camera
ax.scatter([0], [2], color='#2980b9', s=160, zorder=5)
ax.text(-0.2, 2.3, "Camera\n$ro$", color='#2980b9', weight='bold', fontsize=9, ha='right')

# View Plane / Pixel
ax.plot([1.5, 1.5], [0.8, 3.2], color='#7f8c8d', lw=2.5)
ax.scatter([1.5], [2.1], color='#e74c3c', s=50, zorder=5)
ax.text(1.5, 3.4, "Image Plane", color='#7f8c8d', weight='bold', fontsize=8.5, ha='center')

# Primary Ray
ax.annotate('', xy=(4.5, 2.3), xytext=(0, 2), arrowprops=dict(arrowstyle="->", lw=2.2, color='#e74c3c'))
ax.text(2.6, 2.4, "Primary Ray ($ro + t\\cdot rd$)", color='#e74c3c', fontsize=8.5, weight='bold')

# Reflective Sphere 1
circle1 = patches.Circle((4.5, 2.3), 0.9, facecolor='#3498db', edgecolor='#1a5276', lw=2, alpha=0.8)
ax.add_patch(circle1)
ax.text(4.5, 2.3, "Sphere 1\n(Chrome)", color='white', weight='bold', fontsize=8, ha='center', va='center')

# Light Source
ax.scatter([4.0], [4.5], color='#f1c40f', s=250, zorder=6)
ax.text(4.0, 4.85, "Light Source ($L$)", color='#b7950b', weight='bold', fontsize=9, ha='center')

# Shadow Ray
ax.plot([4.5, 4.0], [2.3, 4.5], color='#f39c12', lw=1.8, ls=':')
ax.text(4.7, 3.6, "Shadow Ray", color='#d68910', fontsize=8, weight='bold')

# Secondary Reflection Ray
ax.annotate('', xy=(7.5, 0.8), xytext=(4.5, 2.3), arrowprops=dict(arrowstyle="->", lw=2.2, color='#9b59b6', ls='--'))
ax.text(6.4, 1.9, "Reflected Ray\n$reflect(rd, \\vec{N})$", color='#8e44ad', fontsize=8, weight='bold')

# Sphere 2
circle2 = patches.Circle((7.5, 0.8), 0.7, facecolor='#2ecc71', edgecolor='#196f3d', lw=2, alpha=0.8)
ax.add_patch(circle2)
ax.text(7.5, 0.8, "Sphere 2", color='white', weight='bold', fontsize=8, ha='center', va='center')

plt.title("Figure 1.7: Whitted-Style Recursive Ray Tracing Architecture (Primary, Shadow, & Reflection Rays)",
          fontsize=10.5, weight='bold', pad=15, color='#1a252f')
plt.tight_layout()
plt.savefig('report_assets/raytracing_diagram.png', bbox_inches='tight')
plt.close()

print("All 7 diagrams generated successfully!")
