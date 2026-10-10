#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "font8x8.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using glm::mat3;
using glm::mat4;
using glm::vec2;
using glm::vec3;
using glm::vec4;

static constexpr float PI = 3.14159265359f;

// ---------------- VERTEX & MESH STRUCTURES ----------------
struct Vertex {
    vec3 pos;
    vec3 normal;
};

struct Mesh {
    GLuint vao = 0, vbo = 0, ebo = 0;
    GLsizei indexCount = 0;

    void upload(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices) {
        indexCount = static_cast<GLsizei>(indices.size());
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(vertices.size() * sizeof(Vertex)),
                     vertices.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     static_cast<GLsizeiptr>(indices.size() * sizeof(unsigned int)),
                     indices.data(), GL_STATIC_DRAW);

        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, pos)));

        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void*>(offsetof(Vertex, normal)));

        glBindVertexArray(0);
    }

    void draw() const {
        glBindVertexArray(vao);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }

    void destroy() {
        if (ebo) glDeleteBuffers(1, &ebo);
        if (vbo) glDeleteBuffers(1, &vbo);
        if (vao) glDeleteVertexArrays(1, &vao);
        vao = vbo = ebo = 0;
        indexCount = 0;
    }
};

struct Shader {
    GLuint id = 0;

    static std::string readFile(const std::string& path) {
        std::ifstream file(path);
        if (!file) throw std::runtime_error("Could not open shader: " + path);
        std::stringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }

    static GLuint compile(GLenum type, const std::string& source, const std::string& label) {
        GLuint shader = glCreateShader(type);
        const char* src = source.c_str();
        glShaderSource(shader, 1, &src, nullptr);
        glCompileShader(shader);
        GLint success = GL_FALSE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLint logLength = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
            std::string log(static_cast<size_t>(std::max(1, logLength)), '\0');
            glGetShaderInfoLog(shader, logLength, nullptr, log.data());
            glDeleteShader(shader);
            throw std::runtime_error("Shader compile failed (" + label + "):\n" + log);
        }
        return shader;
    }

    void load(const std::string& vertexPath, const std::string& fragmentPath) {
        const GLuint vs = compile(GL_VERTEX_SHADER, readFile(vertexPath), vertexPath);
        const GLuint fs = compile(GL_FRAGMENT_SHADER, readFile(fragmentPath), fragmentPath);

        id = glCreateProgram();
        glAttachShader(id, vs);
        glAttachShader(id, fs);
        glLinkProgram(id);

        GLint success = GL_FALSE;
        glGetProgramiv(id, GL_LINK_STATUS, &success);
        if (!success) {
            GLint logLength = 0;
            glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logLength);
            std::string log(static_cast<size_t>(std::max(1, logLength)), '\0');
            glGetProgramInfoLog(id, logLength, nullptr, log.data());
            glDeleteShader(vs);
            glDeleteShader(fs);
            glDeleteProgram(id);
            id = 0;
            throw std::runtime_error("Program link failed:\n" + log);
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
    }

    void use() const { glUseProgram(id); }
    GLint loc(const char* name) const { return glGetUniformLocation(id, name); }
    void setMat4(const char* name, const mat4& m) const { glUniformMatrix4fv(loc(name), 1, GL_FALSE, glm::value_ptr(m)); }
    void setVec4(const char* name, const vec4& v) const { glUniform4fv(loc(name), 1, glm::value_ptr(v)); }
    void setVec3(const char* name, const vec3& v) const { glUniform3fv(loc(name), 1, glm::value_ptr(v)); }
    void setVec2(const char* name, const vec2& v) const { glUniform2fv(loc(name), 1, glm::value_ptr(v)); }
    void setFloat(const char* name, float value) const { glUniform1f(loc(name), value); }
    void setBool(const char* name, bool value) const { glUniform1i(loc(name), value ? 1 : 0); }
    void destroy() { if (id) glDeleteProgram(id); id = 0; }
};

// ---------------- GEOMETRY PRIMITIVES ----------------
static Mesh makeCube() {
    const vec3 p[8] = {
        {-0.5f,-0.5f,-0.5f},{ 0.5f,-0.5f,-0.5f},{ 0.5f, 0.5f,-0.5f},{-0.5f, 0.5f,-0.5f},
        {-0.5f,-0.5f, 0.5f},{ 0.5f,-0.5f, 0.5f},{ 0.5f, 0.5f, 0.5f},{-0.5f, 0.5f, 0.5f}
    };
    const int faces[6][4] = {{0,1,2,3},{4,7,6,5},{0,4,5,1},{3,2,6,7},{0,3,7,4},{1,5,6,2}};
    const vec3 normals[6] = {{0,0,-1},{0,0,1},{0,-1,0},{0,1,0},{-1,0,0},{1,0,0}};
    std::vector<Vertex> v;
    std::vector<unsigned int> e;
    for (int f = 0; f < 6; ++f) {
        unsigned int base = static_cast<unsigned int>(v.size());
        for (int i = 0; i < 4; ++i) v.push_back({p[faces[f][i]], normals[f]});
        e.insert(e.end(), {base, base+1, base+2, base, base+2, base+3});
    }
    Mesh m; m.upload(v,e); return m;
}

static Mesh makeCylinder(int segments = 32) {
    std::vector<Vertex> v;
    std::vector<unsigned int> e;
    const float r = 0.5f;
    const float y0 = -0.5f, y1 = 0.5f;

    for (int i = 0; i < segments; ++i) {
        const float a = 2.0f * PI * i / segments;
        const float x = r * std::cos(a), z = r * std::sin(a);
        const vec3 n = glm::normalize(vec3(x,0,z));
        v.push_back({{x,y0,z}, n});
        v.push_back({{x,y1,z}, n});
    }
    for (int i = 0; i < segments; ++i) {
        int ni = (i + 1) % segments;
        unsigned int a = 2u*i, b = 2u*ni, c = 2u*ni+1, d = 2u*i+1;
        e.insert(e.end(), {a,b,c, a,c,d});
    }

    const unsigned int bottomCenter = static_cast<unsigned int>(v.size());
    v.push_back({{0,y0,0},{0,-1,0}});
    const unsigned int topCenter = static_cast<unsigned int>(v.size());
    v.push_back({{0,y1,0},{0,1,0}});
    for (int i = 0; i < segments; ++i) {
        const int ni = (i + 1) % segments;
        e.insert(e.end(), {bottomCenter, static_cast<unsigned int>(2*ni), static_cast<unsigned int>(2*i)});
        e.insert(e.end(), {topCenter, static_cast<unsigned int>(2*i+1), static_cast<unsigned int>(2*ni+1)});
    }
    Mesh m; m.upload(v,e); return m;
}

static Mesh makeCone(int segments = 32) {
    std::vector<Vertex> v;
    std::vector<unsigned int> e;
    const float r = 0.5f;
    const float y0 = -0.5f, y1 = 0.5f;

    // Apex vertex (top)
    const unsigned int apexIndex = static_cast<unsigned int>(v.size());
    v.push_back({{0.0f, y1, 0.0f}, {0.0f, 1.0f, 0.0f}});

    // Side vertices along circular base
    for (int i = 0; i < segments; ++i) {
        const float a = 2.0f * PI * i / segments;
        const float x = r * std::cos(a), z = r * std::sin(a);
        const vec3 n = glm::normalize(vec3(x, 0.5f, z));
        v.push_back({{x, y0, z}, n});
    }

    // Side triangles connecting apex to base ring
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        e.push_back(apexIndex);
        e.push_back(1 + i);
        e.push_back(1 + next);
    }

    // Base center vertex
    const unsigned int baseCenter = static_cast<unsigned int>(v.size());
    v.push_back({{0.0f, y0, 0.0f}, {0.0f, -1.0f, 0.0f}});

    // Base circular fan
    for (int i = 0; i < segments; ++i) {
        int next = (i + 1) % segments;
        e.push_back(baseCenter);
        e.push_back(1 + next);
        e.push_back(1 + i);
    }

    Mesh m; m.upload(v, e); return m;
}

static Mesh makeRing(int segments = 32, float inner = 0.33f, float outer = 0.5f) {
    std::vector<Vertex> v;
    std::vector<unsigned int> e;
    for (int i = 0; i < segments; ++i) {
        float a = 2.0f * PI * i / segments;
        float c = std::cos(a), s = std::sin(a);
        v.push_back({{outer*c,0,outer*s},{0,1,0}});
        v.push_back({{inner*c,0,inner*s},{0,1,0}});
    }
    for (int i = 0; i < segments; ++i) {
        int ni = (i+1)%segments;
        e.insert(e.end(), {2u*i,2u*ni,2u*i+1, 2u*i+1,2u*ni,2u*ni+1});
    }
    Mesh m; m.upload(v,e); return m;
}

static Mesh makeSphere(int latSeg = 18, int lonSeg = 28, float radius = 1.0f) {
    std::vector<Vertex> v;
    std::vector<unsigned int> e;
    for (int i = 0; i <= latSeg; ++i) {
        float theta = i * PI / latSeg;
        float sinT = std::sin(theta);
        float cosT = std::cos(theta);
        for (int j = 0; j <= lonSeg; ++j) {
            float phi = j * 2.0f * PI / lonSeg;
            float x = radius * sinT * std::cos(phi);
            float y = radius * cosT;
            float z = radius * sinT * std::sin(phi);
            vec3 n = glm::normalize(vec3(x, y, z));
            v.push_back({{x, y, z}, n});
        }
    }
    for (int i = 0; i < latSeg; ++i) {
        for (int j = 0; j < lonSeg; ++j) {
            unsigned int p1 = i * (lonSeg + 1) + j;
            unsigned int p2 = p1 + lonSeg + 1;
            e.push_back(p1);
            e.push_back(p2);
            e.push_back(p1 + 1);
            e.push_back(p1 + 1);
            e.push_back(p2);
            e.push_back(p2 + 1);
        }
    }
    Mesh m; m.upload(v, e); return m;
}

// Inverted Sky Dome mesh for rendering realistic skies
static Mesh makeSkyDome(int latSeg = 20, int lonSeg = 32, float radius = 80.0f) {
    std::vector<Vertex> v;
    std::vector<unsigned int> e;
    for (int i = 0; i <= latSeg; ++i) {
        float theta = (i * 0.5f * PI) / latSeg; // Hemisphere
        float sinT = std::sin(theta);
        float cosT = std::cos(theta);
        for (int j = 0; j <= lonSeg; ++j) {
            float phi = j * 2.0f * PI / lonSeg;
            float x = radius * sinT * std::cos(phi);
            float y = radius * cosT;
            float z = radius * sinT * std::sin(phi);
            vec3 n = -glm::normalize(vec3(x, y, z)); // Normals inward
            v.push_back({{x, y, z}, n});
        }
    }
    for (int i = 0; i < latSeg; ++i) {
        for (int j = 0; j < lonSeg; ++j) {
            unsigned int p1 = i * (lonSeg + 1) + j;
            unsigned int p2 = p1 + lonSeg + 1;
            e.push_back(p1);
            e.push_back(p1 + 1);
            e.push_back(p2);
            e.push_back(p1 + 1);
            e.push_back(p2 + 1);
            e.push_back(p2);
        }
    }
    Mesh m; m.upload(v, e); return m;
}

// Subdivided water grid sized to fit precisely within the canal channel (Z: -5.3 to +5.3, X: -320 to +320)
static Mesh makeWaterGrid(int gridX = 140, int gridZ = 24, float sizeX = 640.0f, float sizeZ = 10.6f) {
    std::vector<Vertex> v;
    std::vector<unsigned int> e;
    float dx = sizeX / gridX;
    float dz = sizeZ / gridZ;
    float startX = -sizeX * 0.5f;
    float startZ = -sizeZ * 0.5f;

    for (int j = 0; j <= gridZ; ++j) {
        for (int i = 0; i <= gridX; ++i) {
            float x = startX + i * dx;
            float z = startZ + j * dz;
            v.push_back({{x, 0.0f, z}, {0.0f, 1.0f, 0.0f}});
        }
    }

    for (int j = 0; j < gridZ; ++j) {
        for (int i = 0; i < gridX; ++i) {
            unsigned int row1 = j * (gridX + 1);
            unsigned int row2 = (j + 1) * (gridX + 1);
            e.push_back(row1 + i);
            e.push_back(row2 + i);
            e.push_back(row1 + i + 1);

            e.push_back(row1 + i + 1);
            e.push_back(row2 + i);
            e.push_back(row2 + i + 1);
        }
    }
    Mesh m; m.upload(v, e); return m;
}

