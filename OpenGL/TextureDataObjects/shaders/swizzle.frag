#VERSION_NUMBER_PLACEHOLDER

precision highp float;

#if __VERSION__ >= 300
in vec2 UV;
out vec4 fragColor;
#define texture2D texture
#else
#define fragColor gl_FragColor
varying highp vec2 UV;
#endif

uniform sampler2D tex;
uniform int c0;
uniform int c1;
uniform int c2;
uniform int c3;

void main() {
    vec4 val = texture2D(tex, UV);
    float x = (c0 < 4 && c0 >= 0)? val[c0]: 0.0;
    float y = (c1 < 4 && c1 >= 0)? val[c1]: 0.0;
    float z = (c2 < 4 && c2 >= 0)? val[c2]: 0.0;
    float w = (c3 < 4 && c3 >= 0)? val[c3]: 0.0;
    fragColor = vec4(x, y, z, w);
}