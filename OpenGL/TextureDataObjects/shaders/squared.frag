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
    vec4 v = texture2D(tex, UV);
    float res = 0.0;
    if (size >= 1)
        res += v.r*v.r;
    if (size >= 2)
        res += v.g*v.g;
    if (size >= 3)
        res += v.b*v.b;
    if (size >= 4)
        res += v.a*v.a;
    fragColor = vec4(res, res, res, res);
}