// Fullscreen quad for Ray Tracing demonstration
static GLuint quadVAO = 0, quadVBO = 0;
static void initQuad() {
    float quadVertices[] = {
        -1.0f,  1.0f,
        -1.0f, -1.0f,
         1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

static void renderQuad() {
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

// ---------------- 2D HUD VERTEX BUFFER & BITMAP FONT ----------------
struct HUDVertex {
    vec2 pos;
    vec2 uv;
};

static GLuint hudVAO = 0, hudVBO = 0, fontTexID = 0;

static void initHUD() {
    glGenVertexArrays(1, &hudVAO);
    glGenBuffers(1, &hudVBO);

    glBindVertexArray(hudVAO);
    glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
    glBufferData(GL_ARRAY_BUFFER, 4096 * sizeof(HUDVertex), nullptr, GL_DYNAMIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(HUDVertex), (void*)offsetof(HUDVertex, pos));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(HUDVertex), (void*)offsetof(HUDVertex, uv));

    glBindVertexArray(0);

    // Unpack 8x8 font into 128x64 bitmap texture (16 chars wide x 8 chars high)
    std::vector<uint8_t> fontPixels(128 * 64, 0);
    for (int ch = 0; ch < 128; ++ch) {
        int cellX = (ch % 16) * 8;
        int cellY = (ch / 16) * 8;
        for (int r = 0; r < 8; ++r) {
            uint8_t rowBits = font8x8_basic[ch][r];
            for (int c = 0; c < 8; ++c) {
                // Bit 7 is leftmost pixel (col c=0), Bit 0 is rightmost pixel (col c=7)
                if ((rowBits >> (7 - c)) & 1) {
                    fontPixels[(cellY + r) * 128 + (cellX + c)] = 255;
                }
            }
        }
    }

    glGenTextures(1, &fontTexID);
    glBindTexture(GL_TEXTURE_2D, fontTexID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, 128, 64, 0, GL_RED, GL_UNSIGNED_BYTE, fontPixels.data());
    glBindTexture(GL_TEXTURE_2D, 0);
}

static Mesh cubeMesh, cylinderMesh, ringMesh, sphereMesh, coneMesh, waterMesh, skyDomeMesh;

// Shaders
static Shader gouraudShader, phongShader, raytraceShader, skyShader, hudShader;
enum class ShadingMode { Phong, Gouraud };
static ShadingMode currentShading = ShadingMode::Phong;
static bool raytraceMode = false;

enum class CameraMode { FPSOperator, Free, Overview, CraneCab, ShipFollow, LighthouseView, LorryDrive };
static CameraMode currentCamMode = CameraMode::FPSOperator;

// ---------------- 1ST-PERSON OPERATOR (PLAYER) CONTROLLER ----------------
struct Player {
    vec3 pos = vec3(0.0f, 0.0f, 18.5f); // Start on the dock road
    float yaw = -90.0f; // Look north towards ships and crane
    float pitch = 0.0f;
    float eyeHeight = 1.75f;
    float speed = 8.5f;
    float sprintMultiplier = 1.9f;
    float verticalVelocity = 0.0f;
    bool isGrounded = true;
    bool isClimbing = false;
    float headBob = 0.0f;
    bool cursorLocked = true;
    bool inCraneCab = false;
};

static Player player;
static double lastMouseX = 720.0, lastMouseY = 440.0;
static bool firstMouse = true;

// Orbit Camera (Used for Free, Overview, CraneCab, etc.)
struct Camera {
    float yaw = -55.0f;
    float pitch = 32.0f;
    float distance = 46.0f;
    vec3 target = {0.0f, 1.5f, 2.0f};

    vec3 position() const {
        float cy = std::cos(glm::radians(yaw)), sy = std::sin(glm::radians(yaw));
        float cp = std::cos(glm::radians(pitch)), sp = std::sin(glm::radians(pitch));
        return target + vec3(distance * cp * cy, distance * sp, distance * cp * sy);
    }

    mat4 view() const { return glm::lookAt(position(), target, vec3(0,1,0)); }
};

enum class ShipType { ContainerFeeder, Tugboat, CoastalTanker, PilotBoat, BulkCarrier };

struct Ship {
    float x = -32.0f;
    float z = 0.0f;
    float speed = 1.8f;      // Gentle realistic cruising speed
    float heading = 0.0f;    // 0 = +X (Eastbound), 180 = -X (Westbound)
    float scale = 1.0f;
    vec3 color{0.18f, 0.42f, 0.82f};
    ShipType type = ShipType::ContainerFeeder;
    bool active = true;
    bool isTug = false;
    float phase = 0.0f;
};

// Interactive 3D Crane Operator State
struct Crane {
    enum class AutoState { Idle, ToShip, LowerToShip, GrabCargo, LiftCargo, ToYard, LowerToYard, ReleaseCargo, ReturnCab };
    AutoState autoState = AutoState::Idle;
    float autoTimer = 0.0f;

    bool manual = false;
    float slewingYaw = 0.0f;      // Rotation around vertical turret axis (deg)
    float trolleyZ = 1.0f;        // Position along the boom (over ship: 0.5 ~ over dock bay: 13.5)
    float hookY = 7.5f;           // Height of hook (1.8f to 12.2f)
    float trolleySpeed = 5.2f;
    float hookSpeed = 4.0f;

    bool carrying = false;
    int cargoIndex = 0;
    int score = 0;
    int unloadedCount = 0;
    int camView = 0;              // 0 = 1st-Person Cabin (Zoomable FOV), 1 = 3rd-Person Tactical Orbit Overview, 2 = Spreader Precision Drop Cam
    float fov = 64.0f;            // Zoomable FOV in cabin (35.0f to 85.0f)
    float orbitDist = 28.0f;      // Orbit distance in 3rd-person overview (10.0f to 60.0f)
    bool spreaderCam = false;
    std::string toastMessage = "";
    float toastTimer = 0.0f;
};

// Objectives System (Inspired by the Reference Project)
struct Objective {
    std::string text;
    vec3 targetPos;
    bool completed = false;
};

static std::vector<Objective> objectives = {
    {"INSPECT SHIP-TO-SHORE CRANE", vec3(8.0f, 0.0f, 8.35f), false},
    {"BOARD CONTAINER SHIP DOCK",   vec3(8.0f, 0.5f, 0.5f), false},
    {"INSPECT BASCULE DRAWBRIDGE",  vec3(0.0f, 0.4f, 5.6f), false},
    {"REPORT TO HARBOR HEADQUARTERS",vec3(0.0f, 0.0f, 56.0f), false},
    {"CLIMB PERIMETER WATCHTOWER",  vec3(-45.0f, 8.5f, 24.0f), false}
};
static int currentObjectiveIndex = 0;

static Camera camera;
static std::vector<Ship> ships;
static Crane crane;

// Drivable Cargo Lorry (Port Container Transport Truck)
struct Lorry {
    vec3 pos = vec3(8.0f, 0.0f, 13.5f);   // Parked at crane dock unloading bay
    float yaw = 0.0f;                     // Heading (deg, 0 = +X along the dock road)
    float speed = 0.0f;                   // Current speed (m/s)
    float steerAngle = 0.0f;              // Steering angle (deg)
    float maxSpeed = 16.0f;               // Max road speed
    float accel = 11.0f;                  // Acceleration rate
    float brake = 16.0f;                  // Braking rate
    bool hasCargo = false;                // Carrying container on trailer flatbed
    int cargoIndex = 0;                   // Which container is loaded
    bool playerDriving = false;           // Player in driver seat
    bool chaseCam = false;                // Toggle between 1st person cab POV & 3rd person chase cam
    int deliveriesCompleted = 0;          // Total successful deliveries to depot
};
static Lorry lorry;
static const vec3 freightDepotPos = vec3(56.0f, 0.0f, 18.5f); // Inland distribution depot on dock expressway

static float bridgeAngle = 0.0f;
static bool autoBridge = true;
static bool paused = false;
static bool nightLights = false;
static float simTime = 0.0f;
static float shipSpeedMultiplier = 1.0f;
static float lighthouseAngle = 0.0f;

// Container Colors (Realistic Maersk, Evergreen, Hapag-Lloyd palette)
static const vec3 COLORS[] = {
    {0.18f, 0.48f, 0.85f}, // Maersk Sky Blue
    {0.12f, 0.62f, 0.35f}, // Evergreen Marine
    {0.85f, 0.28f, 0.18f}, // CMA CGM Brick Red
    {0.92f, 0.55f, 0.12f}, // Hapag-Lloyd Orange
    {0.55f, 0.25f, 0.70f}, // Deep Violet
    {0.88f, 0.85f, 0.82f}  // Cold Steel White
};

// Gamified Interactive Ship Cargo System (Serial Container Selection & Unloading)
struct ShipCargo {
    vec3 localPos;
    vec3 color;
    bool unloaded = false;
    std::string name;
};

static std::vector<ShipCargo> shipCargos = {
    {{ 0.0f, 1.65f, 0.0f},  {0.18f, 0.48f, 0.85f}, false, "CARGO #1 (MAERSK BLUE)"},
    {{ 2.4f, 1.65f, 0.0f},  {0.92f, 0.55f, 0.12f}, false, "CARGO #2 (HAPAG ORANGE)"},
    {{-2.4f, 1.65f, 0.0f},  {0.12f, 0.62f, 0.35f}, false, "CARGO #3 (EVERGREEN GREEN)"},
    {{ 4.4f, 1.65f, 0.0f},  {0.85f, 0.28f, 0.18f}, false, "CARGO #4 (CMA BRICK RED)"},
    {{-4.4f, 2.85f, 0.0f},  {0.55f, 0.25f, 0.70f}, false, "CARGO #5 (OCEAN VIOLET)"},
    {{-4.4f, 1.65f, 0.0f},  {0.88f, 0.85f, 0.82f}, false, "CARGO #6 (STEEL WHITE)"}
};
static int activeTargetCargoIndex = 0;

static int getNextActiveCargoIndex() {
    for (size_t i = 0; i < shipCargos.size(); ++i) {
        if (!shipCargos[i].unloaded) return static_cast<int>(i);
    }
    return -1;
}

static Shader& activeShader() {
    return (currentShading == ShadingMode::Phong) ? phongShader : gouraudShader;
}

static mat4 T(const vec3& p) { return glm::translate(mat4(1.0f), p); }
static mat4 S(const vec3& s) { return glm::scale(mat4(1.0f), s); }
static mat4 R(float deg, const vec3& axis) { return glm::rotate(mat4(1.0f), glm::radians(deg), axis); }

static void drawMesh(const Mesh& mesh, const mat4& model, const vec3& color, float shininess = 32.0f,
                     const vec3& specularColor = vec3(0.45f), bool isWater = false, bool isEmissive = false, float alpha = 1.0f) {
    Shader& s = activeShader();
    s.setMat4("uModel", model);
    s.setVec3("uColor", color);
    s.setFloat("uShininess", shininess);
    s.setVec3("uSpecularColor", specularColor);
    s.setBool("uIsWater", isWater);
    s.setBool("uIsEmissive", isEmissive);
    s.setFloat("uAlpha", alpha);
    s.setFloat("uTime", simTime);
    if (alpha < 0.99f) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    mesh.draw();
    if (alpha < 0.99f) {
        glDisable(GL_BLEND);
    }
}

static void drawGlassCube(const mat4& model, const vec3& size, const vec3& color = vec3(0.55f, 0.82f, 0.98f), float alpha = 0.22f, float shininess = 128.0f) {
    drawMesh(cubeMesh, model * S(size), color, shininess, vec3(1.0f), false, false, alpha);
}

static void drawCube(const mat4& model, const vec3& color, float shininess = 32.0f, const vec3& spec = vec3(0.35f), bool emissive = false) {
    drawMesh(cubeMesh, model, color, shininess, spec, false, emissive);
}
static void drawCube(const mat4& model, const vec3& size, const vec3& color, float yaw = 0.0f,
                     float shininess = 32.0f, const vec3& spec = vec3(0.35f), bool emissive = false) {
    drawMesh(cubeMesh, model * R(yaw, {0,1,0}) * S(size), color, shininess, spec, false, emissive);
}
static void drawCube(const vec3& pos, const vec3& size, const vec3& color, float yaw = 0.0f,
                     float shininess = 32.0f, const vec3& spec = vec3(0.35f), bool emissive = false) {
    drawCube(T(pos), size, color, yaw, shininess, spec, emissive);
}

static void drawCylinder(const mat4& model, const vec3& color, float shininess = 32.0f, const vec3& spec = vec3(0.35f), bool emissive = false) {
    drawMesh(cylinderMesh, model, color, shininess, spec, false, emissive);
}
static void drawCylinder(const mat4& model, const vec3& size, const vec3& color, float yaw = 0.0f,
                         float shininess = 32.0f, const vec3& spec = vec3(0.35f), bool emissive = false) {
    drawMesh(cylinderMesh, model * R(yaw, {0,1,0}) * S(size), color, shininess, spec, false, emissive);
}
static void drawCylinder(const vec3& pos, const vec3& size, const vec3& color, float yaw = 0.0f,
                         float shininess = 32.0f, const vec3& spec = vec3(0.35f), bool emissive = false) {
    drawCylinder(T(pos), size, color, yaw, shininess, spec, emissive);
}

static void drawSphere(const mat4& model, const vec3& color, float shininess = 64.0f, const vec3& spec = vec3(0.5f), bool emissive = false) {
    drawMesh(sphereMesh, model, color, shininess, spec, false, emissive);
}
static void drawSphere(const mat4& model, float radius, const vec3& color, float shininess = 64.0f,
                       const vec3& spec = vec3(0.5f), bool emissive = false) {
    drawMesh(sphereMesh, model * S(vec3(radius)), color, shininess, spec, false, emissive);
}
static void drawSphere(const vec3& pos, float radius, const vec3& color, float shininess = 64.0f,
                       const vec3& spec = vec3(0.5f), bool emissive = false) {
    drawSphere(T(pos), radius, color, shininess, spec, emissive);
}

static void drawWheel(const vec3& pos, float radius, float width, const vec3& color, float pitch = 90.0f) {
    drawMesh(cylinderMesh, T(pos) * R(pitch,{1,0,0}) * S({radius,width,radius}), color, 16.0f);
}

static void drawCone(const mat4& model, const vec3& color, float shininess = 32.0f, const vec3& spec = vec3(0.35f), bool emissive = false) {
    drawMesh(coneMesh, model, color, shininess, spec, false, emissive);
}
static void drawCone(const mat4& model, const vec3& size, const vec3& color, float yaw = 0.0f,
                     float shininess = 32.0f, const vec3& spec = vec3(0.35f), bool emissive = false) {
    drawMesh(coneMesh, model * R(yaw, {0,1,0}) * S(size), color, shininess, spec, false, emissive);
}
static void drawCone(const vec3& pos, const vec3& size, const vec3& color, float yaw = 0.0f,
                     float shininess = 32.0f, const vec3& spec = vec3(0.35f), bool emissive = false) {
    drawCone(T(pos), size, color, yaw, shininess, spec, emissive);
}

// ---------------- CELESTIAL BODIES: 3D SUN & MOON (HIGH-ALTITUDE OVERHEAD DIRECTIONAL LIGHT SOURCE) ----------------
static void drawCelestialBodies(const vec3& camPos) {
    // Steep overhead celestial position matching optimal diffuse/specular shading mathematics
    vec3 sunDirection = nightLights ? vec3(0.35f, -0.82f, -0.45f) : vec3(-0.30f, -0.88f, 0.35f);
    vec3 celestialDir = glm::normalize(-sunDirection); // High overhead celestial vector
    vec3 celestialPos = camPos + celestialDir * 275.0f; // High in sky dome near far clipping plane

    glDepthFunc(GL_LEQUAL); // Ensure celestial body renders cleanly over sky dome depth buffer

    if (!nightLights) {
        // DAYTIME DIRECTIONAL LIGHT SOURCE: REALISTIC HIGH-ALTITUDE RADIANT SUN
        // 1. Crisp Brilliant Emissive Solar Core (Pure White-Gold)
        drawSphere(celestialPos, 5.2f, vec3(3.2f, 3.0f, 2.5f), 128.0f, vec3(1.0f), true);

        // 2. Soft Natural Solar Photosphere Glow
        drawSphere(celestialPos, 8.8f, vec3(1.8f, 1.4f, 0.6f), 64.0f, vec3(1.0f), true);

        // 3. Delicate Solar Corona Halo Rings facing camera
        vec3 toCam = glm::normalize(camPos - celestialPos);
        vec3 up = (std::abs(toCam.y) > 0.99f) ? vec3(0, 0, 1) : vec3(0, 1, 0);
        vec3 right = glm::normalize(glm::cross(toCam, up));
        vec3 realUp = glm::normalize(glm::cross(right, toCam));
        mat4 billMat(mat3(right, realUp, toCam));
        mat4 orient = T(celestialPos) * billMat;

        drawMesh(ringMesh, orient * S(vec3(15.0f, 1.0f, 15.0f)), vec3(1.5f, 1.2f, 0.5f), 64.0f, vec3(1.0f), false, true, 0.70f);
        drawMesh(ringMesh, orient * S(vec3(24.0f, 1.0f, 24.0f)), vec3(1.2f, 0.8f, 0.3f), 64.0f, vec3(1.0f), false, true, 0.40f);
    } else {
        // NIGHTTIME DIRECTIONAL LIGHT SOURCE: REALISTIC HIGH-ALTITUDE MOON
        // 1. Silver Luminous Moon Core
        drawSphere(celestialPos, 4.2f, vec3(1.3f, 1.4f, 1.7f), 128.0f, vec3(1.0f), true);

        // 2. Subtle Lunar Maria / Craters
        vec3 crater1 = celestialPos + glm::normalize(vec3(0.35f, 0.45f, 0.15f)) * 4.22f;
        drawSphere(crater1, 1.2f, vec3(0.75f, 0.82f, 0.95f), 32.0f, vec3(0.2f), true);
        vec3 crater2 = celestialPos + glm::normalize(vec3(-0.4f, -0.25f, 0.25f)) * 4.22f;
        drawSphere(crater2, 1.0f, vec3(0.70f, 0.76f, 0.90f), 32.0f, vec3(0.2f), true);

        // 3. Soft Cool-Blue Lunar Corona Glow
        drawSphere(celestialPos, 7.2f, vec3(0.45f, 0.60f, 0.95f), 64.0f, vec3(1.0f), true);

        vec3 toCam = glm::normalize(camPos - celestialPos);
        vec3 up = (std::abs(toCam.y) > 0.99f) ? vec3(0, 0, 1) : vec3(0, 1, 0);
        vec3 right = glm::normalize(glm::cross(toCam, up));
        vec3 realUp = glm::normalize(glm::cross(right, toCam));
        mat4 billMat(mat3(right, realUp, toCam));
        mat4 orient = T(celestialPos) * billMat;

        drawMesh(ringMesh, orient * S(vec3(12.0f, 1.0f, 12.0f)), vec3(0.40f, 0.60f, 1.1f), 64.0f, vec3(1.0f), false, true, 0.50f);
        drawMesh(ringMesh, orient * S(vec3(19.0f, 1.0f, 19.0f)), vec3(0.25f, 0.40f, 0.85f), 64.0f, vec3(1.0f), false, true, 0.25f);
    }

    glDepthFunc(GL_LESS); // Restore default depth testing
}

// Maritime Navigational Channel Buoy (Demonstrating Cone & Cylinder primitives)
static void drawNavigationalBuoy(const vec3& pos, const vec3& color, bool flashRed = true) {
    float bob = 0.12f * std::sin(simTime * 2.2f + pos.x);
    vec3 buoyPos = pos + vec3(0.0f, bob, 0.0f);
    // Floating cylindrical base
    drawCylinder(buoyPos + vec3(0, -0.2f, 0), {1.2f, 0.55f, 1.2f}, {0.18f, 0.20f, 0.22f}, 0.0f, 32.0f);
    // Conical buoy body
    drawCone(buoyPos + vec3(0, 0.65f, 0), {1.1f, 1.35f, 1.1f}, color, 0.0f, 48.0f);
    // Mast
    drawCylinder(buoyPos + vec3(0, 1.55f, 0), {0.10f, 0.75f, 0.10f}, {0.3f, 0.3f, 0.3f}, 0.0f, 32.0f);
    // Flashing signal light top
    bool flash = std::fmod(simTime * 2.0f + pos.z, 2.0f) > 1.0f;
    vec3 lightCol = flashRed ? (flash ? vec3(1.5f, 0.1f, 0.1f) : vec3(0.4f, 0.05f, 0.05f))
                             : (flash ? vec3(0.1f, 1.5f, 0.2f) : vec3(0.05f, 0.4f, 0.05f));
    drawSphere(buoyPos + vec3(0, 2.0f, 0), 0.18f, lightCol, 64.0f, vec3(1.0f), flash);
}

// Traffic Safety Cone (Demonstrating Cone primitive along port roadways)
static void drawTrafficCone(const vec3& pos) {
    // Black square base
    drawCube(pos + vec3(0, 0.03f, 0), {0.55f, 0.06f, 0.55f}, {0.12f, 0.12f, 0.14f}, 0.0f, 16.0f);
    // High-vis bright orange conical body
    drawCone(pos + vec3(0, 0.45f, 0), {0.45f, 0.85f, 0.45f}, {0.95f, 0.45f, 0.05f}, 0.0f, 48.0f);
    // White reflective collar band
    drawCylinder(pos + vec3(0, 0.42f, 0), {0.28f, 0.16f, 0.28f}, {0.95f, 0.95f, 0.98f}, 0.0f, 96.0f);
}

// ---------------- 1. REALISTIC PROCEDURAL SKY DOME & CLOUDS ----------------
static void drawSkyDome(const mat4& view, const mat4& proj) {
    glDepthFunc(GL_LEQUAL);
    skyShader.use();
    skyShader.setMat4("uView", view);
    skyShader.setMat4("uProjection", proj);

    vec3 sunDirection = nightLights ? vec3(0.35f, -0.82f, -0.45f) : vec3(-0.30f, -0.88f, 0.35f);
    vec3 sunLightColor = nightLights ? vec3(0.35f, 0.42f, 0.65f) : vec3(1.35f, 1.30f, 1.18f);

    skyShader.setVec3("uSunDir", sunDirection);
    skyShader.setVec3("uSunColor", sunLightColor);
    skyShader.setBool("uNight", nightLights);
    skyShader.setFloat("uTime", simTime);

    skyDomeMesh.draw();
    glDepthFunc(GL_LESS);
}

// ---------------- 2. ZERO-CLIPPING WATER CANAL & SEABED ----------------
static void drawWaterAndCanal() {
    Shader& s = activeShader();

    // Deep seabed underneath the canal (at Y = -2.8) extending with the full road width
    drawCube({0.0f, -2.8f, 0.0f}, {640.0f, 0.5f, 11.2f}, {0.14f, 0.17f, 0.18f}, 0.0f, 8.0f);

    // Dynamic water surface plane positioned precisely inside channel (Z: -5.3 to +5.3)
    mat4 model = T({0.0f, -0.55f, 0.0f});
    vec3 waterColor = nightLights ? vec3(0.04f, 0.16f, 0.30f) : vec3(0.10f, 0.44f, 0.70f);
    vec3 waterSpecular = vec3(0.95f, 0.98f, 1.0f);

    s.setMat4("uModel", model);
    s.setVec3("uColor", waterColor);
    s.setFloat("uShininess", 128.0f);
    s.setVec3("uSpecularColor", waterSpecular);
    s.setBool("uIsWater", true);
    s.setBool("uIsEmissive", false);
    s.setFloat("uTime", simTime);
    waterMesh.draw();
    s.setBool("uIsWater", false);

    // Navigational Channel Buoys (Demonstrating Cone & Cylinder Primitives)
    drawNavigationalBuoy({-35.0f, -0.55f, 3.8f}, {0.12f, 0.72f, 0.28f}, false); // Starboard Green Buoy
    drawNavigationalBuoy({ 35.0f, -0.55f, -3.8f}, {0.88f, 0.15f, 0.15f}, true);  // Port Red Buoy
    drawNavigationalBuoy({-95.0f, -0.55f, 3.8f}, {0.12f, 0.72f, 0.28f}, false);
    drawNavigationalBuoy({ 95.0f, -0.55f, -3.8f}, {0.88f, 0.15f, 0.15f}, true);
}

// ---------------- 3. EXTENDED ROADS, HIGHWAYS & SURROUNDING WORLD ----------------
static void drawTree(const vec3& pos, float scale = 1.0f, int variant = 0) {
    // Strictly safeguard: do NOT draw trees anywhere in or over the water!
    if (std::abs(pos.z) < 6.0f) return;
    if (pos.z < -24.0f && std::abs(pos.x) < 40.0f) return; // Sea channel north

    vec3 trunkCol{0.34f, 0.24f, 0.16f};
    vec3 leafCol1 = (variant % 2 == 0) ? vec3(0.18f, 0.50f, 0.22f) : vec3(0.14f, 0.44f, 0.18f);
    vec3 leafCol2 = (variant % 3 == 0) ? vec3(0.24f, 0.58f, 0.26f) : vec3(0.20f, 0.52f, 0.24f);

    // Trunk
    drawCylinder(pos + vec3(0, 1.4f * scale, 0), {0.45f * scale, 2.8f * scale, 0.45f * scale}, trunkCol, 0.0f, 16.0f);

    // Natural layered canopy
    drawSphere(pos + vec3(0, 3.6f * scale, 0), 1.8f * scale, leafCol1, 16.0f);
    drawSphere(pos + vec3(0.5f * scale, 4.4f * scale, 0.3f * scale), 1.4f * scale, leafCol2, 16.0f);
    drawSphere(pos + vec3(-0.4f * scale, 4.2f * scale, -0.4f * scale), 1.3f * scale, leafCol1, 16.0f);
    drawSphere(pos + vec3(0, 5.2f * scale, 0), 1.0f * scale, leafCol2, 16.0f);
}

static void drawStreetLamp(const vec3& pos, float yaw = 0.0f) {
    mat4 m = T(pos) * R(yaw, {0,1,0});
    // Tall octagonal steel pole
    drawCylinder(m * T({0, 4.0f, 0}), {0.18f, 8.0f, 0.18f}, {0.22f, 0.25f, 0.28f}, 0.0f, 48.0f);
    // Overhanging cantilever arm
    drawCylinder(m * T({0, 7.8f, -0.6f}), {0.12f, 1.6f, 0.12f}, {0.22f, 0.25f, 0.28f}, 65.0f);
    // Lamp fixture head
    drawCube(m * T({0, 7.55f, -1.25f}), {0.45f, 0.18f, 0.7f}, {0.20f, 0.22f, 0.24f}, 0.0f, 48.0f);
    // Sodium bulb glow
    vec3 bulbCol = nightLights ? vec3(1.4f, 1.15f, 0.55f) : vec3(0.6f, 0.6f, 0.55f);
    drawSphere(m * T({0, 7.45f, -1.25f}), 0.22f, bulbCol, 64.0f, vec3(1.0f), nightLights);
}

static void drawParkedCar(const vec3& pos, float yaw, const vec3& color) {
    mat4 m = T(pos) * R(yaw, {0,1,0});
    // Chassis
    drawCube(m * T({0, 0.45f, 0}), {3.8f, 0.65f, 1.8f}, color, 0.0f, 48.0f);
    // Cabin roof
    drawCube(m * T({-0.2f, 0.95f, 0}), {2.0f, 0.55f, 1.6f}, {0.18f, 0.22f, 0.26f}, 0.0f, 64.0f);
    // Windshield tint
    drawCube(m * T({0.85f, 0.85f, 0}), {0.2f, 0.4f, 1.4f}, {0.6f, 0.8f, 0.9f}, 0.0f, 96.0f);
    // Wheels
    drawWheel(pos + vec3(-1.1f, 0.3f,  0.95f), 0.32f, 0.18f, {0.08f, 0.08f, 0.08f});
    drawWheel(pos + vec3( 1.1f, 0.3f,  0.95f), 0.32f, 0.18f, {0.08f, 0.08f, 0.08f});
    drawWheel(pos + vec3(-1.1f, 0.3f, -0.95f), 0.32f, 0.18f, {0.08f, 0.08f, 0.08f});
    drawWheel(pos + vec3( 1.1f, 0.3f, -0.95f), 0.32f, 0.18f, {0.08f, 0.08f, 0.08f});
}

static void drawPerimeterFence(float startX, float endX, float z) {
    const float step = 4.0f;
    for (float x = startX; x <= endX; x += step) {
        // Steel fence post
        drawCylinder({x, 1.4f, z}, {0.12f, 2.8f, 0.12f}, {0.6f, 0.62f, 0.65f}, 0.0f, 48.0f);
        // Horizontal rails
        drawCylinder({x + step * 0.5f, 0.8f, z}, {step, 0.06f, 0.06f}, {0.55f, 0.58f, 0.60f}, 90.0f);
        drawCylinder({x + step * 0.5f, 2.0f, z}, {step, 0.06f, 0.06f}, {0.55f, 0.58f, 0.60f}, 90.0f);
        // Barbed wire top line
        drawCylinder({x + step * 0.5f, 2.65f, z}, {step, 0.04f, 0.04f}, {0.75f, 0.75f, 0.78f}, 90.0f);
    }
}

static void drawWatchtower(const vec3& pos) {
    mat4 root = T(pos);
    // 4 Heavy timber/steel corner stilts
    for (float dx : {-2.0f, 2.0f}) {
        for (float dz : {-2.0f, 2.0f}) {
            drawCylinder(root * T({dx, 4.25f, dz}), {0.28f, 8.5f, 0.28f}, {0.35f, 0.30f, 0.25f}, 0.0f, 32.0f);
        }
    }
    // Cross brace beams
    drawCube(root * T({0.0f, 4.25f, -2.0f}), {4.0f, 0.15f, 0.15f}, {0.35f, 0.30f, 0.25f});
    drawCube(root * T({0.0f, 4.25f,  2.0f}), {4.0f, 0.15f, 0.15f}, {0.35f, 0.30f, 0.25f});

    // Elevated Guard Platform (At Y = 8.5f)
    drawCube(root * T({0.0f, 8.5f, 0.0f}), {5.2f, 0.35f, 5.2f}, {0.45f, 0.42f, 0.38f}, 0.0f, 24.0f);
    // Perimeter safety railings
    drawCube(root * T({0.0f, 9.15f, -2.45f}), {5.0f, 1.0f, 0.15f}, {0.75f, 0.75f, 0.8f}, 0.0f, 32.0f);
    drawCube(root * T({0.0f, 9.15f,  2.45f}), {5.0f, 1.0f, 0.15f}, {0.75f, 0.75f, 0.8f}, 0.0f, 32.0f);
    drawCube(root * T({-2.45f, 9.15f, 0.0f}), {0.15f, 1.0f, 5.0f}, {0.75f, 0.75f, 0.8f}, 0.0f, 32.0f);
    drawCube(root * T({ 2.45f, 9.15f, 0.0f}), {0.15f, 1.0f, 5.0f}, {0.75f, 0.75f, 0.8f}, 0.0f, 32.0f);

    // Watchtower Roof Canopy
    drawCube(root * T({0.0f, 11.2f, 0.0f}), {5.6f, 0.25f, 5.6f}, {0.28f, 0.30f, 0.32f}, 0.0f, 32.0f);
    for (float dx : {-2.2f, 2.2f}) {
        for (float dz : {-2.2f, 2.2f}) {
            drawCylinder(root * T({dx, 9.85f, dz}), {0.15f, 2.7f, 0.15f}, {0.7f, 0.7f, 0.7f});
        }
    }

    // Climbable Access Ladder (along the front stilt)
    for (float ly = 0.5f; ly <= 8.5f; ly += 0.5f) {
        drawCylinder(root * T({-2.0f, ly, 2.25f}), {0.7f, 0.06f, 0.06f}, {0.8f, 0.8f, 0.8f}, 90.0f);
    }
}

// 3D Humanoid NPC Port Operator / Security Worker
static void drawHumanNPC(const vec3& pos, float yaw, bool walking = false, float animPhase = 0.0f, const vec3& vestColor = vec3(0.95f, 0.85f, 0.1f)) {
    mat4 root = T(pos) * R(yaw, {0,1,0});

    float armSwing = walking ? 25.0f * std::sin(simTime * 6.0f + animPhase) : 0.0f;
    float legSwing = walking ? 30.0f * std::sin(simTime * 6.0f + animPhase) : 0.0f;

    // Safety Hardhat Helmet
    drawSphere(root * T({0.0f, 1.78f, 0.0f}), 0.26f, vestColor, 64.0f);
    drawCylinder(root * T({0.0f, 1.70f, 0.0f}), {0.58f, 0.06f, 0.58f}, vestColor, 0.0f, 48.0f); // Helmet brim

    // Head / Face
    drawSphere(root * T({0.0f, 1.62f, 0.0f}), 0.20f, {0.88f, 0.72f, 0.60f}, 16.0f);

    // Torso with High-Visibility Safety Jacket & Reflective Bands
    drawCube(root * T({0.0f, 1.25f, 0.0f}), {0.50f, 0.55f, 0.32f}, vestColor, 0.0f, 32.0f);
    drawCube(root * T({0.0f, 1.25f, 0.0f}), {0.52f, 0.12f, 0.33f}, {0.85f, 0.88f, 0.92f}, 0.0f, 96.0f); // Reflective silver tape

    // Pelvis & Belt
    drawCube(root * T({0.0f, 0.95f, 0.0f}), {0.46f, 0.15f, 0.30f}, {0.18f, 0.18f, 0.20f}, 0.0f, 16.0f);

    // Left Leg & Boot
    mat4 lLeg = root * T({-0.14f, 0.85f, 0.0f}) * R(legSwing, {1,0,0}) * T({0.0f, -0.42f, 0.0f});
    drawCylinder(lLeg, {0.16f, 0.75f, 0.16f}, {0.20f, 0.22f, 0.26f}, 0.0f, 16.0f);
    drawCube(lLeg * T({0.0f, -0.40f, 0.06f}), {0.18f, 0.14f, 0.28f}, {0.10f, 0.10f, 0.12f});

    // Right Leg & Boot
    mat4 rLeg = root * T({ 0.14f, 0.85f, 0.0f}) * R(-legSwing, {1,0,0}) * T({0.0f, -0.42f, 0.0f});
    drawCylinder(rLeg, {0.16f, 0.75f, 0.16f}, {0.20f, 0.22f, 0.26f}, 0.0f, 16.0f);
    drawCube(rLeg * T({0.0f, -0.40f, 0.06f}), {0.18f, 0.14f, 0.28f}, {0.10f, 0.10f, 0.12f});

    // Left Arm & Hand
    mat4 lArm = root * T({-0.32f, 1.45f, 0.0f}) * R(-armSwing, {1,0,0}) * T({0.0f, -0.28f, 0.0f});
    drawCylinder(lArm, {0.12f, 0.50f, 0.12f}, vestColor, 0.0f, 32.0f);
    drawSphere(lArm * T({0.0f, -0.28f, 0.0f}), 0.08f, {0.88f, 0.72f, 0.60f});

    // Right Arm & Hand
    mat4 rArm = root * T({ 0.32f, 1.45f, 0.0f}) * R(armSwing, {1,0,0}) * T({0.0f, -0.28f, 0.0f});
    drawCylinder(rArm, {0.12f, 0.50f, 0.12f}, vestColor, 0.0f, 32.0f);
    drawSphere(rArm * T({0.0f, -0.28f, 0.0f}), 0.08f, {0.88f, 0.72f, 0.60f});
}

// Radiant Vertical Waypoint Laser Beacon (Directly inspired by Reference Images 7 & 10!)
static void drawLaserWaypointBeacon(const vec3& pos) {
    float pulse = 0.75f + 0.25f * std::sin(simTime * 5.0f);
    vec3 laserCol = vec3(0.1f, 1.3f * pulse, 0.4f * pulse);

    // Ultra-tall vertical radiant beam
    drawCylinder(pos + vec3(0, 35.0f, 0), {0.35f * pulse, 70.0f, 0.35f * pulse}, laserCol, 0.0f, 128.0f, vec3(1.0f), true);

    // Glowing base rings on ground
    drawMesh(ringMesh, T(pos + vec3(0, 0.12f, 0)) * R(90, {1,0,0}) * S({3.5f * pulse, 1.0f, 3.5f * pulse}), laserCol, 64.0f, vec3(1.0f), false, true);
    drawMesh(ringMesh, T(pos + vec3(0, 0.15f, 0)) * R(90, {1,0,0}) * S({5.5f, 1.0f, 5.5f}), laserCol * 0.6f, 64.0f, vec3(1.0f), false, true);
}

static void drawSurroundingWorld() {
    vec3 waterColor = nightLights ? vec3(0.04f, 0.16f, 0.30f) : vec3(0.10f, 0.44f, 0.70f);
    vec3 waterSpecular = vec3(0.95f, 0.98f, 1.0f);

    // ================= 1. MASSIVE TERRAIN (SEAMLESS CONTINUOUS EXTENSION) =================
    // A. Endless Ocean stretching to the northern horizon (North: Z = -26 to -300, X = -320 to +320)
    drawCube({0.0f, -0.62f, -160.0f}, {640.0f, 0.2f, 270.0f}, waterColor, 0.0f, 128.0f, waterSpecular);

    // B. Vast Inland Hinterland Landscape (South: Z = 23.5 to 260, X = -320 to +320)
    drawCube({0.0f, -0.65f, 140.0f}, {640.0f, 1.1f, 235.0f}, {0.30f, 0.42f, 0.25f}, 0.0f, 8.0f); // Grassy inland plain

    // ================= 2. EXTENDED ROAD NETWORK (STRETCHES HUNDREDS OF METERS IN BOTH DIRECTIONS!) =================
    // A. Main Dock Expressway (Z = 18.5f, spanning from X = -320 to +320, width 8.5m)
    drawCube({0.0f, 0.02f, 18.5f}, {640.0f, 0.06f, 8.5f}, {0.20f, 0.21f, 0.22f}, 0.0f, 16.0f);

    // Concrete curbs & sidewalks along the main avenue edges
    drawCube({0.0f, 0.08f, 14.1f}, {640.0f, 0.16f, 0.6f}, {0.55f, 0.56f, 0.58f}, 0.0f, 24.0f);
    drawCube({0.0f, 0.08f, 22.9f}, {640.0f, 0.16f, 0.6f}, {0.55f, 0.56f, 0.58f}, 0.0f, 24.0f);

    // Yellow dashed lane divider stripes every 6m across the entire 640m stretch!
    for (int x = -315; x <= 315; x += 6) {
        drawCube({static_cast<float>(x), 0.06f, 18.5f}, {3.0f, 0.02f, 0.20f}, {0.95f, 0.88f, 0.25f}, 0.0f, 16.0f);
    }
    // Solid white shoulder markings
    drawCube({0.0f, 0.06f, 15.0f}, {640.0f, 0.02f, 0.16f}, {0.90f, 0.90f, 0.92f});
    drawCube({0.0f, 0.06f, 22.0f}, {640.0f, 0.02f, 0.16f}, {0.90f, 0.90f, 0.92f});

    // Street Lamps along the main expressway every 24m
    for (int x = -288; x <= 288; x += 24) {
        drawStreetLamp({static_cast<float>(x), 0.0f, 23.8f}, 0.0f);
    }

    // Pedestrian Zebra Crosswalks at key intersections
    for (float zw = -2.5f; zw <= 2.5f; zw += 0.8f) {
        drawCube({0.0f, 0.065f, 18.5f + zw}, {0.45f, 0.02f, 0.55f}, {0.95f, 0.95f, 0.95f});
        drawCube({-60.0f, 0.065f, 18.5f + zw}, {0.45f, 0.02f, 0.55f}, {0.95f, 0.95f, 0.95f});
        drawCube({ 60.0f, 0.065f, 18.5f + zw}, {0.45f, 0.02f, 0.55f}, {0.95f, 0.95f, 0.95f});
    }

    // B. Inland Cross Boulevard (Z = 45.0f, spanning X = -200 to +200)
    drawCube({0.0f, 0.02f, 45.0f}, {400.0f, 0.06f, 7.5f}, {0.22f, 0.22f, 0.24f}, 0.0f, 16.0f);
    for (int x = -195; x <= 195; x += 6) {
        drawCube({static_cast<float>(x), 0.06f, 45.0f}, {2.8f, 0.02f, 0.18f}, {0.95f, 0.88f, 0.25f}, 0.0f, 16.0f);
    }

    // C. North-South Connecting Avenues
    drawCube({  0.0f, 0.02f, 32.0f}, {8.5f, 0.06f, 26.0f}, {0.22f, 0.22f, 0.24f}, 0.0f, 16.0f);
    drawCube({-60.0f, 0.02f, 32.0f}, {7.5f, 0.06f, 26.0f}, {0.22f, 0.22f, 0.24f}, 0.0f, 16.0f);
    drawCube({ 60.0f, 0.02f, 32.0f}, {7.5f, 0.06f, 26.0f}, {0.22f, 0.22f, 0.24f}, 0.0f, 16.0f);

    // ================= 3. PORT PERIMETER FENCING & SECURITY WATCHTOWERS (Inspired by FPS Project!) =================
    drawPerimeterFence(-120.0f, 120.0f, 25.2f);
    drawWatchtower({-45.0f, 0.0f, 24.0f});
    drawWatchtower({ 45.0f, 0.0f, 24.0f});

    // Security Gatehouse & Boom Barriers at entrance
    drawCube({0.0f, 1.8f, 25.2f}, {4.0f, 3.6f, 3.0f}, {0.85f, 0.88f, 0.90f}, 0.0f, 32.0f);
    drawCube({0.0f, 2.0f, 23.6f}, {2.5f, 1.4f, 0.2f}, {0.35f, 0.65f, 0.85f}, 0.0f, 96.0f); // Window
    drawCylinder({-4.5f, 1.0f, 25.2f}, {0.14f, 6.0f, 0.14f}, {0.95f, 0.2f, 0.15f}, 90.0f); // Barrier
    drawCylinder({ 4.5f, 1.0f, 25.2f}, {0.14f, 6.0f, 0.14f}, {0.95f, 0.2f, 0.15f}, 90.0f);

    // ================= 4. PORT HEADQUARTERS & BUILDINGS =================
    vec3 hqPos = {0.0f, 0.0f, 56.0f};
    drawCube(hqPos + vec3(0, 13.0f, 0), {22.0f, 26.0f, 15.0f}, {0.35f, 0.38f, 0.42f}, 0.0f, 32.0f);
    // Blue reflective glass curtain facade
    drawCube(hqPos + vec3(0, 13.0f, -7.55f), {19.0f, 22.0f, 0.2f}, {0.22f, 0.52f, 0.82f}, 0.0f, 128.0f, vec3(0.9f));
    drawCube(hqPos + vec3(0, 13.0f,  7.55f), {19.0f, 22.0f, 0.2f}, {0.22f, 0.52f, 0.82f}, 0.0f, 128.0f, vec3(0.9f));
    // Window horizontal mullions
    for (int f = 1; f <= 5; ++f) {
        drawCube(hqPos + vec3(0, f * 4.4f, -7.6f), {19.5f, 0.35f, 0.25f}, {0.20f, 0.22f, 0.25f});
    }
    // Entrance canopy & columns
    drawCube(hqPos + vec3(0, 2.2f, -9.5f), {10.0f, 0.4f, 4.5f}, {0.18f, 0.20f, 0.24f}, 0.0f, 48.0f);
    drawCylinder(hqPos + vec3(-4.0f, 1.1f, -11.0f), {0.3f, 2.2f, 0.3f}, {0.7f, 0.7f, 0.7f});
    drawCylinder(hqPos + vec3( 4.0f, 1.1f, -11.0f), {0.3f, 2.2f, 0.3f}, {0.7f, 0.7f, 0.7f});

    // Rooftop Helipad & Mast
    drawCylinder(hqPos + vec3(0, 26.2f, 0), {8.0f, 0.3f, 8.0f}, {0.25f, 0.28f, 0.30f});
    drawMesh(ringMesh, T(hqPos + vec3(0, 26.4f, 0)) * R(90, {1,0,0}) * S({6.0f, 1.0f, 6.0f}), {0.95f, 0.85f, 0.15f}, 32.0f);
    drawCylinder(hqPos + vec3(0, 31.0f, 0), {0.25f, 9.0f, 0.25f}, {0.8f, 0.2f, 0.15f});
    drawSphere(hqPos + vec3(0, 35.5f, 0), 0.35f, {1.2f, 0.1f, 0.1f}, 64.0f, vec3(1.0f), true);

    // Lateral Wings: Customs & Logistics
    drawCube({-32.0f, 8.0f, 52.0f}, {22.0f, 16.0f, 14.0f}, {0.45f, 0.48f, 0.50f}, 0.0f, 32.0f);
    drawCube({ 32.0f, 9.0f, 52.0f}, {22.0f, 18.0f, 14.0f}, {0.42f, 0.45f, 0.48f}, 0.0f, 32.0f);

    // Large Distribution Warehouses along the highway
    for (float wx : {-140.0f, -90.0f, 90.0f, 140.0f}) {
        drawCube({wx, 5.0f, 52.0f}, {36.0f, 10.0f, 18.0f}, {0.36f, 0.40f, 0.44f}, 0.0f, 24.0f);
        drawCube({wx, 10.2f, 52.0f}, {37.0f, 0.5f, 19.0f}, {0.20f, 0.22f, 0.25f}, 0.0f, 32.0f);
    }

    // Parking Bays & Cars
    drawParkedCar({-22.0f, 0.0f, 36.0f}, 0.0f, {0.85f, 0.18f, 0.15f});
    drawParkedCar({-16.0f, 0.0f, 36.0f}, 0.0f, {0.18f, 0.35f, 0.75f});
    drawParkedCar({-10.0f, 0.0f, 36.0f}, 0.0f, {0.90f, 0.92f, 0.94f});
    drawParkedCar({ 10.0f, 0.0f, 36.0f}, 0.0f, {0.14f, 0.55f, 0.30f});
    drawParkedCar({ 16.0f, 0.0f, 36.0f}, 0.0f, {0.88f, 0.55f, 0.12f});
    drawParkedCar({ 22.0f, 0.0f, 36.0f}, 0.0f, {0.20f, 0.22f, 0.25f});

    // ================= 5. NATURAL TREES (STRICTLY ON INLAND SOIL, ZERO IN WATER!) =================
    // Boulevard Trees along inland avenues (Z >= 28.0)
    for (int x = -160; x <= 160; x += 14) {
        if (std::abs(x) < 8) continue;
        drawTree({static_cast<float>(x), 0.0f, 28.5f}, 1.15f, x);
        drawTree({static_cast<float>(x), 0.0f, 40.5f}, 1.10f, x + 1);
    }
    // Park plazas behind Headquarters
    for (int x = -28; x <= 28; x += 7) {
        if (std::abs(x) < 4) continue;
        drawTree({static_cast<float>(x), 0.0f, 68.0f}, 1.35f, x);
        drawTree({static_cast<float>(x), 0.0f, 76.0f}, 1.40f, x + 2);
    }
    // Distant forestry inland (Z = 85 to 130)
    for (int x = -200; x <= 200; x += 22) {
        drawTree({static_cast<float>(x), 0.0f, 92.0f}, 1.5f, x);
        drawTree({static_cast<float>(x) + 8.0f, 0.0f, 115.0f}, 1.6f, x + 3);
    }
}

// ---------------- 4. PORT QUAYS, DOCK PLATFORMS & SEAWALL ----------------
static void drawQuays() {
    // ---------- SOUTH QUAY (CONTAINER TERMINAL) ----------
    // Main concrete dock slab (from Z = 5.4 to Z = 23.4, extending from X = -320 to +320)
    drawCube({0.0f, -0.6f, 14.4f}, {640.0f, 1.2f, 18.0f}, {0.45f, 0.47f, 0.48f}, 0.0f, 24.0f);

    // Continuous Vertical Quay Bulkhead Seawall facing water (Z = 5.35, Y from 0 to -2.8)
    drawCube({0.0f, -1.4f, 5.35f}, {640.0f, 2.8f, 0.4f}, {0.32f, 0.34f, 0.36f}, 0.0f, 16.0f);

    // Quay Edge Safety Hazard Stripes (alternating yellow/black blocks along Z = 5.6)
    for (int x = -120; x <= 120; x += 2) {
        bool yellow = ((x / 2) % 2 == 0);
        vec3 stripeColor = yellow ? vec3(0.95f, 0.80f, 0.12f) : vec3(0.14f, 0.14f, 0.16f);
        drawCube({static_cast<float>(x), 0.05f, 5.65f}, {1.95f, 0.12f, 0.45f}, stripeColor, 0.0f, 32.0f);
    }

    // Heavy Cast-Iron Mooring Bollards along dock edge
    for (int x = -100; x <= 100; x += 10) {
        drawCylinder({static_cast<float>(x), 0.28f, 5.95f}, {0.35f, 0.55f, 0.35f}, {0.18f, 0.18f, 0.20f}, 0.0f, 64.0f);
        drawSphere({static_cast<float>(x), 0.58f, 5.95f}, 0.26f, {0.18f, 0.18f, 0.20f}, 64.0f);
    }

    // Steel Gantry Rails for the Container Crane (parallel tracks at Z = 6.9 and Z = 9.8)
    drawCube({0.0f, 0.04f, 6.9f}, {640.0f, 0.08f, 0.18f}, {0.70f, 0.72f, 0.75f}, 0.0f, 96.0f);
    drawCube({0.0f, 0.04f, 9.8f}, {640.0f, 0.08f, 0.18f}, {0.70f, 0.72f, 0.75f}, 0.0f, 96.0f);

    // Quay Swim Ladders (Yellow steel ladders along the seawall so swimmer can climb out)
    for (float lx : {-20.0f, 0.0f, 20.0f, 40.0f}) {
        for (float ly = -2.2f; ly <= 0.2f; ly += 0.4f) {
            drawCylinder({lx, ly, 5.15f}, {0.6f, 0.05f, 0.05f}, {0.95f, 0.85f, 0.1f}, 90.0f);
        }
    }

    // ---------- NORTH SHORE (INDUSTRIAL WATERFRONT & LIGHTHOUSE) ----------
    // North solid quay embankment (Z = -5.4 to -26.0, X = -320 to +320)
    drawCube({0.0f, -0.6f, -15.7f}, {640.0f, 1.2f, 20.6f}, {0.38f, 0.44f, 0.36f}, 0.0f, 16.0f);
    drawCube({0.0f, -1.4f, -5.35f}, {640.0f, 2.8f, 0.4f}, {0.32f, 0.34f, 0.36f}, 0.0f, 16.0f); // North seawall

    // North concrete quay apron
    drawCube({0.0f, 0.02f, -7.0f}, {640.0f, 0.06f, 3.0f}, {0.52f, 0.54f, 0.56f}, 0.0f, 16.0f);

    // Industrial Port Warehouse
    drawCube({-16.0f, 2.5f, -12.5f}, {16.0f, 5.0f, 7.0f}, {0.38f, 0.42f, 0.46f}, 0.0f, 24.0f);
    drawCube({-16.0f, 5.15f, -12.5f}, {16.8f, 0.4f, 7.6f}, {0.18f, 0.22f, 0.25f}, 0.0f, 32.0f);
    for (int gx = -20; gx <= -12; gx += 4) {
        drawCube({static_cast<float>(gx), 1.6f, -8.9f}, {2.6f, 3.2f, 0.2f}, {0.15f, 0.18f, 0.22f});
    }

    // Fuel / Oil Storage Tanks
    for (int ox = 8; ox <= 16; ox += 8) {
        drawCylinder({static_cast<float>(ox), 2.8f, -12.5f}, {5.0f, 5.6f, 5.0f}, {0.82f, 0.84f, 0.86f}, 0.0f, 64.0f);
        drawCylinder({static_cast<float>(ox), 5.65f, -12.5f}, {4.6f, 0.3f, 4.6f}, {0.25f, 0.28f, 0.32f}, 0.0f, 32.0f);
    }

    // Coastal Radar Tower (Lattice tower with spinning dish)
    drawCylinder({-2.0f, 4.5f, -14.0f}, {0.5f, 9.0f, 0.5f}, {0.70f, 0.72f, 0.75f}, 0.0f, 64.0f);
    drawSphere({-2.0f, 9.2f, -14.0f}, 0.45f, {0.85f, 0.2f, 0.15f});
    mat4 radarDish = T({-2.0f, 9.4f, -14.0f}) * R(simTime * 90.0f, {0,1,0});
    drawMesh(cylinderMesh, radarDish * R(90, {0,0,1}) * S({0.2f, 2.2f, 0.6f}), {0.95f, 0.95f, 0.95f}, 64.0f);
}

// ---------------- 5. COASTAL LIGHTHOUSE WITH ROTATING BEACON ----------------
static void drawLighthouse() {
    vec3 lhPos = vec3(26.0f, 0.0f, -9.5f);

    // Stone foundation
    drawCylinder(lhPos + vec3(0, 0.75f, 0), {4.2f, 1.5f, 4.2f}, {0.38f, 0.40f, 0.42f}, 0.0f, 16.0f);

    // Tapered tower sections (Classic Red & White Maritime Stripes)
    drawCylinder(lhPos + vec3(0, 2.5f, 0), {3.2f, 2.0f, 3.2f}, {0.85f, 0.18f, 0.15f}, 0.0f, 48.0f);
    drawCylinder(lhPos + vec3(0, 4.5f, 0), {2.8f, 2.0f, 2.8f}, {0.92f, 0.92f, 0.95f}, 0.0f, 48.0f);
    drawCylinder(lhPos + vec3(0, 6.5f, 0), {2.4f, 2.0f, 2.4f}, {0.85f, 0.18f, 0.15f}, 0.0f, 48.0f);
    drawCylinder(lhPos + vec3(0, 8.5f, 0), {2.0f, 2.0f, 2.0f}, {0.92f, 0.92f, 0.95f}, 0.0f, 48.0f);

    // Observation Gallery & Railing
    drawCylinder(lhPos + vec3(0, 9.6f, 0), {2.6f, 0.3f, 2.6f}, {0.22f, 0.22f, 0.25f}, 0.0f, 32.0f);

    // Lantern Room (Glass & glowing core)
    drawCylinder(lhPos + vec3(0, 10.6f, 0), {1.7f, 1.7f, 1.7f}, {0.18f, 0.20f, 0.24f}, 0.0f, 96.0f);
    drawSphere(lhPos + vec3(0, 10.6f, 0), 0.65f, vec3(1.2f, 1.1f, 0.6f), 128.0f, vec3(1.0f), true);

    // Dome Cap
    drawSphere(lhPos + vec3(0, 11.5f, 0), 1.0f, {0.85f, 0.18f, 0.15f}, 64.0f);

    // Rotating Golden Beacon Beam
    mat4 beaconMat = T(lhPos + vec3(0, 10.6f, 0)) * R(lighthouseAngle, {0,1,0});
    vec3 beamCol = nightLights ? vec3(1.0f, 0.95f, 0.5f) : vec3(0.9f, 0.85f, 0.6f);
    drawMesh(cylinderMesh, beaconMat * T({6.0f, 0, 0}) * R(90, {0,0,1}) * S({0.8f, 12.0f, 0.8f}), beamCol, 64.0f, vec3(1.0f), false, true);
}

// ---------------- 6. INTERACTIVE BASCULE DRAWBRIDGE ----------------
static void drawBridge() {
    const vec3 bridgeDeckCol{0.22f, 0.24f, 0.27f};
    const vec3 hingeCol{0.15f, 0.16f, 0.18f};

    // Concrete Bridge Abutments on South and North banks
    drawCube({0.0f, 0.4f, 5.6f}, {5.8f, 1.8f, 2.4f}, {0.48f, 0.50f, 0.52f}, 0.0f, 24.0f);
    drawCube({0.0f, 0.4f, -5.6f}, {5.8f, 1.8f, 2.4f}, {0.48f, 0.50f, 0.52f}, 0.0f, 24.0f);

    // Bridge hinge pivot points
    vec3 southHinge = {0.0f, 1.2f, 5.2f};
    vec3 northHinge = {0.0f, 1.2f, -5.2f};

    drawCylinder(southHinge, {4.6f, 0.5f, 0.5f}, hingeCol, 90.0f);
    drawCylinder(northHinge, {4.6f, 0.5f, 0.5f}, hingeCol, 90.0f);

    const float deckLength = 5.2f;

    // South Bridge Leaf (Pivots upward around southHinge X-axis)
    mat4 southLeaf = T(southHinge) * R(-bridgeAngle, {1,0,0}) * T({0.0f, 0.18f, -deckLength * 0.5f}) * S({4.4f, 0.36f, deckLength});
    drawMesh(cubeMesh, southLeaf, bridgeDeckCol, 32.0f);

    // North Bridge Leaf (Pivots upward around northHinge X-axis)
    mat4 northLeaf = T(northHinge) * R(bridgeAngle, {1,0,0}) * T({0.0f, 0.18f, deckLength * 0.5f}) * S({4.4f, 0.36f, deckLength});
    drawMesh(cubeMesh, northLeaf, bridgeDeckCol, 32.0f);

    // Safety Railings on Bridge
    mat4 sRailL = T(southHinge) * R(-bridgeAngle, {1,0,0}) * T({-2.1f, 0.55f, -deckLength * 0.5f}) * S({0.15f, 0.45f, deckLength});
    mat4 sRailR = T(southHinge) * R(-bridgeAngle, {1,0,0}) * T({ 2.1f, 0.55f, -deckLength * 0.5f}) * S({0.15f, 0.45f, deckLength});
    drawMesh(cubeMesh, sRailL, {0.9f, 0.45f, 0.1f}, 48.0f);
    drawMesh(cubeMesh, sRailR, {0.9f, 0.45f, 0.1f}, 48.0f);

    // Flashing Bridge Warning Lights
    bool flashRed = (std::sin(simTime * 6.0f) > 0.0f) && (bridgeAngle > 1.0f);
    vec3 lightCol = flashRed ? vec3(1.2f, 0.1f, 0.1f) : vec3(0.2f, 0.6f, 0.2f);
    drawSphere({-2.5f, 1.8f, 5.2f}, 0.25f, lightCol, 32.0f, vec3(1.0f), true);
    drawSphere({ 2.5f, 1.8f, 5.2f}, 0.25f, lightCol, 32.0f, vec3(1.0f), true);
    drawSphere({-2.5f, 1.8f, -5.2f}, 0.25f, lightCol, 32.0f, vec3(1.0f), true);
    drawSphere({ 2.5f, 1.8f, -5.2f}, 0.25f, lightCol, 32.0f, vec3(1.0f), true);
}

// ---------------- 7. CONTAINER STACKS & YARD ----------------
static void drawContainerYard() {
    float startX = -24.0f;
    int idx = 0;
    for (int col = 0; col < 8; ++col) {
        for (int row = 0; row < 2; ++row) {
            float x = startX + col * 4.2f;
            float z = 12.0f + row * 2.8f;
            int height = ((col + row) % 3) + 1;
            for (int h = 0; h < height; ++h) {
                vec3 cColor = COLORS[(idx++) % 6];
                drawCube({x, 0.65f + h * 1.25f, z}, {3.6f, 1.2f, 2.2f}, cColor, 0.0f, 48.0f);
                drawCube({x, 0.65f + h * 1.25f, z}, {3.64f, 1.22f, 0.1f}, {0.15f, 0.15f, 0.16f});
            }
        }
    }

    // Target Deposit Area for the Crane (High-Visibility Marked Dock Unloading Bay)
    drawCube({8.0f, 0.05f, 13.5f}, {6.8f, 0.05f, 5.2f}, {0.95f, 0.85f, 0.12f}, 0.0f, 32.0f); // Bright yellow safety perimeter
    drawCube({8.0f, 0.06f, 13.5f}, {6.4f, 0.06f, 4.8f}, {0.14f, 0.16f, 0.18f}, 0.0f, 16.0f); // Asphalt bay floor

    // Diagonal hazard warning stripes on bay floor
    for (float hz = 11.5f; hz <= 15.5f; hz += 0.85f) {
        drawCube({8.0f, 0.07f, hz}, {6.0f, 0.02f, 0.26f}, {0.95f, 0.85f, 0.1f}, 25.0f, 32.0f);
    }

    // 4 Corner Tall Warning Towers with pulsing amber & green strobes
    float strobeGlow = 0.5f + 0.5f * std::sin(simTime * 6.0f);
    vec3 strobeCol = vec3(1.3f, 0.85f, 0.1f) * strobeGlow;
    for (float px : {4.8f, 11.2f}) {
        for (float pz : {11.0f, 16.0f}) {
            drawCylinder({px, 2.25f, pz}, {0.25f, 4.5f, 0.25f}, {0.95f, 0.75f, 0.1f});
            drawSphere({px, 4.6f, pz}, 0.26f, strobeCol, 64.0f, vec3(1.0f), true);
        }
    }

    // Sky-Piercing Radiant Green Landing Laser Beacon & Hologram (visible from anywhere!)
    float pulse = 0.65f + 0.35f * std::sin(simTime * 6.0f);
    vec3 beaconCol = crane.carrying ? (vec3(0.2f, 1.4f, 0.45f) * pulse) : (vec3(0.2f, 0.9f, 0.4f) * 0.45f);
    // Vertical laser beam shooting 22 meters straight into the sky
    drawCylinder({8.0f, 11.0f, 13.5f}, {0.35f, 22.0f, 0.35f}, beaconCol, 0.0f, 64.0f, vec3(1.0f), true);
    drawSphere({8.0f, 1.2f, 13.5f}, 0.55f, beaconCol, 64.0f, vec3(1.0f), true);

    // Glowing Holographic Landing Zone Pad when carrying cargo
    if (crane.carrying) {
        drawCube({8.0f, 0.10f, 13.5f}, {5.2f, 0.03f, 3.8f}, beaconCol, 0.0f, 64.0f, vec3(1.0f), true);
        drawCube({8.0f, 0.11f, 13.5f}, {5.6f, 0.02f, 0.4f}, vec3(1.4f, 1.2f, 0.2f) * pulse, 0.0f, 64.0f, vec3(1.0f), true);
        drawCube({8.0f, 0.11f, 13.5f}, {0.4f, 0.02f, 4.2f}, vec3(1.4f, 1.2f, 0.2f) * pulse, 0.0f, 64.0f, vec3(1.0f), true);

        // Dynamic ground landing shadow & crosshair indicator directly underneath the crane spreader
        bool inBayZone = (crane.trolleyZ >= 11.5f && crane.trolleyZ <= 15.0f);
        float pGlow = 0.7f + 0.3f * std::sin(simTime * 8.0f);
        vec3 dropCol = inBayZone ? (vec3(0.2f, 1.5f, 0.4f) * pGlow) : vec3(1.3f, 0.8f, 0.1f);
        // Ground crosshair beneath trolley:
        drawCube({8.0f, 0.12f, crane.trolleyZ}, {3.6f, 0.02f, 2.2f}, dropCol, 0.0f, 64.0f, vec3(1.0f), true);
        drawCube({8.0f, 0.13f, crane.trolleyZ}, {4.4f, 0.015f, 0.15f}, dropCol, 0.0f, 64.0f, vec3(1.0f), true);
        drawCube({8.0f, 0.13f, crane.trolleyZ}, {0.15f, 0.015f, 3.0f}, dropCol, 0.0f, 64.0f, vec3(1.0f), true);
    }

    // Dynamic Unloading Bay Markings & Cargo Transfer Interface
    drawCube({8.0f, 0.075f, 13.5f}, {5.6f, 0.015f, 2.5f}, {0.20f, 0.22f, 0.25f}, 0.0f, 32.0f);
}

// ---------------- 8. PLAYABLE HIGH-TECH SHIP-TO-SHORE CRANE ----------------
static void drawCrane() {
    const float craneBaseX = 8.0f;
    const float craneBaseZ = 8.35f;
    mat4 gantryBase = T({craneBaseX, 0.0f, craneBaseZ});

    // 1. Elevated Heavy-Duty Gantry Portal Legs (spanning across dock, elevated to 10.5m height)
    drawCylinder(gantryBase * T({-2.4f, 5.25f, -1.8f}), {0.6f, 10.5f, 0.6f}, {0.18f, 0.20f, 0.22f}, 0.0f, 48.0f);
    drawCylinder(gantryBase * T({ 2.4f, 5.25f, -1.8f}), {0.6f, 10.5f, 0.6f}, {0.18f, 0.20f, 0.22f}, 0.0f, 48.0f);
    drawCylinder(gantryBase * T({-2.4f, 5.25f,  1.8f}), {0.6f, 10.5f, 0.6f}, {0.18f, 0.20f, 0.22f}, 0.0f, 48.0f);
    drawCylinder(gantryBase * T({ 2.4f, 5.25f,  1.8f}), {0.6f, 10.5f, 0.6f}, {0.18f, 0.20f, 0.22f}, 0.0f, 48.0f);

    // Gantry Ladder for the player to climb into the elevated platform
    for (float ly = 0.5f; ly <= 10.5f; ly += 0.45f) {
        drawCylinder(gantryBase * T({-2.4f, ly, 2.1f}), {0.6f, 0.05f, 0.05f}, {0.95f, 0.85f, 0.1f}, 90.0f);
    }

    // Gantry Upper Platform Beam
    drawCube(gantryBase * T({0.0f, 10.5f, 0.0f}), {5.8f, 0.6f, 4.0f}, {0.85f, 0.55f, 0.10f}, 0.0f, 48.0f);

    // 2. Turret / Cab / Machinery House (Elevated Turret Axis at Y = 11.2m)
    mat4 turret = gantryBase * T({0.0f, 11.2f, 0.0f}) * R(crane.slewingYaw, {0,1,0});
    // Only draw turret slewing ring when outside to keep operator interior line of sight 100% unobstructed!
    if (!player.inCraneCab) {
        drawCylinder(turret * T({0.0f, 0.35f, 0.0f}), {1.8f, 0.7f, 1.8f}, {0.20f, 0.22f, 0.24f}, 0.0f, 64.0f);
    }

    // 2b. High-Tech 360-Degree Panoramic Transparent Glass Operator Room (Suspended at 12.2m height)
    mat4 cabRoot = turret * T({0.0f, 0.85f, -1.8f});

    // Sleek Corner Structural Canopy Struts (Ultra-slim 0.025m struts for full 360 panoramic visibility)
    for (float cx : {-0.65f, 0.65f}) {
        for (float cz : {-0.65f, 0.65f}) {
            drawCube(cabRoot * T({cx, 0.0f, cz}), {0.025f, 1.15f, 0.025f}, {0.12f, 0.12f, 0.14f});
        }
    }

    // Top canopy frame and floor perimeter frame
    drawCube(cabRoot * T({0.0f,  0.58f, 0.0f}), {1.35f, 0.025f, 1.35f}, {0.18f, 0.20f, 0.22f});
    drawCube(cabRoot * T({0.0f, -0.58f, 0.0f}), {1.35f, 0.025f, 1.35f}, {0.18f, 0.20f, 0.22f});

    // 4 Transparent Glass Walls (Front, Back, Left, Right) + Glass Floor + Glass Ceiling
    if (!player.inCraneCab) {
        // Front Glass Windshield (Facing ship)
        drawCube(cabRoot * T({0.0f, 0.0f, -0.65f}), {1.25f, 1.1f, 0.015f}, {0.45f, 0.75f, 0.95f}, 0.0f, 128.0f);
        // Rear Glass Window (Facing dock unloading bay - 100% transparent view!)
        drawCube(cabRoot * T({0.0f, 0.0f,  0.65f}), {1.25f, 1.1f, 0.015f}, {0.45f, 0.75f, 0.95f}, 0.0f, 128.0f);
        // Left & Right Glass Windows (Facing harbor & open water)
        drawCube(cabRoot * T({-0.65f, 0.0f, 0.0f}), {0.015f, 1.1f, 1.25f}, {0.45f, 0.75f, 0.95f}, 0.0f, 128.0f);
        drawCube(cabRoot * T({ 0.65f, 0.0f, 0.0f}), {0.015f, 1.1f, 1.25f}, {0.45f, 0.75f, 0.95f}, 0.0f, 128.0f);
        // Glass Floor (Direct downward viewing of dock & tracks!)
        drawCube(cabRoot * T({0.0f, -0.58f, 0.0f}), {1.25f, 0.015f, 1.25f}, {0.45f, 0.75f, 0.95f}, 0.0f, 128.0f);
        // Glass Ceiling (Skylight)
        drawCube(cabRoot * T({0.0f,  0.58f, 0.0f}), {1.25f, 0.015f, 1.25f}, {0.45f, 0.75f, 0.95f}, 0.0f, 128.0f);
    }

    // Cockpit Ergonomic Pilot Seat (Low profile cushion, strictly below eye level at Y = -0.45f so looking back has ZERO clipping!)
    drawCube(cabRoot * T({0.0f, -0.45f, 0.22f}), {0.52f, 0.10f, 0.52f}, {0.14f, 0.14f, 0.16f});

    // Cockpit Dash & Instrument Consoles (Positioned comfortably low in bottom FOV)
    drawCube(cabRoot * T({0.0f, -0.42f, -0.38f}), {0.85f, 0.16f, 0.32f}, {0.14f, 0.16f, 0.18f});
    // Left telemetry monitor (bright green glow)
    drawCube(cabRoot * T({-0.22f, -0.27f, -0.36f}), {0.22f, 0.11f, 0.02f}, {0.15f, 0.95f, 0.35f}, 0.0f, 64.0f, vec3(1.0f), true);
    // Right status radar monitor (bright cyan glow)
    drawCube(cabRoot * T({ 0.22f, -0.27f, -0.36f}), {0.22f, 0.11f, 0.02f}, {0.15f, 0.65f, 1.3f}, 0.0f, 64.0f, vec3(1.0f), true);
    // Twin Flight-Grade Control Joysticks
    drawCylinder(cabRoot * T({-0.28f, -0.28f, -0.26f}), {0.02f, 0.14f, 0.02f}, {0.85f, 0.85f, 0.1f});
    drawSphere(cabRoot * T({-0.28f, -0.19f, -0.26f}), 0.028f, {0.95f, 0.15f, 0.15f});
    drawCylinder(cabRoot * T({ 0.28f, -0.28f, -0.26f}), {0.02f, 0.14f, 0.02f}, {0.85f, 0.85f, 0.1f});
    drawSphere(cabRoot * T({ 0.28f, -0.19f, -0.26f}), 0.028f, {0.15f, 0.95f, 0.25f});

    // 3. Elevated Long Horizontal Boom Arm (World Height = 14.4m, Length = 22.0m)
    mat4 boom = turret * T({0.0f, 3.2f, 0.0f});
    drawCube(boom * T({0.0f, 0.0f, 0.0f}), {1.2f, 1.2f, 22.0f}, {0.85f, 0.52f, 0.08f}, 0.0f, 48.0f);
    drawCube(boom * T({0.0f, 1.4f, -1.5f}), {0.2f, 2.5f, 0.2f}, {0.3f, 0.3f, 0.35f});

    drawCylinder(boom * T({0.0f, 1.2f, -6.5f}), {0.06f, 10.0f, 0.06f}, {0.2f, 0.2f, 0.2f}, 45.0f);
    drawCylinder(boom * T({0.0f, 1.2f,  6.5f}), {0.06f, 10.0f, 0.06f}, {0.2f, 0.2f, 0.2f}, -45.0f);

    // Realistic STS Winch House & Counterweight on top of rear boom:
    // Placed at Y = 15.6m, Z = 7.5m (above the boom on counter-jib, NEVER blocking operator's rear view!)
    if (!player.inCraneCab) {
        drawCube(boom * T({0.0f, 1.2f, 7.5f}), {2.4f, 1.4f, 3.2f}, {0.22f, 0.24f, 0.28f}, 0.0f, 48.0f);
        drawCube(boom * T({0.0f, 2.05f, 7.5f}), {0.8f, 0.35f, 1.2f}, {0.85f, 0.55f, 0.10f});
        // Heavy counterweights at rear tip
        drawCube(boom * T({0.0f, 0.2f, 9.8f}), {2.6f, 1.0f, 1.4f}, {0.18f, 0.18f, 0.20f}, 0.0f, 32.0f);
    }

    // 4. Trolley (Slides along the elevated boom)
    float trolleyRelZ = crane.trolleyZ - craneBaseZ;
    mat4 trolleyMat = boom * T({0.0f, -0.65f, trolleyRelZ});
    drawCube(trolleyMat, {1.4f, 0.5f, 1.2f}, {0.22f, 0.24f, 0.26f}, 0.0f, 48.0f);

    // 5. Hoist Cables (Hang from elevated boom at Y = 13.75m down to crane.hookY)
    float boomWorldY = 11.2f + 3.2f - 0.65f;
    float cableLength = std::max(0.4f, boomWorldY - crane.hookY);
    mat4 cableMat1 = trolleyMat * T({-0.4f, -cableLength * 0.5f, 0.0f}) * S({0.05f, cableLength, 0.05f});
    mat4 cableMat2 = trolleyMat * T({ 0.4f, -cableLength * 0.5f, 0.0f}) * S({0.05f, cableLength, 0.05f});
    drawMesh(cylinderMesh, cableMat1, {0.12f, 0.12f, 0.14f}, 64.0f);
    drawMesh(cylinderMesh, cableMat2, {0.12f, 0.12f, 0.14f}, 64.0f);

    // 6. Spreader
    mat4 spreaderMat = trolleyMat * T({0.0f, -cableLength, 0.0f});
    drawCube(spreaderMat, {3.4f, 0.28f, 1.8f}, {0.85f, 0.75f, 0.15f}, 0.0f, 64.0f);
    drawCube(spreaderMat * T({0.0f, 0.25f, 0.0f}), {1.0f, 0.35f, 0.8f}, {0.2f, 0.2f, 0.25f});

    // 7. Carried Container on Spreader
    if (crane.carrying) {
        mat4 cargoMat = spreaderMat * T({0.0f, -0.75f, 0.0f}) * S({3.6f, 1.2f, 2.2f});
        vec3 cColor = shipCargos[crane.cargoIndex % shipCargos.size()].color;
        drawMesh(cubeMesh, cargoMat, cColor, 48.0f);
    }
}

// ---------------- 9. SHIPS & MARITIME TRAFFIC ----------------
static void drawDockedContainerShip() {
    mat4 shipRoot = T({8.0f, -0.25f + 0.03f * std::sin(simTime * 2.0f), 0.5f});

    // Commercial Hull
    drawCube(shipRoot * T({0.0f, 0.4f, 0.0f}), {16.0f, 1.6f, 4.4f}, {0.14f, 0.16f, 0.18f}, 0.0f, 48.0f);
    drawCube(shipRoot * T({0.0f, -0.3f, 0.0f}), {16.1f, 0.6f, 4.42f}, {0.75f, 0.18f, 0.14f}, 0.0f, 32.0f);
    drawCylinder(shipRoot * T({8.0f, 0.4f, 0.0f}) * R(90, {0,0,1}), {2.2f, 2.5f, 4.2f}, {0.14f, 0.16f, 0.18f}, 0.0f, 48.0f);

    // Deck Container Stacks rendered from dynamic shipCargos array with active target highlight!
    float pulse = 0.65f + 0.35f * std::sin(simTime * 6.0f);
    for (size_t i = 0; i < shipCargos.size(); ++i) {
        if (!shipCargos[i].unloaded) {
            // If currently carried on crane spreader, do not draw on ship deck
            if (crane.carrying && crane.cargoIndex == static_cast<int>(i)) continue;

            vec3 cPos = shipCargos[i].localPos;
            drawCube(shipRoot * T(cPos), {3.6f, 1.2f, 2.2f}, shipCargos[i].color, 0.0f, 48.0f);
            drawCube(shipRoot * T(cPos), {3.64f, 1.22f, 0.1f}, {0.15f, 0.15f, 0.16f});

            // If this container is the ACTIVE TARGET selected for unloading:
            if (!crane.carrying && static_cast<int>(i) == activeTargetCargoIndex) {
                bool aligned = (crane.trolleyZ <= 2.2f && crane.hookY <= 5.2f);
                vec3 targetHighlightCol = aligned ? (vec3(0.2f, 1.4f, 0.4f) * pulse) : (vec3(1.3f, 1.0f, 0.1f) * pulse);

                // Pulsing target bounding box
                drawCube(shipRoot * T(cPos), {3.8f, 1.35f, 2.4f}, targetHighlightCol, 0.0f, 64.0f, vec3(1.0f), true);

                // Sky-piercing vertical target laser beacon
                drawCylinder(shipRoot * T({cPos.x, cPos.y + 7.0f, cPos.z}), {0.18f, 14.0f, 0.18f}, targetHighlightCol, 0.0f, 64.0f, vec3(1.0f), true);
                drawSphere(shipRoot * T({cPos.x, cPos.y + 1.2f, cPos.z}), 0.38f, targetHighlightCol, 64.0f, vec3(1.0f), true);
            }
        }
    }

    // Ship Bridge Castle & Navigation Mast
    drawCube(shipRoot * T({-7.0f, 2.2f, 0.0f}), {2.4f, 2.2f, 3.8f}, {0.90f, 0.92f, 0.94f}, 0.0f, 32.0f);
    drawCube(shipRoot * T({-7.0f, 3.1f, 0.0f}), {2.5f, 0.4f, 4.0f}, {0.20f, 0.45f, 0.65f}, 0.0f, 96.0f);
    drawCylinder(shipRoot * T({-7.2f, 3.8f, 0.0f}), {0.6f, 1.4f, 0.6f}, {0.85f, 0.22f, 0.15f}, 0.0f, 32.0f);

    // Navigation Lights
    drawSphere(shipRoot * T({-6.0f, 3.4f,  1.9f}), 0.18f, {0.1f, 1.2f, 0.2f}, 64.0f, vec3(1.0f), true);
    drawSphere(shipRoot * T({-6.0f, 3.4f, -1.9f}), 0.18f, {1.2f, 0.1f, 0.1f}, 64.0f, vec3(1.0f), true);
}

static void drawMovingShip(const Ship& s) {
    float bob = 0.08f * std::sin(simTime * 2.2f + s.phase + s.x * 0.15f);
    float roll = 1.6f * std::sin(simTime * 1.8f + s.phase + s.x * 0.10f);
    float pitch = 0.9f * std::cos(simTime * 1.5f + s.phase + s.x * 0.12f);

    mat4 root = T({s.x, -0.25f + bob, s.z}) 
              * R(s.heading, {0, 1, 0}) 
              * R(pitch, {0, 0, 1}) 
              * R(roll, {1, 0, 0}) 
              * S(vec3(s.scale));

    // 1. Dynamic White Frothy Bow Waves & Water Wake
    float foamPulse = 0.85f + 0.15f * std::sin(simTime * 8.0f + s.phase);
    vec3 foamColor = vec3(0.95f, 0.98f, 1.0f) * foamPulse;
    drawCube(root * T({4.8f, 0.06f,  1.6f}), {2.0f, 0.08f, 0.45f}, foamColor,  25.0f, 64.0f, vec3(1.0f), false);
    drawCube(root * T({4.8f, 0.06f, -1.6f}), {2.0f, 0.08f, 0.45f}, foamColor, -25.0f, 64.0f, vec3(1.0f), false);
    drawCube(root * T({-5.2f, 0.04f, 0.0f}), {4.0f, 0.06f, 2.2f}, foamColor * 0.85f, 0.0f, 32.0f, vec3(1.0f), false);

    switch (s.type) {
        case ShipType::Tugboat: {
            // Robust Harbor / Ocean Tugboat
            // Red Anti-fouling bottom
            drawCube(root * T({0.0f, 0.22f, 0.0f}), {6.4f, 0.55f, 2.8f}, {0.65f, 0.18f, 0.15f}, 0.0f, 32.0f);
            // Main Tug Hull (Vibrant safety red / orange)
            drawCube(root * T({0.0f, 0.70f, 0.0f}), {6.5f, 0.55f, 2.85f}, s.color, 0.0f, 48.0f);
            drawCylinder(root * T({3.25f, 0.70f, 0.0f}) * R(90, {0,0,1}), {0.55f, 1.6f, 2.7f}, s.color, 0.0f, 48.0f);
            // Heavy rubber push fender on bow
            drawCylinder(root * T({3.85f, 0.75f, 0.0f}) * R(90, {0,0,1}), {0.60f, 1.2f, 2.5f}, {0.12f, 0.12f, 0.14f}, 0.0f, 32.0f);

            // Wheelhouse Bridge with panoramic windows
            drawCube(root * T({0.2f, 1.60f, 0.0f}), {2.4f, 1.15f, 2.2f}, {0.95f, 0.95f, 0.95f}, 0.0f, 32.0f);
            drawCube(root * T({0.95f, 1.70f, 0.0f}), {0.1f, 0.42f, 1.9f}, {0.25f, 0.65f, 0.85f}, 0.0f, 96.0f); // Window

            // Twin exhaust stacks & aft towing gear
            drawCylinder(root * T({-0.8f, 2.15f,  0.55f}), {0.32f, 0.9f, 0.32f}, {0.18f, 0.18f, 0.20f});
            drawCylinder(root * T({-0.8f, 2.15f, -0.55f}), {0.32f, 0.9f, 0.32f}, {0.18f, 0.18f, 0.20f});
            drawCylinder(root * T({-1.8f, 1.15f, 0.0f}), {0.7f, 0.5f, 0.7f}, {0.35f, 0.38f, 0.42f}); // Tow winch

            // Radar Mast & Navigation Lights
            drawCylinder(root * T({0.2f, 2.55f, 0.0f}), {0.08f, 1.0f, 0.08f}, {0.9f, 0.85f, 0.1f});
            drawSphere(root * T({0.2f, 3.05f, 0.0f}), 0.10f, {1.4f, 1.4f, 1.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({0.2f, 1.80f,  1.15f}), 0.10f, {0.1f, 1.4f, 0.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({0.2f, 1.80f, -1.15f}), 0.10f, {1.4f, 0.1f, 0.1f}, 64.0f, vec3(1.0f), true);
            break;
        }

        case ShipType::CoastalTanker: {
            // Sleek Coastal Oil / Chemical Tanker
            // Red bottom hull
            drawCube(root * T({0.0f, 0.28f, 0.0f}), {12.8f, 0.58f, 3.0f}, {0.65f, 0.18f, 0.15f}, 0.0f, 32.0f);
            drawCube(root * T({0.0f, 0.78f, 0.0f}), {13.0f, 0.58f, 3.05f}, s.color, 0.0f, 48.0f);
            drawCylinder(root * T({6.5f, 0.55f, 0.0f}) * R(90, {0,0,1}), {0.85f, 2.4f, 2.8f}, s.color, 0.0f, 48.0f);
            drawCube(root * T({0.0f, 1.08f, 0.0f}), {13.1f, 0.06f, 3.1f}, {0.92f, 0.92f, 0.94f}); // White waterline stripe

            // Piping manifold & elevated catwalk on deck
            drawCube(root * T({0.8f, 1.25f, 0.0f}), {7.8f, 0.12f, 0.6f}, {0.55f, 0.58f, 0.62f});
            for (float px = -2.2f; px <= 3.8f; px += 2.0f) {
                drawCylinder(root * T({px, 1.30f,  0.8f}), {0.22f, 0.6f, 0.22f}, {0.85f, 0.82f, 0.2f});
                drawCylinder(root * T({px, 1.30f, -0.8f}), {0.22f, 0.6f, 0.22f}, {0.85f, 0.82f, 0.2f});
            }

            // Aft Superstructure & Funnel
            drawCube(root * T({-4.8f, 1.75f, 0.0f}), {2.6f, 1.45f, 2.6f}, {0.92f, 0.92f, 0.95f}, 0.0f, 32.0f);
            drawCube(root * T({-4.2f, 2.05f, 0.0f}), {0.1f, 0.40f, 2.3f}, {0.25f, 0.65f, 0.85f}, 0.0f, 96.0f); // Window
            drawCylinder(root * T({-5.4f, 2.65f, 0.0f}), {0.5f, 1.0f, 0.5f}, {0.85f, 0.2f, 0.15f});
            drawCylinder(root * T({-4.6f, 2.85f, 0.0f}), {0.08f, 1.3f, 0.08f}, {0.8f, 0.8f, 0.8f});
            drawSphere(root * T({-4.6f, 3.50f, 0.0f}), 0.11f, {1.4f, 1.4f, 1.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({-4.8f, 1.95f,  1.35f}), 0.10f, {0.1f, 1.4f, 0.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({-4.8f, 1.95f, -1.35f}), 0.10f, {1.4f, 0.1f, 0.1f}, 64.0f, vec3(1.0f), true);
            break;
        }

        case ShipType::PilotBoat: {
            // Agile Harbor Pilot / Fast Escort Craft
            drawCube(root * T({0.0f, 0.45f, 0.0f}), {5.2f, 0.9f, 2.1f}, s.color, 0.0f, 48.0f);
            drawCylinder(root * T({2.6f, 0.45f, 0.0f}) * R(90, {0,0,1}), {0.9f, 1.5f, 1.9f}, s.color, 0.0f, 48.0f);
            drawCube(root * T({0.0f, 0.85f, 0.0f}), {5.3f, 0.06f, 2.15f}, {0.95f, 0.65f, 0.15f}); // Orange pilot stripe

            // Wheelhouse cabin
            drawCube(root * T({-0.4f, 1.25f, 0.0f}), {2.2f, 0.85f, 1.6f}, {0.95f, 0.95f, 0.95f}, 0.0f, 32.0f);
            drawCube(root * T({0.4f, 1.30f, 0.0f}), {0.08f, 0.35f, 1.4f}, {0.25f, 0.65f, 0.85f}, 0.0f, 96.0f);

            // Pilot Mast & Nav Lights
            drawCylinder(root * T({-0.4f, 1.95f, 0.0f}), {0.06f, 0.8f, 0.06f}, {0.95f, 0.85f, 0.12f});
            drawSphere(root * T({-0.4f, 2.35f, 0.0f}), 0.09f, {1.4f, 1.4f, 1.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({-0.4f, 1.45f,  0.85f}), 0.08f, {0.1f, 1.4f, 0.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({-0.4f, 1.45f, -0.85f}), 0.08f, {1.4f, 0.1f, 0.1f}, 64.0f, vec3(1.0f), true);
            break;
        }

        case ShipType::BulkCarrier: {
            // Large Ocean Bulk Cargo Vessel
            // Hull bottom & main hull
            drawCube(root * T({0.0f, 0.38f, 0.0f}), {16.0f, 0.85f, 3.8f}, {0.55f, 0.20f, 0.16f}, 0.0f, 32.0f);
            drawCube(root * T({0.0f, 1.05f, 0.0f}), {16.2f, 0.65f, 3.85f}, s.color, 0.0f, 48.0f);
            drawCylinder(root * T({8.1f, 0.75f, 0.0f}) * R(90, {0,0,1}), {1.0f, 2.6f, 3.6f}, s.color, 0.0f, 48.0f);

            // 4 Cargo Hatch Covers along deck
            for (float hx : {-4.0f, -1.2f, 1.6f, 4.4f}) {
                drawCube(root * T({hx, 1.50f, 0.0f}), {2.2f, 0.35f, 3.0f}, {0.45f, 0.48f, 0.52f}, 0.0f, 32.0f);
            }
            // 3 Deck Cargo Cranes
            for (float cx : {-2.6f, 0.2f, 3.0f}) {
                drawCylinder(root * T({cx, 1.9f, 0.0f}), {0.35f, 0.8f, 0.35f}, {0.92f, 0.85f, 0.12f});
                drawCube(root * T({cx + 0.6f, 2.4f, 0.0f}), {1.8f, 0.18f, 0.18f}, {0.92f, 0.85f, 0.12f}, 25.0f);
            }

            // Tall Aft Superstructure
            drawCube(root * T({-6.2f, 2.2f, 0.0f}), {2.8f, 2.2f, 3.2f}, {0.92f, 0.92f, 0.95f}, 0.0f, 32.0f);
            drawCube(root * T({-5.6f, 2.6f, 0.0f}), {0.1f, 0.55f, 2.8f}, {0.25f, 0.65f, 0.85f}, 0.0f, 96.0f);
            drawCylinder(root * T({-7.0f, 3.5f, 0.0f}), {0.6f, 1.4f, 0.6f}, {0.18f, 0.35f, 0.75f});
            drawCylinder(root * T({-6.0f, 3.7f, 0.0f}), {0.10f, 1.8f, 0.10f}, {0.85f, 0.85f, 0.85f});
            drawSphere(root * T({-6.0f, 4.6f, 0.0f}), 0.14f, {1.4f, 1.4f, 1.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({-6.2f, 2.6f,  1.65f}), 0.11f, {0.1f, 1.4f, 0.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({-6.2f, 2.6f, -1.65f}), 0.11f, {1.4f, 0.1f, 0.1f}, 64.0f, vec3(1.0f), true);
            break;
        }

        default:
        case ShipType::ContainerFeeder: {
            // Feeder Container Ship with colored container stacks
            // Red bottom hull
            drawCube(root * T({0.0f, 0.25f, 0.0f}), {11.0f, 0.65f, 3.4f}, {0.65f, 0.18f, 0.15f}, 0.0f, 32.0f);
            drawCylinder(root * T({5.5f, 0.25f, 0.0f}) * R(90, {0,0,1}), {0.65f, 2.2f, 3.2f}, {0.65f, 0.18f, 0.15f}, 0.0f, 32.0f);

            // Main Hull
            drawCube(root * T({0.0f, 0.85f, 0.0f}), {11.2f, 0.65f, 3.45f}, s.color, 0.0f, 48.0f);
            drawCylinder(root * T({5.6f, 0.85f, 0.0f}) * R(90, {0,0,1}), {0.65f, 2.3f, 3.2f}, s.color, 0.0f, 48.0f);
            drawCube(root * T({0.0f, 1.20f, 0.0f}), {11.3f, 0.08f, 3.5f}, {0.92f, 0.92f, 0.94f}); // Sheer stripe

            // Multi-colored container cargo stacks on forward deck
            for (int row = -1; row <= 1; ++row) {
                float cx = -0.5f + row * 2.5f;
                for (int h = 0; h < 2; ++h) {
                    vec3 cCol = COLORS[(std::abs(row) + h + static_cast<int>(s.phase)) % 6];
                    drawCube(root * T({cx, 1.65f + h * 0.95f, 0.0f}), {2.4f, 0.90f, 2.6f}, cCol, 0.0f, 48.0f);
                }
            }

            // Rear Bridge & Funnel
            drawCube(root * T({-4.0f, 2.0f, 0.0f}), {2.4f, 1.6f, 2.8f}, {0.92f, 0.92f, 0.95f}, 0.0f, 32.0f);
            drawCube(root * T({-3.6f, 2.3f, 0.0f}), {0.1f, 0.45f, 2.4f}, {0.25f, 0.65f, 0.85f}, 0.0f, 96.0f);
            drawCylinder(root * T({-4.8f, 3.0f, 0.0f}), {0.6f, 1.2f, 0.6f}, {0.85f, 0.55f, 0.12f});
            drawCylinder(root * T({-4.8f, 3.55f, 0.0f}), {0.62f, 0.2f, 0.62f}, {0.12f, 0.12f, 0.14f}); // Black top
            drawCylinder(root * T({-3.8f, 3.3f, 0.0f}), {0.10f, 1.6f, 0.10f}, {0.85f, 0.85f, 0.85f});
            drawSphere(root * T({-3.8f, 4.1f, 0.0f}), 0.12f, {1.4f, 1.4f, 1.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({-4.0f, 2.4f,  1.45f}), 0.10f, {0.1f, 1.4f, 0.2f}, 64.0f, vec3(1.0f), true);
            drawSphere(root * T({-4.0f, 2.4f, -1.45f}), 0.10f, {1.4f, 0.1f, 0.1f}, 64.0f, vec3(1.0f), true);
            break;
        }
    }
}

// ---------------- 10. TRUCKS & MOVING VEHICLES ----------------
static void drawTruck(const vec3& p, float phase, bool carryingCargo = true) {
    float bob = 0.02f * std::sin(simTime * 5.0f + phase);
    mat4 root = T(p + vec3(0, bob, 0));

    drawMesh(cubeMesh, root * T({0.0f, 0.65f, 0.0f}) * S({4.4f, 0.45f, 1.8f}), {0.18f, 0.20f, 0.22f}, 32.0f);
    drawMesh(cubeMesh, root * T({1.7f, 1.25f, 0.0f}) * S({1.1f, 1.1f, 1.7f}), {0.85f, 0.35f, 0.15f}, 48.0f);
    drawMesh(cubeMesh, root * T({1.95f, 1.35f, 0.0f}) * S({0.35f, 0.65f, 1.5f}), {0.65f, 0.85f, 0.95f}, 96.0f);

    if (carryingCargo) {
        drawMesh(cubeMesh, root * T({-0.6f, 1.55f, 0.0f}) * S({3.0f, 1.2f, 1.7f}), COLORS[phase > 2.0f ? 0 : 4], 48.0f);
    }

    drawWheel(p + vec3(-1.3f, 0.35f,  0.95f), 0.38f, 0.22f, {0.08f, 0.08f, 0.08f});
    drawWheel(p + vec3( 1.3f, 0.35f,  0.95f), 0.38f, 0.22f, {0.08f, 0.08f, 0.08f});
    drawWheel(p + vec3(-1.3f, 0.35f, -0.95f), 0.38f, 0.22f, {0.08f, 0.08f, 0.08f});
    drawWheel(p + vec3( 1.3f, 0.35f, -0.95f), 0.38f, 0.22f, {0.08f, 0.08f, 0.08f});

    drawSphere(p + vec3(2.25f, 0.65f,  0.6f), 0.12f, vec3(1.2f, 1.15f, 0.8f), 32.0f, vec3(1.0f), true);
    drawSphere(p + vec3(2.25f, 0.65f, -0.6f), 0.12f, vec3(1.2f, 1.15f, 0.8f), 32.0f, vec3(1.0f), true);
}

// ---------------- 10b. PLAYABLE DRIVABLE CARGO LORRY ----------------
static void drawPlayableLorry() {
    mat4 root = T(lorry.pos) * R(lorry.yaw, {0, 1, 0});

    // 1. Heavy Industrial Truck Chassis & Trailer Bed
    drawCube(root * T({0.0f, 0.58f, 0.0f}), {5.4f, 0.28f, 2.3f}, {0.18f, 0.20f, 0.22f}, 0.0f, 32.0f);
    // Steel side skirts & fuel tanks
    drawCube(root * T({0.2f, 0.40f,  1.05f}), {2.0f, 0.32f, 0.22f}, {0.35f, 0.38f, 0.42f});
    drawCube(root * T({0.2f, 0.40f, -1.05f}), {2.0f, 0.32f, 0.22f}, {0.35f, 0.38f, 0.42f});

    // 2. Driver Cab Structure (Hollow Interior with Real Cabin Architecture)
    // 2a. Lower Cowl / Engine Bay & Heavy Front Bumper (Strictly below dashboard line: Y <= 1.05m)
    drawCube(root * T({2.05f, 0.82f, 0.0f}), {1.45f, 0.38f, 2.10f}, {0.92f, 0.52f, 0.12f}, 0.0f, 48.0f);
    drawCube(root * T({2.74f, 0.65f, 0.0f}), {0.14f, 0.32f, 2.22f}, {0.14f, 0.15f, 0.16f}, 0.0f, 32.0f); // Front Bumper
    drawCube(root * T({2.73f, 0.90f, 0.0f}), {0.05f, 0.26f, 1.60f}, {0.10f, 0.10f, 0.12f}, 0.0f, 16.0f); // Radiator grille
    drawCube(root * T({2.74f, 0.90f, 0.0f}), {0.02f, 0.06f, 0.30f}, {0.85f, 0.85f, 0.90f}, 0.0f, 96.0f); // Chrome badge

    // 2b. Cab Interior Floor & Rear Bulkhead Wall
    drawCube(root * T({1.85f, 0.68f, 0.0f}), {1.55f, 0.06f, 2.05f}, {0.15f, 0.16f, 0.18f}, 0.0f, 16.0f); // Floor
    drawCube(root * T({1.20f, 1.55f, 0.0f}), {0.08f, 1.45f, 2.10f}, {0.92f, 0.52f, 0.12f}, 0.0f, 48.0f); // Rear wall
    drawCube(root * T({1.95f, 2.28f, 0.0f}), {1.55f, 0.08f, 2.10f}, {0.92f, 0.52f, 0.12f}, 0.0f, 48.0f); // Roof

    // Amber Roof Clearance Marker Lights
    for (float lx : {-0.6f, -0.3f, 0.0f, 0.3f, 0.6f}) {
        drawSphere(root * T({2.62f, 2.34f, lx}), 0.045f, {1.4f, 0.8f, 0.1f}, 64.0f, vec3(1.0f), true);
    }

    // 2c. Corner A-Pillars & B-Pillars (Slim Structural Posts)
    drawCube(root * T({2.68f, 1.68f, -1.02f}), {0.06f, 1.15f, 0.06f}, {0.18f, 0.20f, 0.22f}, 0.0f, 32.0f); // Front Left A-pillar
    drawCube(root * T({2.68f, 1.68f,  1.02f}), {0.06f, 1.15f, 0.06f}, {0.18f, 0.20f, 0.22f}, 0.0f, 32.0f); // Front Right A-pillar
    drawCube(root * T({1.22f, 1.68f, -1.02f}), {0.06f, 1.15f, 0.06f}, {0.18f, 0.20f, 0.22f}, 0.0f, 32.0f); // Rear Left B-pillar
    drawCube(root * T({1.22f, 1.68f,  1.02f}), {0.06f, 1.15f, 0.06f}, {0.18f, 0.20f, 0.22f}, 0.0f, 32.0f); // Rear Right B-pillar

    // Lower Door Side Panels (Below side window line at Y = 1.15m)
    drawCube(root * T({1.95f, 1.15f, -1.03f}), {1.45f, 0.32f, 0.05f}, {0.92f, 0.52f, 0.12f}, 0.0f, 48.0f);
    drawCube(root * T({1.95f, 1.15f,  1.03f}), {1.45f, 0.32f, 0.05f}, {0.92f, 0.52f, 0.12f}, 0.0f, 48.0f);

    // 2d. FRONT WINDSHIELD (CRYSTAL TRANSPARENT GLASS FOR 100% UNRESTRICTED DRIVER VIEW)
    drawGlassCube(root * T({2.68f, 1.70f, 0.0f}), {0.02f, 1.05f, 1.98f}, {0.55f, 0.85f, 0.98f}, 0.22f, 128.0f);
    // Windshield frame rubber gaskets
    drawCube(root * T({2.68f, 2.24f, 0.0f}), {0.04f, 0.04f, 2.02f}, {0.12f, 0.12f, 0.14f});
    drawCube(root * T({2.68f, 1.16f, 0.0f}), {0.04f, 0.04f, 2.02f}, {0.12f, 0.12f, 0.14f});
    // Windshield twin wipers
    drawCube(root * T({2.70f, 1.22f, -0.45f}), {0.02f, 0.03f, 0.45f}, {0.10f, 0.10f, 0.10f});
    drawCube(root * T({2.70f, 1.22f,  0.45f}), {0.02f, 0.03f, 0.45f}, {0.10f, 0.10f, 0.10f});

    // Transparent Side Windows (Left & Right)
    drawGlassCube(root * T({1.95f, 1.70f, -1.03f}), {1.40f, 0.48f, 0.02f}, {0.55f, 0.85f, 0.98f}, 0.22f, 128.0f);
    drawGlassCube(root * T({1.95f, 1.70f,  1.03f}), {1.40f, 0.48f, 0.02f}, {0.55f, 0.85f, 0.98f}, 0.22f, 128.0f);

    // 2e. Ergonomic Driver's Bucket Seat & Passenger Seat
    // Driver Seat (at Z = +0.45m)
    drawCube(root * T({1.60f, 0.92f, 0.45f}), {0.58f, 0.18f, 0.52f}, {0.16f, 0.16f, 0.18f}, 0.0f, 24.0f); // Cushion
    drawCube(root * T({1.30f, 1.35f, 0.45f}), {0.14f, 0.65f, 0.48f}, {0.14f, 0.14f, 0.16f}, 0.0f, 24.0f); // Backrest
    drawCube(root * T({1.28f, 1.75f, 0.45f}), {0.12f, 0.18f, 0.32f}, {0.12f, 0.12f, 0.14f}, 0.0f, 24.0f); // Headrest
    // Passenger Seat (at Z = -0.45m)
    drawCube(root * T({1.60f, 0.92f, -0.45f}), {0.58f, 0.18f, 0.52f}, {0.16f, 0.16f, 0.18f}, 0.0f, 24.0f);
    drawCube(root * T({1.30f, 1.35f, -0.45f}), {0.14f, 0.65f, 0.48f}, {0.14f, 0.14f, 0.16f}, 0.0f, 24.0f);

    // 2f. Cockpit Dashboard Table & Glowing Digital Instrument Cluster
    drawCube(root * T({2.38f, 1.08f, 0.0f}), {0.52f, 0.16f, 1.95f}, {0.12f, 0.13f, 0.15f}, 0.0f, 24.0f);
    // Speedometer instrument binnacle
    drawCube(root * T({2.32f, 1.20f, 0.45f}), {0.26f, 0.08f, 0.44f}, {0.15f, 0.16f, 0.18f}, 0.0f, 32.0f);
    // Glowing Digital Speedometer Display (Green/Cyan high-tech HUD)
    drawCube(root * T({2.36f, 1.18f, 0.45f}), {0.02f, 0.08f, 0.34f}, {0.2f, 1.4f, 0.5f}, 0.0f, 64.0f, vec3(1.0f), true);
    // Glowing Central GPS Navigation Screen (Cyan harbor radar map)
    drawCube(root * T({2.38f, 1.16f, 0.0f}), {0.02f, 0.10f, 0.22f}, {0.2f, 0.85f, 1.4f}, 0.0f, 64.0f, vec3(1.0f), true);

    // 2g. Interactive Turning Steering Wheel
    mat4 steerCol = root * T({2.22f, 1.15f, 0.45f}) * R(68.0f, {0,0,1});
    drawCylinder(steerCol, {0.06f, 0.32f, 0.06f}, {0.15f, 0.15f, 0.16f});
    mat4 steerWheel = root * T({2.18f, 1.26f, 0.45f}) * R(68.0f, {0,0,1}) * R(lorry.steerAngle * 2.8f, {1,0,0});
    drawMesh(ringMesh, steerWheel * S({0.30f, 0.04f, 0.30f}), {0.12f, 0.12f, 0.12f}, 32.0f);
    drawCylinder(steerWheel, {0.12f, 0.05f, 0.12f}, {0.20f, 0.20f, 0.22f});

    // Twin Large Commercial Side Mirrors
    drawCube(root * T({2.62f, 1.62f,  1.24f}), {0.10f, 0.36f, 0.18f}, {0.12f, 0.12f, 0.14f});
    drawCube(root * T({2.62f, 1.62f, -1.24f}), {0.10f, 0.36f, 0.18f}, {0.12f, 0.12f, 0.14f});
    // Mirror glass reflective faces
    drawCube(root * T({2.58f, 1.62f,  1.24f}), {0.02f, 0.32f, 0.15f}, {0.65f, 0.80f, 0.95f}, 0.0f, 96.0f);
    drawCube(root * T({2.58f, 1.62f, -1.24f}), {0.02f, 0.32f, 0.15f}, {0.65f, 0.80f, 0.95f}, 0.0f, 96.0f);

    // 3. Front Steerable Wheels (Rotated with steerAngle!)
    mat4 wheelFL = root * T({1.75f, 0.42f,  1.15f}) * R(lorry.steerAngle, {0,1,0});
    mat4 wheelFR = root * T({1.75f, 0.42f, -1.15f}) * R(lorry.steerAngle, {0,1,0});
    drawCylinder(wheelFL, {0.80f, 0.28f, 0.80f}, {0.10f, 0.10f, 0.10f}, 90.0f);
    drawCylinder(wheelFR, {0.80f, 0.28f, 0.80f}, {0.10f, 0.10f, 0.10f}, 90.0f);

    // 4. Rear Twin Axle Wheels
    drawCylinder(root * T({-1.2f, 0.42f,  1.15f}), {0.80f, 0.28f, 0.80f}, {0.10f, 0.10f, 0.10f}, 90.0f);
    drawCylinder(root * T({-1.2f, 0.42f, -1.15f}), {0.80f, 0.28f, 0.80f}, {0.10f, 0.10f, 0.10f}, 90.0f);
    drawCylinder(root * T({-2.1f, 0.42f,  1.15f}), {0.80f, 0.28f, 0.80f}, {0.10f, 0.10f, 0.10f}, 90.0f);
    drawCylinder(root * T({-2.1f, 0.42f, -1.15f}), {0.80f, 0.28f, 0.80f}, {0.10f, 0.10f, 0.10f}, 90.0f);

    // 5. High-Intensity Headlights & Tail Lights
    drawSphere(root * T({2.72f, 0.82f,  0.72f}), 0.14f, vec3(1.3f, 1.25f, 0.9f), 64.0f, vec3(1.0f), true);
    drawSphere(root * T({2.72f, 0.82f, -0.72f}), 0.14f, vec3(1.3f, 1.25f, 0.9f), 64.0f, vec3(1.0f), true);
    drawSphere(root * T({-2.72f, 0.65f,  0.85f}), 0.12f, vec3(1.2f, 0.15f, 0.15f), 64.0f, vec3(1.0f), true);
    drawSphere(root * T({-2.72f, 0.65f, -0.85f}), 0.12f, vec3(1.2f, 0.15f, 0.15f), 64.0f, vec3(1.0f), true);

    // 6. Loaded Container on the Flatbed Trailer!
    if (lorry.hasCargo) {
        vec3 cargoCol = shipCargos[lorry.cargoIndex % shipCargos.size()].color;
        // Container box:
        drawCube(root * T({-0.75f, 1.48f, 0.0f}), {3.6f, 1.2f, 2.2f}, cargoCol, 0.0f, 48.0f);
        // Steel corner edge twistlocks:
        for (float cx : {-2.5f, 1.0f}) {
            for (float cz : {-1.08f, 1.08f}) {
                drawCube(root * T({cx, 0.92f, cz}), {0.15f, 0.18f, 0.15f}, {0.85f, 0.75f, 0.15f});
            }
        }
    }
}

// ---------------- 10c. FREIGHT LOGISTICS DEPOT (DELIVERY GOAL) ----------------
static void drawFreightDepot() {
    mat4 depotRoot = T(freightDepotPos);

    // Large Delivery Parking Bay on the dock road
    drawCube(depotRoot * T({0.0f, 0.04f, 0.0f}), {11.0f, 0.06f, 7.8f}, {0.95f, 0.85f, 0.15f}, 0.0f, 32.0f); // Hazard safety perimeter
    drawCube(depotRoot * T({0.0f, 0.05f, 0.0f}), {10.4f, 0.06f, 7.2f}, {0.16f, 0.18f, 0.20f}, 0.0f, 16.0f);  // Asphalt floor

    // Diagonal hazard stripes on the depot delivery pad
    for (float dx = -4.5f; dx <= 4.5f; dx += 1.4f) {
        drawCube(depotRoot * T({dx, 0.06f, 0.0f}), {0.35f, 0.02f, 6.6f}, {0.95f, 0.85f, 0.12f}, 35.0f, 32.0f);
    }

    // Safety traffic cones flanking the freight delivery terminal (Demonstrating Cone primitive)
    drawTrafficCone(freightDepotPos + vec3(-5.8f, 0.0f, -4.2f));
    drawTrafficCone(freightDepotPos + vec3(-5.8f, 0.0f,  4.2f));
    drawTrafficCone(freightDepotPos + vec3( 5.8f, 0.0f, -4.2f));
    drawTrafficCone(freightDepotPos + vec3( 5.8f, 0.0f,  4.2f));

    // 4 Corner Warning Towers with pulsating cyan & amber strobes
    float pulse = 0.65f + 0.35f * std::sin(simTime * 6.0f);
    vec3 depotCol = lorry.hasCargo ? (vec3(0.2f, 1.4f, 1.2f) * pulse) : vec3(0.3f, 0.8f, 0.9f);
    for (float px : {-5.2f, 5.2f}) {
        for (float pz : {-3.6f, 3.6f}) {
            drawCylinder(depotRoot * T({px, 2.25f, pz}), {0.22f, 4.5f, 0.22f}, {0.18f, 0.20f, 0.22f});
            drawSphere(depotRoot * T({px, 4.6f, pz}), 0.28f, depotCol, 64.0f, vec3(1.0f), true);
        }
    }

    // Sky-Piercing Radiant Cyan / Gold Delivery Beacon (Shoots 28 meters into sky)
    drawCylinder(depotRoot * T({0.0f, 14.0f, 0.0f}), {0.4f, 28.0f, 0.4f}, depotCol, 0.0f, 64.0f, vec3(1.0f), true);
    drawSphere(depotRoot * T({0.0f, 1.4f, 0.0f}), 0.65f, depotCol, 64.0f, vec3(1.0f), true);

    // Glowing Holographic Delivery Target Gate
    if (lorry.hasCargo) {
        drawCube(depotRoot * T({0.0f, 0.10f, 0.0f}), {9.0f, 0.03f, 6.0f}, depotCol, 0.0f, 64.0f, vec3(1.0f), true);
        drawCube(depotRoot * T({0.0f, 0.11f, 0.0f}), {9.6f, 0.02f, 0.35f}, vec3(1.4f, 1.2f, 0.2f) * pulse, 0.0f, 64.0f, vec3(1.0f), true);
        drawCube(depotRoot * T({0.0f, 0.11f, 0.0f}), {0.35f, 0.02f, 6.6f}, vec3(1.4f, 1.2f, 0.2f) * pulse, 0.0f, 64.0f, vec3(1.0f), true);
    }

    // Stacked delivered containers at the Freight Depot storage yard!
    int depotStacks = std::min(lorry.deliveriesCompleted, 12);
    for (int i = 0; i < depotStacks; ++i) {
        float cx = 6.4f + (i % 3) * 2.8f;
        float cz = -2.5f + ((i / 3) % 2) * 3.4f;
        float cy = 0.65f + (i / 6) * 1.25f;
        vec3 col = COLORS[(i + 2) % 6];
        drawCube(depotRoot * T({cx, cy, cz}), {3.6f, 1.2f, 2.2f}, col, 0.0f, 48.0f);
    }
}

// ---------------- 11. 2D HUD RENDERING (COMPASS, OBJECTIVES, CROSSHAIR) ----------------
static void drawHUDQuad(float x, float y, float w, float h, float u0, float v0, float u1, float v1) {
    HUDVertex verts[6] = {
        {{x,     y},     {u0, v0}},
        {{x + w, y},     {u1, v0}},
        {{x + w, y + h}, {u1, v1}},
        {{x,     y},     {u0, v0}},
        {{x + w, y + h}, {u1, v1}},
        {{x,     y + h}, {u0, v1}}
    };
    glBindBuffer(GL_ARRAY_BUFFER, hudVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(verts), verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void drawHUDRect(float x, float y, float w, float h, const vec4& color) {
    hudShader.setBool("uUseTexture", false);
    hudShader.setVec4("uColor", color);
    drawHUDQuad(x, y, w, h, 0, 0, 1, 1);
}

static void drawHUDChar(char c, float x, float y, float scale, const vec4& color) {
    if (c < 0 || c >= 128) c = ' ';
    int col = c % 16;
    int row = c / 16;
    float u0 = (col * 8.0f) / 128.0f;
    float u1 = ((col + 1) * 8.0f) / 128.0f;
    float v0 = (row * 8.0f) / 64.0f;
    float v1 = ((row + 1) * 8.0f) / 64.0f;

    hudShader.setBool("uUseTexture", true);
    hudShader.setVec4("uColor", color);
    drawHUDQuad(x, y, 8.0f * scale, 8.0f * scale, u0, v0, u1, v1);
}

static void drawHUDString(const std::string& str, float x, float y, float scale, const vec4& color) {
    float curX = x;
    for (char c : str) {
        drawHUDChar(c, curX, y, scale, color);
        curX += 8.0f * scale;
    }
}

static void renderHUD(int width, int height) {
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    hudShader.use();
    mat4 ortho = glm::ortho(0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, -1.0f, 1.0f);
    hudShader.setMat4("uOrtho", ortho);

    glBindVertexArray(hudVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, fontTexID);
    hudShader.setBool("uUseTexture", true);

    // ================= A. TOP COMPASS TAPE (Inspired by Reference Images!) =================
    float compassWidth = 480.0f;
    float compassHeight = 36.0f;
    float compassX = (width - compassWidth) * 0.5f;
    float compassY = 18.0f;

    // Translucent background strip
    drawHUDRect(compassX, compassY, compassWidth, compassHeight, vec4(0.05f, 0.08f, 0.12f, 0.65f));
    // Thin border lines
    drawHUDRect(compassX, compassY, compassWidth, 2.0f, vec4(0.85f, 0.90f, 0.95f, 0.60f));
    drawHUDRect(compassX, compassY + compassHeight - 2.0f, compassWidth, 2.0f, vec4(0.85f, 0.90f, 0.95f, 0.60f));

    // Player / Operator / Lorry heading in degrees (0 to 360)
    float activeYaw = player.yaw;
    if (currentCamMode == CameraMode::LorryDrive || lorry.playerDriving) {
        activeYaw = lorry.yaw + (lorry.chaseCam ? 0.0f : camera.yaw);
    } else if (player.inCraneCab || currentCamMode != CameraMode::FPSOperator) {
        activeYaw = camera.yaw;
    }
    float heading = std::fmod(-activeYaw - 90.0f, 360.0f);
    if (heading < 0.0f) heading += 360.0f;

    // Compass tick marks and degree labels
    float centerX = compassX + compassWidth * 0.5f;
    for (int deg = 0; deg < 360; deg += 15) {
        float diff = static_cast<float>(deg) - heading;
        while (diff < -180.0f) diff += 360.0f;
        while (diff > 180.0f) diff -= 360.0f;

        float tickX = centerX + diff * 3.2f;
        if (tickX >= compassX + 10.0f && tickX <= compassX + compassWidth - 10.0f) {
            std::string label = "";
            bool isMajor = false;
            if (deg == 0)   { label = "N";  isMajor = true; }
            else if (deg == 90)  { label = "E";  isMajor = true; }
            else if (deg == 180) { label = "S";  isMajor = true; }
            else if (deg == 270) { label = "W";  isMajor = true; }
            else if (deg % 45 == 0) {
                if (deg == 45) label = "NE";
                else if (deg == 135) label = "SE";
                else if (deg == 225) label = "SW";
                else if (deg == 315) label = "NW";
            } else {
                label = std::to_string(deg);
            }

            vec4 col = isMajor ? vec4(1.0f, 0.85f, 0.2f, 1.0f) : vec4(0.85f, 0.90f, 0.95f, 0.85f);
            drawHUDRect(tickX - 1.0f, compassY + 2.0f, 2.0f, isMajor ? 8.0f : 5.0f, col);
            float labelScale = isMajor ? 1.4f : 1.0f;
            float strW = label.length() * 8.0f * labelScale;
            drawHUDString(label, tickX - strW * 0.5f, compassY + 14.0f, labelScale, col);
        }
    }

    // Active center heading cursor marker (Yellow diamond / caret)
    drawHUDRect(centerX - 4.0f, compassY + compassHeight + 2.0f, 8.0f, 8.0f, vec4(1.0f, 0.85f, 0.15f, 1.0f));
    std::string headingStr = std::to_string(static_cast<int>(heading)) + "^";
    drawHUDString(headingStr, centerX - 12.0f, compassY + compassHeight + 14.0f, 1.3f, vec4(1.0f, 0.85f, 0.15f, 1.0f));

    // ================= B. PORT OBJECTIVES LIST (Top-Left, Inspired by Reference Images!) =================
    float objX = 24.0f;
    float objY = 24.0f;

    // Header info matching FPS reference:
    std::string statusHeader = "SCORE " + std::to_string(crane.score) + "   OPERATORS 6";
    drawHUDString(statusHeader, objX, objY, 1.5f, vec4(0.95f, 0.95f, 0.98f, 0.95f));

    drawHUDString("PORT MANAGEMENT OBJECTIVES", objX, objY + 24.0f, 1.5f, vec4(1.0f, 0.85f, 0.2f, 1.0f));

    for (size_t i = 0; i < objectives.size(); ++i) {
        float lineY = objY + 50.0f + i * 22.0f;
        float dist = glm::length(player.pos - objectives[i].targetPos);
        int distMeters = static_cast<int>(dist);

        std::string mark = objectives[i].completed ? "[X] " : "[ ] ";
        std::string distStr = "(" + std::to_string(distMeters) + " M)";
        std::string fullLine = mark + objectives[i].text;

        // Pad to line up distance
        while (fullLine.length() < 34) fullLine += " ";
        fullLine += distStr;

        vec4 col = objectives[i].completed ? vec4(0.35f, 0.95f, 0.45f, 0.85f)
                  : (static_cast<int>(i) == currentObjectiveIndex ? vec4(1.0f, 1.0f, 1.0f, 1.0f) : vec4(0.70f, 0.75f, 0.80f, 0.70f));

        drawHUDString(fullLine, objX, lineY, 1.35f, col);
    }

    // Top Right: Mode indicator banner
    std::string modeStr = player.inCraneCab ? "CRANE OPERATOR CABIN (1ST PERSON)"
                        : ((currentCamMode == CameraMode::FPSOperator) ? "OPERATOR POV - TAB FOR CURSOR" : "CAMERA MODE: FREE/OVERVIEW");
    float modeStrW = modeStr.length() * 8.0f * 1.3f;
    drawHUDString(modeStr, width - modeStrW - 24.0f, 24.0f, 1.3f, player.inCraneCab ? vec4(0.3f, 1.0f, 0.5f, 1.0f) : vec4(1.0f, 0.85f, 0.2f, 0.95f));

    // Toast Notification Banner (Score updates, cargo events, status messages)
    if (crane.toastTimer > 0.0f && !crane.toastMessage.empty()) {
        float tScale = 1.35f;
        float tW = crane.toastMessage.length() * 8.0f * tScale;
        float tX = (width - tW) * 0.5f;
        float tY = 70.0f;
        drawHUDRect(tX - 16.0f, tY - 6.0f, tW + 32.0f, 28.0f, vec4(0.06f, 0.12f, 0.20f, 0.88f));
        drawHUDRect(tX - 16.0f, tY + 22.0f, tW + 32.0f, 2.0f, vec4(0.25f, 0.95f, 0.45f, 0.95f));
        drawHUDString(crane.toastMessage, tX, tY, tScale, vec4(0.35f, 1.0f, 0.55f, 1.0f));
    }

    // ================= C. INTERACTIVE CONTEXT ACTION PROMPTS (Center-Bottom) =================
    if (currentCamMode == CameraMode::LorryDrive || lorry.playerDriving) {
        // Detailed Truck Dashboard HUD!
        float panelW = 680.0f;
        float panelH = 76.0f;
        float panelX = (width - panelW) * 0.5f;
        float panelY = height - 98.0f;
        drawHUDRect(panelX, panelY, panelW, panelH, vec4(0.04f, 0.08f, 0.14f, 0.85f));
        drawHUDRect(panelX, panelY, panelW, 2.0f, vec4(0.2f, 0.85f, 1.2f, 0.9f));

        std::string line1 = "[W/S] ACCEL / BRAKE / REVERSE  |  [A/D] STEER  |  [SPACE] HANDBRAKE";
        float distToDepot = glm::length(lorry.pos - freightDepotPos);

        std::string line2 = "";
        std::string camStr = lorry.chaseCam ? "[V] CAB POV" : "[V] CHASE CAM";
        if (lorry.hasCargo) {
            line2 = (distToDepot < 6.5f)
                  ? "★ [ARRIVED AT FREIGHT DEPOT! DELIVERING CARGO...] ★"
                  : ("DELIVERY GOAL: FREIGHT DEPOT (" + std::to_string(static_cast<int>(distToDepot)) + "M CYAN BEACON)  |  " + camStr + "  |  [L] EXIT");
        } else {
            line2 = "LORRY EMPTY | DRIVE TO DOCK BAY TO LOAD NEXT CARGO | [C] CRANE | [L] EXIT";
        }

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1);
        int kmh = static_cast<int>(std::abs(lorry.speed) * 3.6f);
        ss << "SPEED: " << kmh << " KM/H | GEAR: " << (lorry.speed >= 0.05f ? "[D]" : (lorry.speed <= -0.05f ? "[R]" : "[N]"))
           << " | CARGO: " << (lorry.hasCargo ? "[LOADED]" : "[EMPTY]")
           << " | DELIVERIES: " << lorry.deliveriesCompleted << " | SCORE: " << crane.score;
        std::string line3 = ss.str();

        drawHUDString(line1, panelX + 16.0f, panelY + 10.0f, 1.25f, vec4(1.0f, 0.85f, 0.25f, 1.0f));
        drawHUDString(line2, panelX + 16.0f, panelY + 32.0f, 1.25f, (lorry.hasCargo && distToDepot < 6.5f) ? vec4(0.2f, 1.4f, 0.4f, 1.0f) : vec4(0.3f, 0.9f, 1.2f, 1.0f));
        drawHUDString(line3, panelX + 16.0f, panelY + 54.0f, 1.2f, vec4(0.9f, 0.95f, 1.0f, 1.0f));

    } else if (player.inCraneCab) {
        // Detailed Cockpit Telemetry Panel for the Crane Operator!
        float panelW = 660.0f;
        float panelH = 76.0f;
        float panelX = (width - panelW) * 0.5f;
        float panelY = height - 98.0f;
        drawHUDRect(panelX, panelY, panelW, panelH, vec4(0.04f, 0.08f, 0.14f, 0.82f));
        drawHUDRect(panelX, panelY, panelW, 2.0f, vec4(0.2f, 0.9f, 0.4f, 0.8f));

        std::string line1 = "[W/S] TROLLEY (SHIP <-> BAY) | [A/D] SLEW | [R/F] HOOK | [V] CAM VIEW | [SCROLL/Z,X] ZOOM";
        bool overShip = (crane.trolleyZ <= 2.2f && crane.hookY <= 5.2f);
        bool overBay = (crane.trolleyZ >= 11.5f && crane.trolleyZ <= 15.0f && crane.hookY <= 5.2f);

        std::string camStr = (crane.camView == 0) ? "[V] 1ST-PERSON CABIN" :
                             (crane.camView == 1) ? "[V] 3RD-PERSON OVERVIEW" : "[V] DROP CAM";
        std::string line2 = "";
        if (!crane.carrying) {
            std::string tName = (activeTargetCargoIndex >= 0 && activeTargetCargoIndex < static_cast<int>(shipCargos.size()))
                              ? shipCargos[activeTargetCargoIndex].name : "CARGO";
            line2 = overShip ? ("[SPACE] LOCK & HOIST " + tName + "  [READY!]")
                             : ("TARGET: " + tName + " (GOLD BEACON)  |  " + camStr + "  |  [C] EXIT");
        } else {
            line2 = overBay ? "[SPACE] LOAD ONTO LORRY  [READY!]"
                            : ("FLY TO UNLOAD BAY (GREEN BEACON)  |  " + camStr + "  |  [C] EXIT");
        }

        std::ostringstream ss;
        ss << std::fixed << std::setprecision(1);
        ss << "TROLLEY: " << crane.trolleyZ << "M | HOOK: " << crane.hookY << "M | ";
        if (crane.camView == 0) ss << "FOV: " << static_cast<int>(crane.fov) << " DEG | ";
        else if (crane.camView == 1) ss << "ZOOM: " << static_cast<int>(crane.orbitDist) << "M | ";
        ss << (crane.carrying ? "PAYLOAD: [ATTACHED]" : "SEEKING: [TARGET]")
           << " | SCORE: " << crane.score;
        std::string line3 = ss.str();

        drawHUDString(line1, panelX + 16.0f, panelY + 10.0f, 1.20f, vec4(1.0f, 0.85f, 0.25f, 1.0f));
        drawHUDString(line2, panelX + 16.0f, panelY + 32.0f, 1.25f, (overShip || overBay) ? vec4(0.2f, 1.4f, 0.4f, 1.0f) : vec4(0.9f, 0.9f, 0.95f, 1.0f));
        drawHUDString(line3, panelX + 16.0f, panelY + 54.0f, 1.20f, vec4(0.4f, 0.85f, 1.0f, 1.0f));

    } else {
        std::string promptStr = "";
        float distToBridge = glm::length(player.pos - vec3(0.0f, 0.4f, 5.6f));
        float distToTower = glm::length(player.pos - vec3(-45.0f, 0.0f, 24.0f));
        float distToLorry = glm::length(player.pos - lorry.pos);

        if (lorry.hasCargo) {
            promptStr = "[L] DRIVE LOADED CARGO LORRY (+1000 PTS)  |  [C] CRANE CABIN";
        } else if (distToLorry < 6.0f) {
            promptStr = "[L] DRIVE CARGO LORRY  |  [C] CRANE CABIN";
        } else if (distToBridge < 6.0f) {
            promptStr = "[E] OPERATE DRAWBRIDGE  |  [C] CRANE CABIN  |  [L] DRIVE LORRY";
        } else if (distToTower < 3.5f) {
            promptStr = "HOLD [W] CLIMB TOWER LADDER  |  [C] CRANE ROOM  |  [L] DRIVE LORRY";
        } else {
            promptStr = "[C] CRANE OPERATOR ROOM (1ST PERSON)  |  [L] DRIVE LORRY";
        }

        if (!promptStr.empty()) {
            float pW = promptStr.length() * 8.0f * 1.5f;
            float pX = (width - pW) * 0.5f;
            float pY = height - 85.0f;
            drawHUDRect(pX - 12.0f, pY - 6.0f, pW + 24.0f, 28.0f, vec4(0.06f, 0.10f, 0.16f, 0.80f));
            drawHUDString(promptStr, pX, pY, 1.5f, vec4(1.0f, 0.90f, 0.25f, 1.0f));
        }
    }

    // ================= D. TACTICAL CENTER CROSSHAIR =================
    if (currentCamMode == CameraMode::FPSOperator || player.inCraneCab) {
        float cx = width * 0.5f;
        float cy = height * 0.5f;
        vec4 crossCol = player.inCraneCab ? vec4(0.3f, 1.0f, 0.5f, 0.85f) : vec4(0.95f, 0.95f, 0.98f, 0.80f);
        drawHUDRect(cx - 8.0f, cy - 1.0f, 16.0f, 2.0f, crossCol);
        drawHUDRect(cx - 1.0f, cy - 8.0f, 2.0f, 16.0f, crossCol);
    }

    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

// ---------------- GAME SIMULATION UPDATES ----------------
static void updateShips(float dt) {
    for (size_t i = 0; i < ships.size(); ++i) {
        float dir = (ships[i].heading > 90.0f) ? -1.0f : 1.0f;
        ships[i].x += dir * ships[i].speed * shipSpeedMultiplier * dt;
        if (dir > 0.0f && ships[i].x > 260.0f) {
            ships[i].x = -260.0f;
        } else if (dir < 0.0f && ships[i].x < -260.0f) {
            ships[i].x = 260.0f;
        }
    }
}

static void updateBridge(float dt) {
    float target = 0.0f;
    if (autoBridge) {
        bool shipNear = false;
        for (const Ship& s : ships) {
            // Only canal ships (within canal boundaries Z: -5.0 to +5.0) can trigger drawbridge
            if (std::abs(s.z) < 5.0f && s.x > -16.0f && s.x < 16.0f) {
                shipNear = true;
                break;
            }
        }
        target = shipNear ? 65.0f : 0.0f;
    } else {
        target = bridgeAngle;
    }

    const float rate = 32.0f * dt;
    if (bridgeAngle < target) bridgeAngle = std::min(target, bridgeAngle + rate);
    if (bridgeAngle > target) bridgeAngle = std::max(target, bridgeAngle - rate);
}

static void updateCrane(float dt) {
    if (crane.toastTimer > 0.0f) {
        crane.toastTimer -= dt;
    }

    if (crane.manual) return;
    if (crane.autoState == Crane::AutoState::Idle) return;

    crane.autoTimer += dt;

    switch (crane.autoState) {
        case Crane::AutoState::ToShip:
            crane.trolleyZ = std::max(0.5f, crane.trolleyZ - crane.trolleySpeed * dt);
            if (crane.trolleyZ <= 0.55f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::LowerToShip;
            }
            break;

        case Crane::AutoState::LowerToShip:
            crane.hookY = std::max(2.4f, crane.hookY - crane.hookSpeed * dt);
            if (crane.hookY <= 2.45f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::GrabCargo;
            }
            break;

        case Crane::AutoState::GrabCargo:
            if (crane.autoTimer > 0.4f) {
                crane.carrying = true;
                crane.toastMessage = "AUTO: SHIP CARGO ATTACHED -> MOVING TO UNLOAD BAY";
                crane.toastTimer = 3.0f;
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::LiftCargo;
            }
            break;

        case Crane::AutoState::LiftCargo:
            crane.hookY = std::min(6.2f, crane.hookY + crane.hookSpeed * dt);
            if (crane.hookY >= 6.15f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::ToYard;
            }
            break;

        case Crane::AutoState::ToYard:
            crane.trolleyZ = std::min(13.5f, crane.trolleyZ + crane.trolleySpeed * dt);
            if (crane.trolleyZ >= 13.45f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::LowerToYard;
            }
            break;

        case Crane::AutoState::LowerToYard:
            crane.hookY = std::max(1.6f, crane.hookY - crane.hookSpeed * dt);
            if (crane.hookY <= 1.65f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::ReleaseCargo;
            }
            break;

        case Crane::AutoState::ReleaseCargo:
            if (crane.autoTimer > 0.5f) {
                crane.carrying = false;
                crane.score += 500;
                crane.unloadedCount++;
                crane.cargoIndex = (crane.cargoIndex + 1) % 6;
                crane.toastMessage = "+500 PTS! CARGO SAFELY DEPOSITED IN DOCK BAY! TOTAL: " + std::to_string(crane.unloadedCount);
                crane.toastTimer = 4.0f;
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::ReturnCab;
            }
            break;

        case Crane::AutoState::ReturnCab:
            crane.hookY = std::min(5.5f, crane.hookY + crane.hookSpeed * dt);
            if (crane.hookY >= 5.45f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::Idle;
            }
            break;

        default:
            break;
    }
}

// Check player proximity to active objectives
static void updateObjectives() {
    for (size_t i = 0; i < objectives.size(); ++i) {
        if (!objectives[i].completed) {
            float dist = glm::length(player.pos - objectives[i].targetPos);
            if (dist < 4.2f) {
                objectives[i].completed = true;
                crane.score += 250;
                std::cout << ">>> [OBJECTIVE COMPLETE] " << objectives[i].text << "! Score +250\n";
                // Advance current objective pointer
                for (size_t j = 0; j < objectives.size(); ++j) {
                    if (!objectives[j].completed) {
                        currentObjectiveIndex = static_cast<int>(j);
                        break;
                    }
                }
            }
        }
    }
}

static void updateWindowTitle(GLFWwindow* window) {
    std::string shadeStr = (currentShading == ShadingMode::Phong) ? "PHONG" : "GOURAUD";
    std::string camStr = "1ST-PERSON OPERATOR (POV)";
    if (currentCamMode == CameraMode::Free) camStr = "FREE";
    else if (currentCamMode == CameraMode::Overview) camStr = "OVERVIEW";
    else if (currentCamMode == CameraMode::CraneCab) camStr = "CRANE CAB";
    else if (currentCamMode == CameraMode::ShipFollow) camStr = "SHIP FOLLOW";
    else if (currentCamMode == CameraMode::LighthouseView) camStr = "LIGHTHOUSE LOOKOUT";
    else if (currentCamMode == CameraMode::LorryDrive) camStr = lorry.chaseCam ? "LORRY CHASE CAM" : "LORRY DRIVER (1ST PERSON)";

    std::string title = "[3D SEA PORT SIM] Shading: " + shadeStr +
                        " | Cam: " + camStr +
                        " | Score: " + std::to_string(crane.score) +
                        " | Sky [N]: " + (nightLights ? "NIGHT" : "DAY") +
                        " | RayTrace [F9]: " + (raytraceMode ? "ON" : "OFF");
    glfwSetWindowTitle(window, title.c_str());
}

// ---------------- INPUT CALLBACKS ----------------
static void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse) {
        lastMouseX = xpos;
        lastMouseY = ypos;
        firstMouse = false;
    }

    float xoffset = static_cast<float>(xpos - lastMouseX);
    float yoffset = static_cast<float>(lastMouseY - ypos); // Reversed since y-coordinates go from bottom to top
    lastMouseX = xpos;
    lastMouseY = ypos;

    if (currentCamMode == CameraMode::FPSOperator) {
        if (!player.cursorLocked) return;
        const float sensitivity = 0.12f;
        player.yaw += xoffset * sensitivity;
        player.pitch += yoffset * sensitivity;

        if (player.pitch > 88.0f) player.pitch = 88.0f;
        if (player.pitch < -88.0f) player.pitch = -88.0f;
    } else if (currentCamMode == CameraMode::CraneCab && player.inCraneCab) {
        if (!player.cursorLocked) return;
        if (crane.camView == 1) {
            // 3rd-person Crane Tactical Orbit Look!
            const float sensitivity = 0.22f;
            camera.yaw += xoffset * sensitivity;
            camera.pitch = std::clamp(camera.pitch + yoffset * sensitivity, 10.0f, 85.0f);
        } else {
            // 1st-person Crane Cabin Operator Look!
            const float sensitivity = 0.14f;
            camera.yaw += xoffset * sensitivity;
            camera.pitch = std::clamp(camera.pitch + yoffset * sensitivity, -85.0f, 65.0f);
        }
    } else if (currentCamMode == CameraMode::LorryDrive || lorry.playerDriving) {
        // Lorry Driver Look around cockpit!
        if (!player.cursorLocked) return;
        const float sensitivity = 0.14f;
        camera.yaw = std::clamp(camera.yaw + xoffset * sensitivity, -80.0f, 80.0f);
        camera.pitch = std::clamp(camera.pitch + yoffset * sensitivity, -35.0f, 30.0f);
    } else {
        // Orbit camera in Overview, Free, 3rd-person CraneCab, ShipFollow, Lighthouse
        if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS ||
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS ||
            player.cursorLocked) {
            const float sensitivity = 0.25f;
            camera.yaw += xoffset * sensitivity;
            camera.pitch = std::clamp(camera.pitch + yoffset * sensitivity, 5.0f, 85.0f);
        }
    }
}

static void scrollCallback(GLFWwindow*, double, double yoffset) {
    if (player.inCraneCab) {
        if (crane.camView == 0) {
            // Zoom FOV in 1st-person cabin (32 to 85 deg)
            crane.fov = std::clamp(crane.fov - static_cast<float>(yoffset) * 3.5f, 32.0f, 85.0f);
        } else if (crane.camView == 1) {
            // Zoom distance in 3rd-person overview (10 to 65m)
            crane.orbitDist = std::clamp(crane.orbitDist - static_cast<float>(yoffset) * 2.5f, 10.0f, 65.0f);
        }
    } else if (currentCamMode != CameraMode::FPSOperator) {
        camera.distance = std::clamp(camera.distance - static_cast<float>(yoffset) * 2.5f, 4.0f, 160.0f);
    }
}

static void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    const bool pressed = (action == GLFW_PRESS);

    switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;

        case GLFW_KEY_TAB:
            if (pressed) {
                player.cursorLocked = !player.cursorLocked;
                glfwSetInputMode(window, GLFW_CURSOR, player.cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
            }
            break;

        case GLFW_KEY_C:
            if (pressed) {
                if (lorry.playerDriving) {
                    // Switch directly from Lorry to Crane Operator Room!
                    lorry.playerDriving = false;
                    if (!lorry.hasCargo) {
                        lorry.pos = vec3(8.0f, 0.0f, 13.5f);
                        lorry.yaw = 0.0f;
                        lorry.speed = 0.0f;
                        lorry.steerAngle = 0.0f;
                    }
                    player.inCraneCab = true;
                    crane.manual = true;
                    crane.camView = 0;
                    currentCamMode = CameraMode::CraneCab;
                    camera.yaw = -90.0f + crane.slewingYaw;
                    camera.pitch = -30.0f;
                    player.cursorLocked = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    crane.toastMessage = "ENTERED CRANE CABIN FROM LORRY! TARGET: " + shipCargos[activeTargetCargoIndex].name;
                    crane.toastTimer = 5.0f;
                    std::cout << ">>> [OPERATOR] Switched from Lorry to Crane Operator Room!\n";
                } else if (player.inCraneCab) {
                    // Exit Crane Operator Room back to dock ground on foot
                    player.inCraneCab = false;
                    crane.manual = false;
                    currentCamMode = CameraMode::FPSOperator;
                    player.pos = vec3(8.0f, 0.0f, 15.5f); // Beside crane ladder and unloading bay
                    player.yaw = -90.0f;
                    player.pitch = 0.0f;
                    player.cursorLocked = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    crane.toastMessage = "EXITED CRANE OPERATOR ROOM -> WALKING ON DOCK";
                    crane.toastTimer = 3.5f;
                    std::cout << ">>> [OPERATOR] Exited crane operator room back to dock ground.\n";
                } else {
                    // Enter Crane Operator Room in 1st Person!
                    if (!lorry.hasCargo) {
                        lorry.pos = vec3(8.0f, 0.0f, 13.5f);
                        lorry.yaw = 0.0f;
                        lorry.speed = 0.0f;
                        lorry.steerAngle = 0.0f;
                    }
                    player.inCraneCab = true;
                    crane.manual = true;
                    crane.camView = 0;
                    currentCamMode = CameraMode::CraneCab;
                    camera.yaw = -90.0f + crane.slewingYaw;
                    camera.pitch = -30.0f; // Look down toward console, boom, and ship
                    player.cursorLocked = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    activeTargetCargoIndex = getNextActiveCargoIndex();
                    if (activeTargetCargoIndex == -1) {
                        for (auto& c : shipCargos) c.unloaded = false;
                        activeTargetCargoIndex = 0;
                    }
                    crane.toastMessage = "ENTERED CRANE CABIN! TARGET: " + shipCargos[activeTargetCargoIndex].name + " (GOLD BEACON)";
                    crane.toastTimer = 5.0f;
                    std::cout << ">>> [OPERATOR] Entered crane operator room in 1st-Person POV!\n";
                }
            }
            break;

        case GLFW_KEY_L:
            if (pressed) {
                if (lorry.playerDriving) {
                    // Exit Lorry back to walking on foot beside the truck
                    lorry.playerDriving = false;
                    currentCamMode = CameraMode::FPSOperator;
                    vec3 fwd = vec3(std::cos(glm::radians(lorry.yaw)), 0.0f, -std::sin(glm::radians(lorry.yaw)));
                    vec3 right = vec3(-fwd.z, 0.0f, fwd.x);
                    player.pos = lorry.pos + right * 2.2f;
                    player.yaw = lorry.yaw - 90.0f;
                    player.pitch = 0.0f;
                    player.cursorLocked = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    crane.toastMessage = "EXITED LORRY -> WALKING ON DOCK ROAD";
                    crane.toastTimer = 3.5f;
                    std::cout << ">>> [VEHICLE] Exited Cargo Lorry.\n";
                } else {
                    // Enter Cargo Lorry Driver Seat!
                    if (player.inCraneCab) {
                        player.inCraneCab = false;
                        crane.manual = false;
                    }
                    lorry.playerDriving = true;
                    lorry.chaseCam = false; // Directly seat in 1st-person driver seat!
                    currentCamMode = CameraMode::LorryDrive;
                    camera.yaw = 0.0f;
                    camera.pitch = -2.0f;
                    player.cursorLocked = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    crane.toastMessage = lorry.hasCargo
                        ? "ENTERED LORRY DRIVER SEAT! DELIVER TO FREIGHT DEPOT (CYAN BEACON)"
                        : "ENTERED LORRY DRIVER SEAT! DRIVE TO DOCK BAY OR EXPLORE ROAD";
                    crane.toastTimer = 5.0f;
                    std::cout << ">>> [VEHICLE] Entered Cargo Lorry driver seat!\n";
                }
            }
            break;

        case GLFW_KEY_V:
            if (pressed) {
                if (currentCamMode == CameraMode::LorryDrive || lorry.playerDriving) {
                    lorry.chaseCam = !lorry.chaseCam;
                    crane.toastMessage = lorry.chaseCam ? "CAM: 3RD-PERSON TRUCK CHASE CAM" : "CAM: 1ST-PERSON LORRY DRIVER SEAT";
                    crane.toastTimer = 3.5f;
                } else if (player.inCraneCab) {
                    crane.camView = (crane.camView + 1) % 3;
                    if (crane.camView == 0) {
                        crane.toastMessage = "CAM 1/3: 1ST-PERSON CABIN (SCROLL / Z,X TO ZOOM FOV)";
                    } else if (crane.camView == 1) {
                        crane.toastMessage = "CAM 2/3: 3RD-PERSON TACTICAL OVERVIEW (MOUSE ORBIT, SCROLL / Z,X DIST)";
                        camera.yaw = -90.0f;
                        camera.pitch = 35.0f;
                    } else {
                        crane.toastMessage = "CAM 3/3: PRECISION SPREADER DROP CAM (TOP-DOWN VIEW)";
                    }
                    crane.toastTimer = 3.5f;
                    std::cout << ">>> [CRANE] Camera view cycled to: " << crane.camView << "\n";
                }
            }
            break;

        case GLFW_KEY_E:
            if (player.inCraneCab) {
                crane.slewingYaw += 4.0f;
                camera.yaw += 4.0f;
            } else if (pressed) {
                float distToBridge = glm::length(player.pos - vec3(0.0f, 0.4f, 5.6f));
                float distToCrane = glm::length(player.pos - vec3(8.0f, 0.0f, 8.35f));

                if (distToBridge < 8.0f) {
                    autoBridge = false;
                    bridgeAngle = (bridgeAngle < 10.0f) ? 65.0f : 0.0f;
                    std::cout << ">>> [DRAWBRIDGE] Toggled by Operator: " << (bridgeAngle > 10.0f ? "OPEN" : "CLOSED") << "\n";
                } else if (distToCrane < 7.5f) {
                    player.inCraneCab = true;
                    crane.manual = true;
                    crane.camView = 0;
                    currentCamMode = CameraMode::CraneCab;
                    camera.yaw = -90.0f + crane.slewingYaw;
                    camera.pitch = -30.0f;
                    player.cursorLocked = true;
                    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                    activeTargetCargoIndex = getNextActiveCargoIndex();
                    if (activeTargetCargoIndex == -1) {
                        for (auto& c : shipCargos) c.unloaded = false;
                        activeTargetCargoIndex = 0;
                    }
                    crane.toastMessage = "TARGET: " + shipCargos[activeTargetCargoIndex].name;
                    crane.toastTimer = 5.0f;
                }
            }
            break;

        case GLFW_KEY_SPACE:
            if (pressed) {
                if (currentCamMode == CameraMode::FPSOperator && !player.inCraneCab && player.isGrounded) {
                    player.verticalVelocity = 6.2f;
                    player.isGrounded = false;
                } else if (player.inCraneCab || crane.manual) {
                    if (!crane.carrying) {
                        // Grab targeted cargo container from docked ship
                        if (crane.trolleyZ <= 2.2f && crane.hookY <= 5.2f) {
                            if (activeTargetCargoIndex >= 0 && activeTargetCargoIndex < static_cast<int>(shipCargos.size())) {
                                crane.carrying = true;
                                crane.cargoIndex = activeTargetCargoIndex;
                                shipCargos[activeTargetCargoIndex].unloaded = true;
                                crane.toastMessage = "CARGO SECURED! MOVE TROLLEY [S] TO LOAD ONTO LORRY (GREEN BEACON)";
                                crane.toastTimer = 5.0f;
                                std::cout << ">>> [CRANE] " << shipCargos[activeTargetCargoIndex].name << " HOISTED FROM SHIP!\n";
                            }
                        } else if (crane.trolleyZ > 2.2f) {
                            crane.toastMessage = "MOVE TROLLEY OUT OVER SHIP [W] (FOLLOW GOLD TARGET BEACON)";
                            crane.toastTimer = 3.0f;
                        } else {
                            crane.toastMessage = "LOWER HOIST HOOK [F] CLOSER TO CARGO (Y <= 5.2M) TO HOIST";
                            crane.toastTimer = 3.0f;
                        }
                    } else {
                        // Deposit cargo directly onto the waiting Cargo Lorry in the unloading bay!
                        if (crane.trolleyZ >= 11.5f && crane.trolleyZ <= 15.0f && crane.hookY <= 5.2f) {
                            if (lorry.hasCargo) {
                                crane.toastMessage = "BAY OCCUPIED! PRESS [L] TO DRIVE LOADED LORRY TO FREIGHT DEPOT!";
                                crane.toastTimer = 5.0f;
                                std::cout << ">>> [CRANE] UNLOAD STOPPED: Lorry is already loaded! Deliver to depot first.\n";
                                break;
                            }
                            if (glm::length(lorry.pos - vec3(8.0f, 0.0f, 13.5f)) > 4.5f) {
                                crane.toastMessage = "NO LORRY AT UNLOAD BAY! RETURN LORRY OR ENTER CRANE CABIN [C]";
                                crane.toastTimer = 5.0f;
                                break;
                            }

                            crane.carrying = false;
                            crane.score += 500;
                            crane.unloadedCount++;

                            // Lock container directly onto the waiting Cargo Lorry!
                            lorry.hasCargo = true;
                            lorry.cargoIndex = crane.cargoIndex;

                            int nextTarget = getNextActiveCargoIndex();
                            if (nextTarget != -1) {
                                activeTargetCargoIndex = nextTarget;
                                crane.toastMessage = "+500 PTS! CARGO LOADED ON LORRY! PRESS [L] TO DRIVE TO FREIGHT DEPOT (+1000 PTS)!";
                            } else {
                                crane.score += 2000;
                                crane.toastMessage = "ALL 6 CONTAINERS UNLOADED! PRESS [L] TO DELIVER LAST LORRY (+1000 PTS)!";
                                for (auto& c : shipCargos) c.unloaded = false;
                                activeTargetCargoIndex = 0;
                            }
                            crane.toastTimer = 6.0f;
                            std::cout << ">>> [CRANE] CARGO LOADED ONTO LORRY! SCORE: " << crane.score << "\n";
                            if (!objectives.empty() && !objectives[0].completed) {
                                objectives[0].completed = true;
                                currentObjectiveIndex = 1;
                            }
                        } else if (crane.trolleyZ < 11.5f) {
                            crane.toastMessage = "MOVE TROLLEY OVER LORRY IN UNLOAD BAY [S] (FOLLOW GREEN BEACON)";
                            crane.toastTimer = 3.0f;
                        } else {
                            crane.toastMessage = "LOWER HOOK [F] CLOSER TO LORRY TRAILER (Y <= 5.2M) TO LOAD";
                            crane.toastTimer = 3.0f;
                        }
                    }
                }
            }
            break;

        case GLFW_KEY_N:
            if (pressed) {
                nightLights = !nightLights;
                std::cout << ">>> Day/Night Cycle toggled: " << (nightLights ? "NIGHT MODE" : "DAY MODE") << "\n";
            }
            break;

        // ---------- SHADING MODES (GOURAUD VS PHONG) ----------
        case GLFW_KEY_F1:
            if (pressed) {
                currentShading = ShadingMode::Gouraud;
                std::cout << ">>> Switched to GOURAUD Shading (Per-vertex lighting)\n";
            }
            break;

        case GLFW_KEY_F2:
            if (pressed) {
                currentShading = ShadingMode::Phong;
                std::cout << ">>> Switched to PHONG Shading (Per-fragment specular lighting)\n";
            }
            break;

        // ---------- RAY TRACING SHOWCASE (REBOUND TO F9 SO R DOES NOT CONFLICT WITH CRANE) ----------
        case GLFW_KEY_F9:
            if (pressed) {
                raytraceMode = !raytraceMode;
                std::cout << ">>> Ray Tracing Showcase Mode [F9]: " << (raytraceMode ? "ACTIVE" : "INACTIVE") << "\n";
            }
            break;

        // ---------- CAMERA MODES ----------
        case GLFW_KEY_F8:
        case GLFW_KEY_1:
            if (pressed) {
                player.inCraneCab = false;
                crane.manual = false;
                currentCamMode = CameraMode::FPSOperator;
                player.cursorLocked = true;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                std::cout << ">>> Camera: 1ST-PERSON OPERATOR (Walking on foot)\n";
            }
            break;
        case GLFW_KEY_F3:
            if (pressed) {
                currentCamMode = CameraMode::Free;
                camera.distance = 36.0f;
                camera.pitch = 28.0f;
                camera.target = {0.0f, 1.5f, 2.0f};
                player.cursorLocked = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                std::cout << ">>> Camera: FREE MODE (Use Arrow keys, J/K/I/L, W/S zoom)\n";
            }
            break;
        case GLFW_KEY_F4:
            if (pressed) {
                currentCamMode = CameraMode::Overview;
                camera.distance = 46.0f;
                camera.pitch = 32.0f;
                camera.yaw = -55.0f;
                camera.target = {0.0f, 1.5f, 2.0f};
                player.cursorLocked = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                std::cout << ">>> Camera: PORT OVERVIEW (Cinematic Wide Angle)\n";
            }
            break;
        case GLFW_KEY_F5:
            if (pressed) {
                currentCamMode = CameraMode::CraneCab;
                camera.distance = 18.0f;
                camera.pitch = 45.0f;
                camera.yaw = -90.0f + crane.slewingYaw;
                camera.target = vec3(8.0f, crane.hookY, crane.trolleyZ);
                player.cursorLocked = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                std::cout << ">>> Camera: CRANE CABIN (Rotate: Arrow keys or Mouse Drag, Zoom: W/S or Scroll)\n";
            }
            break;
        case GLFW_KEY_F6:
            if (pressed) {
                currentCamMode = CameraMode::ShipFollow;
                camera.distance = 22.0f;
                camera.pitch = 24.0f;
                camera.yaw = -70.0f;
                if (!ships.empty()) camera.target = vec3(ships[0].x, 1.2f, ships[0].z);
                player.cursorLocked = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                std::cout << ">>> Camera: SHIP FOLLOW (Rotate: Arrow keys or Mouse Drag, Zoom: W/S or Scroll)\n";
            }
            break;
        case GLFW_KEY_F7:
            if (pressed) {
                currentCamMode = CameraMode::LighthouseView;
                camera.distance = 36.0f;
                camera.pitch = 26.0f;
                camera.yaw = -140.0f;
                camera.target = vec3(4.0f, 1.5f, 0.0f);
                player.cursorLocked = false;
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                std::cout << ">>> Camera: LIGHTHOUSE LOOKOUT (Rotate: Arrow keys or Mouse Drag, Zoom: W/S or Scroll)\n";
            }
            break;

        // ---------- CRANE CONTROLS ----------
        case GLFW_KEY_G:
            if (pressed) {
                crane.manual = false;
                crane.autoState = Crane::AutoState::ToShip;
                crane.autoTimer = 0.0f;
                std::cout << ">>> [CRANE] Automated Unload Cycle STARTED!\n";
            }
            break;
        case GLFW_KEY_T:
            if (pressed) {
                crane.manual = !crane.manual;
                crane.autoState = Crane::AutoState::Idle;
                std::cout << ">>> [CRANE] Control mode: " << (crane.manual ? "MANUAL (Use Q/E, [/], U/D, P)" : "AUTO") << "\n";
            }
            break;
        case GLFW_KEY_P:
            if (pressed) {
                crane.carrying = !crane.carrying;
                if (crane.carrying) {
                    std::cout << ">>> [CRANE] CARGO ATTACHED!\n";
                } else {
                    crane.score += 100;
                    crane.cargoIndex = (crane.cargoIndex + 1) % 6;
                    std::cout << ">>> [CRANE] CARGO RELEASED! SCORE: " << crane.score << "\n";
                }
            }
            break;

        case GLFW_KEY_Q:
            if (crane.manual) crane.slewingYaw -= 4.0f;
            break;

        // ---------- BRIDGE CONTROLS ----------
        case GLFW_KEY_A:
            if (pressed) {
                autoBridge = true;
                std::cout << ">>> Drawbridge: AUTO-OPEN for ships\n";
            }
            break;
        case GLFW_KEY_B:
            if (pressed) autoBridge = !autoBridge;
            break;
        case GLFW_KEY_M:
            if (pressed) {
                autoBridge = false;
                std::cout << ">>> Drawbridge: MANUAL (Hold O to open, C to close)\n";
            }
            break;

        default:
            break;
    }

    updateWindowTitle(window);
}

static void processContinuousInput(GLFWwindow* window, float dt) {
    if (player.inCraneCab) {
        // --- 1ST-PERSON CRANE CABIN OPERATOR CONTROLS ---
        const float trolleyDelta = crane.trolleySpeed * dt;
        const float hookDelta = crane.hookSpeed * dt;
        const float slewSpeed = 32.0f * dt;

        // W / Up Arrow: Trolley toward ship (lower Z)
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
            crane.trolleyZ = std::max(-1.5f, crane.trolleyZ - trolleyDelta);
        }
        // S / Down Arrow: Trolley toward dock unloading bay (higher Z)
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
            crane.trolleyZ = std::min(15.5f, crane.trolleyZ + trolleyDelta);
        }

        // A / Q / Left Arrow: Slew Turret Left
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
            crane.slewingYaw -= slewSpeed;
            camera.yaw -= slewSpeed;
        }
        // D / E / Right Arrow: Slew Turret Right
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
            crane.slewingYaw += slewSpeed;
            camera.yaw += slewSpeed;
        }

        // R / U / Page Up: Hoist hook up
        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS) {
            crane.hookY = std::min(12.2f, crane.hookY + hookDelta);
        }
        // F / Page Down: Lower hook down
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS) {
            crane.hookY = std::max(1.8f, crane.hookY - hookDelta);
        }

        // Z / X: Zoom In (narrow FOV / closer orbit) / Zoom Out (wide FOV / farther orbit)
        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
            if (crane.camView == 0) crane.fov = std::max(32.0f, crane.fov - 28.0f * dt);
            else if (crane.camView == 1) crane.orbitDist = std::max(10.0f, crane.orbitDist - 18.0f * dt);
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
            if (crane.camView == 0) crane.fov = std::min(85.0f, crane.fov + 28.0f * dt);
            else if (crane.camView == 1) crane.orbitDist = std::min(65.0f, crane.orbitDist + 18.0f * dt);
        }
        return;
    }

    if (currentCamMode == CameraMode::LorryDrive || lorry.playerDriving) {
        // --- DRIVABLE CARGO LORRY CONTROLS & DYNAMICS ---
        const float maxForwardSpeed = lorry.maxSpeed; // 16.0 m/s (~58 km/h)
        const float maxReverseSpeed = -6.0f;          // -6.0 m/s (~22 km/h)
        const float steerRate = 75.0f * dt;           // deg/s
        const float maxSteer = 30.0f;                 // max wheel lock deg

        // Throttle & Brake (W / S / Up / Down)
        bool accelerating = (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS);
        bool brakingOrReverse = (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS);
        bool handbrake = (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS);

        if (accelerating) {
            if (lorry.speed < 0.0f) {
                // Brake from reverse
                lorry.speed = std::min(0.0f, lorry.speed + lorry.brake * dt);
            } else {
                lorry.speed = std::min(maxForwardSpeed, lorry.speed + lorry.accel * dt);
            }
        } else if (brakingOrReverse) {
            if (lorry.speed > 0.3f) {
                // Brake forward motion
                lorry.speed = std::max(0.0f, lorry.speed - lorry.brake * dt);
            } else {
                // Reverse
                lorry.speed = std::max(maxReverseSpeed, lorry.speed - lorry.accel * 0.65f * dt);
            }
        } else {
            // Natural rolling resistance / friction
            if (std::abs(lorry.speed) > 0.05f) {
                float friction = 3.8f * dt;
                if (lorry.speed > 0.0f) lorry.speed = std::max(0.0f, lorry.speed - friction);
                else lorry.speed = std::min(0.0f, lorry.speed + friction);
            } else {
                lorry.speed = 0.0f;
            }
        }

        // Handbrake (Space)
        if (handbrake) {
            float hbFriction = 24.0f * dt;
            if (lorry.speed > 0.0f) lorry.speed = std::max(0.0f, lorry.speed - hbFriction);
            else lorry.speed = std::min(0.0f, lorry.speed + hbFriction);
        }

        // Steering (A / D / Left / Right)
        bool steerLeft = (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS);
        bool steerRight = (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS);

        if (steerLeft) {
            lorry.steerAngle = std::max(-maxSteer, lorry.steerAngle - steerRate);
        } else if (steerRight) {
            lorry.steerAngle = std::min(maxSteer, lorry.steerAngle + steerRate);
        } else {
            // Auto-center steering wheel
            if (std::abs(lorry.steerAngle) > 0.5f) {
                float returnRate = 85.0f * dt;
                if (lorry.steerAngle > 0.0f) lorry.steerAngle = std::max(0.0f, lorry.steerAngle - returnRate);
                else lorry.steerAngle = std::min(0.0f, lorry.steerAngle + returnRate);
            } else {
                lorry.steerAngle = 0.0f;
            }
        }

        // Kinematics: Update yaw & position
        if (std::abs(lorry.speed) > 0.05f) {
            float wheelbase = 3.8f;
            float turnRate = (lorry.speed / wheelbase) * std::tan(glm::radians(lorry.steerAngle));
            lorry.yaw -= glm::degrees(turnRate) * dt;

            vec3 fwd = vec3(std::cos(glm::radians(lorry.yaw)), 0.0f, -std::sin(glm::radians(lorry.yaw)));
            lorry.pos += fwd * lorry.speed * dt;
        }

        // Keep lorry within dock roadway bounds (prevent driving into canal water)
        lorry.pos.z = glm::clamp(lorry.pos.z, 7.2f, 48.0f);
        lorry.pos.x = glm::clamp(lorry.pos.x, -48.0f, 66.0f);
        lorry.pos.y = 0.0f;

        // Keep player position synced with lorry
        player.pos = lorry.pos;

        // Check Delivery Goal at Freight Logistics Depot!
        float distToDepot = glm::length(lorry.pos - freightDepotPos);
        if (lorry.hasCargo && distToDepot < 6.5f) {
            lorry.hasCargo = false;
            lorry.deliveriesCompleted++;
            crane.score += 1000;
            crane.toastMessage = "MISSION ACCOMPLISHED! +1000 PTS! CARGO DELIVERED TO DEPOT! DOCK CLEARED!";
            crane.toastTimer = 7.0f;
            std::cout << ">>> [LOGISTICS] CARGO DELIVERED TO FREIGHT DEPOT! TOTAL DELIVERIES: "
                      << lorry.deliveriesCompleted << " | SCORE: " << crane.score << "\n";
        }

        return;
    }

    if (currentCamMode == CameraMode::FPSOperator) {
        // Player Walking / Sprinting
        float speed = player.speed;
        if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) speed *= player.sprintMultiplier;

        vec3 fwd = vec3(std::cos(glm::radians(player.yaw)), 0.0f, std::sin(glm::radians(player.yaw)));
        vec3 right = vec3(-fwd.z, 0.0f, fwd.x);

        vec3 moveDir(0.0f);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) moveDir += fwd;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) moveDir -= fwd;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) moveDir -= right;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) moveDir += right;

        if (glm::length(moveDir) > 0.001f) {
            moveDir = glm::normalize(moveDir);
            player.pos += moveDir * speed * dt;
            player.headBob += speed * dt * 7.5f;
        }

        // Gravity & Jump Physics
        player.verticalVelocity -= 18.0f * dt;
        player.pos.y += player.verticalVelocity * dt;

        // Ground Height Detection & Collision
        float targetGroundY = 0.0f; // Default dock slab & road level

        // Check if on Drawbridge
        if (std::abs(player.pos.x) < 2.3f && std::abs(player.pos.z) < 5.4f) {
            if (bridgeAngle < 2.5f) {
                targetGroundY = 0.4f; // Safe passage across lowered bridge
            } else {
                targetGroundY = -0.5f; // Bridge open, player falls into water
            }
        }
        // Check if in Canal Water
        else if (std::abs(player.pos.z) < 5.3f) {
            targetGroundY = -0.5f + 0.08f * std::sin(simTime * 3.0f + player.pos.x * 0.5f); // Swimming in canal
        }
        // Watchtower Ladder Climb (X = -45, Z = 24)
        float distToTowerLadder = glm::length(vec2(player.pos.x - (-45.0f), player.pos.z - 24.0f));
        if (distToTowerLadder < 1.8f) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && player.pos.y < 8.5f) {
                player.pos.y += 4.5f * dt;
                player.verticalVelocity = 0.0f;
            }
            if (player.pos.y >= 8.4f) targetGroundY = 8.5f; // Platform
        }

        // Crane Gantry Ladder Climb (X = 8.0, Z = 8.35)
        float distToCraneLadder = glm::length(vec2(player.pos.x - 8.0f, player.pos.z - 8.35f));
        if (distToCraneLadder < 2.0f) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && player.pos.y < 6.2f) {
                player.pos.y += 4.5f * dt;
                player.verticalVelocity = 0.0f;
            }
            if (player.pos.y >= 6.1f) targetGroundY = 6.2f; // Crane cabin platform
        }

        // Snap to ground
        if (player.pos.y <= targetGroundY) {
            player.pos.y = targetGroundY;
            player.verticalVelocity = 0.0f;
            player.isGrounded = true;
        }

        updateObjectives();

    } else {
        // ANY 3rd-person camera mode: Free, Overview, CraneCab, ShipFollow, LighthouseView
        const float cameraSpeed = 45.0f * dt;
        const float zoomSpeed = 28.0f * dt;
        const float panSpeed = 22.0f * dt;

        // Orbit Yaw (Turn Camera Left / Right): Left/Right Arrows OR J/L
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
            camera.yaw -= cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            camera.yaw += cameraSpeed;

        // Orbit Pitch (Tilt Camera Up / Down): Up/Down Arrows OR I/K
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
            camera.pitch = std::min(85.0f, camera.pitch + cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
            camera.pitch = std::max(5.0f, camera.pitch - cameraSpeed);

        // Zoom Camera In / Out (Distance): PageUp / PageDown
        if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
            camera.distance = std::max(4.0f, camera.distance - zoomSpeed);
        if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
            camera.distance = std::min(160.0f, camera.distance + zoomSpeed);

        // Also allow W/S to zoom if in Overview/Free/Lighthouse
        if (currentCamMode == CameraMode::Overview || currentCamMode == CameraMode::Free || currentCamMode == CameraMode::LighthouseView) {
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                camera.distance = std::max(4.0f, camera.distance - zoomSpeed);
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera.distance = std::min(160.0f, camera.distance + zoomSpeed);

            // Pan Target across port with A / D
            vec3 fwd = vec3(std::cos(glm::radians(camera.yaw)), 0.0f, std::sin(glm::radians(camera.yaw)));
            vec3 right = vec3(-fwd.z, 0.0f, fwd.x);
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) camera.target -= right * panSpeed;
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) camera.target += right * panSpeed;
        }
    }

    // Manual Bridge Open/Close
    if (!autoBridge) {
        const float bridgeSpeed = 28.0f * dt;
        if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) bridgeAngle = std::min(65.0f, bridgeAngle + bridgeSpeed);
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) bridgeAngle = std::max(0.0f, bridgeAngle - bridgeSpeed);
    }

    // Manual Crane Operation
    if (crane.manual) {
        const float trolleyDelta = crane.trolleySpeed * dt;
        const float hookDelta = crane.hookSpeed * dt;

        if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
            crane.trolleyZ = std::max(-1.5f, crane.trolleyZ - trolleyDelta);
        if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
            crane.trolleyZ = std::min(15.5f, crane.trolleyZ + trolleyDelta);

        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
            crane.hookY = std::min(6.8f, crane.hookY + hookDelta);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
            crane.hookY = std::max(1.2f, crane.hookY - hookDelta);
    }
}

