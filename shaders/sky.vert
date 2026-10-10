#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 uView;
uniform mat4 uProjection;

out vec3 vWorldPos;

void main() {
    vWorldPos = aPos;
    // Remove translation from view matrix so the sky dome stays centered on the camera
    mat4 viewNoTrans = mat4(mat3(uView));
    vec4 pos = uProjection * viewNoTrans * vec4(aPos, 1.0);
    // Setting z = w ensures depth is at the maximum (1.0 in NDC)
    gl_Position = pos.xyww;
}
