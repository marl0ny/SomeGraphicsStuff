# version 330 core

uniform vec3 dynColour;
out vec4 colour;

void main () {
    colour = vec4(dynColour, 1.0);
}
