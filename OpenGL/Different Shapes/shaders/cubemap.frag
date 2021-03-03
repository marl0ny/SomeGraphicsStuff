#version 150 core

varying vec3 fragPosition;
varying vec2 fragTexture;
uniform sampler2D tex;


void main () {
    gl_FragColor = texture(tex, fragTexture);
}