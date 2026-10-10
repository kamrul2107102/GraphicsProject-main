#version 330 core
in vec3 vWorldPos;

uniform vec3 uSunDir;
uniform vec3 uSunColor;
uniform bool uNight;
uniform float uTime;

out vec4 FragColor;

// 2D Hash function
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

// 2D Value Noise with smooth interpolation
float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f); // Hermite curve

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Fractal Brownian Motion (FBM) for soft layered realistic clouds
float fbm(vec2 p) {
    float v = 0.0;
    float a = 0.5;
    mat2 rot = mat2(0.8, -0.6, 0.6, 0.8);
    for (int i = 0; i < 5; ++i) {
        v += a * noise(p);
        p = rot * p * 2.02 + vec2(0.35, 0.42);
        a *= 0.5;
    }
    return v;
}

void main() {
    vec3 dir = normalize(vWorldPos);
    float y = dir.y; // Height on dome (-1 to +1)

    // Sun / Moon vectors
    vec3 sunDirNorm = normalize(-uSunDir);
    float sunDot = max(dot(dir, sunDirNorm), 0.0);

    vec3 skyColor;

    if (!uNight) {
        // DAYTIME ATMOSPHERE: Deep zenith blue transitioning to warm horizon haze
        vec3 zenithColor = vec3(0.24, 0.52, 0.88); // Sky blue
        vec3 horizonColor = vec3(0.72, 0.84, 0.94); // Light airy horizon
        vec3 groundHaze = vec3(0.68, 0.78, 0.85);

        float h = clamp(y * 1.5, 0.0, 1.0);
        skyColor = mix(horizonColor, zenithColor, pow(h, 0.75));

        if (y < 0.0) {
            skyColor = mix(horizonColor, groundHaze, clamp(-y * 2.0, 0.0, 1.0));
        }

        // Realistic compact Sun disc and natural atmospheric solar glow over the sea
        float sunDisc = smoothstep(0.9990, 0.9998, sunDot);
        float sunGlow = pow(sunDot, 48.0) * 0.85 + pow(sunDot, 12.0) * 0.40 + pow(sunDot, 3.0) * 0.15;
        skyColor += vec3(1.0, 0.97, 0.85) * sunDisc * 3.0;
        skyColor += vec3(1.0, 0.85, 0.50) * sunGlow;

        // REALISTIC WISPY & CUMULUS CLOUD LAYERS (Planar projection onto sky ceiling)
        if (y > 0.02) {
            vec2 cloudUV = dir.xz / (y + 0.18); // Perspective flattening toward horizon
            vec2 wind = vec2(uTime * 0.012, uTime * 0.006);

            // Layer 1: Soft wispy cirrus / cumulus clouds
            float c1 = fbm(cloudUV * 1.8 + wind);
            // Layer 2: Detail turbulence
            float c2 = fbm(cloudUV * 3.6 - wind * 0.8);
            float cloudDensity = smoothstep(0.42, 0.78, c1 + 0.25 * c2);

            // Shading on clouds (Silver sunlit highlights & soft grey undersides)
            vec3 cloudHighlight = vec3(1.0, 1.0, 1.0);
            vec3 cloudShadow = vec3(0.74, 0.78, 0.86);
            vec3 cloudFinal = mix(cloudShadow, cloudHighlight, clamp(c1 * 1.2, 0.0, 1.0));

            // Fade clouds near horizon to blend into distance fog
            float horizonFade = smoothstep(0.02, 0.22, y);
            cloudDensity *= horizonFade;

            skyColor = mix(skyColor, cloudFinal, cloudDensity * 0.80);

            // Realistic solar glare shines naturally through the clouds
            skyColor += vec3(1.0, 0.97, 0.85) * sunDisc * 2.2;
            skyColor += vec3(1.0, 0.88, 0.55) * (pow(sunDot, 32.0) * 0.55 + pow(sunDot, 8.0) * 0.25);
        }

    } else {
        // NIGHT ATMOSPHERE: Deep navy zenith down to twilight midnight horizon
        vec3 nightZenith = vec3(0.01, 0.02, 0.06);
        vec3 nightHorizon = vec3(0.04, 0.08, 0.16);

        float h = clamp(y * 1.5, 0.0, 1.0);
        skyColor = mix(nightHorizon, nightZenith, pow(h, 0.8));

        // Realistic compact Moon disc over the sea
        vec3 moonDir = sunDirNorm;
        float moonDot = max(dot(dir, moonDir), 0.0);
        float moonDisc = smoothstep(0.9992, 0.9998, moonDot);
        float moonHalo = pow(moonDot, 48.0) * 0.50 + pow(moonDot, 8.0) * 0.18;
        skyColor += vec3(0.92, 0.95, 1.0) * moonDisc * 2.2;
        skyColor += vec3(0.40, 0.55, 0.85) * moonHalo;

        // Realistic field of twinkling background stars
        if (y > 0.05) {
            vec2 starCoord = dir.xz / (y + 0.3) * 60.0;
            vec2 starCell = floor(starCoord);
            float starHash = hash(starCell);
            if (starHash > 0.965) {
                float twinkle = 0.5 + 0.5 * sin(uTime * 3.0 + starHash * 100.0);
                float starIntensity = smoothstep(0.965, 1.0, starHash) * twinkle;
                skyColor += vec3(0.9, 0.95, 1.0) * starIntensity * smoothstep(0.05, 0.25, y);
            }

            // Night time dark cloud silhouettes drifting across stars
            vec2 cloudUV = dir.xz / (y + 0.2);
            vec2 wind = vec2(uTime * 0.01, uTime * 0.005);
            float c1 = fbm(cloudUV * 1.6 + wind);
            float cloudDensity = smoothstep(0.48, 0.80, c1);
            float horizonFade = smoothstep(0.05, 0.25, y);
            cloudDensity *= horizonFade;

            vec3 nightCloudCol = vec3(0.025, 0.045, 0.085);
            skyColor = mix(skyColor, nightCloudCol, cloudDensity * 0.75);
        }
    }

    FragColor = vec4(skyColor, 1.0);
}
