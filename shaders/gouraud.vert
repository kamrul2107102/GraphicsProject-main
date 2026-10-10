#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProjection;

uniform vec3 uColor;
uniform vec3 uViewPos;
uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform vec3 uAmbientColor;

uniform bool uPointLightEnabled;
uniform vec3 uPointPos;
uniform vec3 uPointColor;

uniform float uShininess;
uniform vec3 uSpecularColor;

uniform float uTime;
uniform bool uIsWater;
uniform bool uIsEmissive;

out vec3 vColor;

void main() {
    if (uIsEmissive) {
        vColor = uColor;
        gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);
        return;
    }
    vec3 pos = aPos;

    // Animated water wave displacement for Gouraud too
    vec3 normal = aNormal;
    if (uIsWater) {
        float wave1 = sin(pos.x * 1.2 + uTime * 2.5) * 0.12;
        float wave2 = cos(pos.z * 1.8 + uTime * 3.0) * 0.08;
        float wave3 = sin((pos.x + pos.z) * 0.8 + uTime * 1.8) * 0.06;
        pos.y += wave1 + wave2 + wave3;

        float dx = 1.2 * cos(pos.x * 1.2 + uTime * 2.5) * 0.12
                  + 0.8 * cos((pos.x + pos.z) * 0.8 + uTime * 1.8) * 0.06;
        float dz = -1.8 * sin(pos.z * 1.8 + uTime * 3.0) * 0.08
                  + 0.8 * cos((pos.x + pos.z) * 0.8 + uTime * 1.8) * 0.06;
        normal = normalize(vec3(-dx, 1.0, -dz));
    }

    vec4 worldPos = uModel * vec4(pos, 1.0);
    vec3 fragPos = worldPos.xyz;
    vec3 n = normalize(mat3(transpose(inverse(uModel))) * normal);

    // Per-vertex Gouraud lighting
    vec3 sunDir = normalize(-uSunDir);
    float diff = max(dot(n, sunDir), 0.0);

    vec3 viewDir = normalize(uViewPos - fragPos);
    vec3 halfDir = normalize(sunDir + viewDir);
    float spec = pow(max(dot(n, halfDir), 0.0), uShininess);

    vec3 lighting = uAmbientColor + diff * uSunColor + spec * uSpecularColor * uSunColor;

    // Point Light
    if (uPointLightEnabled) {
        vec3 toLight = uPointPos - fragPos;
        float dist = length(toLight);
        vec3 pointDir = normalize(toLight);
        float pointDiff = max(dot(n, pointDir), 0.0);
        vec3 pointHalf = normalize(pointDir + viewDir);
        float pointSpec = pow(max(dot(n, pointHalf), 0.0), uShininess);

        float attenuation = 1.0 / (1.0 + 0.06 * dist + 0.012 * dist * dist);
        lighting += attenuation * (pointDiff * uPointColor + pointSpec * uSpecularColor * uPointColor);
    }

    vec3 baseColor = uColor * lighting;
    float distToCam = length(uViewPos - fragPos);
    float fogFactor = clamp((distToCam - 140.0) / 260.0, 0.0, 0.85);
    vec3 fogColor = (uSunColor.r < 0.8) ? vec3(0.02, 0.04, 0.10) : vec3(0.55, 0.72, 0.88);
    vColor = mix(baseColor, fogColor, fogFactor);

    gl_Position = uProjection * uView * worldPos;
}
