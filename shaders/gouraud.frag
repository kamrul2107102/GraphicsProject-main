#version 330 core
in vec3 vColor;
uniform float uAlpha;
out vec4 FragColor;

void main() {
    float a = (uAlpha > 0.001) ? uAlpha : 1.0;
    FragColor = vec4(vColor, a);
}