// ---------------- MAIN SCENE RENDERER ----------------
static void renderScene(int width, int height) {
    if (raytraceMode) {
        glDisable(GL_DEPTH_TEST);
        raytraceShader.use();
        raytraceShader.setVec2("uResolution", vec2(static_cast<float>(width), static_cast<float>(height)));
        raytraceShader.setFloat("uTime", simTime);
        raytraceShader.setBool("uNight", nightLights);
        renderQuad();
        glEnable(GL_DEPTH_TEST);
        return;
    }

    vec3 camPos;
    mat4 view;

    if (currentCamMode == CameraMode::FPSOperator) {
        // First-Person Walking Operator POV with subtle head-bobbing
        float bobOffset = (player.isGrounded && glm::length(vec2(player.verticalVelocity, 0)) < 0.1f)
                        ? std::sin(player.headBob) * 0.05f : 0.0f;
        camPos = player.pos + vec3(0.0f, player.eyeHeight + bobOffset, 0.0f);
        vec3 front;
        front.x = std::cos(glm::radians(player.yaw)) * std::cos(glm::radians(player.pitch));
        front.y = std::sin(glm::radians(player.pitch));
        front.z = std::sin(glm::radians(player.yaw)) * std::cos(glm::radians(player.pitch));
        view = glm::lookAt(camPos, camPos + glm::normalize(front), vec3(0, 1, 0));
    } else if (currentCamMode == CameraMode::CraneCab && player.inCraneCab) {
        if (crane.camView == 1) {
            // 3rd-Person Tactical Orbit Overview (Whole Crane & Port Operation)
            vec3 orbitTarget = vec3(8.0f, 7.5f, 7.0f);
            float radYaw = glm::radians(camera.yaw);
            float radPitch = glm::radians(camera.pitch);
            camPos = orbitTarget + vec3(
                std::cos(radPitch) * std::cos(radYaw),
                std::sin(radPitch),
                std::cos(radPitch) * std::sin(radYaw)
            ) * crane.orbitDist;
            view = glm::lookAt(camPos, orbitTarget, vec3(0.0f, 1.0f, 0.0f));
        } else if (crane.camView == 2) {
            // Direct Top-Down Precision Spreader Drop Cam (Classic Port Sim Experience!)
            camPos = vec3(8.0f, std::min(13.6f, crane.hookY + 3.8f), crane.trolleyZ);
            view = glm::lookAt(camPos, vec3(8.0f, 0.0f, crane.trolleyZ), vec3(0, 0, -1));
        } else {
            // First-Person Crane Operator Inside 360-Degree Transparent Panoramic Cockpit!
            mat4 rotMat = glm::rotate(mat4(1.0f), glm::radians(crane.slewingYaw), vec3(0, 1, 0));
            vec3 rotatedCab = vec3(rotMat * vec4(0.0f, 0.95f, -1.8f, 1.0f));
            camPos = vec3(8.0f, 11.2f, 8.35f) + rotatedCab;
            vec3 front;
            front.x = std::cos(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch));
            front.y = std::sin(glm::radians(camera.pitch));
            front.z = std::sin(glm::radians(camera.yaw)) * std::cos(glm::radians(camera.pitch));
            view = glm::lookAt(camPos, camPos + glm::normalize(front), vec3(0, 1, 0));
        }
    } else if (currentCamMode == CameraMode::LorryDrive || lorry.playerDriving) {
        // --- DRIVABLE CARGO LORRY CAMERA (1ST-PERSON CAB POV & 3RD-PERSON CHASE CAM) ---
        vec3 lorryFwd = vec3(std::cos(glm::radians(lorry.yaw)), 0.0f, -std::sin(glm::radians(lorry.yaw)));
        vec3 lorryRight = vec3(-lorryFwd.z, 0.0f, lorryFwd.x);

        if (lorry.chaseCam) {
            // 3rd-Person Dynamic Chase Camera behind truck
            camPos = lorry.pos - lorryFwd * 7.6f + vec3(0.0f, 3.5f, 0.0f);
            vec3 target = lorry.pos + lorryFwd * 3.5f + vec3(0.0f, 1.4f, 0.0f);
            view = glm::lookAt(camPos, target, vec3(0.0f, 1.0f, 0.0f));
        } else {
            // 1st-Person Driver Seat POV inside lorry cab!
            // Sitting in driver seat with realistic dashboard & windshield view
            camPos = lorry.pos + lorryFwd * 1.60f + lorryRight * 0.45f + vec3(0.0f, 1.58f, 0.0f);
            float lookYaw = lorry.yaw + camera.yaw;
            float lookPitch = camera.pitch;
            vec3 lookDir;
            lookDir.x = std::cos(glm::radians(lookYaw)) * std::cos(glm::radians(lookPitch));
            lookDir.y = std::sin(glm::radians(lookPitch));
            lookDir.z = -std::sin(glm::radians(lookYaw)) * std::cos(glm::radians(lookPitch));
            view = glm::lookAt(camPos, camPos + glm::normalize(lookDir), vec3(0.0f, 1.0f, 0.0f));
        }
    } else {
        if (currentCamMode == CameraMode::CraneCab) {
            camera.target = vec3(8.0f, crane.hookY, crane.trolleyZ);
        } else if (currentCamMode == CameraMode::ShipFollow && !ships.empty()) {
            camera.target = vec3(ships[0].x, 0.8f, ships[0].z);
        }
        camPos = camera.position();
        view = camera.view();
    }

    float activeFov = (currentCamMode == CameraMode::CraneCab && player.inCraneCab && crane.camView == 0)
        ? crane.fov
        : 52.0f;
    mat4 projection = glm::perspective(glm::radians(activeFov), static_cast<float>(width) / std::max(1, height), 0.1f, 320.0f);

    // 1. Render Photorealistic Procedural Sky Dome & Clouds
    drawSkyDome(view, projection);

    Shader& s = activeShader();
    s.use();
    s.setMat4("uView", view);
    s.setMat4("uProjection", projection);
    s.setVec3("uViewPos", camPos);
    s.setFloat("uTime", simTime);
    s.setBool("uIsWater", false);
    s.setBool("uIsEmissive", false);

    // Sun / Moon Directional Light (High Overhead for Optimal Shading Mathematics)
    vec3 sunDirection = nightLights ? vec3(0.35f, -0.82f, -0.45f) : vec3(-0.30f, -0.88f, 0.35f);
    vec3 sunLightColor = nightLights ? vec3(0.35f, 0.42f, 0.65f) : vec3(1.35f, 1.30f, 1.18f);
    vec3 ambientLightColor = nightLights ? vec3(0.08f, 0.10f, 0.18f) : vec3(0.32f, 0.34f, 0.38f);

    s.setVec3("uSunDir", sunDirection);
    s.setVec3("uSunColor", sunLightColor);
    s.setVec3("uAmbientColor", ambientLightColor);

    // Main Point Light
    s.setBool("uPointLightEnabled", true);
    s.setVec3("uPointPos", vec3(8.0f, 5.5f, 6.8f));
    s.setVec3("uPointColor", nightLights ? vec3(1.3f, 0.95f, 0.45f) : vec3(0.6f, 0.58f, 0.52f));

    if (currentShading == ShadingMode::Phong) {
        s.setBool("uSpotLightEnabled", nightLights);
        s.setVec3("uSpotPos", vec3(8.0f, 7.0f, crane.trolleyZ));
        s.setVec3("uSpotDir", vec3(0.0f, -1.0f, 0.0f));
        s.setVec3("uSpotColor", vec3(1.4f, 1.25f, 0.95f));
        s.setFloat("uSpotCutoff", std::cos(glm::radians(32.0f)));

        s.setBool("uMultiPointEnabled", nightLights);
        vec3 lampPositions[7];
        vec3 lampColors[7];
        for (int i = 0; i < 7; ++i) {
            float lx = -18.0f + i * 6.0f;
            lampPositions[i] = vec3(lx, 4.85f, 6.05f);
            lampColors[i] = vec3(1.1f, 0.85f, 0.40f);
        }
        glUniform3fv(s.loc("uPointLights"), 7, glm::value_ptr(lampPositions[0]));
        glUniform3fv(s.loc("uPointLightColors"), 7, glm::value_ptr(lampColors[0]));
    }

    // 3D Sun & Moon Celestial Bodies (Directional Light Source Demonstration)
    drawCelestialBodies(camPos);

    // Render 3D World Geometry
    drawSurroundingWorld();
    drawQuays();
    drawWaterAndCanal();
    drawBridge();
    drawLighthouse();
    drawContainerYard();
    drawCrane();
    drawDockedContainerShip();

    // Render Playable Lorry and Freight Logistics Depot!
    drawPlayableLorry();
    drawFreightDepot();

    // Render 3D NPC Human Operators (Port Personnel)
    drawHumanNPC({0.0f, 0.0f, 23.5f}, 180.0f, false, 0.0f, {0.95f, 0.35f, 0.15f}); // Security Gate Guard
    drawHumanNPC({-45.0f, 8.5f, 24.0f}, 45.0f, false, 0.0f, {0.90f, 0.85f, 0.15f}); // Watchtower Sentry
    drawHumanNPC({11.0f, 0.0f, 8.5f}, -90.0f, false, 0.0f, {0.15f, 0.85f, 0.35f}); // Crane Technician

    // Patrolling Dock Foreman (Walking loop along the dock road)
    float patrolX = std::fmod(simTime * 2.5f, 40.0f) - 20.0f;
    drawHumanNPC({patrolX, 0.0f, 16.5f}, 90.0f, true, 0.0f, {0.95f, 0.85f, 0.15f});

    // Active Objective Laser Waypoint Beacon (Direct inspiration from reference images!)
    if (currentObjectiveIndex < static_cast<int>(objectives.size()) && !objectives[currentObjectiveIndex].completed) {
        drawLaserWaypointBeacon(objectives[currentObjectiveIndex].targetPos);
    }

    // Vehicles & moving traffic
    drawTruck({24.0f, 0.0f, 18.5f}, 1.0f, true);
    drawTruck({-16.0f, 0.0f, 18.5f}, 3.0f, false);

    for (const Ship& ship : ships) {
        if (ship.active) drawMovingShip(ship);
    }

    // Render 2D Management HUD (Compass, Objectives Checklist, Crosshair)
    renderHUD(width, height);
}

