#version 330 core

attribute vec3 position;
varying highp vec2 fragTextCoord;
uniform int texCoordType;

void main () {
    if (texCoordType == 0) {
        fragTextCoord = vec2(0.5, 0.5) + vec2(position.x, position.y)/2.0;
        gl_Position = vec4(position.xyz, 1.0);
    } else {
        fragTextCoord = position.xy;
        gl_Position = vec4(position.xyz, 1.0);
    }
}
