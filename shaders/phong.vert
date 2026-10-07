#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform float uTime;
uniform bool uIsWater;

out vec3 FragPos;
out vec3 Normal;

void main() {
    vec3 pos = aPos;

    // Animated water wave displacement
    if (uIsWater) {
        float wave1 = sin(pos.x * 1.2 + uTime * 2.5) * 0.12;
        float wave2 = cos(pos.z * 1.8 + uTime * 3.0) * 0.08;
        float wave3 = sin((pos.x + pos.z) * 0.8 + uTime * 1.8) * 0.06;
        pos.y += wave1 + wave2 + wave3;

        // Compute wave normal from derivatives
        float dx = 1.2 * cos(pos.x * 1.2 + uTime * 2.5) * 0.12
                  + 0.8 * cos((pos.x + pos.z) * 0.8 + uTime * 1.8) * 0.06;
        float dz = -1.8 * sin(pos.z * 1.8 + uTime * 3.0) * 0.08
                  + 0.8 * cos((pos.x + pos.z) * 0.8 + uTime * 1.8) * 0.06;
        vec3 waveNormal = normalize(vec3(-dx, 1.0, -dz));
        Normal = mat3(transpose(inverse(uModel))) * waveNormal;
    } else {
        Normal = mat3(transpose(inverse(uModel))) * aNormal;
    }

    vec4 worldPos = uModel * vec4(pos, 1.0);
    FragPos = worldPos.xyz;
    gl_Position = uProjection * uView * worldPos;
}
