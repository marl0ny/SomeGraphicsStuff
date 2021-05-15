# version 330 core

in vec3 position;
out highp vec2 fragTextCoord;

void main () {
    gl_Position = vec4(position.xyz, 1.0);
    fragTextCoord = vec2(0.5, 0.5) + position.xy/2.0;
}
