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
uniform int size;

void main() {
    vec4 val = texture2D(tex, UV);
    float res = 0.0;
    // No for loop since older versions of OpenGL
    // do not support for loops.
    if (size >= 1)
        res += val[0];
    if (size >= 2)
        res += val[1];
    if (size >= 3)
        res += val[2];
    if (size >= 4)
        res += val[3];
    fragColor = vec4(res, res, res, res);
}
