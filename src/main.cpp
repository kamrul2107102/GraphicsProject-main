#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using glm::mat4;
using glm::vec3;
using glm::vec2;

static constexpr float PI = 3.14159265359f;

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

// 3D UV Sphere for Sun, Moon, Clouds, Buoys, Radar, and Lighthouse Dome
static Mesh makeSphere(int latSeg = 16, int lonSeg = 24, float radius = 1.0f) {
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

// Subdivided water grid sized to fit precisely within the canal channel (Z: -5.3 to +5.3)
static Mesh makeWaterGrid(int gridX = 80, int gridZ = 24, float sizeX = 80.0f, float sizeZ = 10.6f) {
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

static Mesh cubeMesh, cylinderMesh, ringMesh, sphereMesh, waterMesh;

// Shaders
static Shader gouraudShader, phongShader, raytraceShader;
enum class ShadingMode { Phong, Gouraud };
static ShadingMode currentShading = ShadingMode::Phong;
static bool raytraceMode = false;

enum class CameraMode { Free, Overview, CraneCab, ShipFollow, LighthouseView };
static CameraMode currentCamMode = CameraMode::Overview;

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

struct Ship {
    float x = -32.0f;
    float speed = 3.2f;
    float z = 0.0f;
    vec3 color{0.2f,0.45f,0.75f};
    float scale = 1.0f;
    bool active = true;
    bool isTug = false;
};

// Interactive 3D Crane Operator State
struct Crane {
    enum class AutoState { Idle, ToShip, LowerToShip, GrabCargo, LiftCargo, ToYard, LowerToYard, ReleaseCargo, ReturnCab };
    AutoState autoState = AutoState::Idle;
    float autoTimer = 0.0f;

    bool manual = false;
    float slewingYaw = 0.0f;      // Rotation around vertical turret axis (deg)
    float trolleyZ = 3.0f;        // Position along the boom (over water: 1.5 ~ over dock: 10.0)
    float hookY = 5.5f;           // Height of hook (1.2f to 7.0f)
    float trolleySpeed = 4.5f;
    float hookSpeed = 3.2f;

    bool carrying = false;
    int cargoIndex = 0;
    int score = 0;
};

static Camera camera;
static std::vector<Ship> ships;
static Crane crane;
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

static Shader& activeShader() {
    return (currentShading == ShadingMode::Phong) ? phongShader : gouraudShader;
}

static mat4 T(const vec3& p) { return glm::translate(mat4(1.0f), p); }
static mat4 S(const vec3& s) { return glm::scale(mat4(1.0f), s); }
static mat4 R(float deg, const vec3& axis) { return glm::rotate(mat4(1.0f), glm::radians(deg), axis); }

static void drawMesh(const Mesh& mesh, const mat4& model, const vec3& color, float shininess = 32.0f,
                     const vec3& specularColor = vec3(0.45f), bool isWater = false, bool isEmissive = false) {
    Shader& s = activeShader();
    s.setMat4("uModel", model);
    s.setVec3("uColor", color);
    s.setFloat("uShininess", shininess);
    s.setVec3("uSpecularColor", specularColor);
    s.setBool("uIsWater", isWater);
    s.setBool("uIsEmissive", isEmissive);
    s.setFloat("uTime", simTime);
    mesh.draw();
}

// Overloads for drawCube (mat4 and vec3 pos)
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

// Overloads for drawCylinder (mat4 and vec3 pos)
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

// Overloads for drawSphere (mat4 and vec3 pos)
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

// ---------------- 1. SKY, SUN, MOON & CLOUDS ----------------
static void drawSunAndSky() {
    if (!nightLights) {
        // Bright glowing 3D Sun in the sky
        vec3 sunPos = vec3(28.0f, 24.0f, -22.0f);
        drawSphere(sunPos, 3.2f, vec3(1.2f, 1.05f, 0.45f), 128.0f, vec3(1.0f), true);

        // Radiant Sun Corona rings
        drawMesh(ringMesh, T(sunPos) * R(45.0f, {0,1,1}) * S({4.8f, 1.0f, 4.8f}), vec3(1.0f, 0.85f, 0.35f), 64.0f, vec3(1.0f), false, true);
        drawMesh(ringMesh, T(sunPos) * R(-35.0f, {1,1,0}) * S({6.2f, 1.0f, 6.2f}), vec3(1.0f, 0.70f, 0.20f), 64.0f, vec3(1.0f), false, true);

        // 3D Drifting Volumetric Clouds in the sky
        for (int i = 0; i < 4; ++i) {
            float cx = std::fmod(-35.0f + i * 22.0f + simTime * 1.5f, 75.0f) - 35.0f;
            float cy = 20.0f + std::sin(i * 1.5f) * 2.0f;
            float cz = -18.0f + i * 8.0f;
            vec3 cloudCol = vec3(0.95f, 0.96f, 0.98f);
            drawSphere({cx, cy, cz}, 2.4f, cloudCol, 16.0f, vec3(0.1f));
            drawSphere({cx + 1.8f, cy - 0.3f, cz + 0.5f}, 1.9f, cloudCol, 16.0f, vec3(0.1f));
            drawSphere({cx - 1.8f, cy - 0.4f, cz - 0.4f}, 1.8f, cloudCol, 16.0f, vec3(0.1f));
        }
    } else {
        // Glowing Pale Silver 3D Moon in night sky
        vec3 moonPos = vec3(-26.0f, 22.0f, 18.0f);
        drawSphere(moonPos, 2.5f, vec3(0.88f, 0.92f, 1.0f), 128.0f, vec3(1.0f), true);
        drawMesh(ringMesh, T(moonPos) * R(30.0f, {0,1,0}) * S({3.5f, 1.0f, 3.5f}), vec3(0.4f, 0.5f, 0.8f), 64.0f, vec3(0.8f), false, true);

        // Twinkling background stars
        for (int i = 0; i < 12; ++i) {
            float sx = -35.0f + i * 6.5f;
            float sy = 24.0f + (i % 3) * 3.5f;
            float sz = -22.0f + (i % 4) * 8.0f;
            float twinkle = 0.5f + 0.5f * std::sin(simTime * 4.0f + i * 1.7f);
            drawSphere({sx, sy, sz}, 0.22f * twinkle, vec3(0.95f, 0.95f, 1.0f), 32.0f, vec3(1.0f), true);
        }
    }
}

// ---------------- 2. ZERO-CLIPPING WATER CANAL & SEABED ----------------
static void drawWaterAndCanal() {
    Shader& s = activeShader();

    // Deep seabed underneath the canal (at Y = -2.8)
    drawCube({0.0f, -2.8f, 0.0f}, {82.0f, 0.5f, 11.2f}, {0.14f, 0.17f, 0.18f}, 0.0f, 8.0f);

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
}

// ---------------- 3D ENVIRONMENT: TREES, CARS, TURBINES & SURROUNDING WORLD ----------------
static void drawTree(const vec3& pos, float scale = 1.0f, int variant = 0) {
    vec3 trunkCol{0.34f, 0.24f, 0.16f};
    vec3 leafCol1 = (variant % 2 == 0) ? vec3(0.18f, 0.50f, 0.22f) : vec3(0.14f, 0.44f, 0.18f);
    vec3 leafCol2 = (variant % 3 == 0) ? vec3(0.24f, 0.58f, 0.26f) : vec3(0.20f, 0.52f, 0.24f);

    // Trunk
    drawCylinder(pos + vec3(0, 1.4f * scale, 0), {0.45f * scale, 2.8f * scale, 0.45f * scale}, trunkCol, 0.0f, 16.0f);

    // Multi-layered lush canopy spheres
    drawSphere(pos + vec3(0, 3.6f * scale, 0), 1.8f * scale, leafCol1, 16.0f);
    drawSphere(pos + vec3(0.5f * scale, 4.4f * scale, 0.3f * scale), 1.4f * scale, leafCol2, 16.0f);
    drawSphere(pos + vec3(-0.4f * scale, 4.2f * scale, -0.4f * scale), 1.3f * scale, leafCol1, 16.0f);
    drawSphere(pos + vec3(0, 5.2f * scale, 0), 1.0f * scale, leafCol2, 16.0f);
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

static void drawWindTurbine(const vec3& pos) {
    drawCylinder(pos + vec3(0, 14.0f, 0), {1.2f, 28.0f, 1.2f}, {0.92f, 0.94f, 0.95f}, 0.0f, 64.0f);
    drawCube(pos + vec3(0, 28.5f, 0), {3.6f, 1.6f, 1.8f}, {0.90f, 0.92f, 0.94f}, 0.0f, 64.0f);
    mat4 hub = T(pos + vec3(2.0f, 28.5f, 0)) * R(simTime * 75.0f, {1,0,0});
    drawSphere(hub, 0.85f, {0.95f, 0.95f, 0.95f});
    for (int i = 0; i < 3; ++i) {
        mat4 blade = hub * R(i * 120.0f, {1,0,0}) * T({0, 5.5f, 0}) * S({0.15f, 11.0f, 0.65f});
        drawMesh(cubeMesh, blade, {0.95f, 0.95f, 0.95f}, 64.0f);
    }
}

static void drawSurroundingWorld() {
    vec3 waterColor = nightLights ? vec3(0.04f, 0.16f, 0.30f) : vec3(0.10f, 0.44f, 0.70f);
    vec3 waterSpecular = vec3(0.95f, 0.98f, 1.0f);

    // ================= 1. MASSIVE TERRAIN (NO MORE FLOATING BOX) =================
    // A. Endless Ocean stretching to the horizon (North / Front: Z = -5.4 to -300, X = -300 to +300)
    drawCube({0.0f, -0.62f, -130.0f}, {500.0f, 0.2f, 250.0f}, waterColor, 0.0f, 128.0f, waterSpecular);
    // Lateral sea expanses (East and West)
    drawCube({-170.0f, -0.62f, 0.0f}, {160.0f, 0.2f, 60.0f}, waterColor, 0.0f, 128.0f, waterSpecular);
    drawCube({ 170.0f, -0.62f, 0.0f}, {160.0f, 0.2f, 60.0f}, waterColor, 0.0f, 128.0f, waterSpecular);

    // B. Vast Inland Hinterland Landscape (South / Inland: Z = 23 to 260, X = -280 to +280)
    drawCube({0.0f, -0.65f, 135.0f}, {500.0f, 1.1f, 225.0f}, {0.30f, 0.42f, 0.25f}, 0.0f, 8.0f); // Grassy inland plain
    // Lateral coastal ground extensions
    drawCube({-180.0f, -0.65f, 60.0f}, {160.0f, 1.1f, 120.0f}, {0.32f, 0.44f, 0.26f}, 0.0f, 8.0f);
    drawCube({ 180.0f, -0.65f, 60.0f}, {160.0f, 1.1f, 120.0f}, {0.32f, 0.44f, 0.26f}, 0.0f, 8.0f);

    // C. Distant Mountain Ranges on the horizon (North & Far Hinterland)
    for (int i = 0; i < 9; ++i) {
        float mx = -180.0f + i * 45.0f;
        float mz = -210.0f + (i % 3) * 15.0f;
        float mHeight = 28.0f + (i % 4) * 8.0f;
        drawCylinder({mx, mHeight * 0.5f - 2.0f, mz}, {32.0f, mHeight, 32.0f}, {0.25f, 0.32f, 0.38f}, 45.0f, 16.0f);
    }
    for (int i = 0; i < 9; ++i) {
        float mx = -180.0f + i * 45.0f;
        float mz = 220.0f + (i % 3) * 15.0f;
        float mHeight = 32.0f + (i % 3) * 10.0f;
        drawCylinder({mx, mHeight * 0.5f - 2.0f, mz}, {36.0f, mHeight, 36.0f}, {0.22f, 0.35f, 0.28f}, 30.0f, 16.0f);
    }

    // Distant Wind Turbines on the eastern hills
    drawWindTurbine({ 110.0f, 0.0f, -40.0f});
    drawWindTurbine({ 135.0f, 0.0f, -60.0f});
    drawWindTurbine({ 120.0f, 0.0f,  20.0f});

    // ================= 2. PORT AUTHORITY HEADQUARTERS & OFFICE BUILDINGS (INLAND, Z = 26 to 65) =================
    // Connecting Inland Port Boulevard
    drawCube({0.0f, 0.02f, 32.0f}, {120.0f, 0.06f, 8.5f}, {0.22f, 0.22f, 0.24f}, 0.0f, 16.0f);
    // Yellow dashed road stripes
    for (int x = -55; x <= 55; x += 6) {
        drawCube({static_cast<float>(x), 0.06f, 32.0f}, {2.8f, 0.02f, 0.18f}, {0.95f, 0.88f, 0.25f}, 0.0f, 16.0f);
    }
    // Cross avenue
    drawCube({0.0f, 0.02f, 52.0f}, {14.0f, 0.06f, 36.0f}, {0.22f, 0.22f, 0.24f}, 0.0f, 16.0f);

    // A. Main Port Administration Skyscraper Tower (Headquarters)
    vec3 hqPos = {0.0f, 0.0f, 56.0f};
    drawCube(hqPos + vec3(0, 13.0f, 0), {22.0f, 26.0f, 15.0f}, {0.35f, 0.38f, 0.42f}, 0.0f, 32.0f);
    // Blue reflective glass curtain facade
    drawCube(hqPos + vec3(0, 13.0f, -7.55f), {19.0f, 22.0f, 0.2f}, {0.22f, 0.52f, 0.82f}, 0.0f, 128.0f, vec3(0.9f));
    drawCube(hqPos + vec3(0, 13.0f,  7.55f), {19.0f, 22.0f, 0.2f}, {0.22f, 0.52f, 0.82f}, 0.0f, 128.0f, vec3(0.9f));
    // Window mullion horizontal floor ribs
    for (int f = 1; f <= 5; ++f) {
        drawCube(hqPos + vec3(0, f * 4.4f, -7.6f), {19.5f, 0.35f, 0.25f}, {0.20f, 0.22f, 0.25f});
    }
    // Entrance lobby canopy
    drawCube(hqPos + vec3(0, 2.2f, -9.5f), {10.0f, 0.4f, 4.5f}, {0.18f, 0.20f, 0.24f}, 0.0f, 48.0f);
    drawCylinder(hqPos + vec3(-4.0f, 1.1f, -11.0f), {0.3f, 2.2f, 0.3f}, {0.7f, 0.7f, 0.7f});
    drawCylinder(hqPos + vec3( 4.0f, 1.1f, -11.0f), {0.3f, 2.2f, 0.3f}, {0.7f, 0.7f, 0.7f});
    // Rooftop Helipad & Antenna Mast
    drawCylinder(hqPos + vec3(0, 26.2f, 0), {8.0f, 0.3f, 8.0f}, {0.25f, 0.28f, 0.30f});
    drawMesh(ringMesh, T(hqPos + vec3(0, 26.4f, 0)) * R(90, {1,0,0}) * S({6.0f, 1.0f, 6.0f}), {0.95f, 0.85f, 0.15f}, 32.0f);
    drawCylinder(hqPos + vec3(0, 31.0f, 0), {0.25f, 9.0f, 0.25f}, {0.8f, 0.2f, 0.15f});
    drawSphere(hqPos + vec3(0, 35.5f, 0), 0.35f, {1.2f, 0.1f, 0.1f}, 64.0f, vec3(1.0f), true);

    // B. Port Customs & Maritime Inspection Building (Left wing)
    vec3 customsPos = {-32.0f, 0.0f, 52.0f};
    drawCube(customsPos + vec3(0, 8.0f, 0), {22.0f, 16.0f, 14.0f}, {0.45f, 0.48f, 0.50f}, 0.0f, 32.0f);
    drawCube(customsPos + vec3(0, 8.0f, -7.05f), {18.0f, 12.0f, 0.2f}, {0.24f, 0.46f, 0.68f}, 0.0f, 96.0f);

    // C. Harbor Operations & Logistics Center (Right wing)
    vec3 opsPos = {32.0f, 0.0f, 52.0f};
    drawCube(opsPos + vec3(0, 9.0f, 0), {22.0f, 18.0f, 14.0f}, {0.42f, 0.45f, 0.48f}, 0.0f, 32.0f);
    drawCube(opsPos + vec3(0, 9.0f, -7.05f), {18.0f, 14.0f, 0.2f}, {0.24f, 0.46f, 0.68f}, 0.0f, 96.0f);
    drawCylinder(opsPos + vec3(0, 19.5f, 0), {6.5f, 3.0f, 6.5f}, {0.20f, 0.45f, 0.65f}, 0.0f, 96.0f);

    // D. Port Security Gatehouse & Barriers (Z = 25.0)
    drawCube({0.0f, 1.6f, 25.2f}, {3.5f, 3.2f, 2.5f}, {0.85f, 0.88f, 0.90f}, 0.0f, 32.0f);
    drawCylinder({-4.0f, 1.0f, 25.2f}, {0.12f, 5.0f, 0.12f}, {0.95f, 0.2f, 0.15f}, 90.0f);
    drawCylinder({ 4.0f, 1.0f, 25.2f}, {0.12f, 5.0f, 0.12f}, {0.95f, 0.2f, 0.15f}, 90.0f);

    // E. Parking Lot & Parked Cars
    drawCube({-25.0f, 0.03f, 37.0f}, {30.0f, 0.04f, 10.0f}, {0.20f, 0.20f, 0.22f}, 0.0f, 16.0f);
    drawCube({ 25.0f, 0.03f, 37.0f}, {30.0f, 0.04f, 10.0f}, {0.20f, 0.20f, 0.22f}, 0.0f, 16.0f);
    drawParkedCar({-35.0f, 0.0f, 37.0f}, 0.0f, {0.85f, 0.18f, 0.15f});
    drawParkedCar({-30.0f, 0.0f, 37.0f}, 0.0f, {0.18f, 0.35f, 0.75f});
    drawParkedCar({-25.0f, 0.0f, 37.0f}, 0.0f, {0.90f, 0.92f, 0.94f});
    drawParkedCar({-20.0f, 0.0f, 37.0f}, 0.0f, {0.15f, 0.15f, 0.16f});
    drawParkedCar({-15.0f, 0.0f, 37.0f}, 0.0f, {0.88f, 0.55f, 0.12f});

    drawParkedCar({ 15.0f, 0.0f, 37.0f}, 0.0f, {0.75f, 0.78f, 0.80f});
    drawParkedCar({ 20.0f, 0.0f, 37.0f}, 0.0f, {0.14f, 0.55f, 0.30f});
    drawParkedCar({ 25.0f, 0.0f, 37.0f}, 0.0f, {0.85f, 0.22f, 0.18f});
    drawParkedCar({ 30.0f, 0.0f, 37.0f}, 0.0f, {0.92f, 0.92f, 0.95f});
    drawParkedCar({ 35.0f, 0.0f, 37.0f}, 0.0f, {0.20f, 0.22f, 0.25f});

    // F. Distribution Warehouses & Freight Centers
    drawCube({-65.0f, 5.0f, 52.0f}, {30.0f, 10.0f, 16.0f}, {0.38f, 0.42f, 0.46f}, 0.0f, 24.0f);
    drawCube({ 65.0f, 5.0f, 52.0f}, {30.0f, 10.0f, 16.0f}, {0.38f, 0.42f, 0.46f}, 0.0f, 24.0f);

    // ================= 3. 50+ 3D TREES & NATURAL LANDSCAPING =================
    // Boulevard Trees lining the main avenue
    for (int x = -50; x <= 50; x += 10) {
        if (std::abs(x) < 6) continue;
        drawTree({static_cast<float>(x), 0.0f, 27.5f}, 1.1f, x);
        drawTree({static_cast<float>(x), 0.0f, 43.5f}, 1.0f, x + 1);
    }
    // Park plazas around Headquarters
    for (int x = -14; x <= 14; x += 7) {
        if (std::abs(x) < 4) continue;
        drawTree({static_cast<float>(x), 0.0f, 66.0f}, 1.3f, x);
        drawTree({static_cast<float>(x), 0.0f, 48.0f}, 1.15f, x + 2);
    }
    // Trees on North embankment & riverbanks
    for (int x = -55; x <= 55; x += 11) {
        drawTree({static_cast<float>(x), 0.0f, -19.5f}, 1.2f, x);
        drawTree({static_cast<float>(x) + 4.0f, 0.0f, -25.0f}, 1.35f, x + 3);
    }
    // Coastal nature clusters on East and West
    for (int z = -10; z <= 40; z += 12) {
        drawTree({-52.0f, 0.0f, static_cast<float>(z)}, 1.25f, z);
        drawTree({-62.0f, 0.0f, static_cast<float>(z) + 3.0f}, 1.4f, z + 1);
        drawTree({ 52.0f, 0.0f, static_cast<float>(z)}, 1.25f, z + 2);
        drawTree({ 62.0f, 0.0f, static_cast<float>(z) + 3.0f}, 1.4f, z + 3);
    }
}

// ---------------- 4. PORT QUAYS, DOCK PLATFORMS & SHORELINE ----------------
static void drawQuays() {
    // ---------- SOUTH QUAY (CONTAINER TERMINAL) ----------
    // Main concrete dock slab (from Z = 5.4 to Z = 23.4, Y top = 0.0)
    drawCube({0.0f, -0.6f, 14.4f}, {72.0f, 1.2f, 18.0f}, {0.45f, 0.47f, 0.48f}, 0.0f, 24.0f);

    // Vertical Quay Bulkhead Seawall facing water (Z = 5.4, Y from 0 to -2.8)
    drawCube({0.0f, -1.4f, 5.35f}, {72.0f, 2.8f, 0.4f}, {0.32f, 0.34f, 0.36f}, 0.0f, 16.0f);

    // Quay Edge Safety Hazard Stripes (alternating yellow/black blocks along Z = 5.6)
    for (int x = -35; x <= 35; x += 2) {
        bool yellow = ((x / 2) % 2 == 0);
        vec3 stripeColor = yellow ? vec3(0.95f, 0.80f, 0.12f) : vec3(0.14f, 0.14f, 0.16f);
        drawCube({static_cast<float>(x), 0.05f, 5.65f}, {1.95f, 0.12f, 0.45f}, stripeColor, 0.0f, 32.0f);
    }

    // Heavy Cast-Iron Mooring Bollards along dock edge
    for (int x = -30; x <= 30; x += 10) {
        drawCylinder({static_cast<float>(x), 0.28f, 5.95f}, {0.35f, 0.55f, 0.35f}, {0.18f, 0.18f, 0.20f}, 0.0f, 64.0f);
        drawSphere({static_cast<float>(x), 0.58f, 5.95f}, 0.26f, {0.18f, 0.18f, 0.20f}, 64.0f);
    }

    // Steel Gantry Rails for the Container Crane (parallel tracks at Z = 6.9 and Z = 9.8)
    drawCube({0.0f, 0.04f, 6.9f}, {72.0f, 0.08f, 0.18f}, {0.70f, 0.72f, 0.75f}, 0.0f, 96.0f);
    drawCube({0.0f, 0.04f, 9.8f}, {72.0f, 0.08f, 0.18f}, {0.70f, 0.72f, 0.75f}, 0.0f, 96.0f);

    // Terminal Asphalt Road (Z = 18.5f) with yellow divider stripes
    drawCube({0.0f, 0.02f, 18.5f}, {72.0f, 0.06f, 5.5f}, {0.24f, 0.24f, 0.26f}, 0.0f, 12.0f);
    for (int x = -32; x <= 32; x += 6) {
        drawCube({static_cast<float>(x), 0.06f, 18.5f}, {2.8f, 0.02f, 0.18f}, {0.95f, 0.88f, 0.25f}, 0.0f, 16.0f);
    }

    // ---------- NORTH SHORE (INDUSTRIAL WATERFRONT & LIGHTHOUSE) ----------
    // North dock slab (from Z = -5.4 to Z = -23.4, Y top = 0.0)
    drawCube({0.0f, -0.6f, -14.4f}, {72.0f, 1.2f, 18.0f}, {0.35f, 0.44f, 0.32f}, 0.0f, 12.0f); // Grassy industrial terrain
    drawCube({0.0f, -1.4f, -5.35f}, {72.0f, 2.8f, 0.4f}, {0.32f, 0.34f, 0.36f}, 0.0f, 16.0f); // North seawall

    // North concrete quay apron
    drawCube({0.0f, 0.02f, -7.0f}, {72.0f, 0.06f, 3.0f}, {0.52f, 0.54f, 0.56f}, 0.0f, 16.0f);

    // Industrial Port Warehouse
    drawCube({-16.0f, 2.5f, -12.5f}, {16.0f, 5.0f, 7.0f}, {0.38f, 0.42f, 0.46f}, 0.0f, 24.0f);
    drawCube({-16.0f, 5.15f, -12.5f}, {16.8f, 0.4f, 7.6f}, {0.18f, 0.22f, 0.25f}, 0.0f, 32.0f); // Roof
    for (int gx = -20; gx <= -12; gx += 4) {
        drawCube({static_cast<float>(gx), 1.6f, -8.9f}, {2.6f, 3.2f, 0.2f}, {0.15f, 0.18f, 0.22f}); // Rolling doors
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

    // Decorative Low-Poly Trees along the north embankment
    for (int tx = -28; tx <= 28; tx += 14) {
        if (tx == 0) continue;
        drawCylinder({static_cast<float>(tx), 1.2f, -18.0f}, {0.4f, 2.4f, 0.4f}, {0.35f, 0.22f, 0.12f});
        drawSphere({static_cast<float>(tx), 3.2f, -18.0f}, 1.6f, {0.18f, 0.52f, 0.22f}, 16.0f);
        drawSphere({static_cast<float>(tx), 4.4f, -18.0f}, 1.2f, {0.24f, 0.62f, 0.28f}, 16.0f);
    }
}

// ---------------- 4. COASTAL LIGHTHOUSE WITH ROTATING BEACON ----------------
static void drawLighthouse() {
    vec3 lhPos = vec3(26.0f, 0.0f, -9.5f);

    // Stone foundation
    drawCylinder(lhPos + vec3(0, 0.75f, 0), {4.2f, 1.5f, 4.2f}, {0.38f, 0.40f, 0.42f}, 0.0f, 16.0f);

    // Tapered tower sections (Classic Red & White Maritime Stripes)
    drawCylinder(lhPos + vec3(0, 2.5f, 0), {3.2f, 2.0f, 3.2f}, {0.85f, 0.18f, 0.15f}, 0.0f, 48.0f); // Red
    drawCylinder(lhPos + vec3(0, 4.5f, 0), {2.8f, 2.0f, 2.8f}, {0.92f, 0.92f, 0.95f}, 0.0f, 48.0f); // White
    drawCylinder(lhPos + vec3(0, 6.5f, 0), {2.4f, 2.0f, 2.4f}, {0.85f, 0.18f, 0.15f}, 0.0f, 48.0f); // Red
    drawCylinder(lhPos + vec3(0, 8.5f, 0), {2.0f, 2.0f, 2.0f}, {0.92f, 0.92f, 0.95f}, 0.0f, 48.0f); // White

    // Observation Gallery & Railing
    drawCylinder(lhPos + vec3(0, 9.6f, 0), {2.6f, 0.3f, 2.6f}, {0.22f, 0.22f, 0.25f}, 0.0f, 32.0f);

    // Lantern Room (Glass & glowing core)
    drawCylinder(lhPos + vec3(0, 10.6f, 0), {1.7f, 1.7f, 1.7f}, {0.18f, 0.20f, 0.24f}, 0.0f, 96.0f);
    // Glowing Light Bulb Inside
    drawSphere(lhPos + vec3(0, 10.6f, 0), 0.65f, vec3(1.2f, 1.1f, 0.6f), 128.0f, vec3(1.0f), true);

    // Dome Cap
    drawSphere(lhPos + vec3(0, 11.5f, 0), 1.0f, {0.85f, 0.18f, 0.15f}, 64.0f);

    // Rotating Golden Beacon Beam
    mat4 beaconMat = T(lhPos + vec3(0, 10.6f, 0)) * R(lighthouseAngle, {0,1,0});
    vec3 beamCol = nightLights ? vec3(1.0f, 0.95f, 0.5f) : vec3(0.9f, 0.85f, 0.6f);
    drawMesh(cylinderMesh, beaconMat * T({6.0f, 0, 0}) * R(90, {0,0,1}) * S({0.8f, 12.0f, 0.8f}), beamCol, 64.0f, vec3(1.0f), false, true);
}

// ---------------- 5. INTERACTIVE BASCULE DRAWBRIDGE ----------------
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

    // Orange Safety Railings on Bridge
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

// ---------------- 6. CONTAINER STACKS & YARD ----------------
static void drawContainerYard() {
    // Organized Container Terminal Stacks (Z = 12.0 to 15.5)
    float startX = -18.0f;
    int idx = 0;
    for (int col = 0; col < 6; ++col) {
        for (int row = 0; row < 2; ++row) {
            float x = startX + col * 4.2f;
            float z = 12.0f + row * 2.8f;
            int height = ((col + row) % 3) + 1; // 1, 2, or 3 containers high
            for (int h = 0; h < height; ++h) {
                vec3 cColor = COLORS[(idx++) % 6];
                drawCube({x, 0.65f + h * 1.25f, z}, {3.6f, 1.2f, 2.2f}, cColor, 0.0f, 48.0f);
                // Corner casting details
                drawCube({x, 0.65f + h * 1.25f, z}, {3.64f, 1.22f, 0.1f}, {0.15f, 0.15f, 0.16f});
            }
        }
    }

    // Target Deposit Area for the Crane (Marked with yellow hazard borders on dock at X = 8.0, Z = 9.8)
    drawCube({8.0f, 0.05f, 9.8f}, {4.2f, 0.05f, 2.6f}, {0.22f, 0.24f, 0.26f}, 0.0f, 16.0f);
    drawCube({8.0f, 0.08f, 8.5f}, {4.4f, 0.04f, 0.15f}, {0.95f, 0.85f, 0.1f}, 0.0f, 32.0f);
    drawCube({8.0f, 0.08f, 11.1f}, {4.4f, 0.04f, 0.15f}, {0.95f, 0.85f, 0.1f}, 0.0f, 32.0f);
    drawCube({5.8f, 0.08f, 9.8f}, {0.15f, 0.04f, 2.6f}, {0.95f, 0.85f, 0.1f}, 0.0f, 32.0f);
    drawCube({10.2f, 0.08f, 9.8f}, {0.15f, 0.04f, 2.6f}, {0.95f, 0.85f, 0.1f}, 0.0f, 32.0f);
}

// ---------------- 7. PLAYABLE HIGH-TECH SHIP-TO-SHORE CRANE ----------------
static void drawCrane() {
    const float craneBaseX = 8.0f;
    const float craneBaseZ = 8.35f; // Centered between tracks at Z = 6.9 and Z = 9.8
    mat4 gantryBase = T({craneBaseX, 0.0f, craneBaseZ});

    // 1. Gantry Portal Legs (spanning across the dock)
    drawCylinder(gantryBase * T({-2.2f, 2.5f, -1.45f}), {0.55f, 5.0f, 0.55f}, {0.18f, 0.20f, 0.22f}, 0.0f, 48.0f);
    drawCylinder(gantryBase * T({ 2.2f, 2.5f, -1.45f}), {0.55f, 5.0f, 0.55f}, {0.18f, 0.20f, 0.22f}, 0.0f, 48.0f);
    drawCylinder(gantryBase * T({-2.2f, 2.5f,  1.45f}), {0.55f, 5.0f, 0.55f}, {0.18f, 0.20f, 0.22f}, 0.0f, 48.0f);
    drawCylinder(gantryBase * T({ 2.2f, 2.5f,  1.45f}), {0.55f, 5.0f, 0.55f}, {0.18f, 0.20f, 0.22f}, 0.0f, 48.0f);

    // Gantry Upper Platform Beam
    drawCube(gantryBase * T({0.0f, 5.0f, 0.0f}), {5.4f, 0.6f, 3.6f}, {0.85f, 0.55f, 0.10f}, 0.0f, 48.0f);

    // 2. Turret / Cab / Machinery House (Slewing Yaw Rotation Q/E)
    mat4 turret = gantryBase * T({0.0f, 5.5f, 0.0f}) * R(crane.slewingYaw, {0,1,0});
    drawCylinder(turret * T({0.0f, 0.35f, 0.0f}), {1.6f, 0.7f, 1.6f}, {0.20f, 0.22f, 0.24f}, 0.0f, 64.0f); // Slewing ring
    drawCube(turret * T({0.0f, 1.2f, 0.0f}), {3.2f, 1.5f, 2.6f}, {0.88f, 0.56f, 0.10f}, 0.0f, 48.0f); // Machine house

    // Operator Cab with glass windows (positioned with direct view of boom)
    drawCube(turret * T({1.3f, 0.75f, -1.0f}), {1.2f, 1.2f, 1.2f}, {0.22f, 0.24f, 0.28f}, 0.0f, 64.0f);
    drawCube(turret * T({1.3f, 0.75f, -1.0f}), {1.22f, 0.9f, 0.9f}, {0.55f, 0.75f, 0.88f}, 0.0f, 128.0f); // Window glass

    // 3. Long Horizontal Boom Arm (Extends out over the ship at Z <= 2 and back over the yard at Z >= 10)
    mat4 boom = turret * T({0.0f, 2.6f, 0.0f});
    drawCube(boom * T({0.0f, 0.0f, 0.0f}), {1.1f, 1.1f, 16.0f}, {0.85f, 0.52f, 0.08f}, 0.0f, 48.0f); // Main boom truss
    drawCube(boom * T({0.0f, 1.2f, -1.5f}), {0.2f, 2.2f, 0.2f}, {0.3f, 0.3f, 0.35f}); // A-frame mast

    // Boom tension cables
    drawCylinder(boom * T({0.0f, 1.0f, -4.5f}), {0.06f, 7.0f, 0.06f}, {0.2f, 0.2f, 0.2f}, 45.0f);
    drawCylinder(boom * T({0.0f, 1.0f,  4.5f}), {0.06f, 7.0f, 0.06f}, {0.2f, 0.2f, 0.2f}, -45.0f);

    // 4. Trolley (Slides along the Boom in Z axis relative to turret)
    float trolleyRelZ = crane.trolleyZ - craneBaseZ;
    mat4 trolleyMat = boom * T({0.0f, -0.65f, trolleyRelZ});
    drawCube(trolleyMat, {1.4f, 0.5f, 1.2f}, {0.22f, 0.24f, 0.26f}, 0.0f, 48.0f);

    // 5. Hoist Cables (Dynamic length based on crane.hookY)
    float boomWorldY = 5.5f + 2.6f - 0.65f; // ~7.45m
    float cableLength = std::max(0.4f, boomWorldY - crane.hookY);
    mat4 cableMat1 = trolleyMat * T({-0.4f, -cableLength * 0.5f, 0.0f}) * S({0.05f, cableLength, 0.05f});
    mat4 cableMat2 = trolleyMat * T({ 0.4f, -cableLength * 0.5f, 0.0f}) * S({0.05f, cableLength, 0.05f});
    drawMesh(cylinderMesh, cableMat1, {0.12f, 0.12f, 0.14f}, 64.0f);
    drawMesh(cylinderMesh, cableMat2, {0.12f, 0.12f, 0.14f}, 64.0f);

    // 6. Container Spreader (The heavy motorized lifting frame)
    mat4 spreaderMat = trolleyMat * T({0.0f, -cableLength, 0.0f});
    drawCube(spreaderMat, {3.4f, 0.28f, 1.8f}, {0.85f, 0.75f, 0.15f}, 0.0f, 64.0f); // Yellow spreader frame
    drawCube(spreaderMat * T({0.0f, 0.25f, 0.0f}), {1.0f, 0.35f, 0.8f}, {0.2f, 0.2f, 0.25f}); // Cable block
    // Corner flippers
    for (float fx : {-1.7f, 1.7f}) {
        for (float fz : {-0.9f, 0.9f}) {
            drawCube(spreaderMat * T({fx, -0.2f, fz}), {0.12f, 0.35f, 0.12f}, {0.15f, 0.15f, 0.15f});
        }
    }

    // 7. Attached Cargo (Carried Container)
    if (crane.carrying) {
        mat4 cargoMat = spreaderMat * T({0.0f, -0.75f, 0.0f}) * S({3.6f, 1.2f, 2.2f});
        drawMesh(cubeMesh, cargoMat, COLORS[crane.cargoIndex % 6], 48.0f);
    }
}

// ---------------- 8. SHIPS & MARITIME TRAFFIC ----------------
static void drawDockedContainerShip() {
    // Large Container Ship moored alongside the quay berth at X = 8.0, Z = 2.4
    mat4 shipRoot = T({8.0f, -0.25f + 0.03f * std::sin(simTime * 2.0f), 2.4f});

    // Red & Black Commercial Hull
    drawCube(shipRoot * T({0.0f, 0.4f, 0.0f}), {16.0f, 1.6f, 4.4f}, {0.14f, 0.16f, 0.18f}, 0.0f, 48.0f);
    drawCube(shipRoot * T({0.0f, -0.3f, 0.0f}), {16.1f, 0.6f, 4.42f}, {0.75f, 0.18f, 0.14f}, 0.0f, 32.0f); // Red waterline
    // Bow flare
    drawCylinder(shipRoot * T({8.0f, 0.4f, 0.0f}) * R(90, {0,0,1}), {2.2f, 2.5f, 4.2f}, {0.14f, 0.16f, 0.18f}, 0.0f, 48.0f);

    // Deck Container Stacks (ready to be unloaded by crane!)
    // Container ready for crane to grab (if not already picked up)
    if (!crane.carrying) {
        drawCube(shipRoot * T({0.0f, 1.65f, 0.0f}), {3.6f, 1.2f, 2.2f}, COLORS[crane.cargoIndex % 6], 0.0f, 48.0f);
    }
    // Other deck containers
    drawCube(shipRoot * T({-4.2f, 1.65f, 0.0f}), {3.6f, 1.2f, 2.2f}, COLORS[1], 0.0f, 48.0f);
    drawCube(shipRoot * T({-4.2f, 2.85f, 0.0f}), {3.6f, 1.2f, 2.2f}, COLORS[3], 0.0f, 48.0f);
    drawCube(shipRoot * T({ 4.2f, 1.65f, 0.0f}), {3.6f, 1.2f, 2.2f}, COLORS[2], 0.0f, 48.0f);

    // Ship Bridge Castle & Navigation Mast (Stern)
    drawCube(shipRoot * T({-7.0f, 2.2f, 0.0f}), {2.4f, 2.2f, 3.8f}, {0.90f, 0.92f, 0.94f}, 0.0f, 32.0f);
    drawCube(shipRoot * T({-7.0f, 3.1f, 0.0f}), {2.5f, 0.4f, 4.0f}, {0.20f, 0.45f, 0.65f}, 0.0f, 96.0f); // Bridge windows
    // Smokestack Funnel with smoke
    drawCylinder(shipRoot * T({-7.2f, 3.8f, 0.0f}), {0.6f, 1.4f, 0.6f}, {0.85f, 0.22f, 0.15f}, 0.0f, 32.0f);

    // Navigation Lights: Red (Port) and Green (Starboard)
    drawSphere(shipRoot * T({-6.0f, 3.4f,  1.9f}), 0.18f, {0.1f, 1.2f, 0.2f}, 64.0f, vec3(1.0f), true); // Starboard green
    drawSphere(shipRoot * T({-6.0f, 3.4f, -1.9f}), 0.18f, {1.2f, 0.1f, 0.1f}, 64.0f, vec3(1.0f), true); // Port red
}

static void drawMovingShip(const Ship& s) {
    float bob = 0.06f * std::sin(simTime * 2.8f + s.x * 0.25f);
    float roll = 2.0f * std::sin(simTime * 2.4f + s.x * 0.15f);
    float pitch = 1.4f * std::cos(simTime * 2.0f + s.x * 0.2f);

    mat4 root = T({s.x, -0.3f + bob, s.z}) * R(pitch, {0,0,1}) * R(roll, {1,0,0}) * S(vec3(s.scale));

    if (!s.isTug) {
        // Cargo Feeder Vessel
        drawCube(root * T({0, 0.55f, 0}), {8.0f, 1.3f, 2.8f}, s.color, 0.0f, 48.0f);
        drawCylinder(root * T({4.0f, 0.55f, 0}) * R(90, {0,0,1}), {1.3f, 1.8f, 2.6f}, s.color, 0.0f, 48.0f);
        drawCube(root * T({-2.4f, 1.55f, 0}), {1.8f, 1.1f, 2.2f}, {0.9f, 0.9f, 0.92f}, 0.0f, 32.0f);
        drawCylinder(root * T({-2.6f, 2.4f, 0}), {0.4f, 0.8f, 0.4f}, {0.85f, 0.2f, 0.15f}, 0.0f, 32.0f);
        drawCube(root * T({0.8f, 1.45f, 0}), {3.2f, 0.95f, 2.0f}, COLORS[3], 0.0f, 48.0f);
    } else {
        // Harbor Tugboat
        drawCube(root * T({0, 0.45f, 0}), {4.5f, 1.1f, 2.2f}, {0.85f, 0.28f, 0.15f}, 0.0f, 48.0f);
        drawCylinder(root * T({2.2f, 0.45f, 0}) * R(90, {0,0,1}), {1.1f, 1.2f, 2.0f}, {0.85f, 0.28f, 0.15f}, 0.0f, 48.0f);
        drawCube(root * T({-0.3f, 1.35f, 0}), {1.6f, 1.0f, 1.6f}, {0.95f, 0.95f, 0.95f}, 0.0f, 32.0f);
        drawCylinder(root * T({-0.4f, 2.1f, 0}), {0.3f, 0.7f, 0.3f}, {0.15f, 0.15f, 0.15f}, 0.0f, 32.0f);
    }

    // Bow wave foam
    float foamAlpha = 0.5f + 0.3f * std::sin(simTime * 8.0f);
    drawSphere(root * T({4.2f, 0.05f, 0}), 0.35f * foamAlpha, vec3(0.9f, 0.95f, 1.0f), 16.0f, vec3(1.0f));
}

// Channel Navigation Buoys (Floating with blinking lights)
static void drawBuoys() {
    for (int i = 0; i < 2; ++i) {
        float bx = -18.0f + i * 36.0f;
        float bz1 = -3.8f; // Green Starboard buoy
        float bz2 =  3.8f; // Red Port buoy
        float bob1 = 0.08f * std::sin(simTime * 3.0f + i);
        float bob2 = 0.08f * std::cos(simTime * 2.8f + i);

        // Green Buoy (North channel edge)
        drawCylinder({bx, -0.4f + bob1, bz1}, {0.7f, 0.8f, 0.7f}, {0.15f, 0.75f, 0.25f}, 0.0f, 48.0f);
        drawSphere({bx, 0.2f + bob1, bz1}, 0.22f, {0.1f, 1.2f, 0.2f}, 32.0f, vec3(1.0f), true);

        // Red Buoy (South channel edge)
        drawCylinder({bx, -0.4f + bob2, bz2}, {0.7f, 0.8f, 0.7f}, {0.85f, 0.20f, 0.15f}, 0.0f, 48.0f);
        drawSphere({bx, 0.2f + bob2, bz2}, 0.22f, {1.2f, 0.1f, 0.1f}, 32.0f, vec3(1.0f), true);
    }
}

// ---------------- 9. TRUCKS & PORT TRAFFIC ----------------
static void drawTruck(const vec3& p, float phase, bool carryingCargo = true) {
    float bob = 0.02f * std::sin(simTime * 5.0f + phase);
    mat4 root = T(p + vec3(0, bob, 0));

    // Truck Chassis & Cab
    drawMesh(cubeMesh, root * T({0.0f, 0.65f, 0.0f}) * S({4.4f, 0.45f, 1.8f}), {0.18f, 0.20f, 0.22f}, 32.0f);
    drawMesh(cubeMesh, root * T({1.7f, 1.25f, 0.0f}) * S({1.1f, 1.1f, 1.7f}), {0.85f, 0.35f, 0.15f}, 48.0f); // Orange cab
    drawMesh(cubeMesh, root * T({1.95f, 1.35f, 0.0f}) * S({0.35f, 0.65f, 1.5f}), {0.65f, 0.85f, 0.95f}, 96.0f); // Windshield

    // Carried container on truck flatbed
    if (carryingCargo) {
        drawMesh(cubeMesh, root * T({-0.6f, 1.55f, 0.0f}) * S({3.0f, 1.2f, 1.7f}), COLORS[phase > 2.0f ? 0 : 4], 48.0f);
    }

    // Wheels
    drawWheel(p + vec3(-1.3f, 0.35f,  0.95f), 0.38f, 0.22f, {0.08f, 0.08f, 0.08f});
    drawWheel(p + vec3( 1.3f, 0.35f,  0.95f), 0.38f, 0.22f, {0.08f, 0.08f, 0.08f});
    drawWheel(p + vec3(-1.3f, 0.35f, -0.95f), 0.38f, 0.22f, {0.08f, 0.08f, 0.08f});
    drawWheel(p + vec3( 1.3f, 0.35f, -0.95f), 0.38f, 0.22f, {0.08f, 0.08f, 0.08f});

    // Headlights
    drawSphere(p + vec3(2.25f, 0.65f,  0.6f), 0.12f, vec3(1.2f, 1.15f, 0.8f), 32.0f, vec3(1.0f), true);
    drawSphere(p + vec3(2.25f, 0.65f, -0.6f), 0.12f, vec3(1.2f, 1.15f, 0.8f), 32.0f, vec3(1.0f), true);
}

// ---------------- 10. HIGH-MAST FLOODLIGHTS ----------------
static void drawLamps() {
    for (int i = 0; i < 7; ++i) {
        float x = -18.0f + i * 6.0f;
        float z = 6.8f; // Located on dock apron

        // Tall steel pole
        drawCylinder({x, 2.5f, z}, {0.16f, 5.0f, 0.16f}, {0.20f, 0.22f, 0.25f}, 0.0f, 64.0f);
        // Cantilever horizontal arm
        drawCylinder({x, 4.95f, z - 0.4f}, {0.12f, 0.9f, 0.12f}, {0.20f, 0.22f, 0.25f}, 90.0f);

        // Lamp fixture & glowing sodium bulb
        vec3 lampGlow = nightLights ? vec3(1.2f, 0.95f, 0.45f) : vec3(0.5f, 0.5f, 0.45f);
        drawSphere({x, 4.85f, z - 0.75f}, 0.25f, lampGlow, 64.0f, vec3(1.0f), nightLights);
    }
}

// ---------------- GAME SIMULATION UPDATES ----------------
static void updateShips(float dt) {
    for (size_t i = 0; i < ships.size(); ++i) {
        ships[i].x += ships[i].speed * shipSpeedMultiplier * dt;
        if (ships[i].x > 36.0f) {
            ships[i].x = -36.0f - static_cast<float>(i) * 5.0f;
        }
    }
}

static void updateBridge(float dt) {
    float target = 0.0f;
    if (autoBridge) {
        bool shipNear = false;
        for (const Ship& s : ships) {
            if (s.x > -9.0f && s.x < 9.0f) {
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

// Interactive & Automated Crane State Machine
static void updateCrane(float dt) {
    if (crane.manual) {
        // Player is controlling manually!
        return;
    }

    // Auto Crane Cycle (Full container unload sequence)
    if (crane.autoState == Crane::AutoState::Idle) return;

    crane.autoTimer += dt;

    switch (crane.autoState) {
        case Crane::AutoState::ToShip:
            // Move trolley out over ship cargo hold (Z = 2.4f)
            crane.trolleyZ = std::max(2.4f, crane.trolleyZ - crane.trolleySpeed * dt);
            if (crane.trolleyZ <= 2.45f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::LowerToShip;
            }
            break;

        case Crane::AutoState::LowerToShip:
            // Lower spreader down onto ship deck container (Y = 2.4f)
            crane.hookY = std::max(2.4f, crane.hookY - crane.hookSpeed * dt);
            if (crane.hookY <= 2.45f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::GrabCargo;
            }
            break;

        case Crane::AutoState::GrabCargo:
            // Lock twist-locks onto container
            if (crane.autoTimer > 0.4f) {
                crane.carrying = true;
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::LiftCargo;
                std::cout << ">>> [CRANE] Container locked onto spreader! Hoisting up.\n";
            }
            break;

        case Crane::AutoState::LiftCargo:
            // Lift container high above ship superstructure (Y = 6.2f)
            crane.hookY = std::min(6.2f, crane.hookY + crane.hookSpeed * dt);
            if (crane.hookY >= 6.15f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::ToYard;
            }
            break;

        case Crane::AutoState::ToYard:
            // Trolley moves container inward toward dock target zone (Z = 9.8f)
            crane.trolleyZ = std::min(9.8f, crane.trolleyZ + crane.trolleySpeed * dt);
            if (crane.trolleyZ >= 9.75f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::LowerToYard;
            }
            break;

        case Crane::AutoState::LowerToYard:
            // Lower container onto the dock flatbed (Y = 1.6f)
            crane.hookY = std::max(1.6f, crane.hookY - crane.hookSpeed * dt);
            if (crane.hookY <= 1.65f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::ReleaseCargo;
            }
            break;

        case Crane::AutoState::ReleaseCargo:
            if (crane.autoTimer > 0.5f) {
                crane.carrying = false;
                crane.score += 100;
                crane.cargoIndex = (crane.cargoIndex + 1) % 6;
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::ReturnCab;
                std::cout << ">>> [CRANE] Container deposited! SCORE: " << crane.score << "\n";
            }
            break;

        case Crane::AutoState::ReturnCab:
            // Hoist spreader back up
            crane.hookY = std::min(5.5f, crane.hookY + crane.hookSpeed * dt);
            if (crane.hookY >= 5.45f) {
                crane.autoTimer = 0.0f;
                crane.autoState = Crane::AutoState::Idle;
                std::cout << ">>> [CRANE] Unload cycle complete. Crane ready.\n";
            }
            break;

        default:
            break;
    }
}

static void updateWindowTitle(GLFWwindow* window) {
    std::string shadeStr = (currentShading == ShadingMode::Phong) ? "PHONG" : "GOURAUD";
    std::string camStr = "OVERVIEW";
    if (currentCamMode == CameraMode::Free) camStr = "FREE";
    else if (currentCamMode == CameraMode::CraneCab) camStr = "CRANE CAB (FPV)";
    else if (currentCamMode == CameraMode::ShipFollow) camStr = "SHIP FOLLOW";
    else if (currentCamMode == CameraMode::LighthouseView) camStr = "LIGHTHOUSE LOOKOUT";

    std::string craneStr = crane.manual ? (crane.carrying ? "MANUAL [HOLDING CARGO]" : "MANUAL [EMPTY]") : "AUTO";

    std::string title = "[3D PORT SIM] Shading: " + shadeStr +
                        " | Cam: " + camStr +
                        " | Crane: " + craneStr +
                        " | Score: " + std::to_string(crane.score) +
                        " | Sky [N]: " + (nightLights ? "MOON (NIGHT)" : "SUN (DAY)") +
                        " | RayTrace [R]: " + (raytraceMode ? "ON" : "OFF");
    glfwSetWindowTitle(window, title.c_str());
}

static void keyCallback(GLFWwindow* window, int key, int, int action, int) {
    if (action != GLFW_PRESS && action != GLFW_REPEAT) return;
    const bool pressed = (action == GLFW_PRESS);

    switch (key) {
        case GLFW_KEY_ESCAPE:
            glfwSetWindowShouldClose(window, GLFW_TRUE);
            break;

        case GLFW_KEY_SPACE:
            if (pressed) {
                if (crane.manual) {
                    crane.carrying = !crane.carrying;
                    if (crane.carrying) {
                        std::cout << ">>> [CRANE] CARGO ATTACHED TO SPREADER!\n";
                    } else {
                        crane.score += 100;
                        crane.cargoIndex = (crane.cargoIndex + 1) % 6;
                        std::cout << ">>> [CRANE] CARGO RELEASED! SCORE: " << crane.score << "\n";
                    }
                } else {
                    paused = !paused;
                }
            }
            break;

        case GLFW_KEY_N:
            if (pressed) {
                nightLights = !nightLights;
                std::cout << ">>> Day/Night Cycle toggled: " << (nightLights ? "NIGHT MODE (Moon & Lamps ON)" : "DAY MODE (Sun & Clouds)") << "\n";
            }
            break;

        // ---------- SHADING MODES (GOURAUD VS PHONG) ----------
        case GLFW_KEY_F1:
            if (pressed) {
                currentShading = ShadingMode::Gouraud;
                std::cout << ">>> Switched to GOURAUD Shading (Per-vertex lighting calculation)\n";
            }
            break;

        case GLFW_KEY_F2:
            if (pressed) {
                currentShading = ShadingMode::Phong;
                std::cout << ">>> Switched to PHONG Shading (Per-fragment lighting with sharp Specular glints)\n";
            }
            break;

        // ---------- RAY TRACING SHOWCASE (BONUS MARKS) ----------
        case GLFW_KEY_R:
            if (pressed) {
                raytraceMode = !raytraceMode;
                std::cout << ">>> Ray Tracing Showcase Mode: " << (raytraceMode ? "ACTIVE (Realtime Reflections)" : "INACTIVE") << "\n";
            }
            break;

        // ---------- CAMERA MODES ----------
        case GLFW_KEY_F3:
            if (pressed) {
                currentCamMode = CameraMode::Free;
                camera.distance = 36.0f;
                camera.pitch = 28.0f;
                camera.target = {0.0f, 1.5f, 2.0f};
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
                std::cout << ">>> Camera: PORT OVERVIEW (Cinematic Wide Angle)\n";
            }
            break;
        case GLFW_KEY_F5:
            if (pressed) {
                currentCamMode = CameraMode::CraneCab;
                std::cout << ">>> Camera: CRANE CABIN (First-Person View looking down at containers)\n";
            }
            break;
        case GLFW_KEY_F6:
            if (pressed) {
                currentCamMode = CameraMode::ShipFollow;
                std::cout << ">>> Camera: SHIP FOLLOW (Tracking cargo ship through canal)\n";
            }
            break;
        case GLFW_KEY_F7:
            if (pressed) {
                currentCamMode = CameraMode::LighthouseView;
                std::cout << ">>> Camera: LIGHTHOUSE LOOKOUT (High bird's-eye view)\n";
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
                    std::cout << ">>> [CRANE] CARGO ATTACHED TO SPREADER!\n";
                } else {
                    crane.score += 100;
                    crane.cargoIndex = (crane.cargoIndex + 1) % 6;
                    std::cout << ">>> [CRANE] CARGO DEPOSITED! SCORE: " << crane.score << "\n";
                }
            }
            break;

        // Slewing Yaw in Manual Mode
        case GLFW_KEY_Q:
            if (crane.manual) crane.slewingYaw -= 4.0f;
            break;
        case GLFW_KEY_E:
            if (crane.manual) crane.slewingYaw += 4.0f;
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

        // ---------- SHIPS ----------
        case GLFW_KEY_1:
            if (pressed) shipSpeedMultiplier = (shipSpeedMultiplier == 0.0f) ? 1.0f : 0.0f;
            break;
        case GLFW_KEY_2:
            if (pressed) shipSpeedMultiplier = 0.5f;
            break;
        case GLFW_KEY_3:
            if (pressed) shipSpeedMultiplier = 1.0f;
            break;
        case GLFW_KEY_4:
            if (pressed) shipSpeedMultiplier = 2.0f;
            break;

        // Camera Adjustments
        case GLFW_KEY_LEFT:
            camera.yaw -= 4.0f;
            break;
        case GLFW_KEY_RIGHT:
            camera.yaw += 4.0f;
            break;
        case GLFW_KEY_UP:
            camera.pitch = std::min(75.0f, camera.pitch + 3.0f);
            break;
        case GLFW_KEY_DOWN:
            camera.pitch = std::max(8.0f, camera.pitch - 3.0f);
            break;
        case GLFW_KEY_W:
            camera.distance = std::max(8.0f, camera.distance - 1.5f);
            break;
        case GLFW_KEY_S:
            camera.distance = std::min(80.0f, camera.distance + 1.5f);
            break;
        default:
            break;
    }

    updateWindowTitle(window);
}

static void processContinuousInput(GLFWwindow* window, float dt) {
    const float cameraSpeed = 24.0f * dt;

    if (currentCamMode == CameraMode::Free) {
        if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) camera.yaw -= cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) camera.yaw += cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) camera.pitch = std::min(80.0f, camera.pitch + cameraSpeed);
        if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) camera.pitch = std::max(5.0f, camera.pitch - cameraSpeed);
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

        // Trolley in/out
        if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
            crane.trolleyZ = std::max(1.8f, crane.trolleyZ - trolleyDelta); // Out toward ship
        if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
            crane.trolleyZ = std::min(11.2f, crane.trolleyZ + trolleyDelta); // In toward dock

        // Hook up/down
        if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
            crane.hookY = std::min(6.8f, crane.hookY + hookDelta);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
            crane.hookY = std::max(1.2f, crane.hookY - hookDelta);
    }
}

// ---------------- MAIN SCENE RENDERER ----------------
static void renderScene(int width, int height) {
    // Ray Tracing Full-screen Shader Showcase Mode (Bonus Marks)
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

    // Dynamic Camera Tracking based on Mode
    if (currentCamMode == CameraMode::CraneCab) {
        // Inside Crane Cabin looking down at the ship & trolley
        camera.target = vec3(8.0f, crane.hookY, crane.trolleyZ);
        camera.distance = 11.5f;
        camera.pitch = 50.0f;
        camera.yaw = -90.0f + crane.slewingYaw;
    } else if (currentCamMode == CameraMode::ShipFollow && !ships.empty()) {
        camera.target = vec3(ships[0].x, 0.8f, ships[0].z);
        camera.distance = 16.0f;
        camera.pitch = 18.0f;
    } else if (currentCamMode == CameraMode::LighthouseView) {
        camera.target = vec3(4.0f, 1.0f, 0.0f);
        camera.distance = 38.0f;
        camera.pitch = 24.0f;
        camera.yaw = -140.0f;
    }

    vec3 camPos = camera.position();
    mat4 projection = glm::perspective(glm::radians(45.0f), static_cast<float>(width) / std::max(1, height), 0.1f, 180.0f);
    mat4 view = camera.view();

    Shader& s = activeShader();
    s.use();
    s.setMat4("uView", view);
    s.setMat4("uProjection", projection);
    s.setVec3("uViewPos", camPos);
    s.setFloat("uTime", simTime);
    s.setBool("uIsWater", false);
    s.setBool("uIsEmissive", false);

    // 1. Directional Sunlight / Moonlight
    vec3 sunDirection = nightLights ? vec3(0.5f, -0.85f, -0.3f) : vec3(-0.45f, -0.9f, 0.35f);
    vec3 sunLightColor = nightLights ? vec3(0.35f, 0.42f, 0.65f) : vec3(1.35f, 1.30f, 1.18f); // Radiant warm daylight
    vec3 ambientLightColor = nightLights ? vec3(0.08f, 0.10f, 0.18f) : vec3(0.32f, 0.34f, 0.38f);

    s.setVec3("uSunDir", sunDirection);
    s.setVec3("uSunColor", sunLightColor);
    s.setVec3("uAmbientColor", ambientLightColor);

    // 2. Main Dock Quay Light (Active in both, intense at night)
    s.setBool("uPointLightEnabled", true);
    s.setVec3("uPointPos", vec3(8.0f, 5.5f, 6.8f));
    s.setVec3("uPointColor", nightLights ? vec3(1.3f, 0.95f, 0.45f) : vec3(0.6f, 0.58f, 0.52f));

    // 3. Spotlight & Multiple Dock Floodlights (in Phong Shader)
    if (currentShading == ShadingMode::Phong) {
        // Crane Spotlight pointing down from trolley onto container
        s.setBool("uSpotLightEnabled", nightLights);
        s.setVec3("uSpotPos", vec3(8.0f, 7.0f, crane.trolleyZ));
        s.setVec3("uSpotDir", vec3(0.0f, -1.0f, 0.0f));
        s.setVec3("uSpotColor", vec3(1.4f, 1.25f, 0.95f));
        s.setFloat("uSpotCutoff", std::cos(glm::radians(32.0f)));

        // 7 Multiple Point Lights along the dock apron
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

    // Render Scene Elements
    drawSunAndSky();
    drawSurroundingWorld();
    drawQuays();
    drawWaterAndCanal();
    drawBridge();
    drawLighthouse();
    drawContainerYard();
    drawCrane();
    drawDockedContainerShip();
    drawBuoys();
    drawLamps();

    // Vehicles & moving traffic
    drawTruck({14.0f, 0.0f, 18.5f}, 1.0f, true);
    drawTruck({-6.0f, 0.0f, 18.5f}, 3.0f, false);

    for (const Ship& ship : ships) {
        if (ship.active) drawMovingShip(ship);
    }
}

static void initShips() {
    ships = {
        {-28.0f, 3.6f, -2.0f, {0.18f, 0.42f, 0.82f}, 1.00f, true, false}, // Feeder cargo ship
        {-14.0f, 4.2f, -1.0f, {0.85f, 0.28f, 0.15f}, 0.75f, true, true},  // Harbor tugboat
        { 15.0f, 3.2f, -2.5f, {0.16f, 0.65f, 0.38f}, 0.92f, true, false}  // Green coaster
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

    GLFWwindow* window = glfwCreateWindow(1440, 880, "3D Sea Port Simulator — Computer Graphics Showcase", nullptr, nullptr);
    if (!window) {
        std::cerr << "GLFW window creation failed.\n";
        glfwTerminate();
        return 1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, keyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "GLAD initialization failed.\n";
        glfwDestroyWindow(window);
        glfwTerminate();
        return 1;
    }

    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << "\n";
    std::cout << "Renderer:       " << glGetString(GL_RENDERER) << "\n";

    std::cout << "\n==================================================================================\n";
    std::cout << "              🚢 3D SEA PORT SIMULATOR & GRAPHICS SHOWCASE 🚢\n";
    std::cout << "==================================================================================\n";
    std::cout << "  [F1] Gouraud Shading (Per-vertex)   | [F2] Phong Shading (Per-fragment specular)\n";
    std::cout << "  [N]  Day / Night Cycle (Sun vs Moon)| [R]  Ray Tracing Demo Mode (Bonus marks)\n";
    std::cout << "  [F3] Free Camera                    | [F4] Overview Camera\n";
    std::cout << "  [F5] Crane Operator Cab View (FPV)  | [F6] Ship Follow Camera  | [F7] Lighthouse\n";
    std::cout << "  ------------------------------------------------------------------------------\n";
    std::cout << "  [G]  Auto Crane Unload Cycle        | [T]  Toggle Manual Crane Mode\n";
    std::cout << "  [Q/E] Rotate Crane Turret           | [ [ / ] ] Move Trolley (In/Out)\n";
    std::cout << "  [U/D] Hoist Hook (Up/Down)          | [P / SPACE] Pick / Release Container\n";
    std::cout << "  [A/B/M] Drawbridge Auto / Manual    | Hold [O]/[C] Open / Close Bridge\n";
    std::cout << "  [1/2/3/4] Ships Speed Multiplier    | [ESC] Exit Simulator\n";
    std::cout << "==================================================================================\n\n";

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE); // Disabling face culling so all container and building faces are 100% solid

    try {
        phongShader.load("shaders/phong.vert", "shaders/phong.frag");
        gouraudShader.load("shaders/gouraud.vert", "shaders/gouraud.frag");
        raytraceShader.load("shaders/quad.vert", "shaders/raytracer.frag");

        cubeMesh = makeCube();
        cylinderMesh = makeCylinder(32);
        ringMesh = makeRing(32);
        sphereMesh = makeSphere(18, 28, 1.0f);
        waterMesh = makeWaterGrid(80, 24, 80.0f, 10.6f);
        initQuad();
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

        // Sky atmosphere clear color
        if (nightLights) glClearColor(0.02f, 0.04f, 0.10f, 1.0f); // Deep starry night
        else glClearColor(0.42f, 0.70f, 0.92f, 1.0f);            // Crisp bright sunny sky

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderScene(width, height);
        glfwSwapBuffers(window);
    }

    phongShader.destroy();
    gouraudShader.destroy();
    raytraceShader.destroy();
    cubeMesh.destroy();
    cylinderMesh.destroy();
    ringMesh.destroy();
    sphereMesh.destroy();
    waterMesh.destroy();
    if (quadVAO) glDeleteVertexArrays(1, &quadVAO);
    if (quadVBO) glDeleteBuffers(1, &quadVBO);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
