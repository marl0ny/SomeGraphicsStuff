# version 330 core

in vec2 position;
uniform mat4 rotation;

void main () {
    gl_Position = rotation*vec4(position, 0.0, 1.0);
}
