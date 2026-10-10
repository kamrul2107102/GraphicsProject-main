#version 330 core
in vec2 TexCoord;

uniform sampler2D uFontTexture;
uniform vec4 uColor;
uniform bool uUseTexture;

out vec4 FragColor;

void main() {
    if (uUseTexture) {
        float alpha = texture(uFontTexture, TexCoord).r;
        if (alpha < 0.1) discard;
        FragColor = vec4(uColor.rgb, uColor.a * alpha);
    } else {
        FragColor = uColor;
    }
}
