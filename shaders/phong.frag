#version 330 core
in vec3 FragPos;
in vec3 Normal;

uniform vec3 uColor;
uniform vec3 uViewPos;
uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform vec3 uAmbientColor;

uniform bool uPointLightEnabled;
uniform vec3 uPointPos;
uniform vec3 uPointColor;

// Crane Spotlight
uniform bool uSpotLightEnabled;
uniform vec3 uSpotPos;
uniform vec3 uSpotDir;
uniform vec3 uSpotColor;
uniform float uSpotCutoff;

// Multiple Point Lights (dock lamps)
uniform bool uMultiPointEnabled;
uniform vec3 uPointLights[7];
uniform vec3 uPointLightColors[7];

uniform float uShininess;
uniform vec3 uSpecularColor;

uniform bool uIsWater;
uniform bool uIsEmissive;
uniform float uAlpha;
uniform float uTime;

out vec4 FragColor;

void main() {
    float alphaVal = (uAlpha > 0.001) ? uAlpha : 1.0;
    if (uIsEmissive) {
        FragColor = vec4(uColor, alphaVal);
        return;
    }
    vec3 n = normalize(Normal);
    vec3 viewDir = normalize(uViewPos - FragPos);

    // 1. Directional Light (Sun/Moon) — Phong per-fragment
    vec3 sunDir = normalize(-uSunDir);
    float diff = max(dot(n, sunDir), 0.0);
    vec3 halfDir = normalize(sunDir + viewDir); // Blinn-Phong
    float spec = pow(max(dot(n, halfDir), 0.0), uShininess);

    vec3 lighting = uAmbientColor + diff * uSunColor + spec * uSpecularColor * uSunColor;

    // 2. Main Point Light
    if (uPointLightEnabled) {
        vec3 toLight = uPointPos - FragPos;
        float dist = length(toLight);
        vec3 pointDir = normalize(toLight);
        float pointDiff = max(dot(n, pointDir), 0.0);
        vec3 pointHalf = normalize(pointDir + viewDir);
        float pointSpec = pow(max(dot(n, pointHalf), 0.0), uShininess);

        float attenuation = 1.0 / (1.0 + 0.06 * dist + 0.012 * dist * dist);
        lighting += attenuation * (pointDiff * uPointColor + pointSpec * uSpecularColor * uPointColor);
    }

    // 3. Multiple Dock Lamp Point Lights
    if (uMultiPointEnabled) {
        for (int i = 0; i < 7; ++i) {
            vec3 toLight = uPointLights[i] - FragPos;
            float dist = length(toLight);
            vec3 plDir = normalize(toLight);
            float plDiff = max(dot(n, plDir), 0.0);
            float atten = 1.0 / (1.0 + 0.09 * dist + 0.02 * dist * dist);
            lighting += atten * plDiff * uPointLightColors[i];
        }
    }

    // 4. Spotlight (Crane Worklight)
    if (uSpotLightEnabled) {
        vec3 toSpot = uSpotPos - FragPos;
        float dist = length(toSpot);
        vec3 lightDir = normalize(toSpot);
        float theta = dot(lightDir, normalize(-uSpotDir));

        if (theta > uSpotCutoff) {
            float spotDiff = max(dot(n, lightDir), 0.0);
            vec3 spotHalf = normalize(lightDir + viewDir);
            float spotSpec = pow(max(dot(n, spotHalf), 0.0), uShininess);
            float atten = 1.0 / (1.0 + 0.04 * dist + 0.008 * dist * dist);
            float intensity = clamp((theta - uSpotCutoff) / (1.0 - uSpotCutoff), 0.0, 1.0);
            lighting += atten * intensity * (spotDiff * uSpotColor + spotSpec * uSpecularColor * uSpotColor);
        }
    }

    vec3 finalColor = uColor * lighting;

    // Water: add specular shimmer and foam-like edge
    if (uIsWater) {
        // Extra specular glitter on water
        float sunGlitter = pow(max(dot(reflect(-sunDir, n), viewDir), 0.0), 128.0);
        finalColor += sunGlitter * uSunColor * 0.6;

        // Subtle fresnel effect - edges appear brighter
        float fresnel = pow(1.0 - max(dot(viewDir, n), 0.0), 3.0);
        finalColor = mix(finalColor, uSunColor * 0.4, fresnel * 0.25);
    }

    // Smooth atmospheric distance fog blending into the sky horizon
    float distToCam = length(uViewPos - FragPos);
    float fogFactor = clamp((distToCam - 140.0) / 260.0, 0.0, 0.85);
    vec3 fogColor = (uSunColor.r < 0.8) ? vec3(0.02, 0.04, 0.10) : vec3(0.55, 0.72, 0.88);
    finalColor = mix(finalColor, fogColor, fogFactor);

    FragColor = vec4(finalColor, alphaVal);
}
