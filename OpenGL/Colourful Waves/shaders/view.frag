# version 330 core

in highp vec2 fragTextCoord;
uniform sampler2D tex;

void main() {
    highp vec4 col = texture(tex, fragTextCoord);
    gl_FragColor = col;
}