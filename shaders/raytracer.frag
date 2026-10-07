#version 330 core
out vec4 FragColor;

uniform vec2 uResolution;
uniform float uTime;
uniform vec3 uCamPos;
uniform bool uNight;

struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
    float specular;
    float reflectivity;
};

// Ray intersection with Sphere
float intersectSphere(vec3 ro, vec3 rd, Sphere s) {
    vec3 oc = ro - s.center;
    float b = dot(oc, rd);
    float c = dot(oc, oc) - s.radius * s.radius;
    float h = b * b - c;
    if (h < 0.0) return -1.0;
    float t = -b - sqrt(h);
    if (t > 0.001) return t;
    t = -b + sqrt(h);
    if (t > 0.001) return t;
    return -1.0;
}

// Ray intersection with water plane (y = -0.4)
float intersectPlane(vec3 ro, vec3 rd) {
    float planeY = -0.4;
    if (abs(rd.y) < 0.001) return -1.0;
    float t = (planeY - ro.y) / rd.y;
    return (t > 0.001) ? t : -1.0;
}

void main() {
    // Normalized pixel coordinates [-1, 1]
    vec2 uv = (gl_FragCoord.xy - 0.5 * uResolution) / uResolution.y;

    // Camera setup
    vec3 ro = vec3(0.0, 2.5, 7.0);
    vec3 target = vec3(0.0, 0.5, 0.0);
    vec3 fwd = normalize(target - ro);
    vec3 right = normalize(cross(fwd, vec3(0.0, 1.0, 0.0)));
    vec3 up = cross(right, fwd);
    vec3 rd = normalize(uv.x * right + uv.y * up + 1.6 * fwd);

    // Light source
    vec3 lightDir = normalize(uNight ? vec3(0.4, 0.9, 0.3) : vec3(-0.6, 1.0, 0.5));
    vec3 lightCol = uNight ? vec3(0.3, 0.4, 0.7) : vec3(1.0, 0.95, 0.85);
    vec3 skyCol = uNight ? vec3(0.02, 0.03, 0.07) : vec3(0.35, 0.6, 0.85);

    // Spheres representing harbor buoys and shiny metallic navigational orbs
    Sphere spheres[3];
    spheres[0] = Sphere(vec3(-2.2, 0.6 + sin(uTime * 2.0) * 0.15, 0.0), 0.85, vec3(0.85, 0.2, 0.15), 64.0, 0.4); // Red Port Buoy
    spheres[1] = Sphere(vec3( 0.0, 1.0 + cos(uTime * 1.8) * 0.2, -1.2), 1.05, vec3(0.9, 0.85, 0.8), 128.0, 0.75); // Chrome Sphere
    spheres[2] = Sphere(vec3( 2.2, 0.5 + sin(uTime * 2.2 + 1.0) * 0.15, 0.2), 0.75, vec3(0.15, 0.7, 0.3), 48.0, 0.3); // Green Buoy

    vec3 finalColor = vec3(0.0);
    vec3 curRo = ro;
    vec3 curRd = rd;
    float throughput = 1.0;

    // Ray tracing bounce loop (Primary ray + 1 reflection bounce)
    for (int bounce = 0; bounce < 2; ++bounce) {
        float closestT = 1e5;
        int hitType = 0; // 0=none, 1=sphere, 2=plane
        int hitSphereIdx = -1;
        vec3 hitNormal = vec3(0.0);
        vec3 hitColor = vec3(0.0);
        float hitSpec = 32.0;
        float hitRefl = 0.0;

        for (int i = 0; i < 3; ++i) {
            float t = intersectSphere(curRo, curRd, spheres[i]);
            if (t > 0.0 && t < closestT) {
                closestT = t;
                hitType = 1;
                hitSphereIdx = i;
            }
        }

        float tp = intersectPlane(curRo, curRd);
        if (tp > 0.0 && tp < closestT) {
            closestT = tp;
            hitType = 2;
        }

        if (hitType == 0) {
            // Miss: Sky background with sun glow
            float sunGlow = max(dot(curRd, lightDir), 0.0);
            vec3 sky = skyCol + pow(sunGlow, 16.0) * lightCol * 0.8;
            finalColor += throughput * sky;
            break;
        }

        vec3 hitPos = curRo + curRd * closestT;

        if (hitType == 1) {
            Sphere s = spheres[hitSphereIdx];
            hitNormal = normalize(hitPos - s.center);
            hitColor = s.color;
            hitSpec = s.specular;
            hitRefl = s.reflectivity;
        } else if (hitType == 2) {
            // Water plane with subtle wave normal
            float wave = sin(hitPos.x * 3.0 + uTime * 3.0) * cos(hitPos.z * 3.0 + uTime * 2.5) * 0.05;
            hitNormal = normalize(vec3(wave, 1.0, wave));
            // Water color checker/depth
            float grid = mod(floor(hitPos.x * 0.5) + floor(hitPos.z * 0.5), 2.0);
            hitColor = mix(vec3(0.05, 0.22, 0.42), vec3(0.03, 0.16, 0.32), grid);
            hitSpec = 64.0;
            hitRefl = 0.45;
        }

        // Shadow ray
        float shadow = 1.0;
        vec3 shadowRo = hitPos + hitNormal * 0.002;
        for (int i = 0; i < 3; ++i) {
            if (i != hitSphereIdx) {
                float st = intersectSphere(shadowRo, lightDir, spheres[i]);
                if (st > 0.0) { shadow = 0.25; break; }
            }
        }

        // Blinn-Phong shading at intersection
        float diff = max(dot(hitNormal, lightDir), 0.0);
        vec3 refl = reflect(curRd, hitNormal);
        float spec = pow(max(dot(refl, lightDir), 0.0), hitSpec);
        vec3 ambient = (uNight ? 0.08 : 0.2) * hitColor;
        vec3 localShading = ambient + shadow * (diff * hitColor * lightCol + spec * lightCol * 0.6);

        finalColor += throughput * (1.0 - hitRefl) * localShading;

        // Next bounce
        throughput *= hitRefl;
        if (throughput < 0.05) break;

        curRo = hitPos + hitNormal * 0.002;
        curRd = refl;
    }

    FragColor = vec4(finalColor, 1.0);
}