static void initShips() {
    ships = {
        // --- CANAL PASSAGE SHIPS (Navigate through port channel under drawbridge) ---
        // 0. Canal Container Feeder (Eastbound)
        {-28.0f, -2.0f, 2.0f, 0.0f, 1.0f, {0.18f, 0.42f, 0.82f}, ShipType::ContainerFeeder, true, false, 0.0f},
        // 1. Canal Escort Tugboat (Westbound)
        { 24.0f,  1.2f, 1.6f, 180.0f, 0.85f, {0.85f, 0.28f, 0.15f}, ShipType::Tugboat, true, true, 1.5f},

        // --- NEAR OFFSHORE SHIPS (Visible directly behind the lighthouse & North quay!) ---
        // 2. Harbor Pilot Boat (Fast escort approaching port mouth)
        {-45.0f, -36.0f, 2.6f, 0.0f, 0.70f, {0.15f, 0.45f, 0.85f}, ShipType::PilotBoat, true, false, 0.8f},
        // 3. Inbound Feeder Container Ship (Sailing toward harbor)
        { 65.0f, -48.0f, 1.4f, 180.0f, 0.95f, {0.14f, 0.65f, 0.38f}, ShipType::ContainerFeeder, true, false, 2.2f},
        // 4. Harbor Service Tugboat
        {-90.0f, -42.0f, 1.5f, 0.0f, 0.80f, {0.92f, 0.45f, 0.12f}, ShipType::Tugboat, true, true, 3.1f},

        // --- MID-OFFSHORE MARITIME LANES ---
        // 5. Coastal Oil/Chemical Tanker (Cruising East)
        {-85.0f, -72.0f, 1.2f, 0.0f, 1.15f, {0.20f, 0.22f, 0.25f}, ShipType::CoastalTanker, true, false, 3.4f},
        // 6. Maersk Blue Feeder Vessel (Cruising West)
        {110.0f, -66.0f, 1.35f, 180.0f, 1.05f, {0.22f, 0.52f, 0.88f}, ShipType::ContainerFeeder, true, false, 1.1f},
        // 7. Orange Sea Tugboat
        { 35.0f, -84.0f, 1.1f, 180.0f, 0.90f, {0.88f, 0.35f, 0.15f}, ShipType::Tugboat, true, true, 4.1f},

        // --- OUTER SHIPPING CHANNEL ---
        // 8. Large Ocean Bulk Carrier (Cruising West)
        {130.0f, -118.0f, 0.95f, 180.0f, 1.35f, {0.55f, 0.22f, 0.18f}, ShipType::BulkCarrier, true, false, 5.0f},
        // 9. Inbound Evergreen Container Vessel (Cruising East)
        {-140.0f, -135.0f, 1.05f, 0.0f, 1.25f, {0.16f, 0.58f, 0.32f}, ShipType::ContainerFeeder, true, false, 2.8f},
        // 10. Coast Guard Patrol Cutter
        {  15.0f, -98.0f, 2.2f, 0.0f, 0.75f, {0.15f, 0.35f, 0.75f}, ShipType::PilotBoat, true, false, 1.7f},

        // --- DEEP OCEAN HORIZON ---
        // 11. Massive Bulk Freighter on Horizon (Cruising East)
        {-180.0f, -180.0f, 0.80f, 0.0f, 1.55f, {0.45f, 0.20f, 0.18f}, ShipType::BulkCarrier, true, false, 0.5f},
        // 12. Distant Supertanker on Horizon (Cruising West)
        { 160.0f, -215.0f, 0.70f, 180.0f, 1.65f, {0.18f, 0.20f, 0.24f}, ShipType::CoastalTanker, true, false, 3.9f}
    };
}

