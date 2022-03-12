# version 330 core

precision highp float;

in vec2 fragTextCoord;
out vec4 fragColor;
uniform sampler2D tex;

void main() {
    highp vec4 col = texture(tex, fragTextCoord);
    fragColor = col;
}