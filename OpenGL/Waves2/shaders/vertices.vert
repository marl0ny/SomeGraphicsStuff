# version 330 core

precision highp float;

in vec3 position;
out vec2 fragTextCoord;


/*
The vertices
-1.0, -1.0, 0.0,
-1.0, 1.0, 0.0, 
1.0, 1.0, 0.0, 
1.0, -1.0, 0.0
ensure that the quad covers the entire view space
and camera view.
When passing it to the fragment shader, this view
is transformed to only the upper quadrant since this
contains the full extent of the texture coordinates.
*/
void main () {
    gl_Position = vec4(position.xyz, 1.0);
    fragTextCoord = position.xy/2.0 + vec2(0.5, 0.5);
}