#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec3 position;
in vec2 colour;
#define texture2D texture
out highp vec2 UV;
#else
attribute vec3 position;
attribute vec2 colour;
varying highp UV;
#endif

uniform sampler2D tex;
uniform float scale;

void main() {
    vec2 offset = texture2D(tex, position.xy).xy;
    gl_Position = vec4(vec3(offset, 0.0)
                       + scale*position - vec3(0.5, 0.5, 0.0), 1.0);
    UV = position.xy;
}
