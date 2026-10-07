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

out vec4 FragColor;

void main() {
    vec3 n = normalize(Normal);
    vec3 sunDir = normalize(-uSunDir);
    float diff = max(dot(n, sunDir), 0.0);

    vec3 viewDir = normalize(uViewPos - FragPos);
    vec3 reflectDir = reflect(-sunDir, n);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 lighting = uAmbientColor + diff * uSunColor + 0.22 * spec * uSunColor;

    if (uPointLightEnabled) {
        vec3 toLight = uPointPos - FragPos;
        float dist = length(toLight);
        vec3 pointDir = normalize(toLight);
        float pointDiff = max(dot(n, pointDir), 0.0);
        float attenuation = 1.0 / (1.0 + 0.08 * dist + 0.025 * dist * dist);
        lighting += attenuation * pointDiff * uPointColor;
    }

    FragColor = vec4(uColor * lighting, 1.0);
}
