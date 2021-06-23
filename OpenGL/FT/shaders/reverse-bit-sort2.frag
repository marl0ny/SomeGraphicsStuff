/*
* In order to get this to work,
* it was learned that using the function texture2D 
* to sample the given textures using values that are
* not uniforms, constants, or the incoming varying
* texture coordinates is actually undefined behaviour.
* This issue is mentioned Khronos OpenGL Wiki:
*  https://www.khronos.org/opengl/wiki/Sampler_(GLSL)
*  #Non-uniform_flow_control
* Unlike texture2D, texelFetch seems to escape this issue.
*/

# version 150 core

varying highp vec2 fragTextCoord;
precision highp float;
uniform float width;
uniform float height;
uniform sampler2D tex;
uniform sampler2D lookupTex;


void main() {
    vec2 xy = fragTextCoord;
    vec2 lookupPos = texture2D(lookupTex, xy).xy;
    vec4 col = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 intLookup = ivec2(int(width*lookupPos.x), int(height*lookupPos.y));
    col += texelFetch(tex, intLookup, 0);
    gl_FragColor =  col;
}