int main() {
    if (!glfwInit()) {
        std::cerr << "GLFW initialization failed.\n";
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1440, 880, "3D Sea Port Simulator & Management Showcase", nullptr, nullptr);
    if (!window) {
        std::cerr << "GLFW window creation failed.\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Start in 1st-person FPS operator mode!

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD initialization failed.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "Renderer:       " << glGetString(GL_RENDERER) << "\n";

    std::cout << "\n==================================================================================\n";
    std::cout << "         🚢 3D SEA PORT MANAGEMENT & 1ST-PERSON OPERATOR SHOWCASE 🚢\n";
    std::cout << "==================================================================================\n";
    std::cout << "  [W, A, S, D] Move Operator / Steer Lorry | [Mouse] Look Around Pitch/Yaw\n";
    std::cout << "  [C]  Enter / Exit Crane Operator Cab     | [L]  Enter / Exit Cargo Lorry Driver\n";
    std::cout << "  [V]  Lorry: 1st-Person POV / Chase Cam   | Crane: Cab POV / Spreader Drop Cam\n";
    std::cout << "  [Space] Crane: Grab / Release Cargo      | Lorry: Handbrake | Player: Jump\n";
    std::cout << "  [F1] Gouraud Shading  | [F2] Phong Shading | [N] Day/Night | [F9] Ray Tracing\n";
    std::cout << "  [F8/1] 1st-Person Operator POV | [F4] Overview Camera | [F5] Crane Cab Camera\n";
    std::cout << "==================================================================================\n\n";

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    try {
        phongShader.load("shaders/phong.vert", "shaders/phong.frag");
        gouraudShader.load("shaders/gouraud.vert", "shaders/gouraud.frag");
        raytraceShader.load("shaders/quad.vert", "shaders/raytracer.frag");
        skyShader.load("shaders/sky.vert", "shaders/sky.frag");
        hudShader.load("shaders/hud.vert", "shaders/hud.frag");

        cubeMesh = makeCube();
        cylinderMesh = makeCylinder(32);
        ringMesh = makeRing(32);
        sphereMesh = makeSphere(18, 28, 1.0f);
        coneMesh = makeCone(32);
        skyDomeMesh = makeSkyDome(20, 32, 100.0f);
        waterMesh = makeWaterGrid(140, 24, 640.0f, 10.6f);
        initQuad();
        initHUD();
        initShips();
    } catch (const std::exception& ex) {
        std::cerr << "Shader/Resource initialization error:\n" << ex.what() << "\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    double lastTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        double now = glfwGetTime();
        float dt = static_cast<float>(std::min(0.05, now - lastTime));
        lastTime = now;

        glfwPollEvents();
        processContinuousInput(window, dt);

        if (!paused) {
            simTime += dt;
            lighthouseAngle += 75.0f * dt;
            updateShips(dt);
            updateBridge(dt);
            updateCrane(dt);
        }

        int width = 1, height = 1;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);

        if (nightLights) glClearColor(0.02f, 0.04f, 0.10f, 1.0f);
        else glClearColor(0.42f, 0.70f, 0.92f, 1.0f);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderScene(width, height);
        glfwSwapBuffers(window);
    }

    phongShader.destroy();
    gouraudShader.destroy();
    raytraceShader.destroy();
    skyShader.destroy();
    hudShader.destroy();

    cubeMesh.destroy();
    cylinderMesh.destroy();
    ringMesh.destroy();
    sphereMesh.destroy();
    skyDomeMesh.destroy();
    waterMesh.destroy();

    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);
    if (hudVAO) glDeleteVertexArrays(1, &hudVAO);
    if (hudVBO) glDeleteBuffers(1, &hudVBO);
    if (fontTexID) glDeleteTextures(1, &fontTexID);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
